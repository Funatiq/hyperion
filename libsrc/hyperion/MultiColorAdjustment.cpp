
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
		
		uint8_t OR = 0, OG = 0, OB = 0;
		
		uint8_t WR = adjustment->_rgbRedAdjustment.getadjustmentR();
		uint8_t WG = adjustment->_rgbGreenAdjustment.getadjustmentG();
		uint8_t WB = adjustment->_rgbBlueAdjustment.getadjustmentB();
		
		uint8_t RR = color.red;
		uint8_t	RG = adjustment->_rgbRedAdjustment.getadjustmentG(color.red);
		uint8_t	RB = adjustment->_rgbRedAdjustment.getadjustmentB(color.red);
		
		uint8_t GR = adjustment->_rgbGreenAdjustment.getadjustmentR(255-color.red);
		uint8_t	GG = (255-color.red);
		uint8_t	GB = adjustment->_rgbGreenAdjustment.getadjustmentB(255-color.red);
		
		uint8_t BR =  adjustment->_rgbBlueAdjustment.getadjustmentR(255-color.red);
		uint8_t	BG = adjustment->_rgbBlueAdjustment.getadjustmentG(255-color.red);
		uint8_t	BB = (255-color.red);
		
		uint8_t CR = (uint16_t) adjustment->_rgbGreenAdjustment.getadjustmentR(255-color.red)+adjustment->_rgbBlueAdjustment.getadjustmentR(255-color.red) < 255 ? 
			adjustment->_rgbGreenAdjustment.getadjustmentR(255-color.red)+adjustment->_rgbBlueAdjustment.getadjustmentR(255-color.red) : 255;
		uint8_t	CG = adjustment->_rgbGreenAdjustment.getadjustmentG(255-color.red);
		uint8_t	CB = adjustment->_rgbBlueAdjustment.getadjustmentB(255-color.red);
		
		uint8_t MR = adjustment->_rgbRedAdjustment.getadjustmentR(color.red);
		uint8_t	MG = (uint16_t) adjustment->_rgbRedAdjustment.getadjustmentG(color.red)+adjustment->_rgbBlueAdjustment.getadjustmentG(color.red) < 255 ? 
			adjustment->_rgbRedAdjustment.getadjustmentG(color.red)+adjustment->_rgbBlueAdjustment.getadjustmentG(color.red) : 255;
		uint8_t	MB = adjustment->_rgbBlueAdjustment.getadjustmentB(color.red);
		
		uint8_t YR = adjustment->_rgbRedAdjustment.getadjustmentR(color.red);
		uint8_t	YG = adjustment->_rgbGreenAdjustment.getadjustmentG(color.red);
		uint8_t	YB = (uint16_t) adjustment->_rgbRedAdjustment.getadjustmentB(color.red)+adjustment->_rgbGreenAdjustment.getadjustmentB(color.red) < 255 ? 
			adjustment->_rgbRedAdjustment.getadjustmentB(color.red)+adjustment->_rgbGreenAdjustment.getadjustmentB(color.red) : 255;
		// red
		uint8_t s1 = OR + RR;
		uint8_t e1 = GR + YR;
		
		uint8_t s2 = BR + MR;
		uint8_t e2 = CR + WR;
		
		uint8_t s3 = (uint16_t)s1*(1-color.green)/255 + (uint16_t)e1*color.green/255
		uint8_t e3 = (uint16_t)s2*(1-color.green)/255 + (uint16_t)e2*color.green/255
		
		uint8_t ledR = (uint16_t)s3*(1-color.blue)/255 + (uint16_t)e3*color.blue/255;
		// green
		s1 = OG + RG;
		e1 = GG + YG;
		
		s2 = BG + MG;
		e2 = CG + WG;
		
		s3 = (uint16_t)s1*(1-color.green)/255 + (uint16_t)e1*color.green/255
		e3 = (uint16_t)s2*(1-color.green)/255 + (uint16_t)e2*color.green/255
		
		uint8_t ledG = (uint16_t)s3*(1-color.blue)/255 + (uint16_t)e3*color.blue/255;
		// blue
		s1 = OB + RB;
		e1 = GB + YB;
		
		s2 = BB + MB;
		e2 = CB + WB;
		
		s3 = (uint16_t)s1*(1-color.green)/255 + (uint16_t)e1*color.green/255
		e3 = (uint16_t)s2*(1-color.green)/255 + (uint16_t)e2*color.green/255
		
		uint8_t ledB = (uint16_t)s3*(1-color.blue)/255 + (uint16_t)e3*color.blue/255;
		
		color.red   = ledR;
		color.green = ledG;
		color.blue  = ledB;
	}
	return ledColors;
}
