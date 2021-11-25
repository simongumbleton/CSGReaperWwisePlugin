#pragma once

#include "stdlib.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "waapi_structs.h"


#include <AK/WwiseAuthoringAPI/AkAutobahn/AkJson.h>
#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>




///////////////////////////////
////     Connect to wwise client
///////////////////////////////
bool waapi_Connect(CurrentWwiseConnection &wwiseConnectionReturn);

void waapi_DisconnectHandler();

void waapi_Disconnect(CurrentWwiseConnection &wwiseConnectionReturn);

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

bool waapi_SetNotesForObject(std::string id, std::string notes,AK::WwiseAuthoringAPI::AkJson & results);

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


bool waapi_SetupSubscription(const char *subscription,AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback,uint64_t &subscriptionID);

bool waapi_Unsubscribe(const uint64_t& in_subscriptionId); //bool Unsubscribe(const uint64_t& in_subscriptionId, AkJson& out_result, int in_timeoutMs = -1);

bool waapi_CheckForProjectFileChanges();

bool waapi_CopyPasteWwiseObject(std::string sourceID, std::string destinationParentID, std::string newName);

WwiseObject waapi_ResultToWiseObject(AK::WwiseAuthoringAPI::AkJson Result);

bool waapi_RenameObject(std::string objectID,std::string newName);

bool waapi_MoveWwiseObject(std::string sourceID, std::string destinationParentID);
