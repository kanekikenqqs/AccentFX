// UI building blocks. Visual language: dark navy -> violet gradient, round "terrain" XY pad,
// pill shaped modules, vertical keys (inspired by the reference screenshot).
#pragma once
#include "PluginProcessor.h"
#include <functional>

namespace ui
{
using namespace juce;

inline Colour cBgTop()   { return Colour (0xff2b1356); }
inline Colour cBg()      { return Colour (0xff0d0e15); }
inline Colour cPanel()   { return Colour (0xff15161f); }
inline Colour cLine()    { return Colour (0xff2a2c3b); }
inline Colour cText()    { return Colour (0xffeceef7); }
inline Colour cMuted()   { return Colour (0xff868a9e); }
inline Colour cAccent()  { return Colour (0xff7b6cff); }
inline Colour cAccent2() { return Colour (0xff3fd5ff); }
inline Colour cWarn()    { return Colour (0xffff8a3d); }

inline void setBool (RangedAudioParameter* p, bool v)  { p->beginChangeGesture(); p->setValueNotifyingHost (v ? 1.f : 0.f); p->endChangeGesture(); }
inline void setChoice (RangedAudioParameter* p, int i) { p->beginChangeGesture(); p->setValueNotifyingHost (p->convertTo0to1 ((float) i)); p->endChangeGesture(); }
inline bool isOn (RangedAudioParameter* p)             { return p->getValue() > 0.5f; }
inline int  choiceIndex (RangedAudioParameter* p)      { return (int) (p->convertFrom0to1 (p->getValue()) + 0.5f); }

// ======================================================================================
class AccentLnF : public LookAndFeel_V4
{
public:
    AccentLnF()
    {
        setColour (ComboBox::backgroundColourId, cPanel());
        setColour (ComboBox::outlineColourId, cLine());
        setColour (ComboBox::textColourId, cText());
        setColour (ComboBox::arrowColourId, cMuted());
        setColour (PopupMenu::backgroundColourId, Colour (0xff181925));
        setColour (PopupMenu::textColourId, cText());
        setColour (PopupMenu::highlightedBackgroundColourId, cAccent().withAlpha (0.55f));
        setColour (PopupMenu::highlightedTextColourId, Colours::white);
        setColour (Label::textColourId, cMuted());
        setColour (Slider::trackColourId, cAccent());
        setColour (TextButton::buttonColourId, cPanel());
        setColour (TextButton::textColourOffId, cMuted());
        setColour (TextButton::textColourOnId, Colour (0xff101116));
    }

    void drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, Slider& s) override
    {
        auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
        const float r = jmin (b.getWidth(), b.getHeight()) * 0.5f;
        const auto c = b.getCentre();
        const float th = jmax (2.f, r * 0.13f), ar = r - th * 0.5f;

        Path track; track.addCentredArc (c.x, c.y, ar, ar, 0.f, a0, a1, true);
        g.setColour (cLine()); g.strokePath (track, PathStrokeType (th, PathStrokeType::curved, PathStrokeType::rounded));

        const float ang = a0 + pos * (a1 - a0);
        Path val; val.addCentredArc (c.x, c.y, ar, ar, 0.f, a0, ang, true);
        g.setColour (s.findColour (Slider::rotarySliderFillColourId));
        g.strokePath (val, PathStrokeType (th, PathStrokeType::curved, PathStrokeType::rounded));

        const float kr = r - th * 1.9f;
        g.setColour (Colour (0xff1c1d29)); g.fillEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f);
        g.setColour (cLine().brighter (0.2f)); g.drawEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f, 1.f);
        Path p; p.addRoundedRectangle (-1.4f, -kr + 2.f, 2.8f, kr * 0.5f, 1.4f);
        g.setColour (cText()); g.fillPath (p, AffineTransform::rotation (ang).translated (c.x, c.y));
    }

    void drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& box) override
    {
        auto b = Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f);
        g.setColour (box.findColour (ComboBox::backgroundColourId)); g.fillRoundedRectangle (b, 7.f);
        g.setColour (cLine().brighter (0.15f)); g.drawRoundedRectangle (b, 7.f, 1.f);
        Path p; const float ax = (float) w - 15.f, ay = (float) h * 0.5f - 2.f;
        p.addTriangle (ax, ay, ax + 8.f, ay, ax + 4.f, ay + 5.f);
        g.setColour (cMuted()); g.fillPath (p);
    }

    void drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        const bool on = b.getToggleState();
        Colour fill = on ? b.findColour (TextButton::buttonOnColourId) : cPanel();
        if (over) fill = fill.brighter (0.08f);
        if (down) fill = fill.darker (0.1f);
        g.setColour (fill); g.fillRoundedRectangle (r, 7.f);
        g.setColour (on ? fill : cLine().brighter (0.15f)); g.drawRoundedRectangle (r, 7.f, 1.f);
    }

    void drawLinearSlider (Graphics& g, int x, int y, int w, int h, float pos, float, float, Slider::SliderStyle, Slider& s) override
    {
        const float cy = (float) y + (float) h * 0.5f;
        g.setColour (cLine()); g.fillRoundedRectangle ((float) x, cy - 2.f, (float) w, 4.f, 2.f);
        g.setColour (s.findColour (Slider::trackColourId)); g.fillRoundedRectangle ((float) x, cy - 2.f, pos - (float) x, 4.f, 2.f);
        g.setColour (cText()); g.fillRoundedRectangle (pos - 3.f, cy - 8.f, 6.f, 16.f, 3.f);
    }
};

