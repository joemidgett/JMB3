// CompressorBand.h

#pragma once

#include <JuceHeader.h>

struct CompressorBand
{
public:

    void prepareCompressor(const juce::dsp::ProcessSpec& spec);

    void updateCompressorSettings();

    void processCompressor(juce::AudioBuffer<float>& buffer);

    juce::AudioParameterFloat* attack{ nullptr };
    juce::AudioParameterFloat* release{ nullptr };
    juce::AudioParameterFloat* threshold{ nullptr };
    juce::AudioParameterChoice* ratio{ nullptr };
    juce::AudioParameterBool* bypass{ nullptr };
    juce::AudioParameterBool* mute{ nullptr };
    juce::AudioParameterBool* solo{ nullptr };

private:
    juce::dsp::Compressor<float> compressor;
};