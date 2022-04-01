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
#include <unordered_map>




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
	auto NewEndTime_secs = TimecodeToSeconds(EdlCompSettings.timeLineOffset) * 2;
	SetTimeSelection("00:00:00:00",SecondsToTimecodeString(NewEndTime_secs));
	Main_OnCommand(cmd_CropProjectToSelection,0);
}


void EDLconformer::FinishWork() { 
	Print("Finished");
	UpdateTimeline();
	Main_OnCommand(cmd_CloseAllRunningScripts,0);
	ResetConform();
}


void EDLconformer::PrepareUnchangedSections() {
	int64_t unchangedSectionIndex = -1;
	int64_t lastUnchangedSourceStart = -1;
	int64_t lastUnchangedSourceEnd = -1;
	int64_t lastUnchangedDestStart = -1;
	int64_t lastUnchangedDestEnd = -1;
	int64_t newShotOffset = INT_MAX;
	int64_t lastChangeStartTime = -1;
	int64_t lastChangeEndTime = -1;
	
	for (auto result : conformResults)
	{
		auto oldShot = result.oldShot;
		auto newShot = result.newShot;
		bool changed = result.changed;
		
		if (changed or newShot.empty)
		{
			lastUnchangedSourceStart = -1;
			if (changed)
			{
				lastChangeStartTime = TimecodeToFrames(newShot.destStartTC);
				lastChangeEndTime = TimecodeToFrames(newShot.destEndTC);
			}
		}
		else
		{
			auto sourceStartTime_frames = TimecodeToFrames(oldShot.destStartTC);
			auto sourceEndTime_frames = TimecodeToFrames(oldShot.destEndTC);
			auto destStartTime_frames = TimecodeToFrames(newShot.destStartTC);
			auto destEndTime_frames = TimecodeToFrames(newShot.destEndTC);
			
			if ((lastChangeStartTime != -1)
				and (destStartTime_frames >= lastChangeStartTime)
				and (destStartTime_frames <= lastChangeEndTime))
			{
				lastUnchangedSourceStart = -1; // clear the tracking of unchanged shots if this shot clashes with a known change
			}
			if ((lastUnchangedSourceStart == -1) or (destStartTime_frames-sourceStartTime_frames != newShotOffset))
			{
				//new unchanged section save the start times and increment the index
				unchangedSectionIndex = unchangedSectionIndex + 1;
				lastUnchangedSourceStart = sourceStartTime_frames;
				lastUnchangedDestStart = destStartTime_frames;
				newShotOffset = destStartTime_frames - sourceStartTime_frames;
				lastUnchangedSourceEnd = sourceEndTime_frames;
				lastUnchangedDestEnd = destEndTime_frames;
				ShotTCInfo info;
				info.sourceStartTC = FramesToTimecodeString(lastUnchangedSourceStart);
				info.sourceEndTC = FramesToTimecodeString(lastUnchangedSourceEnd);
				info.destStartTC = FramesToTimecodeString(lastUnchangedDestStart);
				info.destEndTC = FramesToTimecodeString(lastUnchangedDestEnd);
				info.shotName = oldShot.shotName;
				info.empty = false;
				
				if (unchangedSectionIndex >= unchangedSections.size()) {
					unchangedSections.resize(unchangedSectionIndex+1);
				}
				unchangedSections[unchangedSectionIndex] = info;
				
			}
			else
			{
				ShotTCInfo & info = unchangedSections[unchangedSectionIndex];
				lastUnchangedSourceEnd = std::fmax(FramesToSeconds(sourceEndTime_frames),TimecodeToSeconds( info.sourceEndTC));
				lastUnchangedDestEnd = std::fmax(FramesToSeconds(destEndTime_frames),TimecodeToSeconds(info.destEndTC));
				info.sourceEndTC = SecondsToTimecodeString(lastUnchangedSourceEnd);
				info.destEndTC = SecondsToTimecodeString(lastUnchangedDestEnd);
			}
		}
	}
}


