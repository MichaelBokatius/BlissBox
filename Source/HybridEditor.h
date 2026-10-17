#pragma once
#include "PluginEditor.h"
#include <set>

// Both views edit the same APVTS. The browser never processes audio.
class HybridEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit HybridEditor(BlissBoxAudioProcessor&);
    ~HybridEditor() override;
    void resized() override;
    void paint(juce::Graphics&) override;

private:
    class Browser final : public juce::WebBrowserComponent
    {
    public:
        explicit Browser(const Options& options) : WebBrowserComponent(options) {}
        bool pageAboutToLoad(const juce::String& url) override
        { return url == getResourceProviderRoot() || url == getResourceProviderRoot() + "index.html"; }
        void newWindowAttemptingToLoad(const juce::String&) override {}
    };
    void timerCallback() override;
    void handleControl(const juce::var&);
    void endGestures();
    void showBrowser(bool);
    static std::optional<juce::WebBrowserComponent::Resource> resource(const juce::String&);

    BlissBoxAudioProcessor& processor;
    BlissBoxAudioProcessorEditor classic;
    class ViewToggle final : public juce::TextButton
    {
    public:
        bool modern = true;
        void paintButton(juce::Graphics& g, bool, bool) override
        {
            g.setFont(14.0f);
            g.setColour(juce::Colours::grey);
            g.drawText("UI:", 0, 0, 24, getHeight(), juce::Justification::centredLeft);
            g.setColour(modern ? juce::Colours::grey : juce::Colours::white);
            g.drawText("Classic", 26, 0, 48, getHeight(), juce::Justification::centred);
            g.setColour(juce::Colours::grey);
            g.drawText("|", 76, 0, 12, getHeight(), juce::Justification::centred);
            g.setColour(modern ? juce::Colours::white : juce::Colours::grey);
            g.drawText("Modern", 90, 0, 54, getHeight(), juce::Justification::centred);
        }
    } viewToggle;
    juce::Label status;
    std::set<juce::String> gestures;
    std::unique_ptr<Browser> browser;
    bool webVisible = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HybridEditor)
};
