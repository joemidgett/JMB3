// PluginEditor.cpp

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "DSP/SingleChannelSampleFifo.h"

// ==============================================================================
void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;

    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                monoBuffer.getReadPointer(0, size),
                monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size);

            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }

    /*
        If there are FFT data buffers to pull
            If we can pull a buffer
                Generate a path
    */
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();

    // 48000 / 2048 = 23Hz <- This is the bin width
    const auto binWidth = sampleRate / (double)fftSize;

    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }

    /*
    While there are paths that can be pulled
        Pull as many as we can
            Display the most recent path
    */

    while (pathProducer.getNumPathsAvailable())
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
}

//==============================================================================
JMB3AudioProcessorEditor::JMB3AudioProcessorEditor (JMB3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lnf);
    
    // addAndMakeVisible(controlBarArea);
    addAndMakeVisible(analyzerArea);
    addAndMakeVisible(globalControlsArea);
    addAndMakeVisible(bandControlsArea);

    setSize (600, 500);
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
