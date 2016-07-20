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
#include "WivigramComponent.h"
#include "RealtimeBook.h"

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
//            changeState(Stopped);
            timeline->updateBook(&audioEngine->rtBook);
        }
    }
    
    void updateWivigram();
    
    
    
    void timerCallback() override
    {
        float newPos = audioEngine->getTransportPosition();
        timeline->setCursorPosition( newPos);
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
    
    ScopedPointer<AtomicTimelineComponent> timeline;
    
    Viewport timelineViewport;
    
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

    
   

    enum TransportState
    {
        Decomposing,
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping,
        Scrubbing
    };
    
    void changeState (TransportState newState);
    
    void decompButtonClicked()
    {
        changeState (Decomposing);
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
