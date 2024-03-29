#include "reaperHelpers.h"
#include "ReaperRenderQueParser.h"
#include "waapi_structs.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "platformhelpers.h"

#include <filesystem>


//using namespace std;

/////
////  This is an example of what a render que file looks like. 1st line is header, then the WAV files, then the RPP project
////
//<REAPER_PROJECT 0.1 "5.92/x64" 1532979245
//	QUEUED_RENDER_OUTFILE "E:\Projects\Personal\ReaperAPI\TestReaperProject\Media\TestReaperProject1.wav" 3
//	QUEUED_RENDER_OUTFILE "E:\Projects\Personal\ReaperAPI\TestReaperProject\Media\TestReaperProject2.wav" 3
//	QUEUED_RENDER_OUTFILE "E:\Projects\Personal\ReaperAPI\TestReaperProject\Media\TestReaperProject3.wav" 3
//	QUEUED_RENDER_OUTFILE "E:\Projects\Personal\ReaperAPI\TestReaperProject\Media\TestReaperProject4.wav" 3
//	QUEUED_RENDER_OUTFILE "E:\Projects\Personal\ReaperAPI\TestReaperProject\Media\TestReaperProject5.wav" 3
//	QUEUED_RENDER_OUTFILE "E:\Projects\Personal\ReaperAPI\TestReaperProject\Media\TestReaperProject6.wav" 3
//	QUEUED_RENDER_ORIGINAL_FILENAME E : \Projects\Personal\ReaperAPI\TestReaperProject\TestReaperProject.rpp
//	RIPPLE 0
//	GROUPOVERRIDE 1 0 1

// Example Case need to handle filenames with spaces! -   QUEUED_RENDER_OUTFILE "N:\Projects\CSG\Audio\_Development Projects\WWS-E\London Studio\VR2\ProjectFiles\Simon\HeroMoments\HERO_Desert_CarSmashGate\Exports\V1\VR2_SFX_HERO_Desert_GateSmash_B_Smash_CARPARTS 2D.wav" 3



void TestReadRenderQue()
{
	std::string resourcePath = GetReaperResourcePath();
	std::string QrenderPath = resourcePath + PLATFORMHELPERS::kPathSeparator +"QueuedRenders";
	std::vector<std::string> renderQueFiles;

	for (const auto & p : std::filesystem::directory_iterator(QrenderPath))
	{
		std::filesystem::path resourceFile = p.path();
		std::string s_resourceFile = resourceFile.string();
		if (s_resourceFile.find(".rpp") != s_resourceFile.npos)
		{
			renderQueFiles.push_back(s_resourceFile);
		}
	}
	for (auto RenderQueFile : renderQueFiles)
	{
		ParseRenderQueFile(RenderQueFile);
	}
}

std::vector<std::string> GetListOfRenderQues()
{
	std::string resourcePath = GetReaperResourcePath();
	std::string QrenderPath = resourcePath + PLATFORMHELPERS::kPathSeparator + "QueuedRenders";
	std::vector<std::string> renderQueFiles;

	//TODO add some saftey checks here. It can crash if it finds a file it doesn't know what to do with. Reaper crashed and left a file with no extension and Japanese characters, which crashed the plugin here

	for (const auto & p : std::filesystem::directory_iterator(QrenderPath))
	{
		std::filesystem::path resourceFile = p.path();
		std::string s_resourceFile = resourceFile.string();
		//make a lower case version to search for .rpp extension
		std::string s_resourceFile_ToLower = s_resourceFile;
		transform(s_resourceFile_ToLower.begin(), s_resourceFile_ToLower.end(), s_resourceFile_ToLower.begin(), [](unsigned char c){ return tolower(c); });
		
		if (s_resourceFile_ToLower.find(".rpp") != s_resourceFile_ToLower.npos)
		{
			//Push back the original file name
			renderQueFiles.push_back(s_resourceFile);
		}
	}
	return renderQueFiles;
}

