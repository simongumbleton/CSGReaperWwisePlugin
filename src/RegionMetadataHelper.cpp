#include "RegionMetadataHelper.h"
#include "reaperHelpers.h"
#include "reaper_plugin_functions.h"
#include "platformhelpers.h"
#include "WwiseConnectionHandler.h"

#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <filesystem>




using namespace std;


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
	std::size_t found = stringToLower(regionName).find("_master",regionName.length()-7);
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
	MasterRegions.empty();
	for (auto &region : ProjectRegions)
	{
		if (isMasterRegion_Name(region.first))
		{
			ProjectRegions[region.first].children.empty();
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
	for (auto event : GetEventListFromProjExtState())
	{
		size_t found = stringToLower(event).find(stringToLower(rinfo.name));
		if (found != std::string::npos)
		{
			rinfo.events.insert(event);
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
		if (values.find("tag") != values.end())
		{
			ProjectRegions.at(RegionName).tag = values.at("tag");
		}
		if (values.find("attach") != values.end())
		{
			ProjectRegions.at(RegionName).attach = values.at("attach");
		}
		//add more string properties to the region metadata here
	}
	
}

void ProjectRegionMetadataHelper::writeMasterRegionInfoToJson()
{
	//std::map<std::string, std::map<std::string, RegionInfo>> ProjectRegionInfo;
	//ProjectRegionInfo["master_regions"] = MasterRegions;

	json ProjectRegionInfo;
	ProjectRegionInfo["reaper_project"] = stringToLower(GetCurrentReaperProjectName());
	for (auto& master : MasterRegions)
	{
		ProjectRegionInfo["master_regions"][master.first] = RegionInfoToJson(master.second);
	}
	
	//std::string output = ProjectRegionInfo.dump(4);
	//PrintToConsole(output);

	//std::string projNameJson = stringReplace(stringToLower(GetCurrentReaperProject()),".rpp" , ".json");
	std::string projPathjson = stringReplace(stringToLower(GetCurrentReaperProjectPath()), ".rpp", ".json"); ;
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
	j["tag"] = rinfo.tag;
	j["attach"] = rinfo.attach;
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

	ofstream newFile;
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

std::vector<std::string> ProjectRegionMetadataHelper::GetEventListFromProjExtState()
{
	std::string svalue = "";
	std::vector<std::string> tempListValues;
	svalue = getProjExState("EVENTS", "CSGTRANSFERWWISEEVENTS");
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
