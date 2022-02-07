//
//  settings_structs.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 03/02/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#pragma once

struct TransferSettingsStruct
{
	
};

struct MetadataSettingsStruct
{
	std::vector<std::string>PropertyNames
	{
		"Attach",
		"Tag"
	};

};

struct EDLSettingsStruct
{
	bool CreateEDLFileRegion = false;
	bool CopyExistingRegions = true;
	bool CreateRegionsForChangedShots = true;
	std::string timeLineOffset = "01:00:00:00";
	float framerate = 30;
};
