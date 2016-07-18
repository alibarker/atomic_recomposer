/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/


#include "MainComponent.h"


//==============================================================================
MainContentComponent::MainContentComponent()
{
    setSize (1000, 600);
    
    int wiviWidth = getWidth() - 1;
    int wiviHeight = getHeight() - 61;

    File defaultDict("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/dictGabor_original.xml");
    File defaultSignal("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/glock2.wav");

    
    audioEngine = new AtomicAudioEngine(wiviWidth, wiviHeight);
    audioEngine->setAudioChannels (0, 2);
    audioEngine->addChangeListener(this);
    
    textEditorDictionary = new TextEditor("Dictionary");
    textEditorDictionary->setBounds(649, 1, 300, 25);
    addAndMakeVisible(textEditorDictionary);
    textEditorDictionary->setText(defaultDict.getFullPathName());
    
    buttonSelectDictionary = new TextButton("SelectDict");
    addAndMakeVisible(buttonSelectDictionary);
    buttonSelectDictionary->setBounds(949, 1, 50, 25);
    buttonSelectDictionary->setButtonText("...");
    buttonSelectDictionary->addListener(this);

    labelSelectDictionary = new Label("Dictionary Label", "Dictionary:");
    addAndMakeVisible(labelSelectDictionary);
    labelSelectDictionary->setBounds(574, 1, 75, 25);
    
    textEditorSignal = new TextEditor("Signal");
    textEditorSignal->setBounds(649, 27, 300, 25);
    addAndMakeVisible(textEditorSignal);
    textEditorSignal->setText(defaultSignal.getFullPathName());

    buttonSelectSignal = new TextButton("SelectSignal");
    addAndMakeVisible(buttonSelectSignal);
    buttonSelectSignal->setBounds(949, 26, 50, 25);
    buttonSelectSignal->setButtonText("...");
    buttonSelectSignal->addListener(this);

    labelSelectSignal = new Label("Signal Label", "Signal:");
    addAndMakeVisible(labelSelectSignal);
    labelSelectSignal->setBounds(600, 26, 49, 25);
    
    sliderBleed = new Slider("Bleed Amount");
    sliderBleed->setBounds(160, 35, 340, 20);
    addAndMakeVisible(sliderBleed);
    sliderBleed->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    sliderBleed->setTextBoxStyle(Slider::TextBoxRight, false, 50, 20);
    sliderBleed->addListener(this);
    sliderBleed->setRange(0.25, 4);
    sliderBleed->setValue(audioEngine->getBleedValue());

    button_decomp = new TextButton("Decompose");
    button_decomp->setBounds (1, 1, 144, 32);
    addAndMakeVisible (button_decomp);
    button_decomp->addListener (this);
    
    label_num_iterations = new Label();
    label_num_iterations->setBounds(1, 35, 80, 20);
    label_num_iterations->setText("Iterations",dontSendNotification);
    addAndMakeVisible(label_num_iterations);
    
    text_editor_num_iterations = new TextEditor();
    text_editor_num_iterations->setText("10");
    text_editor_num_iterations->setMultiLine(false);
    text_editor_num_iterations->setBounds(70, 35, 50, 20);
    addAndMakeVisible (text_editor_num_iterations);
    
    wivigram = new ImageComponent("Wivigram");
    addAndMakeVisible(wivigram);
    wivigram->setBounds(1, 60, wiviWidth, wiviHeight);
    
    // TODO: Does this line need to wivigram->...
    addMouseListener(this, true);

    cursor = new Cursor();
    addAndMakeVisible(cursor);
    cursor->setBounds(wivigram->getBounds());
                                 
//    numAtoms = new Label();
//    numAtoms->setBounds(160, 1, 800, 20);
//    addAndMakeVisible(numAtoms);
//    
    statusLabel = new Label();
    statusLabel->setBounds(160, 1, 340, 20);
    addAndMakeVisible(statusLabel);
    
    /* Start/Stop Buttons */
    
    buttonStart = new TextButton("Start");
    buttonStart->setBounds(160, 1, 150, 20);
    addAndMakeVisible(buttonStart);
    buttonStart->addListener(this);

  
    buttonScrub = new TextButton("Scrub");
    buttonScrub->setBounds(312, 1, 150, 20);
    addAndMakeVisible(buttonScrub);
    buttonScrub->addListener(this);
    buttonScrub->setClickingTogglesState(true);
    
    startTimerHz(60);
    

}

