#include "PluginEditor.h"

using namespace juce;
using namespace ui;

// =====================================================================================
//  PLAY page   (page area is 1100 x 704)
// =====================================================================================
PlayPage::PlayPage (AccentFXProcessor& p)
    : proc (p), pad (p), keys (p), accent (p)
{
    addAndMakeVisible (pad);
    for (int i = 0; i < 4; ++i)
    {
        readouts[i] = std::make_unique<CornerReadout> (p, i);
        addAndMakeVisible (*readouts[i]);
    }

    const char* names[8] = { "Pitch", "FX", "Glitch", "Mod", "Gen", "Time", "Pad", "Gate" };
    for (int i = 0; i < 8; ++i)
    {
        const Colour c = i < 6 ? params::catColour (i) : (i == 6 ? cAccent() : cAccent2());
        pills.push_back (std::make_unique<PillToggle> (p, "rnd" + String (i), names[i], c));
        addAndMakeVisible (*pills.back());
    }

    addAndMakeVisible (keys);
    addAndMakeVisible (accent);
    addAndMakeVisible (shuffleBtn); addAndMakeVisible (prevBtn); addAndMakeVisible (nextBtn);
    shuffleBtn.onClick = [this] { proc.randomizeNow(); };
    prevBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + n - 1) % n); };
    nextBtn.onClick = [this] { proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % proc.getNumPrograms()); };

    chaos = std::make_unique<ParamControl> (p.apvts, "chaos", "Chaos", cAccent());
    addAndMakeVisible (*chaos);
}

void PlayPage::resized()
{
    pad.setBounds (300, 0, 500, 500);

    readouts[0]->setBounds (34, 96, 230, 26);    // TL
    readouts[1]->setBounds (34, 128, 230, 26);   // TR
    readouts[2]->setBounds (34, 366, 230, 26);   // BL
    readouts[3]->setBounds (34, 398, 230, 26);   // BR

    shuffleBtn.setBounds (940, 142, 36, 32);
    prevBtn.setBounds (984, 142, 36, 32);
    nextBtn.setBounds (1028, 142, 36, 32);

    for (int i = 0; i < 8; ++i)
        pills[(size_t) i]->setBounds (846 - i * 16, 196 + i * 42, 220, 34);

    chaos->setBounds (24, 592, 74, 90);
    keys.setBounds (130, 556, 800, 144);
    accent.setBounds (962, 556, 122, 144);
}

void PlayPage::tick (float dt)
{
    pad.tick (dt);
    for (auto& r : readouts) r->update();

    // activity of every pill
    float cat[6] = {};
    for (int i = 0; i < djx::kNumFx; ++i)
        cat[djx::kFxCategory[i]] = jmax (cat[djx::kFxCategory[i]], proc.uiAmt[i].load());
    for (int i = 0; i < 6; ++i) pills[(size_t) i]->setActivity (cat[i]);
    const float x = proc.apvts.getRawParameterValue ("padX")->load() - 0.5f, y = proc.apvts.getRawParameterValue ("padY")->load() - 0.5f;
    pills[6]->setActivity (std::sqrt (x * x + y * y) * 2.f);
    pills[7]->setActivity ((float) BigInteger ((int64) keys.value()).countNumberOfSetBits() / 16.f);

    keys.repaint();
    accent.repaint();

    const String name = proc.getProgramName (proc.getCurrentProgram());
    if (name != shownPreset) { shownPreset = name; repaint(); }
}

void PlayPage::paint (Graphics& g)
{
    // connector lines from the left read-outs to the pad rim (like the reference)
    const auto in = pad.getBounds().toFloat();
    const float R = jmin (in.getWidth(), in.getHeight()) * 0.5f;
    const auto c = in.getCentre();
    g.setColour (cLine().brighter (0.25f));
    auto connect = [&] (Rectangle<int> from, Rectangle<int> to, float ang)
    {
        const float y0 = (float) (from.getCentreY() + to.getCentreY()) * 0.5f;
        const float x0 = (float) from.getX() + 14.f;
        const Point<float> rim (c.x + std::cos (ang) * R, c.y + std::sin (ang) * R);
        Path p;
        p.startNewSubPath (x0, y0); p.lineTo (x0 + 240.f, y0); p.lineTo (rim.x - 18.f, rim.y); p.lineTo (rim.x, rim.y);
        g.strokePath (p, PathStrokeType (1.f));
        g.fillEllipse (rim.x - 2.5f, rim.y - 2.5f, 5.f, 5.f);
    };
    // thin underline of the group, as in the reference
    g.drawLine (30.f, (float) readouts[1]->getBottom() + 4.f, 270.f, (float) readouts[1]->getBottom() + 4.f, 1.f);
    g.drawLine (30.f, (float) readouts[3]->getBottom() + 4.f, 270.f, (float) readouts[3]->getBottom() + 4.f, 1.f);
    connect (readouts[0]->getBounds(), readouts[1]->getBounds(), -2.4f);
    connect (readouts[2]->getBounds(), readouts[3]->getBounds(), 2.4f);

    // preset block (right)
    g.setColour (cMuted()); g.setFont (14.f);
    g.drawText ("Accent FX Presets", Rectangle<int> (846, 28, 240, 20), Justification::centredLeft);
    g.setColour (cText()); g.setFont (30.f);
    g.setFont (g.getCurrentFont().boldened());
    g.drawFittedText (shownPreset.toUpperCase(), Rectangle<int> (846, 50, 240, 84), Justification::topLeft, 2, 0.8f);

    g.setColour (cMuted()); g.setFont (11.f);
    g.drawText ("TRANCE GATE  -  click or drag the keys", Rectangle<int> (130, 536, 400, 16), Justification::centredLeft);
    g.drawText ("RANDOMIZE POOL", Rectangle<int> (846 - 7 * 16, 176, 220, 16), Justification::centredRight);

    // logo mark bottom left
    Path m; m.startNewSubPath (30.f, 540.f); m.lineTo (46.f, 512.f); m.lineTo (62.f, 540.f); m.lineTo (54.f, 540.f); m.lineTo (46.f, 526.f); m.lineTo (38.f, 540.f); m.closeSubPath();
    g.setColour (cText().withAlpha (0.9f)); g.strokePath (m, PathStrokeType (1.6f));
}

