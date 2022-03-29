
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BlissBoxSlider.h"

using namespace juce;

//==============================================================================
BlissBoxAudioProcessorEditor::BlissBoxAudioProcessorEditor (BlissBoxAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Images
    background_png = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    led_On_png = juce::ImageCache::getFromMemory(BinaryData::led_on_png, BinaryData::led_on_pngSize);
    led_Off_png = juce::ImageCache::getFromMemory(BinaryData::led_off_png, BinaryData::led_off_pngSize);

    filmroll_Delay_L_png = juce::ImageCache::getFromMemory(BinaryData::delay_l_filmstrip_png, BinaryData::delay_l_filmstrip_pngSize);
    filmroll_Delay_R_png = juce::ImageCache::getFromMemory(BinaryData::delay_r_filmstrip_png, BinaryData::delay_r_filmstrip_pngSize);
    filmroll_Delay_Feedback_png = juce::ImageCache::getFromMemory(BinaryData::delay_feedback_filmstrip_png, BinaryData::delay_feedback_filmstrip_pngSize);
    filmroll_Delay_Mix_png = juce::ImageCache::getFromMemory(BinaryData::delay_mix_filmstrip_png, BinaryData::delay_mix_filmstrip_pngSize);

    filmroll_Reverb_Size_png = juce::ImageCache::getFromMemory(BinaryData::reverb_size_filmstrip_png, BinaryData::reverb_size_filmstrip_pngSize);
    filmroll_Reverb_Width_png = juce::ImageCache::getFromMemory(BinaryData::reverb_width_filmstrip_png, BinaryData::reverb_width_filmstrip_pngSize);
    filmroll_Reverb_EQMode_png = juce::ImageCache::getFromMemory(BinaryData::reverb_eq_filmstrip_png, BinaryData::reverb_eq_filmstrip_pngSize);
    filmroll_Reverb_Mix_png = juce::ImageCache::getFromMemory(BinaryData::reverb_mix_filmstrip_png, BinaryData::reverb_mix_filmstrip_pngSize);

    filmroll_Chorus_Rate_png = juce::ImageCache::getFromMemory(BinaryData::chorus_rate_filmstrip_png, BinaryData::chorus_rate_filmstrip_pngSize);
    filmroll_Chorus_Depth_png = juce::ImageCache::getFromMemory(BinaryData::chorus_depth_filmstrip_png, BinaryData::chorus_depth_filmstrip_pngSize);
    filmroll_Chorus_Feedback_png = juce::ImageCache::getFromMemory(BinaryData::chorus_feedback_filmstrip_png, BinaryData::chorus_feedback_filmstrip_pngSize);
    filmroll_Chorus_Mix_png = juce::ImageCache::getFromMemory(BinaryData::chorus_mix_filmstrip_png, BinaryData::chorus_mix_filmstrip_pngSize);

    // FOOTSWITCHES
    Footswitch_Limiter = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "Limiter_OnOff", Footswitch_Limiter_Button);
    Footswitch_Limiter_Button.addListener(this);
    Footswitch_Limiter_Button.setBounds(460, 436, 150, 150);
    Footswitch_Limiter_Button.setClickingTogglesState(true);
    Footswitch_Limiter_Button.setName("Limiter");
    addAndMakeVisible(&Footswitch_Limiter_Button);

    Footswitch_ReverbFreeze = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "Reverb_Freeze", Footswitch_ReverbFreeze_Button);
    Footswitch_ReverbFreeze_Button.addListener(this);
    Footswitch_ReverbFreeze_Button.setBounds(742, 387, 150, 150);
    Footswitch_ReverbFreeze_Button.setClickingTogglesState(true);
    Footswitch_ReverbFreeze_Button.setName("Freeze");
    addAndMakeVisible(&Footswitch_ReverbFreeze_Button);

    Footswitch_Bypass = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "Bypass_OnOff", Footswitch_Bypass_Button);
    Footswitch_Bypass_Button.addListener(this);
    Footswitch_Bypass_Button.setBounds(1018, 421, 150, 150);
    Footswitch_Bypass_Button.setClickingTogglesState(true);
    Footswitch_Bypass_Button.setName("Bypass");
    addAndMakeVisible(&Footswitch_Bypass_Button);

    // SLIDER
    setSliderComponent(Slider_ReverbSize_Slider, Slider_ReverbSize, "Reverb_Size");
    setSliderComponentNoPopUpDisplay(Switch_ReverbWidth_Slider, Switch_ReverbWidth, "Reverb_Width");
    setSliderComponentNoPopUpDisplay(Switch_ReverbEQMode_Slider, Switch_ReverbEQMode, "Reverb_EQMode");
    setSliderComponent(Slider_ReverbMix_Slider, Slider_ReverbMix, "Reverb_Mix");

    setSliderComponent(Slider_ChorusRate_Slider, Slider_ChorusRate, "Chorus_Rate");
    setSliderComponent(Slider_ChorusDepth_Slider, Slider_ChorusDepth, "Chorus_Depth");
    setSliderComponent(Slider_ChorusFeedback_Slider, Slider_ChorusFeedback, "Chorus_Feedback");
    setSliderComponent(Slider_ChorusMix_Slider, Slider_ChorusMix, "Chorus_Mix");

    setSliderComponentNoPopUpDisplay(Slider_Delay_TimeL_Slider, Slider_Delay_TimeL, "Delay_TimeL");
    setSliderComponentNoPopUpDisplay(Slider_Delay_TimeR_Slider, Slider_Delay_TimeR, "Delay_TimeR");
    setSliderComponent(Slider_DelayFeedback_Slider, Slider_DelayFeedback, "Delay_Feedback");
    setSliderComponent(Slider_DelayMix_Slider, Slider_DelayMix, "Delay_Mix");

    // LED
    LED_Button.addListener(this);
    LED_Button.setBounds(926, 236, 100, 100);
    addAndMakeVisible(&LED_Button);

    setSize (1280, 650);
    repaintFlag = true;
}

