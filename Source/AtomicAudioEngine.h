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
#include "AtomicAudioSource.h"

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
    
    void updateWivigram(MP_TF_Map_c* map)
    {
        
        const int width = getWidth();
        const int height = getHeight();
        
        MP_Tfmap_t* column;
        MP_Real_t val;
        
        for (int i = 0; i < width; i++ )
        {
            column = map->channel[0] + i * map->numRows; /* Seek the column */
            
            for (int j = 0; j < height; j++ )
            {
                val = (MP_Real_t) column[j];
                image->setPixelAt(i, height - j, Colour::fromHSV (1.0f, 0.0f, 1 - val, 1.0f));
            }
        }
        
        sendChangeMessage();
        
    }
    
    ScopedPointer<Image> image;
    
    
private:
    Rectangle<int> size;
    
};


class AtomicAudioEngine : public AudioAppComponent, public ChangeBroadcaster, public Thread, public Timer {
public:
  
    AtomicAudioEngine(int wivigramWidth, int wivigramHeight);
    ~AtomicAudioEngine() {}
    
    void timerCallback() override
    {
        if (isReadyToPlay)
        {
            updateWivigram();
            transportSource.start();
            stopTimer();
        }

    }
 
    
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        transportSource.getNextAudioBlock(bufferToFill);
    }
    
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
        return *wivigram->image;
    }
    
    
    void startDecomposition();
    void run() override;
    void triggerDecomposition(File dictionary, File signal, int numIterations);

private:
    
    File dictionary;
    File signal;
    int numIterations;
    
    ScopedPointer<Wivigram> wivigram;
    ScopedPointer<MP_Book_c> book;
    
    void updateWivigram();
    
    bool isReadyToPlay;
    
    
};



#endif  // ATOMICAUDIOENGINE_H_INCLUDED
