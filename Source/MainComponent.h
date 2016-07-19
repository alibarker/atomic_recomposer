/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef __MAINCOMPONENT_H_6363A7FA__
#define __MAINCOMPONENT_H_6363A7FA__

#include "../JuceLibraryCode/JuceHeader.h"
#include "mptk.h"
#include "gabor_atom_plugin.h"
#include "AtomicAudioSource.h"
#include "AtomicAudioEngine.h"


//#include "harmonic_atom_plugin.h"
#define HAVE_FFTW3 1


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent : public ButtonListener, public Component, public ChangeListener, public Timer, public Slider::Listener
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void changeListenerCallback(ChangeBroadcaster* source) override
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
            updateWivigram();
        }
    }
    
    void updateWivigram();
    
    
    
    void timerCallback() override
    {
        float newPos = audioEngine->getTransportPosition();
        cursor->scrubPos = newPos * wivigram->getWidth();
        cursor->isPlayingLeftToRight = audioEngine->getIsPlayingLeftToRight();
        cursor->repaint();
        
        checkStatus();
        
    }
    
    void checkStatus()
    {
        String status = audioEngine->getStatus();
        statusLabel->setText(status, dontSendNotification);
    }
    
    void mouseDrag(const MouseEvent& event) override;
    void mouseDown(const MouseEvent &event) override;
    void sliderValueChanged (Slider* slider) override
    {
        audioEngine->setBleedValue(slider->getValue());
    }

private:
    ScopedPointer<TextButton> button_decomp;
    ScopedPointer<TextEditor> text_editor_num_iterations;
    ScopedPointer<Label> label_num_iterations;
    
    ScopedPointer<ImageComponent> wivigram;
    MP_TF_Map_c* map;
    
    
    ScopedPointer<Label> statusLabel;
    
    ScopedPointer<TextEditor> textEditorDictionary;
    ScopedPointer<TextEditor> textEditorSignal;
    ScopedPointer<TextButton> buttonSelectDictionary;
    ScopedPointer<TextButton> buttonSelectSignal;
    ScopedPointer<Label> labelSelectDictionary;
    ScopedPointer<Label> labelSelectSignal;

    ScopedPointer<Slider> sliderBleed;
    
    ScopedPointer<TextButton> buttonStart;
    ScopedPointer<TextButton> buttonStop;
    ScopedPointer<TextButton> buttonScrub;

    
    class Cursor : public Component
    {
    public:
        void paint(Graphics& g) {
            if (isPlayingLeftToRight)
                g.drawLine(scrubPos, 0, scrubPos, getHeight(), 3);
            else
                g.drawLine(0, scrubPos, getWidth(), scrubPos, 3);
        }
        
        int scrubPos;
        
        bool isPlayingLeftToRight;

    };
    
    ScopedPointer<Cursor> cursor;

    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping,
        Scrubbing
    };
    
    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;
            
            switch (state)
            {
                case Stopped:                           // [3]
                    buttonStop->setEnabled (false);
                    buttonStop->setButtonText("Stop");
                    buttonStart->setButtonText ("Resume");
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
                    break;
                    
                case Pausing:
                    audioEngine->stopPlaying();
                    audioEngine->setScrubbing(false);
                    break;
                    
                case Paused:
                    buttonStart->setButtonText("Resume");
                    buttonStop->setButtonText("Restart");
                    break;
                    
                case Stopping:                          // [6]
                    audioEngine->stopPlaying();
                    audioEngine->setScrubbing(false);
                    break;
                    
                case Scrubbing:                          // [6]
                    audioEngine->setScrubbing(true);
                    break;
            }
        }
    }
    
    void playButtonClicked()
    {
        if (state == Stopped || state == Paused)
            changeState (Starting);
        else if (state == Playing)
            changeState(Pausing);
    }
    
    void stopButtonClicked()
    {
        if (state == Paused)
            changeState (Stopped);
        else
            changeState (Stopping);
    }
    
    void scrubButtonClicked()
    {
        if (audioEngine->transportSource.isLooping())
            changeState(Stopped);
        else
            changeState(Scrubbing);
    }

    
    TransportState state;

    ScopedPointer<AtomicAudioEngine> audioEngine;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // __MAINCOMPONENT_H_6363A7FA__
