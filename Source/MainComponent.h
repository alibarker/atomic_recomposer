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
        
//        wivigram->setImage(audioEngine->getWivigramImage());
//        wivigram->repaint();
        updateWivigram();
    }
    
    void updateWivigram()
    {
        
        int width = wivigram->getWidth();
        int height = wivigram->getHeight();
        
        MP_Tfmap_t* column;
        MP_Real_t val;
        
        Image image(Image::RGB, width, height, true);
;
        
        for (int i = 0; i < width; i++ )
        {
            column = audioEngine->map->channel[0] + i * audioEngine->map->numRows; /* Seek the column */
            
            for (int j = 0; j < height; j++ )
            {
                val = (MP_Real_t) column[j];
                image.setPixelAt(i, height - j, Colour::fromHSV (1.0f, 0.0f, 1 - val, 1.0f));
            }
        }
        
        wivigram->setImage(image);
    }
    
    
    void timerCallback() override
    {
        float newPos = audioEngine->getTransportPosition();
        cursor->scrubPos = newPos * wivigram->getWidth();
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
        void paint(Graphics& g) {g.drawLine(scrubPos, 0, scrubPos, getHeight(), 3);}
        int scrubPos;

    };
    
    ScopedPointer<Cursor> cursor;

    

    ScopedPointer<AtomicAudioEngine> audioEngine;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // __MAINCOMPONENT_H_6363A7FA__
