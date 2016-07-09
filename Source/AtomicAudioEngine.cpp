/*
  ==============================================================================

    AtomicAudioEngine.cpp
    Created: 8 Jul 2016 12:29:40pm
    Author:  Alistair Barker

  ==============================================================================
*/

#include "AtomicAudioEngine.h"

AtomicAudioEngine::AtomicAudioEngine(int wivigramWidth, int wivigramHeight) : Thread("Decomposition")
{
    wivigram = new Wivigram(wivigramWidth, wivigramHeight);
}

void AtomicAudioEngine::updateWivigram()
{
    MP_TF_Map_c* map = new MP_TF_Map_c(wivigram->getWidth(), wivigram->getHeight(), book->numChans, 0, book->numSamples, 0.0, 0.5);
    book->add_to_tfmap(map, MP_TFMAP_PSEUDO_WIGNER, NULL);
    wivigram->updateWivigram(map);
    sendChangeMessage();
    
}

void AtomicAudioEngine::startDecomposition()
{
    
    transportSource.stop();
    transportSource.releaseResources();
    
    isReadyToPlay = false;
    startTimerHz(50);
    startThread();
  
    
}

void AtomicAudioEngine::run()
{
    std::cout << "running MPTK\n";
    
    ScopedPointer<MP_Dict_c>			dict;
    ScopedPointer<MP_Signal_c>			sig;
    ScopedPointer<MP_Mpd_Core_c>		mpdCore;
    
    /* Load the MPTK environment */
    if(! (MPTK_Env_c::get_env()->load_environment_if_needed("/usr/local/mptk/path.xml")) )
    {
        return;
    }
    
    /* Read the dictionary */
    
    dict = MP_Dict_c::read_from_xml_file( dictionary.getFullPathName().toRawUTF8());
    if ( dict == NULL )
    {
        return;
    }
    if ( dict->numBlocks == 0 )
    {
        return;
    }
    
    
    
    /* Load the signal */
    sig = MP_Signal_c::init( signal.getFullPathName().toRawUTF8() );
    if ( sig == NULL )
    {
        return;
    }
    
    /* Create the book */
    book = MP_Book_c::create(sig->numChans, sig->numSamples, sig->sampleRate );
    if ( book == NULL )
    {
        return;
    }
    
    /* Set up the core */
    mpdCore = MP_Mpd_Core_c::create( sig, book, dict );
    mpdCore->set_iter_condition( numIterations );
    mpdCore->run();
    
    
    ScopedPointer<AtomicAudioSource> atomicSource = new AtomicAudioSource();
    atomicSource->setBook(book);
    transportSource.setSource(atomicSource.release());
    
    isReadyToPlay = true;
    
}

void AtomicAudioEngine::triggerDecomposition(File dict, File sig, int numIter)
{
    dictionary = File(dict);
    signal = File(sig);
    numIterations = numIter;
    startDecomposition();
    
}

void AtomicAudioEngine::setTransportPosition(float posAsPercentage)
{
    if (transportSource.isPlaying())
    {
        int length = transportSource.getTotalLength();
        int64 newPos = min ( (int) round(posAsPercentage*length), length -1) ;
        transportSource.setNextReadPosition( newPos );
    }
}


float AtomicAudioEngine::getTransportPosition()
{
    float scrubPos = 0;
    if (transportSource.isPlaying())
    {
        int length = transportSource.getTotalLength();
        int64 pos = transportSource.getNextReadPosition();
        scrubPos = (float) pos / (float) length;
    }
    return scrubPos;
}