// PluginProcessor.h

#pragma once

#include <array>
#include <JuceHeader.h>

#include "DSP/CompressorBand.h"
#include "DSP/SingleChannelSampleFifo.h"

class JMB3AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    JMB3AudioProcessor();
    ~JMB3AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;

    static APVTS::ParameterLayout createParameterLayout();

    APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo{ Channel::Left };
    SingleChannelSampleFifo<BlockType> rightChannelFifo{ Channel::Right };

private:

    std::array<CompressorBand, 3> compressors;

    CompressorBand& lowBandComp = compressors[0];
    CompressorBand& midBandComp = compressors[1];
    CompressorBand& highBandComp = compressors[2];


    using LinkwitzRileyFilter = juce::dsp::LinkwitzRileyFilter<float>;
    //                  filterCutoff0 filterCutoff1
    LinkwitzRileyFilter lrLowpassOne, lrAllpassTwo,
                        lrHighpassOne, lrLowpassTwo,
                                       lrHighpassTwo;

    // LinkwitzRileyFilter invertedAllpassOne, invertedAllpassTwo;
    // juce::AudioBuffer<float> invertedAllpassBuffer;

    juce::AudioParameterFloat* lowMidCrossover{ nullptr };
    juce::AudioParameterFloat* midHighCrossover{ nullptr };

    std::array<juce::AudioBuffer<float>, 3> filterBuffers;

    juce::dsp::Gain<float> inputGain, outputGain;

    juce::AudioParameterFloat* inputGainParam{ nullptr };
    juce::AudioParameterFloat* outputGainParam{ nullptr };

    template<typename T, typename U>
    void applyGain(T& buffer, U& gain)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        gain.process(context);
    }

    void updateState();

    void splitBands(const juce::AudioBuffer<float>& inputBuffer);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JMB3AudioProcessor)
};
