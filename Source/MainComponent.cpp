/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/


#include "MainComponent.h"


//==============================================================================
MainContentComponent::MainContentComponent()
{
    setSize (1000, 650);
    
    int timelineWidth = getWidth() - 1;
    int timelineHeight = getHeight() - 101;

    File defaultDict("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/dictGabor_original.xml");
    File defaultSignal("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/glock2.wav");

    
    audioEngine = new AtomicAudioEngine(this);
    audioEngine->setAudioChannels (0, 2);
    audioEngine->addChangeListener(this);
    audioEngine->transportSource.addChangeListener(this);
    audioEngine->addActionListener(this);
    
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
    
    /* Bleed */
    
    FloatParameter* bleedParam = dynamic_cast<FloatParameter*>(audioEngine->getParameter(0));
    
    sliderBleed = new Slider("Bleed Amount");
    sliderBleed->setBounds(160, 35, 340, 20);
    addAndMakeVisible(sliderBleed);
    sliderBleed->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    sliderBleed->setTextBoxStyle(Slider::TextBoxRight, false, 50, 20);
    sliderBleed->addListener(this);
    sliderBleed->setRange(bleedParam->range.start, bleedParam->range.end);
    sliderBleed->setValue(*bleedParam);
    
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
    
    /* Timeline/Wivigram Viewport*/
    
    wivigram = new WivigramComponent("Wivigram");
    wivigram->setInterceptsMouseClicks(false, false);
    timeline = new AtomicTimelineComponent("Timeline", wivigram, timelineHeight);
    timeline->addMouseListener(this, false);

    timelineViewport = new Viewport();
    
    timelineViewport->setViewedComponent(timeline);
    timelineViewport->setBounds(1, 60, timelineWidth, timelineHeight);
    addAndMakeVisible(timelineViewport);
    timelineViewport->setScrollBarsShown(false, true);
    
    /* Status */
    
    statusLabel = new Label();
    statusLabel->setBounds(1, 600, 300, 30);
    addAndMakeVisible(statusLabel);
    
    /* Start/Stop/Scrub Buttons */
    
    buttonStart = new TextButton("Start");
    buttonStart->setBounds(160, 1, 100, 20);
    addAndMakeVisible(buttonStart);
    buttonStart->addListener(this);

    buttonStop = new TextButton("Stop");
    buttonStop->setBounds(262, 1, 100, 20);
    addAndMakeVisible(buttonStop);
    buttonStop->addListener(this);
    
    buttonScrub = new TextButton("Scrub");
    buttonScrub->setBounds(364, 1, 100, 20);
    addAndMakeVisible(buttonScrub);
    buttonScrub->addListener(this);
    
    /* Misc */
    
//    addParameters();
    startTimerHz(30);
    changeState(Inactive);
//    initialiseParameters();

}

MainContentComponent::~MainContentComponent()
{
    audioEngine->shutdownAudio();
    timeline.release();
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

void MainContentComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == button_decomp)
    {
        decompButtonClicked();
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
        playButtonClicked();
    }
    else if (buttonThatWasClicked == buttonStop)
    {
        stopButtonClicked();
    }
    else if (buttonThatWasClicked == buttonScrub)
    {
        scrubButtonClicked();
    }
}

void MainContentComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == &(audioEngine->transportSource))
    {
        if (audioEngine->isPlaying())
            changeState (Playing);
        else if (state == Stopping || state == Playing)
            changeState (Stopped);
        else if (state == Pausing)
            changeState(Paused);
    }
    else if (source == audioEngine)
    {
        changeState(Stopped);
        setNewBook();
    }
}

void MainContentComponent::timerCallback()
{
    float newPos = audioEngine->getTransportPosition();
    timeline->setCursorPosition( newPos);

}

void MainContentComponent::actionListenerCallback (const String& message)
{
    statusLabel->setText(message, dontSendNotification);
}


void MainContentComponent::mouseDrag(const MouseEvent &event)
{
    if (event.originalComponent == timeline)
    {
        float scrubPos = (float) event.x / (float) timeline->getWidth();
        audioEngine->setTransportPosition(scrubPos, !event.mouseWasClicked());
    }
}

