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
#include "Parameters.h"

class AtomicAudioSource;

enum
{
    pBleedAmount = 0,
    pAtomLimit,
    pMaxScrubSpeed,
    pWindowShape
};

class AtomicAudioEngine :   public AudioProcessor,
                            public ChangeBroadcaster,
                            public Thread,
                            public ActionBroadcaster
{
public:
  
    AtomicAudioEngine(ChangeListener* cl);
    
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
    
    float getParameter(int index) { return *parameters.getUnchecked(index); }
    void setParameter(int index, float value) { *parameters[index] = value; }
    
    FloatParameter* paramBleed;
    FloatParameter* atomLimit;
    FloatParameter* maxScrubSpeed;
    FloatParameter* windowShape;
    
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
    
private:
    
    void makeOtherWindows(int windowLength);
    OwnedArray<AudioBuffer<double>> windowBuffers;
    AudioBuffer<double>* currentWindow;
    int windowLength;

    
    OwnedArray<FloatParameter> parameters;
    void initialiseParameters(ChangeListener* cl);

    void decomposition();
    
    int scrubSmoothAmount;
    
    bool isPlayingLeftRight;

    bool isScrubbing = false;
    File dictionary;
    File signal;
    int numIterations;
    
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
