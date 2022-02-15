// CompressorBand.cpp

#include "CompressorBand.h"

void CompressorBand::prepareCompressor(const juce::dsp::ProcessSpec& spec)
{
    compressor.prepare(spec);
}

void CompressorBand::updateCompressorSettings()
{
    compressor.setAttack(attack->get());
    compressor.setRelease(release->get());
    compressor.setThreshold(threshold->get());
    compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue());
}

void CompressorBand::processCompressor(juce::AudioBuffer<float>& buffer)
{
    auto audioBlock = juce::dsp::AudioBlock<float>(buffer);

    auto processContextReplacing = juce::dsp::ProcessContextReplacing<float>(audioBlock);

    processContextReplacing.isBypassed = bypass->get();

    compressor.process(processContextReplacing);
}