/*
  ==============================================================================

    ParametersWindow.cpp
    Created: 25 Jul 2016 6:27:03pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "ParametersWindow.h"


ParametersWindow::ParametersWindow(AtomicAudioEngine* aae)
{
    setBounds(0, 0, 300, 500); // set size of window
    
    // set size of components
    int sliderWidth = 220;
    int sliderHeight = 30;
    int textBoxSize = 50;
    int labelWidth = 80;
    
  
    /* Bleed */

    paramBleed = aae->paramBleed; // point local pointer to glocal parameter
    
    sliderBleed = new Slider("Bleed Amount"); // create component
    sliderBleed->setRange(paramBleed->range.start, paramBleed->range.end); // set range to that of the parameter
    sliderBleed->setSkewFactorFromMidPoint(1.0);
    sliderBleed->setValue(*paramBleed); // set value to to that of parameter
    sliderBleed->addListener(this); // set slider listener

    // set appearance and make visible
    sliderBleed->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderBleed->setSliderStyle(Slider::LinearHorizontal);
    sliderBleed->setBounds(1, 1, sliderWidth, sliderHeight);
    addAndMakeVisible(sliderBleed);

    // create label
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
    sliderVocoderValue->setValue(*paramVocoderEffect);
    sliderVocoderValue->addListener(this);

    sliderVocoderValue->setBounds(1, 3 * (1 + sliderHeight), sliderWidth, sliderHeight);
    sliderVocoderValue->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderVocoderValue->setSliderStyle(Slider::LinearHorizontal);
    addAndMakeVisible(sliderVocoderValue);

    labelVocoderValue = new Label("Vocoder", "Vocoder: 0 = Off");
    labelVocoderValue->setBounds(sliderWidth + 2, 3 * (1 + sliderHeight), labelWidth, sliderHeight);
    addAndMakeVisible(labelVocoderValue);
    
    /* Playback Speed */
    
    paramSpeed = aae->paramSpeed;
    
    sliderSpeed = new Slider("Playback Speed");
    sliderSpeed->setRange(paramSpeed->range.start, paramSpeed->range.end, 0);
    sliderSpeed->setValue(*paramSpeed);
    sliderSpeed->setSkewFactorFromMidPoint(1.0);
    sliderSpeed->addListener(this);
    
    sliderSpeed->setBounds(1, 4 * (1 + sliderHeight), sliderWidth, sliderHeight);
    sliderSpeed->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderSpeed->setSliderStyle(Slider::LinearHorizontal);
    addAndMakeVisible(sliderSpeed);
    
    labelSpeed = new Label("Playback Speed", "Playback Speed");
    labelSpeed->setBounds(sliderWidth + 2, 4 * (1 + sliderHeight), labelWidth, sliderHeight);
    addAndMakeVisible(labelSpeed);

    /* Pitch Shift */
    
    paramPitch = aae->paramPitchShift;
    
    sliderPitch = new Slider("Playback Speed");
    sliderPitch->setRange(paramPitch->range.start, paramPitch->range.end, 0);
    sliderPitch->setValue(*paramPitch);
    sliderPitch->setSkewFactorFromMidPoint(1.0);
    sliderPitch->addListener(this);
    
    sliderPitch->setBounds(1, 5 * (1 + sliderHeight), sliderWidth, sliderHeight);
    sliderPitch->setTextBoxStyle(Slider::TextBoxRight, false, textBoxSize, sliderHeight);
    sliderPitch->setSliderStyle(Slider::LinearHorizontal);
    addAndMakeVisible(sliderPitch);
    
    labelPitch = new Label("pitch", "Pitch Shift");
    labelPitch->setBounds(sliderWidth + 2, 5 * (1 + sliderHeight), labelWidth, sliderHeight);
    addAndMakeVisible(labelPitch);
    
}

void ParametersWindow::sliderValueChanged(Slider *slider)
{
    // set each parameter when a slider is changed
    
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
    else if (slider == sliderSpeed)
    {
        *paramSpeed = sliderSpeed->getValue();
    }
    else if (slider == sliderPitch)
    {
        *paramPitch = sliderPitch->getValue();
    }
}