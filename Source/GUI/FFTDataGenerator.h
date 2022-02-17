// FFTDataGenerator.h

#pragma once

#include <JuceHeader.h>

#include "../DSP/Fifo.h"
#include "Utilities.h"

template<typename BlockType>
struct FFTDataGenerator
{
    // Produces FFT data from an audio buffer
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
    {
        const auto fftSize = getFFTSize();

        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());

        // Apply windowing function to data
        window->multiplyWithWindowingTable(fftData.data(), fftSize);

        // Render FFT data
        forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());

        int numBins = (int)fftSize / 2;

        // Normalize the FFT values
        for (int i = 0; i < numBins; ++i)
        {
            fftData[i] /= (float)numBins;
        }

        // Convert FFT data to decibels
        for (int i = 0; i < numBins; ++i)
        {
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }

        fftDataFifo.push(fftData);
    }

    void changeOrder(FFTOrder newOrder)
    {
        // When you change order, recreate the window, forwardFFT, fifo, fftData
        // Also reset the fifoIndex
        // Things that need recreating should be created on the heap via std::make_unique<>

        order = newOrder;
        auto fftSize = getFFTSize();

        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);

        fftData.clear();
        fftData.resize(fftSize * 2, 0);

        fftDataFifo.prepare(fftData.size());
    }

    // ==============================================================================
    int getFFTSize() const { return 1 << order; }
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }

    // ==============================================================================
    bool getFFTData(BlockType& fftData) { return fftDataFifo.pull(fftData); }

private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;

    Fifo<BlockType> fftDataFifo;
};