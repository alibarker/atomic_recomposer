/*
  ==============================================================================

    AtomicAudioEngine.cpp
    Created: 8 Jul 2016 12:29:40pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "AtomicAudioEngine.h"
#include "AtomicAudioSource.h"

AtomicAudioEngine::AtomicAudioEngine(int wiviWidth, int wiviHeight) : Thread("Decomposition")
{
    wivigramWidth = wiviWidth;
    wivigramHeight = wiviHeight;
    
    bleedValue = 1.0;
    
    startThread();
    
}

void AtomicAudioEngine::setScrubbing(bool status)
{
    atomicSource->setLooping(status);
}


void AtomicAudioEngine::updateWivigram()
{
    
    map = new MP_TF_Map_c(wivigramWidth, wivigramHeight, book->numChans, 0, book->numSamples, 0.0, 0.5);
    {
        ScopedReadLock srl(bookLock);
        book->add_to_tfmap(map, MP_TFMAP_PSEUDO_WIGNER, NULL);
    
//        wivigram->updateWivigram(map);
    }
    
    sendChangeMessage();
    
}



void AtomicAudioEngine::setStatus(String val)
{
    ScopedWriteLock srl (statusLock);
    status = val;
}

void AtomicAudioEngine::decomposition()
{
    
    
    setStatus(String("Beginning Decomposition"));
    
    ScopedPointer<MP_Dict_c>			dict;
    ScopedPointer<MP_Signal_c>			sig;
    ScopedPointer<MP_Mpd_Core_c>		mpdCore;
    

    /* Load the MPTK environment */
    
    setStatus(String("Loading MPTK Environment"));

    if(! (MPTK_Env_c::get_env()->load_environment_if_needed("/usr/local/mptk/path.xml")) )
    {
        return;
    }
    
    /* Read the dictionary */
    
    setStatus(String("Loading Dictionary"));

    dict = MP_Dict_c::read_from_xml_file( dictionary.getFullPathName().toRawUTF8());
    if ( dict == NULL )
    {
        return;
    }
    if ( dict->numBlocks == 0 )
    {
        return;
    }

    
    /* Load the signal */
    
    setStatus("Loading signal");
    
    sig = MP_Signal_c::init( signal.getFullPathName().toRawUTF8() );
    if ( sig == NULL )
    {
        return;
    }
    
    /* Acquire Lock */
    {
        setStatus("Creating Book");
        
        ScopedWriteLock sl(bookLock);
        startTimerHz(50);
        
        /* Create the book */
        book = MP_Book_c::create(sig->numChans, sig->numSamples, sig->sampleRate );
        if ( book == NULL )
        {
            return;
        }
        
        setStatus("Creating mpd Core");
        
        /* Set up the core */
        mpdCore = MP_Mpd_Core_c::create( sig, book, dict );
        mpdCore->set_iter_condition( numIterations );
        
        setStatus("Running Matcing Pursuit");

        mpdCore->run();
        
        setStatus("Preparing Book for Playback");
        
        MP_Signal_c* sig = MP_Signal_c::init(book->numChans, book->numSamples, book->sampleRate);
        
        book->substract_add(NULL, sig, NULL);
        
        String filename(String(signal.getParentDirectory().getFullPathName() + "/" +  signal.getFileNameWithoutExtension() + "approx" + signal.getFileExtension()));
        
        DBG(filename);
        sig->wavwrite(filename.getCharPointer());
        
        
        atomicSource.release();
        atomicSource = new AtomicAudioSource(this);
        prepareBook();
        transportSource.setSource(atomicSource);
      
        startDecomposition = false;
        
        setStatus("");
    }
}

void AtomicAudioEngine::run()
{
    while (!threadShouldExit()) {
        if (startDecomposition) {
            decomposition();
            
        }
        
        wait(1000);
    }
}

void AtomicAudioEngine::triggerDecomposition(File dict, File sig, int numIter)
{
    dictionary = File(dict);
    signal = File(sig);
    numIterations = numIter;
    
    transportSource.stop();
    
    startDecomposition = true;
    notify();
    
}

void AtomicAudioEngine::setTransportPosition(float posAsPercentage)
{
    int length = transportSource.getTotalLength();
    if (length != 0) {
        int64 newPos = min ( (int) round(posAsPercentage*length), length -1) ;
        transportSource.setNextReadPosition( newPos );
    }
}


float AtomicAudioEngine::getTransportPosition()
{
    int length = transportSource.getTotalLength();
    if (length != 0) {
        int64 pos = transportSource.getNextReadPosition();
        return (float) pos / (float) length;
    }
    
    return 0.0;
}

void AtomicAudioEngine::prepareBook()
{
    if (book != nullptr) {
        tempBuffer = new MP_Real_t[16384];
        scrubAtoms.clear();
   
        
        for (int i = 0; i < book->numAtoms; ++i)
        {
            
            MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)book->atom[i];
            
            unsigned char windowType = gabor_atom->windowType;
            double windowOption = gabor_atom->windowOption;
            int length = gabor_atom->support[0].len;
            double* window = new double[length];
            
            make_window(window, length, windowType, windowOption);
            
            
            
            float atomPhaseInc = 2 * M_PI * gabor_atom->freq;
            float atomInitialPhase = gabor_atom->phase[0];
            
            if (atomInitialPhase < 0) {
                atomInitialPhase += 2 * M_PI;
            }
            
            ScrubAtom* newAtom = new ScrubAtom;
            newAtom->atom = gabor_atom;
            newAtom->currentPhase = atomInitialPhase;
            newAtom->phaseInc = atomPhaseInc;
            newAtom->window = window;
            newAtom->originalSupport = *gabor_atom->support;
            
            scrubAtoms.add(newAtom);
        }
        
        
    }
    
    
}
