/*
  ==============================================================================

    AtomicAudioEngine.cpp
    Created: 8 Jul 2016 12:29:40pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "AtomicAudioEngine.h"
#include "AtomicAudioSource.h"

AtomicAudioEngine::AtomicAudioEngine(ChangeListener* cl)
                            : Thread("Decomposition"), paramListener(cl)
{
    targetPosition = -1;
    isPlayingLeftRight = true;
    
    startThread();
    
    initialiseParameters(cl);
    
}

AtomicAudioEngine::~AtomicAudioEngine() {
    stopThread(-1);
    atomicSource.release();
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



void AtomicAudioEngine::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    
    if (transportSource.isPlaying())
    {
        ScopedReadLock srl (bookLock);
        transportSource.getNextAudioBlock(bufferToFill);
        smoothScrubbing();
        
        String status = "Currently Playing: " + String(atomicSource->currentlyPlaying);
        sendActionMessage(status);
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


void AtomicAudioEngine::setStatus(String status)
{
    sendActionMessage(status);
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
        rtBook.book = MP_Book_c::create(sig->numChans, sig->numSamples, sig->sampleRate );
        if ( rtBook.book == NULL )
        {
            return;
        }
        
        setStatus("Creating mpd Core");
        
        /* Set up the core */
        mpdCore = MP_Mpd_Core_c::create( sig, rtBook.book, dict );
        mpdCore->set_iter_condition( numIterations );
        
        setStatus("Running Matcing Pursuit");

        mpdCore->run();
        
        setStatus("Preparing Book for Playback");
        
        MP_Signal_c* sig = MP_Signal_c::init(rtBook.book->numChans, rtBook.book->numSamples, rtBook.book->sampleRate);
        
        rtBook.book->substract_add(NULL, sig, NULL);
        
        String filename(String(signal.getParentDirectory().getFullPathName() + "/" +  signal.getFileNameWithoutExtension() + "approx" + signal.getFileExtension()));
        
        sig->wavwrite(filename.getCharPointer());
        
        // create new source
        atomicSource.release();
        atomicSource = new AtomicAudioSource(this);
        prepareBook();
        transportSource.setSource(atomicSource);
      
        // mark as complete
        currentlyDecomposing = false;
        
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
    
    stopPlaying();
    setScrubbing(false);

    dictionary = File(dict);
    signal = File(sig);
    numIterations = numIter;
    
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

double AtomicAudioEngine::getWindowValue(int atomLength, int sampleInAtom)
{
    
    double output;
    
    if ( isPositiveAndBelow(sampleInAtom, atomLength)) {
        
        float ratio = (float) windowBuffer->getNumSamples() / atomLength;
        
        int bufferSamplePos = floor(sampleInAtom * ratio);
        
        output = windowBuffer->getSample(0, bufferSamplePos);
        
    }
    else
    {
        output = 0.0;
    }
    
    
    
    return output;
    
    
}


void AtomicAudioEngine::prepareBook()
{
    if (rtBook.book != nullptr) {
        
        int windowLength = 16384;
        windowBuffer = new AudioBuffer<MP_Real_t>(1, windowLength);
        rtBook.realtimeAtoms.clear();
        
        unsigned char windowType = 9;
        double windowOption = 0;

        make_window(windowBuffer->getWritePointer(0), windowLength, windowType, windowOption);

        
        for (int i = 0; i < rtBook.book->numAtoms; ++i)
        {
            
            MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)rtBook.book->atom[i];
            
            // Prepare additional parameters
            
            RealtimeAtom* newAtom = new RealtimeAtom;
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
            rtBook.realtimeAtoms.add(newAtom);
        }
        
        
    }
    
    
}


void AtomicAudioEngine::initialiseParameters(ChangeListener* cl)
{
    
    /* Bleed */
    FloatParameter* bleed = new FloatParameter ( String("Bleed Amount"),
                                                          NormalisableRange<float>(0.25, 4.0, 0.01, 1.0),
                                                          1.0 );
    bleed->addChangeListener(cl);
    parameters.add(bleed);
    
    /* Atom limit */
    
    IntParameter* atomLimit = new IntParameter(String("Atom Playback Limit"), Range<int>(10, 500), 200) ;
    atomLimit->addChangeListener(cl);
    parameters.add(atomLimit);
}

