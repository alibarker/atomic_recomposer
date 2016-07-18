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

class AtomicAudioSource;

class AtomicAudioEngine :   public AudioAppComponent,
                            public ChangeBroadcaster,
                            public Thread,
                            public Timer
{
public:
  
    AtomicAudioEngine(int wivigramWidth, int wivigramHeight);
    ~AtomicAudioEngine() {stopThread(-1);}
    
    void timerCallback() override
    {
        if (bookLock.tryEnterRead())
        {
            updateWivigram();
            stopTimer();
            bookLock.exitRead();
        }
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
    
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (transportSource.isPlaying())
        {
            ScopedReadLock srl (bookLock);
            transportSource.getNextAudioBlock(bufferToFill);
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
    }
    
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
    
    
    void setStatus(String val);
    String getStatus()
    {
        ScopedReadLock srl(statusLock);
        return status;
    }
    
    void run() override;
    void triggerDecomposition(File dictionary, File signal, int numIterations);

    void prepareBook();
  
    struct ScrubAtom {
        double* currentPhase;
        double phaseInc;
        MP_Atom_c* atom;
        double ratio;
        MP_Support_t originalSupport;
    };
    
    OwnedArray<ScrubAtom> scrubAtoms;
    ScopedPointer<MP_Book_c> book;
    void updateWivigram();
    ReadWriteLock bookLock;

    ScopedPointer<MP_TF_Map_c> map;

    ScopedPointer<AudioBuffer<MP_Real_t>> windowBuffer;
    void updateBleed();

private:
    
    void decomposition();
    
    int targetPosition;
    int scrubSmoothAmount;

    bool isScrubbing = false;
    File dictionary;
    File signal;
    int numIterations;
    
    int wivigramHeight;
    int wivigramWidth;
    
    bool startDecomposition = false;
    
    String status;
    ReadWriteLock statusLock;
    
    float bleedValue;
    
    ScopedPointer<AtomicAudioSource> atomicSource;
    
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
