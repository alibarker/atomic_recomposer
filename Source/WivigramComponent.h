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
#include "RealtimeBook.h"
#include "gabor_atom_plugin.h"

//==============================================================================


// class that draws one atom with different affine transforms applied

class AtomComponent : public Component
{
public:
    AtomComponent(Image& image, int channel, AffineTransform* transform);
    
    void paint(Graphics& g) override;
    
    DrawableImage alphaImage;
    
    AffineTransform transformOriginal;
    AffineTransform transformModifier;
    
    Point<float> centre;
    float amp;
private:
    int channelNumber;
};

//==============================================================================

// class which draws the wivigram component

class WivigramComponent : public Component, public AsyncUpdater
{
public:
    WivigramComponent(const String& name);
    
    void clearBook() { removeAllChildren(); }
    
    void paint(Graphics &g) override { g.fillAll(Colours::white); }
    
    void updateBook(RealtimeBook* newBook);
    void setBleed(float value);
    
    void handleAsyncUpdate() override;
private:
    
    double wignerVille(double t, double f);
    void drawGenericGaborAtom();
    
    int genericAtomSize;
    ScopedPointer<Image> genericGaborAtom;
    
    OwnedArray<AtomComponent> atomImages;
    
    RealtimeBook* rtBook;
    
    float currentBleedValue;
    
    int numAtoms;
    int numSamples;
    int numChans;
    
    AffineTransform overallTransform;
    
};

//==============================================================================


#endif  // WIVIGRAMCOMPONENT_H_INCLUDED
