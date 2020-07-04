#pragma once

#include "stdlib.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


#include <AK/WwiseAuthoringAPI/AkAutobahn/AkJson.h>



struct WwiseProjectGlobals {
	std::string Project;
	std::vector<std::string> Languages;
	std::string DefaultLanguage;
};

///struct to hold the current Waapi connection info (version, port etc)
struct CurrentWwiseConnection {
	int port = 8095;
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
	std::vector<std::string> Where{"",""};
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
};

struct ImportObjectArgs {
	std::string ImportLanguage = "";
	std::string ImportLocation = "";
	std::string OriginalsSubFolder = "";
	//bool CreatePlayEvent = false;
	std::vector<std::pair<std::string,std::string>> ImportFileList;
	std::string importOperation = "useExisting";
	std::string objectType;
	int eventCreateOption = 0; // 0 = no event, 1 = children, 2 = parent
	std::string Workunit = "";

};



enum undoStep {Begin = 0,End = 1,Cancel = 2};

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
		"ActorMixer","Blend", "Random", "Sequence", "Switch","Sound","Voice"
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

enum SourceControlOperation
{
	CheckoutWWU,RevertWWU,UpdateWWU,CommitWWU
};

	///////////////////////////////
	////     Connect to wwise client
	///////////////////////////////
	bool waapi_Connect(CurrentWwiseConnection &wwiseConnectionReturn);

	// Set the automation mode
	bool waapi_SetAutomationMode(bool enable);

	///////////////////////////////
	////     Get Selected Objects
	///////////////////////////////
	///Get Seleted Objects
	bool waapi_GetSelectedWwiseObjects(AK::WwiseAuthoringAPI::AkJson &resultsOut,bool getNotes = false);

	///////////////////////////////
	////     Get Object by GUID
	///////////////////////////////

	bool waapi_GetChildrenFromGUID(const AK::WwiseAuthoringAPI::AkVariant &id,AK::WwiseAuthoringAPI::AkJson &results);

	bool waapi_GetParentFromGUID(const AK::WwiseAuthoringAPI::AkVariant &id, AK::WwiseAuthoringAPI::AkJson &results);

	bool waapi_GetObjectFromArgs(ObjectGetArgs & getArgs, AK::WwiseAuthoringAPI::AkJson & results);

	std::string GetPropertyFromGUID(const AK::WwiseAuthoringAPI::AkVariant &id, std::string property, bool usePath);

	///////////////////////////////
	////    Create and Import
	///////////////////////////////
	bool waapi_CreateObjectFromArgs(CreateObjectArgs & createArgs, AK::WwiseAuthoringAPI::AkJson & results);

	bool wappi_ImportFromArgs(ImportObjectArgs & importArgs, AK::WwiseAuthoringAPI::AkJson & results);

	///////////////////////////////
	////     Get the results array for calls to Waapi functions
	///////////////////////////////
	///get the array for a succesfull call to any of the above functions, results is 'resultsOut' from above functions
	void waapi_GetWaapiResultsArray(AK::WwiseAuthoringAPI::AkJson::Array &arrayIn, AK::WwiseAuthoringAPI::AkJson &results);

	///////////////////////////////
	////     Helper function to print JSON data in a more readable way
	///////////////////////////////
	void waapi_HELPER_Print_AkJson_Array(AK::WwiseAuthoringAPI::AkJson::Array &printResults);
	void waapi_HELPER_Print_AkJson_Map(AK::WwiseAuthoringAPI::AkJson::Map &printResults);

	///////////////////////////////
	////     Helper function to Get the Results arrays in a more useful way than AkJson
	///////////////////////////////
	bool waapi_TranslateJSONResults(std::map<std::string,std::string> &INstringResults, std::map<std::string, double> &INnumberResults, AK::WwiseAuthoringAPI::AkJson result, std::string stringKey);


	///////////////////////////////
	////     Project Save, Undo, Open etc
	///////////////////////////////
	bool waapi_SaveWwiseProject();

	bool waapi_OpenWwiseProject(std::string proj);

	bool waapi_UndoHandler(undoStep undoStep, std::string undoTag);

	///////////////////////////////
	////     Workgroup operations
	///////////////////////////////
	bool waapi_DoWorkgoupOperation(SourceControlOperation operation, std::string target);

