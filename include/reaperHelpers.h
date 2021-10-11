#pragma once
#include <string>

class ReaProject;

void GetReaperGlobals();

std::string GetReaperResourcePath();

std::string GetCurrentReaperProject();

ReaProject* GetCurrentReaProject();

void SaveProject();

void Reaper_RenderAllQuedJobs();

void PrintToConsole(std::string text);

void PrintToConsole(int text);

std::vector<std::string> getNonMasterProjectRegions();

int countRegions();

int countMarkers();

void saveProjExState(const char* Key, const char* Value);

void getProjExState(const char* Key, char* &OutValue,int &OutSize);
