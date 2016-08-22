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

    void startPlaying() { transportSource.start(); }
    void stopPlaying() { transportSource.stop(); }
    
    bool isPlaying() {return transportSource.isPlaying(); }
    
    void setLooping(bool isLooping, int startSample, int endSample);
    void setReverse(bool shouldReverse);
    
    void setScrubbing(bool status);
    bool isCurrentlyScrubbing();

    void prepareToPlay ( double sampleRate, int samplesPerBlockExpected) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        expBufferSize = samplesPerBlockExpected;
        fs = sampleRate;
        lastBufferTime = Time::getCurrentTime();
    }
    
    
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;
    

    virtual void releaseResources() override {}
    AudioTransportSource transportSource;
    
    void setTransportPosition(float posAsPercentage, bool isCurrentlyDragging);
    float getTransportPosition();
    
    double getWindowValue(int atomLength, int sampleInAtom);

    bool getIsPlayingLeftToRight() {return isPlayingLeftRight;}
    
    void setStatus(String val);
    String getStatus()
    {
        ScopedReadLock srl(statusLock);
        return status;
    }
    
    void run() override;
    void triggerDecomposition(File dictionary, File signal, int numIterations);
    bool isDecomposing() {return currentlyDecomposing;}

    void prepareBook();
  
    RealtimeBook rtBook;
    
    ReadWriteLock bookLock;

    ScopedPointer<MP_TF_Map_c> map;

    void updateBleed();
    
    // Parameters
    
    AudioParameterFloat* paramBleed;
    AudioParameterInt* atomLimit;
    AudioParameterFloat* maxScrubSpeed;
    AudioParameterInt* windowShape;
    AudioParameterInt* vocoderEffect;
    AudioParameterFloat* paramSpeed;
    AudioParameterFloat* paramPitchShift;

    //==============================================================================
    
    
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
    
    File dictionary;
    File signal;
    int numIterations;

    
private:
    
    ScopedPointer<FileLogger> logger;

    
    void makeOtherWindows(int windowLength);
    OwnedArray<AudioBuffer<double>> windowBuffers;
    AudioBuffer<double>* currentWindow;
    int windowLength;

    void quantizeAtomFrequencies(int midiNumber);
    
    void initialiseParameters();

    void decomposition();
    
    int scrubSmoothAmount;
    int prevVocoderValue;
    bool isPlayingLeftRight;

    bool isScrubbing = false;
    
    bool currentlyDecomposing;
    
    String status;
    ReadWriteLock statusLock;
    
    float bleedValue;
    float prevBleedValue;
    int expBufferSize;
    bool readyForPlayback;
    
    uint64 sampleCount;
    float fs;
    Time lastBufferTime;
    
    ScopedPointer<AtomicAudioSource> atomicSource;

    ChangeListener* paramListener;
    
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
