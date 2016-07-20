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

class AtomicAudioEngine :   public AudioAppComponent,
                            public ChangeBroadcaster,
                            public Thread
{
public:
  
    AtomicAudioEngine(int wivigramWidth, int wivigramHeight);
    ~AtomicAudioEngine() {
        stopThread(-1);
        atomicSource.release();
        bleedValue = prevBleedValue = 1.0;
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
    
    float getBleedValue() { return bleedValue; }
    void setBleedValue(float value) { bleedValue = value;}

    virtual void releaseResources() override {}
    AudioTransportSource transportSource;
    
    void setTransportPosition(float posAsPercentage, bool isCurrentlyDragging);
    float getTransportPosition();
    
    double getWindowValue(int atomLength, int sampleInAtom);

    void resizeWivigram(int width, int height)
    {
        wivigramWidth = width;
        wivigramHeight = height;
        updateWivigram();
    }
    
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
    
    void updateWivigram();
    ReadWriteLock bookLock;

    ScopedPointer<MP_TF_Map_c> map;

    ScopedPointer<AudioBuffer<MP_Real_t>> windowBuffer;
    void updateBleed();

private:
    void smoothScrubbing();
    void decomposition();
    
    int targetPosition;
    int scrubSmoothAmount;
    
    bool isPlayingLeftRight;

    bool isScrubbing = false;
    File dictionary;
    File signal;
    int numIterations;
    
    int wivigramHeight;
    int wivigramWidth;
    
    bool currentlyDecomposing = false;
    
    String status;
    ReadWriteLock statusLock;
    
    float bleedValue;
    float prevBleedValue;
    
    bool readyForPlayback;
    
    ScopedPointer<AtomicAudioSource> atomicSource;
    

    
    
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
