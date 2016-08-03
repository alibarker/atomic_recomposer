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

class AtomicAudioEngine :   public AudioAppComponent,
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
        
    void setScrubbing(bool status);
    bool isCurrentlyScrubbing();

    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        expBufferSize = samplesPerBlockExpected;
    }
    
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    

    virtual void releaseResources() override {}
    AudioTransportSource transportSource;
    
    void setTransportPosition(float posAsPercentage, bool isCurrentlyDragging);
    float getTransportPosition();
    
    double getWindowValue(int atomLength, int sampleInAtom, int shapeValue);

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
    
    Parameter* getParameter(int index) { return parameters[index]; }
    

    
private:
    
    void makeOtherWindows(int windowLength);
    OwnedArray<AudioBuffer<double>> windowBuffers;
    int windowLength;

    
    OwnedArray<Parameter> parameters;
    void initialiseParameters(ChangeListener* cl);

    void smoothScrubbing();
    void decomposition();
    
    int targetPosition;
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
    
    ScopedPointer<AtomicAudioSource> atomicSource;

    ChangeListener* paramListener;
    
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
