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
    targetPosition = -1;
    
    isPlayingLeftRight = true;
    
    startThread();
    
}

bool AtomicAudioEngine::isCurrentlyScrubbing() { return atomicSource->isLooping(); }


void AtomicAudioEngine::setScrubbing(bool isScrubbing)
{
    if (atomicSource != nullptr) {
        atomicSource->setLooping(isScrubbing);
        if (isScrubbing) {
            targetPosition = -1;
            transportSource.start();
        }
    }
}


void AtomicAudioEngine::updateWivigram()
{
    
    map = new MP_TF_Map_c(wivigramWidth, wivigramHeight, book->numChans, 0, book->numSamples, 0.0, 0.5);
    {
        ScopedReadLock srl(bookLock);
        book->add_to_tfmap(map, MP_TFMAP_PSEUDO_WIGNER, NULL);
    }
    
}

void AtomicAudioEngine::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (transportSource.isPlaying())
    {
        ScopedReadLock srl (bookLock);
        transportSource.getNextAudioBlock(bufferToFill);
        
        smoothScrubbing();
    }
}

void AtomicAudioEngine::smoothScrubbing()
{
    // smooth scrubbing
    if (targetPosition > 0)
    {
        int currentPos = transportSource.getNextReadPosition();
        int nextPos = targetPosition;
        
        if (targetPosition > currentPos)
        {
            nextPos = currentPos + scrubSmoothAmount;
            if (nextPos >= targetPosition)
            {
                nextPos = targetPosition;
                targetPosition = -1;
            }
        }
        else if (targetPosition <= currentPos)
        {
            nextPos = currentPos - scrubSmoothAmount;
            if (nextPos <= targetPosition)
            {
                nextPos = targetPosition;
                targetPosition = -1;
            }
            
        }
        
        transportSource.setNextReadPosition(nextPos);
    }
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
    
    /* Create Book */
    {
        setStatus("Creating Book");
        
        ScopedWriteLock sl(bookLock);
        
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
        
        sig->wavwrite(filename.getCharPointer());
        
        // create new source
        atomicSource.release();
        atomicSource = new AtomicAudioSource(this);
        prepareBook();
        transportSource.setSource(atomicSource);
      
        // mark as complete
        currentlyDecomposing = false;
        
        setStatus("Updating Map");
        
        updateWivigram();
        
        setStatus("");
        
        sendChangeMessage();
        
    }
}

void AtomicAudioEngine::run()
{
    while (!threadShouldExit()) {
        if (currentlyDecomposing) {
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
    
    currentlyDecomposing = true;
    notify();
    
}

void AtomicAudioEngine::setTransportPosition(float posAsPercentage, bool isCurrentlyDragging)
{
    int length = transportSource.getTotalLength();
    if (length != 0) {
        int64 newPos = min ( (int) round(posAsPercentage*length), length -1) ;
        
        if (isCurrentlyDragging)
            targetPosition = newPos;
        else
        {
            transportSource.setNextReadPosition( newPos );
            targetPosition = -1;
        }
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

double AtomicAudioEngine::getWindowValue(int atomNumber, int sampleInAtom)
{
    
    double output;
    
    if ( isPositiveAndBelow(sampleInAtom, (int) book->atom[atomNumber]->support->len)) {
        
        int bufferSamplePos = floor(sampleInAtom * scrubAtoms.getUnchecked(atomNumber)->ratio);
        
        output = windowBuffer->getSample(0, bufferSamplePos);
        
    }
    else
    {
        output = 0.0;
    }
    
    
    
    return output;
    
    
}

void AtomicAudioEngine::updateBleed()
{
    {
        
        for (int i = 0; i < book->numAtoms; ++i)
        {
            MP_Atom_c* atom = book->atom[i];
            
            int originalLength = scrubAtoms[i]->originalSupport.len;
            int originalStart = scrubAtoms[i]->originalSupport.pos;
            
            int newLength = originalLength * getBleedValue();
            int newStart = originalStart + round((originalLength - newLength) / 2.0);
            scrubAtoms[i]->ratio = windowBuffer->getNumSamples() / newLength;

            for (int ch = 0; ch < atom->numChans; ++ch)
            {
                atom->support[ch].len = newLength;
                atom->support[ch].pos = newStart;
            }
        }
    }
    updateWivigram();
}

void AtomicAudioEngine::prepareBook()
{
    if (book != nullptr) {
        
        int windowLength = 16384;
        windowBuffer = new AudioBuffer<MP_Real_t>(1, windowLength);
        scrubAtoms.clear();

        
        unsigned char windowType = 9;
        double windowOption = 0;

        make_window(windowBuffer->getWritePointer(0), windowLength, windowType, windowOption);

        
        for (int i = 0; i < book->numAtoms; ++i)
        {
            
            MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)book->atom[i];
            
            // Prepare additional parameters
            
            ScrubAtom* newAtom = new ScrubAtom;
            int numChannels = gabor_atom->numChans;

            newAtom->currentPhase = new double[numChannels];

            for (int ch = 0; ch < gabor_atom->numChans; ++ch)
            {
                double atomInitialPhase = gabor_atom->phase[ch];
                
                if (atomInitialPhase < 0)
                {
                    atomInitialPhase += 2 * M_PI;
                }
                
                newAtom->currentPhase[ch] = atomInitialPhase;
            }
            
            newAtom->atom = gabor_atom;
            newAtom->phaseInc = 2 * M_PI * gabor_atom->freq;
            newAtom->originalSupport = *gabor_atom->support;
            newAtom->ratio = windowLength / gabor_atom->support->len;
            scrubAtoms.add(newAtom);
        }
        
        
    }
    
    
}
