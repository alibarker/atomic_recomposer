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
#include "RealtimeBook.h"



class AtomComponent : public Component
{
public:
    AtomComponent(Image& image, int channel) : channelNumber(channel)
    {
        alphaImage.setImage(image);
        addAndMakeVisible(alphaImage);
        if (channel ==0 )
            alphaImage.setOverlayColour(Colour(255, 0, 0));
        else if (channel == 1)
            alphaImage.setOverlayColour(Colour(0, 255, 0));
        else
            alphaImage.setOverlayColour(Colour(0, 0, 255));

        
    }
    
    void resized() override
    {
        alphaImage.setTransformToFit(Rectangle<float>(0, 0, getWidth(), getHeight()), 64);

    }
    
    void paint(Graphics& g) override
    {
        alphaImage.drawAt(g, 0, 0, 1.0);
        
    }
    
    
private:
    int channelNumber;
    DrawableImage alphaImage;
};

class WivigramComponent :public Component
{
public:
    WivigramComponent(const String& name) : Component(name)
    {
        drawGenericGaborAtom();
        setBufferedToImage(true);

    }
    
    void updateWivigram(float bleedValue)
    {
        for (int ch = 0; ch < numChans; ++ch)
        {
            for (int i = 0; i < numAtoms; ++i)
            {
                
                MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)rtBook->book->atom[i];
                
                int originalLength = gabor_atom->support->len;
                int originalStart = gabor_atom->support->pos;
                
                int atomLength = originalLength * bleedValue;
                int atomStart = originalStart - (atomLength - originalLength) / 2.0f;

                
                
                float xPos = (atomStart) / (float) numSamples;
                float width = atomLength / (float) numSamples;
                float height = (float) 80 / ( atomLength) ;
                float yPos = 1 - gabor_atom->freq * 2.0 - height * 0.5;

                
                AtomComponent* ac = atomImages[ch][i];
                
                ac->setBoundsRelative(xPos, yPos, width, height);
                
            }
                
        }
        
        repaint();
    }
    
    void updateBook(RealtimeBook* newBook)
    {
    
        deleteAllChildren();
        atomImages.clearQuick();
        
        rtBook = newBook;
        
        numAtoms = rtBook->book->numAtoms;
        numSamples = rtBook->book->numSamples;
        numChans = rtBook->book->numChans;
        
        for (int ch = 0; ch < numChans; ++ch)
        {
            
            Array<AtomComponent*> channelArray;
            
            for (int i = 0; i < numAtoms; ++i)
            {
                AtomComponent* ac = new AtomComponent(*genericGaborAtom, ch);
                addAndMakeVisible(ac);
                channelArray.add(ac);
            }
            
            atomImages.add(channelArray);
            
        }
        
        updateWivigram(1.0);
        
    }
    
    double wignerVille(double t, double f)
    {
        /* This function is copied from gabor_atom_plugin.cpp */
        
//        static double factor = 1/MP_PI;
        double x = t - 0.5;
        double y = f - 0.5;

        double val =  exp( -(x*x)/DSP_GAUSS_DEFAULT_OPT - (y*y)/DSP_GAUSS_DEFAULT_OPT );
        
        
        return val;
        
    }

    
    void drawGenericGaborAtom()
    {
        int size = 100;
        genericGaborAtom = new Image(Image::ARGB, size, size, true);
        
        
        for (int i = 0; i < size; i++ )
        {
            for (int j = 0; j < size; j++ )
            {
                float val = wignerVille((double) i / size, (double) j / size);
                genericGaborAtom->setPixelAt(i, j, Colour::fromHSV(1.0f, 1.0f, 1.0f, val));
            }
        }
        
    }
    
    ScopedPointer<Image> genericGaborAtom;
    
    Array<Array<AtomComponent*>> atomImages;
    
    RealtimeBook* rtBook;
    
    int numAtoms;
    int numSamples;
    int numChans;
    
};

class AtomicTimelineComponent : public Component
{
public:
    AtomicTimelineComponent(const String& componentName, int width, int newHeight)
                                : Component (componentName) , wivigram("WiviView"), height(newHeight)
    {
        addAndMakeVisible(wivigram);
//        addAndMakeVisible(cursor);
        wivigram.setInterceptsMouseClicks(false, false);
    }

    void resized() override
    {
    }

    void updateBook(RealtimeBook* rtBook)
    {
        
        int newWidth = rtBook->book->numSamples / 100;
        
        setBounds(0, 0, newWidth, height);
        
        wivigram.setBoundsRelative(0.0f, 0.0f, 1.0f, 1.0f);
        wivigram.updateBook(rtBook);
        wivigram.updateWivigram(1.0);
    }
    
    void updateWivigram(float bleedValue)
    {
        wivigram.updateWivigram(bleedValue);
    }
    
    void setCursorPosition(float newPos)
    {
        scrubPos = newPos * getWidth();
        repaint();
    }
    
    void paintOverChildren(Graphics& g) override
    {
        if (isPlayingLeftToRight)
            g.drawLine(scrubPos, 0, scrubPos, getHeight(), 3);
        else
            g.drawLine(0, scrubPos, getWidth(), scrubPos, 3);
    }
    
private:
    WivigramComponent wivigram;

    
    int scrubPos;
    bool isPlayingLeftToRight = true;

    int height;
    
};






//==============================================================================









#endif  // WIVIGRAMCOMPONENT_H_INCLUDED
