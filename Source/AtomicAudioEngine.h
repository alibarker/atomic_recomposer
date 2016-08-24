/*
  ==============================================================================

    AtomicAudioEngine.h
    Created: 8 Jul 2016 12:29:40pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef ATOMICAUDIOENGINE_H_INCLUDED
#define ATOMICAUDIOENGINE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "mptk.h"
#include "RealtimeBook.h"

class AtomicAudioSource;

enum
{
    pBleedAmount = 0,
    pAtomLimit,
    pMaxScrubSpeed,
    pWindowShape,
    pVocoder,
    pSpeed
};

class AtomicAudioEngine :   public AudioProcessor,
                            public ChangeBroadcaster,
                            public Thread,
                            public ActionBroadcaster
{
public:
  
    AtomicAudioEngine();
    
    ~AtomicAudioEngine();

    // transport interface functions
    void startPlaying() { transportSource.start(); }
    void stopPlaying() { transportSource.stop(); }
    bool isPlaying() {return transportSource.isPlaying(); }
    
    void setLooping(bool isLooping, int startSample, int endSample);
    void setReverse(bool shouldReverse);
    void setScrubbing(bool status);
    bool isCurrentlyScrubbing();

    void setTransportPosition(float posAsPercentage, bool isCurrentlyDragging);
    float getTransportPosition();
    
    // audio interface functions
    void prepareToPlay ( double sampleRate, int samplesPerBlockExpected) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        expBufferSize = samplesPerBlockExpected;
        fs = sampleRate;
        lastBufferTime = Time::getCurrentTime();
    }
    
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    virtual void releaseResources() override {}
    
    // decomposition functions
    
    void run() override;
    void triggerDecomposition(File dictionary, File signal, int numIterations);
    bool isDecomposing() {return currentlyDecomposing;}
    void prepareBook();
    void decomposition();

    // misc
    double getWindowValue(int atomLength, int sampleInAtom);

    void setStatus(String val);
    String getStatus() { ScopedReadLock srl(statusLock); return status; }

    //==============================================================================
    // audioProcessor functions
    
    const String getName() const override {return "Atomic Audio Decomposer"; }
    
    double getTailLengthSeconds() const override { return 0.0; }
    
    bool acceptsMidi() const override { return false; }
    
    bool producesMidi() const override { return false; }
    
    AudioProcessorEditor* createEditor() override { return nullptr; }
    
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override {return 0; }
    void setCurrentProgram (int index) override {}
    const String getProgramName (int index) override { return "";}
    void changeProgramName (int index, const String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override {}
    void setStateInformation (const void* data, int sizeInBytes) override {}

    //==============================================================================
    
    // decomposition information
    
    File dictionary;
    File signal;
    int numIterations;
    
    RealtimeBook rtBook;
    ReadWriteLock bookLock;

    // Parameters
    
    AudioParameterFloat* paramBleed;
    AudioParameterInt* atomLimit;
    AudioParameterFloat* maxScrubSpeed;
    AudioParameterInt* windowShape;
    AudioParameterInt* vocoderEffect;
    AudioParameterFloat* paramSpeed;
    AudioParameterFloat* paramPitchShift;
    
    // Misc Objects
    
    AudioTransportSource transportSource;
    ScopedPointer<AtomicAudioSource> atomicSource;

private:
    void initialiseParameters();

    void quantizeAtomFrequencies(int midiNumber);

    // array of buffers containing the window shapes
    OwnedArray<AudioBuffer<double>> windowBuffers;
    int windowLength;
    void makeOtherWindows(int windowLength);

    // pointer to current window
    AudioBuffer<double>* currentWindow;

    // used for generating atom playback info in debug mode
    ScopedPointer<FileLogger> logger;
    
    // variables used for calculating underruns
    uint64 sampleCount;
    float fs;
    Time lastBufferTime;
    int expBufferSize;
    
    // misc
    int prevVocoderValue;
    
    bool isScrubbing = false;
    
    bool currentlyDecomposing;
    
    String status;
    ReadWriteLock statusLock;
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
