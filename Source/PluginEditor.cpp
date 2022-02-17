// PluginEditor.cpp

#include "PluginEditor.h"
#include "PluginProcessor.h"

// ==============================================================================
ResponseCurveComponent::ResponseCurveComponent(JMB3AudioProcessor& p) :
    audioProcessor(p),

    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

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

void ResponseCurveComponent::timerCallback()
{
    if (shouldShowFFTAnalysis)
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();

        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if (parametersChanged.compareAndSetBool(false, true))
    {
        DBG("params changed");
        // Update the monochain
        // updateChain();
        // repaint();
    }

    repaint();
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;
    g.fillAll(Colours::black);

    g.drawImage(background, getLocalBounds().toFloat());

    auto responseArea = getAnalysisArea();

    auto w = responseArea.getWidth();

    //auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    //auto& peak = monoChain.get<ChainPositions::Peak>();
    //auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(w);

    //for (int i = 0; i < w; ++i)
    //{
    //    double mag = 1.f;
    //    auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

    //    /*if (!monoChain.isBypassed<ChainPositions::Peak>())
    //        mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

    //    if (!monoChain.isBypassed<ChainPositions::LowCut>())
    //    {
    //        if (!lowcut.isBypassed<0>())
    //            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //        if (!lowcut.isBypassed<1>())
    //            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //        if (!lowcut.isBypassed<2>())
    //            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //        if (!lowcut.isBypassed<3>())
    //            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //    }

    //    if (!monoChain.isBypassed<ChainPositions::HighCut>())
    //    {
    //        if (!highcut.isBypassed<0>())
    //            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //        if (!highcut.isBypassed<1>())
    //            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //        if (!highcut.isBypassed<2>())
    //            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //        if (!highcut.isBypassed<3>())
    //            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    //    }*/

    //    mags[i] = Decibels::gainToDecibels(mag);
    //}

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    if (shouldShowFFTAnalysis)
    {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colour(254u, 78u, 218u)); // Darker value
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colour(224u, 176u, 255u)); // Lighter value
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }

    g.setColour(Colour(216u, 191u, 216u));
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

    //g.setColour(Colours::white);
    //g.strokePath(responseCurve, PathStrokeType(2.f));
}

void ResponseCurveComponent::resized()
{
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);

    Graphics g(background);

    Array<float> freqs
    {
        20, /*30, 40,*/ 50, 100,
        200, /*300, 400,*/ 500, 1000,
        2000, /*3000, 4000,*/ 5000, 10000,
        20000
    };

    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    Array<float> xs;
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(left + width * normX);
    }

    g.setColour(Colours::dimgrey);

    for (auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom);
    }

    Array<float> gain
    {
        -24, -12, 0, 12, 24
    };

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
    }

    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    for (int i = 0; i < freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if (f > 999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

        String str;
        if (gDb > 0)
            str << "+";
        str << gDb;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);

        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);

        g.drawFittedText(str, r, juce::Justification::centred, 1);

        str.clear();
        str << (gDb - 24.f);

        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(12);
    bounds.removeFromBottom(1);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
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