// =====================================================================================
//  RACK page (1100 x 704)
// =====================================================================================
RackPage::RackPage (AccentFXProcessor& p)
{
    for (int i = 0; i < djx::kNumFx; ++i)
    {
        strips.push_back (std::make_unique<FxStrip> (p, i));
        addAndMakeVisible (*strips.back());
    }
}

void RackPage::resized()
{
    static const int rowCount[5] = { 6, 6, 6, 6, 4 };
    auto area = getLocalBounds().reduced (10, 8);
    const int rowH = area.getHeight() / 5;
    int idx = 0;
    for (int r = 0; r < 5; ++r)
    {
        auto row = area.removeFromTop (rowH);
        // wider cards for effects with many controls
        float total = 0.f; float wgt[6];
        for (int k = 0; k < rowCount[r]; ++k)
        {
            const int nd = (int) params::fxInfo()[(size_t) (idx + k)].details.size();
            wgt[k] = nd >= 5 ? 1.45f : 1.f; total += wgt[k];
        }
        float x = (float) row.getX();
        for (int k = 0; k < rowCount[r]; ++k)
        {
            const float w = (float) row.getWidth() * wgt[k] / total;
            strips[(size_t) (idx + k)]->setBounds ((int) x, row.getY(), (int) w, row.getHeight());
            x += w;
        }
        idx += rowCount[r];
    }
}

void RackPage::tick() { for (auto& s : strips) s->refresh(); }

// =====================================================================================
//  SETUP page (1100 x 704)
// =====================================================================================
SetupPage::SetupPage (AccentFXProcessor& p)
{
    auto add = [&] (std::vector<std::unique_ptr<ParamControl>>& v, const char* id, const char* cap, Colour c)
    {
        v.push_back (std::make_unique<ParamControl> (p.apvts, id, cap, c));
        addAndMakeVisible (*v.back());
    };
    add (knobs, "fade", "Engage Fade", cAccent());
    add (knobs, "chaos", "Chaos", cAccent());
    add (knobs, "padDepth", "Pad Depth", cAccent2());
    add (combos, "engMode", "Footswitch Mode", cAccent());
    add (combos, "autoRand", "Auto Randomize", cAccent());
    add (combos, "padMode", "Pad Mode", cAccent());
    add (toggles, "midi", "MIDI Hold = Accent", cAccent());
    add (toggles, "padTouch", "Touch Pad = Accent", cAccent());
    add (toggles, "padSpring", "Pad Springs Back", cAccent());
}

void SetupPage::resized()
{
    auto b = getLocalBounds().withSizeKeepingCentre (760, 520);
    b.removeFromTop (60);
    auto k = b.removeFromTop (120);
    const int kw = k.getWidth() / 3;
    for (int i = 0; i < 3; ++i) knobs[(size_t) i]->setBounds (k.removeFromLeft (kw).reduced (50, 0));
    b.removeFromTop (24);
    auto c = b.removeFromTop (64);
    const int cw = c.getWidth() / 3;
    for (int i = 0; i < 3; ++i) combos[(size_t) i]->setBounds (c.removeFromLeft (cw).reduced (16, 8));
    b.removeFromTop (20);
    auto t = b.removeFromTop (44);
    const int tw = t.getWidth() / 3;
    for (int i = 0; i < 3; ++i) toggles[(size_t) i]->setBounds (t.removeFromLeft (tw).reduced (16, 4));
}

