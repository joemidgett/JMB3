/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

Placeholder::Placeholder()
{
    juce::Random randomValue;
    customColor = juce::Colour(randomValue.nextInt(255), randomValue.nextInt(255), randomValue.nextInt(255));
}

//==============================================================================
JMB3AudioProcessorEditor::JMB3AudioProcessorEditor (JMB3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(controlBarArea);
    addAndMakeVisible(analyzerArea);
    addAndMakeVisible(globalControlsArea);
    addAndMakeVisible(bandControlsArea);

    setSize (600, 500);
}

JMB3AudioProcessorEditor::~JMB3AudioProcessorEditor()
{
}

//==============================================================================
void JMB3AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void JMB3AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();

    controlBarArea.setBounds(bounds.removeFromTop(32));

    controlBarArea.setBounds(bounds.removeFromBottom(135));

    analyzerArea.setBounds(bounds.removeFromTop(225));

    globalControlsArea.setBounds(bounds);
}
