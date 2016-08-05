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
    isCurrentlyLooping = false;
    isCurrentlyReversing = false;
    
    prevReadPosition = 0;
    nextReadPosition = 0;
    
    
    
}

int64 AtomicAudioSource::getTotalLength() const
{
    if (engine->rtBook.book != nullptr)
        return engine->rtBook.book->numSamples;
    else
        return 0;
}

void AtomicAudioSource::smoothScrubbing()
{
    // smooth scrubbing
    if (targetPosition > 0)
    {
        int currentPos = nextReadPosition;
        int nextPos = targetPosition;
        int jumpAmount = expBufferSize * engine->getParameter(pMaxScrubSpeed);
        if (targetPosition > currentPos)
        {
            nextPos = currentPos + jumpAmount;
            if (nextPos >= targetPosition)
            {
                nextPos = targetPosition;
                targetPosition = -1;
            }
        }
        else if (targetPosition <= currentPos)
        {
            nextPos = currentPos - jumpAmount;
            if (nextPos <= targetPosition)
            {
                nextPos = targetPosition;
                targetPosition = -1;
            }
            
        }
        
        nextReadPosition = nextPos;
    }
}


void AtomicAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    smoothScrubbing();
    
    int numSamples = bufferToFill.numSamples;
    int numChans = min((int) bufferToFill.buffer->getNumChannels(), (int) engine->rtBook.book->numChans);
    bufferToFill.buffer->clear();
    
    float currentBleedValue = engine->getParameter(pBleedAmount);
    int playbackLimit = engine->getParameter(pAtomLimit);

    
    // current atom status info
    int numAtoms = engine->rtBook.realtimeAtoms.size();
    int numAtomsCurrentlyPlaying = 0;
    int numAtomsTooQuiet = 0;
    int numAtomsNotSupported = numAtoms;
    
    float shortAtomLimit = 10.f/1000.0f * currentSampleRate;

    for (int i = 0; i < numAtoms; ++i)
    {
        
        RealtimeAtom* scrubAtom = engine->rtBook.realtimeAtoms.getUnchecked(i);
        MP_Atom_c* atom = scrubAtom->atom;
        
        MP_Support_t* support = atom->support;
        
        int originalLength = support->len;
        int originalStart = support->pos;
        
        int atomLength = originalLength * currentBleedValue;
        if (atomLength < shortAtomLimit)
            atomLength = shortAtomLimit;
    
        int atomStart = originalStart - (atomLength - originalLength) / 2.0f;
        int atomEnd = atomStart + atomLength;
        
        // Check if atom is supported currently
        
        if ( nextReadPosition > atomStart && nextReadPosition < atomEnd )
        {
            // Check if atom could be heard
            
            if (Decibels::gainToDecibels(*atom->amp) >= -90)
            {
                /* generate window */
                            
                double window[bufferToFill.numSamples];
                
                int start = max((int) (atomStart - nextReadPosition), 0);
                int end = max((int) (nextReadPosition - atomEnd), numSamples);

                if (isCurrentlyScrubbing)
                {
                    double currentWindowAmp = getScaledWindowValue( atomLength,  nextReadPosition - atomStart);
                    
                    if (nextReadPosition != prevReadPosition)
                    {
                        
                        double prevWindowAmp = getScaledWindowValue( atomLength,  prevReadPosition - atomStart);
                        
                        for (int n = 0; n < numSamples; ++n)
                        {
                            float fadeAmount = (float) n / (numSamples - 1);
                            window[n] = (1 - fadeAmount) * prevWindowAmp + fadeAmount * currentWindowAmp;
                        }
                    }
                    else
                    {
                        for (int n = start; n < end; ++n)
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
                       
                        int pos;
                        
                        if (isCurrentlyReversing)
                            pos = nextReadPosition - atomStart - n;
                        else
                            pos = nextReadPosition - atomStart + n;

                        
                        if (pos < 0 || pos >= atomLength)
                            value = 0.0;
                        else
                            value = getScaledWindowValue( atomLength,  pos);
                        
                        window[n] = value;
                    }

                }
            
            
                
                /* Generate output for whole buffer */
                
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
                
                numAtomsCurrentlyPlaying++;
                numAtomsNotSupported--;

                if (numAtomsCurrentlyPlaying == playbackLimit) {
                    break;
                }
                
            } else {numAtomsTooQuiet++;}
        }
        
    }

    // update current status

    currentlyPlaying = numAtomsCurrentlyPlaying;
    currentlyTooQuiet = numAtomsTooQuiet;
    currentlyNotSupported = numAtomsNotSupported;
    
    // update position as necessary

    prevReadPosition = nextReadPosition;

    if (!isCurrentlyScrubbing && !isCurrentlyReversing)
        nextReadPosition += bufferToFill.numSamples;
    else if(!isCurrentlyScrubbing && isCurrentlyReversing)
        nextReadPosition -= bufferToFill.numSamples;
    
    
    if (nextReadPosition >= getTotalLength() && isCurrentlyLooping)
    {
        setNextReadPosition(0);
    }
    
    
    
}

double AtomicAudioSource::getScaledWindowValue(int atomLength, int pos)
{
//    int shape = engine->getParameter(pWindowShape);
    
    return engine->getWindowValue(atomLength, pos);
}


void AtomicAudioSource::prepareToPlay (int samplesPerBlockExpected,
                            double sampleRate)
{
    tempBuffer = new MP_Real_t[16834];
    currentSampleRate = sampleRate;
    expBufferSize = samplesPerBlockExpected;
}



