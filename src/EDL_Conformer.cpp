//
//  EDL_Conformer.cpp
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 10/01/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#include "EDL_Conformer.h"




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
	return tcinfo;
}


std::vector<std::string> EDLconformer::ReadFile(std::string inFilepath) { 
	std::vector<std::string> fileLines;
	return fileLines;
}


void EDLconformer::SetEditCursorToTimecode(std::string inTimecode) { 
	
}


void EDLconformer::GatherAndCheckCommandIDs() { 
	
}


std::string EDLconformer::SecondsToTimecodeString(float inSeconds) { 
	std::string result;
	return result;
}


bool EDLconformer::TimeIsEqual(float num1, float num2, int decimalPlaces) { 
	return true;
}


float EDLconformer::TruncateFloat(float inFloat, int decimalPlaces) { 
	float result = 0.0f;
	return result;
}
