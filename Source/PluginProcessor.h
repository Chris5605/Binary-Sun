#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <array>

class BinarySunDistortionAudioProcessor final : public juce::AudioProcessor
{
public:
    using ParameterState = juce::AudioProcessorValueTreeState;

    BinarySunDistortionAudioProcessor();

    static ParameterState::ParameterLayout createParameterLayout();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    ParameterState parameters;

private:
    using Filter = juce::dsp::IIR::Filter<float>;

    std::array<Filter, 2> toneFilters;
    std::array<std::array<Filter, 2>, 3> eqFilters;
    std::array<float, 2> gateEnvelope {};
    std::array<float, 2> gateGain { 1.0f, 1.0f };
    std::array<float, 2> postGateGain { 1.0f, 1.0f };
    double currentSampleRate = 44100.0;
    std::array<float, 5> lastFilterValues {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinarySunDistortionAudioProcessor)
};
