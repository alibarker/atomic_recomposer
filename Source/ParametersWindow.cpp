/*
  ==============================================================================

    ParametersWindow.cpp
    Created: 25 Jul 2016 6:27:03pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "ParametersWindow.h"


ParametersWindow::ParametersWindow(AtomicAudioEngine* aae) {
    int sliderWidth = 150;
    int sliderHeight = 30;
    
    /* Bleed */
    paramBleed = dynamic_cast<FloatParameter*>(aae->getParameter(pBleedAmount));

    sliderBleed = new Slider("Bleed Amount");
    sliderBleed->setSliderStyle(Slider::LinearHorizontal);
    sliderBleed->setRange(paramBleed->range.start, paramBleed->range.end);
    sliderBleed->setValue(*paramBleed);
    sliderBleed->addListener(this);
    
    addAndMakeVisible(sliderBleed);
    sliderBleed->setBounds(1, 1, sliderWidth, sliderHeight);
    
    /* Atom Limit */
    paramAtomLimit = dynamic_cast<IntParameter*>(aae->getParameter(pAtomLimit));

    
    sliderAtomsLimit = new Slider("Atoms limit");
    sliderAtomsLimit->setSliderStyle(Slider::LinearHorizontal);
    sliderAtomsLimit->setRange(paramAtomLimit->range.getStart(), paramAtomLimit->range.getEnd(), 1);
    sliderAtomsLimit->setValue(*paramAtomLimit);
    sliderAtomsLimit->addListener(this);
    
    addAndMakeVisible(sliderAtomsLimit);
    sliderAtomsLimit->setBounds(1, 2 + sliderHeight, sliderWidth, sliderHeight);

    
}

void ParametersWindow::sliderValueChanged(Slider *slider)
{
    if (slider == sliderBleed)
    {
        *paramBleed = sliderBleed->getValue();
    }
    else if (slider == sliderAtomsLimit)
    {
        *paramAtomLimit = sliderAtomsLimit->getValue();
    }
}