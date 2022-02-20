// PluginEditor.cpp

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "DSP/SingleChannelSampleFifo.h"

ControlBar::ControlBar()
{
    analyzerButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(analyzerButton);
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds();

    analyzerButton.setBounds(bounds.removeFromLeft(50).withTrimmedTop(4).withTrimmedBottom(4));
}

JMB3AudioProcessorEditor::JMB3AudioProcessorEditor (JMB3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lnf);

    controlBarArea.analyzerButton.onClick = [this]()
    {
        auto shouldBeOn = controlBarArea.analyzerButton.getToggleState();
        analyzerArea.toggleAnalysisEnablement(shouldBeOn);
    };
    
    addAndMakeVisible(controlBarArea);
    addAndMakeVisible(analyzerArea);
    addAndMakeVisible(globalControlsArea);
    addAndMakeVisible(bandControlsArea);

    setSize (600, 500);

    startTimerHz(60);
}

JMB3AudioProcessorEditor::~JMB3AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void JMB3AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    g.fillAll(juce::Colours::black);
}

void JMB3AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();

    controlBarArea.setBounds(bounds.removeFromTop(32));

    bandControlsArea.setBounds(bounds.removeFromBottom(135));

    analyzerArea.setBounds(bounds.removeFromTop(225));

    globalControlsArea.setBounds(bounds);
}

void JMB3AudioProcessorEditor::timerCallback()
{
    std::vector<float> values
    {
        audioProcessor.lowBandComp.getRMSInputLevelDb(),
        audioProcessor.lowBandComp.getRMSOutputLevelDb(),
        audioProcessor.midBandComp.getRMSInputLevelDb(),
        audioProcessor.midBandComp.getRMSOutputLevelDb(),
        audioProcessor.highBandComp.getRMSInputLevelDb(),
        audioProcessor.highBandComp.getRMSOutputLevelDb()
    };

    analyzerArea.update(values);
}