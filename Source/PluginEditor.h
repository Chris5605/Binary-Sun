#pragma once

#include "PluginProcessor.h"

class BinarySunDistortionAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                      private juce::Timer
{
public:
    explicit BinarySunDistortionAudioProcessorEditor(BinarySunDistortionAudioProcessor& processor);

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void timerCallback() override;

    BinarySunDistortionAudioProcessor& pluginProcessor;
    std::array<juce::Slider, 11> controls;
    std::array<juce::Label, 11> controlLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 11>
        controlAttachments;
    std::array<juce::Label, 2> modeLabels;
    std::array<juce::ComboBox, 2> modeSelectors;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>, 2>
        modeAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinarySunDistortionAudioProcessorEditor)
};
