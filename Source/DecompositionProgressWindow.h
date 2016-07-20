/*
  ==============================================================================

    DecompositionProgressWindow.h
    Created: 19 Jul 2016 2:11:26pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef DECOMPOSITIONPROGRESSWINDOW_H_INCLUDED
#define DECOMPOSITIONPROGRESSWINDOW_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class DecompositionProgressWindow : public ThreadWithProgressWindow
{
public:
    DecompositionProgressWindow() : ThreadWithProgressWindow("Decomposition Progress", true, true)
    {
        setStatusMessage ("Starting...");

    }
    
    void run() override
    {
        
    }
    
    void threadComplete (bool userPressedCancel) override
    {
        if (userPressedCancel)
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Progress window",
                                              "You pressed cancel!");
        }
        else
        {
            // thread finished normally..
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Progress window",
                                              "Thread finished ok!");
        }
        
        // ..and clean up by deleting our thread object..
        delete this;
    }

    
    
private:
    
}


#endif  // DECOMPOSITIONPROGRESSWINDOW_H_INCLUDED
