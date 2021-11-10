#pragma once
#include <string>
#include <vector>

class ReaProject;

void GetReaperGlobals();

std::string GetReaperResourcePath();

std::string GetCurrentReaperProject();

std::string GetCurrentReaperProjectName();

ReaProject* GetCurrentReaProject();

std::string GetCurrentReaperProjectPath();

void SaveProject();

void Reaper_RenderAllQuedJobs();

void PrintToConsole(std::string text);

void PrintToConsole(int text);

std::vector<std::string> getNonMasterProjectRegionNames();

int countRegions();

int countMarkers();

void saveProjExState(std::string Key, std::string Value, std::string extName = "CSGREGIONPROPERTIES");

std::string getProjExState(std::string Key, std::string extName = "CSGREGIONPROPERTIES");

void bringWindowsToFront();