// ======================================================================================
// Generic parameter control (rotary / combo / toggle) bound through the APVTS
class ParamControl : public Component
{
public:
    ParamControl (AudioProcessorValueTreeState& s, const String& id, const String& caption, Colour col, bool showCaption = true)
        : showCap (showCaption)
    {
        auto* p = s.getParameter (id);
        jassert (p != nullptr);

        if (auto* cp = dynamic_cast<AudioParameterChoice*> (p))
        {
            combo = std::make_unique<ComboBox>();
            combo->addItemList (cp->choices, 1);
            addAndMakeVisible (*combo);
            comboAtt = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (s, id, *combo);
        }
        else if (dynamic_cast<AudioParameterBool*> (p) != nullptr)
        {
            button = std::make_unique<TextButton> (caption);
            button->setClickingTogglesState (true);
            button->setColour (TextButton::buttonOnColourId, col);
            addAndMakeVisible (*button);
            buttonAtt = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (s, id, *button);
            showCap = false;
        }
        else
        {
            slider = std::make_unique<Slider>();
            slider->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
            slider->setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
            slider->setColour (Slider::rotarySliderFillColourId, col);
            slider->setPopupDisplayEnabled (true, true, nullptr);
            addAndMakeVisible (*slider);
            sliderAtt = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (s, id, *slider);
            slider->setDoubleClickReturnValue (true, (double) p->convertFrom0to1 (p->getDefaultValue()));
        }

        if (showCap)
        {
            cap.setText (caption, dontSendNotification);
            cap.setJustificationType (Justification::centred);
            cap.setColour (Label::textColourId, cMuted());
            cap.setFont (cap.getFont().withHeight (11.f));
            cap.setInterceptsMouseClicks (false, false);
            addAndMakeVisible (cap);
        }
    }

    void resized() override
    {
        auto b = getLocalBounds();
        if (showCap) cap.setBounds (b.removeFromBottom (14));
        if (slider)
        {
            const int sz = jmin (b.getWidth(), b.getHeight());
            slider->setBounds (b.withSizeKeepingCentre (sz, sz));
        }
        if (combo)  combo->setBounds (b.withSizeKeepingCentre (b.getWidth(), jmin (26, b.getHeight())));
        if (button) button->setBounds (b.withSizeKeepingCentre (b.getWidth(), jmin (28, b.getHeight())));
    }

private:
    bool showCap;
    Label cap;
    std::unique_ptr<Slider> slider;
    std::unique_ptr<ComboBox> combo;
    std::unique_ptr<TextButton> button;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sliderAtt;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> comboAtt;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> buttonAtt;
};

// ======================================================================================
// One effect card for the Rack page: amount knob + detail knobs + activity bar
class FxStrip : public Component
{
public:
    FxStrip (AccentFXProcessor& p, int fxIndex)
        : proc (p), idx (fxIndex), col (params::catColour (djx::kFxCategory[fxIndex]))
    {
        amount = std::make_unique<ParamControl> (p.apvts, String (params::amtId (fxIndex)), "Amount", col);
        addAndMakeVisible (*amount);
        for (auto& id : params::fxInfo()[(size_t) fxIndex].details)
        {
            const auto* d = params::find (id);
            auto c = std::make_unique<ParamControl> (p.apvts, String (id), d != nullptr ? String (d->name) : String (id), col);
            addAndMakeVisible (*c);
            details.push_back (std::move (c));
        }
    }

    void refresh()
    {
        const float a = proc.uiAmt[idx].load();
        const bool e = proc.uiEngage.load() > 0.3f;
        if (std::abs (a - shown) > 0.004f || e != engaged) { shown = a; engaged = e; repaint(); }
    }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (2.f);
        g.setColour (cPanel()); g.fillRoundedRectangle (b, 12.f);
        g.setColour (col.withAlpha (0.18f + 0.5f * shown)); g.drawRoundedRectangle (b, 12.f, 1.2f);

        auto t = b.reduced (10.f, 6.f);
        g.setColour (cText()); g.setFont (14.f);
        g.drawText (String (djx::kFxNames[idx]), t.removeFromTop (20.f), Justification::centredLeft);

