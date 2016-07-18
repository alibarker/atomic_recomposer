/*
  ==============================================================================

    AtomicAudioSource.cpp
    Created: 4 Jul 2016 5:13:17pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "AtomicAudioEngine.h"
#include "AtomicAudioSource.h"

AtomicAudioSource::AtomicAudioSource(AtomicAudioEngine* aae) : engine(aae)
{
    osc = new WavetableSinOscillator(4096);
    isCurrentlyScrubbing = false;
    isCurrentlyRunning = false;
    
    prevReadPosition = 0;
    nextReadPosition = 0;
    
}

int64 AtomicAudioSource::getTotalLength() const
{
    if (engine->book != nullptr)
        return engine->book->numSamples;
    else
        return 0;
}

void AtomicAudioSource::setLooping(bool shouldLoop)
{
    isCurrentlyScrubbing = shouldLoop;
}


void AtomicAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    
    int numSamples = bufferToFill.numSamples;
    int numChans = min((int) bufferToFill.buffer->getNumChannels(), (int) engine->book->numChans);
    
    bufferToFill.buffer->clear();
    
    
    if (currentBleedValue != engine->getBleedValue())
    {
        currentBleedValue = engine->getBleedValue();
        updateBleed();
    }
    
    // current atom status info
    
    int numAtoms = 0;
    int numAtomsTooQuiet = 0;
    int numAtomsNotSupported = engine->scrubAtoms.size();

    for (int i = 0; i < engine->scrubAtoms.size(); ++i)
    {
        
        // was at 13%
        AtomicAudioEngine::ScrubAtom* scrubAtom = engine->scrubAtoms.getUnchecked(i);
        MP_Atom_c* atom = scrubAtom->atom;
        
        MP_Support_t* support = atom->support;
        int atomLength = support->len;
        int atomStart = support->pos;
        int atomEnd = atomStart + atomLength;
        
        // Check if atom is supported currently
        
        if ( nextReadPosition > atomStart && nextReadPosition < atomEnd )
        {
            // Check if atom could be heard
            
            if (Decibels::gainToDecibels(*atom->amp) >= -90)
            {
                /* generate window */
                
                // TODO: Improve this line!!!!
            
                double window[bufferToFill.numSamples];
                
                if (isCurrentlyScrubbing)
                {
                    double currentWindowAmp = engine->getWindowValue( atomLength,  nextReadPosition - atomStart);
                    
                    if (nextReadPosition != prevReadPosition)
                    {
                        
                        double prevWindowAmp = engine->getWindowValue( atomLength,  prevReadPosition - atomStart);
                        
                        for (int n = 0; n < numSamples; ++n)
                        {
                            float fadeAmount = (float) n / (numSamples - 1);
                            window[n] = (1 - fadeAmount) * prevWindowAmp + fadeAmount * currentWindowAmp;
                        }
                    }
                    else
                    {
                        for (int n = 0; n < numSamples; ++n)
                        {
                            window[n] = currentWindowAmp;
                        }
                    }
                }
                else // not scrubbing
                {
                    for (int n = 0; n < numSamples; ++n)
                    {
                        double value;
                       
                        int pos = nextReadPosition - atomStart + n;
                        
                        if (pos < 0 || pos >= atom->support->len)
                            value = 0.0;
                        else
                            value = engine->getWindowValue( atomLength,  pos);
                        
                        window[n] = value;
                    }

                }
            
            
                numAtoms++;
                numAtomsNotSupported--;
                
                /* Generate output for whole buffer */
                
                int start = max((int) (atomStart - nextReadPosition), 0);
                int end = max((int) (nextReadPosition - atomEnd), numSamples);
                for (int ch = 0; ch < numChans; ch++)
                {
                    for (int n = start; n < end; ++n)
                    {
                        double amp = atom->amp[ch];

                        float phase = scrubAtom->currentPhase[ch] + scrubAtom->phaseInc;
                        
                        if (phase >= 2 * M_PI)
                            phase -= 2 * M_PI;
                        
                        double output = osc->getSample(phase) * window[n] * amp;
                        
                        bufferToFill.buffer->addSample(ch, n, output);
                        
                        
                        scrubAtom->currentPhase[ch] = phase;
                    }
                }
            } else {numAtomsTooQuiet++;}
        }
        
    }

    // update current status

    numAtomsCurrentlyPlaying = numAtoms;
    numAtomsCurrentlyTooQuiet = numAtomsTooQuiet;
    numAtomsCurrentlyNotSupported = numAtomsNotSupported;
    
    // update position as necessary
    
    if (!isCurrentlyScrubbing)
        nextReadPosition += bufferToFill.numSamples;

    prevReadPosition = nextReadPosition;
    
}

void AtomicAudioSource::prepareToPlay (int samplesPerBlockExpected,
                            double sampleRate)
{
    tempBuffer = new MP_Real_t[16834];
}



void AtomicAudioSource::updateBleed()
{
    {
                
        for (int i = 0; i < engine->book->numAtoms; ++i)
        {
            MP_Atom_c* atom = engine->book->atom[i];
            
            int originalLength = engine->scrubAtoms[i]->originalSupport.len;
            int originalStart = engine->scrubAtoms[i]->originalSupport.pos;
            
            int newLength = originalLength * currentBleedValue;
            int newStart = originalStart + round((originalLength - newLength) / 2.0);
            
            for (int ch = 0; ch < atom->numChans; ++ch)
            {
                atom->support[ch].len = newLength;
                atom->support[ch].pos = newStart;
            }
        }
    }
    engine->updateWivigram();
}