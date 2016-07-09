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

int64 AtomicAudioSource::getTotalLength() const
{
    if (book != nullptr)
        return book->numSamples;
    else
        return 0;
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
        int numAtoms = 0;
        int numAtomsTooQuiet = 0;
        int numAtomsNotSupported = scrubAtoms.size();

            for (int i = 0; i < scrubAtoms.size(); ++i)
            {
                    
                
                ScrubAtom* scrubAtom = scrubAtoms[i];
                MP_Atom_c* atom = scrubAtom->atom;
                
                MP_Support_t* support = atom->support;
                int atomStart = support->pos;
                int atomEnd = atomStart + support->len;
                
                // Check if atom is supported currently
                
                if ( nextReadPosition > atomStart && nextReadPosition < atomEnd )
                {
                    
                    double amp = *atom->amp * scrubAtom->window[nextReadPosition - atomStart];
                    
                    // Check if atom could be heard
                    
                    if (Decibels::gainToDecibels(amp) >= -90)
                    {
                        numAtoms++;
                        numAtomsNotSupported--;
                    
                        // Generate output for whole buffer
                        
                        for (int n = 0; n < numSamples; ++n)
                        {

                            float phase = scrubAtom->currentPhase + scrubAtom->phaseInc;
                            
                            if (phase >= 2 * M_PI)
                                phase -= 2 * M_PI;
                            
                            bufferToFill.buffer->addSample(0, n, osc->getSample(phase) * amp);
                            
                            scrubAtom->currentPhase = phase;
                        }
                    } else {numAtomsTooQuiet++;}
                }
                
            }
            
            numAtomsCurrentlyPlaying = numAtoms;
            numAtomsCurrentlyTooQuiet = numAtomsTooQuiet;
            numAtomsCurrentlyNotSupported = numAtomsNotSupported;
        
    }
    
}

void AtomicAudioSource::prepareToPlay (int samplesPerBlockExpected,
                            double sampleRate)
{
}

void AtomicAudioSource::setBook(MP_Book_c* newBook)
{
    book = newBook;

    if (book != nullptr) {
        tempBuffer = new MP_Real_t[16384];
        
        isCurrentlyScrubbing = true;
        
        prevReadPosition = -1;
        nextReadPosition = 0;
        
        for (int i = 0; i < book->numAtoms; ++i)
        {
            
            MP_Gabor_Atom_Plugin_c* gabor_atom = (MP_Gabor_Atom_Plugin_c*)book->atom[i];
            
            unsigned char windowType = gabor_atom->windowType;
            double windowOption = gabor_atom->windowOption;
            int length = gabor_atom->support[0].len;
            double* window = new double[length];
            
            make_window(window, length, windowType, windowOption);
            
            
            
            float atomPhaseInc = 2 * M_PI * gabor_atom->freq;
            float atomInitialPhase = gabor_atom->phase[0];
            
            if (atomInitialPhase < 0) {
                atomInitialPhase += 2 * M_PI;
            }
            
            ScrubAtom* newAtom = new ScrubAtom;
            newAtom->atom = gabor_atom;
            newAtom->currentPhase = atomInitialPhase;
            newAtom->phaseInc = atomPhaseInc;
            newAtom->window = window;
            
            scrubAtoms.add(newAtom);
        }
        

    }
    
    
    
    
    
    
}