        auto bar = t.removeFromTop (6.f).reduced (0.f, 1.f);
        g.setColour (cLine()); g.fillRoundedRectangle (bar, 2.f);
        g.setColour (engaged ? col : col.withAlpha (0.45f));
        g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * shown), 2.f);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (6);
        b.removeFromTop (30);
        if (details.empty()) { amount->setBounds (b); return; }
        amount->setBounds (b.removeFromLeft ((int) ((float) b.getWidth() * 0.40f)));
        const int n = (int) details.size();
        const int cols = jlimit (1, n, b.getWidth() / 46);
        const int rows = (n + cols - 1) / cols;
        const int cw = b.getWidth() / cols, ch = b.getHeight() / rows;
        for (int i = 0; i < n; ++i)
            details[(size_t) i]->setBounds (b.getX() + (i % cols) * cw, b.getY() + (i / cols) * ch, cw, ch);
    }

private:
    AccentFXProcessor& proc;
    int idx;
    Colour col;
    float shown = 0.f;
    bool engaged = false;
    std::unique_ptr<ParamControl> amount;
    std::vector<std::unique_ptr<ParamControl>> details;
};

// ======================================================================================
// Round XY pad with an animated voxel-terrain texture, rings and a puck
class CircleXYPad : public Component, private Timer
{
public:
    explicit CircleXYPad (AccentFXProcessor& p) : proc (p)
    {
        px = p.apvts.getParameter ("padX");   py = p.apvts.getParameter ("padY");
        pe = p.apvts.getParameter ("engage"); pTouch = p.apvts.getParameter ("padTouch");
        pSpring = p.apvts.getParameter ("padSpring"); pByp = p.apvts.getParameter ("bypass");
        img = Image (Image::ARGB, kN, kN, true);
        heights.assign ((size_t) ((kN + 1) * (kN + 1)), 0.f);
        setMouseCursor (MouseCursor::CrosshairCursor);
    }

    Rectangle<float> outerCircle() const
    {
        auto b = getLocalBounds().toFloat().reduced (4.f);
        return b.withSizeKeepingCentre (jmin (b.getWidth(), b.getHeight()), jmin (b.getWidth(), b.getHeight()));
    }
    Rectangle<float> innerCircle() const { auto o = outerCircle(); return o.reduced (o.getWidth() * 0.0275f); }

    void tick (float dt)
    {
        float energy = 0.f;
        for (int i = 0; i < djx::kNumFx; ++i) energy += proc.uiAmt[i].load();
        energy = jmin (1.f, energy / 3.f);
        const float eng = proc.uiEngage.load();
        phase += dt * (0.35f + 1.6f * eng + 1.2f * energy);
        render (energy);
        repaint();
    }

