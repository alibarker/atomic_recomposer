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

class AtomicAudioSource : public PositionableAudioSource
{
public:
    
    AtomicAudioSource();
    ~AtomicAudioSource() {}
    
    void setNextReadPosition (int64 newPosition) override {nextReadPosition = newPosition;}
    
    int64 getNextReadPosition() const override {return nextReadPosition;}
    
    /** Returns the total length of the stream (in samples). */
    int64 getTotalLength() const override {return book->numSamples;}
    
    /** Returns true if this source is actually playing in a loop. */
    bool isLooping() const override {return false;}
    
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override;

    virtual void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    ScopedPointer<WavetableSinOscillator> osc;

    void setBook(MP_Book_c* newBook)
    {
        book = newBook;
    }
    
    int numAtomsCurrentlyPlaying;

    
private:
    
    MP_Book_c* book;
    
    ScopedPointer<MP_Real_t> tempBuffer;
    
    struct ScrubAtom {
        MP_Atom_c* atom;
        float currentPhase;
        float phaseInc;
    };
    
    OwnedArray<ScrubAtom> scrubAtoms;
    
    int64 prevReadPosition;
    int64 nextReadPosition;
    
    bool isCurrentlyScrubbing;
    
    
};



#endif  // ATOMICAUDIOSOURCE_H_INCLUDED

