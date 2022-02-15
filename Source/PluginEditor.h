// PluginEditor.h

#pragma once

#include <JuceHeader.h>

#include "GUI/CompressorBandControls.h"
#include "GUI/GlobalControls.h"
#include "GUI/LookAndFeel.h"
#include "PluginProcessor.h"
#include "GUI/UtilityComponents.h"

class JMB3AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    JMB3AudioProcessorEditor (JMB3AudioProcessor&);
    ~JMB3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LookAndFeel lnf;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JMB3AudioProcessor& audioProcessor;

    Placeholder controlBarArea, analyzerArea;

    GlobalControls globalControlsArea { audioProcessor.apvts };

    CompressorBandControls bandControlsArea { audioProcessor.apvts };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JMB3AudioProcessorEditor)
};