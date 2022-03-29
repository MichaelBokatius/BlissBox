/*
  ==============================================================================

	Based on FilmStripSlider for juce from Colin Gill
	https://github.com/ColinGill/AudioPlugIn---Vibe-rato/tree/master/C%2B%2B/VibeRato/Source

  ==============================================================================
*/

#include <JuceHeader.h>
#pragma once
using namespace juce;

class BlissBoxSlider
{
public:

	BlissBoxSlider(Image* _knobStrip, unsigned int numOfPositions);
	void drawFrame(Graphics& g, int x, int y, int width, int height, Slider& slider, double position);
	Image* knobStrip;

private:

	int frameCount;
	int frameSize;
	bool isVerticalStrip;
};