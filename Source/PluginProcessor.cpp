#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

AccentFXProcessor::AccentFXProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  AudioChannelSet::stereo(), true)
                          .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", params::createLayout())
{
#define X(n) raw.n = apvts.getRawParameterValue (#n);
    DJX_RAW_PARAMS(X)
#undef X
    for (int i = 0; i < djx::kNumFx; ++i)
    {
        raw.amt[i] = apvts.getRawParameterValue (params::amtId (i));
        uiAmt[i].store (0.f);
    }
}

bool AccentFXProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    if (out != AudioChannelSet::mono() && out != AudioChannelSet::stereo()) return false;
    return out == l.getMainInputChannelSet();
}

void AccentFXProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    engine.prepare (sampleRate);
    monoR.assign ((size_t) jmax (512, samplesPerBlock), 0.f);
    heldNotes = 0;
    prevEngaged = false;

    const int maxBlock = jmax (512, samplesPerBlock);
    dryBuf.setSize (2, maxBlock);
    dryBuf.clear();
    bypassRamp.assign ((size_t) maxBlock, 0.f);
    bypassSm.reset (sampleRate, 0.015);
    wasBypassed = raw.bypass->load() > 0.5f;
    bypassSm.setCurrentAndTargetValue (wasBypassed ? 1.f : 0.f);
}

// ------------------------------------------------------------------------------------------
void AccentFXProcessor::buildSettings (djx::Settings& s, bool engagedNow)
{
    using namespace djx;
    auto V = [] (std::atomic<float>* p) { return p->load(); };
    auto IDX = [] (float v, int hi) { return jlimit (0, hi, (int) (v + 0.5f)); };

    s.engaged = engagedNow;
    s.fadeMs  = V (raw.fade);
    s.inGain  = Decibels::decibelsToGain (V (raw.inGain),  -60.f);
    s.outGain = Decibels::decibelsToGain (V (raw.outGain), -60.f);

    for (int i = 0; i < kNumFx; ++i) s.amt[(size_t) i] = V (raw.amt[i]);

    // XY pad adds to the manual amounts
    float w[4];
    params::padWeights (V (raw.padX), V (raw.padY), IDX (V (raw.padMode), 1), w);
    const int corner[4] = { IDX (V (raw.padTL), kNumFx) - 1, IDX (V (raw.padTR), kNumFx) - 1,
                            IDX (V (raw.padBL), kNumFx) - 1, IDX (V (raw.padBR), kNumFx) - 1 };
    const float depth = V (raw.padDepth);
    for (int k = 0; k < 4; ++k)
        if (corner[k] >= 0 && corner[k] < kNumFx) s.amt[(size_t) corner[k]] += w[k] * depth;
    for (auto& a : s.amt) a = jlimit (0.f, 1.f, a);

    s.pitchWinMs = V (raw.pitchWin);  s.harmSemis = V (raw.harmInt);   s.whamSemis = V (raw.whamPos);
    s.diveSemis  = V (raw.diveDepth); s.diveTime  = V (raw.diveTime);  s.detCents  = V (raw.detCents);
    s.fuzzDrive  = V (raw.fuzzDrive); s.crushBits = V (raw.crushBits); s.crushRate = V (raw.crushRate);
    s.ringFreq   = V (raw.ringFreq);  s.filtMode  = IDX (V (raw.filtMode), 2);
    s.filtFreq   = V (raw.filtFreq);  s.filtRes   = V (raw.filtRes);   s.filtRate  = V (raw.filtRate);
    s.filtDepth  = V (raw.filtDepth); s.wahSens   = V (raw.wahSens);
    s.glBeats    = kDivBeats[IDX (V (raw.glDiv), kNumDivs - 1)];
    s.gateBeats  = kDivBeats[IDX (V (raw.gateDiv), kNumDivs - 1)];
    s.gatePat    = (int) (V (raw.gatePat) + 0.5f);
    s.tsTime     = V (raw.tsTime);    s.frzMs = V (raw.frzSize);
    s.choRate    = V (raw.choRate);   s.choDepth = V (raw.choDepth);
    s.flRate     = V (raw.flRate);    s.flFb = V (raw.flFb);
    s.phRate     = V (raw.phRate);    s.phFb = V (raw.phFb);
    s.trBeats    = kDivBeats[IDX (V (raw.trDiv), kNumDivs - 1)];
    s.trDepth    = V (raw.trDepth);   s.vibRate = V (raw.vibRate); s.vibDepth = V (raw.vibDepth);
    s.subFreq    = V (raw.subFreq);   s.subDecay = V (raw.subDecay); s.risTime = V (raw.risTime);
    const int dd = IDX (V (raw.dlyDiv), kNumDivs);
    s.dlyFree    = dd >= kNumDivs;
    s.dlyBeats   = kDivBeats[jmin (dd, kNumDivs - 1)];
    s.dlyMs      = V (raw.dlyMs);     s.dlyFb = V (raw.dlyFb); s.dlyTone = V (raw.dlyTone); s.dlyPing = V (raw.dlyPing);
    s.revDecay   = V (raw.revDecay);  s.revDamp = V (raw.revDamp); s.revPreMs = V (raw.revPre);
    s.shDecay    = V (raw.shDecay);   s.shFb = V (raw.shFb);       s.shSemis = V (raw.shInt);
}

void AccentFXProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    for (int ch = getTotalNumInputChannels(); ch < numCh; ++ch) buffer.clear (ch, 0, n);

    // ---- MIDI: any held note engages the accent (optional)
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn()) ++heldNotes;
        else if (m.isNoteOff()) heldNotes = jmax (0, heldNotes - 1);
        else if (m.isAllNotesOff() || m.isAllSoundOff()) heldNotes = 0;
    }
    const bool midiOn = raw.midi->load() > 0.5f;
    if (! midiOn) heldNotes = 0;

    // ---- bypass: fully bypassed => audio passes through untouched and the engine sleeps
    const bool byp = raw.bypass->load() > 0.5f;
    bypassSm.setTargetValue (byp ? 1.f : 0.f);
    const bool canFade = n <= dryBuf.getNumSamples() && numCh <= dryBuf.getNumChannels() && n <= (int) bypassRamp.size();
    if (byp && (! canFade || (! bypassSm.isSmoothing() && bypassSm.getCurrentValue() >= 1.f)))
    {
        if (! wasBypassed)
        {
            engine.reset();
            for (auto& a : uiAmt) a.store (0.f);
            uiEngage.store (0.f);
        }
        wasBypassed = true;
        prevEngaged = false;
        bypassSm.setCurrentAndTargetValue (1.f);
        return;
    }
    wasBypassed = false;

    const bool crossfade = canFade && (byp || bypassSm.isSmoothing() || bypassSm.getCurrentValue() > 0.f);
    if (crossfade)
        for (int ch = 0; ch < numCh; ++ch) dryBuf.copyFrom (ch, 0, buffer, ch, 0, n);

    const bool engagedNow = raw.engage->load() > 0.5f || (midiOn && heldNotes > 0);

    // ---- auto randomize on engage / release
    const int rm = (int) (raw.autoRand->load() + 0.5f);
    if ((rm == 1 && engagedNow && ! prevEngaged) || (rm == 2 && ! engagedNow && prevEngaged))
        triggerAsyncUpdate();
    prevEngaged = engagedNow;

    buildSettings (settings, engagedNow);

    // ---- tempo
    djx::Tempo t;
    bool playing = false; double bpm = 120.0, ppq = 0.0; bool hasPpq = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            playing = pos->getIsPlaying();
            if (auto p = pos->getPpqPosition()) { ppq = *p; hasPpq = true; }
        }
    t.bpm = bpm;
    t.playing = playing && hasPpq;
    t.beat = t.playing ? ppq : freeBeat;
    const double beatPerSample = jlimit (20.0, 400.0, bpm) / (60.0 * currentSampleRate);
    freeBeat += (double) n * beatPerSample;

    if (numCh >= 2)
    {
        engine.process (buffer.getWritePointer (0), buffer.getWritePointer (1), n, settings, t);
    }
    else if (numCh == 1)
    {
        float* L = buffer.getWritePointer (0);
        const int chunk = (int) monoR.size();
        djx::Tempo tc = t;
        for (int pos = 0; pos < n; pos += chunk)
        {
            const int m = jmin (chunk, n - pos);
            std::copy (L + pos, L + pos + m, monoR.begin());
            engine.process (L + pos, monoR.data(), m, settings, tc);
            tc.beat += (double) m * beatPerSample;
        }
    }

    if (crossfade)
    {
        for (int i = 0; i < n; ++i) bypassRamp[(size_t) i] = bypassSm.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* out = buffer.getWritePointer (ch);
            const float* dry = dryBuf.getReadPointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float m = bypassRamp[(size_t) i];
                out[i] = out[i] * (1.f - m) + dry[i] * m;
            }
        }
    }

    for (int i = 0; i < djx::kNumFx; ++i) uiAmt[i].store (settings.amt[(size_t) i]);
    uiEngage.store (engine.engageGain());
    uiGateStep.store (engine.gateStep());
}