//==============================================================================
void BlissBoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    BlissBoxSlider filmstripSlider_ChorusRate(&filmroll_Chorus_Rate_png, 101);
    BlissBoxSlider filmstripSlider_ChorusDepth(&filmroll_Chorus_Depth_png, 101);
    BlissBoxSlider filmstripSlider_ChorusFeedback(&filmroll_Chorus_Feedback_png, 101);
    BlissBoxSlider filmstripSlider_ChorusMix(&filmroll_Chorus_Mix_png, 101);

    BlissBoxSlider filmstripSlider_DelayL(&filmroll_Delay_L_png, 7);
    BlissBoxSlider filmstripSlider_DelayR(&filmroll_Delay_R_png, 7);
    BlissBoxSlider filmstripSlider_DelayFeedback(&filmroll_Delay_Feedback_png, 101);
    BlissBoxSlider filmstripSlider_DelayMix(&filmroll_Delay_Mix_png, 101);

    BlissBoxSlider filmstripSlider_ReverbSize(&filmroll_Reverb_Size_png, 101);
    BlissBoxSlider filmstripSwitch_ReverbWidth(&filmroll_Reverb_Width_png, 3);
    BlissBoxSlider filmstripSwitch_ReverbEQMode(&filmroll_Reverb_EQMode_png, 3);
    BlissBoxSlider filmstripSlider_ReverbMix(&filmroll_Reverb_Mix_png, 101);


    g.drawImage(background_png, 0, 0, background_png.getWidth(), background_png.getHeight(), 0, 0,
        background_png.getWidth(), background_png.getHeight(), false);

	if (repaintFlag == true) {
		if (Footswitch_BypassPressed == true)
		{
			Footswitch_Bypass_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::bypass_on_png, BinaryData::bypass_on_pngSize), 1.000f, Colour(0x00000000), Image(), 1.000f, Colour(0x00000000), Image(), 1.000f, Colour(0x00000000));

			LED_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::led_off_png, BinaryData::led_off_pngSize), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000));

			repaintFlag = false;
		}
		else {
			Footswitch_Bypass_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::bypass_off_png, BinaryData::bypass_off_pngSize), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000));

			LED_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::led_on_png, BinaryData::led_on_pngSize), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000));

			repaintFlag = false;
		}
		if (Footswitch_FreezePressed == true)
		{
			Footswitch_ReverbFreeze_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::freeze_on_png, BinaryData::freeze_on_pngSize), 1.000f, Colour(0x00000000), Image(), 1.000f, Colour(0x00000000), Image(), 1.000f, Colour(0x00000000));
			repaintFlag = false;

		}
		else {
			Footswitch_ReverbFreeze_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::freeze_off_png, BinaryData::freeze_off_pngSize), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000));
			repaintFlag = false;
		}
		if (Footswitch_LimiterPressed == true)
		{
			Footswitch_Limiter_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::limiter_on_png, BinaryData::limiter_on_pngSize), 1.000f, Colour(0x00000000), Image(), 1.000f, Colour(0x00000000), Image(), 1.000f, Colour(0x00000000));
			repaintFlag = false;

		}
		else {
			Footswitch_Limiter_Button.setImages(false, true, true,
				ImageCache::getFromMemory(BinaryData::limiter_off_png, BinaryData::limiter_off_pngSize), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000),
				Image(), 1.000f, Colour(0x00000000));
			repaintFlag = false;
		}
	}

    double Chorus_Rate = audioProcessor.getParameterValueFloat("Chorus_Rate");
    double Chorus_Depth = audioProcessor.getParameterValueFloat("Chorus_Depth");
    double Chorus_Feedback = audioProcessor.getParameterValueFloat("Chorus_Feedback");
    double Chorus_Mix = audioProcessor.getParameterValueFloat("Chorus_Mix");
    filmstripSlider_ChorusRate.drawFrame(g, 166, 96, 100, 100, Slider_ChorusRate_Slider, Chorus_Rate);
    filmstripSlider_ChorusDepth.drawFrame(g, 283, 107, 100, 100, Slider_ChorusDepth_Slider, Chorus_Depth);
    filmstripSlider_ChorusFeedback.drawFrame(g, 397, 106, 100, 100, Slider_ChorusFeedback_Slider, Chorus_Feedback);
    filmstripSlider_ChorusMix.drawFrame(g, 507, 106, 100, 100, Slider_ChorusMix_Slider, Chorus_Mix);

    double Delay_TimeL = audioProcessor.getParameterValueFloat("Delay_TimeL");
    double Delay_TimeR = audioProcessor.getParameterValueFloat("Delay_TimeR");
    double Delay_Feedback = audioProcessor.getParameterValueFloat("Delay_Feedback");
    double Delay_Mix = audioProcessor.getParameterValueFloat("Delay_Mix");
    filmstripSlider_DelayL.drawFrame(g, 692, 119, 100, 100, Slider_Delay_TimeL_Slider, Delay_TimeL);
    filmstripSlider_DelayR.drawFrame(g, 801, 124, 100, 100, Slider_Delay_TimeR_Slider, Delay_TimeR);
    filmstripSlider_DelayFeedback.drawFrame(g, 913, 105, 100, 100, Slider_DelayFeedback_Slider, Delay_Feedback);
    filmstripSlider_DelayMix.drawFrame(g, 1032, 108, 100, 100, Slider_DelayMix_Slider, Delay_Mix);

    double Reverb_Size = audioProcessor.getParameterValueFloat("Reverb_Size");
    double Reverb_Width = audioProcessor.getParameterValueFloat("Reverb_Width");
    double Reverb_EQMode = audioProcessor.getParameterValueFloat("Reverb_EQMode");
    double Reverb_Mix = audioProcessor.getParameterValueFloat("Reverb_Mix");
    filmstripSlider_ReverbSize.drawFrame(g, 160, 283, 100, 100, Slider_ReverbSize_Slider, Reverb_Size);
    filmstripSwitch_ReverbWidth.drawFrame(g, 263, 298, 100, 100, Switch_ReverbWidth_Slider, Reverb_Width);
    filmstripSwitch_ReverbEQMode.drawFrame(g, 388, 290, 100, 100, Switch_ReverbEQMode_Slider, Reverb_EQMode);
    filmstripSlider_ReverbMix.drawFrame(g, 510, 283, 100, 100, Slider_ReverbMix_Slider, Reverb_Mix);

} 

