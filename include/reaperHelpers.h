#pragma once
#include <string>
#include <vector>

class ReaProject;

struct ReaperRegion
{
	bool isRegion;
	double pos;
	double regEnd;
	//std::string name;
	const char* name;
	int index;
};

void GetReaperGlobals();

std::string GetReaperResourcePath();

std::string GetCurrentReaperProject();

std::string GetCurrentReaperProjectName();

ReaProject* GetCurrentReaProject();

std::string GetCurrentReaperProjectPath();

ReaProject* GetReaProjectFromProjectName(std::string inName);

void SaveProject();

void Reaper_RenderAllQuedJobs();

void PrintToConsole(std::string text);

void PrintToConsole(int text);

std::vector<std::string> getNonMasterProjectRegionNames();

int countRegions();

int countMarkers();

void saveProjExState(std::string Key, std::string Value, std::string extName = "CSGREGIONPROPERTIES",ReaProject* inProj = nullptr);

std::string getProjExState(std::string Key, std::string extName = "CSGREGIONPROPERTIES",ReaProject* inProj = nullptr);

void deleteProjExtState(std::string Key = "", std::string extName = "CSGREGIONPROPERTIES",ReaProject* inProj = nullptr);

void saveGlobalExtState(std::string Key, std::string Value,bool persist = true, std::string extName = "CSGPlugin");

std::string getGlobalExtState(std::string Key, std::string extName = "CSGPlugin");

void deleteGlobalExtState(std::string Key,bool persist = true, std::string extName = "CSGPlugin");

std::vector<ReaperRegion> findRegionsAtTime(float inSeconds, ReaProject* inProj = nullptr);

bool doesRegionContainTime(std::string regionName, float inSeconds, ReaProject* inProj = nullptr);

std::vector<ReaperRegion> getAllProjectRegions(ReaProject* inProj = nullptr);

void bringWindowsToFront();

