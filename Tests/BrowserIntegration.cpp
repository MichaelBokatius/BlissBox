#include "HybridEditor.h"

// Runs the real embedded WebView and APVTS, without opening audio devices.
class BrowserIntegration final : public juce::JUCEApplication, private juce::Timer,
                                 private juce::AudioProcessorParameter::Listener
{
public:
    const juce::String getApplicationName() override { return "BlissBox browser integration"; }
    const juce::String getApplicationVersion() override { return "1"; }
    void initialise(const juce::String&) override
    {
        report = juce::File::getCurrentWorkingDirectory().getChildFile("build/browser-test-report.txt");
        report.replaceWithText("");
        processor = std::make_unique<BlissBoxAudioProcessor>();
        for (auto* p : processor->getParameters()) p->addListener(this);
        editor = std::make_unique<HybridEditor>(*processor);
        editor->addToDesktop(juce::ComponentPeer::windowIsTemporary);
        editor->setVisible(true);
        for (auto* child : editor->getChildren())
        {
            if (auto* b = dynamic_cast<juce::WebBrowserComponent*>(child)) browser = b;
            if (auto* b = dynamic_cast<juce::TextButton*>(child)) toggle = b;
            if (auto* c = dynamic_cast<BlissBoxAudioProcessorEditor*>(child)) classic = c;
        }
        if (!browser || !toggle || !classic) { finish(false, "WebView2 unavailable"); return; }
        startTimer(100);
    }
    void shutdown() override
    {
        editor.reset();
        if (processor) for (auto* p : processor->getParameters()) p->removeListener(this);
        processor.reset();
    }
private:
    void parameterValueChanged(int, float) override {}
    void parameterGestureChanged(int, bool starting) override { starting ? ++begins : ++ends; }
    void finish(bool pass, const juce::String& message)
    {
        stopTimer();
        report.appendText((pass ? "PASS: " : "FAIL: ") + message + "\n");
        setApplicationReturnValue(pass ? 0 : 1);
        quit();
    }
    bool check(bool condition, const juce::String& message)
    {
        if (!condition) { finish(false, message); return false; }
        report.appendText("PASS: " + message + "\n");
        return true;
    }
    void evaluate(const juce::String& script, std::function<void(bool)> callback)
    {
        pending = true;
        browser->evaluateJavascript(script, [this, callback](auto result) {
            pending = false;
            callback(result.getResult() && (bool) *result.getResult());
        });
    }
    void timerCallback() override
    {
        if (++ticks > 300) { finish(false, "Timed out at phase " + juce::String(phase)); return; }
        if (pending) return;
        switch (phase)
        {
        case 0:
            evaluate("document.querySelectorAll('.control').length === 15 && !document.querySelector('.control').disabled && !!document.querySelector('#art svg')", [this](bool ready) {
                if (ready) { check(true, "Embedded assets loaded and native bridge connected"); ++phase; }
            });
            break;
        case 1:
            evaluate("(() => { const knob = document.querySelector('.knob'); knob.dispatchEvent(new PointerEvent('pointerenter')); const shown = activeKnob?.id === knob.id; knob.dispatchEvent(new PointerEvent('pointerleave')); return shown && activeKnob === null && getComputedStyle(knob.querySelector('.value')).display === 'none' && !document.querySelector('#pedal').classList.contains('bypassed'); })()", [this](bool ok) { if (check(ok, "Knob hover readout clears on exit, tooltip hidden, sun initially enabled")) phase = 11; });
            break;
        case 11:
            evaluate("(() => { document.querySelectorAll('.knob').forEach(e => e.dispatchEvent(new KeyboardEvent('keydown', {key:'End'}))); document.querySelectorAll('.footswitch').forEach(e => e.click()); return true; })()", [this](bool ok) { if (check(ok, "Browser control events dispatched")) phase = 2; });
            break;
        case 2:
            for (auto* p : processor->getParameters())
                if (!check(std::abs(p->getValue() - 1.0f) < .0001f, "Browser -> engine: " + p->getName(100))) return;
            if (!check(begins == 15 && ends == 15, "All one-shot controls notify host with balanced gestures")) return;
            evaluate("(() => { document.querySelectorAll('.knob').forEach(e => e.dispatchEvent(new MouseEvent('dblclick'))); document.querySelectorAll('.footswitch').forEach(e => e.click()); return true; })()", [this](bool ok) { if (check(ok, "Reset events dispatched")) ++phase; });
            break;
        case 3:
            for (auto* p : processor->getParameters())
                if (!check(std::abs(p->getValue() - ((p->getName(100).startsWith("Delay_Time") || p->getName(100) == "Reverb_Width" || p->getName(100) == "Reverb_EQMode") ? p->getDefaultValue() : 0.0f)) < .0001f, "Double-click reset: " + p->getName(100))) return;
            processor->getStateInformation(saved);
            for (auto* p : processor->getParameters()) p->setValueNotifyingHost(1.0f);
            ++phase;
            break;
        case 4:
            evaluate("[...document.querySelectorAll('.knob')].every(e => Number(e.getAttribute('aria-valuenow')) === Number(e.getAttribute('aria-valuemax'))) && [...document.querySelectorAll('.footswitch')].every(e => e.getAttribute('aria-pressed') === 'true') && document.querySelector('#pedal').classList.contains('bypassed') && getComputedStyle(document.querySelector('.bb-sun-dot')).fill === 'rgb(0, 0, 0)'", [this](bool ok) {
                if (check(ok, "Host automation -> all browser controls")) { processor->setStateInformation(saved.getData(), (int) saved.getSize()); ++phase; }
            });
            break;
        case 5:
            evaluate("Number(document.querySelector('#Chorus_Rate').getAttribute('aria-valuenow')) === 0 && document.querySelector('#Reverb_Freeze').getAttribute('aria-pressed') === 'false'", [this](bool ok) {
                if (check(ok, "Preset state restore -> browser controls")) { toggle->triggerClick(); ++phase; }
            });
            break;
        case 6:
            if (!check(classic->isVisible() && !browser->isVisible(), "Top-right toggle selects classic editor")) return;
            for (auto* child : classic->getChildren())
                if (auto* slider = dynamic_cast<juce::Slider*>(child))
                    if (slider->getX() == 166 && slider->getY() == 96) slider->setValue(7.5, juce::sendNotificationSync);
            if (!check(std::abs(processor->getParameterValueFloat("Chorus_Rate") - 7.5f) < .001f, "Classic knob -> same engine parameter")) return;
            toggle->triggerClick(); ++phase;
            break;
        case 7:
            if (!check(browser->isVisible() && !classic->isVisible(), "Top-right toggle returns to browser")) return;
            evaluate("Number(document.querySelector('#Chorus_Rate').getAttribute('aria-valuenow')) === 7.5", [this](bool ok) {
                if (check(ok, "Classic edits retained in browser")) ++phase;
            });
            break;
        case 8:
            evaluate("(() => { window.__JUCE__.backend.emitEvent('control', {id:'Chorus_Rate',action:'begin'}); window.__JUCE__.backend.emitEvent('control', {id:'Chorus_Rate',action:'set',value:8}); return true; })()", [this](bool ok) { if (check(ok, "Drag gesture started")) ++phase; });
            break;
        case 9:
            if (!check(begins == ends + 1, "Drag holds one host gesture open")) return;
            toggle->triggerClick(); ++phase;
            break;
        case 10:
            if (!check(begins == ends, "Switching editors closes active gesture")) return;
            editor.reset();
            editor = std::make_unique<HybridEditor>(*processor);
            if (!check(!(bool)processor->apvts.state.getProperty("browserEditor"), "Editor choice retained on reopen")) return;
            finish(true, "Browser integration complete");
            break;
        }
    }
    juce::File report;
    std::unique_ptr<BlissBoxAudioProcessor> processor;
    std::unique_ptr<HybridEditor> editor;
    juce::WebBrowserComponent* browser = nullptr;
    juce::TextButton* toggle = nullptr;
    BlissBoxAudioProcessorEditor* classic = nullptr;
    juce::MemoryBlock saved;
    int phase = 0, ticks = 0, begins = 0, ends = 0;
    bool pending = false;
};
START_JUCE_APPLICATION(BrowserIntegration)


