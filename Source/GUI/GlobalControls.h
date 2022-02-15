// GlobalControls.h

#pragma once

#include <JuceHeader.h>

#include "RotarySliderWithLabels.h"

struct GlobalControls : juce::Component
{
    GlobalControls(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics& g) override;

    void resized() override;

private:
    using RSWL = RotarySliderWithLabels;
    std::unique_ptr<RSWL> inputGainSlider, lowMidXoverSlider, midHighXoverSlider, outputGainSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputGainSliderAttachment,
        lowMidXoverSliderAttachment,
        midHighXoverSliderAttachment,
        outputGainSliderAttachment;
};