void EDLconformer::PrepareChangedSections() { 
	bool working = true;
	Print("Starting Conform.......");
	Print("_____________________");
	Print(" ");
	Print("Number of old shots = "+std::to_string(old_shotTimeInfo.size()));
	Print("Number of new shots = "+std::to_string(new_shotTimeInfo.size()));
	PreventUIRefresh(1);
	
	int i = 0;
	std::unordered_map<std::string,int> oldShotCounts;
	while (i < old_shotTimeInfo.size())
	{
		int matchingOldShotIndex = i;
		if (matchingOldShotIndex > old_shotTimeInfo.size()-1){
			Print("No more old shots to check");
			break;
		}
		auto shot = old_shotTimeInfo[matchingOldShotIndex].shotName;
		
		//store the old shot in a result table, assume changed  = false by default until we find a match and determine change later
		//this is used later on to group the unchanged sections together
		ConformResultsInfo cinfo;
		cinfo.oldShot = old_shotTimeInfo[matchingOldShotIndex];
		conformResults.push_back(cinfo);
		//track how many times we've seen this shot in the list
		if (oldShotCounts.find(shot)==oldShotCounts.end())
		{
			oldShotCounts[shot] = 1;
		}else
		{
			oldShotCounts[shot]++;
		}
		int matchingNewShotIndex = -1;
		bool shotChanged = false;
		bool shotHasAnimChanges = false;
		bool foundMatch = false;
		int matchCount = 0;
		
		int newShotIndex = 0;
		//try to find a matching entry in the new shot list
		while ((newShotIndex < new_shotTimeInfo.size()) and not foundMatch)
		{
			//Print("Comparing..."+shot+" with "+new_shotTimeInfo[newShotIndex].shotName);
			if (new_shotTimeInfo[newShotIndex].shotName == shot)
			{
				matchCount = matchCount + 1;
				if (matchCount == oldShotCounts[shot])
				{
					foundMatch = true;
					matchingNewShotIndex = newShotIndex;
					//Print("Found a matching shot");
					break;
				}
			}
			newShotIndex = newShotIndex + 1;
		}
		
		if (foundMatch)
		{
			auto oldshotTCinfo = old_shotTimeInfo[matchingOldShotIndex];
			auto newshotTCinfo = new_shotTimeInfo[matchingNewShotIndex];
			if (oldshotTCinfo.empty or newshotTCinfo.empty)
			{
				Print("Invalid shot index after match attempt..Exiting");
				return;
			}
			conformResults[i].oldShot = oldshotTCinfo;
			conformResults[i].newShot = newshotTCinfo;
			
			auto old_offset_In_frames = TimecodeToFrames(oldshotTCinfo.sourceStartTC);
			auto old_offset_Out_frames = TimecodeToFrames(oldshotTCinfo.sourceEndTC);
			auto new_offset_In_frames = TimecodeToFrames(newshotTCinfo.sourceStartTC);
			auto new_offset_Out_frames = TimecodeToFrames(newshotTCinfo.sourceEndTC);
			int old_duration_frames = old_offset_Out_frames - old_offset_In_frames;
			int new_duration_frames = new_offset_Out_frames - new_offset_In_frames;
			
			if ((old_duration_frames != new_duration_frames)
				or (old_offset_In_frames != new_offset_In_frames)
				or (old_offset_Out_frames != new_offset_Out_frames))
			{
				shotChanged = true;
				conformResults[i].changed = true;
			}
			
			


			int shotInternalStartDifference = new_offset_In_frames - old_offset_In_frames;
			int shotInternalEndDifference = new_offset_Out_frames - old_offset_Out_frames;
			int durationDifference = new_duration_frames - old_duration_frames;
			
			int sourceStartTime_frames = TimecodeToFrames(oldshotTCinfo.destStartTC);
			int sourceEndTime_frames = TimecodeToFrames(oldshotTCinfo.destEndTC);
			int destStartTime_frames = TimecodeToFrames(newshotTCinfo.destStartTC);
			int destEndTime_frames = TimecodeToFrames(newshotTCinfo.destEndTC);
			
			if (shotInternalStartDifference > 0)
			{ // new shot starts later in the source
				sourceStartTime_frames = sourceStartTime_frames + shotInternalStartDifference;
				if (new_duration_frames < (sourceEndTime_frames - sourceStartTime_frames))
				{
					sourceEndTime_frames = sourceStartTime_frames + new_duration_frames;
				}
				else
				{//}-- new shot starts earlier or the same in the source
					if (destEndTime_frames < sourceEndTime_frames)
					{
						sourceEndTime_frames = destEndTime_frames;
					}
				}
			}
			auto sourceStartTime_TC = FramesToTimecodeString(sourceStartTime_frames);
			auto sourceEndTime_TC = FramesToTimecodeString(sourceEndTime_frames);
			auto destStartTime_TC = FramesToTimecodeString(destStartTime_frames);
			auto destEndTime_TC = FramesToTimecodeString(destEndTime_frames);
			
			
			if (shotHasAnimClipChanges(oldshotTCinfo, newshotTCinfo))
			{
				shotHasAnimChanges = true;
				conformResults[i].animClipChanged = true;
				///TODO - should we track this as a shot change?
				//shotChanged = true;
				//conformResults[i].changed = true;
				animClipChangedSections.push_back(newshotTCinfo);
			}
			
			
			if (shotChanged)
			{
				auto sourceEndTimeTrimmed = FramesToTimecodeString(sourceEndTime_frames);
				if (new_duration_frames < old_duration_frames)
				{
					sourceEndTimeTrimmed = FramesToTimecodeString(sourceStartTime_frames + new_duration_frames);
				}
				ShotTCInfo changedShot;
				changedShot.sourceStartTC = sourceStartTime_TC;
				changedShot.sourceEndTC = sourceEndTimeTrimmed;
				changedShot.destStartTC = destStartTime_TC;
				changedShot.destEndTC = destEndTime_TC;
				changedShot.shotName = shot;
				changedShot.empty = false;
				changedShot.ShotAnimInfos = newshotTCinfo.ShotAnimInfos;
				changedSections.push_back(changedShot);
				
			}
			else
			{
				conformResults[i].changed = false;
			}
		}
		i++;
	}
	
	working = false;
	PreventUIRefresh(-1);
}


