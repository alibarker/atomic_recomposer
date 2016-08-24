/*
  ==============================================================================

    WavetableSinOscillator.h
    Created: 6 Jul 2016 2:04:55pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef WAVETABLESINOSCILLATOR_H_INCLUDED
#define WAVETABLESINOSCILLATOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

// class that efficiently retuns

static const double two_pi = 2 * M_PI;
static const double inv_two_pi = 1 / (2 * M_PI);

class WavetableSinOscillator
{
public:
    // default constructor which creates a wavetable size of 4096
    WavetableSinOscillator() : WavetableSinOscillator(4096) {}
    
    // creates the wavetable
    WavetableSinOscillator(const int wavetableSize)
    {
        size = wavetableSize;
        sizeOver2Pi = size * inv_two_pi;
        wavetable.setSize(1, wavetableSize);

        for (int i = 0; i < wavetableSize; ++i)
        {
            float phase = two_pi * i / (float) wavetableSize;
            wavetable.setSample(0, i, sin(phase));
        }
        
        interpolation = noInterpolation;
        
    }
    
    ~WavetableSinOscillator() {}
    
    float getSample(float phase)
    {
        float output;

        // calculate pos in wavetable
        float samplePos = phase * sizeOver2Pi;
        
        int intPhase = floor(samplePos);
        
        switch (interpolation)
        {
            case noInterpolation:
                output = *wavetable.getReadPointer(0, intPhase);
                break;
                
            case linearInterpolation:
                int intPhasePlusOne = intPhase + 1;
                
                if (intPhasePlusOne == size)
                {
                    intPhasePlusOne = 0;
                }
                
                float fractPhase = samplePos - intPhase;
                output = (1 - fractPhase) * wavetable.getSample(0, intPhase)
                + fractPhase * wavetable.getSample(0, intPhasePlusOne);
                break;
                
        }
        
        return output;
        
    }
    
private:
    AudioSampleBuffer wavetable;
    int size;
    float phaseInc;
    
    float sizeOver2Pi;
    
    enum InterpolationType
    {
        noInterpolation = 0,
        linearInterpolation
    };
    
    InterpolationType interpolation;
};



#endif  // WAVETABLESINOSCILLATOR_H_INCLUDED