void SetupPage::paint (Graphics& g)
{
    auto b = getLocalBounds().withSizeKeepingCentre (760, 520).toFloat();
    g.setColour (cPanel().withAlpha (0.85f)); g.fillRoundedRectangle (b, 18.f);
    g.setColour (cLine()); g.drawRoundedRectangle (b, 18.f, 1.2f);
    g.setColour (cText()); g.setFont (20.f);
    g.drawText ("SETUP", b.removeFromTop (60.f).withTrimmedLeft (28.f), Justification::centredLeft);
    g.setColour (cMuted()); g.setFont (12.5f);
    g.drawText ("Tip: map the Accent / Bypass parameters to a MIDI footswitch in your DAW, or enable MIDI Hold and play any note.",
                b.removeFromBottom (60.f).reduced (28.f, 10.f), Justification::centredLeft);
}

// =====================================================================================
//  EDITOR
// =====================================================================================
AccentFXEditor::AccentFXEditor (AccentFXProcessor& p)
    : AudioProcessorEditor (&p), proc (p), presetLabel (p), bypass (p), play (p), rack (p), setup (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);

    for (auto* t : { &tabPlay, &tabRack, &tabSetup }) content.addAndMakeVisible (*t);
    tabPlay.onClick  = [this] { showPage (0); };
    tabRack.onClick  = [this] { showPage (1); };
    tabSetup.onClick = [this] { showPage (2); };

    content.addAndMakeVisible (presetLabel);
    content.addAndMakeVisible (presetPrev);
    content.addAndMakeVisible (presetNext);
    presetPrev.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + n - 1) % n); presetLabel.repaint(); };
    presetNext.onClick = [this] { proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % proc.getNumPrograms()); presetLabel.repaint(); };

    content.addAndMakeVisible (bypass);

    for (auto* s : { &inSlider, &outSlider })
    {
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        s->setPopupDisplayEnabled (true, true, nullptr);
        content.addAndMakeVisible (*s);
    }
    inAtt  = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "inGain", inSlider);
    outAtt = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "outGain", outSlider);
    inSlider.setDoubleClickReturnValue (true, 0.0);
    outSlider.setDoubleClickReturnValue (true, 0.0);

    content.addAndMakeVisible (play);
    content.addAndMakeVisible (rack);
    content.addAndMakeVisible (setup);
    showPage (0);

    setResizable (true, true);
    setResizeLimits (kDesignW * 2 / 3, kDesignH * 2 / 3, kDesignW * 2, kDesignH * 2);
    getConstrainer()->setFixedAspectRatio ((double) kDesignW / (double) kDesignH);
    setSize (kDesignW, kDesignH);
    startTimerHz (30);
}

AccentFXEditor::~AccentFXEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void AccentFXEditor::showPage (int i)
{
    page = i;
    play.setVisible (i == 0);
    rack.setVisible (i == 1);
    setup.setVisible (i == 2);
    tabPlay.selected = i == 0; tabRack.selected = i == 1; tabSetup.selected = i == 2;
    tabPlay.repaint(); tabRack.repaint(); tabSetup.repaint();
}

void AccentFXEditor::paint (Graphics& g)
{
    g.fillAll (cBg());
    g.setGradientFill (ColourGradient (cBgTop(), 0.f, 0.f, cBg(), 0.f, (float) getHeight() * 0.62f, false));
    g.fillAll();
}

void AccentFXEditor::paintContent (Graphics& g)
{
    // top bar
    g.setColour (cText());
    g.setFont (22.f);
    g.setFont (g.getCurrentFont().boldened().withExtraKerningFactor (0.14f));
    g.drawText ("ACCENT FX", Rectangle<int> (24, 8, 220, 40), Justification::centredLeft);

    g.setColour (cLine());
    g.drawLine (0.f, 56.f, (float) kDesignW, 56.f, 1.f);
    g.drawLine (80.f, 56.f, 80.f, (float) kDesignH, 1.f);

    g.setColour (cMuted()); g.setFont (11.f);
    g.drawText ("IN",  Rectangle<int> (770, 16, 24, 24), Justification::centredRight);
    g.drawText ("OUT", Rectangle<int> (904, 16, 30, 24), Justification::centredRight);
}

void AccentFXEditor::resized()
{
    content.setBounds (0, 0, kDesignW, kDesignH);
    content.setTransform (AffineTransform::scale ((float) getWidth() / (float) kDesignW));

    tabPlay.setBounds (0, 76, 80, 70);
    tabRack.setBounds (0, 150, 80, 70);
    tabSetup.setBounds (0, 224, 80, 70);

    presetPrev.setBounds (372, 14, 30, 28);
    presetLabel.setBounds (410, 14, 260, 28);
    presetNext.setBounds (678, 14, 30, 28);

    inSlider.setBounds (798, 16, 90, 24);
    outSlider.setBounds (940, 16, 90, 24);
    bypass.setBounds (1048, 14, 116, 28);

    const juce::Rectangle<int> pageArea (80, 57, kDesignW - 80, kDesignH - 57);
    play.setBounds (pageArea);
    rack.setBounds (pageArea);
    setup.setBounds (pageArea);
}

void AccentFXEditor::timerCallback()
{
    if (page == 0) play.tick (1.f / 30.f);
    if (page == 1) rack.tick();
    bypass.repaint();
}
