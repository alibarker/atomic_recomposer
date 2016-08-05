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
                            : Thread("Decomposition"), paramListener(cl), sampleCount(0)
{
    targetPosition = -1;
    isPlayingLeftRight = true;
    currentlyDecomposing = false;
    startThread();
    
    initialiseParameters(cl);
    
}

AtomicAudioEngine::~AtomicAudioEngine() {
    stopThread(-1);
    atomicSource.release();
}


bool AtomicAudioEngine::isCurrentlyScrubbing() { return atomicSource->isScrubbing(); }


void AtomicAudioEngine::setScrubbing(bool isScrubbing)
{
    if (atomicSource != nullptr) {
        atomicSource->setScrubbing(isScrubbing);
        if (isScrubbing) {
            targetPosition = -1;
            transportSource.start();
        }
    }
}

void AtomicAudioEngine::setReverse(bool shouldReverse)
{
    atomicSource->setReversing(shouldReverse);
}

void AtomicAudioEngine::setLooping(bool isLooping, int startSample, int endSample)
{
    atomicSource->setLooping(isLooping);
}



void AtomicAudioEngine::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
 
    RelativeTime timeDifference = Time::getCurrentTime() - lastBufferTime;
    lastBufferTime = Time::getCurrentTime();

    
    if (transportSource.isPlaying())
    {
        // Check for underruns
        if ( (timeDifference.inSeconds() - bufferToFill.numSamples / (float) fs) >  0.004)
        {
            String status = "UNDERRUN";
            DBG(status);
            sendActionMessage(status);
        }
        
        // update parameters
        currentWindow = windowBuffers.getUnchecked( getParameter(pWindowShape) );
        
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
        int jumpAmount = expBufferSize * getParameter(pMaxScrubSpeed);
        if (targetPosition > currentPos)
        {
            nextPos = currentPos + jumpAmount;
            if (nextPos >= targetPosition)
            {
                nextPos = targetPosition;
                targetPosition = -1;
            }
        }
        else if (targetPosition <= currentPos)
        {
            nextPos = currentPos - jumpAmount;
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
    sendChangeMessage();
    
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
        
        
        // Generate approximant for debuggin
        
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
        wait(1000);

        
        if (currentlyDecomposing) {
            decomposition();
            
        }
        
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

double AtomicAudioEngine::getWindowValue(int atomLength, int sampleInAtom, int shapeValue)
{
    
    double output;
    
    if ( isPositiveAndBelow(sampleInAtom, atomLength)) {
        
        float ratio = (float) currentWindow->getNumSamples() / atomLength;
        
        int bufferSamplePos = floor(sampleInAtom * ratio);
        
        output = *currentWindow->getReadPointer(0, bufferSamplePos);
        
    }
    else
    {
        output = 0.0;
    }
    
    
    
    return output;
    
    
}


void AtomicAudioEngine::makeOtherWindows(int windowLength)
{
    
    for (int i = 1; i < 10; ++i) {
        AudioBuffer<double> *gammaWindow = new AudioBuffer<double>;
        gammaWindow->setSize(1, windowLength);
        make_window(gammaWindow->getWritePointer(0), windowLength, DSP_GAMMA_WIN, i + 0.001);
        
        windowBuffers.add(gammaWindow);
    }

    AudioBuffer<double> *gaussWindow = new AudioBuffer<double>;
    gaussWindow->setSize(1, windowLength);
    make_window(gaussWindow->getWritePointer(0), windowLength, DSP_GAUSS_WIN, 0.02);
    windowBuffers.add(gaussWindow);

    

}

void AtomicAudioEngine::prepareBook()
{
    if (rtBook.book != nullptr) {
        
//        windowBuffer = new AudioBuffer<MP_Real_t>(1, windowLength);
//
//        unsigned char windowType = 9;
//        double windowOption = 0;
//
//        make_window(windowBuffer->getWritePointer(0), windowLength, windowType, windowOption);

        
        
        rtBook.realtimeAtoms.clear();
        
        
        windowLength = pow(2, 15);
        makeOtherWindows(windowLength);
        
        
        
        
        
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
//            newAtom->originalSupport = *gabor_atom->support;
//            newAtom->ratio = windowLength / gabor_atom->support->len;
            rtBook.realtimeAtoms.add(newAtom);
        }
        
        
    }
    
    
}


void AtomicAudioEngine::initialiseParameters(ChangeListener* cl)
{
    /* Bleed */
    
    float maxBleedAmount = 8;
    paramBleed = new FloatParameter ( String("Bleed Amount"),
                                                          NormalisableRange<float>(1.0f/maxBleedAmount, maxBleedAmount, 0.01, 1.0),
                                                          1.0 );
    
    parameters.add(paramBleed);
    paramBleed->addChangeListener(cl);
    
    /* Atom limit */
    
    atomLimit = new FloatParameter(String("Atom Playback Limit"), NormalisableRange<float>(10, 500, 1, 1.0), 200) ;
    
    parameters.add(atomLimit);
    atomLimit->addChangeListener(cl);

    
    /* Max Scrubbing Speed */
    
    maxScrubSpeed = new FloatParameter ( String("Max. Scrubbing Speed"),
                                                NormalisableRange<float>(1, 10, 0.01, 1.0),
                                                5.0 );
    
    parameters.add(maxScrubSpeed);
    maxScrubSpeed->addChangeListener(cl);

    
    /* Window Shape */
    
    windowShape = new FloatParameter (String("Window Shape"), NormalisableRange<float>(0, 9, 1, 1.0), 9);
    
    parameters.add(windowShape);
    windowShape->addChangeListener(cl);

}

