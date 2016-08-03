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
#include "Parameters.h"
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

    FloatParameter* paramBleed;
    IntParameter* paramAtomLimit;
    IntParameter* paramWindowShape;
    
    AtomicAudioEngine* engine;
    
};



#endif  // PARAMETERSWINDOW_H_INCLUDED
