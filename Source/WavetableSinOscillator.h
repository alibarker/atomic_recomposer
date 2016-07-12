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


class WavetableSinOscillator
{
public:
    WavetableSinOscillator() : WavetableSinOscillator(4096) {}
    
    WavetableSinOscillator(const int wavetableSize)
    {
        size = wavetableSize;
        
        wavetable.setSize(1, wavetableSize);
        
        for (int i = 0; i < wavetableSize; ++i)
        {
            float phase = 2 * M_PI * i / (float) wavetableSize;
            wavetable.setSample(0, i, sin(phase));
        }
        
        interpolation = linearInterpolation;
        
    }
    
    ~WavetableSinOscillator() {}
    
    float getSample(float phase)
    {
        float samplePos = phase * size / (2 * M_PI);
        
        int intPhase = floor(samplePos);
        float output;
        
        switch (interpolation) {
                
            case noInterpolation:
                output = wavetable.getSample(0, intPhase);
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
    
    enum InterpolationType
    {
        noInterpolation = 0,
        linearInterpolation
    };
    
    InterpolationType interpolation;
};



#endif  // WAVETABLESINOSCILLATOR_H_INCLUDED
