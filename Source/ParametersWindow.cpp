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
    sliderBleed->setRange(paramBleed->range.start, paramBleed->range.end);
    sliderBleed->setSkewFactorFromMidPoint(1.0);
    sliderBleed->setValue(*paramBleed);
    sliderBleed->addListener(this);

    sliderBleed->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderBleed->setSliderStyle(Slider::LinearHorizontal);
    sliderBleed->setBounds(1, 1, sliderWidth, sliderHeight);
    addAndMakeVisible(sliderBleed);

    labelBleed = new Label("Bleed", "Bleed");
    addAndMakeVisible(labelBleed);
    labelBleed->setBounds(sliderWidth + 2, 1, labelWidth, sliderHeight);
   
    
    /* Atom Limit */
    
    paramAtomLimit= aae->atomLimit;
    
    sliderAtomsLimit = new Slider("Atoms limit");
    sliderAtomsLimit->setRange(paramAtomLimit->getRange().getStart() , paramAtomLimit->getRange().getEnd(), 1);
    sliderAtomsLimit->setSkewFactorFromMidPoint(*paramAtomLimit);
    sliderAtomsLimit->setValue(*paramAtomLimit);
    sliderAtomsLimit->addListener(this);
    
    sliderAtomsLimit->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderAtomsLimit->setSliderStyle(Slider::LinearHorizontal);
    sliderAtomsLimit->setBounds(1, 1 * (1 + sliderHeight), sliderWidth, sliderHeight);
    addAndMakeVisible(sliderAtomsLimit);

    labelAtomsLimit = new Label("Atom Limit", "Atom limit");
    labelAtomsLimit->setBounds(sliderWidth + 2, 1 * (1 + sliderHeight), labelWidth, sliderHeight);
    addAndMakeVisible(labelAtomsLimit);
    
 
    /* Window Shape */
    
    paramWindowShape = aae->windowShape;
    
    sliderWindowShape = new Slider("Window Shape");
    sliderWindowShape->setRange(paramWindowShape->getRange().getStart(), paramWindowShape->getRange().getEnd(), 1);
    sliderWindowShape->setValue(*paramWindowShape);
    sliderWindowShape->addListener(this);
    
    sliderWindowShape->setBounds(1, 2 * (1 + sliderHeight), sliderWidth, sliderHeight);
    sliderWindowShape->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderWindowShape->setSliderStyle(Slider::LinearHorizontal);
    addAndMakeVisible(sliderWindowShape);

    labelWindowShape = new Label("Window Shape", "Window Shape");
    addAndMakeVisible(labelWindowShape);
    labelWindowShape->setBounds(sliderWidth + 2, 2 * (1 + sliderHeight), labelWidth, sliderHeight);

    
    /* Vocoder */
    
    paramVocoderEffect = aae->vocoderEffect;
    
    sliderVocoderValue = new Slider("Vocoder Effect");
    sliderVocoderValue->setRange(paramVocoderEffect->getRange().getStart(),
                                 paramVocoderEffect->getRange().getEnd(), 1);
    sliderVocoderValue->setValue(floor(aae->getParameter(pVocoder)));
    sliderVocoderValue->addListener(this);

    sliderVocoderValue->setBounds(1, 3 * (1 + sliderHeight), sliderWidth, sliderHeight);
    sliderVocoderValue->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderVocoderValue->setSliderStyle(Slider::LinearHorizontal);
    addAndMakeVisible(sliderVocoderValue);

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