    void paint (Graphics& g) override
    {
        const auto o = outerCircle(), in = innerCircle();
        const auto c = in.getCentre();
        const float R = in.getWidth() * 0.5f;
        const float eng = proc.uiEngage.load();
        const bool byp = isOn (pByp);

        g.setColour (Colour (0xff07080c)); g.fillEllipse (o);
        g.setColour (cLine()); g.drawEllipse (o.reduced (1.f), 1.4f);

        {
            Graphics::ScopedSaveState ss (g);
            Path clip; clip.addEllipse (in);
            g.reduceClipRegion (clip);
            g.setImageResamplingQuality (Graphics::lowResamplingQuality);   // chunky "voxel" look
            g.drawImage (img, (int) in.getX(), (int) in.getY(), (int) in.getWidth() + 1, (int) in.getHeight() + 1, 0, 0, kN, kN);
            g.setGradientFill (ColourGradient (Colours::transparentBlack, c.x, c.y, Colours::black.withAlpha (0.55f), c.x + R, c.y, true));
            g.fillEllipse (in);
            if (byp) { g.setColour (Colours::black.withAlpha (0.62f)); g.fillEllipse (in); }
        }

        // corner markers on the rim + weights
        float w[4];
        const float xv = px->getValue(), yv = py->getValue();
        params::padWeights (xv, yv, choiceIndexOf ("padMode"), w);
        const float depth = proc.apvts.getRawParameterValue ("padDepth")->load();
        const char* ids[4] = { "padTL", "padTR", "padBL", "padBR" };
        const float ang[4] = { -2.35619449f, -0.78539816f, 2.35619449f, 0.78539816f };
        const Point<float> pk = puckPos();
        for (int k = 0; k < 4; ++k)
        {
            const int fx = choiceIndexOf (ids[k]) - 1;
            const Colour cc = fx >= 0 ? params::catColour (djx::kFxCategory[fx]) : cMuted();
            const Point<float> m (c.x + std::cos (ang[k]) * (R + 1.f), c.y + std::sin (ang[k]) * (R + 1.f));
            const float wt = w[k] * depth;
            g.setColour (cc.withAlpha (0.12f + 0.6f * wt));
            g.drawLine (pk.x, pk.y, m.x, m.y, 1.f);
            const float rad = 4.f + 7.f * wt;
            g.setColour (Colour (0xff07080c)); g.fillEllipse (m.x - rad - 2.f, m.y - rad - 2.f, (rad + 2.f) * 2.f, (rad + 2.f) * 2.f);
            g.setColour (cc.withAlpha (0.35f + 0.65f * wt)); g.fillEllipse (m.x - rad, m.y - rad, rad * 2.f, rad * 2.f);
        }

        // concentric rings + puck
        const float pulse = 1.f + 0.07f * std::sin (phase * 3.f) * eng;
        for (float rr : { 70.f, 38.f })
        {
            const float r = rr * (R / 215.f) * 1.f * pulse;
            g.setColour (Colours::white.withAlpha (0.16f + 0.2f * eng));
            g.drawEllipse (pk.x - r, pk.y - r, r * 2.f, r * 2.f, 1.2f);
        }
        const float pr = 11.f;
        g.setColour (Colours::black.withAlpha (0.45f)); g.fillEllipse (pk.x - pr, pk.y - pr, pr * 2.f, pr * 2.f);
        g.setColour (Colours::white); g.drawEllipse (pk.x - pr, pk.y - pr, pr * 2.f, pr * 2.f, 2.f);
        if (eng > 0.05f)
        {
            g.setGradientFill (ColourGradient (cAccent2().withAlpha (0.55f * eng), pk.x, pk.y, Colours::transparentBlack, pk.x + 46.f, pk.y, true));
            g.fillEllipse (pk.x - 46.f, pk.y - 46.f, 92.f, 92.f);
        }
        if (byp)
        {
            g.setColour (cWarn()); g.setFont (16.f);
            g.drawText ("BYPASSED", in.withSizeKeepingCentre (200.f, 30.f).translated (0.f, R * 0.55f), Justification::centred);
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        springTimerOff();
        px->beginChangeGesture(); py->beginChangeGesture();
        gestureOpen = true;
        setFromMouse (e.position);
        if (isOn (pTouch) && ! isOn (pe))
        {
            touchEngaged = true;
            pe->beginChangeGesture(); pe->setValueNotifyingHost (1.f);
        }
    }
    void mouseDrag (const MouseEvent& e) override { setFromMouse (e.position); }
    void mouseUp (const MouseEvent&) override
    {
        if (touchEngaged) { pe->setValueNotifyingHost (0.f); pe->endChangeGesture(); touchEngaged = false; }
        if (isOn (pSpring)) startTimerHz (60);
        else closeGestures();
    }
    void mouseDoubleClick (const MouseEvent&) override
    {
        px->beginChangeGesture(); py->beginChangeGesture();
        px->setValueNotifyingHost (0.5f); py->setValueNotifyingHost (0.5f);
        px->endChangeGesture(); py->endChangeGesture();
    }

private:
    static constexpr int kN = 96;

    int choiceIndexOf (const char* id) const
    {
        return (int) (proc.apvts.getRawParameterValue (id)->load() + 0.5f);
    }
    Point<float> puckPos() const
    {
        const auto in = innerCircle(); const auto c = in.getCentre(); const float R = in.getWidth() * 0.5f;
        return { c.x + (px->getValue() - 0.5f) * 2.f * R, c.y - (py->getValue() - 0.5f) * 2.f * R };
    }
    void setFromMouse (Point<float> p)
    {
        const auto in = innerCircle(); const auto c = in.getCentre(); const float R = in.getWidth() * 0.5f;
        float dx = (p.x - c.x) / R, dy = (c.y - p.y) / R;
        const float len = std::sqrt (dx * dx + dy * dy);
        if (len > 1.f) { dx /= len; dy /= len; }
        px->setValueNotifyingHost (0.5f + 0.5f * dx);
        py->setValueNotifyingHost (0.5f + 0.5f * dy);
    }
    void closeGestures() { if (gestureOpen) { px->endChangeGesture(); py->endChangeGesture(); gestureOpen = false; } }
    void springTimerOff() { stopTimer(); closeGestures(); }
    void timerCallback() override
    {
        float x = px->getValue(), y = py->getValue();
        x += (0.5f - x) * 0.25f; y += (0.5f - y) * 0.25f;
        if (std::abs (x - 0.5f) < 0.004f && std::abs (y - 0.5f) < 0.004f) { x = y = 0.5f; stopTimer(); px->setValueNotifyingHost (x); py->setValueNotifyingHost (y); closeGestures(); return; }
        px->setValueNotifyingHost (x); py->setValueNotifyingHost (y);
    }

    void render (float energy)
    {
        const float t = phase;
        const float bx = px->getValue(), by = 1.f - py->getValue();
        const int S = kN + 1;
        for (int j = 0; j < S; ++j)
            for (int i = 0; i < S; ++i)
            {
                const float u = (float) i / (float) kN, v = (float) j / (float) kN;
                float h = 0.5f + 0.22f * std::sin (u * 8.f + t * 0.7f + 2.f * std::sin (v * 3.f + t * 0.4f))
                               + 0.18f * std::sin (v * 10.f - t * 0.55f + u * 4.f)
                               + 0.10f * std::sin ((u + v) * 19.f + t * 1.1f);
                const float d2 = (u - bx) * (u - bx) + (v - by) * (v - by);
                h += 0.38f * std::exp (-d2 * 26.f) * (0.45f + 0.55f * energy);
                h = jlimit (0.f, 1.f, h);
                heights[(size_t) (j * S + i)] = std::floor (h * 15.f) / 15.f;
            }

        Image::BitmapData bd (img, Image::BitmapData::writeOnly);
        for (int j = 0; j < kN; ++j)
            for (int i = 0; i < kN; ++i)
            {
                const float h = heights[(size_t) (j * S + i)];
                const float hx = heights[(size_t) (j * S + i + 1)] - h;
                const float hy = heights[(size_t) ((j + 1) * S + i)] - h;
                const float l = jlimit (0.3f, 1.45f, 0.85f - (hx + hy) * 7.f);
                float r, g, b;                          // violet -> blue -> cyan -> ice
                if (h < 0.45f)      { const float k = h / 0.45f;          r = 0.12f + 0.15f * k; g = 0.04f + 0.14f * k; b = 0.38f + 0.52f * k; }
                else if (h < 0.75f) { const float k = (h - 0.45f) / 0.3f; r = 0.27f - 0.13f * k; g = 0.18f + 0.50f * k; b = 0.90f + 0.10f * k; }
                else                { const float k = (h - 0.75f) / 0.25f; r = 0.14f + 0.55f * k; g = 0.68f + 0.30f * k; b = 1.f; }
                bd.setPixelColour (i, j, Colour::fromFloatRGBA (jmin (1.f, r * l), jmin (1.f, g * l), jmin (1.f, b * l), 1.f));
            }
    }

    AccentFXProcessor& proc;
    RangedAudioParameter *px, *py, *pe, *pTouch, *pSpring, *pByp;
    Image img;
    std::vector<float> heights;
    float phase = 0.f;
    bool touchEngaged = false, gestureOpen = false;
};

// ======================================================================================
// Left side readout: which effect lives in which corner + how much of it is active.
class CornerReadout : public Component
{
public:
    CornerReadout (AccentFXProcessor& p, int cornerIndex) : proc (p), corner (cornerIndex)
    {
        static const char* ids[4] = { "padTL", "padTR", "padBL", "padBR" };
        pc = p.apvts.getParameter (ids[cornerIndex]);
        setMouseCursor (MouseCursor::PointingHandCursor);
    }

