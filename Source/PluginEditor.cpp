#include "PluginEditor.h"
#include "PluginProcessor.h"

BinarySunDistortionAudioProcessorEditor::BinarySunDistortionAudioProcessorEditor(
    BinarySunDistortionAudioProcessor& audioProcessor)
    : AudioProcessorEditor(audioProcessor),
      modeAttachment(audioProcessor.parameters, "mode", modeSelector)
{
    static constexpr std::array<const char*, 7> parameterIds {{
        "drive", "output", "tone", "gate", "low", "mid", "high"
    }};
    static constexpr std::array<const char*, 7> parameterNames {{
        "Drive", "Output", "Tone", "Gate", "Low", "Mid", "High"
    }};

    for (size_t index = 0; index < controls.size(); ++index)
    {
        auto& slider = controls[index];
        auto& label = controlLabels[index];

        label.setText(parameterNames[index], juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);

        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
        addAndMakeVisible(slider);

        controlAttachments[index] = std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters, parameterIds[index], slider);
    }

    modeLabel.setText("Distortion", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(modeLabel);

    modeSelector.addItem("Warm (Soft Clip)", 1);
    modeSelector.addItem("Hard Clip", 2);
    modeSelector.addItem("Asymmetric Overdrive", 3);
    modeSelector.addItem("Fuzz", 4);
    modeSelector.addItem("Tube Saturation", 5);
    addAndMakeVisible(modeSelector);

    setSize(720, 390);
}

void BinarySunDistortionAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff202124));
    graphics.setColour(juce::Colour(0xffffbd59));
    graphics.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    graphics.drawText("BINARY SUN DISTORTION",
                      20,
                      12,
                      getWidth() - 40,
                      28,
                      juce::Justification::centred);
}

void BinarySunDistortionAudioProcessorEditor::resized()
{
    modeLabel.setBounds(150, 48, 105, 28);
    modeSelector.setBounds(265, 48, 190, 28);

    constexpr int columns = 4;
    constexpr int left = 12;
    constexpr int top = 88;
    constexpr int cellWidth = 174;
    constexpr int cellHeight = 142;

    for (size_t index = 0; index < controls.size(); ++index)
    {
        const auto column = static_cast<int>(index % columns);
        const auto row = static_cast<int>(index / columns);
        const auto x = left + column * cellWidth;
        const auto y = top + row * cellHeight;

        controlLabels[index].setBounds(x, y, cellWidth, 20);
        controls[index].setBounds(x + 35, y + 18, cellWidth - 70, cellHeight - 18);
    }
}
