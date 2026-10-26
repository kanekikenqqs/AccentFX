#pragma once
#include "PluginProcessor.h"
#include "UiWidgets.h"

// Logical design size; the whole UI is scaled when the host window is resized.
constexpr int kDesignW = 1180, kDesignH = 760;

// ------------------------------------------------------------------ PLAY page
class PlayPage : public juce::Component
{
public:
    explicit PlayPage (AccentFXProcessor& p);
    void tick (float dt);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AccentFXProcessor& proc;
    ui::CircleXYPad pad;
    std::unique_ptr<ui::CornerReadout> readouts[4];
    std::vector<std::unique_ptr<ui::PillToggle>> pills;
    ui::GateKeys keys;
    ui::AccentButton accent;
    ui::IconButton shuffleBtn { ui::IconButton::Shuffle }, prevBtn { ui::IconButton::Prev }, nextBtn { ui::IconButton::Next };
    std::unique_ptr<ui::ParamControl> chaos;
    juce::String shownPreset;
};

// ------------------------------------------------------------------ RACK page
class RackPage : public juce::Component
{
public:
    explicit RackPage (AccentFXProcessor& p);
    void tick();
    void resized() override;
private:
    std::vector<std::unique_ptr<ui::FxStrip>> strips;
};

// ------------------------------------------------------------------ SETUP page
class SetupPage : public juce::Component
{
public:
    explicit SetupPage (AccentFXProcessor& p);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    std::vector<std::unique_ptr<ui::ParamControl>> knobs, combos, toggles;
};

// ------------------------------------------------------------------ EDITOR
class AccentFXEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit AccentFXEditor (AccentFXProcessor&);
    ~AccentFXEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Content : public juce::Component
    {
        explicit Content (AccentFXEditor& o) : owner (o) {}
        void paint (juce::Graphics& g) override { owner.paintContent (g); }
        AccentFXEditor& owner;
    };
    void paintContent (juce::Graphics&);
    void timerCallback() override;
    void showPage (int);

    AccentFXProcessor& proc;
    ui::AccentLnF lnf;
    Content content { *this };

    ui::NavTab tabPlay { 0, "Play" }, tabRack { 1, "Rack" }, tabSetup { 2, "Setup" };
    ui::PresetLabel presetLabel;
    ui::IconButton presetPrev { ui::IconButton::Prev }, presetNext { ui::IconButton::Next };
    ui::BypassButton bypass;
    juce::Slider inSlider, outSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inAtt, outAtt;

    PlayPage play;
    RackPage rack;
    SetupPage setup;
    int page = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccentFXEditor)
};