    void update()
    {
        float w[4];
        params::padWeights (proc.apvts.getRawParameterValue ("padX")->load(), proc.apvts.getRawParameterValue ("padY")->load(),
                            (int) (proc.apvts.getRawParameterValue ("padMode")->load() + 0.5f), w);
        const float v = w[corner] * proc.apvts.getRawParameterValue ("padDepth")->load();
        const int fx = choiceIndex (pc) - 1;
        if (std::abs (v - weight) > 0.004f || fx != shownFx) { weight = v; shownFx = fx; repaint(); }
    }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const int fx = choiceIndex (pc) - 1;
        const Colour col = fx >= 0 ? params::catColour (djx::kFxCategory[fx]) : cMuted();

        auto ic = Rectangle<float> (0.f, b.getCentreY() - 7.f, 14.f, 14.f);
        g.setColour (cLine().brighter (0.5f)); g.drawRoundedRectangle (ic, 3.f, 1.2f);
        const float dx = (corner % 2 == 0) ? ic.getX() + 3.5f : ic.getRight() - 3.5f;
        const float dy = (corner < 2) ? ic.getY() + 3.5f : ic.getBottom() - 3.5f;
        g.setColour (col); g.fillEllipse (dx - 2.6f, dy - 2.6f, 5.2f, 5.2f);

        g.setFont (15.f);
        g.setColour (fx >= 0 ? cText() : cMuted());
        g.drawText (fx >= 0 ? String (djx::kFxNames[fx]) : String ("None"), b.withTrimmedLeft (26.f).withTrimmedRight (54.f), Justification::centredLeft);
        g.setColour (weight > 0.01f ? col : cMuted());
        g.drawText (String (roundToInt (weight * 100.f)) + "%", b.removeFromRight (52.f), Justification::centredRight);
    }

    void mouseDown (const MouseEvent&) override
    {
        PopupMenu m;
        m.addItem (1, "None", true, choiceIndex (pc) == 0);
        for (int cat = 0; cat < 6; ++cat)
        {
            PopupMenu sub;
            for (int i = 0; i < djx::kNumFx; ++i)
                if (djx::kFxCategory[i] == cat) sub.addItem (i + 2, djx::kFxNames[i], true, choiceIndex (pc) == i + 1);
            m.addSubMenu (params::kCatNames[cat], sub);
        }
        Component::SafePointer<CornerReadout> sp (this);
        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this), [sp] (int r)
        {
            if (sp != nullptr && r > 0) setChoice (sp->pc, r - 1);
        });
    }

