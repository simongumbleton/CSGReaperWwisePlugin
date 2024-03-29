#include "RegionMetadataHelper.h"

#include "platformhelpers.h"
#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"
#include "reaperHelpers.h"

#include "WwiseConnectionHandler.h"
#include "Reaper_ExtState_Helper.h"

#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <filesystem>




//using namespace std;


ProjectRegionMetadataHelper::ProjectRegionMetadataHelper()
{
	//fillProjectRegionInfos();
	//createMasterRegionData();
	//PrintToConsole("Done");
}

ProjectRegionMetadataHelper::~ProjectRegionMetadataHelper()
{
}


void ProjectRegionMetadataHelper::fillProjectRegionInfos()
{
	MasterRegions.clear();
	ProjectRegions.clear();
	
	int total = countRegions() + countMarkers();
	std::vector<RegionInfo> results;
	int i = 0;
	while (i < total)
	{
		bool isRegion;
		double pos;
		double regEnd;
		//std::string name;
		const char* name;
		int index;
		EnumProjectMarkers(i, &isRegion,&pos,&regEnd,&name,&index);
		if (isRegion)
		{
			std::string namestr = name;
			if (!namestr.empty())
			{
				RegionInfo rinfo;
				rinfo.name = name;
				rinfo.start = pos;
				rinfo.end = regEnd;
				rinfo.index = index;
				ProjectRegions.emplace(name,rinfo);
			}
		}
		i++;
	}
	//return results;
}

bool ProjectRegionMetadataHelper::isMasterRegion_Name(std::string regionName)
{
	std::size_t found = PLATFORMHELPERS::stringToLower(regionName).find("_master",regionName.length()-7);
	if (found != regionName.npos)
	{
		return true;
	}
	return false;
}

bool ProjectRegionMetadataHelper::isMasterRegion_Info(RegionInfo &regionInfo)
{
	return isMasterRegion_Name(regionInfo.name);
}


void ProjectRegionMetadataHelper::getProjectMasterRegions()
{
	MasterRegions.clear();
	for (auto &region : ProjectRegions)
	{
		if (isMasterRegion_Name(region.first))
		{
			ProjectRegions[region.first].children.clear();
			MasterRegions.emplace(region);
		}
	}
}

void ProjectRegionMetadataHelper::getMasterRegionForRegion(RegionInfo &rinfo)
{
	for (auto &mRegion : MasterRegions)
	{
		if (rinfo.name != mRegion.first)
		{
			if ((rinfo.start >= mRegion.second.start)and(rinfo.start < mRegion.second.end))
			{
				//found a master
				ProjectRegions[rinfo.name].master = mRegion.first;
				MasterRegions[mRegion.first].children.emplace(rinfo.name,&rinfo);
			}
		}
	}
}

void ProjectRegionMetadataHelper::getEventsForRegion(RegionInfo& rinfo)
{
	rinfo.events.clear();
	std::unordered_map<std::string, float> RenderedWavs = EXTSTATE::GetRenderOutputFilesFromProjExState(nullptr);//current project
	for (auto event : GetEventListFromProjExtState())
	{
		size_t found = PLATFORMHELPERS::stringToLower(event).find(PLATFORMHELPERS::stringToLower(rinfo.name));
		if (found != std::string::npos)
		{
			//found a region that could match an event. Check if we have a matching rendered audio file with a timecode reference
			for (auto& wav : RenderedWavs)
			{
				auto fileName = std::filesystem::path(wav.first).replace_extension("").string();
				size_t found = PLATFORMHELPERS::stringToLower(event).find(PLATFORMHELPERS::stringToLower(fileName));
				if (found != event.npos)
				{
					//Event name contains the filename, so it must be Play_<filename>. See if the saved time reference matches this region
					auto timeRef = wav.second;
					if (doesRegionContainTime(rinfo.name,timeRef))
					{
						//The wav time ref falls within this regions bounds
						rinfo.events.insert(event);
					}
				}
			}
		}
	}
}


void ProjectRegionMetadataHelper::getRelativeStartForChildrenInMaster(RegionInfo &masterRegion)
{
	for (auto &child: masterRegion.children)
	{
		float relativeStart = child.second->start - masterRegion.start;
		child.second->relativeStart = relativeStart;		
	}
}

