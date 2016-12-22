
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
		
		// uint8_t black   = (uint32_t)(255-color.red)*(255-color.green)*(255-color.blue)/65025;
		// uint8_t red     = (uint32_t)(color.red)    *(255-color.green)*(255-color.blue)/65025;
		// uint8_t green   = (uint32_t)(255-color.red)*(color.green)    *(255-color.blue)/65025;
		// uint8_t blue    = (uint32_t)(255-color.red)*(255-color.green)*(color.blue)    /65025;
		// uint8_t cyan    = (uint32_t)(255-color.red)*(color.green)    *(color.blue)    /65025;
		// uint8_t magenta = (uint32_t)(color.red)    *(255-color.green)*(color.blue)    /65025;
		// uint8_t yellow  = (uint32_t)(color.red)    *(color.green)    *(255-color.blue)/65025;
		// uint8_t white   = (uint32_t)(color.red)    *(color.green)    *(color.blue)    /65025;

		uint32_t nrng = (uint32_t) (255-color.red)*(255-color.green);
		uint32_t rng  = (uint32_t) (color.red)    *(255-color.green);
		uint32_t nrg  = (uint32_t) (255-color.red)*(color.green);
		uint32_t rg   = (uint32_t) (color.red)    *(color.green);
		
		uint8_t black   = nrng*(255-color.blue)/65025;
		uint8_t red     = rng *(255-color.blue)/65025;
		uint8_t green   = nrg *(255-color.blue)/65025;
		uint8_t blue    = nrng*(color.blue)    /65025;
		uint8_t cyan    = nrg *(color.blue)    /65025;
		uint8_t magenta = rng *(color.blue)    /65025;
		uint8_t yellow  = rg  *(255-color.blue)/65025;
		uint8_t white   = rg  *(color.blue)    /65025;
		
		uint8_t OR = 0;
		uint8_t OG = 0;
		uint8_t OB = 0;
		// uint8_t OR = adjustment->_rgbCyanAdjustment.getadjustmentR(black);
		// uint8_t OG = adjustment->_rgbCyanAdjustment.getadjustmentG(black);
		// uint8_t OB = adjustment->_rgbCyanAdjustment.getadjustmentB(black);
		
		uint8_t RR = red;
		// uint8_t RR = adjustment->_rgbRedAdjustment.getadjustmentR(red);
		uint8_t	RG = adjustment->_rgbRedAdjustment.getadjustmentG(red);
		uint8_t	RB = adjustment->_rgbRedAdjustment.getadjustmentB(red);
		
		uint8_t GR = adjustment->_rgbGreenAdjustment.getadjustmentR(green);
		uint8_t	GG = green;
		// uint8_t	GG = adjustment->_rgbGreenAdjustment.getadjustmentG(green);
		uint8_t	GB = adjustment->_rgbGreenAdjustment.getadjustmentB(green);
		
		uint8_t BR = adjustment->_rgbBlueAdjustment.getadjustmentR(blue);
		uint8_t	BG = adjustment->_rgbBlueAdjustment.getadjustmentG(blue);
		uint8_t	BB = blue;
		// uint8_t	BB = adjustment->_rgbBlueAdjustment.getadjustmentB(blue);
		
		uint8_t CR = (uint16_t) adjustment->_rgbGreenAdjustment.getadjustmentR(cyan)+adjustment->_rgbBlueAdjustment.getadjustmentR(cyan) < 255 ? 
			adjustment->_rgbGreenAdjustment.getadjustmentR(cyan)+adjustment->_rgbBlueAdjustment.getadjustmentR(cyan) : 255;
		uint8_t	CG = adjustment->_rgbGreenAdjustment.getadjustmentG(cyan);
		uint8_t	CB = adjustment->_rgbBlueAdjustment.getadjustmentB(cyan);
		// uint8_t CR = adjustment->_rgbCyanAdjustment.getadjustmentR(cyan);
		// uint8_t CG = adjustment->_rgbCyanAdjustment.getadjustmentG(cyan);
		// uint8_t CB = adjustment->_rgbCyanAdjustment.getadjustmentB(cyan);
		
		uint8_t MR = adjustment->_rgbRedAdjustment.getadjustmentR(magenta);
		uint8_t	MG = (uint16_t) adjustment->_rgbRedAdjustment.getadjustmentG(magenta)+adjustment->_rgbBlueAdjustment.getadjustmentG(magenta) < 255 ? 
			adjustment->_rgbRedAdjustment.getadjustmentG(magenta)+adjustment->_rgbBlueAdjustment.getadjustmentG(magenta) : 255;
		uint8_t	MB = adjustment->_rgbBlueAdjustment.getadjustmentB(magenta);
		// uint8_t MR = adjustment->_rgbMagentaAdjustment.getadjustmentR(magenta);
		// uint8_t MG = adjustment->_rgbMagentaAdjustment.getadjustmentG(magenta);
		// uint8_t MB = adjustment->_rgbMagentaAdjustment.getadjustmentB(magenta);
		
		uint8_t YR = adjustment->_rgbRedAdjustment.getadjustmentR(yellow);
		uint8_t	YG = adjustment->_rgbGreenAdjustment.getadjustmentG(yellow);
		uint8_t	YB = (uint16_t) adjustment->_rgbRedAdjustment.getadjustmentB(yellow)+adjustment->_rgbGreenAdjustment.getadjustmentB(yellow) < 255 ? 
			adjustment->_rgbRedAdjustment.getadjustmentB(yellow)+adjustment->_rgbGreenAdjustment.getadjustmentB(yellow) : 255;
		// uint8_t YR = adjustment->_rgbYellowAdjustment.getadjustmentR(yellow);
		// uint8_t YG = adjustment->_rgbYellowAdjustment.getadjustmentG(yellow);
		// uint8_t YB = adjustment->_rgbYellowAdjustment.getadjustmentB(yellow);
		
		uint8_t WR = adjustment->_rgbRedAdjustment.getadjustmentR(white);
		uint8_t WG = adjustment->_rgbGreenAdjustment.getadjustmentG(white);
		uint8_t WB = adjustment->_rgbBlueAdjustment.getadjustmentB(white);
		// uint8_t WR = adjustment->_rgbWhiteAdjustment.getadjustmentR(white);
		// uint8_t WG = adjustment->_rgbWhiteAdjustment.getadjustmentG(white);
		// uint8_t WB = adjustment->_rgbWhiteAdjustment.getadjustmentB(white);
		
		color.red   = OR + RR + GR + BR + CR + MR + YR + WR;
		color.green = OG + RG + GG + BG + CG + MG + YG + WG;
		color.blue  = OB + RB + GB + BB + CB + MB + YB + WB;
	}
	return ledColors;
}
