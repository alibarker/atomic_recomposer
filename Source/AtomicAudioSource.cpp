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
    osc = new WavetableSinOscillator(pow(2, 15));
    
    isCurrentlyScrubbing = false;
    isCurrentlyRunning = false;
    isCurrentlyLooping = false;
    isCurrentlyReversing = false;
    
    prevReadPosition = 0;
    nextReadPosition = 0;
    
    currentVocoderValue = 0;
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
        
        // if moving forward increment by jump amount
        // and check whether it is past the target
        if (targetPosition > currentPos)
        {
            nextPos = currentPos + jumpAmount;
            if (nextPos >= targetPosition)
            {
                nextPos = targetPosition;
                targetPosition = -1;
            }
        }
        // do the opposite for reverse
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
 
    // buffer preperation
    int numSamples = bufferToFill.numSamples;
    int numChans = min((int) bufferToFill.buffer->getNumChannels(), (int) engine->rtBook.book->numChans);
    
    // current atom status info
    int numAtoms = engine->rtBook.realtimeAtoms.size();
    int numAtomsCurrentlyPlaying = 0;
    int numAtomsTooQuiet = 0;
    int numAtomsNotSupported = numAtoms;
    
    float shortAtomLimit = 0.02 * currentSampleRate; // limit atom length to 20ms

    smoothScrubbing();
    
    for (int i = 0; i < numAtoms; ++i)
    {
        // get current atom details
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
                       
                        int posInAtom;
                        
                        if (isCurrentlyReversing)
                            posInAtom = nextReadPosition - atomStart - n * currentSpeed;
                        else
                            posInAtom = nextReadPosition - atomStart + n * currentSpeed;

                        
                        if (posInAtom < 0 || posInAtom >= atomLength)
                            value = 0.0;
                        else
                            value = getScaledWindowValue( atomLength,  posInAtom);
                        
                        window[n] = value;
                    }

                }
            
            
                
                /* Generate output for whole buffer */
                
                for (int ch = 0; ch < numChans; ch++)
                {
                    float currentPhaseInc = scrubAtom->phaseInc * currentPitchShift;
                    
                    double amp = atom->amp[ch];
                    double output;

                    
                    if (currentPhaseInc < M_PI)
                    {
                        for (int n = start; n < end; ++n)
                        {
                            
                            float phase = scrubAtom->currentPhase[ch] + currentPhaseInc;
                            
                            if (phase >= 2 * M_PI)
                                phase -= 2 * M_PI;
                            
                            output = osc->getSample(phase) * window[n] * amp;
                            
                            bufferToFill.buffer->addSample(ch, n, output);
                            
                            
                            scrubAtom->currentPhase[ch] = phase;
                        }
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
    
    int sampleInc = bufferToFill.numSamples * currentSpeed;

    if (!isCurrentlyScrubbing && !isCurrentlyReversing)
        nextReadPosition += sampleInc;
    else if(!isCurrentlyScrubbing && isCurrentlyReversing)
        nextReadPosition -= sampleInc;
    
    
    if (nextReadPosition >= getTotalLength() && isCurrentlyLooping)
    {
        setNextReadPosition(0);
    }
    
    
    
}

double AtomicAudioSource::getScaledWindowValue(int atomLength, int pos)
{
    
    return engine->getWindowValue(atomLength, pos);
}


void AtomicAudioSource::prepareToPlay (int samplesPerBlockExpected,
                            double sampleRate)
{
    tempBuffer = new MP_Real_t[16834];
    currentSampleRate = sampleRate;
    expBufferSize = samplesPerBlockExpected;
}



