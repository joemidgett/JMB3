// PluginEditor.cpp

#include "DSP/Params.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "DSP/SingleChannelSampleFifo.h"

ControlBar::ControlBar()
{
    analyzerButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    
    addAndMakeVisible(analyzerButton);
    addAndMakeVisible(globalBypassButton);
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds();

    analyzerButton.setBounds(bounds.removeFromLeft(50).withTrimmedTop(4).withTrimmedBottom(4));
    globalBypassButton.setBounds(bounds.removeFromRight(60).withTrimmedTop(2).withTrimmedBottom(2));
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

    controlBarArea.globalBypassButton.onClick = [this]() 
    {
        toggleGlobalBypassState();
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

    updateGlobalBypassButton();
}

void JMB3AudioProcessorEditor::updateGlobalBypassButton()
{
    auto params = getBypassParams();

    bool allBandsAreBypassed = std::all_of(params.begin(), params.end(),
        [](const auto& param){ return param->get(); });

    controlBarArea.globalBypassButton.setToggleState(allBandsAreBypassed, juce::NotificationType::dontSendNotification);
}

void JMB3AudioProcessorEditor::toggleGlobalBypassState()
{
    auto shouldEnableEverything = !controlBarArea.globalBypassButton.getToggleState();

    auto params = getBypassParams();

    auto bypassParamHelper = [](auto* param, bool shouldBeBypassed)
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost(shouldBeBypassed ? 1.f : 0.f);
        param->endChangeGesture();
    };

    for (auto* param : params)
    {
        bypassParamHelper(param, !shouldEnableEverything);
    }

    bandControlsArea.toggleAllBands(!shouldEnableEverything);
}

std::array<juce::AudioParameterBool*, 3> JMB3AudioProcessorEditor::getBypassParams()
{
    using namespace Params;
    using namespace juce;
    const auto& params = Params::getParams();
    auto& apvts = audioProcessor.apvts;

    auto boolHelper = [&apvts, &params](const auto& paramName)
    {
        auto param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);

        return param;
    };

    auto* lowBypassParam = boolHelper(Names::Bypassed_Low_Band);
    auto* midBypassParam = boolHelper(Names::Bypassed_Mid_Band);
    auto* highBypassParam = boolHelper(Names::Bypassed_High_Band);

    return
    {
        lowBypassParam,
        midBypassParam,
        highBypassParam
    };
}