private:
    AccentFXProcessor& proc;
    int corner;
    RangedAudioParameter* pc;
    float weight = 0.f;
    int shownFx = -2;
};

// ======================================================================================
// Pill: power glyph + name + activity bars. Bound to a randomizer-pool flag.
class PillToggle : public Component
{
public:
    PillToggle (AccentFXProcessor& p, const String& paramId, const String& text, Colour c)
        : pr (p.apvts.getParameter (paramId)), label (text), col (c)
    {
        setMouseCursor (MouseCursor::PointingHandCursor);
    }
    void setActivity (float a) { a = jlimit (0.f, 1.f, a); if (std::abs (a - activity) > 0.01f) { activity = a; repaint(); } }

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.f);
        const bool on = isOn (pr);
        g.setColour (cPanel().withAlpha (0.92f)); g.fillRoundedRectangle (b, b.getHeight() * 0.5f);
        g.setColour (on ? col.withAlpha (0.55f) : cLine()); g.drawRoundedRectangle (b, b.getHeight() * 0.5f, 1.3f);

        // power glyph
        const auto c = Point<float> (b.getX() + b.getHeight() * 0.5f, b.getCentreY());
        const float r = 6.f;
        Path pw; pw.addCentredArc (c.x, c.y, r, r, 0.f, 0.65f, MathConstants<float>::twoPi - 0.65f, true);
        pw.startNewSubPath (c.x, c.y - r - 1.f); pw.lineTo (c.x, c.y - 1.f);
        g.setColour (on ? col : cMuted().withAlpha (0.6f));
        g.strokePath (pw, PathStrokeType (1.8f, PathStrokeType::curved, PathStrokeType::rounded));

        g.setColour (on ? cText() : cMuted().withAlpha (0.7f)); g.setFont (14.f);
        g.drawText (label, b.withTrimmedLeft (b.getHeight()).withTrimmedRight (46.f), Justification::centred);

        // two little level bars
        for (int k = 0; k < 2; ++k)
        {
            const float bh = 16.f, bw = 4.f;
            const float x = b.getRight() - 40.f + (float) k * 9.f, y = b.getCentreY() - bh * 0.5f;
            g.setColour (cLine()); g.fillRoundedRectangle (x, y, bw, bh, 2.f);
            const float lv = (k == 0 ? activity : activity * 0.7f) * bh;
            g.setColour (on ? col : cMuted().withAlpha (0.4f)); g.fillRoundedRectangle (x, y + bh - lv, bw, lv, 2.f);
        }
    }
    void mouseDown (const MouseEvent&) override { setBool (pr, ! isOn (pr)); repaint(); }

private:
    RangedAudioParameter* pr;
    String label;
    Colour col;
    float activity = 0.f;
};

// ======================================================================================
// 16 vertical keys = the trance-gate step pattern. Dots above show the playing step.
class GateKeys : public Component
{
public:
    explicit GateKeys (AccentFXProcessor& p) : proc (p) { pat = p.apvts.getParameter ("gatePat"); }

    int  value() const { return (int) (pat->convertFrom0to1 (pat->getValue()) + 0.5f); }
    void setValue (int v) { pat->setValueNotifyingHost (pat->convertTo0to1 ((float) v)); }

    Rectangle<float> keyRect (int i) const
    {
        auto b = getLocalBounds().toFloat();
        const float dots = 28.f, gap = 7.f;
        const float kw = (b.getWidth() - gap * 15.f) / 16.f;
        return { b.getX() + (float) i * (kw + gap), b.getY() + dots, kw, b.getHeight() - dots };
    }

    void paint (Graphics& g) override
    {
        const int v = value(), cur = proc.uiGateStep.load();
        const bool live = proc.uiEngage.load() > 0.2f;
        for (int i = 0; i < 16; ++i)
        {
            const auto k = keyRect (i);
            const bool on = ((v >> i) & 1) != 0, isCur = live && cur == i;

            const float dr = isCur ? 6.f : 5.f;
            g.setColour (isCur ? cAccent2() : (i % 4 == 0 ? cLine().brighter (0.5f) : cLine().brighter (0.2f)));
            g.fillEllipse (k.getCentreX() - dr, k.getY() - 16.f - dr, dr * 2.f, dr * 2.f);

            const float rad = k.getWidth() * 0.5f;
            if (on) g.setGradientFill (ColourGradient (cAccent().brighter (0.35f), k.getX(), k.getY(), cAccent().darker (0.35f), k.getX(), k.getBottom(), false));
            else    g.setColour (Colour (0xff3f4356));
            g.fillRoundedRectangle (k, rad);
            if (isCur) { g.setColour (Colours::white.withAlpha (0.9f)); g.drawRoundedRectangle (k.reduced (1.f), rad, 2.f); }
            if (i % 4 == 0)
            {
                auto lab = k;
                g.setColour (on ? Colours::white.withAlpha (0.9f) : cMuted()); g.setFont (11.f);
                g.drawText (String (i + 1), lab.removeFromBottom (20.f), Justification::centred);
            }
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        const int i = indexAt (e.position.x);
        paintState = ((value() >> i) & 1) == 0;
        pat->beginChangeGesture();
        apply (i);
    }
    void mouseDrag (const MouseEvent& e) override { apply (indexAt (e.position.x)); }
    void mouseUp (const MouseEvent&) override { pat->endChangeGesture(); }

private:
    int indexAt (float x) const { return jlimit (0, 15, (int) (x / ((float) getWidth() / 16.f))); }
    void apply (int i)
    {
        int v = value();
        v = paintState ? (v | (1 << i)) : (v & ~(1 << i));
        setValue (v); repaint();
    }
    AccentFXProcessor& proc;
    RangedAudioParameter* pat;
    bool paintState = true;
};

// ======================================================================================
// Big "ACCENT" footswitch (latch / momentary). Click the small label to flip the mode.
class AccentButton : public Component
{
public:
    explicit AccentButton (AccentFXProcessor& p) : proc (p)
    {
        pe = p.apvts.getParameter ("engage"); pm = p.apvts.getParameter ("engMode");
        setMouseCursor (MouseCursor::PointingHandCursor);
    }
    Rectangle<float> discArea() const { auto b = getLocalBounds().toFloat(); b.removeFromBottom (24.f); return b; }

