/*
  ==============================================================================

    WivigramComponent.cpp
    Created: 22 Aug 2016 11:10:05am
    Author:  Alistair Barker

  ==============================================================================
*/

#include "WivigramComponent.h"

AtomComponent::AtomComponent(Image& image, int channel, AffineTransform* transform) : channelNumber(channel)
{
    // point image to generic atom
    alphaImage.setImage(image);
 
    // set overlay colour according to channel
    if (channel ==0 )
        alphaImage.setOverlayColour(Colour(255, 0, 0));
    else if (channel == 1)
        alphaImage.setOverlayColour(Colour(0, 255, 0));
    else
        alphaImage.setOverlayColour(Colour(0, 0, 255));
}


void AtomComponent::paint(Graphics& g)
{
    // draw atom using all transforms
    alphaImage.draw(g, 1.0, transformOriginal.followedBy(transformModifier) );
}

WivigramComponent::WivigramComponent(const String& name) : Component(name),
numAtoms(0)
{
    drawGenericGaborAtom();
    setBufferedToImage(true);
}


void WivigramComponent::updateBook(RealtimeBook* newBook)
{
    // set pointer to book
    rtBook = newBook;
    
    // get details of new book
    numAtoms = rtBook->book->numAtoms;
    numSamples = rtBook->book->numSamples;
    numChans = rtBook->book->numChans;
    int genericSize = genericGaborAtom->getWidth();

    // normalize range of amplitudes to [0 -> 1]
    float maxAmp = rtBook->book->atom[0]->amp[0];
    NormalisableRange<float> skew(0.0, maxAmp, 0.01, 0.25);
    
    // loop through atoms
    for (int i = 0; i < numAtoms; ++i)
    {
        // create new atom component and add to Wiviram
        AtomComponent* ac = new AtomComponent(*genericGaborAtom, 0, &overallTransform);
        addAndMakeVisible(ac);
        
        // set size ready for transformation
        ac->setBoundsRelative(0.0, 0.0, 1.0, 1.0);
        
        // get pointer to gabor atom
        MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)rtBook->book->atom[i];
        
        // get normalized amplitude
        ac->amp = skew.convertTo0to1(gabor_atom->amp[0]);
        
        // time in samples
        int atomLength = gabor_atom->support->len;
        int atomStart = gabor_atom->support->pos;
        
        // frequency relative to nyquist
        float atomHeight = 80.0f/atomLength;
        float atomFreqPos = 1 - gabor_atom->freq * 2 - 0.5 * atomHeight;
        
        // dimensions in pixels
        float xPos = (atomStart / (float) numSamples) * getWidth();
        float width = (atomLength / (float) numSamples) * getWidth();
        float height = atomHeight * getHeight();
        float yPos = atomFreqPos * getHeight();
        
        // set centre and set transform
        ac->centre.setXY(xPos + width/2.0f, yPos + height/2.0f);
        ac->transformOriginal = AffineTransform::fromTargetPoints(0, 0, xPos, yPos,
                                                                  genericSize, 0, xPos + width, yPos,
                                                                  0, genericSize, xPos, yPos + height);
        atomImages.add(ac);
    }
    

}

void WivigramComponent::setBleed(float value)
{
    // if bleed value has changed trigger asynchronous update of transforms
    if (value != currentBleedValue)
    {
        currentBleedValue = value;
        triggerAsyncUpdate();
    }
}


void WivigramComponent::handleAsyncUpdate()
{
    
    // asynchronously recalculate atom images and repaint
    for (int i = 0; i < numAtoms; ++i)
    {
        AtomComponent* ac = atomImages[i];
        ac->transformModifier = AffineTransform::scale(currentBleedValue,
                                                       1/currentBleedValue,
                                                       ac->centre.getX(),
                                                       ac->centre.getY() );
    }
    repaint();
}

double WivigramComponent::wignerVille(double t, double f)
{
    /* This function is taken from gabor_atom_plugin.cpp */
    
    double x = t - 0.5;
    double y = f - 0.5;
    
    return  exp( -(x*x)/DSP_GAUSS_DEFAULT_OPT - (y*y)/DSP_GAUSS_DEFAULT_OPT );
}


void WivigramComponent::drawGenericGaborAtom()
{
    genericAtomSize = 20;
    
    // create image to write to
    genericGaborAtom = new Image(Image::ARGB, genericAtomSize, genericAtomSize, true);
    
    // set each pixel
    for (int i = 0; i < genericAtomSize; i++ )
    {
        for (int j = 0; j < genericAtomSize; j++ )
        {
            float val = wignerVille((double) i / genericAtomSize, (double) j / genericAtomSize);
            genericGaborAtom->setPixelAt(i, j, Colour::fromHSV(1.0f, 1.0f, 1.0f, val));
        }
    }
    
}