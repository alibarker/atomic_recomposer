/*
  ==============================================================================

    AtomicAudioSource.cpp
    Created: 4 Jul 2016 5:13:17pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "AtomicAudioSource.h"

AtomicAudioSource::AtomicAudioSource()
{
    osc = new WavetableSinOscillator(4096);
}

void AtomicAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    
    int numSamples = bufferToFill.numSamples;
    
    bufferToFill.buffer->clear();
    
    int bufferStart = nextReadPosition;
    int bufferEnd = nextReadPosition + numSamples;
    
    
    if (!isCurrentlyScrubbing)
    {
        for (int i = 0; i < scrubAtoms.size(); ++i)
        {
            MP_Atom_c* atom = scrubAtoms[i]->atom;
            MP_Support_t support = atom->support[0];
            
            
            
            int atomStart = support.pos;
            int atomEnd = atomStart + support.len;
            
            if ( !(atomEnd < bufferStart || atomStart >= bufferEnd) )
            {
                atom->build_waveform(tempBuffer);
                
                int posWithinWaveform = max(0, bufferStart - atomStart);
                int posWithinBuffer = max(0, atomStart - bufferStart);
                
                int numSamplesToCopy = support.len - max(0, bufferStart - atomStart) - max(0, atomEnd - bufferEnd);
                
                while (numSamplesToCopy > 0)
                {
                    bufferToFill.buffer->addSample(0, posWithinBuffer, (float) tempBuffer[posWithinWaveform]);
                    
                    ++posWithinWaveform;
                    ++posWithinBuffer;
                    --numSamplesToCopy;
                }
                
            }
        }
        
        nextReadPosition += bufferToFill.numSamples;
        
    }
    else
    {
        for (int n = 0; n < numSamples; ++n)
        {
            int numAtoms = 0;

            for (int i = 0; i < scrubAtoms.size(); ++i)
            {
                    
                
                ScrubAtom* scrubAtom = scrubAtoms[i];
                MP_Atom_c* atom = scrubAtom->atom;
                
                MP_Support_t* support = atom->support;
                int atomStart = support->pos;
                int atomEnd = atomStart + support->len;
                
                
                if ( nextReadPosition > atomStart && nextReadPosition < atomEnd )
                {
                    numAtoms++;
                    
                    float phase = scrubAtom->currentPhase + scrubAtom->phaseInc;
                    
                    if (phase >= 2 * M_PI)
                    {
                        phase -= 2 * M_PI;
                    }
                    else if (phase < 0)
                    {
                        phase += 2 * M_PI;
                    }
                    
                    bufferToFill.buffer->addSample(0, n, osc->getSample(phase));
                    
                    scrubAtom->currentPhase = phase;
                    
                    
                }
                
            }
            
            numAtomsCurrentlyPlaying = numAtoms;
//            DBG("Num atoms: " << numAtomsCurrentlyPlaying);
            
            
            
        }
    }
    
}

void AtomicAudioSource::prepareToPlay (int samplesPerBlockExpected,
                            double sampleRate)
{
    tempBuffer = new MP_Real_t[16384];
    
    
    
    isCurrentlyScrubbing = true;
    
    prevReadPosition = -1;
    nextReadPosition = 0;
    
    for (int i = 0; i < book->numAtoms; ++i)
    {
        
        MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)book->atom[i];
        
        
        
        float atomPhaseInc = 2 * M_PI * gabor_atom->freq;
        float atomInitialPhase = gabor_atom->phase[0];
        
        ScrubAtom* newAtom = new ScrubAtom;
        newAtom->atom = gabor_atom;
        newAtom->currentPhase = atomInitialPhase;
        newAtom->phaseInc = atomPhaseInc;
        
        scrubAtoms.add(newAtom);
    }
    
    
}