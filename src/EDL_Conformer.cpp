//
//  EDL_Conformer.cpp
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 10/01/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#include "EDL_Conformer.h"
#include "platformhelpers.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"
#include "reaperHelpers.h"
#include <filesystem>
#include <fstream>
#include <algorithm>




void Print(std::string inText)
{
	PrintToConsole(inText);
}

std::vector<std::string> EDLconformer::FindTimecodeValuesInString(std::string inString)
{
	std::regex tcPattern("[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]");
	std::vector<std::string> matches;
	std::sregex_iterator iter(inString.begin(), inString.end(),tcPattern);
	std::sregex_iterator end;
	int count=0;
	while(iter != end)
	{
		for(unsigned i = 0; i < iter->size(); ++i)
		{
			matches.push_back((*iter)[i]);
		}
		++iter;
		count++;
	}
	return matches;
}




void EDLconformer::CropProject() { 
	
}


void EDLconformer::FinishWork() { 

}


void EDLconformer::handleUnchangedSections() { 
	
}


void EDLconformer::ExecuteConform() { 
	
}


void EDLconformer::RefreshTimeline() { 
	
}


void EDLconformer::IngestEDLFiles() { 
	
}


void EDLconformer::CropProjectToTime(std::string newProjEndTime) { 
	
}


void EDLconformer::CopyOldSliceToNewTime(std::string oldStart, std::string oldEnd, std::string newStart, std::string NewEnd, std::string ShotName) { 
	
}


void EDLconformer::MovePreviousRegions() { 
	
}


void EDLconformer::CopyPreviousRegions() { 
	
}


void EDLconformer::AddRegion(std::string timeCodeStart, std::string timeCodeEnd, std::string name, int colour) { 
	
}


void EDLconformer::ShiftExistingTimeline() { 
	
}


void EDLconformer::SetTimeSelection(std::string startTimecode, std::string endTimecode) { 
	
}


std::vector<ShotTCInfo> EDLconformer::CreateShotTimeInfo(std::vector<std::string> inFilelines) { 
	std::vector<ShotTCInfo> tcinfo;
	for (int index = 0; index < inFilelines.size()-1; ++index)
	{
		// access using []
		std::string currentLine = inFilelines[index];
		std::string nextLine = inFilelines[index+1];
		std::vector<std::string> matches = FindTimecodeValuesInString(currentLine);
		//did the line contain 4 time code values indicating the shot?
		if (matches.size() == 4)
		{
			//nextLine here is : "* FROM CLIP NAME: TestSequence_EDL_Shot1.avi"
			nextLine.erase(remove(nextLine.begin(), nextLine.end(), '*'), nextLine.end());
			std::size_t found = nextLine.find("FROM CLIP NAME");
			if (found != nextLine.npos)
			{
				// Search for the substring in string
				std::string toErase = " FROM CLIP NAME: ";
				size_t pos = nextLine.find(toErase);
				if (pos != nextLine.npos)
				{
					// If found then erase it from string
					nextLine.erase(pos, toErase.length());
				}
				toErase = ".avi";
				pos = nextLine.find(toErase);
				if (pos != nextLine.npos)
				{
					// If found then erase it from string
					nextLine.erase(pos, toErase.length());
				}
				std::string shotName = nextLine;
				ShotTCInfo shotInfo;
				shotInfo.shotName = shotName;
				shotInfo.sourceStartTC = matches[0];
				shotInfo.sourceEndTC = matches[1];
				shotInfo.destStartTC = matches[2];
				shotInfo.destEndTC = matches[3];
				tcinfo.push_back(shotInfo);
			}
		}
	}
	return tcinfo;
}


std::vector<std::string> EDLconformer::ReadFile(std::string inFilepath) { 
	std::vector<std::string> fileLines;
	std::ifstream File;
	File.open(inFilepath);
	std::string line;
	while (std::getline(File, line))
	{
		if (line != "")
		{
			fileLines.push_back(line);
		}
	}
	File.close();
	return fileLines;
}


void EDLconformer::SetEditCursorToTimecode(std::string inTimecode) {
	int posSeconds = parse_timestr_pos(inTimecode.c_str(),5);
	SetEditCurPos(posSeconds,0,0);
}


