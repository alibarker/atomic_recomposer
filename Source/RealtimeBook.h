/*
  ==============================================================================

    RealtimeBook.h
    Created: 20 Jul 2016 3:23:24pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef REALTIMEBOOK_H_INCLUDED
#define REALTIMEBOOK_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "mptk.h"

// Sttucts that contain the information needed for realtime playback

struct RealtimeAtom
{
    double* currentPhase;
    double phaseInc;
    double originalPhaseInc;
    MP_Atom_c* atom;
};

struct RealtimeBook
{
    MP_Book_c* book;
    OwnedArray<RealtimeAtom> realtimeAtoms;
};



#endif  // REALTIMEBOOK_H_INCLUDED
