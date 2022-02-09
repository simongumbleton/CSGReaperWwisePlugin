//
//  versioning.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 15/11/2021.
//  Copyright © 2021 My Company. All rights reserved.
//
#pragma once
#include <string>
#include <iomanip>

#ifndef versioning_h
#define versioning_h

static std::string GetPluginVersionAsString(double Vnumber)
{
	// Create an output string stream
	std::ostringstream streamObj3;
	// Set Fixed -Point Notation
	streamObj3 << std::fixed;
	// Set precision to 2 digits
	streamObj3 << std::setprecision(2);
	//Add double to stream
	streamObj3 << "v" << Vnumber;
	// Get string from output string stream
	return streamObj3.str();
}


namespace CreateTransfer{
	static const double pluginVersionNumber = 1.05;

	static std::string GetPluginVersionString()
	{
		return GetPluginVersionAsString(pluginVersionNumber);
	}
}

namespace RegionMetadata{
	static const double pluginVersionNumber = 1.05;

	static std::string GetPluginVersionString()
	{
		return GetPluginVersionAsString(pluginVersionNumber);
	}
}

namespace Reconformer{
	static const double pluginVersionNumber = 1.05;

	static std::string GetPluginVersionString()
	{
		return GetPluginVersionAsString(pluginVersionNumber);
	}
}
#endif /* versioning_h */
