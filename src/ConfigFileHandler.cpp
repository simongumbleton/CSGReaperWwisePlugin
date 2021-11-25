
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

std::map<std::string, std::string> configFileDefaults = {
	{"waapiPort","8080"},
	{"useAutomationMode","1"},
	{"userorigsubdir","ImportedFromReaper/"},
	{"versionToken","_v00"},
	{"templatePath","\\Actor-Mixer Hierarchy\\Templates\\Templates"}
};
//const char dummyconfig[] = "";

// MyMap myMap = { {"hello", 42}, {"world", 88} };
	 
string configFileDir = "";
map<string, string> rawConfigData;


bool ReadConfigFile(config & outConfig)
{
	string reaperPath = GetReaperResourcePath();
	configFileDir = reaperPath + kPathSeparator + "UserPlugins";

	ifstream configFile;
	CheckConfigExists();

	configFile.open(configFileDir + kPathSeparator + "csg_reaperwwise.config");

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
	configFile.close();
	bool needsReread = false;
	// Set the config properties
	
	if (rawConfigData.find( "waapiPort" ) != rawConfigData.end())
	{
		outConfig.waapiPort = std::stoi(rawConfigData["waapiPort"]);	//stoi converts string to integer
	}
	else{
		if (insertDefaultValueInConfig("waapiPort")) {needsReread = true;}
	}
	
	if (rawConfigData.find( "useAutomationMode" ) != rawConfigData.end())
	{
		outConfig.useAutomationMode = std::stoi(rawConfigData["useAutomationMode"]);
	}
	else{
		if (insertDefaultValueInConfig("useAutomationMode")) {needsReread = true;}
	}
	
	if (rawConfigData.find( "userorigsubdir" ) != rawConfigData.end())
	{
		outConfig.userOrigDir = rawConfigData["userorigsubdir"];
	}
	else{
		if (insertDefaultValueInConfig("userorigsubdir")) {needsReread = true;}
	}
	
	if (rawConfigData.find( "versionToken" ) != rawConfigData.end())
	{
		outConfig.versionToken = rawConfigData["versionToken"];
	}
	else{
		if (insertDefaultValueInConfig("versionToken")) {needsReread = true;}
	}
	
	if (needsReread)
	{
		ReadConfigFile(outConfig); //call self again to reread any insertions
	}
	return true;
}

bool CheckConfigExists()
{
	ifstream outFile(configFileDir + kPathSeparator +"csg_reaperwwise.config");
	if (!outFile.is_open())
	{
		CreateConfigFile();
	}
	return true;
}

bool CreateConfigFile()
{
	ofstream newFile;
	newFile.open(configFileDir + kPathSeparator + "csg_reaperwwise.config");
	for (auto entry : configFileDefaults)
	{
		newFile << entry.first << "=" << entry.second << "\n";
	}
	//newFile << defaults; //dummyconfig;
	newFile.close();
	return true;
}

bool insertDefaultValueInConfig(string Key)
{
	bool result = false;
	if (configFileDefaults.find(Key) != configFileDefaults.end())
	{
		ofstream configFile;
		CheckConfigExists();
		configFile.open(configFileDir + kPathSeparator + "csg_reaperwwise.config",std::ofstream::out | std::ofstream::app);
		configFile << Key << "=" << configFileDefaults.at(Key) << "\n";
		configFile.close();
		result = true;
	}
	return result;
}