    void paint (Graphics& g) override
    {
        auto d = discArea(); const float s = jmin (d.getWidth(), d.getHeight());
        const auto c = d.withSizeKeepingCentre (s, s).reduced (6.f);
        const float e = proc.uiEngage.load();
        if (e > 0.01f)
        {
            g.setGradientFill (ColourGradient (cAccent().withAlpha (0.6f * e), c.getCentreX(), c.getCentreY(), Colours::transparentBlack, c.getCentreX() + s * 0.8f, c.getCentreY(), true));
            g.fillEllipse (c.expanded (s * 0.3f));
        }
        g.setColour (cPanel()); g.fillEllipse (c);
        g.setColour (e > 0.5f ? cAccent2() : cLine().brighter (0.3f)); g.drawEllipse (c, 3.f);
        g.setColour (e > 0.5f ? cText() : cMuted()); g.setFont (15.f);
        g.drawText ("ACCENT", c, Justification::centred);

        g.setColour (cMuted()); g.setFont (12.f);
        g.drawText (isOn (pm) ? "HOLD" : "LATCH", getLocalBounds().removeFromBottom (24), Justification::centred);
    }
    void mouseDown (const MouseEvent& e) override
    {
        if (e.position.y > (float) getHeight() - 24.f) { setChoice (pm, isOn (pm) ? 0 : 1); repaint(); return; }
        if (isOn (pm)) { pe->beginChangeGesture(); pe->setValueNotifyingHost (1.f); held = true; }
        else           { pe->beginChangeGesture(); pe->setValueNotifyingHost (isOn (pe) ? 0.f : 1.f); pe->endChangeGesture(); }
    }
    void mouseUp (const MouseEvent&) override
    {
        if (held) { pe->setValueNotifyingHost (0.f); pe->endChangeGesture(); held = false; }
    }
private:
    AccentFXProcessor& proc;
    RangedAudioParameter *pe, *pm;
    bool held = false;
};

// ======================================================================================
class BypassButton : public Component
{
public:
    explicit BypassButton (AccentFXProcessor& p) : pb (p.apvts.getParameter ("bypass")) { setMouseCursor (MouseCursor::PointingHandCursor); }

    void paint (Graphics& g) override
    {
        const bool bypassed = isOn (pb);
        auto b = getLocalBounds().toFloat().reduced (1.f);
        const float rad = b.getHeight() * 0.5f;
        g.setColour (bypassed ? cWarn() : cPanel()); g.fillRoundedRectangle (b, rad);
        g.setColour (bypassed ? cWarn() : cLine().brighter (0.3f)); g.drawRoundedRectangle (b, rad, 1.4f);

        const auto c = Point<float> (b.getX() + rad + 2.f, b.getCentreY());
        const Colour ic = bypassed ? Colour (0xff1c0e03) : cAccent2();
        Path pw; pw.addCentredArc (c.x, c.y, 6.5f, 6.5f, 0.f, 0.65f, MathConstants<float>::twoPi - 0.65f, true);
        pw.startNewSubPath (c.x, c.y - 8.f); pw.lineTo (c.x, c.y - 1.f);
        g.setColour (ic); g.strokePath (pw, PathStrokeType (2.f, PathStrokeType::curved, PathStrokeType::rounded));

        g.setFont (13.f);
        g.setColour (bypassed ? Colour (0xff1c0e03) : cText());
        g.drawText (bypassed ? "BYPASSED" : "BYPASS", b.withTrimmedLeft (rad * 2.f + 4.f), Justification::centredLeft);
    }
    void mouseDown (const MouseEvent&) override { setBool (pb, ! isOn (pb)); repaint(); }
private:
    RangedAudioParameter* pb;
};

// ======================================================================================
class IconButton : public Button
{
public:
    enum Glyph { Shuffle, Prev, Next };
    explicit IconButton (Glyph gl) : Button (""), glyph (gl) { setMouseCursor (MouseCursor::PointingHandCursor); }

