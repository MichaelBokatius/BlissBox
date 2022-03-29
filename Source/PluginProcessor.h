#pragma once

#include <JuceHeader.h>

#define DEBUG

//==============================================================================
/**
*/
class BlissBoxAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    BlissBoxAudioProcessor();
    ~BlissBoxAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;


    // BLISSBOX ====================================================================

	    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

	    float hostBPM{ 100.0f };                    
	    float delayFeedback{ 0.0 }; // feedback in dB for delay feedback volume            
	    float delayMIX{ 0.5 };                      
	    float delayTimeInSamplesL{ 10000.0f };      
	    float delayTimeInSamplesR{ 10000.0f };      
	    float globalRawVolume{ 1.0f }; // volume applied in processBlock for volume correction
	    float currentSampleRate{ 44100.0f }; ;      

	    void updateChorus(); 
        void updateDelay(juce::AudioBuffer<float>& buffer, int totalNumInputChannels);
	    void updateReverb(); 
        void updateLimiter(juce::AudioBuffer<float>& buffer, juce::dsp::ProcessContextReplacing<float>);

        // functions to get and set values in the PluginEditor
        float getParameterValueFloat(juce::String paramName) {
            return *apvts.getRawParameterValue(paramName);
        }
        juce::String getParameterValueString(juce::String paramName) {
            auto choiceNameCast = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramName));
            juce::String choiceName = choiceNameCast->getCurrentChoiceName();
            return choiceName;
        }
        bool getParameterValueBool(juce::String paramName) {
            return *apvts.getRawParameterValue(paramName);
        }

	    //TO DO:
	    //void getHostInformation(); // grabs host-information like bpm, recording sate and playing state
	    //void BlissBoxAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

private:

        juce::LinearSmoothedValue<float> reverb_mix_smoothed{ 0.0f }; 

        juce::dsp::DryWetMixer<float> dryWet, dryWetLimiter;

        juce::dsp::Chorus<float> chorus;
        juce::AudioPlayHead::CurrentPositionInfo positionInfo;

        void fillBuffer(juce::AudioBuffer<float>& buffer, int channel);
        void readBuffer(juce::AudioBuffer<float>& buffer, int channel, juce::AudioBuffer<float>& delayBuffer);
        juce::AudioBuffer<float> delayBuffer;
        int writePosition{ 0 };

        juce::dsp::Reverb::Parameters paramsReverb;    
        juce::dsp::Reverb leftReverb, rightReverb;     

        juce::dsp::Limiter<float> limiter;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlissBoxAudioProcessor)
};