void EDLconformer::RefreshTimeline() { 
	UpdateTimeline();
}


bool EDLconformer::IngestEDLFiles() {
	if (not std::filesystem::exists(filepath_Old_EDL))
	{
		PrintToConsole("Warning! Invalid filepath provided for OLD edl");
		return false;
	}
	if (not std::filesystem::exists(filepath_New_EDL))
	{
		PrintToConsole("Warning! Invalid filepath provided for NEW edl");
		return false;
	}
	
	std::vector<std::string> old_fileLines = ReadFile(filepath_Old_EDL);
	std::vector<std::string> new_fileLines = ReadFile(filepath_New_EDL);

	old_shotTimeInfo = CreateShotTimeInfo(old_fileLines);
	if (not old_shotTimeInfo.empty())
	{
		originalEndTime = old_shotTimeInfo.back().destEndTC;
	}
	new_shotTimeInfo = CreateShotTimeInfo(new_fileLines);
	return (not old_shotTimeInfo.empty() && not new_shotTimeInfo.empty());
}


void EDLconformer::CropProjectToTime(std::string newProjEndTime) { 
	UpdateTimeline();
}


void EDLconformer::CopyOldSliceToNewTime(std::string oldStart, std::string oldEnd, std::string newStart, std::string NewEnd, std::string ShotName) { 
	Main_OnCommand(cmd_UnSelectAllTracks,0);
	Main_OnCommand(cmd_UnselectAllItems,0);
	float offsetInSeconds = TimecodeToSeconds(EdlCompSettings.timeLineOffset);
	float oldStartWOffset_secs = TimecodeToSeconds(oldStart) + offsetInSeconds;
	std::string oldStartWOffset_TC = SecondsToTimecodeString(oldStartWOffset_secs);
	float oldEndWOffset_secs = TimecodeToSeconds(oldEnd) + offsetInSeconds;
	std::string oldEndWOffset_TC = SecondsToTimecodeString(oldEndWOffset_secs);
	SetTimeSelection(oldStartWOffset_TC,oldEndWOffset_TC);
	SetEditCursorToTimecode(newStart);
	Main_OnCommand(cmd_SelectAllTracks,0);
	Main_OnCommand(cmd_DuplicateItemsInSelectionToEditCursor,0);
}