    void paintButton (Graphics& g, bool over, bool down) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (down ? cLine().brighter (0.25f) : (over ? cLine().brighter (0.1f) : Colours::transparentBlack));
        g.fillRoundedRectangle (b, 8.f);
        g.setColour (cLine().brighter (0.35f)); g.drawRoundedRectangle (b, 8.f, 1.3f);
        g.setColour (cText());
        const auto c = b.getCentre(); const float s = jmin (b.getWidth(), b.getHeight()) * 0.2f;
        if (glyph == Shuffle)
        {
            Path p;
            p.addArrow (Line<float> (c.x - s * 1.4f, c.y + s, c.x + s * 1.4f, c.y - s), 1.6f, 7.f, 6.f);
            p.addArrow (Line<float> (c.x - s * 1.4f, c.y - s, c.x + s * 1.4f, c.y + s), 1.6f, 7.f, 6.f);
            g.fillPath (p);
        }
        else
        {
            const float d = glyph == Prev ? -1.f : 1.f;
            Path p; p.startNewSubPath (c.x - d * s * 0.6f, c.y - s); p.lineTo (c.x + d * s * 0.6f, c.y); p.lineTo (c.x - d * s * 0.6f, c.y + s);
            g.strokePath (p, PathStrokeType (1.9f, PathStrokeType::curved, PathStrokeType::rounded));
        }
    }
private:
    Glyph glyph;
};

// ======================================================================================
class NavTab : public Component
{
public:
    NavTab (int iconType, const String& text) : icon (iconType), label (text) { setMouseCursor (MouseCursor::PointingHandCursor); }
    std::function<void()> onClick;
    bool selected = false;

    void paint (Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (selected) { g.setColour (cAccent()); g.fillRect (0.f, 6.f, 2.f, b.getHeight() - 12.f); }
        const auto c = Point<float> (b.getCentreX(), 26.f);
        g.setColour (selected ? cText() : cMuted());
        const PathStrokeType st (1.8f, PathStrokeType::curved, PathStrokeType::rounded);
        if (icon == 0)        // play: ring + dot
        {
            g.drawEllipse (c.x - 12.f, c.y - 12.f, 24.f, 24.f, 1.8f);
            g.fillEllipse (c.x + 3.f, c.y - 6.f, 4.f, 4.f); g.fillEllipse (c.x - 5.f, c.y + 3.f, 5.f, 5.f);
        }
        else if (icon == 1)   // rack: three faders
        {
            for (int i = 0; i < 3; ++i)
            {
                const float x = c.x - 9.f + (float) i * 9.f;
                Path p; p.startNewSubPath (x, c.y - 11.f); p.lineTo (x, c.y + 11.f); g.strokePath (p, st);
                g.fillRoundedRectangle (x - 4.f, c.y - 4.f + (float) ((i * 5) % 9) - 3.f, 8.f, 4.f, 1.5f);
            }
        }
        else                  // setup: sliders (horizontal)
        {
            for (int i = 0; i < 3; ++i)
            {
                const float y = c.y - 8.f + (float) i * 8.f;
                Path p; p.startNewSubPath (c.x - 12.f, y); p.lineTo (c.x + 12.f, y); g.strokePath (p, st);
                g.fillEllipse (c.x - 6.f + (float) ((i * 9) % 13), y - 3.f, 6.f, 6.f);
            }
        }
        g.setFont (12.f);
        g.drawText (label, getLocalBounds().removeFromBottom (24), Justification::centred);
    }
    void mouseDown (const MouseEvent&) override { if (onClick) onClick(); }
private:
    int icon; String label;
};

// preset name in the top bar, click for the list
class PresetLabel : public Component
{
public:
    explicit PresetLabel (AccentFXProcessor& p) : proc (p) { setMouseCursor (MouseCursor::PointingHandCursor); }
    void paint (Graphics& g) override
    {
        g.setColour (cText());
        g.setFont (15.f);
        g.drawText (String (proc.getProgramName (proc.getCurrentProgram())).toUpperCase(), getLocalBounds(), Justification::centred);
    }
    void mouseDown (const MouseEvent&) override
    {
        PopupMenu m;
        for (int i = 0; i < proc.getNumPrograms(); ++i) m.addItem (i + 1, proc.getProgramName (i), true, i == proc.getCurrentProgram());
        Component::SafePointer<PresetLabel> sp (this);
        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this), [sp] (int r)
        {
            if (sp != nullptr && r > 0) { sp->proc.setCurrentProgram (r - 1); sp->repaint(); }
        });
    }
private:
    AccentFXProcessor& proc;
};
} // namespace ui
