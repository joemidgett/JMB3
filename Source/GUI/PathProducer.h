// PathProducer.h

#pragma once

#include <JuceHeader.h>

#include "AnalyzerPathGenerator.h"
#include "FFTDataGenerator.h"
#include "../PluginProcessor.h"
#include "../DSP/SingleChannelSampleFifo.h"
#include "Utilities.h"

struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<JMB3AudioProcessor::BlockType>& scsf) :
        leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }

    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return leftChannelFFTPath; }

    void updateNegativeInfinity(float nf) { negativeInfinity = nf; }

private:
    SingleChannelSampleFifo<JMB3AudioProcessor::BlockType>* leftChannelFifo;

    juce::AudioBuffer<float> monoBuffer;

    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;

    AnalyzerPathGenerator<juce::Path> pathProducer;

    juce::Path leftChannelFFTPath;

    float negativeInfinity { -48.f };
};