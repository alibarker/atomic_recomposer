/*
  ==============================================================================

    ParametersWindow.h
    Created: 25 Jul 2016 6:27:03pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef PARAMETERSWINDOW_H_INCLUDED
#define PARAMETERSWINDOW_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AtomicAudioEngine.h"

class ParametersWindow : public Component, public Slider::Listener
{
public:
    ParametersWindow(AtomicAudioEngine* aae);
    ~ParametersWindow() {}
    
    void sliderValueChanged(Slider *slider) override;

    
private:
    ScopedPointer<Slider> sliderBleed;
    ScopedPointer<Slider> sliderAtomsLimit;
    ScopedPointer<Slider> sliderWindowShape;
    ScopedPointer<Slider> sliderVocoderValue;
    ScopedPointer<Slider> sliderSpeed;

    AudioParameterFloat* paramBleed;
    AudioParameterInt* paramAtomLimit;
    AudioParameterInt* paramWindowShape;
    AudioParameterInt* paramVocoderEffect;
    AudioParameterFloat* paramSpeed;

    ScopedPointer<Label> labelBleed;
    ScopedPointer<Label> labelAtomsLimit;
    ScopedPointer<Label> labelWindowShape;
    ScopedPointer<Label> labelVocoderValue;
    ScopedPointer<Label> labelSpeed;

    
    AtomicAudioEngine* engine;
    
};



#endif  // PARAMETERSWINDOW_H_INCLUDED
