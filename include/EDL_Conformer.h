#pragma once
#include <string>
#include <vector>
#include <map>
#include <regex>

struct EDLSettingsStruct
{
	bool CreateEDLRegions = false;
	bool CopyExistingRegions = true;
	bool CreateRegionsForChangedShots = true;
	std::string timeLineOffset = "01:00:00:00";
	float framerate = 30;
};


struct ShotTCInfo
{
	std::string sourceStartTC;
	std::string sourceEndTC;
	std::string destStartTC;
	std::string destEndTC;
	std::string shotName;
	bool empty = true;
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
	
	std::vector<ShotTCInfo> unchangedSections;
	std::vector<ShotTCInfo> changedSections;
	
	std::string filepath_Old_EDL;
	std::string filepath_New_EDL;
	
	EDLSettingsStruct Settings;
	
	EDLconformer()
	{
		Init();
	}
	
	~EDLconformer()
	{
	}
	
	void Init();
	
	bool SetupConform();
	
	bool DoConform();
	
	void ResetConform();
	
	float GetNewEndTime();
	
	std::string SecondsToTimecodeString(float inSeconds);
	
	float TimecodeToSeconds(std::string inTimecode);
	
	int TimecodeToFrames(std::string inTimecode);
	
	std::string FramesToTimecodeString(int inFrames);
	
	float FramesToSeconds(int inFrames);
	
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
	
	std::vector<ShotTCInfo> old_shotTimeInfo;
	std::vector<ShotTCInfo> new_shotTimeInfo;
	
	std::vector<ConformResultsInfo> conformResults;
	
	
	std::string originalEndTime = "";
	int numberOfShots = 0;
	bool working = false;
	
	std::vector<std::string> FindTimecodeValuesInString(std::string inString);
	
	float TruncateFloat(float inFloat,int decimalPlaces, std::string& outInts, std::string& outDecs);
	
	bool TimeIsEqual(float num1,float num2,int decimalPlaces=4);
	
	
	
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
	
	bool IngestEDLFiles();
	
	void RefreshTimeline();
	
	void PrepareChangedSections();
	
	void PrepareUnchangedSections();
	
	void FinishWork();
	
	void CropProject();
	
	bool isConformReady();
	
};
