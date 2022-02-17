// SpectrumAnalyzer.h

#pragma once

#include <JuceHeader.h>

#include "PathProducer.h"
#include "SpectrumAnalyzer.h"

struct SpectrumAnalyzer : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    SpectrumAnalyzer(JMB3AudioProcessor&);
    ~SpectrumAnalyzer();

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;

    void paint(juce::Graphics& g) override;

    void resized() override;

    void toggleAnalysisEnablement(bool enabled)
    {
        shouldShowFFTAnalysis = enabled;
    }

private:
    JMB3AudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };

    juce::Image background;

    juce::Rectangle<int> getRenderArea();

    juce::Rectangle<int> getAnalysisArea();

    PathProducer leftPathProducer, rightPathProducer;

    bool shouldShowFFTAnalysis = true;
};