void EDLconformer::MovePreviousRegions() { 
	float originalContentEndTime_secs = TimecodeToSeconds(originalEndTime) + TimecodeToSeconds(EdlCompSettings.timeLineOffset);
	float originalContentStartTime_secs = TimecodeToSeconds(EdlCompSettings.timeLineOffset);
	int markerCount = countMarkers() + countRegions();
	for (int i = 0; i < markerCount; i++)
	{
		bool isRegion;
		double pos;
		double regEnd;
		//std::string name;
		const char* name;
		int index;
		EnumProjectMarkers(i, &isRegion,&pos,&regEnd,&name,&index);
		if (pos >= originalContentStartTime_secs and pos < originalContentEndTime_secs)
		{
			if (isRegion)
			{
				float newPos = pos - originalContentStartTime_secs;
				float newEnd = regEnd - originalContentStartTime_secs;
				SetProjectMarker( index, isRegion, newPos, newEnd, name );
			}
			else
			{
				float newPos = pos - originalContentStartTime_secs;
				SetProjectMarker( index, isRegion, newPos, newPos, name );
			}
		}
	}
}


void EDLconformer::CopyPreviousRegions() { 
	float originalEndTime_secs = TimecodeToSeconds(originalEndTime) + TimecodeToSeconds(EdlCompSettings.timeLineOffset);
	//Print("Copy Regions");
	//Print(Settings.timeLineOffset);
	//Print(SecondsToTimecodeString(originalEndTime_secs));
	SetTimeSelection(EdlCompSettings.timeLineOffset,SecondsToTimecodeString(originalEndTime_secs));
	Main_OnCommand(cmd_CopyRegionsInTimeSelection,0);
	SetEditCursorToTimecode("00:00:00:00");
	Main_OnCommand(cmd_PasteRegionsToEditCursor,0);
}


void EDLconformer::AddRegion(std::string timeCodeStart, std::string timeCodeEnd, std::string name, int colour) { 
	AddProjectMarker2(0, true, TimecodeToSeconds(timeCodeStart), TimecodeToSeconds(timeCodeEnd), name.c_str(), 0, colour);
}


void EDLconformer::ShiftExistingTimeline() { 
	SetTimeSelection("00:00:00:00",EdlCompSettings.timeLineOffset);
	Main_OnCommand(cmd_InsertEmptySpaceAtSelection,0);
}


void EDLconformer::SetTimeSelection(std::string startTimecode, std::string endTimecode) { 
	SetEditCursorToTimecode(startTimecode);
	Main_OnCommand(cmd_SetSelectionStart,0);
	SetEditCursorToTimecode(endTimecode);
	Main_OnCommand(cmd_SetSelectionEnd,0);
}