void ProjectRegionMetadataHelper::createMasterRegionData()
{
	getProjectMasterRegions();
	
	for (auto &region : ProjectRegions)
	{
		getMasterRegionForRegion(region.second);
		getEventsForRegion(region.second);
	}
	for (auto &mregion : MasterRegions)
	{
		getRelativeStartForChildrenInMaster(mregion.second);
	}
}

void ProjectRegionMetadataHelper::updateRegionGUIProperties(std::string RegionName, std::map<std::string, std::string> values)
{
	if (ProjectRegions.find(RegionName) != ProjectRegions.end())
	{
		//add more string properties to the region metadata here
		for (auto& keyval : values)
		{
			ProjectRegions.at(RegionName).userProperties[keyval.first] = keyval.second;
		}
	}
	
}

void ProjectRegionMetadataHelper::writeMasterRegionInfoToJson()
{
	//std::map<std::string, std::map<std::string, RegionInfo>> ProjectRegionInfo;
	//ProjectRegionInfo["master_regions"] = MasterRegions;

	json ProjectRegionInfo;
	ProjectRegionInfo["reaper_project"] = PLATFORMHELPERS::stringToLower(GetCurrentReaperProjectName());
	for (auto& master : MasterRegions)
	{
		ProjectRegionInfo["master_regions"][master.first] = RegionInfoToJson(master.second);
	}
	
	//std::string output = ProjectRegionInfo.dump(4);
	//PrintToConsole(output);

	//std::string projNameJson = stringReplace(stringToLower(GetCurrentReaperProject()),".rpp" , ".json");
	std::string projPathjson = PLATFORMHELPERS::stringReplace(PLATFORMHELPERS::stringToLower(GetCurrentReaperProjectPath()), ".rpp", ".json"); ;
	//	filename = projPath..projNameJson

	bool result = CreateProjectRegionJsonFile(projPathjson, ProjectRegionInfo);

	if (result)
	{
		PrintToConsole("Saved region info to: " + projPathjson);
	}
	else
	{
		PrintToConsole("Failed to save region info to: " + projPathjson);
	}
	return;
}

json ProjectRegionMetadataHelper::RegionInfoToJson(RegionInfo& rinfo)
{
	json j;
	j["name"] = rinfo.name;
	j["start"] = rinfo.start;
	j["relativeStart"] = rinfo.relativeStart;
	j["end"] = rinfo.end;
	j["index"] = rinfo.index;
	j["master"] = rinfo.master;
	for (auto& keyVal : rinfo.userProperties)
	{
		j[keyVal.first] = keyVal.second;
	}
	j["events"] = {};
	for (auto& event : rinfo.events)
	{
		j["events"].push_back(event);
	}
	
	if (rinfo.children.empty())
	{
		j["children"] = {};
	}
	else
	{
		for (auto& child : rinfo.children)
		{
			j["children"][child.first] = RegionInfoToJson(*child.second);
		}
	}
	

	
	return j;
}

bool ProjectRegionMetadataHelper::CreateProjectRegionJsonFile(std::string filename, json data)
{

	std::ofstream newFile;
	newFile.open(filename, std::ofstream::out | std::ofstream::trunc);

	if (!newFile.good()) 
	{ 
		return false; 
	}
	newFile << data.dump(4);
	newFile.close();
	if (newFile.good())
	{
		return true;
	}
	else 
	{
		return false;
	}
}

std::vector<std::string> ProjectRegionMetadataHelper::GetEventListFromProjExtState(ReaProject* inProject)
{
	std::string svalue = "";
	std::vector<std::string> tempListValues;
	svalue = getProjExState("EVENTS", EXTSTATE::transferEventsName,inProject);//"CSGTRANSFERWWISEEVENTS"
	if (!svalue.empty())
	{
		char* pch;
		printf("Splitting string \"%s\" into tokens:\n", svalue.c_str());
		//char delims[] = "\n !@#$%^&*)(_+-=][}{|:;'<>?,./\"\\";
		char delims[] = "!@#$%^&*)(+-=][}{|:;<>?,./\\";
		pch = strtok(&svalue[0], delims);
		
		while (pch != NULL)
		{
			printf("%s\n", pch);
			std::string value = std::string(pch);
			value.erase(std::remove(value.begin(), value.end(), '\''), value.end());


			tempListValues.push_back(value);
			pch = strtok(NULL, delims);
		}
	}
	return tempListValues;
}
