// UtilityComponents.cpp

#include "UtilityComponents.h"

Placeholder::Placeholder()
{
    juce::Random randomValue;
    customColor = juce::Colour(randomValue.nextInt(255), randomValue.nextInt(255), randomValue.nextInt(255));
}

void Placeholder::paint(juce::Graphics& g)
{
    g.fillAll(customColor);
}

//==============================================================================
RotarySlider::RotarySlider() :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::NoTextBox)
{ }