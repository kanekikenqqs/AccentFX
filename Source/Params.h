// Parameter table: single source of truth for the APVTS layout, the UI, the randomizer and presets.
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include <string>
#include <vector>
#include <cmath>

namespace params
{
enum Kind { kFloat = 0, kInt, kChoice, kBool };

struct PDef
{
    std::string id, name, unit, choices;
    int   kind = kFloat;
    float mn = 0.f, mx = 1.f, def = 0.f, skew = 1.f;
    int   group = -1;        // 0..5 effect category (used by the randomizer), -1 = global
    bool  inPreset = false;  // reset to default when a preset is loaded
};

inline const char* const kCatNames[6] = { "Pitch", "FX", "Glitch", "Mod", "Gen", "Time" };

inline juce::Colour catColour (int cat)
{
    static const juce::uint32 c[6] = { 0xffffa62b, 0xffff4d6d, 0xff2ec4b6, 0xffa06cd5, 0xff7bd88f, 0xff4cc9f0 };
    return juce::Colour (c[juce::jlimit (0, 5, cat)]);
}

struct FxInfo { std::string key; std::vector<std::string> details; };

inline const std::vector<FxInfo>& fxInfo()
{
    static const std::vector<FxInfo> v = {
        { "octdn", { "pitchWin" } }, { "octup", {} }, { "harm", { "harmInt" } }, { "wham", { "whamPos" } },
        { "dive", { "diveDepth", "diveTime" } }, { "det", { "detCents" } },
        { "fuzz", { "fuzzDrive" } }, { "crush", { "crushBits", "crushRate" } }, { "ring", { "ringFreq" } },
        { "filt", { "filtMode", "filtFreq", "filtRes", "filtRate", "filtDepth" } }, { "radio", {} }, { "wah", { "wahSens" } },
        { "stut", { "glDiv" } }, { "rev", { "glDiv" } }, { "tstop", { "tsTime" } }, { "gate", { "gateDiv" } },
        { "frz", { "frzSize" } }, { "scat", { "glDiv" } },
        { "cho", { "choRate", "choDepth" } }, { "fla", { "flRate", "flFb" } }, { "pha", { "phRate", "phFb" } },
        { "trem", { "trDiv", "trDepth" } }, { "vib", { "vibRate", "vibDepth" } },
        { "sub", { "subFreq", "subDecay" } }, { "ris", { "risTime" } },
        { "dly", { "dlyDiv", "dlyMs", "dlyFb", "dlyTone", "dlyPing" } },
        { "rvb", { "revDecay", "revDamp", "revPre" } }, { "shim", { "shDecay", "shFb", "shInt" } } };
    return v;
}
inline std::string amtId (int fx) { return "a_" + fxInfo()[(size_t) fx].key; }

inline std::string padChoices()
{
    std::string s = "None";
    for (int i = 0; i < djx::kNumFx; ++i) s += std::string ("|") + djx::kFxNames[i];
    return s;
}

inline const std::vector<PDef>& defs()
{
    static const std::vector<PDef> v = []
    {
        std::vector<PDef> d;
        auto F = [&] (const char* id, const char* name, float mn, float mx, float def, float skew, const char* unit, int grp, bool pre = true)
        { PDef p; p.id = id; p.name = name; p.kind = kFloat; p.mn = mn; p.mx = mx; p.def = def; p.skew = skew; p.unit = unit; p.group = grp; p.inPreset = pre && grp >= 0; d.push_back (p); };
        auto I = [&] (const char* id, const char* name, int mn, int mx, int def, const char* unit, int grp, bool pre = true)
        { PDef p; p.id = id; p.name = name; p.kind = kInt; p.mn = (float) mn; p.mx = (float) mx; p.def = (float) def; p.unit = unit; p.group = grp; p.inPreset = pre && grp >= 0; d.push_back (p); };
        auto C = [&] (const char* id, const char* name, const std::string& ch, int def, int grp, bool pre = true)
        { PDef p; p.id = id; p.name = name; p.kind = kChoice; p.choices = ch; p.def = (float) def; p.group = grp; p.inPreset = pre && grp >= 0; d.push_back (p); };
        auto B = [&] (const char* id, const char* name, bool def)
        { PDef p; p.id = id; p.name = name; p.kind = kBool; p.def = def ? 1.f : 0.f; d.push_back (p); };

        // ---- globals
        B ("bypass", "Bypass", false);
        F ("inGain", "Input", -24, 12, 0, 1, "dB", -1);
        F ("outGain", "Output", -24, 12, 0, 1, "dB", -1);
        B ("engage", "Accent", false);
        C ("engMode", "Footswitch", "Latch|Momentary", 0, -1);
        F ("fade", "Fade", 1, 200, 8, 0.5f, "ms", -1);
        B ("midi", "MIDI Hold", false);
        C ("autoRand", "Auto Random", "Off|On Engage|On Release", 0, -1);
        F ("chaos", "Chaos", 0, 1, 0.5f, 1, "", -1);
        const char* rn[8] = { "Pitch", "FX", "Glitch", "Mod", "Gen", "Time", "Pad", "Gate" };
        for (int i = 0; i < 8; ++i) { const std::string rid = "rnd" + std::to_string (i); B (rid.c_str(), rn[i], true); }

        // ---- XY pad
        F ("padX", "Pad X", 0, 1, 0.5f, 1, "", -1);
        F ("padY", "Pad Y", 0, 1, 0.5f, 1, "", -1);
        F ("padDepth", "Depth", 0, 1, 1, 1, "", -1);
        C ("padMode", "Pad Mode", "Radial|Morph", 0, -1);
        B ("padTouch", "Touch = Accent", true);
        B ("padSpring", "Spring", true);
        const std::string pc = padChoices();
        { PDef p; p.kind = kChoice; p.choices = pc; p.inPreset = true;
          p.id = "padTL"; p.name = "Top Left";     p.def = (float) (djx::Fx::Shimmer + 1); d.push_back (p);
          p.id = "padTR"; p.name = "Top Right";    p.def = (float) (djx::Fx::Scatter + 1); d.push_back (p);
          p.id = "padBL"; p.name = "Bottom Left";  p.def = (float) (djx::Fx::OctDown + 1); d.push_back (p);
          p.id = "padBR"; p.name = "Bottom Right"; p.def = (float) (djx::Fx::Dive + 1);    d.push_back (p); }

        // ---- amounts
        for (int i = 0; i < djx::kNumFx; ++i)
            { const std::string aid = amtId (i); F (aid.c_str(), djx::kFxNames[i], 0, 1, 0, 1, "", djx::kFxCategory[i]); }

        // ---- details
        const std::string divs = djx::kDivNames;
        // pitch
        F ("pitchWin", "Window", 20, 120, 50, 1, "ms", 0);
        I ("harmInt", "Interval", -24, 24, 7, "st", 0);
        F ("whamPos", "Position", -24, 24, -12, 1, "st", 0);
        F ("diveDepth", "Depth", 1, 36, 24, 1, "st", 0);
        F ("diveTime", "Time", 0.1f, 4, 1.2f, 0.5f, "s", 0);
        F ("detCents", "Cents", 1, 50, 18, 1, "ct", 0);
        // fx
        F ("fuzzDrive", "Drive", 0, 1, 0.6f, 1, "", 1);
        I ("crushBits", "Bits", 2, 12, 6, "bit", 1);
        F ("crushRate", "Down", 1, 40, 8, 0.5f, "x", 1);
        F ("ringFreq", "Freq", 20, 4000, 220, 0.3f, "Hz", 1);
        C ("filtMode", "Type", "LP|BP|HP", 1, 1);
        F ("filtFreq", "Cutoff", 40, 12000, 800, 0.3f, "Hz", 1);
        F ("filtRes", "Reso", 0, 1, 0.5f, 1, "", 1);
        F ("filtRate", "Rate", 0.1f, 20, 2, 0.4f, "Hz", 1);
        F ("filtDepth", "Sweep", 0, 1, 0.5f, 1, "", 1);
        F ("wahSens", "Sens", 0, 1, 0.6f, 1, "", 1);
        // glitch
        C ("glDiv", "Slice", divs, 6, 2);
        F ("tsTime", "Time", 0.05f, 3, 0.6f, 0.5f, "s", 2);
        C ("gateDiv", "Step", divs, 6, 2);
        I ("gatePat", "Gate Pattern", 0, 65535, 11693, "", -1);   // 16 steps as bit mask, edited by the step sequencer
        d.back().inPreset = true;
        F ("frzSize", "Size", 40, 400, 150, 1, "ms", 2);
        // mod
        F ("choRate", "Rate", 0.1f, 5, 0.6f, 0.5f, "Hz", 3);
        F ("choDepth", "Depth", 0, 1, 0.5f, 1, "", 3);
        F ("flRate", "Rate", 0.05f, 5, 0.3f, 0.5f, "Hz", 3);
        F ("flFb", "Fdbk", -0.9f, 0.9f, 0.6f, 1, "", 3);
        F ("phRate", "Rate", 0.05f, 8, 0.4f, 0.5f, "Hz", 3);
        F ("phFb", "Fdbk", 0, 0.9f, 0.5f, 1, "", 3);
        C ("trDiv", "Rate", divs, 4, 3);
        F ("trDepth", "Depth", 0, 1, 1, 1, "", 3);
        F ("vibRate", "Rate", 0.5f, 12, 5, 0.5f, "Hz", 3);
        F ("vibDepth", "Depth", 0, 1, 0.5f, 1, "", 3);
        // gen
        F ("subFreq", "Pitch", 30, 90, 55, 1, "Hz", 4);
        F ("subDecay", "Decay", 0.2f, 3, 1.2f, 0.6f, "s", 4);
        F ("risTime", "Time", 0.25f, 8, 2, 0.5f, "s", 4);
        // time
        C ("dlyDiv", "Time", divs + "|Free", 3, 5);
        F ("dlyMs", "Free ms", 20, 2000, 350, 0.4f, "ms", 5);
        F ("dlyFb", "Fdbk", 0, 0.95f, 0.45f, 1, "", 5);
        F ("dlyTone", "Tone", 500, 12000, 5000, 0.4f, "Hz", 5);
        F ("dlyPing", "Ping", 0, 1, 0.7f, 1, "", 5);
        F ("revDecay", "Decay", 0.3f, 20, 3.5f, 0.4f, "s", 5);
        F ("revDamp", "Damp", 0, 1, 0.5f, 1, "", 5);
        F ("revPre", "Pre", 0, 200, 20, 1, "ms", 5);
        F ("shDecay", "Decay", 1, 20, 8, 0.4f, "s", 5);
        F ("shFb", "Fdbk", 0, 0.85f, 0.5f, 1, "", 5);
        I ("shInt", "Interval", -12, 12, 12, "st", 5);
        return d;
    }();
    return v;
}

inline const PDef* find (const std::string& id)
{
    for (auto& d : defs()) if (d.id == id) return &d;
    return nullptr;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> ps;
    for (auto& d : defs())
    {
        const ParameterID pid { d.id, 1 };
        switch (d.kind)
        {
            case kFloat:
            {
                NormalisableRange<float> r (d.mn, d.mx);
                r.skew = d.skew;
                ps.push_back (std::make_unique<AudioParameterFloat> (pid, d.name, r, d.def, AudioParameterFloatAttributes().withLabel (d.unit)));
                break;
            }
            case kInt:
                ps.push_back (std::make_unique<AudioParameterInt> (pid, d.name, (int) d.mn, (int) d.mx, (int) d.def, AudioParameterIntAttributes().withLabel (d.unit)));
                break;
            case kChoice:
            {
                StringArray sa; sa.addTokens (String (d.choices), "|", "");
                ps.push_back (std::make_unique<AudioParameterChoice> (pid, d.name, sa, (int) d.def));
                break;
            }
            default:
                ps.push_back (std::make_unique<AudioParameterBool> (pid, d.name, d.def > 0.5f));
        }
    }
    return { ps.begin(), ps.end() };
}

// XY pad -> weights of the 4 corners (TL, TR, BL, BR). x,y in 0..1, y = 1 is the top.
inline void padWeights (float x, float y, int mode, float* w)
{
    // The UI pad is a disc: stored x/y = 0.5 + 0.5 * (unit-disc coordinate)
    if (mode == 1)   // Morph: bilinear blend, centre = all four equally
    {
        const float xs = std::min (1.f, std::max (0.f, 0.5f + (x - 0.5f) * 1.41421356f));
        const float ys = std::min (1.f, std::max (0.f, 0.5f + (y - 0.5f) * 1.41421356f));
        const float f[4] = { (1.f - xs) * ys, xs * ys, (1.f - xs) * (1.f - ys), xs * (1.f - ys) };
        for (int k = 0; k < 4; ++k) w[k] = std::pow (f[k], 0.6f);
    }
    else             // Radial: centre = nothing, distance = intensity, direction = which corner
    {
        const float px = (x - 0.5f) * 2.f, py = (y - 0.5f) * 2.f;
        const float len = std::sqrt (px * px + py * py);
        const float r = std::min (1.f, len);
        const float ux[4] = { -1.f, 1.f, -1.f, 1.f }, uy[4] = { 1.f, 1.f, -1.f, -1.f };
        for (int k = 0; k < 4; ++k)
        {
            const float c = len > 1e-4f ? (px * ux[k] + py * uy[k]) / (len * 1.41421356f) : 0.f;
            w[k] = r * std::pow (std::max (0.f, c), 1.5f);
        }
    }
}

// ------------------------------------------------------------------ factory presets
struct Preset { const char* name; std::vector<std::pair<const char*, float>> v; };

inline const std::vector<Preset>& presets()
{
    using namespace djx;
    static const std::vector<Preset> p = {
        { "Init", {} },
        { "Octave Stab", { { "a_octdn", 0.85f }, { "a_rvb", 0.25f }, { "revDecay", 1.8f }, { "a_dly", 0.2f }, { "dlyDiv", 4 }, { "dlyFb", 0.3f } } },
        { "Dive Bomb", { { "a_dive", 1.f }, { "diveDepth", 30 }, { "diveTime", 0.9f }, { "a_sub", 0.5f }, { "subFreq", 48 }, { "a_rvb", 0.2f } } },
        { "Breakdown Shimmer", { { "a_shim", 0.85f }, { "shDecay", 10 }, { "shFb", 0.6f }, { "a_rvb", 0.45f }, { "revDecay", 5 }, { "a_octup", 0.25f }, { "a_dly", 0.4f }, { "dlyDiv", 3 }, { "dlyFb", 0.5f } } },
        { "Gate Chop 16ths", { { "a_gate", 1.f }, { "gateDiv", 6 }, { "gatePat", 53970 }, { "a_crush", 0.25f }, { "crushBits", 7 }, { "crushRate", 4 }, { "a_dly", 0.3f }, { "dlyDiv", 6 }, { "dlyFb", 0.35f } } },
        { "Tape Stop Kill", { { "a_tstop", 1.f }, { "tsTime", 0.45f }, { "a_sub", 0.6f }, { "a_rvb", 0.15f } } },
        { "Riser + Impact", { { "a_ris", 1.f }, { "risTime", 3 }, { "a_sub", 1.f }, { "subDecay", 1.8f }, { "a_rvb", 0.45f }, { "revDecay", 4 }, { "a_fuzz", 0.15f } } },
        { "Glitch Scatter", { { "a_scat", 0.9f }, { "glDiv", 6 }, { "a_crush", 0.4f }, { "crushBits", 5 }, { "crushRate", 6 }, { "a_ring", 0.25f }, { "ringFreq", 330 }, { "a_dly", 0.35f }, { "dlyPing", 1.f }, { "dlyDiv", 3 } } },
        { "Stutter Freeze", { { "a_stut", 1.f }, { "glDiv", 7 }, { "a_frz", 0.5f }, { "frzSize", 120 }, { "a_rvb", 0.3f } } },
        { "Ambient Freeze", { { "a_frz", 1.f }, { "frzSize", 220 }, { "a_shim", 0.6f }, { "a_rvb", 0.55f }, { "revDecay", 8 }, { "a_filt", 0.5f }, { "filtMode", 0 }, { "filtFreq", 2500 }, { "filtRes", 0.3f }, { "filtDepth", 0.3f }, { "filtRate", 0.5f } } },
        { "Robot Fifth", { { "a_harm", 0.55f }, { "harmInt", 7 }, { "a_ring", 0.5f }, { "ringFreq", 150 }, { "a_pha", 0.5f }, { "phRate", 0.3f } } },
        { "Wobble Filter", { { "a_filt", 1.f }, { "filtMode", 1 }, { "filtFreq", 600 }, { "filtRes", 0.7f }, { "filtRate", 4 }, { "filtDepth", 0.9f }, { "a_trem", 0.4f }, { "a_rvb", 0.15f } } },
        { "Whammy Octave Up", { { "a_wham", 1.f }, { "whamPos", 12 }, { "a_det", 0.3f }, { "a_dly", 0.25f } } },
        { "Underwater", { { "a_cho", 0.7f }, { "choRate", 0.3f }, { "choDepth", 1.f }, { "a_vib", 0.4f }, { "a_filt", 0.6f }, { "filtMode", 0 }, { "filtFreq", 1200 }, { "filtRes", 0.2f }, { "filtDepth", 0.2f }, { "filtRate", 0.3f }, { "a_rvb", 0.5f } } },
        { "Pad: Cinematic", { { "padTL", (float) (Fx::Shimmer + 1) }, { "padTR", (float) (Fx::Riser + 1) }, { "padBL", (float) (Fx::OctDown + 1) }, { "padBR", (float) (Fx::TapeStop + 1) }, { "a_rvb", 0.2f } } },
        { "Pad: Chaos Glitch", { { "padTL", (float) (Fx::Scatter + 1) }, { "padTR", (float) (Fx::Stutter + 1) }, { "padBL", (float) (Fx::Fuzz + 1) }, { "padBR", (float) (Fx::Crush + 1) }, { "a_dly", 0.25f } } },
    };
    return p;
}
} // namespace params
