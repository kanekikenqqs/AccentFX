#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>
#include <vector>
#include "Engine.h"
#include "Params.h"

// Parameters read on the audio thread (cached raw pointers, ids == member names)
#define DJX_RAW_PARAMS(X) \
    X(bypass) X(inGain) X(outGain) X(engage) X(fade) X(midi) X(autoRand) X(chaos) \
    X(padX) X(padY) X(padDepth) X(padMode) X(padTL) X(padTR) X(padBL) X(padBR) \
    X(pitchWin) X(harmInt) X(whamPos) X(diveDepth) X(diveTime) X(detCents) \
    X(fuzzDrive) X(crushBits) X(crushRate) X(ringFreq) X(filtMode) X(filtFreq) X(filtRes) X(filtRate) X(filtDepth) X(wahSens) \
    X(glDiv) X(tsTime) X(gateDiv) X(gatePat) X(frzSize) \
    X(choRate) X(choDepth) X(flRate) X(flFb) X(phRate) X(phFb) X(trDiv) X(trDepth) X(vibRate) X(vibDepth) \
    X(subFreq) X(subDecay) X(risTime) \
    X(dlyDiv) X(dlyMs) X(dlyFb) X(dlyTone) X(dlyPing) X(revDecay) X(revDamp) X(revPre) X(shDecay) X(shFb) X(shInt)

class AccentFXProcessor : public juce::AudioProcessor,
                          private juce::AsyncUpdater
{
public:
    AccentFXProcessor();
    ~AccentFXProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Accent FX"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 20.0; }
    juce::AudioProcessorParameter* getBypassParameter() const override { return apvts.getParameter ("bypass"); }

    int getNumPrograms() override { return (int) params::presets().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // message thread
    void randomizeNow();

    juce::AudioProcessorValueTreeState apvts;

    // values the editor polls
    std::atomic<float> uiAmt[djx::kNumFx];
    std::atomic<float> uiEngage { 0.f };
    std::atomic<int>   uiGateStep { 0 };

private:
    void handleAsyncUpdate() override { randomizeNow(); }
    void buildSettings (djx::Settings& s, bool engagedNow);
    void applySettingsAndRun (float* L, float* R, int n, const djx::Settings& s, const djx::Tempo& t);

    struct Raw
    {
#define X(n) std::atomic<float>* n = nullptr;
        DJX_RAW_PARAMS(X)
#undef X
        std::atomic<float>* amt[djx::kNumFx] {};
    } raw;

    djx::Engine engine;
    djx::Settings settings;
    std::vector<float> monoR;
    double currentSampleRate = 48000.0;
    double freeBeat = 0.0;
    int heldNotes = 0;
    bool prevEngaged = false;
    int currentProgram = 0;

    // bypass (smooth crossfade to the untouched input; engine is idle once fully bypassed)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassSm;
    juce::AudioBuffer<float> dryBuf;
    std::vector<float> bypassRamp;
    bool wasBypassed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccentFXProcessor)
};
