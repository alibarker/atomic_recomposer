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

    
    audioEngine = new AtomicAudioEngine(wiviWidth, wiviHeight);
    audioEngine->setAudioChannels (0, 2);
    audioEngine->addChangeListener(this);
    

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
    addMouseListener(this, true);

    cursor = new Cursor();
    addAndMakeVisible(cursor);
    cursor->setBounds(wivigram->getBounds());
                                 
    numAtoms = new Label();
    numAtoms->setBounds(160, 1, 800, 20);
    addAndMakeVisible(numAtoms);
    
    startTimerHz(60);
    

}

MainContentComponent::~MainContentComponent()
{
    button_decomp = 0;
    label_num_iterations = 0;
    text_editor_num_iterations = 0;
    audioEngine->shutdownAudio();
    
}

void MainContentComponent::mouseDrag(const juce::MouseEvent &event)
{
    float scrubPos = (float) event.x / (float) getWidth();
    audioEngine->setTransportPosition(scrubPos);
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
        audioEngine->triggerDecomposition(File("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/dictGabor_original.xml"),
                                        File("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/glock2.wav"),
                                        text_editor_num_iterations->getText().getIntValue());
    }
    

}