bool EDLconformer::GatherAndCheckCommandIDs() {
	
	std::string command = "Script: cfillion_Copy project markers and regions in time selection.lua";
	cmd_CopyRegionsInTimeSelection = NamedCommandLookup("_RSa09cbea8104d59c4dcebdc4aa0f59e2bbe7edb62");
	if (cmd_CopyRegionsInTimeSelection == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Script: cfillion_Paste project markers and regions at edit cursor.lua";
	cmd_PasteRegionsToEditCursor = NamedCommandLookup("_RS31215d883514f64891e953d6ef4baf38a6452f55");
	if (cmd_PasteRegionsToEditCursor == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Time selection: Crop project to time selection";
	cmd_CropProjectToTimeSelection = NamedCommandLookup("40049");
	if (cmd_CropProjectToTimeSelection == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Item: Copy selected area of items";
	cmd_CopySelectedAreaOfItems = NamedCommandLookup("40060");
	if (cmd_CopySelectedAreaOfItems == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Track: Set first selected track as last touched track";
	cmd_SetFirstSelectedTrackAsLastTouched = NamedCommandLookup("40914");
	if (cmd_SetFirstSelectedTrackAsLastTouched == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Item: Paste items/tracks";
	cmd_PasteItemsTracks = NamedCommandLookup("42398");
	if (cmd_PasteItemsTracks == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Time selection: Set start point";
	cmd_SetSelectionStart = NamedCommandLookup("40625");
	if (cmd_SetSelectionStart == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Time selection: Set end point";
	cmd_SetSelectionEnd = NamedCommandLookup("40626");
	if (cmd_SetSelectionEnd == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Time selection: Insert empty space at time selection (moving later items)";
	cmd_InsertEmptySpaceAtSelection = NamedCommandLookup("40200");
	if (cmd_InsertEmptySpaceAtSelection == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Time selection: Crop project to time selection";
	cmd_CropProjectToSelection = NamedCommandLookup("40049");
	if (cmd_CropProjectToSelection == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "SWS: Split items at time selection (if exists), else at edit cursor (also during playback)";
	cmd_SplitAndSelectItemsAtTimeSelection = NamedCommandLookup("_SWS_SMARTSPLIT2");
	if (cmd_SplitAndSelectItemsAtTimeSelection == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Track: Select all tracks";
	cmd_SelectAllTracks = NamedCommandLookup("40296");
	if (cmd_SelectAllTracks == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Script: js_Area selection - Duplicate items and automation in time selection of selected tracks to edit cursor.lua";
	cmd_DuplicateItemsInSelectionToEditCursor = NamedCommandLookup("_RS046c3a9a2e621a1ea58e6aac40b563161ccf67bd");
	if (cmd_DuplicateItemsInSelectionToEditCursor == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Track: Unselect all tracks";
	cmd_UnSelectAllTracks = NamedCommandLookup("40297");
	if (cmd_UnSelectAllTracks == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "Item: Unselect all items";
	cmd_UnselectAllItems = NamedCommandLookup("40289");
	if (cmd_UnselectAllItems == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	command = "ReaScript: Close all running ReaScripts";
	cmd_CloseAllRunningScripts = NamedCommandLookup("41898");
	if (cmd_CloseAllRunningScripts == 0)
	{
		Print("Error! Missing one or more required Reaper actions!!");
		Print(command);
		return false;
	}
	
	Print("All command IDs found. Success");
	return true;
}


std::string EDLconformer::SecondsToTimecodeString(float inSeconds) { 
	std::string result = "";
	std::stringstream temp;
	int wholeSeconds = std::floorf(inSeconds);
	float decimals = inSeconds - wholeSeconds;
	int hours = std::floorf(wholeSeconds/3600);
	int mins = std::floorf((wholeSeconds/60) % 60);
	int secs = std::floorf(wholeSeconds % 60);
	int frames = (framerate * decimals);
	
	temp << std::internal << std::setfill('0') << std::setw(2) << hours <<
	std::internal << std::setfill('0') << std::setw(2) << mins <<
	std::internal << std::setfill('0') << std::setw(2) << secs <<
	std::internal << std::setfill('0') << std::setw(2) << frames;
	
	result = temp.str();
	return result;
}

float EDLconformer::TimecodeToSeconds(std::string inTimecode) {
	std::stringstream result;
	float seconds = 0.0f;
	int i = 0;
	std::vector<std::string> tokens = stringSplitToList(inTimecode, ":");
	for (auto token : tokens)
	{
		switch (i) {
			case 0:
				//hours
				seconds += std::stof(token) * 3600;
				break;
			case 1:
				//mins
				seconds += std::stof(token) * 60;
				break;
			case 2:
				//secs
				seconds += std::stof(token);
				break;
			case 3:
				//frames
				seconds += std::stof(token) * (1/framerate);
				break;
			default:
				break;
		}
		i++;
	}
	result << std::setprecision(5) << seconds;
	result >> seconds;
	return seconds;
}

bool EDLconformer::TimeIsEqual(float num1, float num2, int decimalPlaces) {
	return std::abs(TruncateFloat(num1,decimalPlaces) - TruncateFloat(num2,decimalPlaces)) <= (1/std::pow(10, decimalPlaces));
}


float EDLconformer::TruncateFloat(float inFloat, int decimalPlaces) { 
	float result = 0.0f;
	std::string ints = "0";
	std::string decs = "0";
	int i = 0;
	std::vector<std::string> tokens = stringSplitToList(std::to_string(inFloat), ".");
	for (auto token : tokens)
	{
		switch (i) {
			case 0:
				ints = token;
				break;
			case 1:
				decs = token.substr(0,decimalPlaces);
			default:
				break;
		}
		i++;
	}
	result = std::stof(ints + "." + decs);
	return result;
}



