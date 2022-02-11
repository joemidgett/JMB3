/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JMB3AudioProcessor::JMB3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace Params;

    const auto& params = getParams();

    auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    floatHelper(compressor.attack, Names::Attack_Low_Band);
    floatHelper(compressor.release, Names::Release_Low_Band);
    floatHelper(compressor.threshold, Names::Threshold_Low_Band);

    auto choiceHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    choiceHelper(compressor.ratio, Names::Ratio_Low_Band);

    auto boolHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    boolHelper(compressor.bypass, Names::Bypassed_Low_Band);

    floatHelper(lowCrossover, Names::Low_Mid_Crossover_Freq);

    lrLowpass.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    lrHighpass.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    lrAllpass.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
}

JMB3AudioProcessor::~JMB3AudioProcessor()
{
}

//==============================================================================
const juce::String JMB3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JMB3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JMB3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JMB3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JMB3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JMB3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JMB3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void JMB3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String JMB3AudioProcessor::getProgramName (int index)
{
    return {};
}

void JMB3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void JMB3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec processSpec;
    processSpec.maximumBlockSize = samplesPerBlock;
    processSpec.numChannels = getTotalNumOutputChannels();
    processSpec.sampleRate = sampleRate;

    compressor.prepareCompressor(processSpec);

    lrLowpass.prepare(processSpec);
    lrHighpass.prepare(processSpec);
    lrAllpass.prepare(processSpec);

    lrAllpassBuffer.setSize(processSpec.numChannels, samplesPerBlock);

    for (auto& buffer : filterBuffers)
    {
        buffer.setSize(processSpec.numChannels, samplesPerBlock);
    }
}

void JMB3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JMB3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void JMB3AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //compressor.updateCompressorSettings();
    //compressor.processCompressor(buffer);

    for (auto& filterBuffer : filterBuffers)
    {
        filterBuffer = buffer;
    }

    auto lowCrossoverCutoff = lowCrossover->get();
    lrLowpass.setCutoffFrequency(lowCrossoverCutoff);
    lrHighpass.setCutoffFrequency(lowCrossoverCutoff);
    lrAllpass.setCutoffFrequency(lowCrossoverCutoff);

    auto filterBufferZeroBlock = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto filterBufferOneBlock = juce::dsp::AudioBlock<float>(filterBuffers[1]);

    auto filterBufferZeroContext = juce::dsp::ProcessContextReplacing<float>(filterBufferZeroBlock);
    auto filterBufferOneContext = juce::dsp::ProcessContextReplacing<float>(filterBufferOneBlock);

    lrLowpass.process(filterBufferZeroContext);
    lrHighpass.process(filterBufferOneContext);

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    //if (compressor.bypass->get())
    //    return;
    lrAllpassBuffer = buffer;

    auto allpassBlock = juce::dsp::AudioBlock<float>(lrAllpassBuffer);
    auto allpassContext = juce::dsp::ProcessContextReplacing<float>(allpassBlock);

    lrAllpass.process(allpassContext);

    buffer.clear();

    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
    {
        for (auto i = 0; i < nc; ++i)
        {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        }
    };

    //if (!compressor.bypass->get())
    //{
    //    addFilterBand(buffer, filterBuffers[0]);
    //    addFilterBand(buffer, filterBuffers[1]);
    //}
    //else
    //{
    //    addFilterBand(buffer, lrAllpassBuffer);
    //}

    addFilterBand(buffer, filterBuffers[0]);
    addFilterBand(buffer, filterBuffers[1]);

    if (compressor.bypass->get())
    {
        for (auto channel = 0; channel < numChannels; ++channel)
        {
            juce::FloatVectorOperations::multiply(lrAllpassBuffer.getWritePointer(channel), -1.f, numSamples);
        }

        addFilterBand(buffer, lrAllpassBuffer);
    }
}

//==============================================================================
bool JMB3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JMB3AudioProcessor::createEditor()
{
    // return new JMB3AudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void JMB3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream memoryOutputStream(destData, true);

    apvts.state.writeToStream(memoryOutputStream);
}

void JMB3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto valueTree = juce::ValueTree::readFromData(data, sizeInBytes);

    if (valueTree.isValid())
    {
        apvts.replaceState(valueTree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout JMB3AudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    using namespace juce;
    using namespace Params;

    const auto& params = getParams();

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Threshold_Low_Band),
        params.at(Names::Threshold_Low_Band),
        NormalisableRange<float>(-60, 12, 1, 1), 
        0));

    auto attackReleaseRange = NormalisableRange<float>(5, 500, 1, 1);

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Attack_Low_Band),
        params.at(Names::Attack_Low_Band),
        attackReleaseRange, 
        50));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Release_Low_Band),
        params.at(Names::Release_Low_Band),
        attackReleaseRange,
        250));

    auto choices = std::vector<double>{ 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100 };

    juce::StringArray stringArray;
    for (auto choice : choices)
    {
        stringArray.add(juce::String(choice, 1));
    }

    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::Ratio_Low_Band),
        params.at(Names::Ratio_Low_Band),
        stringArray, 
        3));

    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::Bypassed_Low_Band),
        params.at(Names::Bypassed_Low_Band),
        false));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Low_Mid_Crossover_Freq),
        params.at(Names::Low_Mid_Crossover_Freq),
        NormalisableRange<float>(20, 20000, 1, 1),
        500));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JMB3AudioProcessor();
}
