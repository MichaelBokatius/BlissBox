
#include "PluginProcessor.h"
#include "PluginEditor.h"


juce::AudioProcessorValueTreeState::ParameterLayout BlissBoxAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

	// Chorus - Rate, Depth, Feedback, Mix
	layout.add(std::make_unique<juce::AudioParameterFloat>("Chorus_Rate", "Chorus_Rate", juce::NormalisableRange<float>(0.0f, 10.0f, 0.25f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Chorus_Depth", "Chorus_Depth", juce::NormalisableRange<float>(0.0f, 0.05f, 0.001f), 0.018f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Chorus_Feedback", "Chorus_Feedback", juce::NormalisableRange<float>(0.0f, 100.f, 1.f), 20.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Chorus_Mix", "Chorus_Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 25.f));

	// Delay - TimeL, TimeR, Feedback, Mix
	layout.add(std::make_unique<juce::AudioParameterFloat>("Delay_TimeL", "Delay_TimeL", juce::NormalisableRange<float>(0.0f, 6.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Delay_TimeR", "Delay_TimeR", juce::NormalisableRange<float>(0.0f, 6.0f, 1.0f, 1.0f), 3.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Delay_Feedback", "Delay_Feedback", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Delay_Mix", "Delay_Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.f));

	// Reverb - Size, EQ-Mode, Space, Mix, Freeze
	layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb_Size", "Reverb_Size", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 0.3f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb_Width", "Reverb_Width", juce::NormalisableRange<float>(0.0f, 2.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb_EQMode", "Reverb_EQMode", juce::NormalisableRange<float>(0.0f, 2.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb_Mix", "Reverb_Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 25.f));
	layout.add(std::make_unique<juce::AudioParameterBool>("Reverb_Freeze", "Reverb_Freeze", false));

	layout.add(std::make_unique<juce::AudioParameterBool>("Limiter_OnOff", "Limiter_OnOff", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("Bypass_OnOff", "Bypass_OnOff", false)); // true = bypass

    return layout;
}

//==============================================================================
BlissBoxAudioProcessor::BlissBoxAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{

}

BlissBoxAudioProcessor::~BlissBoxAudioProcessor()
{
}

//==============================================================================
const juce::String BlissBoxAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BlissBoxAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BlissBoxAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool BlissBoxAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double BlissBoxAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BlissBoxAudioProcessor::getNumPrograms()
{
    return 1;   
}

int BlissBoxAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BlissBoxAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String BlissBoxAudioProcessor::getProgramName(int index)
{
    return {};
}

void BlissBoxAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void BlissBoxAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    reverb_mix_smoothed.reset(sampleRate, 0.0005);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    dryWet.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    dryWetLimiter.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    dryWet.prepare(spec);
    dryWetLimiter.prepare(spec);

    chorus.prepare(spec);
    chorus.reset();

    delayFeedback = 0.0f;
    auto delayBufferSize = sampleRate * 25.0f; // samplerate equals 1 second of delayTime                   
    delayBuffer.setSize(getTotalNumOutputChannels(), (int)delayBufferSize);

    leftReverb.prepare(spec);
    rightReverb.prepare(spec);
    leftReverb.reset();
    rightReverb.reset();

    limiter.prepare(spec);
    limiter.reset();
}

void BlissBoxAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BlissBoxAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

//==============================================================================
//==============================================================================

void BlissBoxAudioProcessor::fillBuffer(juce::AudioBuffer<float>& buffer, int channel) {

    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    if (delayBufferSize >= bufferSize + writePosition) {
        delayBuffer.copyFrom(channel, writePosition, buffer.getWritePointer(channel), bufferSize);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - writePosition;
        delayBuffer.copyFrom(channel, writePosition, buffer.getWritePointer(channel), numSamplesToEnd);

        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        delayBuffer.copyFrom(channel, 0, buffer.getWritePointer(channel, numSamplesToEnd), numSamplesAtStart);
    }
}

void BlissBoxAudioProcessor::readBuffer(juce::AudioBuffer<float>& buffer, int channel,
    juce::AudioBuffer<float>& delayBuffer)
{
    float rawFeedback = *apvts.getRawParameterValue("Delay_Feedback");
    float rawFeedbackMapped = juce::jmap(rawFeedback, 0.0f, 100.f, 0.0f, 1.0f);
    delayFeedback = rawFeedbackMapped;

    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    auto delayTime = channel == 0 ? delayTimeInSamplesL : delayTimeInSamplesR;
    auto readPosition = std::round(writePosition - (delayTime));

    if (readPosition < 0) { readPosition += delayBufferSize; }

    if (readPosition + bufferSize < delayBufferSize) {
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, delayFeedback, delayFeedback);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - readPosition;
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, delayFeedback, delayFeedback);
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        buffer.addFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, delayFeedback, delayFeedback);
    }
}

