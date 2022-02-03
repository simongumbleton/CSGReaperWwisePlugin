//
//  image_helper.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 03/02/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#ifndef image_helper_h
#define image_helper_h
#pragma once

#include "JUCE/JuceHeader.h"
#include "cog.h"

static Image GetSettingsImage()
{
	return ImageFileFormat::loadFrom(cog::cog_png, cog::cog_pngSize);
}

#endif /* image_helper_h */