std::vector<ShotTCInfo> EDLconformer::CreateShotTimeInfo(std::vector<std::string> inFilelines) {
	std::vector<ShotTCInfo> tcinfo;
	if (inFilelines.empty()) return tcinfo;
	for (int64_t index = 0; index < inFilelines.size()-1; ++index)
	{
		// access using []
		std::string currentLine = inFilelines[index];
		
		std::vector<std::string> matches = FindTimecodeValuesInString(currentLine);
		//did the line contain 4 time code values indicating the shot?
		if (matches.size() == 4)
		{
			// Read through the next chunk of lines to gather the info for this shot. It looks like this for Unreal
			/*
			* FROM CLIP NAME: SC01_0010_V2.avi

			* SKELETAL ACTOR: BP_Eva_Full
			* ANIMATION CLIP: Sc01_Sh10_Through_Sc02_Sh90_Master_tk009_Eva
			* FRAME RANGE FROM: 0 TO: 149
			* TIMECODE RANGE FROM: 09:35:24:07 TO: 09:35:29:05
			*/
			ShotTCInfo shotInfo;
			shotInfo.sourceStartTC = matches[0];
			shotInfo.sourceEndTC = matches[1];
			shotInfo.destStartTC = matches[2];
			shotInfo.destEndTC = matches[3];
			shotInfo.empty = false;

			int64_t nextLineIndex = index + 1;
			bool done = false;
			while (!done && nextLineIndex < inFilelines.size() - 1)
			{
				std::string nextLine = inFilelines[nextLineIndex];

				std::vector<std::string> matches = FindTimecodeValuesInString(nextLine);
				//we hit the start of another shot so break out of the while loop
				if (matches.size() == 4) {
					done = true;
					break;
				}
				if (nextLine.empty()){
					//line is empty so increment the index and move on
					nextLineIndex++;
					continue;
				}
				nextLine.erase(remove(nextLine.begin(), nextLine.end(), '*'), nextLine.end());
				std::size_t found;

				found = nextLine.find("FROM CLIP NAME"); 
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
					shotInfo.shotName = shotName;
					nextLineIndex++;
					continue;
				}
				found = nextLine.find("ANIMATION CLIP");
				if (found != nextLine.npos)
				{
					// Search for the substring in string
					std::string toErase = " ANIMATION CLIP: ";
					size_t pos = nextLine.find(toErase);
					if (pos != nextLine.npos)
					{
						// If found then erase it from string
						nextLine.erase(pos, toErase.length());
					}
					std::string animClipName = nextLine;
					ShotAnimInfo shotAnimInfo;
					shotAnimInfo.animClipName = animClipName;

					if ((nextLineIndex + 2) < (inFilelines.size() - 1))
					{
						std::string shotAnimTCline = inFilelines[nextLineIndex + 2];
						std::vector<std::string> animmatches = FindTimecodeValuesInString(shotAnimTCline);
						if (animmatches.size() == 2)
						{
							shotAnimInfo.animTCStart = animmatches[0];
							shotAnimInfo.animTCEnd = animmatches[1];
							shotInfo.ShotAnimInfos.try_emplace(animClipName, shotAnimInfo);
						}
					}
					nextLineIndex++;
					continue;
				}

				nextLineIndex++;
			}
			tcinfo.push_back(shotInfo);
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
	double posSeconds = parse_timestr_pos(inTimecode.c_str(),5);
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
	
	//Print("All command IDs found. Success");
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
	int frames = (EdlCompSettings.framerate * decimals);
	
	temp << std::internal << std::setfill('0') << std::setw(2) << hours << ":" <<
	std::internal << std::setfill('0') << std::setw(2) << mins << ":" <<
	std::internal << std::setfill('0') << std::setw(2) << secs << ":" <<
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
				seconds += std::stof(token) * (1/EdlCompSettings.framerate);
				break;
			default:
				break;
		}
		i++;
	}
//	result << std::setprecision(5) << seconds;
//	result >> seconds;
	return seconds;
}

bool EDLconformer::TimeIsEqual(float num1, float num2, int decimalPlaces) {
	auto isequal = std::abs(num1 - num2) <= (1/std::pow(10, decimalPlaces));
	return isequal;
}

bool EDLconformer::shotHasAnimClipChanges(ShotTCInfo oldShot, ShotTCInfo newShot)
{
	if (oldShot.ShotAnimInfos.size() != newShot.ShotAnimInfos.size()) { return true; }
	for (auto oldAnimClip : oldShot.ShotAnimInfos)
	{
		auto clipName = oldAnimClip.first;
		if (newShot.ShotAnimInfos.find(clipName) == newShot.ShotAnimInfos.end()) { return true; }
		if (oldAnimClip.second.animTCStart != newShot.ShotAnimInfos[clipName].animTCStart) { return true; }
		if (oldAnimClip.second.animTCEnd != newShot.ShotAnimInfos[clipName].animTCEnd) { return true; }
	}
	return false;
}


float EDLconformer::TruncateFloat(float inFloat, int decimalPlaces, std::string& outInts, std::string& outDecs)
{
	float result = 0.0f;
	std::string ints = "0";
	std::string decs = "0";
	int i = 0;
	std::stringstream floatToString;
	floatToString.precision(decimalPlaces);
	floatToString << std::fixed << inFloat;
	std::vector<std::string> tokens = stringSplitToList(floatToString.str(), ".");
	for (auto token : tokens)
	{
		switch (i) {
			case 0:
				ints = token;
				break;
			case 1:
				decs = token.substr(0,decimalPlaces);
				break;
			default:
				break;
		}
		i++;
	}
	outInts = ints;
	outDecs = decs;
	result = std::stof(ints + "." + decs);
	return result;
}

