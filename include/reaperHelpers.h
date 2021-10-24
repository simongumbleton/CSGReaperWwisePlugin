#pragma once
#include <string>
#include <vector>

class ReaProject;

void GetReaperGlobals();

std::string GetReaperResourcePath();

std::string GetCurrentReaperProject();

ReaProject* GetCurrentReaProject();

void SaveProject();

void Reaper_RenderAllQuedJobs();

void PrintToConsole(std::string text);

void PrintToConsole(int text);

std::vector<std::string> getNonMasterProjectRegionNames();

int countRegions();

int countMarkers();

void saveProjExState(std::string Key, std::string Value);

std::string getProjExState(std::string Key);
