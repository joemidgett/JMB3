// CompressorBand.h

#pragma once

#include <JuceHeader.h>

#include "../GUI/Utilities.h"

struct CompressorBand
{
public:

    void prepareCompressor(const juce::dsp::ProcessSpec& spec);

    void updateCompressorSettings();

    void processCompressor(juce::AudioBuffer<float>& buffer);

    float getRMSInputLevelDb() const { return rmsInputLevelDb; }
    float getRMSOutputLevelDb() const { return rmsOutputLevelDb; }

    juce::AudioParameterFloat* attack{ nullptr };
    juce::AudioParameterFloat* release{ nullptr };
    juce::AudioParameterFloat* threshold{ nullptr };
    juce::AudioParameterChoice* ratio{ nullptr };
    juce::AudioParameterBool* bypass{ nullptr };
    juce::AudioParameterBool* mute{ nullptr };
    juce::AudioParameterBool* solo{ nullptr };

private:
    juce::dsp::Compressor<float> compressor;

    std::atomic<float> rmsInputLevelDb { NEGATIVE_INFINITY };
    std::atomic<float> rmsOutputLevelDb { NEGATIVE_INFINITY };

    template<typename T>
    float computeRMSLevel(const T& buffer)
    {
        int numChannels = static_cast<int>(buffer.getNumChannels());
        int numSamples = static_cast<int>(buffer.getNumSamples());
        auto rms = 0.f;
        for (int chan = 0; chan < numChannels; ++chan)
        {
            rms += buffer.getRMSLevel(chan, 0, numSamples);
        }

        rms /= static_cast<float>(numChannels);
        return rms;
    }
};