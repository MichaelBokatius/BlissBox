
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
using namespace juce;

//==============================================================================
/**
*/
class BlissBoxAudioProcessorEditor  : public juce::AudioProcessorEditor, public Slider::Listener, public Button::Listener, public Timer
{

public:
    BlissBoxAudioProcessorEditor (BlissBoxAudioProcessor&);
    ~BlissBoxAudioProcessorEditor() override {};

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================

        void sliderValueChanged(Slider* sliderThatWasMoved) override;
        void buttonClicked(Button* buttonThatWasClicked) override;
        void timerCallback() override {};
        void setSliderComponent(Slider& slider, std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, juce::String paramName);
        void setSliderComponentNoPopUpDisplay(Slider& slider, std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, juce::String paramName);

        bool repaintFlag = false;
        bool Footswitch_BypassPressed = false, Footswitch_FreezePressed = false, Footswitch_LimiterPressed = false;

private:

    BlissBoxAudioProcessor& audioProcessor;
   
        Image background_png;                           
        Image led_On_png;                               
        Image led_Off_png;

        Image filmroll_Chorus_Rate_png;      
        Image filmroll_Chorus_Depth_png;     
        Image filmroll_Chorus_Feedback_png;  
        Image filmroll_Chorus_Mix_png; 

        Image filmroll_Delay_L_png;          
        Image filmroll_Delay_R_png;          
        Image filmroll_Delay_Feedback_png;   
        Image filmroll_Delay_Mix_png; 

        Image filmroll_Reverb_Size_png;      
        Image filmroll_Reverb_Width_png;         
        Image filmroll_Reverb_EQMode_png;        
        Image filmroll_Reverb_Mix_png;    

        Slider Slider_ChorusRate_Slider;
        Slider Slider_ChorusDepth_Slider;
        Slider Slider_ChorusFeedback_Slider;
        Slider Slider_ChorusMix_Slider;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_ChorusRate;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_ChorusDepth;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_ChorusFeedback;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_ChorusMix;

        Slider Slider_Delay_TimeL_Slider;
        Slider Slider_Delay_TimeR_Slider;
        Slider Slider_DelayFeedback_Slider;
        Slider Slider_DelayMix_Slider;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_Delay_TimeL;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_Delay_TimeR;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_DelayFeedback;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_DelayMix;

        Slider Slider_ReverbSize_Slider;
        Slider Switch_ReverbEQMode_Slider;
        Slider Switch_ReverbWidth_Slider;
        Slider Slider_ReverbMix_Slider;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_ReverbSize;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Switch_ReverbEQMode;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Switch_ReverbWidth;
        std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment> Slider_ReverbMix;

        ImageButton Footswitch_Limiter_Button;
        ImageButton Footswitch_ReverbFreeze_Button;
        ImageButton Footswitch_Bypass_Button;
        std::unique_ptr <AudioProcessorValueTreeState::ButtonAttachment> Footswitch_Limiter;
        std::unique_ptr <AudioProcessorValueTreeState::ButtonAttachment> Footswitch_ReverbFreeze;
        std::unique_ptr <AudioProcessorValueTreeState::ButtonAttachment> Footswitch_Bypass;

        ImageButton LED_Button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlissBoxAudioProcessorEditor)
};
