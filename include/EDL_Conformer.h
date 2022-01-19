#pragma once
#include <string>
#include <vector>
#include <map>
#include <regex>



struct ShotTCInfo
{
	std::string sourceStartTC;
	std::string sourceEndTC;
	std::string destStartTC;
	std::string destEndTC;
	std::string shotName;
	bool empty = true;
};

struct ShotTimeInfoSeconds
{
	float sourceStart;
	float sourceEnd;
	float destStart;
	float destEnd;
	std::string shotName;
};

struct ConformResultsInfo
{
	ShotTCInfo oldShot;
	ShotTCInfo newShot;
	bool changed = false;
};

class EDLconformer
{
public:
	
	bool CreateEDLRegions = false;
	bool CopyExistingRegions = true;
	bool CreateRegionsForShots = true;
	bool CreateRegionsForShotsOnlyChanged = true;
	
	float framerate = 30;
	
	EDLconformer()
	{
		Main();
	}
	
	~EDLconformer()
	{
	
	}
	
	void Main();
	
private:
	
	//command IDs
	int cmd_SetSelectionStart=0;
	int cmd_SetSelectionEnd=0;
	int cmd_InsertEmptySpaceAtSelection=0;
	int cmd_CropProjectToSelection=0;
	int cmd_SplitAndSelectItemsAtTimeSelection=0;
	int cmd_SelectAllTracks=0;
	int cmd_DuplicateItemsInSelectionToEditCursor=0;
	int cmd_UnSelectAllTracks=0;
	int cmd_UnselectAllItems=0;
	int cmd_CloseAllRunningScripts=0;
	int cmd_CopySelectedAreaOfItems=0;
	int cmd_SetFirstSelectedTrackAsLastTouched=0;
	int cmd_PasteItemsTracks=0;
	int cmd_CropProjectToTimeSelection=0;
	int cmd_CopyRegionsInTimeSelection=0;
	int cmd_PasteRegionsToEditCursor=0;
	
	std::vector<std::string> old_fileLines;
	std::vector<ShotTCInfo> old_shotTimeInfo;
	std::vector<std::string> new_fileLines;
	std::vector<ShotTCInfo> new_shotTimeInfo;
	std::string new_EDL_Filename;
	std::vector<ShotTimeInfoSeconds> unchangedSections;
	std::vector<ConformResultsInfo> conformResults;
	
	std::string timeLineOffset = "01:00:00:00";
	std::string originalEndTime = "";
	int numberOfShots = 0;
	bool working = false;
	
	std::vector<std::string> FindTimecodeValuesInString(std::string inString);
	
	float TruncateFloat(float inFloat,int decimalPlaces, std::string& outInts, std::string& outDecs);
	
	bool TimeIsEqual(float num1,float num2,int decimalPlaces=4);
	
	std::string SecondsToTimecodeString(float inSeconds);
	
	float TimecodeToSeconds(std::string inTimecode);
	
	int TimecodeToFrames(std::string inTimecode);
	
	std::string FramesToTimecodeString(int inFrames);
	
	float FramesToSeconds(int inFrames);
	
	bool GatherAndCheckCommandIDs();
	
	void SetEditCursorToTimecode(std::string inTimecode);
	
	std::vector<std::string> ReadFile(std::string inFilepath);
	
	std::vector<ShotTCInfo> CreateShotTimeInfo(std::vector<std::string> inFilelines);
	
	void SetTimeSelection(std::string startTimecode,std::string endTimecode);
	
	void ShiftExistingTimeline();
	
	void AddRegion(std::string timeCodeStart,std::string timeCodeEnd,std::string name,int colour = 0);
	
	void CopyPreviousRegions();
	
	void MovePreviousRegions();
	
	void CopyOldSliceToNewTime(std::string oldStart,std::string oldEnd,std::string newStart,std::string NewEnd,std::string ShotName);
	
	void CropProjectToTime(std::string newProjEndTime);
	
	void IngestEDLFiles();
	
	void RefreshTimeline();
	
	void ExecuteConform();
	
	void handleUnchangedSections();
	
	void FinishWork();
	
	void CropProject();
	
	
	
};
