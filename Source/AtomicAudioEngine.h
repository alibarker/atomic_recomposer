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
    pBleedAmount = 0
};

class AtomicAudioEngine :   public AudioAppComponent,
                            public ChangeBroadcaster,
                            public Thread,
                            public ActionBroadcaster
{
public:
  
    AtomicAudioEngine(ChangeListener* cl);
    
    ~AtomicAudioEngine() {
        stopThread(-1);
        atomicSource.release();
    }
    

    
    void startPlaying() { transportSource.start(); }
    void stopPlaying() { transportSource.stop(); }
    
    bool isPlaying() {return transportSource.isPlaying(); }
    
    void setScrubbing(bool status);
    bool isCurrentlyScrubbing();

    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        scrubSmoothAmount = samplesPerBlockExpected;
    }
    
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
//    float getBleedValue() { return bleedValue; }
//    void setBleedValue(float value) { bleedValue = value;}

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

    ScopedPointer<AudioBuffer<MP_Real_t>> windowBuffer;
    void updateBleed();
    
    // Parameters
    
    Parameter* getParameter(int index) { return parameters[index]; }
    

    
private:
    
    OwnedArray<Parameter> parameters;
    void initialiseParameters();

    void smoothScrubbing();
    void decomposition();
    
    int targetPosition;
    int scrubSmoothAmount;
    
    bool isPlayingLeftRight;

    bool isScrubbing = false;
    File dictionary;
    File signal;
    int numIterations;
    
    bool currentlyDecomposing = false;
    
    String status;
    ReadWriteLock statusLock;
    
    float bleedValue;
    float prevBleedValue;
    
    bool readyForPlayback;
    
    ScopedPointer<AtomicAudioSource> atomicSource;

    ChangeListener* paramListener;
    
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
