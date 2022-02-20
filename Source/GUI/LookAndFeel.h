// LookAndFeel.h

#pragma once

#include <JuceHeader.h>

#define USE_LIVE_CONSTANT false;

#if USE_LIVE_CONSTANT
#define colorHelper(c) JUCE_LIVE_CONSTANT(c);
#else
#define colorHelper(c) c;
#endif

namespace ColorScheme
{
    inline juce::Colour getGainReductionColor() { return colorHelper(juce::Colour(0xff453850)); }
    inline juce::Colour getInputSignalColor() { return colorHelper(juce::Colour(0xfffb02d1)); }
    inline juce::Colour getOutputSignalColor() { return colorHelper(juce::Colour(0xff0bc60c)); }
    inline juce::Colour getSliderFillColor() { return colorHelper(juce::Colour(0xff3628f0)); }
    inline juce::Colour getOrangeBorderColor() { return colorHelper(juce::Colour(0xff0bc60c)); }
    inline juce::Colour getSliderRangeTextColor() { return colorHelper(juce::Colour(0xff0bc60c)); }
    inline juce::Colour getSliderBorderColor() { return colorHelper(juce::Colour(0xffd4bce5)); }
    inline juce::Colour getThresholdColor() { return colorHelper(juce::Colour(0xffd4bce5)); }
    inline juce::Colour getModuleBorderColor() { return colorHelper(juce::Colour(0xff1e1526)); }
    inline juce::Colour getTitleColor() { return colorHelper(juce::Colour(0xffd4bce5)); }
    inline juce::Colour getAnalyzerGridColor() { return colorHelper(juce::Colour(0xff262626)); }
    inline juce::Colour getTickColor() { return colorHelper(juce::Colour(0xff313131)); }
    inline juce::Colour getMeterLineColor() { return colorHelper(juce::Colour(0xff3c3c3c)); }
    inline juce::Colour getScaleTextColor() { return juce::Colours::lightgrey; }
}

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle,
        float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
};