MainContentComponent::~MainContentComponent()
{
    button_decomp = 0;
    label_num_iterations = 0;
    text_editor_num_iterations = 0;
    audioEngine->shutdownAudio();
    
}

void MainContentComponent::mouseDrag(const MouseEvent &event)
{
    if (event.originalComponent == cursor)
    {
 
            float scrubPos = (float) event.x / (float) getWidth();
            audioEngine->setTransportPosition(scrubPos, !event.mouseWasClicked());
    }
}

void MainContentComponent::mouseDown(const MouseEvent &event)
{
    
    
//    if (event.originalComponent == cursor && event.mouseWasClicked()) {
//        float scrubPos = (float) event.x / (float) getWidth();
//        audioEngine->setTransportPosition(scrubPos, false);
//    }
}



void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour (Colours::whitesmoke));
        
}

void MainContentComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

float estimateAnalyticIP(MP_Atom_c* atom1, MP_Atom_c* atom2)
{
    float posDiff = fabs(atom2->get_field(MP_POS_PROP, 0) - atom1->get_field(MP_POS_PROP, 0));
    float freqDiff = fabs(atom2->get_field(MP_FREQ_PROP, 0) - atom1->get_field(MP_FREQ_PROP, 0));
    
    float timeSpread1 = atom1->get_field(MP_LEN_PROP, 0);
    float timeSpread2 = atom2->get_field(MP_LEN_PROP, 0);
    
    float averageTimeSpread = (timeSpread1 + timeSpread2) / 2.0;
    float averageFreqSpread = (1.0/timeSpread1 + 1.0/timeSpread2)/4.0;

    if (posDiff <= averageTimeSpread && fabs(freqDiff) <= averageFreqSpread)
    {
        float result = (posDiff - averageTimeSpread) * (fabs(freqDiff) - averageFreqSpread);
        return result;
    }
    return 0.0;
}


void MainContentComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == button_decomp) {
        audioEngine->triggerDecomposition(File(textEditorDictionary->getText()),
                                        File(textEditorSignal->getText()),
                                        text_editor_num_iterations->getText().getIntValue());
    }
    else if (buttonThatWasClicked == buttonSelectDictionary)
    {
        
        FileChooser chooser ("Select a MPTK Dictionary to use...",
                             File(textEditorDictionary->getText()),
                             "*.xml");
    
        if (chooser.browseForFileToOpen())
        {
            File file(chooser.getResult());
            textEditorDictionary->setText(file.getFullPathName());
        }
    }
    else if (buttonThatWasClicked == buttonSelectSignal)
    {
        
        FileChooser chooser ("Select a signal to use...",
                             File(textEditorSignal->getText()),
                             "*.wav");
        
        if (chooser.browseForFileToOpen())
        {
            File file(chooser.getResult());
            textEditorSignal->setText(file.getFullPathName());
        }
    }
    else if (buttonThatWasClicked == buttonStart)
    {
        if ( audioEngine->isPlaying() )
        {
            audioEngine->stopPlaying();
            buttonStart->setButtonText("Start");
        }
        else
        {
            audioEngine->startPlaying();
            buttonStart->setButtonText("Stop");
        }
    }
    else if (buttonThatWasClicked == buttonScrub)
    {
        if (buttonScrub->getToggleState()) {
            audioEngine->setScrubbing(buttonScrub->getToggleState());
            buttonScrub->setButtonText("Stop Scrubbing");
            audioEngine->startPlaying();
        }
        else
        {
            audioEngine->setScrubbing(buttonThatWasClicked->getToggleState());
            buttonScrub->setButtonText("Start Scrubbing");
            audioEngine->stopPlaying();
        }
    }
}