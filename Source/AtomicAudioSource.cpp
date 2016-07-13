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
    
    prevReadPosition = -1;
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
    
    bufferToFill.buffer->clear();
    
//    int bufferStart = nextReadPosition;
//    int bufferEnd = nextReadPosition + numSamples;
    
    if (currentBleedValue != engine->getBleedValue())
    {
        currentBleedValue = engine->getBleedValue();
        updateBleed();
    }
    
//    if (!isCurrentlyScrubbing && isCurrentlyPlaying)
//    {
//        for (int i = 0; i < engine->scrubAtoms.size(); ++i)
//        {
//            MP_Atom_c* atom = engine->scrubAtoms[i]->atom;
//            MP_Support_t support = atom->support[0];
//            
//            
//            
//            int atomStart = support.pos;
//            int atomEnd = atomStart + support.len;
//            
//            if ( !(atomEnd < bufferStart || atomStart >= bufferEnd) )
//            {
//                atom->build_waveform(tempBuffer);
//                
//                int posWithinWaveform = max(0, bufferStart - atomStart);
//                int posWithinBuffer = max(0, atomStart - bufferStart);
//                
//                int numSamplesToCopy = support.len - max(0, bufferStart - atomStart) - max(0, atomEnd - bufferEnd);
//                
//                while (numSamplesToCopy > 0)
//                {
//                    bufferToFill.buffer->addSample(0, posWithinBuffer, (float) tempBuffer[posWithinWaveform]);
//                    
//                    ++posWithinWaveform;
//                    ++posWithinBuffer;
//                    --numSamplesToCopy;
//                }
//                
//            }
//        }
//        
//        nextReadPosition += bufferToFill.numSamples;
//        
//    }
//    else
//    if (isCurrentlyRunning)
//    {
        int numAtoms = 0;
        int numAtomsTooQuiet = 0;
        int numAtomsNotSupported = engine->scrubAtoms.size();

            for (int i = 0; i < engine->scrubAtoms.size(); ++i)
            {
                    
                
                AtomicAudioEngine::ScrubAtom* scrubAtom = engine->scrubAtoms[i];
                MP_Atom_c* atom = scrubAtom->atom;
                
                MP_Support_t* support = atom->support;
                int atomStart = support->pos;
                int atomEnd = atomStart + support->len;
                
                // Check if atom is supported currently
                
                if ( nextReadPosition > atomStart && nextReadPosition < atomEnd )
                {
                    // Check if atom could be heard
                    
                    if (Decibels::gainToDecibels(*atom->amp) >= -90)
                    {
                        // generate window
                    
                        double window[bufferToFill.numSamples];
                        
                        double amp = *atom->amp;
                        
                        if (isCurrentlyScrubbing)
                        {
                            double currentWindowAmp = scrubAtom->window[nextReadPosition - atomStart];
                            
                            for (int n = 0; n < numSamples; ++n)
                                window[n] = amp * currentWindowAmp;
                        }
                        else
                        {
                            for (int n = 0; n < numSamples; ++n)
                            {
                                double value;
                               
                                int pos = nextReadPosition - atomStart + n;
                                
                                if (pos < 0 || pos >= atom->support->len)
                                    value = 0.0;
                                else
                                    value = scrubAtom->window[pos];
                                
                                window[n] = value * amp;
                            }

                        }
                    
                    
                        numAtoms++;
                        numAtomsNotSupported--;
                        
                        // Generate output for whole buffer
                        
                        int start = max((int) (atomStart - nextReadPosition), 0);
                        int end = max((int) (nextReadPosition - atomEnd), numSamples);

                        for (int n = start; n < end; ++n)
                        {

                            float phase = scrubAtom->currentPhase + scrubAtom->phaseInc;
                            
                            if (phase >= 2 * M_PI)
                                phase -= 2 * M_PI;
                            
                            double output = osc->getSample(phase) * window[n];
                            
                            bufferToFill.buffer->addSample(0, n, output);
                            
                            
                            scrubAtom->currentPhase = phase;
                        }
                    } else {numAtomsTooQuiet++;}
                }
                
            }
    
    
            numAtomsCurrentlyPlaying = numAtoms;
            numAtomsCurrentlyTooQuiet = numAtomsTooQuiet;
            numAtomsCurrentlyNotSupported = numAtomsNotSupported;
        
        if (!isCurrentlyScrubbing) {
            nextReadPosition += bufferToFill.numSamples;
            
        }
    
//    }
    
}

void AtomicAudioSource::prepareToPlay (int samplesPerBlockExpected,
                            double sampleRate)
{
    tempBuffer = new MP_Real_t[16834];

}

void AtomicAudioSource::updateBleed()
{
    {
        
//        ScopedWriteLock swl (engine->bookLock);
        
        for (int i = 0; i < engine->book->numAtoms; ++i)
        {
            MP_Atom_c* atom = engine->book->atom[i];
            
            int originalLength = engine->scrubAtoms[i]->originalSupport.len;
            int originalStart = engine->scrubAtoms[i]->originalSupport.pos;
            
            int newLength = originalLength * currentBleedValue;
            int newStart = originalStart + round((originalLength - newLength) / 2.0);
            
            atom->support[0].len = newLength;
            atom->support[0].pos = newStart;
        }
    }
    engine->updateWivigram();
}