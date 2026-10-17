#include "HybridEditor.h"
#include <BinaryData.h>
#include <cmath>

HybridEditor::HybridEditor(BlissBoxAudioProcessor& p)
    : AudioProcessorEditor(p), processor(p), classic(p)
{
    addAndMakeVisible(classic);
    addAndMakeVisible(viewToggle);
    addAndMakeVisible(status);
    status.setColour(juce::Label::textColourId, juce::Colour(0xffb8b8b8));
    viewToggle.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff262626));
    viewToggle.onClick = [this] { showBrowser(!webVisible); };

    auto options = juce::WebBrowserComponent::Options{}
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
            .withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory)
                .getChildFile("BlissBox-WebView2")))
        .withKeepPageLoadedWhenBrowserIsHidden()
        .withNativeIntegrationEnabled()
        .withResourceProvider(resource)
        .withEventListener("control", [this](const juce::var& event) { handleControl(event); })
        .withEventListener("readout", [this](const juce::var& text) {
            if (webVisible && text.isString()) status.setText(text.toString(), juce::dontSendNotification);
        });
    if (juce::WebBrowserComponent::areOptionsSupported(options))
    {
        browser = std::make_unique<Browser>(options);
        addChildComponent(*browser);
        browser->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    }
    viewToggle.setEnabled(browser != nullptr);
    setSize(1280, 704);
    showBrowser(browser != nullptr && (bool) processor.apvts.state.getProperty("browserEditor", true));
    startTimerHz(30);
}

HybridEditor::~HybridEditor()
{
    stopTimer();
    endGestures();
    browser.reset();
}

void HybridEditor::showBrowser(bool show)
{
    endGestures();
    webVisible = show && browser != nullptr;
    classic.setVisible(!webVisible);
    if (browser) browser->setVisible(webVisible);
    viewToggle.modern = webVisible;
    viewToggle.setButtonText(webVisible ? "UI: Classic | Modern (active)" : "UI: Classic (active) | Modern");
    status.setText(browser ? "" : "Modern UI requires Microsoft Edge WebView2 Runtime", juce::dontSendNotification);
    if (browser) processor.apvts.state.setProperty("browserEditor", webVisible, nullptr);
    resized();
    repaint();
}

void HybridEditor::resized()
{
    viewToggle.setBounds(getWidth() - 154, 5, 144, 28);
    status.setBounds(10, 5, getWidth() - 290, 28);
    classic.setBounds(0, 40, 1280, 650);
    if (browser) browser->setBounds(0, 40, getWidth(), getHeight() - 40);
}

void HybridEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff0a0a0a)); }

std::optional<juce::WebBrowserComponent::Resource> HybridEditor::resource(const juce::String& url)
{
    const char* name = nullptr;
    juce::String mime;
    if (url == "/" || url == "/index.html") { name = "index_html"; mime = "text/html"; }
    else if (url == "/app.js") { name = "app_js"; mime = "text/javascript"; }
    else if (url == "/style.css") { name = "style_css"; mime = "text/css"; }
    else if (url == "/faceplate.svg") { name = "faceplate_svg"; mime = "image/svg+xml"; }
    if (name == nullptr) return std::nullopt;
    int size = 0;
    auto* data = BinaryData::getNamedResource(name, size);
    if (!data) return std::nullopt;
    auto* bytes = reinterpret_cast<const std::byte*>(data);
    return juce::WebBrowserComponent::Resource { {bytes, bytes + size}, mime };
}

void HybridEditor::endGestures()
{
    for (const auto& id : gestures)
        if (auto* parameter = processor.apvts.getParameter(id)) parameter->endChangeGesture();
    gestures.clear();
}

void HybridEditor::handleControl(const juce::var& event)
{
    if (!webVisible || !event.isObject()) return;
    auto id = event.getProperty("id", {}).toString();
    auto action = event.getProperty("action", {}).toString();
    auto* parameter = processor.apvts.getParameter(id);
    if (!parameter) return;
    if (action == "begin")
    {
        if (gestures.insert(id).second) parameter->beginChangeGesture();
    }
    else if (action == "end")
    {
        if (gestures.erase(id)) parameter->endChangeGesture();
    }
    else if (action == "set")
    {
        const auto value = event.getProperty("value", {});
        if (!(value.isDouble() || value.isInt() || value.isInt64())) return;
        const auto raw = (double) value;
        if (!std::isfinite(raw)) return;
        const bool oneShot = gestures.count(id) == 0;
        if (oneShot) parameter->beginChangeGesture();
        const auto& range = parameter->getNormalisableRange();
        const auto snapped = range.snapToLegalValue((float) juce::jlimit((double) range.start, (double) range.end, raw));
        parameter->setValueNotifyingHost(parameter->convertTo0to1(snapped));
        if (oneShot) parameter->endChangeGesture();
    }
}

void HybridEditor::timerCallback()
{
    const bool requested = (bool) processor.apvts.state.getProperty("browserEditor", true);
    if (browser && requested != webVisible) showBrowser(requested);
    if (!webVisible) status.setText(classic.getParameterReadout(), juce::dontSendNotification);
    if (!webVisible || !browser) return;
    juce::Array<juce::var> values;
    for (auto* base : processor.getParameters())
        if (auto* parameter = dynamic_cast<juce::RangedAudioParameter*>(base))
        {
            auto* item = new juce::DynamicObject;
            const auto& range = parameter->getNormalisableRange();
            item->setProperty("id", parameter->paramID);
            item->setProperty("value", parameter->convertFrom0to1(parameter->getValue()));
            item->setProperty("min", range.start);
            item->setProperty("max", range.end);
            item->setProperty("step", range.interval);
            item->setProperty("defaultValue", parameter->convertFrom0to1(parameter->getDefaultValue()));
            values.add(juce::var(item));
        }
    browser->emitEventIfBrowserIsVisible("parameters", values);
}
