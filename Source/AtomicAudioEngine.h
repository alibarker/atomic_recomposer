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

class Wivigram : public ChangeBroadcaster
{
public:
    Wivigram(int width, int height)
    {
        size.setSize(width, height);
        image = new Image(Image::RGB, width, height, true);
    }
    
    Image getImage();
    
    void updateDimensions(int width, int height)
    {
        size.setSize(width, height);
    }
    
    int getHeight() { return size.getHeight(); }
    int getWidth() { return size.getWidth(); }
    

    ScopedPointer<Image> image;
    
    
private:
    Rectangle<int> size;
    
};

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
    void setScrubbing(bool status);
    

    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (transportSource.isPlaying())
        {
            ScopedReadLock srl (bookLock);
            transportSource.getNextAudioBlock(bufferToFill);
            
        }
    }
    
    float getBleedValue() { return bleedValue; }
    void setBleedValue(float value) { bleedValue = value;}

    virtual void releaseResources() override {}
    AudioTransportSource transportSource;
    
    void setTransportPosition(float posAsPercentage);
    
    
    float getTransportPosition();
    

    
    void resizeWivigram(int width, int height)
    {
        wivigram = new Wivigram(width, height);
        updateWivigram();
    }
    
    Image getWivigramImage()
    {
        ScopedReadLock srl(bookLock);
        return *wivigram->image;
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
        MP_Atom_c* atom;
        float currentPhase;
        float phaseInc;
        double* window;
        MP_Support_t originalSupport;
    };
    
    OwnedArray<ScrubAtom> scrubAtoms;
    ScopedPointer<MP_Book_c> book;
    void updateWivigram();
    ReadWriteLock bookLock;

    ScopedPointer<MP_TF_Map_c> map;

    
private:
    
    void decomposition();
    ScopedPointer<MP_Real_t> tempBuffer;

    bool isScrubbing = false;
    File dictionary;
    File signal;
    int numIterations;
    
    ScopedPointer<Wivigram> wivigram;
    
    
    bool startDecomposition = false;
    
    String status;
    ReadWriteLock statusLock;
    
    float bleedValue;
    
    ScopedPointer<AtomicAudioSource> atomicSource;
    
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
