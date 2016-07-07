/*
  ==============================================================================

    WivigramComponent.h
    Created: 16 Jun 2016 4:58:23pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef WIVIGRAMCOMPONENT_H_INCLUDED
#define WIVIGRAMCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "mptk.h"

//==============================================================================
/*
*/
class WivigramComponent    : public Component, public Timer
{
public:
    WivigramComponent(const int width, const int height, AudioTransportSource* ats) : wivigramImage(Image::RGB, width, height, true), transport(ats) {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        startTimerHz(50);
        
    }
    
    void timerCallback() override
    {
        currentPosPercentage = transport->getCurrentPosition()/transport->getLengthInSeconds();
        repaint();
    }

    ~WivigramComponent()
    {
    }
    
    
    virtual void mouseDrag(const MouseEvent& event) override
    {
        float scrubPos = (float) event.x / (float) getWidth();
        int64 length = transport->getTotalLength();
        int64 newPos = min ( (int64) round(scrubPos*length), length -1) ;
        
        
        
        transport->setNextReadPosition( newPos );
    }

    void paint (Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        g.drawImageWithin (wivigramImage, 0, 0, getWidth(), getHeight(), RectanglePlacement::stretchToFit);
//        g.drawImageWithin (wivigramLongImage, 0, getHeight()/2.0, getWidth(), getHeight()/2.0, RectanglePlacement::stretchToFit);
        
        int xPos = getWidth() * currentPosPercentage;
        
        g.drawLine(xPos, 0, xPos , getHeight(), 5);
        
        
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }
    
    void updateWiviGram(MP_TF_Map_c* shortMap, MP_TF_Map_c* longMap)
    {
        
        const int width = getWidth();
        const int height = getHeight();
                
        MP_Tfmap_t* column;
        MP_Real_t val;

        for (int i = 0; i < width; i++ )
        {
            column = shortMap->channel[0] + i * shortMap->numRows; /* Seek the column */
            
            for (int j = 0; j < height; j++ )
            {
                val = (MP_Real_t) column[j];
                wivigramImage.setPixelAt(i, height - j, Colour::fromHSV (1.0f, 0.0f, 1 - val, 1.0f));
            }
        }
 
        
        repaint();
    }


private:
    Image wivigramImage;
    
    AudioTransportSource* transport;
    float currentPosPercentage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WivigramComponent)
};


#endif  // WIVIGRAMCOMPONENT_H_INCLUDED
