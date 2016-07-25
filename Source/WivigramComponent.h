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


class WivigramComponent :public Component
{
public:
    WivigramComponent(const String& name) : Component(name)
    {
        drawGenericGaborAtom();
        setBufferedToImage(true);
    }
    

    
    void updateWivigram()
    {
    }
    
    
    void updateBook(RealtimeBook* newBook)
    {
    
        removeAllChildren();
        
        rtBook = newBook;
        
        numAtoms = rtBook->book->numAtoms;
        numSamples = rtBook->book->numSamples;
        numChans = rtBook->book->numChans;
        

        
        for (int i = 0; i < numAtoms; ++i)
        {
            AtomComponent* ac = new AtomComponent(*genericGaborAtom, 0, &overallTransform);
            addAndMakeVisible(ac);

            ac->setBoundsRelative(0.0, 0.0, 1.0, 1.0);
            int genericSize = genericGaborAtom->getWidth();
            
            MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)rtBook->book->atom[i];
            
            // in samples
            int atomLength = gabor_atom->support->len;
            int atomStart = gabor_atom->support->pos;
            
            // relative to nyquist
            float atomHeight = 80.0f/atomLength;
            float atomFreqPos = 1 - gabor_atom->freq * 2 - 0.5 * atomHeight;
            
            // pixels
            float xPos = atomStart / (float) numSamples * getWidth();
            float width = atomLength / (float) numSamples * getWidth();
            float height = atomHeight * getHeight();
            float yPos = atomFreqPos * getHeight();
            
            ac->centre.setXY(xPos + width/2.0f, yPos + height/2.0f);
            
            ac->transformOriginal = AffineTransform::fromTargetPoints(0, 0, xPos, yPos,
                                                                      genericSize, 0, xPos + width, yPos,
                                                                      0, genericSize, xPos, yPos + height);
            atomImages.add(ac);
        }
        
        
        
        updateWivigram();
        
    }
    
    void setBleed(float value)
    {
        if (value != currentBleedValue)
        {
            currentBleedValue = value;
            
                for (int i = 0; i < numAtoms; ++i)
                {
                    
                    //                MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)rtBook->book->atom[i];
                                    AtomComponent* ac = atomImages[i];
                    
                    ac->transformModifier = AffineTransform::scale(currentBleedValue, 1/currentBleedValue,
                                                                   ac->centre.getX(), ac->centre.getY() );
                    
                    
                    
                }
                
                        repaint();
        }
    }
    
private:
    
    double wignerVille(double t, double f)
    {
        /* This function is copied from gabor_atom_plugin.cpp */
        
        double x = t - 0.5;
        double y = f - 0.5;

        return  exp( -(x*x)/DSP_GAUSS_DEFAULT_OPT - (y*y)/DSP_GAUSS_DEFAULT_OPT );
    }

    
    void drawGenericGaborAtom()
    {
        genericAtomSize = 20;
        genericGaborAtom = new Image(Image::ARGB, genericAtomSize, genericAtomSize, true);

        for (int i = 0; i < genericAtomSize; i++ )
        {
            for (int j = 0; j < genericAtomSize; j++ )
            {
                float val = wignerVille((double) i / genericAtomSize, (double) j / genericAtomSize);
                genericGaborAtom->setPixelAt(i, j, Colour::fromHSV(1.0f, 1.0f, 1.0f, val));
            }
        }
        
    }
    
    int genericAtomSize;
    ScopedPointer<Image> genericGaborAtom;
    
    class AtomComponent : public Component
    {
    public:
        AtomComponent(Image& image, int channel, AffineTransform* transform) : channelNumber(channel), transformGlobal(transform)
        {
            alphaImage.setImage(image);
            if (channel ==0 )
                alphaImage.setOverlayColour(Colour(255, 0, 0));
            else if (channel == 1)
                alphaImage.setOverlayColour(Colour(0, 255, 0));
            else
                alphaImage.setOverlayColour(Colour(0, 0, 255));
            
            
        }
        
        
        void paint(Graphics& g) override
        {
            alphaImage.draw(g, 1.0, transformOriginal.followedBy(transformModifier).followedBy(*transformGlobal) );
        }
        
        DrawableImage alphaImage;
        
        AffineTransform transformOriginal;
        AffineTransform transformModifier;
        AffineTransform* transformGlobal;

        Point<float> centre;
    private:
        int channelNumber;
    };
    
    OwnedArray<AtomComponent> atomImages;
    
    RealtimeBook* rtBook;
    
    float currentBleedValue;
    
    int numAtoms;
    int numSamples;
    int numChans;
    
    AffineTransform overallTransform;
    
};


//==============================================================================



class AtomicTimelineComponent : public Component
{
public:
    AtomicTimelineComponent(const String& componentName, WivigramComponent* wivi, int newHeight)
                                : Component (componentName) , wivigram(wivi), height(newHeight)
    {
        addAndMakeVisible(wivigram);
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
    WivigramComponent* wivigram;

    int scrubPos;
    bool isPlayingLeftToRight = true;

    int height;
    
};






//==============================================================================


#endif  // WIVIGRAMCOMPONENT_H_INCLUDED
