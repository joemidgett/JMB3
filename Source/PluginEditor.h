// PluginEditor.h

#pragma once

#include <JuceHeader.h>

#include "GUI/AnalyzerPathGenerator.h"
#include "GUI/CompressorBandControls.h"
#include "GUI/CustomButtons.h"
#include "GUI/FFTDataGenerator.h"
#include "GUI/GlobalControls.h"
#include "GUI/LookAndFeel.h"
#include "GUI/PathProducer.h"
#include "PluginProcessor.h"
#include "GUI/SpectrumAnalyzer.h"
#include "GUI/UtilityComponents.h"

struct ControlBar : juce::Component
{
    ControlBar();

    void resized() override;

    AnalyzerButton analyzerButton;
};

class JMB3AudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    JMB3AudioProcessorEditor (JMB3AudioProcessor&);
    ~JMB3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    LookAndFeel lnf;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JMB3AudioProcessor& audioProcessor;

    ControlBar controlBarArea;

    GlobalControls globalControlsArea { audioProcessor.apvts };

    CompressorBandControls bandControlsArea { audioProcessor.apvts };

    SpectrumAnalyzer analyzerArea { audioProcessor };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JMB3AudioProcessorEditor)
};