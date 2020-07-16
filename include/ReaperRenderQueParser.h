#pragma once
#include <string>
#include <map>
#include <vector>
//#include "WwiseConnectionHandler.h"


typedef struct WwiseObject WwiseObject;

struct WwiseObject
{
	std::map<std::string, std::string> properties;
	std::map<std::string, double> numericProperties;
	std::vector<WwiseObject*> childObjects;
	WwiseObject* parentObject;
};

struct RenderJobFileOverride
{
	std::string RenderJobFile;
	WwiseObject parentWwiseObject;
	bool isVoice = false;
	std::string ImportLanguage;
	bool OrigDirMatchesWwise = false;
	std::string userOrigsSubDir;
	bool hasImported = false;
	std::string createEventOption;
};

struct RenderQueJob
{
	std::string RenderQueFilePath;
	WwiseObject parentWwiseObject;
	std::vector<std::string> RenderQueJobFileList;
	std::string ParentReaperProject;
	bool hasRendered = false;
	bool hasImported = false;
	bool isVoice = false;
	std::string ImportLanguage;
	bool OrigDirMatchesWwise = false;
	std::string userOrigsSubDir;
	bool hasPerFileOverrides = false;
	std::map<std::string, RenderJobFileOverride> perFileOverridesmap;
	std::string createEventOption;
	

};

//Test Functions

void ParseRenderQueFile(std::string pathToQueFile);

void TestReadRenderQue();


//Real functions

std::vector<std::string> GetListOfRenderQues();

RenderQueJob CreateRenderQueJobFromRenderQueFile(std::string pathToQueFile);
