/*
  ==============================================================================

    Parameters.h
    Created: 24 Jul 2016 5:36:20pm
    Author:  Alistair Barker

  ==============================================================================
*/

#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class Parameter : public ChangeBroadcaster
{
public:
    Parameter(const String& name) : paramName(name) {}
    virtual ~Parameter() {}
    void notify() { sendChangeMessage(); }
    
    
private:
    String paramName;
};

class FloatParameter : public Parameter
{
public:
    FloatParameter(const String& name, NormalisableRange<float> normalisableRange, float defaultValue) : Parameter(name), range(normalisableRange), value(defaultValue) {}
    
    NormalisableRange<float> range;
    operator float() const noexcept             { return value; }
    FloatParameter& operator= (float newValue) {value = newValue; notify(); return *this; }

private:
    float value;
};

class IntParameter : public Parameter
{
public:
    IntParameter(const String& name, Range<int> range, float defaultValue) : Parameter(name), range(range), value(defaultValue) {}
    
    Range<int> range;
    operator float() const noexcept             { return value; }
    IntParameter& operator= (float newValue) {value = newValue; notify(); return *this; }
    
private:
    float value;
};



#endif  // PARAMETERS_H_INCLUDED
