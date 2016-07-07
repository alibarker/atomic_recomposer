/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/


#include "MainComponent.h"


//==============================================================================
MainContentComponent::MainContentComponent()
{
    setSize (1000, 600);
    
    
    button_decomp = new TextButton("Decompose");
    button_decomp->setBounds (1, 1, 144, 32);
    addAndMakeVisible (button_decomp);
    button_decomp->addListener (this);
    
    label_num_iterations = new Label();
    label_num_iterations->setBounds(1, 35, 80, 20);
    label_num_iterations->setText("Iterations",dontSendNotification);
    addAndMakeVisible(label_num_iterations);
    
    text_editor_num_iterations = new TextEditor();
    text_editor_num_iterations->setText("1000");
    text_editor_num_iterations->setMultiLine(false);
    text_editor_num_iterations->setBounds(70, 35, 50, 20);
    addAndMakeVisible (text_editor_num_iterations);
    
    wivigram = new WivigramComponent(getWidth() - 1, getHeight() - 61, &transportSource);
    addAndMakeVisible(wivigram);
    wivigram->setBounds(1, 60, getWidth() - 1, getHeight() - 61);
    
    numAtoms = new Label();
    numAtoms->setBounds(160, 1, 100, 20);
    addAndMakeVisible(numAtoms);
    
    
    setAudioChannels (0, 2);

}

MainContentComponent::~MainContentComponent()
{
    button_decomp = 0;
    label_num_iterations = 0;
    text_editor_num_iterations = 0;
    shutdownAudio();
    
}

void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour (Colours::whitesmoke));

    g.setFont (Font (16.0f));
    g.setColour (Colours::black);
    g.drawText ("MainContentComponent!", getLocalBounds(), Justification::centred, true);
}

void MainContentComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

float estimateAnalyticIP(MP_Atom_c* atom1, MP_Atom_c* atom2)
{
    float posDiff = fabs(atom2->get_field(MP_POS_PROP, 0) - atom1->get_field(MP_POS_PROP, 0));
    float freqDiff = fabs(atom2->get_field(MP_FREQ_PROP, 0) - atom1->get_field(MP_FREQ_PROP, 0));
    
    float timeSpread1 = atom1->get_field(MP_LEN_PROP, 0);
    float timeSpread2 = atom2->get_field(MP_LEN_PROP, 0);
    
    float averageTimeSpread = (timeSpread1 + timeSpread2) / 2.0;
    float averageFreqSpread = (1.0/timeSpread1 + 1.0/timeSpread2)/4.0;

    if (posDiff <= averageTimeSpread && fabs(freqDiff) <= averageFreqSpread)
    {
        float result = (posDiff - averageTimeSpread) * (fabs(freqDiff) - averageFreqSpread);
        return result;
    }
    return 0.0;
}


