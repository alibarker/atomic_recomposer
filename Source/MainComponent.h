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
#include "AtomicAudioEngine.h"


//#include "harmonic_atom_plugin.h"
#define HAVE_FFTW3 1


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent : public ButtonListener, public Component, public ChangeListener, public Timer
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
        wivigram->setImage(audioEngine->getWivigramImage());
        wivigram->repaint();
    }
    
    void timerCallback() override
    {
        float newPos = audioEngine->getTransportPosition();
        cursor->scrubPos = newPos * wivigram->getWidth();
        cursor->repaint();
        
    }
    
    void updateWivigram();
    
    void mouseDrag(const MouseEvent& event) override;

private:
    ScopedPointer<TextButton> button_decomp;
    ScopedPointer<TextEditor> text_editor_num_iterations;
    ScopedPointer<Label> label_num_iterations;
    ScopedPointer<ImageComponent> wivigram;
    ScopedPointer<Label> numAtoms;
    
    class Cursor : public Component
    {
    public:
        void paint(Graphics& g) {g.drawLine(scrubPos, 0, scrubPos, getHeight(), 3);}
        int scrubPos;

    };
    
    ScopedPointer<Cursor> cursor;

    

    ScopedPointer<AtomicAudioEngine> audioEngine;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // __MAINCOMPONENT_H_6363A7FA__
