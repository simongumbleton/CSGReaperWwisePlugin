//
//  settings_structs.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 03/02/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#pragma once

enum ETransferEventCreationOption
{
	E_MirrorActorMixerWorkUnits = 0,
	E_UserEventWorkUnitPath = 1
};

//constexpr std::initializer_list<ETransferEventCreationOption> all_EventCreationOptions = { E_MirrorActorMixerWorkUnits, E_UserEventWorkUnitPath };
static const char* all_EventCreationOptions[] =
{ "Mirror Actor-Mixer Work Unit Structure", "Use Default/Custom Path" };

struct TransferSettingsStruct
{
	int waapiport = 8080;
	bool useAtomationMode = true;
	std::string userorigsubdir = "ImportedFromReaper/";
	std::string versionToken = "_v00";
	std::string templatePath = "\\Actor-Mixer Hierarchy\\Templates\\Templates";
	std::string eventWorkUnitSuffix = "";
	std::string UserEventPath = "\\Events\\Default Work Unit";
	ETransferEventCreationOption eEventCreationOption = ETransferEventCreationOption::E_MirrorActorMixerWorkUnits;
	int eventMirroringDepth = 4;
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
