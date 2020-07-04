#include "reaperHelpers.h"
#include "ReaperRenderQueParser.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <filesystem>

using namespace std;

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
	string resourcePath = GetReaperResourcePath();
	string QrenderPath = resourcePath + "\\QueuedRenders";
	std::vector<string> renderQueFiles;

	for (const auto & p : filesystem::directory_iterator(QrenderPath))
	{
		filesystem::path resourceFile = p.path();
		string s_resourceFile = resourceFile.string();
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
	string resourcePath = GetReaperResourcePath();
	string QrenderPath = resourcePath + "\\QueuedRenders";
	std::vector<string> renderQueFiles;

	//TODO add some saftey checks here. It can crash if it finds a file it doesn't know what to do with. Reaper crashed and left a file with no extension and Japanese characters, which crashed the plugin here

	for (const auto & p : filesystem::directory_iterator(QrenderPath))
	{
		filesystem::path resourceFile = p.path();
		string s_resourceFile = resourceFile.string();
		if (s_resourceFile.find(".rpp") != s_resourceFile.npos)
		{
			renderQueFiles.push_back(s_resourceFile);
		}
	}
	return renderQueFiles;
}

RenderQueJob CreateRenderQueJobFromRenderQueFile(std::string pathToQueFile)
{
	RenderQueJob myRenderQueJob;
	std::vector<string> RenderFiles;
	string RPPfile;
	bool done = false;
	ifstream file;
	file.open(pathToQueFile);
	if (file.good())
	{
		/// Looking for QUEUED_RENDER_OUTFILE
		string line;
		getline(file, line);	//Read the first line here, its always <REAPER_PROJECT 0.1 "5.92/x64" 1532979245 or similar
		while (!done && getline(file, line))
		{
			stringstream tkns(line);
			if (line.find("QUEUED_RENDER_OUTFILE") != line.npos)
			{
				string word = line;
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
					string sStart(start);
					string sEnd(end);
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
					string rppfile;
					while (tkns >> rppfile)
					{
						if (rppfile.find(".rpp") != line.npos)
						{
							RPPfile = rppfile;
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
	
	return myRenderQueJob;
}


void ParseRenderQueFile(string pathToQueFile)
{
	std::vector<string> RenderFiles;
	string RPPfile;

	bool done = false;
	ifstream file;
	//file.open("E:\\Projects\\Personal\\ReaperAPI\\SimonsReaperPlugin\\qrender_TestReaperProject.rpp");
	file.open(pathToQueFile);
	/// Looking for QUEUED_RENDER_OUTFILE
	string line;
	
	getline(file, line);	//Read the first line here, its always <REAPER_PROJECT 0.1 "5.92/x64" 1532979245 or similar

	while (!done && getline(file, line))
	{
		stringstream tkns(line);
		if (line.find("QUEUED_RENDER_OUTFILE") != line.npos)
		{
			string word = line;
			if (word.find(".wav") != line.npos)
			{
				//Found a render output file!
				// --- /////   word.erase(std::remove(word.begin(), word.end(), '"'), word.end());	// Removing "" from the result
				char input[256];
				strcpy(input, word.c_str());
				char* start = strchr(input, '\"')+1;
				char* end = strrchr(input, '\"');
				string sStart(start);
				string sEnd(end);
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
				string rppfile;
				while (tkns >> rppfile)
				{
					if (rppfile.find(".rpp") != line.npos)
					{
						RPPfile = rppfile;
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