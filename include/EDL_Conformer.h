#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <regex>
#include "settings_structs.h"

class MediaTrack;

struct ShotAnimInfo
{
	std::string animClipName;
	std::string animTCStart;
	std::string animTCEnd;
	std::string actor;
};

struct ShotTCInfo
{
	std::string sourceStartTC;
	std::string sourceEndTC;
	std::string destStartTC;
	std::string destEndTC;
	std::string shotName;
	std::unordered_map<std::string, ShotAnimInfo>ShotAnimInfos;
	bool empty = true;
};

struct ConformResultsInfo
{
	ShotTCInfo oldShot;
	ShotTCInfo newShot;
	bool changed = false;
	bool animClipChanged = false;
};

struct AnimClipAssemblerInfo
{
	std::string wavFilePath = "";
	std::string actorName = "";
};


class EDLconformer
{
public:

	std::vector<ShotTCInfo> unchangedSections;
	std::vector<ShotTCInfo> changedSections;
	std::vector<ShotTCInfo> animClipChangedSections;
	
	std::string filepath_Old_EDL;
	std::string filepath_New_EDL;
	
	EDLSettingsStruct EdlCompSettings;
	
	EDLconformer()
	{
		LoadSettingsFromExtState();
		Init();
	}
	
	~EDLconformer()
	{
		SaveSettingsToExtState();
	}
	
	void Init();
	
	bool SetupConform();
	
	bool DoConform();
	
	void ResetConform();

	bool SetupDialogueAssembly();
	
	float GetNewEndTime();
	
	std::string SecondsToTimecodeString(float inSeconds);
	
	float TimecodeToSeconds(std::string inTimecode);
	
	int64_t TimecodeToFrames(std::string inTimecode);
	
	std::string FramesToTimecodeString(int inFrames);
	
	float FramesToSeconds(int inFrames);
	
	void LoadSettingsFromExtState();
	
	void SaveSettingsToExtState();
	
	void InitiateDialogueAssembly(std::string folderpath);

	void MatchAnimClipsWithWavs(std::vector<std::string>wavFiles);
	
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
	int cmd_SelectAllItemsOnSelectedTracksInTimeSelection = 0;
	int cmd_SetSelectedTrackFolderState_Parent = 0;
	int cmd_SetSelectedTrackFolderState_Normal = 0;
	int cmd_SetSelectedTrackFolderState_LastInFolder = 0;
	int cmd_InsertTrackAfterCurrentSelection = 0;
	int cmd_MakeFolderFromSelectedTracks = 0;
	int cmd_SelectChildrenOfSelectedFolderTrack = 0;
	int cmd_SetSelectedTracksToOneRandomColor = 0;
	
	std::vector<ShotTCInfo> old_shotTimeInfo;
	std::vector<ShotTCInfo> new_shotTimeInfo;
	
	std::vector<ConformResultsInfo> conformResults;
	
	std::unordered_map < std::string, AnimClipAssemblerInfo>AnimClipToWavMap;//anim clip name is the key

	std::string originalEndTime = "";
	int numberOfShots = 0;
	bool working = false;
	
	std::vector<std::string> FindTimecodeValuesInString(std::string inString);
	
	float TruncateFloat(float inFloat,int decimalPlaces, std::string& outInts, std::string& outDecs);
	
	bool TimeIsEqual(float num1,float num2,int decimalPlaces=4);
	
	bool shotHasAnimClipChanges(ShotTCInfo oldShot, ShotTCInfo newShot);
	
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

	void AssembleEditFromAnimClipTimecode(std::string sourceStart, std::string sourceEnd, std::string destStart, std::string destEnd, MediaTrack* sourceTrack, MediaTrack* destTrack);
	
	void CropProjectToTime(std::string newProjEndTime);
	
	bool IngestEDLFiles();
	
	void RefreshTimeline();
	
	void PrepareChangedSections();
	
	void PrepareUnchangedSections();
	
	void FinishWork();
	
	void CropProject();
	
	bool isConformReady();
	
	
	
};