// ------------------------------------------------------------------------------------------
void AccentFXProcessor::setCurrentProgram (int index)
{
    const auto& ps = params::presets();
    if (! isPositiveAndBelow (index, (int) ps.size())) return;
    currentProgram = index;

    for (auto& d : params::defs())
        if (d.inPreset)
            if (auto* p = apvts.getParameter (d.id)) p->setValueNotifyingHost (p->getDefaultValue());

    for (auto& kv : ps[(size_t) index].v)
        if (auto* p = apvts.getParameter (kv.first))
            p->setValueNotifyingHost (p->convertTo0to1 (kv.second));
}

const String AccentFXProcessor::getProgramName (int index)
{
    const auto& ps = params::presets();
    return isPositiveAndBelow (index, (int) ps.size()) ? String (ps[(size_t) index].name) : String();
}

// ------------------------------------------------------------------------------------------
void AccentFXProcessor::randomizeNow()
{
    using namespace djx;
    Random& R = Random::getSystemRandom();
    const float chaos = raw.chaos->load();
    auto flag = [this] (int i) { return apvts.getRawParameterValue ("rnd" + String (i))->load() > 0.5f; };
    auto setN = [this] (const std::string& id, float norm)
    {
        if (auto* p = apvts.getParameter (id)) p->setValueNotifyingHost (jlimit (0.f, 1.f, norm));
    };

    std::vector<int> pool;     // effects belonging to enabled categories
    bool anyOn = false;
    for (int i = 0; i < kNumFx; ++i)
        if (flag (kFxCategory[i])) pool.push_back (i);

    for (auto& d : params::defs())
    {
        if (d.group < 0 || d.group > 5 || ! flag (d.group)) continue;
        auto* p = apvts.getParameter (d.id);
        if (p == nullptr) continue;

        if (d.id.rfind ("a_", 0) == 0)                       // effect amount
        {
            const bool on = R.nextFloat() < 0.10f + 0.55f * chaos;
            anyOn = anyOn || on;
            setN (d.id, on ? 0.4f + 0.6f * R.nextFloat() : 0.f);
        }
        else if (d.kind == params::kChoice || d.kind == params::kInt)
        {
            if (d.id == "gatePat") continue;
            if (R.nextFloat() < 0.35f + 0.65f * chaos) setN (d.id, R.nextFloat());
        }
        else
        {
            const float cur = p->getValue(), target = R.nextFloat();
            setN (d.id, cur + (target - cur) * (0.35f + 0.65f * chaos));
        }
    }

    if (! anyOn && ! pool.empty())                           // never produce a silent patch
        setN (params::amtId (pool[(size_t) R.nextInt ((int) pool.size())]), 0.6f + 0.4f * R.nextFloat());

    if (flag (6))                                            // pad corners: four different effects
    {
        std::vector<int> src = pool.size() >= 4 ? pool : std::vector<int>();
        if (src.empty()) for (int i = 0; i < kNumFx; ++i) src.push_back (i);
        const char* ids[4] = { "padTL", "padTR", "padBL", "padBR" };
        for (int k = 0; k < 4; ++k)
        {
            const int j = R.nextInt ((int) src.size());
            const int fx = src[(size_t) j];
            src.erase (src.begin() + j);
            setN (ids[k], (float) (fx + 1) / (float) kNumFx);   // choice has kNumFx+1 entries => norm = idx / kNumFx
        }
    }

    if (flag (7))                                            // trance gate pattern
    {
        int pat = 0;
        const float dens = 0.35f + 0.4f * chaos;
        for (int b = 0; b < 16; ++b) if (R.nextFloat() < dens) pat |= (1 << b);
        pat |= 1;
        if (juce::BigInteger ((juce::int64) pat).countNumberOfSetBits() < 3) pat |= 0x0110;
        if (auto* p = apvts.getParameter ("gatePat")) p->setValueNotifyingHost (p->convertTo0to1 ((float) pat));
    }
}

// ------------------------------------------------------------------------------------------
void AccentFXProcessor::getStateInformation (MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void AccentFXProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = ValueTree::fromXml (*xml);
            currentProgram = (int) vt.getProperty ("program", 0);
            apvts.replaceState (vt);
        }
}

AudioProcessorEditor* AccentFXProcessor::createEditor() { return new AccentFXEditor (*this); }
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AccentFXProcessor(); }
