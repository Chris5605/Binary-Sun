#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace
{
const juce::Colour backgroundColour { 0xff07111f };
const juce::Colour blueColour { 0xff19baff };
const juce::Colour brightBlueColour { 0xff8be7ff };
const juce::Colour goldColour { 0xffffbd59 };
}

BinarySunDistortionAudioProcessorEditor::BinarySunDistortionAudioProcessorEditor(
    BinarySunDistortionAudioProcessor& audioProcessor)
    : AudioProcessorEditor(audioProcessor),
      pluginProcessor(audioProcessor)
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
        label.setColour(juce::Label::textColourId, juce::Colour(0xffcad8e8));
        addAndMakeVisible(label);

        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
        slider.setColour(juce::Slider::rotarySliderFillColourId, blueColour);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff263d55));
        slider.setColour(juce::Slider::thumbColourId, brightBlueColour);
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff101f30));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff263d55));
        addAndMakeVisible(slider);

        controlAttachments[index] = std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters, parameterIds[index], slider);
    }

    for (size_t engine = 0; engine < modeSelectors.size(); ++engine)
    {
        modeLabels[engine].setText(modeNames[engine], juce::dontSendNotification);
        modeLabels[engine].setJustificationType(juce::Justification::right);
        modeLabels[engine].setColour(juce::Label::textColourId, brightBlueColour);
        addAndMakeVisible(modeLabels[engine]);

        for (size_t style = 0; style < distortionNames.size(); ++style)
            modeSelectors[engine].addItem(distortionNames[style], static_cast<int>(style + 1));

        modeSelectors[engine].setColour(juce::ComboBox::backgroundColourId,
                                        juce::Colour(0xff101f30));
        modeSelectors[engine].setColour(juce::ComboBox::textColourId,
                                        juce::Colours::white);
        modeSelectors[engine].setColour(juce::ComboBox::outlineColourId,
                                        juce::Colour(0xff2878a5));
        addAndMakeVisible(modeSelectors[engine]);
        modeAttachments[engine] = std::make_unique<
            juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                audioProcessor.parameters, modeParameterIds[engine], modeSelectors[engine]);
    }

    setSize(800, 680);
    startTimerHz(30);
}

void BinarySunDistortionAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(backgroundColour);

    juce::ColourGradient backdrop(juce::Colour(0xff10263a),
                                  0.0f,
                                  0.0f,
                                  backgroundColour,
                                  0.0f,
                                  330.0f,
                                  false);
    graphics.setGradientFill(backdrop);
    graphics.fillRect(0, 0, getWidth(), 360);

    graphics.setColour(goldColour);
    graphics.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    graphics.drawText("BINARY SUN",
                      20, 12, getWidth() - 40, 28, juce::Justification::centred);
    graphics.setColour(juce::Colour(0xff8299b0));
    graphics.setFont(juce::FontOptions(11.0f));
    graphics.drawText("DUAL-BAND DISTORTION",
                      20, 38, getWidth() - 40, 18, juce::Justification::centred);

    const auto getParameter = [this](const char* id)
    {
        return pluginProcessor.parameters.getRawParameterValue(id)->load();
    };

    const auto driveA = getParameter("drive");
    const auto driveB = getParameter("driveB");
    const auto blend = getParameter("blend") * 0.01f;
    const auto coupling = getParameter("coupling") * 0.01f;
    const auto crossover = getParameter("crossover");
    const auto modeA = juce::roundToInt(getParameter("mode"));
    const auto modeB = juce::roundToInt(getParameter("modeB"));
    const auto spacing = juce::jlimit(
        220.0f,
        390.0f,
        245.0f + blend * 50.0f + coupling * 35.0f
            + std::abs(driveA - driveB) * 0.8f);
    const auto centerY = 139.0f;
    const std::array<float, 2> centers {{
        static_cast<float>(getWidth()) * 0.5f - spacing * 0.5f,
        static_cast<float>(getWidth()) * 0.5f + spacing * 0.5f
    }};
    const std::array<float, 2> drives {{ driveA, driveB }};
    const std::array<int, 2> modes {{ modeA, modeB }};
    const auto phase = static_cast<float>(juce::Time::getMillisecondCounterHiRes() * 0.001);

    for (size_t engine = 0; engine < centers.size(); ++engine)
    {
        const auto radius = 56.0f + juce::jlimit(0.0f, 16.0f, drives[engine] * 0.4f);
        const auto spikeDepth = 3.0f + drives[engine] * 0.42f;
        const auto modeIndex = juce::jlimit(0, 4, modes[engine]);
        const auto xCentre = centers[engine];
        const auto yCentre = centerY;

        juce::Path waveform;
        constexpr int points = 192;
        for (int point = 0; point <= points; ++point)
        {
            const auto angle = juce::MathConstants<float>::twoPi
                * static_cast<float>(point) / static_cast<float>(points);
            const auto mainHarmonic = std::sin(
                angle * static_cast<float>(5 + modeIndex * 2) + phase * (engine == 0 ? 1.0f : -0.8f));
            const auto fineHarmonic = std::sin(
                angle * static_cast<float>(11 + modeIndex * 3) - phase * 0.65f);
            const auto fuzzHarmonic = std::sin(angle * 23.0f + phase * 1.4f);
            const auto modeShape = modeIndex == 3 ? fuzzHarmonic * 0.38f : fineHarmonic * 0.24f;
            const auto splitMotion = (crossover - 700.0f) / 2300.0f;
            const auto modulation = mainHarmonic * 0.58f + modeShape
                + std::sin(angle * 2.0f + phase * 0.4f) * splitMotion * 0.12f;
            const auto pointRadius = radius + spikeDepth * modulation;
            const auto x = xCentre + std::cos(angle) * pointRadius;
            const auto y = yCentre + std::sin(angle) * pointRadius;

            if (point == 0)
                waveform.startNewSubPath(x, y);
            else
                waveform.lineTo(x, y);
        }

        const auto engineColour = engine == 0 ? blueColour : brightBlueColour;
        graphics.setColour(engineColour.withAlpha(0.08f));
        graphics.strokePath(waveform, juce::PathStrokeType(12.0f));
        graphics.setColour(engineColour.withAlpha(0.2f));
        graphics.strokePath(waveform, juce::PathStrokeType(5.0f));
        graphics.setColour(engineColour);
        graphics.strokePath(waveform, juce::PathStrokeType(1.8f));

        graphics.setColour(engineColour.withAlpha(0.28f));
        graphics.drawEllipse(xCentre - radius - 17.0f,
                             yCentre - radius - 17.0f,
                             (radius + 17.0f) * 2.0f,
                             (radius + 17.0f) * 2.0f,
                             1.0f);

        graphics.setColour(engineColour.withAlpha(0.75f));
        graphics.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        graphics.drawText(engine == 0 ? "LOW / ENGINE A" : "HIGH / ENGINE B",
                          static_cast<int>(xCentre - 90.0f),
                          228,
                          180,
                          18,
                          juce::Justification::centred);
    }

    graphics.setColour(juce::Colour(0xff263d55));
    graphics.drawHorizontalLine(260, 24.0f, static_cast<float>(getWidth() - 24));
    graphics.drawHorizontalLine(382, 24.0f, static_cast<float>(getWidth() - 24));
}

void BinarySunDistortionAudioProcessorEditor::resized()
{
    modeLabels[0].setBounds(91, 280, 94, 28);
    modeSelectors[0].setBounds(187, 280, 210, 28);
    modeLabels[1].setBounds(403, 280, 94, 28);
    modeSelectors[1].setBounds(499, 280, 210, 28);

    constexpr int cellWidth = 190;
    constexpr int top = 392;
    constexpr int rowHeight = 92;
    constexpr std::array<int, 11> columns {{ 0, 3, 1, 2, 0, 1, 2, 3, 0, 1, 2 }};
    constexpr std::array<int, 11> rows {{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2 }};

    for (size_t index = 0; index < controls.size(); ++index)
    {
        auto x = (getWidth() - 4 * cellWidth) / 2 + columns[index] * cellWidth;
        if (rows[index] == 2)
            x = (getWidth() - 3 * cellWidth) / 2
                + (columns[index] - 0) * cellWidth;

        const auto y = top + rows[index] * rowHeight;
        controlLabels[index].setBounds(x, y, cellWidth, 18);
        controls[index].setBounds(x + 48, y + 18, cellWidth - 96, 72);
    }
}

void BinarySunDistortionAudioProcessorEditor::timerCallback()
{
    repaint();
}
