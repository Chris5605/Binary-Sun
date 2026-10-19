#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float maximumDriveDb = 36.0f;
constexpr float maximumDriveCompensationDb = -9.0f;
constexpr float gateRangeDb = 18.0f;
constexpr float postGateRangeDb = 48.0f;

enum class DistortionMode
{
    warm,
    hard,
    asymmetric,
    fuzz,
    tube
};

float applyDistortion(float sample, DistortionMode mode)
{
    switch (mode)
    {
        case DistortionMode::warm:
            return std::tanh(sample);
        case DistortionMode::hard:
            return juce::jlimit(-1.0f, 1.0f, sample);
        case DistortionMode::asymmetric:
            return sample >= 0.0f
                ? std::tanh(sample * 1.35f)
                : 0.85f * std::tanh(sample * 1.8f);
        case DistortionMode::fuzz:
        {
            const auto saturated = std::tanh(sample * 2.5f);
            return std::copysign(std::pow(std::abs(saturated), 0.55f), saturated);
        }
        case DistortionMode::tube:
        {
            constexpr float bias = 0.2f;
            const auto zeroPoint = std::tanh(bias);
            const auto shaped = std::tanh(sample * 1.4f + bias) - zeroPoint;
            const auto scale = shaped >= 0.0f ? 1.0f - zeroPoint : 1.0f + zeroPoint;
            return shaped / scale;
        }
    }

    return sample;
}
}

BinarySunDistortionAudioProcessor::BinarySunDistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

BinarySunDistortionAudioProcessor::ParameterState::ParameterLayout
BinarySunDistortionAudioProcessor::createParameterLayout()
{
    ParameterState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "mode", 1 },
        "Engine A Style",
        juce::StringArray {
            "Warm (Soft Clip)",
            "Hard Clip",
            "Asymmetric Overdrive",
            "Fuzz",
            "Tube Saturation"
        },
        0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "modeB", 1 },
        "Engine B Style",
        juce::StringArray {
            "Warm (Soft Clip)",
            "Hard Clip",
            "Asymmetric Overdrive",
            "Fuzz",
            "Tube Saturation"
        },
        4));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "drive", 1 },
        "Engine A Drive",
        juce::NormalisableRange<float> { 0.0f, maximumDriveDb, 0.1f },
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "driveB", 1 },
        "Engine B Drive",
        juce::NormalisableRange<float> { 0.0f, maximumDriveDb, 0.1f },
        12.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "blend", 1 },
        "Blend",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f },
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "coupling", 1 },
        "Coupling",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f },
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "crossover", 1 },
        "Crossover",
        juce::NormalisableRange<float> { 150.0f, 3000.0f, 1.0f, 0.35f },
        700.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "output", 1 },
        "Output",
        juce::NormalisableRange<float> { -24.0f, 12.0f, 0.1f },
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "tone", 1 },
        "Tone",
        juce::NormalisableRange<float> { 1000.0f, 20000.0f, 1.0f, 0.35f },
        20000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "gate", 1 },
        "Gate",
        juce::NormalisableRange<float> { -90.0f, -30.0f, 0.1f },
        -60.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    for (const auto& band : std::array<std::pair<const char*, const char*>, 3> {{
             { "low", "Low" }, { "mid", "Mid" }, { "high", "High" } }})
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { band.first, 1 },
            band.second,
            juce::NormalisableRange<float> { -12.0f, 12.0f, 0.1f },
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));
    }

    return layout;
}

void BinarySunDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    lastFilterValues.fill(-1.0f);
    lastCrossoverFrequency = -1.0f;
    gateEnvelope.fill(0.0f);
    lowBandEnvelope.fill(0.0f);
    gateGain.fill(1.0f);
    postGateGain.fill(1.0f);
    crossover.prepare({ sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 });
    crossover.reset();
    for (auto& channel : toneFilters)
        channel.reset();
    for (auto& band : eqFilters)
        for (auto& channel : band)
            channel.reset();
}

void BinarySunDistortionAudioProcessor::releaseResources()
{
}

bool BinarySunDistortionAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();

    return input == output
        && (input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo());
}

void BinarySunDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                     juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto inputChannels = getTotalNumInputChannels();
    const auto outputChannels = getTotalNumOutputChannels();

    for (auto channel = inputChannels; channel < outputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    const auto driveADb = parameters.getRawParameterValue("drive")->load();
    const auto driveBDb = parameters.getRawParameterValue("driveB")->load();
    const auto blend = parameters.getRawParameterValue("blend")->load() * 0.01f;
    const auto coupling = parameters.getRawParameterValue("coupling")->load() * 0.01f;
    const auto crossoverFrequency = parameters.getRawParameterValue("crossover")->load();
    const auto outputDb = parameters.getRawParameterValue("output")->load();
    const auto toneHz = parameters.getRawParameterValue("tone")->load();
    const auto gateThresholdDb = parameters.getRawParameterValue("gate")->load();
    const std::array<float, 3> eqGains {{
        parameters.getRawParameterValue("low")->load(),
        parameters.getRawParameterValue("mid")->load(),
        parameters.getRawParameterValue("high")->load()
    }};

    const auto safeNyquist = static_cast<float>(currentSampleRate * 0.45);
    const auto safeCrossoverFrequency = juce::jlimit(
        20.0f, safeNyquist, crossoverFrequency);
    if (safeCrossoverFrequency != lastCrossoverFrequency)
    {
        crossover.setCutoffFrequency(safeCrossoverFrequency);
        lastCrossoverFrequency = safeCrossoverFrequency;
    }

    const auto cutoff = juce::jlimit(20.0f, safeNyquist, toneHz);
    const std::array<float, 5> filterValues {{
        cutoff, eqGains[0], eqGains[1], eqGains[2], safeNyquist
    }};

    if (filterValues != lastFilterValues)
    {
        for (auto& filter : toneFilters)
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
                currentSampleRate, cutoff);

        const std::array<float, 3> frequencies {{
            juce::jmin(120.0f, safeNyquist),
            juce::jmin(1000.0f, safeNyquist),
            juce::jmin(8000.0f, safeNyquist)
        }};

        for (size_t band = 0; band < eqFilters.size(); ++band)
        {
            const auto gain = juce::Decibels::decibelsToGain(eqGains[band]);
            juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>> coefficients;

            if (band == 0)
                coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                    currentSampleRate, frequencies[band], 0.707f, gain);
            else if (band == 1)
                coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                    currentSampleRate, frequencies[band], 0.8f, gain);
            else
                coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                    currentSampleRate, frequencies[band], 0.707f, gain);

            for (auto& filter : eqFilters[band])
                filter.coefficients = coefficients;
        }

        lastFilterValues = filterValues;
    }

    const auto inputGainA = juce::Decibels::decibelsToGain(driveADb);
    const auto averageDriveDb = 0.5f * (driveADb + driveBDb);
    const auto driveCompensationDb =
        maximumDriveCompensationDb * (averageDriveDb / maximumDriveDb);
    const auto outputGain =
        juce::Decibels::decibelsToGain(outputDb + driveCompensationDb);
    const auto modeA = static_cast<DistortionMode>(
        juce::roundToInt(parameters.getRawParameterValue("mode")->load()));
    const auto modeB = static_cast<DistortionMode>(
        juce::roundToInt(parameters.getRawParameterValue("modeB")->load()));
    const auto attackCoefficient =
        std::exp(-1.0f / (static_cast<float>(currentSampleRate) * 0.005f));
    const auto releaseCoefficient =
        std::exp(-1.0f / (static_cast<float>(currentSampleRate) * 0.12f));
    const auto blendAngle = blend * juce::MathConstants<float>::halfPi;
    const auto engineAGain = std::cos(blendAngle);
    const auto engineBGain = std::sin(blendAngle);

    for (auto channel = 0; channel < inputChannels; ++channel)
    {
        auto* samples = buffer.getWritePointer(channel);
        auto& envelope = gateEnvelope[static_cast<size_t>(channel)];
        auto& lowEnvelope = lowBandEnvelope[static_cast<size_t>(channel)];
        auto& gate = gateGain[static_cast<size_t>(channel)];
        auto& postGate = postGateGain[static_cast<size_t>(channel)];

        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto input = samples[sample];
            const auto magnitude = std::abs(input);
            const auto detectorCoefficient =
                magnitude > envelope ? attackCoefficient : releaseCoefficient;
            envelope = detectorCoefficient * envelope
                + (1.0f - detectorCoefficient) * magnitude;

            float lowBand = 0.0f;
            float highBand = 0.0f;
            crossover.processSample(channel, input, lowBand, highBand);

            const auto lowMagnitude = std::abs(lowBand);
            const auto lowDetectorCoefficient =
                lowMagnitude > lowEnvelope ? attackCoefficient : releaseCoefficient;
            lowEnvelope = lowDetectorCoefficient * lowEnvelope
                + (1.0f - lowDetectorCoefficient) * lowMagnitude;

            const auto envelopeDb = juce::Decibels::gainToDecibels(envelope, -120.0f);
            const auto lowEnvelopeDb = juce::Decibels::gainToDecibels(lowEnvelope, -120.0f);
            const auto couplingAmount = juce::jlimit(
                0.0f, 1.0f, (lowEnvelopeDb + 36.0f) / 30.0f);
            const auto couplingDriveBoostDb = coupling * couplingAmount * 12.0f;
            const auto coupledDriveDb = driveBDb + couplingDriveBoostDb;
            const auto inputGainB = juce::Decibels::decibelsToGain(coupledDriveDb);
            const auto couplingCompensationGain = juce::Decibels::decibelsToGain(
                maximumDriveCompensationDb * couplingDriveBoostDb
                / (2.0f * maximumDriveDb));
            const auto targetGateDb = juce::jlimit(
                -gateRangeDb, 0.0f, envelopeDb - gateThresholdDb);
            const auto targetGate = juce::Decibels::decibelsToGain(targetGateDb);
            const auto targetPostGateDb = juce::jlimit(
                -postGateRangeDb, 0.0f, (envelopeDb - gateThresholdDb) * 4.0f);
            const auto targetPostGate = juce::Decibels::decibelsToGain(targetPostGateDb);
            const auto gateCoefficient =
                targetGate > gate ? attackCoefficient : releaseCoefficient;
            gate = gateCoefficient * gate + (1.0f - gateCoefficient) * targetGate;
            const auto postGateCoefficient =
                targetPostGate > postGate ? attackCoefficient : releaseCoefficient;
            postGate = postGateCoefficient * postGate + (1.0f - postGateCoefficient) * targetPostGate;

            const auto engineA = applyDistortion(lowBand * gate * inputGainA, modeA);
            const auto engineB = applyDistortion(highBand * gate * inputGainB, modeB);
            auto value = engineA * engineAGain + engineB * engineBGain;
            value = toneFilters[static_cast<size_t>(channel)].processSample(value);

            for (auto& band : eqFilters)
                value = band[static_cast<size_t>(channel)].processSample(value);

            samples[sample] = value * postGate * outputGain * couplingCompensationGain;
        }
    }
}

juce::AudioProcessorEditor* BinarySunDistortionAudioProcessor::createEditor()
{
    return new BinarySunDistortionAudioProcessorEditor(*this);
}

bool BinarySunDistortionAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String BinarySunDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BinarySunDistortionAudioProcessor::acceptsMidi() const
{
    return false;
}

bool BinarySunDistortionAudioProcessor::producesMidi() const
{
    return false;
}

bool BinarySunDistortionAudioProcessor::isMidiEffect() const
{
    return false;
}

double BinarySunDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BinarySunDistortionAudioProcessor::getNumPrograms()
{
    return 1;
}

int BinarySunDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BinarySunDistortionAudioProcessor::setCurrentProgram(int)
{
}

const juce::String BinarySunDistortionAudioProcessor::getProgramName(int)
{
    return {};
}

void BinarySunDistortionAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void BinarySunDistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData)
{
    const auto state = parameters.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destinationData);
}

void BinarySunDistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BinarySunDistortionAudioProcessor();
}
