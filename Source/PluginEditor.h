#pragma once

#include "PluginProcessor.h"

class BinarySunDistortionAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BinarySunDistortionAudioProcessorEditor(BinarySunDistortionAudioProcessor& processor);

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    std::array<juce::Slider, 7> controls;
    std::array<juce::Label, 7> controlLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 7>
        controlAttachments;
    juce::Label modeLabel;
    juce::ComboBox modeSelector;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment modeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinarySunDistortionAudioProcessorEditor)
};
