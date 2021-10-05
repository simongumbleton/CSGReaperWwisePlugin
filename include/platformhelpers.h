//
//  platformhelpers.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 12/10/2020.
//  Copyright © 2020 My Company. All rights reserved.
//
#pragma once
#include <string>
#include <algorithm>

#ifndef platformhelpers_h
#define platformhelpers_h

const char kPathSeparator =
#ifdef _WIN32
							'\\';
#else
							'/';
#endif

static std::string stringToLower(std::string input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c){ return std::tolower(c); });
	return result;
}

#endif /* platformhelpers_h */
