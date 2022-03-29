/*
  ==============================================================================

	Based on FilmStripSlider for juce from Colin Gill
	https://github.com/ColinGill/AudioPlugIn---Vibe-rato/tree/master/C%2B%2B/VibeRato/Source

  ==============================================================================
*/

#include "BlissBoxSlider.h"

BlissBoxSlider::BlissBoxSlider(Image* _knobStrip, unsigned int numOfPositions) : knobStrip(_knobStrip), frameCount(numOfPositions)
{
	frameSize = knobStrip->getHeight() / frameCount;
	isVerticalStrip = true;
}


void BlissBoxSlider::drawFrame(Graphics& g, int x, int y, int width, int height, Slider& slider, double position)
{
	const double div = (slider.getMaximum() + slider.getInterval()) / frameCount;
	double pos = (int)(slider.getValue() / div);

	g.drawImage(*knobStrip, x, y, width, height, 0, (int)(pos * frameSize), width, height, false);

}