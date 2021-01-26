
#include <iomanip>
#include <cstdlib>

#include <sstream>
#include <map>

#include <algorithm>
#include "reaperHelpers.h"

#include <filesystem>
#include "ConfigFileHandler.h"
#include "platformhelpers.h"

using namespace std;

string defaults = 
"waapiPort=8095\n"
"useAutomationMode=1\n"
;

//const char dummyconfig[] = "";

string configFileDir = "";
map<string, string> rawConfigData;


bool ReadConfigFile(config & outConfig)
{
	string reaperPath = GetReaperResourcePath();
	configFileDir = reaperPath + kPathSeparator + "UserPlugins";

	ifstream configFile;
	CheckConfigExists();

	configFile.open("csg_reaperwwise.config");

	//std::istringstream is_file(dummyconfig);

	std::string line;
	while (std::getline(configFile, line))
	{
		std::istringstream is_line(line);
		std::string key;
		if (std::getline(is_line, key, '='))
		{
			std::string value;
			if (std::getline(is_line, value))
				rawConfigData[key] = value;
				
		}
	}

	// Set the config properties 
	outConfig.waapiPort = std::stoi(rawConfigData["waapiPort"]);	//stoi converts string to integer
	outConfig.useAutomationMode = std::stoi(rawConfigData["useAutomationMode"]);

	return true;
}

bool CheckConfigExists()
{
	ifstream outFile("csg_reaperwwise.config");
	if (!outFile.is_open())
	{
		CreateConfigFile();
	}
	return true;
}

bool CreateConfigFile()
{
	ofstream newFile;
	newFile.open("csg_reaperwwise.config");
	newFile << defaults; //dummyconfig;
	newFile.close();
	return true;
}
