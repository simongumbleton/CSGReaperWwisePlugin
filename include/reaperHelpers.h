#pragma once
#include <string>

void GetReaperGlobals();

std::string GetReaperResourcePath();

std::string GetCurrentReaperProject();

void Reaper_RenderAllQuedJobs();

void PrintToConsole(std::string text);

void PrintToConsole(int text);