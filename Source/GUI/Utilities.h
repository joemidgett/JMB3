// Utilities.h

#pragma once

#include <JuceHeader.h>

enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

juce::String getValString(const juce::RangedAudioParameter& param,
    bool getLow,
    juce::String suffix);

template<typename SliderAttachment, typename APVTS, typename Params, typename ParamName, typename SliderType>
void makeAttachment(std::unique_ptr<SliderAttachment>& sliderAttachment,
    APVTS& apvts,
    const Params& params,
    const ParamName& name,
    SliderType& slider)
{
    sliderAttachment = std::make_unique<SliderAttachment>(apvts, params.at(name), slider);
}

template<typename APVTS, typename Params, typename Name>
juce::RangedAudioParameter& getParam(APVTS& apvts, const Params& params, const Name& name)
{
    auto param = apvts.getParameter(params.at(name));
    jassert(param != nullptr);

    return *param;
}

template<typename T>
bool truncateKiloVaue(T& value)
{
    if (value > static_cast<T>(999))
    {
        value /= static_cast<T>(1000);
        return true;
    }

    return false;
}

template<typename Labels, typename ParamType, typename SuffixType>
void addLabelPairs(Labels& labels, const ParamType& param, const SuffixType& suffix)
{
    labels.clear();
    labels.add({ 0.f, getValString(param, true, suffix) });
    labels.add({ 1.f, getValString(param, false, suffix) });
}

juce::Rectangle<int> drawModuleBackground(juce::Graphics& g,
    juce::Rectangle<int> bounds);