/*
  ==============================================================================

    ParametersWindow.cpp
    Created: 25 Jul 2016 6:27:03pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "ParametersWindow.h"


ParametersWindow::ParametersWindow(AtomicAudioEngine* aae) {
    int sliderWidth = 230;
    int sliderHeight = 30;
    int textBoxSize = 50;
    int labelWidth = 100;
    
  
    /* Bleed */

    paramBleed = aae->paramBleed;
    
    sliderBleed = new Slider("Bleed Amount");
    sliderBleed->setSliderStyle(Slider::LinearHorizontal);
    sliderBleed->setRange(aae->paramBleed->range.start, aae->paramBleed->range.end);
    sliderBleed->setSkewFactorFromMidPoint(1.0);
    sliderBleed->setValue(aae->getParameter(pBleedAmount));
    sliderBleed->addListener(this);
    sliderBleed->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    
    addAndMakeVisible(sliderBleed);
    sliderBleed->setBounds(1, 1, sliderWidth, sliderHeight);
    
    labelBleed = new Label("Bleed", "Bleed");
    addAndMakeVisible(labelBleed);
    labelBleed->setBounds(sliderWidth + 2, 1, labelWidth, sliderHeight);
   
    
    /* Atom Limit */
    
    paramAtomLimit= aae->atomLimit;
    
    sliderAtomsLimit = new Slider("Atoms limit");
    sliderAtomsLimit->setSliderStyle(Slider::LinearHorizontal);
    sliderAtomsLimit->setRange(aae->atomLimit->range.start, aae->atomLimit->range.end, 1);
    sliderAtomsLimit->setSkewFactorFromMidPoint(aae->getParameter(pAtomLimit));
    sliderAtomsLimit->setValue(aae->getParameter(pAtomLimit));
    sliderAtomsLimit->addListener(this);
    sliderAtomsLimit->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);

    addAndMakeVisible(sliderAtomsLimit);
    sliderAtomsLimit->setBounds(1, 1 * (1 + sliderHeight), sliderWidth, sliderHeight);

    labelAtomsLimit = new Label("Atom Limit", "Atom limit");
    labelAtomsLimit->setBounds(sliderWidth + 2, 1 * (1 + sliderHeight), labelWidth, sliderHeight);
    addAndMakeVisible(labelAtomsLimit);
    
 
    /* Window Shape */
    
    paramWindowShape = aae->windowShape;
    
    sliderWindowShape = new Slider("Window Shape");
    sliderWindowShape->setSliderStyle(Slider::LinearHorizontal);
    sliderWindowShape->setRange(aae->windowShape->range.start, aae->windowShape->range.end, 1);
    sliderWindowShape->setValue(floor(aae->getParameter(pWindowShape)));
    sliderWindowShape->addListener(this);
    addAndMakeVisible(sliderWindowShape);
    sliderWindowShape->setBounds(1, 2 * (1 + sliderHeight), sliderWidth, sliderHeight);
    sliderWindowShape->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);

    labelWindowShape = new Label("Window Shape", "Window Shape");
    addAndMakeVisible(labelWindowShape);
    labelWindowShape->setBounds(sliderWidth + 2, 2 * (1 + sliderHeight), labelWidth, sliderHeight);

    
    /* Vocoder */
    
    paramVocoderEffect = aae->vocoderEffect;
    
    sliderVocoderValue = new Slider("Vocoder Effect");
    sliderVocoderValue->setSliderStyle(Slider::LinearHorizontal);
    sliderVocoderValue->setRange(paramVocoderEffect->range.start, paramVocoderEffect->range.end, 1);
    sliderVocoderValue->setValue(floor(aae->getParameter(pVocoder)));
    sliderVocoderValue->setBounds(1, 3 * (1 + sliderHeight), sliderWidth, sliderHeight);
    addAndMakeVisible(sliderVocoderValue);
    sliderVocoderValue->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderVocoderValue->addListener(this);
    
    labelVocoderValue = new Label("Vocoder", "Vocoder: 0 = Off");
    labelVocoderValue->setBounds(sliderWidth + 2, 3 * (1 + sliderHeight), labelWidth, sliderHeight);
    addAndMakeVisible(labelVocoderValue);

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
    else if (slider == sliderVocoderValue)
    {
        *paramVocoderEffect = sliderVocoderValue->getValue();
    }
}