RenderQueJob CreateRenderQueJobFromRenderQueFile(std::string pathToQueFile)
{
	RenderQueJob myRenderQueJob;
	std::vector<std::string> RenderFiles;
	std::string RPPfile;
	bool done = false;
	std::ifstream file;
	file.open(pathToQueFile);
	if (file.good())
	{
		/// Looking for QUEUED_RENDER_OUTFILE
		std::string line;
		getline(file, line);	//Read the first line here, its always <REAPER_PROJECT 0.1 "5.92/x64" 1532979245 or similar
		while (!done && getline(file, line))
		{
			std::stringstream tkns(line);
			if (line.find("QUEUED_RENDER_OUTFILE") != line.npos)
			{
				std::string word = line;
				if (word.find(".wav") != line.npos)
				{
					//Found a render output file!
					// --- /////   word.erase(std::remove(word.begin(), word.end(), '"'), word.end());	// Removing "" from the result
					if (word.length() > 256) {
						PrintToConsole("WARNING! Export path found in render que greater than 256 chars. File not availble for import");
						continue;
					}
					char input[256];
					strcpy(input, word.c_str());
					char* start = strchr(input, '\"') + 1;
					char* end = strrchr(input, '\"');
					std::string sStart(start);
					std::string sEnd(end);
					size_t eraseFrom = sStart.find(sEnd);
					sStart.erase(eraseFrom, sStart.npos);
					RenderFiles.push_back(sStart);
					//PrintToConsole(line);
				}
			}
			else
			{
				if (line.find("QUEUED_RENDER_ORIGINAL_FILENAME") != line.npos)
				{
					std::string rppfile;
					while (tkns >> rppfile)
					{
						if (PLATFORMHELPERS::stringToLower(rppfile).find(".rpp") != line.npos)
						{
							RPPfile = PLATFORMHELPERS::stringToLower(rppfile);
						}
					}
				}
				else { done = true; break; }
			}
		}

	}
	else
	{
		PrintToConsole("Error opening render que file: " + pathToQueFile);
		return myRenderQueJob;
	}

	myRenderQueJob.RenderQueFilePath = pathToQueFile;
	myRenderQueJob.RenderQueJobFileList = RenderFiles;
	myRenderQueJob.ParentReaperProject = RPPfile;
	
	return myRenderQueJob;
}


void ParseRenderQueFile(std::string pathToQueFile)
{
	std::vector<std::string> RenderFiles;
	std::string RPPfile;

	bool done = false;
	std::ifstream file;
	//file.open("E:\\Projects\\Personal\\ReaperAPI\\SimonsReaperPlugin\\qrender_TestReaperProject.rpp");
	file.open(pathToQueFile);
	/// Looking for QUEUED_RENDER_OUTFILE
	std::string line;
	
	getline(file, line);	//Read the first line here, its always <REAPER_PROJECT 0.1 "5.92/x64" 1532979245 or similar

	while (!done && getline(file, line))
	{
		std::stringstream tkns(line);
		if (line.find("QUEUED_RENDER_OUTFILE") != line.npos)
		{
			std::string word = line;
			if (word.find(".wav") != line.npos)
			{
				//Found a render output file!
				// --- /////   word.erase(std::remove(word.begin(), word.end(), '"'), word.end());	// Removing "" from the result
				char input[256];
				strcpy(input, word.c_str());
				char* start = strchr(input, '\"')+1;
				char* end = strrchr(input, '\"');
				std::string sStart(start);
				std::string sEnd(end);
				size_t eraseFrom = sStart.find(sEnd);
				sStart.erase(eraseFrom, sStart.npos);
				RenderFiles.push_back(sStart);
				//PrintToConsole(line);
			}
		}
		else
		{
			if (line.find("QUEUED_RENDER_ORIGINAL_FILENAME") != line.npos)
			{
				std::string rppfile;
				while (tkns >> rppfile)
				{
					if (PLATFORMHELPERS::stringToLower(rppfile).find(".rpp") != line.npos)
					{
						RPPfile = PLATFORMHELPERS::stringToLower(rppfile);
					}
				}
			}
			else { done = true; break; }
		}
	}

	PrintToConsole("Reaper Render Items...");
	for (auto file : RenderFiles)
	{
		PrintToConsole(file);
	}
	PrintToConsole(RPPfile + "\n");

}
