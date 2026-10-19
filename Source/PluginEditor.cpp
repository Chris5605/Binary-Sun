#include "PluginEditor.h"
#include "PluginProcessor.h"

BinarySunDistortionAudioProcessorEditor::BinarySunDistortionAudioProcessorEditor(
    BinarySunDistortionAudioProcessor& audioProcessor)
    : AudioProcessorEditor(audioProcessor)
{
    static constexpr std::array<const char*, 11> parameterIds {{
        "drive", "driveB", "blend", "coupling", "crossover", "output",
        "tone", "gate", "low", "mid", "high"
    }};
    static constexpr std::array<const char*, 11> parameterNames {{
        "Engine A Drive", "Engine B Drive", "Blend", "Coupling", "Crossover", "Output",
        "Tone", "Gate", "Low", "Mid", "High"
    }};
    static constexpr std::array<const char*, 2> modeParameterIds {{ "mode", "modeB" }};
    static constexpr std::array<const char*, 2> modeNames {{ "Engine A", "Engine B" }};
    static constexpr std::array<const char*, 5> distortionNames {{
        "Warm (Soft Clip)",
        "Hard Clip",
        "Asymmetric Overdrive",
        "Fuzz",
        "Tube Saturation"
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

    for (size_t engine = 0; engine < modeSelectors.size(); ++engine)
    {
        modeLabels[engine].setText(modeNames[engine], juce::dontSendNotification);
        modeLabels[engine].setJustificationType(juce::Justification::right);
        addAndMakeVisible(modeLabels[engine]);

        for (size_t style = 0; style < distortionNames.size(); ++style)
            modeSelectors[engine].addItem(distortionNames[style], static_cast<int>(style + 1));

        addAndMakeVisible(modeSelectors[engine]);
        modeAttachments[engine] = std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                audioProcessor.parameters, modeParameterIds[engine], modeSelectors[engine]);
    }

    setSize(760, 500);
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
    modeLabels[0].setBounds(60, 50, 105, 28);
    modeSelectors[0].setBounds(170, 50, 220, 28);
    modeLabels[1].setBounds(390, 50, 105, 28);
    modeSelectors[1].setBounds(500, 50, 220, 28);

    constexpr int columns = 4;
    constexpr int left = 14;
    constexpr int top = 104;
    constexpr int cellWidth = 182;
    constexpr int cellHeight = 128;

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
