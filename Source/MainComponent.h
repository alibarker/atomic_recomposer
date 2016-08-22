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

//#include "harmonic_atom_plugin.h"
#define HAVE_FFTW3 1


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent :    public AudioAppComponent,
                                public ButtonListener,
//                                public Component,
                                public ChangeListener,
                                public Timer,
                                public Slider::Listener,
                                public ActionListener,
                                public AudioProcessorListener
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;
    
    void buttonClicked (Button* buttonThatWasClicked) override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void timerCallback() override;
    void actionListenerCallback (const String& message) override;

    void mouseDrag(const MouseEvent& event) override;

    void sliderValueChanged (Slider* slider) override;

    void setNewBook();
    void updateWivigramParametersAndRedraw();

    void audioProcessorParameterChanged (AudioProcessor* processor,
                                         int parameterIndex,
                                         float newValue) override
    {
        if (parameterIndex == pBleedAmount)
        {
            wivigram->setBleed(*audioEngine->paramBleed);
        }
    }
    
    void audioProcessorChanged (AudioProcessor* processor) override {}
    
    void checkStatus()
    {
        statusLabel->setText(audioEngine->getStatus(), dontSendNotification);
    }
    
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override
    {
        audioEngine->prepareToPlay(sampleRate, samplesPerBlockExpected);
    }
    
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        MidiBuffer emptyBuffer;
        audioEngine->processBlock(*bufferToFill.buffer, emptyBuffer);
    }
    
    
    virtual void releaseResources() override
    {
        audioEngine->releaseResources();
    }
  

    
    Value getParameter(const Identifier& parameterId);
    void setParameter(const Identifier& parameterId, Value value);
    
private:
    
    
    
    void addParameters();

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

    ScopedPointer<Slider> sliderBleed;
    
    ScopedPointer<AtomicAudioEngine> audioEngine;
    
    
    //==============================================================================
    /* Parameters */
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // __MAINCOMPONENT_H_6363A7FA__
