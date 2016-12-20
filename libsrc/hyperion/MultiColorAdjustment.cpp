
// STL includes
#include <cassert>

// Hyperion includes
#include "MultiColorAdjustment.h"

MultiColorAdjustment::MultiColorAdjustment(const unsigned ledCnt) :
	_ledAdjustments(ledCnt, nullptr)
{
}

MultiColorAdjustment::~MultiColorAdjustment()
{
	// Clean up all the transforms
	for (ColorAdjustment * adjustment : _adjustment)
	{
		delete adjustment;
	}
}

void MultiColorAdjustment::addAdjustment(ColorAdjustment * adjustment)
{
	_adjustmentIds.push_back(adjustment->_id);
	_adjustment.push_back(adjustment);
}

void MultiColorAdjustment::setAdjustmentForLed(const std::string& id, const unsigned startLed, const unsigned endLed)
{
	assert(startLed <= endLed);
	assert(endLed < _ledAdjustments.size());

	// Get the identified adjustment (don't care if is nullptr)
	ColorAdjustment * adjustment = getAdjustment(id);
	for (unsigned iLed=startLed; iLed<=endLed; ++iLed)
	{
		_ledAdjustments[iLed] = adjustment;
	}
}

bool MultiColorAdjustment::verifyAdjustments() const
{
	bool allLedsSet = true;
	for (unsigned iLed=0; iLed<_ledAdjustments.size(); ++iLed)
	{
		if (_ledAdjustments[iLed] == nullptr)
		{
			std::cerr << "HYPERION (C.adjustment) ERROR: No adjustment set for " << iLed << std::endl;
			allLedsSet = false;
		}
	}
	return allLedsSet;
}

const std::vector<std::string> & MultiColorAdjustment::getAdjustmentIds()
{
	return _adjustmentIds;
}

ColorAdjustment* MultiColorAdjustment::getAdjustment(const std::string& id)
{
	// Iterate through the unique adjustments until we find the one with the given id
	for (ColorAdjustment* adjustment : _adjustment)
	{
		if (adjustment->_id == id)
		{
			return adjustment;
		}
	}

	// The ColorAdjustment was not found
	return nullptr;
}

std::vector<ColorRgb> MultiColorAdjustment::applyAdjustment(const std::vector<ColorRgb>& rawColors)
{
	// Create a copy, as we will do the rest of the adjustment in place
	std::vector<ColorRgb> ledColors(rawColors);

	const size_t itCnt = std::min(_ledAdjustments.size(), rawColors.size());
	for (size_t i=0; i<itCnt; ++i)
	{
		ColorAdjustment* adjustment = _ledAdjustments[i];
		if (adjustment == nullptr)
		{
			// No transform set for this led (do nothing)
			continue;
		}
		ColorRgb& color = ledColors[i];
		
		int r2 = color.red*color.red/255;
		int g2 = color.green*color.green/255;
		int b2 = color.blue*color.blue/255;
		int rg = color.red*color.green/255;
		int rb = color.red*color.blue/255;
		int gb = color.green*color.blue/255;
		
		int ledR = r2;
		ledR += adjustment->_rgbGreenAdjustment.adjustmentR(g2);
		ledR += adjustment->_rgbBlueAdjustment.adjustmentR(b2);
		ledR += adjustment->_rgbRedAdjustment.adjustmentR(rg) - rg - adjustment->_rgbGreenAdjustment.adjustmentR(rg);
		ledR += adjustment->_rgbRedAdjustment.adjustmentR(rb) - rb - adjustment->_rgbBlueAdjustment.adjustmentR(rb);
		ledR += gb - adjustment->_rgbRedAdjustment.adjustmentR(gb);

		int ledG += adjustment->_rgbRedAdjustment.adjustmentG(r2);
		ledG = g2;
		ledG += adjustment->_rgbBlueAdjustment.adjustmentG(b2);
		ledG += adjustment->_rgbGreenAdjustment.adjustmentG(rg) - rg - adjustment->_rgbRedAdjustment.adjustmentG(rg);
		ledG += rb - adjustment->_rgbGreenAdjustment.adjustmentG(rb);
		ledG += adjustment->_rgbGreenAdjustment.adjustmentG(gb) - gb - adjustment->_rgbBlueAdjustment.adjustmentG(gb);
		
		int ledB += adjustment->_rgbRedAdjustment.adjustmentB(r2);
		ledB += adjustment->_rgbGreenAdjustment.adjustmentB(g2);
		ledB = b2;
		ledB += rg - adjustment->_rgbBlueAdjustment.adjustmentB(rg);
		ledB += adjustment->_rgbBlueAdjustment.adjustmentB(rb) - rb - adjustment->_rgbRedAdjustment.adjustmentB(rb);
		ledB += adjustment->_rgbBlueAdjustment.adjustmentB(gb) - gb - adjustment->_rgbGreenAdjustment.adjustmentB(gb);
		
		int maxR = 255;
		int maxG = 255;
		int maxB = 255;
		
		if (ledR > maxR)
		  color.red = (uint8_t)maxR;
		else
		  color.red = (uint8_t)ledR;
		
		if (ledG > maxG)
		  color.green = (uint8_t)maxG;
		else
		  color.green = (uint8_t)ledG;
		
		if (ledB > maxB)
		  color.blue = (uint8_t)maxB;
		else
		  color.blue = (uint8_t)ledB;
	}
	return ledColors;
}
