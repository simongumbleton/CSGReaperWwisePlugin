#pragma once
#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

struct config
{
	int waapiPort = 8080;
	bool useAutomationMode = true;
	std::string userOrigDir = "ImportedFromReaper/";
	std::string versionToken = "_v00";
};

// PUBLIC client calls this function
bool ReadConfigFile(config& outConfig);

// PRIVATE internal helper functions
bool CheckConfigExists();
bool CreateConfigFile();
bool insertDefaultValueInConfig(string Key);
