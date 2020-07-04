#pragma once
#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

struct config
{
	int waapiPort = 8095;
	bool useAutomationMode = true;
};

// PUBLIC client calls this function
bool ReadConfigFile(config& outConfig);

// PRIVATE internal helper functions
bool CheckConfigExists();
bool CreateConfigFile();