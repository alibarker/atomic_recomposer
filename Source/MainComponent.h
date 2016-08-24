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
#include "ParametersWindow.h"
#include "AtomicTimeLineComponent.h"

// forces MPTK to use latest version of MPTK
#define HAVE_FFTW3 1


//==============================================================================


class MainContentComponent :    public AudioAppComponent,
                                public ButtonListener,
                                public ChangeListener,
                                public Timer,
                                public ActionListener,
                                public AudioProcessorListener
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;
    
    // various async call back methods
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void timerCallback() override;
    void actionListenerCallback (const String& message) override;

    void buttonClicked (Button* buttonThatWasClicked) override;
    void mouseDrag(const MouseEvent& event) override;

    void setNewBook();
    void updateWivigramParametersAndRedraw();

    // used to update GUI when parameters change
    void audioProcessorParameterChanged (AudioProcessor* processor,
                                         int parameterIndex,
                                         float newValue) override;
    
    void audioProcessorChanged (AudioProcessor* processor) override {}
    
    void checkStatus() { statusLabel->setText(audioEngine->getStatus(), dontSendNotification); }
    
    // audio call back methods
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override;
    
    virtual void releaseResources() override { audioEngine->releaseResources(); }
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    
private:
    
    void decompButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void scrubButtonClicked();
    void loopButtonClicked();
    void reverseButtonClicked();
    
    enum TransportState
    {
        Inactive,
        Decomposing,
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping,
        Scrubbing
    };
    
    TransportState state;
    void changeState (TransportState newState);

    // UI Components
    ScopedPointer<TextButton> button_decomp;
    ScopedPointer<TextEditor> text_editor_num_iterations;
    ScopedPointer<Label> label_num_iterations;
    
    ScopedPointer<TextEditor> textEditorDictionary;
    ScopedPointer<TextButton> buttonSelectDictionary;
    ScopedPointer<Label> labelSelectDictionary;

    ScopedPointer<TextEditor> textEditorSignal;
    ScopedPointer<TextButton> buttonSelectSignal;
    ScopedPointer<Label> labelSelectSignal;

    ScopedPointer<TextButton> buttonStart;
    ScopedPointer<TextButton> buttonStop;
    ScopedPointer<TextButton> buttonScrub;
    ScopedPointer<TextButton> buttonLoopOn;
    ScopedPointer<TextButton> buttonReverse;
    
    ScopedPointer<AtomicTimelineComponent> timeline;
    ScopedPointer<Viewport> timelineViewport;
    ScopedPointer<WivigramComponent> wivigram;
    
    ScopedPointer<DocumentWindow> parametersWindow;
    ScopedPointer<ParametersWindow> paramComponent;

    ScopedPointer<Label> underrunStatus;
    ScopedPointer<Label> statusLabel;
    int underrunCounter;
    
    // The audio engine
    ScopedPointer<AtomicAudioEngine> audioEngine;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // __MAINCOMPONENT_H_6363A7FA__