void MainContentComponent::buttonClicked (Button* buttonThatWasClicked)
{
    std::cout << "running MPTK\n";
    
    MP_Dict_c			*dict = NULL;
	MP_Signal_c			*sig = NULL;
	MP_Book_c			*book = NULL;
	MP_Mpd_Core_c		*mpdCore = NULL;
    
    /* Load the MPTK environment */
	if(! (MPTK_Env_c::get_env()->load_environment_if_needed("/usr/local/mptk/path.xml")) )
	{
		exit(0);
	}
    
    /* Read the dictionary */
    
    dict = MP_Dict_c::read_from_xml_file("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/dictGabor_original.xml");
    if ( dict == NULL )
    {
		exit( 0 );
    }
	if ( dict->numBlocks == 0 )
	{
        exit( 0 );
    }
    
    
    
    /* Load the signal */
    sig = MP_Signal_c::init( "/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/glock2.wav" );
	if ( sig == NULL )
    {
		exit( 0 );
    }

    /* Create the book */
    book = MP_Book_c::create(sig->numChans, sig->numSamples, sig->sampleRate );
	if ( book == NULL )
    {
		exit( 0 );
    }
    
    /* Set up the core */
    mpdCore = MP_Mpd_Core_c::create( sig, book, dict );
    mpdCore->set_iter_condition( text_editor_num_iterations->getText().getIntValue() );
    mpdCore->run();
    
    /* Generate Mask */
    MP_Mask_c* isLongMask = MP_Mask_c::init(book->numAtoms);
    MP_Mask_c* isShortMask = MP_Mask_c::init(book->numAtoms);
    
    MP_Book_c* shortBook = MP_Book_c::create();
    MP_Book_c* longBook = MP_Book_c::create();

    int numAtoms = book->numAtoms;
    
    for (int i = 0; i < numAtoms; ++i)
    {
        MP_Atom_c* atom = book->atom[i];
    
        if (atom->get_field(MP_LEN_PROP, 0) >= 4096)
        {
            isLongMask->set_true(i);
            isShortMask->set_false(i);
            
            longBook->append(atom);
            
        }
        else
        {
            isLongMask->set_false(i);
            isShortMask->set_true(i);
            
            shortBook->append(atom);

        }
    
    }
    
    /* TUNE */
    
//    float fundamental = 426.0 / book->sampleRate;
//
//    for (int i = 0; i < book->numAtoms; ++i) {
//        
//        float incomingFreq = book->atom[i]->get_field(MP_FREQ_PROP, 0);
//        
//        float outgoingFreq = max(round(incomingFreq/fundamental) * fundamental, fundamental);
//        
//        MP_Gabor_Atom_Plugin_c* at = NULL;
//        at = (MP_Gabor_Atom_Plugin_c*)book->atom[i];
//
//        at->freq = outgoingFreq;
//        
//        DBG("Old Freq:\t" << incomingFreq << "\t\tNew Freq:\t" << outgoingFreq);
//        
//    }
//    


    /* Generate Approximant */
    MP_Signal_c			*approxComplete = NULL;
    MP_Signal_c			*approxLong = NULL;
    MP_Signal_c			*approxShort = NULL;
    
    approxComplete = MP_Signal_c::init(sig->numChans, sig->numSamples, sig->sampleRate);
    approxLong = MP_Signal_c::init(sig->numChans, sig->numSamples, sig->sampleRate);
    approxShort = MP_Signal_c::init(sig->numChans, sig->numSamples, sig->sampleRate);
    
    book->substract_add(NULL, approxComplete, NULL);
    book->substract_add(NULL, approxLong, isLongMask);
    book->substract_add(NULL, approxShort, isShortMask);

    approxComplete->wavwrite("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/approx.wav");
    approxLong->wavwrite("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/approxLong.wav");
    approxShort->wavwrite("/Users/alibarker89/Dropbox/QMUL/Final Project/Code/mpdgui/Data/approxShort.wav");

    MP_TF_Map_c* map = new MP_TF_Map_c(wivigram->getWidth(), wivigram->getHeight(), book->numChans, 0, book->numSamples, 0.0, 0.5);

    book->add_to_tfmap(map, MP_TFMAP_PSEUDO_WIGNER, NULL);
    
    wivigram->updateWiviGram(map, NULL);
    
    ScopedPointer<AtomicAudioSource> newSource = new AtomicAudioSource();
    
    if (newSource != nullptr)
    {
        newSource->setBook(book);
        
        
        transportSource.setSource(newSource);
        
        atomicSource = newSource.release();

    }
    
    DBG("STARTING");
    
    transportSource.start();
    
    startTimerHz(30);

    
    
//    
//    MP_Book_c* molecule1book = MP_Book_c::create(book->numChans, book->numSamples, book->sampleRate);
//    MP_Book_c* molecule2book = MP_Book_c::create(book->numChans, book->numSamples, book->sampleRate);
//
//    
//    // CREATE SIMILARITY MATRIX FOR SHORT
//    
//    const int numShortAtoms = shortBook->numAtoms;
//    
//    bool* similarityMatrix;
//    
//    similarityMatrix = (bool*) calloc(numShortAtoms * numShortAtoms, sizeof(bool));
//    
//    float timeDistanceLimit = 2000;
//    
//    printf("Short similirty Matrix:\n");
//    
//    for (int i = 0; i < numShortAtoms; ++i)
//    {
//        int k = 0;
//        while (k < i) {
//            ++k;
//            printf(" ");
//        }
//        
//        for (int j = i; j < numShortAtoms; ++j)
//        {
//            if (abs(shortBook->atom[i]->get_field(MP_POS_PROP, 0) - shortBook->atom[j]->get_field(MP_POS_PROP, 0)) <= timeDistanceLimit )
//            {
//                similarityMatrix[i * numShortAtoms + j] = true;
//                printf("1");
//            }
//            else
//            {
//                similarityMatrix[i * numShortAtoms + j] = false;
//                printf("0");
//            }
//        }
//        printf("\n");
//    }
//    
//    Array<int> molecule1;
//    
//    molecule1.add(0);
//    
//    // FIRST ITERATION
//        for (int j = 1; j < numShortAtoms; ++j)
//        {
//            if (similarityMatrix[0*numShortAtoms + j])
//            {
//                molecule1.add(j);
//            }
//        }
//    
//    for (int i = 1; i < numShortAtoms; ++i)
//    {
//        if (molecule1.contains(i))
//        {
//            for (int j = i + 1; j < numShortAtoms; ++j)
//            {
//                if (similarityMatrix[i * numShortAtoms + j])
//                {
//                    molecule1.add(j);
//                }
//            }
//        }
//    }
//    
//    // PRINT
//    
//    printf("Molecule 1 contains: ");
//    
//    for (int i = 0; i < numShortAtoms; ++i)
//    {
//        if(molecule1.contains(i))
//        {
//            printf("%d ", i);
//            molecule1book->append(shortBook->atom[i]);
//        }
//    }
//    
//    printf("\n");
//    
//    
//    
//    
//    
//    
//    // CREATE SIMILARITY MATRIX FOR Long
//    
//    printf("Long similirty Matrix:\n");
//
//    
//    const int numLongAtoms = longBook->numAtoms;
//    
//    bool* similarityMatrixLong;
//    
//    similarityMatrixLong = (bool*) calloc(numLongAtoms * numLongAtoms, sizeof(bool));
//    
//    float epsilon = 0.002;
//    
//    for (int i = 0; i < numLongAtoms; ++i)
//    {
//        int k = 0;
//        while (k < i) {
//            ++k;
//            printf(" ");
//        }
//
//        for (int j = i; j < numLongAtoms; ++j)
//        {
//            MP_Atom_c* atom1 = longBook->atom[i];
//            MP_Atom_c* atom2 = longBook->atom[j];
//
//            
//            float similarity = estimateAnalyticIP(atom1, atom2);
//            
////            DBG("PosDiff " << posDiff << " freqDiff " << freqDiff << " timeSpread1 " << timeSpread1 << " timeSpread2 " << timeSpread2 << " OUTPUT: " << similarity);
//            
//            if (similarity >= epsilon)
//            {
//                similarityMatrixLong[i * numLongAtoms + j] = true;
//                printf("1");
//            }
//            else
//            {
//                similarityMatrixLong[i * numLongAtoms + j] = false;
//                printf("0");
//            }
//        }
//        printf("\n");
//    }
//
//    Array<int> longmolecule1;
//
//    // FIRST ITERATION
//    for (int j = 1; j < numLongAtoms; ++j)
//    {
//        if (similarityMatrixLong[0*numShortAtoms + j])
//        {
//            longmolecule1.add(j);
//     
//        }
//    }
//
//    for (int i = 1; i < numLongAtoms; ++i)
//    {
//        if (longmolecule1.contains(i))
//        {
//            for (int j = i + 1; j < numLongAtoms; ++j)
//            {
//                if (similarityMatrixLong[i * numLongAtoms + j])
//                {
//                    longmolecule1.add(j);
//                }
//            }
//        }
//    }
//
//    // PRINT
//    
//
//    printf("Long Molecule 1 contains: ");
//
//    for (int i = 0; i < numLongAtoms; ++i)
//    {
//        if(longmolecule1.contains(i))
//        {
//            printf("%d ", i);
//            molecule2book->append(longBook->atom[i]);
//
//        }
//    }
//
//    printf("\n");
//
//    
//    /* Generate Maps */
//    
//    
//    MP_TF_Map_c* map1 = new MP_TF_Map_c(wivigram->getWidth(), wivigram->getHeight(), book->numChans, 0, book->numSamples, 0.0, 0.5);
//    MP_TF_Map_c* map2 = new MP_TF_Map_c(wivigram->getWidth(), wivigram->getHeight(), book->numChans, 0, book->numSamples, 0.0, 0.5);
//
//    
//    
//    
//    char tfMapType = MP_TFMAP_PSEUDO_WIGNER;
//    
//    shortBook->add_to_tfmap(map1, tfMapType, NULL);
//    longBook->add_to_tfmap(map2, tfMapType, NULL);
//    
//     Draw maps 
//    wivigram->updateWiviGram(map1, map2);
//    
    if(sig)
		delete sig;
	if(mpdCore)
		delete mpdCore;
	if(dict)
		delete dict;
//	if(book)
//		delete book;

}