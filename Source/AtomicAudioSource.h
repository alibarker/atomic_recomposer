/*
  ==============================================================================

    AtomicAudioSource.h
    Created: 4 Jul 2016 5:13:17pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef ATOMICAUDIOSOURCE_H_INCLUDED
#define ATOMICAUDIOSOURCE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "mptk.h"
#include "gabor_atom_plugin.h"
#include "WavetableSinOscillator.h"

class AtomicAudioEngine;

class AtomicAudioSource : public PositionableAudioSource
{
public:
    
    AtomicAudioSource(AtomicAudioEngine* aae);
    ~AtomicAudioSource() {}
    
    void setNextReadPosition (int64 newPosition) override {nextReadPosition = newPosition;}
    
    int64 getNextReadPosition() const override {return nextReadPosition;}
    
    /** Returns the total length of the stream (in samples). */
    int64 getTotalLength() const override;
    
    /** Returns true if this source is actually playing in a loop. */
    bool isLooping() const override {return isCurrentlyLooping;}
    virtual void setLooping (bool shouldLoop) override { isCurrentlyLooping = shouldLoop; }
    
    bool isScrubbing() const { return isCurrentlyScrubbing; }
    void setScrubbing (bool shouldScrub) { isCurrentlyScrubbing = shouldScrub; }

    void setReversing(bool shouldReverse) { isCurrentlyReversing = shouldReverse; }
    
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override;

    virtual void releaseResources() override {}

    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    
    ScopedPointer<WavetableSinOscillator> osc;
    
    int currentlyPlaying;
    int currentlyTooQuiet;
    int currentlyNotSupported;
    
    void updateBleed();
    
private:
    
    double getScaledWindowValue(int atomLength, int pos);
    
    
    ScopedPointer<MP_Real_t> tempBuffer;
    double getWindowValues(int atomLength, int startSample);

    
    AtomicAudioEngine* engine;
    
    int64 prevReadPosition;
    int64 nextReadPosition;
    
    bool isCurrentlyScrubbing;
    bool isCurrentlyRunning;
    bool isCurrentlyLooping;
    bool isCurrentlyReversing;
    
    float currentBleedValue;
    float currentSampleRate;
    
    
};



#endif  // ATOMICAUDIOSOURCE_H_INCLUDED

