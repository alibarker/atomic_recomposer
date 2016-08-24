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
    
    int64 getTotalLength() const override;
    
    bool isLooping() const override {return isCurrentlyLooping;}
    virtual void setLooping (bool shouldLoop) override { isCurrentlyLooping = shouldLoop; }
    
    bool isScrubbing() const { return isCurrentlyScrubbing; }
    void setScrubbing (bool shouldScrub) { isCurrentlyScrubbing = shouldScrub; }

    void setReversing(bool shouldReverse) { isCurrentlyReversing = shouldReverse; }
    
    // audioSource functions
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override;

    virtual void releaseResources() override {}
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    // parameter functions
    void setBleed(float value) { currentBleedValue = value; }
    void setPlaybacklimit(int value) { playbackLimit = value; }
    void setJumpAmount(int value) { jumpAmount = value; }
    void setSpeed(float value) { currentSpeed = value; }
    void setPitchShift(float value) { currentPitchShift = value; }

    int targetPosition;
    int expBufferSize;

    ScopedPointer<WavetableSinOscillator> osc;
    
    // current playback info
    int currentlyPlaying;
    int currentlyTooQuiet;
    int currentlyNotSupported;

private:
    
    double getScaledWindowValue(int atomLength, int pos);
    void smoothScrubbing();
    
    ScopedPointer<MP_Real_t> tempBuffer;
    double getWindowValues(int atomLength, int startSample);
    
    AtomicAudioEngine* engine;
    
    int64 prevReadPosition;
    int64 nextReadPosition;
    
    // transport status
    bool isCurrentlyScrubbing;
    bool isCurrentlyRunning;
    bool isCurrentlyLooping;
    bool isCurrentlyReversing;
    
    // parameter status
    float currentBleedValue;
    int playbackLimit;
    int jumpAmount; // used for limiting scrubbing speed
    float currentSpeed;
    float currentPitchShift;
    int currentVocoderValue;

    float currentSampleRate;
    
};



#endif  // ATOMICAUDIOSOURCE_H_INCLUDED