void BlissBoxAudioProcessorEditor::resized()
{
	Footswitch_Limiter_Button.setBounds(460, 436, 150, 150);
	Footswitch_Bypass_Button.setBounds(1018, 421, 150, 150);
	Footswitch_ReverbFreeze_Button.setBounds(742, 387, 150, 150);
	LED_Button.setBounds(926, 236, 100, 100);

	Slider_ChorusRate_Slider.setBounds(166, 96, 100, 100);
	Slider_ChorusDepth_Slider.setBounds(283, 107, 100, 100);
	Slider_ChorusFeedback_Slider.setBounds(397, 106, 100, 100);
	Slider_ChorusMix_Slider.setBounds(507, 106, 100, 100);

	Slider_Delay_TimeL_Slider.setBounds(692, 119, 100, 100);
	Slider_Delay_TimeR_Slider.setBounds(801, 124, 100, 100);
	Slider_DelayFeedback_Slider.setBounds(913, 105, 100, 100);
	Slider_DelayMix_Slider.setBounds(1032, 108, 100, 100);

	Slider_ReverbSize_Slider.setBounds(160, 283, 100, 100);
	Switch_ReverbWidth_Slider.setBounds(263, 298, 100, 100);
	Switch_ReverbEQMode_Slider.setBounds(390, 288, 100, 100);
	Slider_ReverbMix_Slider.setBounds(510, 283, 100, 100);
}


void BlissBoxAudioProcessorEditor::sliderValueChanged(Slider* sliderThatWasMoved)
{
    repaintFlag = true;
    repaint();
} 


void BlissBoxAudioProcessorEditor::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked->getName() == "Bypass")
    {
        Footswitch_BypassPressed = !Footswitch_BypassPressed;
    } 
    else if ( buttonThatWasClicked->getName() == "Freeze")
    {
        Footswitch_FreezePressed = !Footswitch_FreezePressed;
    }
    else if ( buttonThatWasClicked->getName() == "Limiter")
    {
        Footswitch_LimiterPressed = !Footswitch_LimiterPressed;
    }
    repaintFlag = true;
}

void BlissBoxAudioProcessorEditor::setSliderComponent(Slider& slider, std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, juce::String paramName)
{
    sliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramName, slider);
    slider.addListener(this);
    slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(Slider::TextBoxLeft, false, 0, 0);

    slider.setPopupDisplayEnabled(true, true, this, 2000);

    slider.setValue(audioProcessor.getParameterValueFloat(paramName));
    addAndMakeVisible(&slider);
    slider.setAlpha(0);
}

void BlissBoxAudioProcessorEditor::setSliderComponentNoPopUpDisplay(Slider& slider, std::unique_ptr <AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, juce::String paramName)
{
    sliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramName, slider);
    slider.addListener(this);
    slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(Slider::TextBoxLeft, false, 0, 0);
    slider.setValue(audioProcessor.getParameterValueFloat(paramName));
    addAndMakeVisible(&slider);
    slider.setAlpha(0);
}