/*
  ==============================================================================

    ParametersWindow.cpp
    Created: 25 Jul 2016 6:27:03pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "ParametersWindow.h"


ParametersWindow::ParametersWindow(AtomicAudioEngine* aae) {
    int sliderWidth = 300;
    int sliderHeight = 30;
    int textBoxSize = 50;
    /* Bleed */
    paramBleed = dynamic_cast<FloatParameter*>(aae->getParameter(pBleedAmount));

    sliderBleed = new Slider("Bleed Amount");
    sliderBleed->setSliderStyle(Slider::LinearHorizontal);
    sliderBleed->setRange(paramBleed->range.start, paramBleed->range.end);
    sliderBleed->setValue(*paramBleed);
    sliderBleed->addListener(this);
    sliderBleed->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    
    addAndMakeVisible(sliderBleed);
    sliderBleed->setBounds(1, 1, sliderWidth, sliderHeight);
    
    /* Atom Limit */
    paramAtomLimit = dynamic_cast<IntParameter*>(aae->getParameter(pAtomLimit));

    
    sliderAtomsLimit = new Slider("Atoms limit");
    sliderAtomsLimit->setSliderStyle(Slider::LinearHorizontal);
    sliderAtomsLimit->setRange(paramAtomLimit->range.getStart(), paramAtomLimit->range.getEnd(), 1);
    sliderAtomsLimit->setValue(*paramAtomLimit);
    sliderAtomsLimit->addListener(this);
    sliderAtomsLimit->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);

    addAndMakeVisible(sliderAtomsLimit);
    sliderAtomsLimit->setBounds(1, 1 * (1 + sliderHeight), sliderWidth, sliderHeight);

    /* Window Shape */
    
    paramWindowShape = dynamic_cast<IntParameter*>(aae->getParameter(pWindowShape));
    
    sliderWindowShape = new Slider("Window Shape");
    sliderWindowShape->setSliderStyle(Slider::LinearHorizontal);
    sliderWindowShape->setRange(paramWindowShape->range.getStart(), paramWindowShape->range.getEnd(), 1);
    sliderWindowShape->setValue(*paramWindowShape);
    sliderWindowShape->addListener(this);
    addAndMakeVisible(sliderWindowShape);
    sliderWindowShape->setBounds(1, 2 * (1 + sliderHeight), sliderWidth, sliderHeight);
    sliderWindowShape->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);

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
    else if (slider == sliderWindowShape)
    {
        *paramWindowShape = sliderWindowShape->getValue();
    }
}