void BlissBoxAudioProcessor::updateChorus()
{
    chorus.setRate(*apvts.getRawParameterValue("Chorus_Rate"));
    chorus.setDepth(*apvts.getRawParameterValue("Chorus_Depth"));
    chorus.setCentreDelay(1.0f);

    float rawFeedback = *apvts.getRawParameterValue("Chorus_Feedback");
    float rawFeedbackMapped = juce::jmap(rawFeedback, 0.0f, 100.f, 0.0f, 0.7f);
    chorus.setFeedback(rawFeedbackMapped);

    float rawMix = *apvts.getRawParameterValue("Chorus_Mix");
    float rawMixMapped = juce::jmap(rawMix, 0.0f, 100.0f, 0.0f, 1.0f);
    chorus.setMix(rawMixMapped);
}

void BlissBoxAudioProcessor::updateDelay(juce::AudioBuffer<float>& buffer, int totalNumInputChannels)
{
    // drywet grabs sample-block before processing
    juce::dsp::AudioBlock<float> inputBlock(buffer);
    dryWet.pushDrySamples(inputBlock);

    // calculate amount of samples of decay for fixed note delaytime
    float beatsPerSecond = hostBPM / 60;
    float secondsPerBeat = 1 / beatsPerSecond; 
    float samplesOfDelay = secondsPerBeat * currentSampleRate;
    float samplesOfDelayPerBar = samplesOfDelay * 4; // 1 bar = 4 beats

    // hardcoded 
    // possible choices: { "1/1", "1/2", "1/4", "1/8", "1/3", "1/6", "1/12" };
    float delay_TimeL_Selection = *apvts.getRawParameterValue("Delay_TimeL");
    float delay_TimeR_Selection = *apvts.getRawParameterValue("Delay_TimeR");

    if (delay_TimeL_Selection == 0.f) { delayTimeInSamplesL = samplesOfDelayPerBar; }       // 1/1
    if (delay_TimeL_Selection == 1.f) { delayTimeInSamplesL = samplesOfDelay * 2; }         // 1/2
    if (delay_TimeL_Selection == 3.f) { delayTimeInSamplesL = samplesOfDelay * 1; }         // 1/4
    if (delay_TimeL_Selection == 5.f) { delayTimeInSamplesL = samplesOfDelay * 0.5; }       // 1/8
    if (delay_TimeL_Selection == 2.f) { delayTimeInSamplesL = samplesOfDelayPerBar / 3; }   // 1/3
    if (delay_TimeL_Selection == 4.f) { delayTimeInSamplesL = samplesOfDelayPerBar / 6; }   // 1/6
    if (delay_TimeL_Selection == 6.f) { delayTimeInSamplesL = samplesOfDelayPerBar / 12; }  // 1/12

    if (delay_TimeR_Selection == 0.f) { delayTimeInSamplesR = samplesOfDelayPerBar; }
    if (delay_TimeR_Selection == 1.f) { delayTimeInSamplesR = samplesOfDelay * 2; }
    if (delay_TimeR_Selection == 3.f) { delayTimeInSamplesR = samplesOfDelay * 1; }
    if (delay_TimeR_Selection == 5.f) { delayTimeInSamplesR = samplesOfDelay * 0.5; }
    if (delay_TimeR_Selection == 2.f) { delayTimeInSamplesR = samplesOfDelayPerBar / 3; }
    if (delay_TimeR_Selection == 4.f) { delayTimeInSamplesR = samplesOfDelayPerBar / 6; }
    if (delay_TimeR_Selection == 6.f) { delayTimeInSamplesR = samplesOfDelayPerBar / 12; }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        fillBuffer(buffer, channel);
        readBuffer(buffer, channel, delayBuffer);
        fillBuffer(buffer, channel); // feedback loop (without delay acts as simple one-tap echo effect )             
    }

    writePosition += buffer.getNumSamples();        
    writePosition %= delayBuffer.getNumSamples();   

    // drywet grabs sample-block after processing and applies mix proportion
    juce::dsp::AudioBlock<float> outputBlock(buffer);
    dryWet.mixWetSamples(outputBlock);

    float rawMix = *apvts.getRawParameterValue("Delay_Mix");
    float rawMixMapped = juce::jmap(rawMix, 0.0f, 100.0f, 0.0f, 1.0f);
    delayMIX = rawMixMapped;
    dryWet.setWetMixProportion(rawMixMapped);

    //DBG("Write Position: " << writePosition);
    //DBG("DelayTIME: " << delayTIME);
}