void EDLconformer::Init() {
	GatherAndCheckCommandIDs();
}

float EDLconformer::FramesToSeconds(int inFrames) { 
	return inFrames / EdlCompSettings.framerate;
}


std::string EDLconformer::FramesToTimecodeString(int inFrames) { 
	std::string result = "";
	std::stringstream temp;
	int framesPerhour = EdlCompSettings.framerate * 3600;
	int framesPermin = EdlCompSettings.framerate * 60;
	int hours = std::floorf(inFrames/framesPerhour);
	inFrames = inFrames - (hours * framesPerhour);
	int mins = std::floorf(inFrames/framesPermin);
	inFrames = inFrames - (mins * framesPermin);
	int secs = std::floorf(inFrames / EdlCompSettings.framerate);
	//inFrames = inFrames - (secs * Settings.framerate);
	int frames = inFrames - (secs * EdlCompSettings.framerate);
	
	temp << std::internal << std::setfill('0') << std::setw(2) << hours << ":" <<
	std::internal << std::setfill('0') << std::setw(2) << mins << ":" <<
	std::internal << std::setfill('0') << std::setw(2) << secs << ":" <<
	std::internal << std::setfill('0') << std::setw(2) << frames;
	
	result = temp.str();
	return result;
}


int64_t EDLconformer::TimecodeToFrames(std::string inTimecode) { 
	std::stringstream result;
	int framesPerhour = EdlCompSettings.framerate * 3600;
	int framesPermin = EdlCompSettings.framerate * 60;
	int frames = 0;
	int i = 0;
	std::vector<std::string> tokens = stringSplitToList(inTimecode, ":");
	for (auto token : tokens)
	{
		switch (i) {
			case 0:
				//hours
				frames += std::stoi(token) * framesPerhour;
				break;
			case 1:
				//mins
				frames += std::stoi(token) * framesPermin;
				break;
			case 2:
				//secs
				frames += std::stoi(token) * EdlCompSettings.framerate;
				break;
			case 3:
				//frames
				frames += std::stof(token);
				break;
			default:
				break;
		}
		i++;
	}
//	result << std::setprecision(5) << seconds;
//	result >> seconds;
	return frames;
}


bool EDLconformer::DoConform() { 

	if (not isConformReady())
	{
		if (not SetupConform())
		{
			Print("Some required data missing for conform..");
			Print("Check EDL files and run comparison again..");
			return false;
		}
	}
	Undo_BeginBlock(); //Begining of the undo block. Leave it at the top of your main function.
	
	ShiftExistingTimeline();
		
	if (EdlCompSettings.CopyExistingRegions) {
		MovePreviousRegions();
	}
	
	if (EdlCompSettings.CreateEDLFileRegion){
		auto startTime = new_shotTimeInfo.front().destStartTC;
		auto endTime = new_shotTimeInfo.back().destEndTC;
		int colour = ColorToNative(255,255,255)|16777216; //white
		AddRegion(startTime,endTime,filepath_New_EDL,colour);
	}
	
	for (auto changedShot : changedSections)
	{
		CopyOldSliceToNewTime(changedShot.sourceStartTC,changedShot.sourceEndTC,changedShot.destStartTC,changedShot.destEndTC, changedShot.shotName);
		Print("CHANGE: "+changedShot.shotName+": New Timecode: "+changedShot.destStartTC+" "+changedShot.destEndTC);
		if (EdlCompSettings.CreateRegionsForChangedShots)
		{
			int colour = ColorToNative(255,255,0)|16777216;// -- yellow
			AddRegion( changedShot.destStartTC, changedShot.destEndTC ,"CHANGE: "+ changedShot.shotName ,colour);
		}
	}
	
	int i = 0;
	for (auto section : unchangedSections)
	{
		i++;
		auto sourceStartTime_TC = section.sourceStartTC;
		auto sourceEndTime_TC = section.sourceEndTC;
		auto destStartTime_TC = section.destStartTC;
		auto destEndTime_TC = section.destEndTC;
		CopyOldSliceToNewTime(sourceStartTime_TC,sourceEndTime_TC,destStartTime_TC,destEndTime_TC, "Unchanged section "+std::to_string(i));
	}
	
	if (EdlCompSettings.CreateRegionsForChangedShots)
	{
		int colour = ColorToNative(255,0,255)|16777216;// -- pink
		for (auto animChangeSection : animClipChangedSections)
		{
			AddRegion( animChangeSection.destStartTC, animChangeSection.destEndTC ,"ANIMATION CLIP CHANGE: " ,colour);
		}
		
	}
	
	
	CropProject();
	FinishWork();
	Undo_EndBlock("CSG EDL Conform", -1);
	return true;
}

