//
//  waapi_structs.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 18/07/2020.
//  Copyright © 2020 My Company. All rights reserved.
//

#pragma once
#include <string>
#include <map>
#include <vector>


typedef struct WwiseObject WwiseObject;

struct WwiseObject
{
	std::map<std::string, std::string> properties;
	std::map<std::string, double> numericProperties;
	std::vector<WwiseObject*> childObjects;
	WwiseObject* parentObject;
};

struct RenderJobFileOverride
{
	std::string RenderJobFile;
	WwiseObject parentWwiseObject;
	bool isVoice = false;
	std::string ImportLanguage;
	bool OrigDirMatchesWwise = false;
	std::string userOrigsSubDir;
	bool hasImported = false;
	std::string createEventOption;
};

struct RenderQueJob
{
	std::string RenderQueFilePath;
	WwiseObject parentWwiseObject;
	std::vector<std::string> RenderQueJobFileList;
	std::string ParentReaperProject;
	bool hasRendered = false;
	bool hasImported = false;
	bool isVoice = false;
	std::string ImportLanguage;
	bool OrigDirMatchesWwise = false;
	std::string userOrigsSubDir;
	bool hasPerFileOverrides = false;
	std::map<std::string, RenderJobFileOverride> perFileOverridesmap;
	std::string createEventOption;
	

};
