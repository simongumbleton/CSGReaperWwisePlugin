//
//  waapi_structs.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 18/07/2020.
//  Copyright © 2020 My Company. All rights reserved.
//

#pragma once
#include <string>
#include <map>
#include <vector>

typedef struct WwiseObject WwiseObject;

enum IMPORT_TYPE
{
	Sfx, Voice, Music
};
enum SourceControlOperation
{
	CheckoutWWU, RevertWWU, UpdateWWU, CommitWWU
};



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
	IMPORT_TYPE EimportType;
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
	IMPORT_TYPE EimportType;
	

};

struct WwiseProjectGlobals {
	std::string ProjectPath;
	std::string ProjectName;
	std::vector<std::string> Languages;
	std::string DefaultLanguage;
	std::string OriginalsPath;
	std::string ProjectRootFolder;
};

///struct to hold the current Waapi connection info (version, port etc)
struct CurrentWwiseConnection {
	int port = 8080;
	std::string Version;
	int timeoutMS = 2000;
	bool connected = false;
	bool supressDebugOutput = false;
	int year = 0;
	WwiseProjectGlobals projectGlobals;
	bool useAutomationMode = false;
};

struct ObjectGetArgs {	/// Init struct with some sensible defaults that can be error checked
	std::vector<std::string> From{ "","" };
	std::string Select = "descendants";
	std::vector<std::string> Where{ "","" };
	std::vector<std::string> customReturnArgs{};
	bool fromSelected = false;
};

struct EventCreateArgs {
	int action = 1; // Play
	std::string target = "";
};

struct CreateObjectArgs {
	std::string ParentID = "";
	std::string Type = "";
	std::string Name = "";
	std::string onNameConflict = "merge";
	std::string Notes = "";
	int RandomOrSequence = 0;
	std::string language = "SFX";
	bool createPlayEvent = false;
	EventCreateArgs eventArgs;
	std::string Workunit = "";
	std::string ParentType = "";
};

struct ImportObjectArgs {
	std::string ImportLanguage = "";
	std::string ImportLocation = "";
	std::string OriginalsSubFolder = "";
	//bool CreatePlayEvent = false;
	std::vector<std::pair<std::string, std::string>> ImportFileList;
	std::string importOperation = "useExisting";
	std::string objectType;
	int eventCreateOption = 0; // 0 = no event, 1 = children, 2 = parent
	std::string Workunit = "";
	std::string Notes = "";
	std::string SourceReaperProject = "";

};



enum undoStep { Begin = 0, End = 1, Cancel = 2 };

/// Vectors to store GET choices
struct GetObjectChoices {
	std::vector<std::string> waapiGETchoices_FROM
	{
		"Wwise Selection","id", "search", "path", "ofType"
	};

	std::vector<std::string> waapiGETchoices_SELECT
	{
		"","descendants", "parent", "children", "ancestors"
	};

	std::vector<std::string> waapiGETchoices_WHERE
	{
		"","name:contains", "name:matches", "type:isIn", "category:isIn"
	};

	std::vector<std::string> waapiGETchoices_RETURN
	{
		"id", "name", "type", "path", "parent", "notes",
		"shortId","classId", "category", "filePath",
		"owner",  "isPlayable", "childrenCount",
		"sound:convertedWemFilePath", "sound:originalWavFilePath",
		"soundbank:bnkFilePath", "music:transitionRoot", "music:playlistRoot",
		"audioSource:playbackDuration", "audioSource:maxDurationSource",
		"audioSource:trimValues", "workunit:isDefault", "workunit:type",
		"workunit:isDirty", "switchContainerChild:context","@DefaultLanguage"
	};
};

struct CreateObjectChoices {
	std::vector<std::string> waapiCREATEchoices_TYPE
	{
		"ActorMixer","Blend", "Random", "Sequence",
		"Switch","Sound","Voice",
		"MusicPlaylistContainer", "MusicSwitchContainer", "MusicSegment", "MusicTrack"
	};
	std::vector<std::string> waapiCREATEchoices_NAMECONFLICT
	{
		"merge","rename", "replace", "fail"
	};
	std::vector<std::string> waapiCREATEchoices_IMPORTOP
	{
		"useExisting","createNew", "replaceExisting"
	};
	std::vector<std::string> waapiCREATEchoices_EVENTOPTIONS
	{
		"None","Play@Parent", "Play@Children"
	};
	std::vector<std::string> waapiCREATEchoices_IMPORTTYPE
	{
		"Sfx","Voice", "Music"
	};

};



struct WorkgroupOperations {
	std::vector<std::string> WorkUnit
	{
		"WorkgroupCheckoutWWU",
		"WorkgroupRevertWWU",
		"WorkgroupUpdateWWU",
		"WorkgroupCommitWWU"
	};
	//TO DO - add other workgroup commands 
};

