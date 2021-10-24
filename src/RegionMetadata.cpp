#include "RegionMetadata.h"
#include "reaperHelpers.h"
#include "reaper_plugin_functions.h"
#include "platformhelpers.h"

using namespace std;


ProjectRegionMetadataHelper::ProjectRegionMetadataHelper()
{
	fillProjectRegionInfos();
	createMasterRegionData();
	PrintToConsole("Done");
}

ProjectRegionMetadataHelper::~ProjectRegionMetadataHelper()
{
}


void ProjectRegionMetadataHelper::fillProjectRegionInfos()
{
	ProjectRegions.empty();
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
	}
	for (auto &mregion : MasterRegions)
	{
		getRelativeStartForChildrenInMaster(mregion.second);
	}
}


/*

RegionMetadataHelper = {}

--Add the script directory to the path so we can load the helper module
local Info       = debug.getinfo (1, 'S');
local ScriptPath = Info.source:match[[^@?(.*[\/])[^\/]-$]];
package.path = ScriptPath.. '?.lua;'  .. package.path
JSON = require("JSON")

local MasterRegionMap = {}

function msg(msg)
  reaper.ShowConsoleMsg(tostring(msg))
  reaper.ShowConsoleMsg("\n")
end

local ProjectRegions = {}
local MasterRegions = {}
projName = reaper.GetProjectName("","")
msg(projName)
projPath = reaper.GetProjectPath("")
projPath = string.gsub(projPath, "Media","")
msg(projPath)
local fullProjectFilepath = projPath..projName
msg(fullProjectFilepath)

function RegionMetadataHelper.saveProjectInfoToExState()
	--msg("----SAVE----")
	jsonStringToSave = JSON:encode_pretty(ProjectRegions)
	--msg(jsonStringToSave)
	reaper.SetProjExtState(fullProjectFilepath, "RegionMetadata", "ProjectRegions", jsonStringToSave)
	return jsonStringToSave
end

function RegionMetadataHelper.clearProjectInfoToExState()
	--msg("----CLEAR----")
	reaper.SetProjExtState(fullProjectFilepath, "RegionMetadata", "ProjectRegions", "")
	ProjectRegions = {}
	MasterRegions = {}
	return true
end

function RegionMetadataHelper.getProjectInfoFromExState()
	--msg("----LOAD----")
	retval, val = reaper.GetProjExtState(fullProjectFilepath, "RegionMetadata", "ProjectRegions")
	jsonDataFromExState = JSON:decode(val)
	return jsonDataFromExState
end

function RegionMetadataHelper.gatherProjectRegions()
	--RegionMetadataHelper.clearProjectInfoToExState()
	savedInfo = RegionMetadataHelper.getProjectInfoFromExState()
	if not savedInfo then
	-- No ex state info found in the project, so gather fresh region data
	msg("no ex state creating fresh")
		ProjectRegions = RegionMetadataHelper.getProjectRegionInfo(projName)
		return ProjectRegions
	else
	-- load the data from the ex state, add/remove/update regions
		ProjectRegions = savedInfo
		transientProjRegions = RegionMetadataHelper.getProjectRegionInfo(projName)
		for key,value in pairs(transientProjRegions) do
			if ProjectRegions[key] == nil then --region was not in the saved list, must be new
				ProjectRegions[key] = value
			else
				--Found a matching region so just update the relevant info
				ProjectRegions[key]["start"] = value["start"]
				ProjectRegions[key]["end"] = value["end"]
				ProjectRegions[key]["index"] = value["index"]
				ProjectRegions[key]["name"] = value["name"]
			end
		end
		for key,value in pairs(ProjectRegions) do
			if transientProjRegions[key] == nil then
				--remove region that was in the list but not in the transient list
				ProjectRegions[key] = nil
			end
		end

	end
	return ProjectRegions
end

function RegionMetadataHelper.getRegionsExcludingMasters()
	local nonMasterRegions = {}
	for key,value in pairs(ProjectRegions) do
		if not isMasterRegion_Name(key) then
			nonMasterRegions[key] = value
		end
	end
	--table.sort(nonMasterRegions, function(a, b) return nonMasterRegions[a]["start"] < nonMasterRegions[b]["start"] end)
	return nonMasterRegions
end

function getTableSize(table)
	size = 0
	for key,value in pairs(table) do
		size = size + 1
	end
	return size
end

function RegionMetadataHelper.getRegionNamesSortedByTime()
	local keysSortedByTime = {}
	local nonMasterRegions = RegionMetadataHelper.getRegionsExcludingMasters()
	table.sort(nonMasterRegions, function(a, b) return nonMasterRegions[a]["start"] < nonMasterRegions[b]["start"] end)
	for i=1,1,-1 do
		print(i)
	end
	return nonMasterRegions
end

function RegionMetadataHelper.createMasterRegionData()
	MasterRegions = RegionMetadataHelper.getProjectMasterRegions()
--	return RegionMetadataHelper.writeJSONToFile()
--end
	for region,rinfo in pairs(ProjectRegions) do
		--msg("Get master region for region")
		--msg(region)
		if MasterRegions[region] == nil then
			RegionMetadataHelper.getMasterRegionForRegion(ProjectRegions[region])
		end
	end
	for region,rinfo in pairs(MasterRegions) do
		RegionMetadataHelper.getRelativeStartForChildrenInMaster(region)
	end
	RegionMetadataHelper.saveProjectInfoToExState()
	return RegionMetadataHelper.writeJSONToFile()
end

function RegionMetadataHelper.updateRegionProperties(regionName,propertyName,propertyValue)
	--RegionMetadataHelper.gatherProjectRegions()
	--msg("------GOT TO THE UPDATE----------")
	--msg(ProjectRegions[regionName][propertyName])
	if ProjectRegions[regionName] ~= nil then
		if ProjectRegions[regionName][propertyName] ~= nil then
			ProjectRegions[regionName][propertyName] = propertyValue
			--RegionMetadataHelper.saveProjectInfoToExState()
			return true
		end
	end
	msg("Failed to set region property")
	msg(regionName)
	msg(propertyName)
	return false
end

function countRegions(proj)
	count =0
	retval,  num_markers,  num_regions = reaper.CountProjectMarkers(proj)
	return num_regions + num_markers
end

function RegionMetadataHelper.getProjectRegionInfo(project)
	local regions = {}
	numMarkers = countRegions(project)
	i = 0
	while i < numMarkers do
		local ret, isrgn, pos, rgnend, name, markrgnindexnumber = reaper.EnumProjectMarkers(i)
		if isrgn and name ~= "" then
			rInfo = {}
			rInfo["name"] = name
			rInfo["start"] = pos
			rInfo["relativeStart"] = 0
			rInfo["end"] = rgnend
			rInfo["index"]=markrgnindexnumber
			rInfo["master"]="none"
			rInfo["children"]={}
			rInfo["tag"]=""
			rInfo["attach"]=""
			regions[name] = rInfo
		end
		i = i+1
	end
	return regions
end

function isMasterRegion_Name(regionName)
	i,j = string.find(string.lower(tostring(regionName)),"_master",string.len(regionName)-7,true)
	if i ~= nil and j == string.len(regionName) then
		return true
	else
		return false
	end
end

function isMasterRegion_Info(regionInfo)
	i,j = string.find(string.lower(tostring(regionInfo.name)),"_master",string.len(regionInfo.name)-7,true)
	if i ~= nil and j == string.len(regionInfo.name) then
		return true
	else
		return false
	end
end

function RegionMetadataHelper.getProjectMasterRegions()
	local masters ={}
	msg("-----GET MASTER REGIONS--------")
	for region,rinfo in pairs(ProjectRegions) do
		msg(region)
		--if true then
		if isMasterRegion_Name(region) then
			rinfo["children"] = {}
			masters[region] = rinfo
			--ProjectRegions[region]=nil
		end
	end
	msg("-----DONE  GET MASTER REGIONS--------")
	return masters
end

function RegionMetadataHelper.getMasterRegionForRegion(region)
	for master,minfo in pairs(MasterRegions) do
		if minfo.name ~= region.name then
			--if minfo.start <= region.start and minfo["end"] >= region["end"] then
			if region.start >= minfo.start and region.start < minfo["end"] then
				msg("found a master")
				msg(region.name)
				msg("owned by..")
				msg(master)
				ProjectRegions[region.name]["master"] = master
				MasterRegions[master]["children"][region.name] = region
				--table.insert(MasterRegions[master]["children"],region)
			end
		end
	end
	return true
end

function RegionMetadataHelper.getRelativeStartForChildrenInMaster(masterRegion)
	mInfo = MasterRegions[masterRegion]
	children = mInfo["children"]
	for k, v in pairs(children) do
		msg(v["name"])
		relativeStart = v["start"] - mInfo["start"]
		if ProjectRegions[v["name"]] ~= nil then
			ProjectRegions[v["name"]]["relativeStart"] = relativeStart
		end
	end
	return true
end

function RegionMetadataHelper.writeJSONToFile()
	ProjectRegionInfo = {}
	ProjectRegionInfo["master_regions"] = MasterRegions
	--ProjectRegionInfo["project_regions"] = ProjectRegions

	jsonstring = JSON:encode(ProjectRegionInfo)
	prettyData = JSON:encode_pretty(ProjectRegionInfo)
	--msg(prettyData)
	msg(projPath)
	projNameJson = string.gsub(projName:lower(), ".rpp",".json")
	filename = projPath..projNameJson
	file = io.open(filename,"w")
	file:write(prettyData)
	file:close()
	msg("Done!")
	msg("Saved region info to: "..filename)
	return filename
end

function RegionMetadataHelper.readJSONfromFile()
	projNameJson = string.gsub(projName:lower(), ".rpp",".json")
	filename = projPath..projNameJson
	file = io.open(filename,"r")
	if not file then return false end
	data = file.read("*all")
	file:close()
	return data
end


numRegions = countRegions(projName)
msg("Writing info for "..numRegions.." regions...")
--msg(numRegions)


--RegionMetadataHelper.clearProjectInfoToExState()
RegionMetadataHelper.gatherProjectRegions()
jsonFile = RegionMetadataHelper.createMasterRegionData()



return RegionMetadataHelper


*/
