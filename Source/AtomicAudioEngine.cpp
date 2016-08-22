/*
  ==============================================================================

    AtomicAudioEngine.cpp
    Created: 8 Jul 2016 12:29:40pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "AtomicAudioEngine.h"
#include "AtomicAudioSource.h"

AtomicAudioEngine::AtomicAudioEngine()
                            : Thread("Decomposition"), sampleCount(0)
{
    isPlayingLeftRight = true;
    currentlyDecomposing = false;
    startThread();
    
    
    windowLength = pow(2, 15);
    makeOtherWindows(windowLength);
    
    initialiseParameters();
    
#if JUCE_DEBUG
    
    File logFile("~/Dropbox/QMUL/Final Project/Code/mpdgui/Data/atom_playback.log");
    String header = "Atom Playback Data: " + signal.getFullPathName()
                        + " " + dictionary.getFullPathName()
                        + " " + String(numIterations);
    
    logger = new FileLogger(logFile.getNonexistentSibling(false) , header);
    
    logger->logMessage("LOG START");
#endif
    
}

AtomicAudioEngine::~AtomicAudioEngine()
{
    stopThread(-1);
    transportSource.setSource(nullptr);
}


bool AtomicAudioEngine::isCurrentlyScrubbing() { return atomicSource->isScrubbing(); }


void AtomicAudioEngine::setScrubbing(bool isScrubbing)
{
    if (atomicSource != nullptr) {
        atomicSource->setScrubbing(isScrubbing);
        if (isScrubbing)
        {
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



void AtomicAudioEngine::processBlock(AudioSampleBuffer &buffer, MidiBuffer &midiMessages)
{
    int numSamples = buffer.getNumSamples();
    RelativeTime timeDifference = Time::getCurrentTime() - lastBufferTime;
    lastBufferTime = Time::getCurrentTime();
    
    // update parameters
    
    
    if (transportSource.isPlaying())
    {
        atomicSource->setBleed(*paramBleed);
        atomicSource->setPlaybacklimit(*atomLimit);
        atomicSource->setJumpAmount(*maxScrubSpeed);

        
        // Check for underruns
        if ( (timeDifference.inSeconds() - numSamples / (float) fs) >  0.004)
        {
            String status = "UNDERRUN";
            DBG(status);
            sendActionMessage(status);
        }
        
        // update parameters
        currentWindow = windowBuffers.getUnchecked( *windowShape );
       
        int vocoderValue = *vocoderEffect;
        
        if (prevVocoderValue != vocoderValue)
        {
            quantizeAtomFrequencies(vocoderValue);
            prevVocoderValue = vocoderValue;
        }
        
        // process
        ScopedReadLock srl (bookLock);
        transportSource.getNextAudioBlock(AudioSourceChannelInfo(buffer));
        
        String status = "Currently Playing: " + String(atomicSource->currentlyPlaying);
        sendActionMessage(status);
    
#if JUCE_DEBUG

        String msg = String(atomicSource->getNextReadPosition()) + " "
                        + String(atomicSource->currentlyPlaying) + " "
                        + String(atomicSource->currentlyTooQuiet) + " "
                        + String(atomicSource->currentlyNotSupported);
        
        logger->logMessage(msg);
        
#endif
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
    
    if (length != 0)
    {
        int64 newPos = min ( (int) round(posAsPercentage*length), length -1) ;
        
        if (isCurrentlyDragging)
        {
            atomicSource->targetPosition = newPos;
        }
        else
        {
            transportSource.setNextReadPosition( newPos );
            atomicSource->targetPosition = -1;
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

void AtomicAudioEngine::quantizeAtomFrequencies(int midiNumber)
{
    int numAtoms = rtBook.realtimeAtoms.size();

    if (midiNumber != 0)
    {
        float fundamentalPhaseInc = M_PI * MidiMessage::getMidiNoteInHertz(midiNumber) / fs;
        
        for (int i = 0; i < numAtoms; ++i)
        {
            RealtimeAtom* atom = rtBook.realtimeAtoms.getUnchecked(i);
            atom->phaseInc = ceil( atom->originalPhaseInc / fundamentalPhaseInc ) * fundamentalPhaseInc;
        }
    }
    else
    {
        for (int i = 0; i < numAtoms; ++i)
        {
            RealtimeAtom* atom = rtBook.realtimeAtoms.getUnchecked(i);
            atom->phaseInc = atom->originalPhaseInc;
        }
    }
}


void AtomicAudioEngine::prepareBook()
{
    if (rtBook.book != nullptr) {
        
        
        rtBook.realtimeAtoms.clear();
        
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
            newAtom->phaseInc = newAtom->originalPhaseInc = 2 * M_PI * gabor_atom->freq;
            rtBook.realtimeAtoms.add(newAtom);
        }
        
        
    }
    
    
}


void AtomicAudioEngine::initialiseParameters()
{
    /* Bleed */
    
    float maxBleedAmount = 8;
    paramBleed = new AudioParameterFloat(String("bleed"),
                                         String("Bleed Amount"),
                                         NormalisableRange<float>(1.0f/maxBleedAmount, maxBleedAmount, 0.01, 1.0),
                                         1.0 );
    
    addParameter(paramBleed);
    
    /* Atom limit */
    
    atomLimit = new AudioParameterInt(String("limit" ),
                                        String("Atom Playback Limit"),
                                        10,
                                        5000,
                                        200) ;
    
    addParameter(atomLimit);
    
    /* Max Scrubbing Speed */
    
    maxScrubSpeed = new AudioParameterFloat (String("speed"),
                                             String("Max. Scrubbing Speed"),
                                             NormalisableRange<float>(1, 10, 0.01, 1.0),
                                             5.0 );
    
    addParameter(maxScrubSpeed);
    
    /* Window Shape */
    
    windowShape = new AudioParameterInt (String("window"),
                                         String("Window Shape"),
                                         0,
                                         9,
                                         9);
    
    addParameter(windowShape);

    
    /* Vocoder */
    
    vocoderEffect = new AudioParameterInt (String("vocoder"),
                                        String("Vocoder Effect"),
                                        0,
                                        128,
                                        0);
    addParameter(vocoderEffect);
    
    
}