void MainContentComponent::sliderValueChanged (Slider* slider)
{
    if (slider == sliderBleed)
    {
        float value = sliderBleed->getValue();
        *dynamic_cast<FloatParameter*>(audioEngine->getParameter(pBleedAmount)) = value;
        wivigram->setBleed(value);
    }
}

void MainContentComponent::setNewBook()
{
    int newWidth = audioEngine->rtBook.book->numSamples / 100;
    
    timeline->setBounds(0, 0, newWidth, timelineViewport->getMaximumVisibleHeight());
    wivigram->setBoundsRelative(0.0f, 0.0f, 1.0f, 1.0f);
    wivigram->setBleed( *dynamic_cast<FloatParameter*>(audioEngine->getParameter(pBleedAmount)));
    wivigram->updateBook(&audioEngine->rtBook);
}



//void MainContentComponent::addParameters()
//{
//    parameters.add(new AudioParameterFloat ("bleed", "Bleed Value",
//                                            NormalisableRange<float>(0.125, 8, 0.001, 1.0),
//                                            1.0));
//    
////    jassert(parameters.size() == pNumParams);
//}



//void MainContentComponent::initialiseParameters()
//{
//    setParameter(pBleedAmount, audioEngine->getBleedValue());
//}


void MainContentComponent::decompButtonClicked()
{
    changeState (Decomposing);
}

void MainContentComponent::playButtonClicked()
{
    if (state == Playing)
        changeState(Pausing);
    else if (state != Decomposing)
        changeState (Starting);
}

void MainContentComponent::stopButtonClicked()
{
    if (state == Paused)
        changeState (Stopped);
    else
        changeState (Stopping);
}

void MainContentComponent::scrubButtonClicked()
{
    if (audioEngine->transportSource.isLooping())
        changeState(Stopped);
    else
        changeState(Scrubbing);
}

void MainContentComponent::changeState (TransportState newState)
{
    if (state != newState)
    {
        state = newState;
        
        switch (state)
        {
            case Inactive:
                buttonStop->setEnabled(false);
                buttonStart->setEnabled(false);
                buttonScrub->setEnabled(false);
                
            case Decomposing:
                buttonStop->setEnabled(false);
                buttonStart->setEnabled(false);
                buttonScrub->setEnabled(false);
                button_decomp->setEnabled(false);
                audioEngine->triggerDecomposition( File(textEditorDictionary->getText()),
                                                  File(textEditorSignal->getText()),
                                                  text_editor_num_iterations->getText().getIntValue() );
                break;
            case Stopped:                           // [3]
                buttonStop->setEnabled (false);
                buttonStop->setButtonText("Stop");
                buttonStart->setButtonText ("Start");
                buttonStart->setEnabled(true);
                buttonScrub->setEnabled(true);
                buttonScrub->setButtonText("Scrub On");
                button_decomp->setEnabled(true);
                audioEngine->setTransportPosition(0.0, false);
                audioEngine->setScrubbing(false);
                break;
                
            case Starting:                          // [4]
                audioEngine->startPlaying();
                audioEngine->setScrubbing(false);
                break;
                
            case Playing:                           // [5]
                buttonStart->setButtonText("Pause");
                buttonStop->setButtonText("Stop");
                buttonStop->setEnabled (true);
                buttonScrub->setButtonText("Scrub On");
                break;
                
            case Pausing:
                audioEngine->stopPlaying();
                audioEngine->setScrubbing(false);
                break;
                
            case Paused:
                buttonStart->setButtonText("Resume");
                buttonStop->setButtonText("Restart");
                buttonScrub->setButtonText("Scrub On");
                
                break;
                
            case Stopping:                          // [6]
                audioEngine->stopPlaying();
                audioEngine->setScrubbing(false);
                break;
                
            case Scrubbing:                          // [6]
                audioEngine->setScrubbing(true);
                buttonStop->setEnabled(false);
                buttonStart->setEnabled(false);
                buttonScrub->setButtonText("Scrub Off");
                break;
        }
    }
}