void BlissBoxAudioProcessor::updateReverb()
{
    paramsReverb.roomSize = *apvts.getRawParameterValue("Reverb_Size");
    
    float reverb_Width_Selection = *apvts.getRawParameterValue("Reverb_Width");
    float reverb_EQMode_Selection = *apvts.getRawParameterValue("Reverb_EQMode");

    if (reverb_Width_Selection == 0.f) { paramsReverb.width = 0.90; }
    if (reverb_Width_Selection == 1.f) { paramsReverb.width = 0.48f; }
    if (reverb_Width_Selection == 2.f) { paramsReverb.width = 0.10f; }

    if (reverb_EQMode_Selection == 0.f) { paramsReverb.damping = 0.01f; }
    if (reverb_EQMode_Selection == 1.f) { paramsReverb.damping = 0.45f; }
    if (reverb_EQMode_Selection == 2.f) { paramsReverb.damping = 0.65f; }

    float rawMix = *apvts.getRawParameterValue("Reverb_Mix");
    reverb_mix_smoothed.setTargetValue(rawMix);
    float rawMixMapped = juce::jmap(reverb_mix_smoothed.getNextValue(), 0.0f, 100.0f, 0.0f, 1.0f);

    paramsReverb.wetLevel = rawMixMapped;
    paramsReverb.dryLevel = 1.0f - rawMixMapped;

	/*  The Reverb class works with scale factors to change the volume.
        This results in volume differences with each mix position.
        In order to keep the volume at a consistent level, the dry volume has
        to be lowerd by 6 dB if the mix is fully dry. Otherwise the signal
        is clipping. The mapping applies -6 dB at the global volume if
        the mix is 100% dry and 0 dB if the mix is 100% wet.*/

    float reverbMixPercent = 1.0f - rawMixMapped;
    float volumeCorrection = 0.0f; 

    if (reverbMixPercent > 0) {
        volumeCorrection = juce::mapToLog10<float>(reverbMixPercent, 0.1, 6) * -1; 
    }
    else
    {
        volumeCorrection = juce::Decibels::decibelsToGain(6);                  
        volumeCorrection = juce::Decibels::gainToDecibels(volumeCorrection);    
    }
    globalRawVolume = juce::Decibels::decibelsToGain(volumeCorrection);

    //DBG("gainCorrection: " << gainCorrection);
    //DBG("globalRawVolume: " << globalRawVolume)
    paramsReverb.freezeMode = *apvts.getRawParameterValue("Reverb_Freeze");    

    leftReverb.setParameters(paramsReverb);
    rightReverb.setParameters(paramsReverb);
}

void BlissBoxAudioProcessor::updateLimiter(juce::AudioBuffer<float>& buffer, juce::dsp::ProcessContextReplacing<float>blockLimiter)
{
    if (*apvts.getRawParameterValue("Limiter_OnOff")) {
        limiter.setThreshold(-0.15); // -0.15 dB to prevent true peak values
        limiter.setRelease(100.0);
        limiter.process(juce::dsp::ProcessContextReplacing<float>(blockLimiter));
    }
}

void BlissBoxAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    // bypass option (could be improved by providing a seperate function such as processBlockBypassed)
    if (*apvts.getRawParameterValue("Bypass_OnOff")) {

        // if bypassed the plugin signal is not processed 
        for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
        {
            auto data = buffer.getWritePointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                data[i] = data[i];
            }
        }
    }
    else {

        juce::dsp::AudioBlock<float> block(buffer);
        using PCR_Float = juce::dsp::ProcessContextReplacing<float>;
        auto leftBlock = block.getSingleChannelBlock(0);            
        auto rightBlock = block.getSingleChannelBlock(1);               

        if (auto* playHead = getPlayHead())
        {
            juce::AudioPlayHead::CurrentPositionInfo info;  
            playHead->getCurrentPosition(info);             
            hostBPM = info.bpm;                             
        }

        updateChorus();
        chorus.process(juce::dsp::ProcessContextReplacing<float>(block));

        // DELAY
        // circular buffer design for delay based on tutorial from TheAudioProgrammer
        // https://www.youtube.com/c/TheAudioProgrammer/videos
        // added delay time choices based on host-bpm, seperate channel delay times
        // as well as dry/wet mixing
        updateDelay(buffer, totalNumInputChannels);

        updateReverb();
        PCR_Float leftContextReverb(leftBlock);
        PCR_Float rightContextReverb(rightBlock);
        leftReverb.process(leftContextReverb);   
        rightReverb.process(rightContextReverb);

        PCR_Float blockLimiter(block);
        updateLimiter(buffer, blockLimiter);

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                channelData[sample] = channelData[sample] * globalRawVolume;
            }
        }
    }
}


//==============================================================================
bool BlissBoxAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BlissBoxAudioProcessor::createEditor()
{
    // GENERIC GUI - for testing 
    //return new juce::GenericAudioProcessorEditor(*this);

    // CUSTOM GUI - final GUI
    return new BlissBoxAudioProcessorEditor (*this);
}

//==============================================================================
void BlissBoxAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // PARAM / custom parameters
    auto state = apvts.copyState();

    // copy xml to binary
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BlissBoxAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // PARAM / xml-State init
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    // copy params to XML
    if (xmlState.get() != nullptr) {

        // costum params
        if (xmlState->hasTagName(apvts.state.getType())) {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BlissBoxAudioProcessor();
}
