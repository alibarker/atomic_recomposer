/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef __MAINCOMPONENT_H_6363A7FA__
#define __MAINCOMPONENT_H_6363A7FA__

#include "../JuceLibraryCode/JuceHeader.h"
#include "WivigramComponent.h"
#include "mptk.h"
#include "gabor_atom_plugin.h"
#include "AtomicAudioSource.h"


//#include "harmonic_atom_plugin.h"
#define HAVE_FFTW3 1


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent : public ButtonListener, public AudioAppComponent, public Timer
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    
    void timerCallback() override
    {
        numAtoms->setText(String(atomicSource->numAtomsCurrentlyPlaying), dontSendNotification);
    }
    
    virtual void prepareToPlay (int samplesPerBlockExpected,
                                double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        
    }
   
    virtual void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        transportSource.getNextAudioBlock(bufferToFill);
    }
    
    

private:
    ScopedPointer<TextButton> button_decomp;
    ScopedPointer<TextEditor> text_editor_num_iterations;
    ScopedPointer<Label> label_num_iterations;
    ScopedPointer<WivigramComponent> wivigram;
    ScopedPointer<Label> numAtoms;

    ScopedPointer<AtomicAudioSource> atomicSource;
    AudioTransportSource transportSource;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // __MAINCOMPONENT_H_6363A7FA__