void EDLconformer::ResetConform() {
	unchangedSections.clear();
	changedSections.clear();
	animClipChangedSections.clear();
	old_shotTimeInfo.clear();
	new_shotTimeInfo.clear();
	conformResults.clear();
}

bool EDLconformer::isConformReady() { 
	return not
	(
	 unchangedSections.empty() or
	 changedSections.empty() or
	 old_shotTimeInfo.empty() or
	 new_shotTimeInfo.empty() or
	 conformResults.empty()
	 );
}

bool EDLconformer::SetupConform() {
	ResetConform();
	if (IngestEDLFiles())
	{
		PrepareChangedSections();
		PrepareUnchangedSections();
	}
	return isConformReady();
}

float EDLconformer::GetNewEndTime() {
	if (not isConformReady()) return 0.0f;
	return fmax(
				TimecodeToSeconds(changedSections.back().destEndTC),
				TimecodeToSeconds(unchangedSections.back().destEndTC)
				);
}

void EDLconformer::SaveSettingsToExtState() { 
	std::string name = "CSGEDLSettings";
	std::stringstream valuesToJson;
	//"{ 'id': 1234, 'name': 'nandini' }"
	valuesToJson << '{';
	valuesToJson << "'" << EdlCompSettings.CreateEDLFileRegion << "'" << ",";
	valuesToJson << "'" << EdlCompSettings.CopyExistingRegions << "'" << ",";
	valuesToJson << "'" << EdlCompSettings.CreateRegionsForChangedShots << "'" << ",";
	valuesToJson << "'" << EdlCompSettings.timeLineOffset << "'" << ",";
	valuesToJson << "'" << EdlCompSettings.framerate << "'";
	valuesToJson << "}";
	saveProjExState("EDL", valuesToJson.str(), name);
}


void EDLconformer::LoadSettingsFromExtState() { 
	
	std::string svalue = "";
	std::vector<std::string> tempListValues;
	svalue = getProjExState("EDL", "CSGEDLSettings");
	
	if (svalue.empty()) {return;}
	
	char* pch;
	printf("Splitting string \"%s\" into tokens:\n", svalue.c_str());
	//char delims[] = "\n !@#$%^&*)(_+-=][}{|:;'<>?,./\"\\";
	char delims[] = "{,}";
	pch = strtok(&svalue[0], delims);
	
	while (pch != NULL)
	{
		printf("%s\n", pch);
		std::string value = std::string(pch);
		value.erase(std::remove(value.begin(), value.end(), '\''), value.end());


		tempListValues.push_back(value);
		pch = strtok(NULL, delims);
	}
	
	if (tempListValues.size() != 5)//number of settings in the struct
	{
		printf("Warning! Mismatch in number of settings retrived from extstate");
		return;
	}
	
	EdlCompSettings.CreateEDLFileRegion = std::stoi(tempListValues[0]);
	EdlCompSettings.CopyExistingRegions = std::stoi(tempListValues[1]);
	EdlCompSettings.CreateRegionsForChangedShots = std::stoi(tempListValues[2]);
	EdlCompSettings.timeLineOffset = tempListValues[3];
	EdlCompSettings.framerate = std::stof(tempListValues[4]);
}












