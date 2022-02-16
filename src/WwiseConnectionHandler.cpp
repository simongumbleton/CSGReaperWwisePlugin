
#include <iostream> 
#include <sstream>
#include <vector>
#include <mutex>

#include "WwiseConnectionHandler.h"

#include "reaperHelpers.h"
#include <AK/WwiseAuthoringAPI/waapi.h>
//#include "RapidJsonUtils.h"
//#include "rapidjson/document.h"


#include "ReaperRenderQueParser.h"
#include "platformhelpers.h"


//static PluginWindow myPluginWindow = PluginWindow();
//static CreateImportWindow myCreateImportWindow = CreateImportWindow();
std::mutex mx_w;
std::vector<BaseWwiseGuiComponent*> WwiseConnectionHandler::ActiveComponents;

WwiseConnectionHandler::WwiseConnectionHandler()
{
	
}

WwiseConnectionHandler::~WwiseConnectionHandler()
{
}

bool WwiseConnectionHandler::SaveWwiseProject()
{
	return waapi_SaveWwiseProject();
}

bool WwiseConnectionHandler::CheckForProjectFileChanges()
{
	return waapi_CheckForProjectFileChanges();
}

void WwiseConnectionHandler::ReportConnectionError(CurrentWwiseConnection attemptedConnection)
{
	std::string errorLog = "Wwise Connection not found on port " + std::to_string(attemptedConnection.port) + "...Exiting!";
	PrintToConsole(errorLog);
}



bool WwiseConnectionHandler::handle_GUI_Connect()
{
	return ConnectToWwise(false, MyCurrentWwiseConnection.port);
}

bool WwiseConnectionHandler::ConnectToWwise(bool suppressOuputMessages, int port)
{

	if (waapi_Connect(MyCurrentWwiseConnection))  //Connect to Wwise. Optionally pass bool true to supress message boxes from wwise connection
	{
		if (!suppressOuputMessages)
		{
			//create a status text string and set it
			std::stringstream status;
			status << "Connected on port " + std::to_string(port) + ": ";
			status << " - " + MyCurrentWwiseConnection.Version;
			std::string WwiseConnectionStatus = status.str();
			//MessageBox(NULL, WwiseConnectionStatus.c_str(), "Wwise Connection Status", MB_OK);
			//PrintToConsole(WwiseConnectionStatus);
		}

		GetWwiseProjectGlobals(false, MyCurrentWwiseConnection.projectGlobals);

		return true;
	}
	else
	{
		ReportConnectionError(MyCurrentWwiseConnection);
		return false;
	}
}

void WwiseConnectionHandler::DisconnectFromWwise()
{
	waapi_Disconnect(MyCurrentWwiseConnection);
}

WwiseObject WwiseConnectionHandler::GetSelectedObject()
{
	WwiseObject mySelectedObject;
	if (!MyCurrentWwiseConnection.connected)
	{
		/// WWise connection not found!
		ReportConnectionError(MyCurrentWwiseConnection);
		return mySelectedObject;
	}
	using namespace AK::WwiseAuthoringAPI;
	AkJson RawReturnResults;

	if (!waapi_GetSelectedWwiseObjects(RawReturnResults, true))
	{
		//Something went wrong!
		PrintToConsole("ERROR. Get Selected Object Call Failed. Exiting.");
		return mySelectedObject;
	}

	AkJson::Array MyReturnResults;
	waapi_GetWaapiResultsArray(MyReturnResults, RawReturnResults);

	
	for (const auto &result : MyReturnResults)
	{
		//WwiseObject obj;
		mySelectedObject.isEmpty = false;
		mySelectedObject.properties.insert(std::make_pair("id", result["id"].GetVariant().GetString()));
		mySelectedObject.properties.insert(std::make_pair("name", result["name"].GetVariant().GetString()));
		mySelectedObject.properties.insert(std::make_pair("type", result["type"].GetVariant().GetString()));
		mySelectedObject.properties.insert(std::make_pair("path", result["path"].GetVariant().GetString()));
		mySelectedObject.properties.insert(std::make_pair("workunit", result["workunit"]["id"].GetVariant().GetString()));
	}

	return mySelectedObject;
}


std::vector<WwiseObject> WwiseConnectionHandler::GetWwiseObjects(bool suppressOuputMessages, ObjectGetArgs& getargs, AK::WwiseAuthoringAPI::AkJson::Array& Results)
{
	if (!MyCurrentWwiseConnection.connected)
	{
		/// WWise connection not found!
		ReportConnectionError(MyCurrentWwiseConnection);
		return std::vector<WwiseObject>();
		//throw std::string("Wwise connection not found!");
	}
	
	using namespace AK::WwiseAuthoringAPI;

	if (getargs.fromSelected)	/// Need to get the selected object first
	{
		WwiseObject selectedObject = GetSelectedObject();
		getargs.From[0] = "id";
		getargs.From[1] = selectedObject.properties["id"];
	}

	AkJson MoreRawReturnResults;
	if (!waapi_GetObjectFromArgs(getargs, MoreRawReturnResults))
	{
		//Something went wrong!
		std::string message = MoreRawReturnResults["message"].GetVariant().GetString().c_str();
		PrintToConsole(message);
		PrintToConsole("ERROR. Get Object Call Failed. Exiting.");
		return std::vector<WwiseObject>();
		//throw std::string("ERROR. Get Object Call Failed!");
	}
	waapi_GetWaapiResultsArray(Results, MoreRawReturnResults);
	std::vector<WwiseObject> WwiseObjects;
	for (const auto &result : Results)
	{
		WwiseObject Result = ResultToWwiseObject(result);
		WwiseObjects.push_back(Result);
	}

	LinkParentChildObjects(WwiseObjects);

	return WwiseObjects;
}

bool WwiseConnectionHandler::CreateWwiseObject(bool suppressOutputMessages, CreateObjectArgs & createArgs, AK::WwiseAuthoringAPI::AkJson::Array & Results,bool undoAndSave)
{
	if (!MyCurrentWwiseConnection.connected)
	{
		/// WWise connection not found!
		ReportConnectionError(MyCurrentWwiseConnection);
		return false;
	}
	using namespace AK::WwiseAuthoringAPI;

	//Sort the inputs  "ActorMixer","Blend", "Random", "Sequence", "Switch"			RandomOrSequence 0 or 1
	if (createArgs.Type == "Blend")
	{
		createArgs.Type = "BlendContainer";
	}
	else if (createArgs.Type == "Switch")
	{
		createArgs.Type = "SwitchContainer";
	}
	else if (createArgs.Type == "Random")
	{
		createArgs.Type = "RandomSequenceContainer";
		createArgs.RandomOrSequence = 1;
	}
	else if (createArgs.Type == "Sequence")
	{
		createArgs.Type = "RandomSequenceContainer";
		createArgs.RandomOrSequence = 0;
	}
	else if (createArgs.Type == "Event")
	{
		//Check the type for the event target
	}
	else if (createArgs.Type == "MusicTrack")
	{
		if (stringToLower(createArgs.ParentType).compare("musicsegment") != 0)
		{
			//Something went wrong!
			PrintToConsole("ERROR. Parent of MusicTrack must be MusicSegment!");
			waapi_UndoHandler(Cancel, "Create Object");
			return false;
		}
	}

	
	AkJson MoreRawReturnResults;
	if (!waapi_CreateObjectFromArgs(createArgs, MoreRawReturnResults))
	{
		//Something went wrong!
		//waapi_GetWaapiResultsArray(Results, MoreRawReturnResults);
		PrintToConsole("ERROR. Create Object Call Failed. Check if the selected parent is correct.");
		return false;
	}
	waapi_GetWaapiResultsArray(Results, MoreRawReturnResults);

	return true;
}

bool WwiseConnectionHandler::ImportAudioToWwise(bool suppressOutputMessages, ImportObjectArgs & importArgs, AK::WwiseAuthoringAPI::AkJson::Array & Results)
{
	if (!MyCurrentWwiseConnection.connected)
	{
		/// WWise connection not found!
		ReportConnectionError(MyCurrentWwiseConnection);
		return false;
	}
	
	bool autoAddToSourceControl = false;

	if (MyCurrentWwiseConnection.year > 2017)
	{
		autoAddToSourceControl = true;
	}

	using namespace AK::WwiseAuthoringAPI;

	if (MyCurrentWwiseConnection.useAutomationMode)
	{
		if (!waapi_SetAutomationMode(true))
		{
			//PrintToConsole("Failed to set automation mode. Not supported in WAAPI 2017 or earlier");
		}
	}
	
	if (!importArgs.templateObject.isEmpty)
	{
		//put a template object in the right place
		for (auto importFile : importArgs.ImportFileList)
		{
			std::string audiofile = importFile.first.substr(importFile.first.find_last_of("/\\")+1);
			std::string rawAudioFile = audiofile.substr(0, audiofile.find_last_of("."));
			SetupTemplateObject(importArgs.templateObject.properties["id"], importArgs.ImportParentID, rawAudioFile);
		}
	}

	waapi_UndoHandler(Begin, "Auto Import");
	AkJson MoreRawReturnResults;

	if (!wappi_ImportFromArgs(importArgs, MoreRawReturnResults))
	{
		//Something went wrong!
		
		//rapidjson::Document RJresults;
		std::map<std::string,std::string> errorRes;
		std::map<std::string, double> errorResNum;
		waapi_TranslateJSONResults(errorRes,errorResNum,MoreRawReturnResults,"details");
		std::string message;
		std::string uri;
		for (auto res : errorRes)
		{
			if (res.first == "message")
			{
				message = res.second;
				continue;
			}else if (res.first == "uri")
			{
				uri = res.second;
				continue;
			}
		}
		
		//std::string akJsonResults = RapidJsonUtils::GetAkJsonString(MoreRawReturnResults);
		//RJresults.Parse(stringRes.c_str());
		
		PrintToConsole("ERROR. Import Failed. Exiting. "+message+" : "+uri);
		waapi_UndoHandler(Cancel, "Auto Import");
		return false;
	}
	waapi_GetWaapiResultsArray(Results, MoreRawReturnResults);


	//std::map<std::string, std::string> stringRes;
	//std::map<std::string, double> numRes;

	//waapi_TranslateJSONResults(stringRes, numRes, MoreRawReturnResults, "");


	
	waapi_SaveWwiseProject();

	if (!waapi_SetAutomationMode(false))
	{
		//PrintToConsole("Failed to set automation mode. Not supported in WAAPI 2017 or earlier");
	}

	return true;

}

bool WwiseConnectionHandler::GetWwiseProjectGlobals(bool suppressOutputMessages, WwiseProjectGlobals & WwiseProjGlobals)
{
	if (!MyCurrentWwiseConnection.connected)
	{
		/// WWise connection not found!
		ReportConnectionError(MyCurrentWwiseConnection);
		return false;
	}
	using namespace AK::WwiseAuthoringAPI;

	ObjectGetArgs Project;
	Project.From = { std::string("ofType"),std::string("Project") };
	Project.customReturnArgs = { std::string("@DefaultLanguage"),std::string("filePath") };
	Project.Select = "";

	AkJson MoreRawReturnResults;
	if (!waapi_GetObjectFromArgs(Project, MoreRawReturnResults))
	{
		//Something went wrong!
		PrintToConsole("ERROR. Get Object Call Failed. Exiting.");
		return false;
	}
	AkJson::Array Results;
	waapi_GetWaapiResultsArray(Results, MoreRawReturnResults);

	if (Results.size() != 0)
	{
		WwiseProjGlobals.DefaultLanguage = string(Results[0]["@DefaultLanguage"].GetVariant());
		WwiseProjGlobals.ProjectName = string(Results[0]["name"].GetVariant());
		WwiseProjGlobals.ProjectPath = string(Results[0]["filePath"].GetVariant());
		
#ifndef _WIN32 //Not windows! Wwise gives crappy paths on mac e.g.
		//replace("Y:", "~").replace('\\', '/')
		juce::String tempPath = WwiseProjGlobals.ProjectPath;
		tempPath = tempPath.replace("Y:", "~").replace("\\", "/");
		WwiseProjGlobals.ProjectPath = tempPath.toStdString();
		std::size_t pos = WwiseProjGlobals.ProjectPath.rfind("/");
		std::string rootPath = WwiseProjGlobals.ProjectPath.substr (0,pos);
		WwiseProjGlobals.ProjectRootFolder = rootPath;
		WwiseProjGlobals.OriginalsPath = rootPath + "/Originals";
#else
		std::size_t pos = WwiseProjGlobals.ProjectPath.rfind("\\");
		std::string rootPath = WwiseProjGlobals.ProjectPath.substr (0,pos);
		WwiseProjGlobals.ProjectRootFolder = rootPath;
		WwiseProjGlobals.OriginalsPath = rootPath + "\\Originals";
#endif
	}

	ObjectGetArgs langs;
	langs.From = { std::string("ofType"),std::string("Language") };
	langs.Select = "";

	AkJson langRawReturnResults;
	if (!waapi_GetObjectFromArgs(langs, langRawReturnResults))
	{
		//Something went wrong!
		PrintToConsole("ERROR. Get Object Call Failed. Exiting.");
		return false;
	}
	AkJson::Array langResults;
	waapi_GetWaapiResultsArray(langResults, langRawReturnResults);

	WwiseProjGlobals.Languages.clear();
	WwiseProjGlobals.Languages.push_back(WwiseProjGlobals.DefaultLanguage);
	for (auto language : langResults)
	{
		std::string lang = language["name"].GetVariant();
		if (lang == "SFX" || lang == "External" || lang == "Mixed" || lang == WwiseProjGlobals.DefaultLanguage)
		{
			continue;
		}
		else {
			WwiseProjGlobals.Languages.push_back(language["name"].GetVariant());
		}
	}
	return true;
}

WwiseObject WwiseConnectionHandler::ResultToWwiseObject(AK::WwiseAuthoringAPI::AkJson Result)
{
	using namespace AK::WwiseAuthoringAPI;
	WwiseObject returnWwiseObject;
	
	for (const auto i : Result.GetMap()) {
		AkJson::Type type;
		std::string stringKey = i.first;
		type = i.second.GetType();

		///Type is already AK Variant
		if (type == AkJson::Type::Variant)
		{
			AkVariant variant = Result[stringKey].GetVariant();
			if (variant.IsString())
			{
				//push value into string results
				std::string key = stringKey;
				std::string value = variant.GetString();
				returnWwiseObject.properties.emplace(std::make_pair(key, value));
				returnWwiseObject.isEmpty = false;
			}
			else if (variant.IsNumber())
			{
				//push value into number results
				std::string key = stringKey;
				double value = variant.operator double();
				returnWwiseObject.numericProperties.emplace(std::make_pair(key, value));
				returnWwiseObject.isEmpty = false;
			}
			else if (variant.GetType() == 11)//Type is bool
			{
				std::string key = stringKey;
				bool b_value = variant.GetBoolean();
				std::string value = std::to_string(b_value);
				returnWwiseObject.properties.emplace(std::make_pair(key, value));
				returnWwiseObject.isEmpty = false;
			}
		}
		else if (type == AK::WwiseAuthoringAPI::AkJson::Type::Map)
		{
			for (const auto x : Result[stringKey].GetMap())
			{
				std::string first = x.first;
				AkVariant variant = x.second.GetVariant();
				if (variant.IsString())
				{
					//push value into string results
					std::string key = first;
					std::string value = variant.GetString();
					returnWwiseObject.properties.emplace(std::make_pair(stringKey+"_"+key, value));
					returnWwiseObject.isEmpty = false;
				}
				else if (variant.IsNumber())
				{
					//push value into number results
					std::string key = first;
					double value = variant.operator double();
					returnWwiseObject.numericProperties.emplace(std::make_pair(stringKey + "_" + key, value));
					returnWwiseObject.isEmpty = false;
				}
				else if (variant.GetType() == 11)//Type is bool
				{
					std::string key = first;
					bool b_value = variant.GetBoolean();
					std::string value = std::to_string(b_value);
					returnWwiseObject.properties.emplace(std::make_pair(stringKey + "_" + key, value));
					returnWwiseObject.isEmpty = false;
				}
			}
		}
		else if (type == AK::WwiseAuthoringAPI::AkJson::Type::Array)
		{
			auto x = Result[stringKey].GetArray()[0];
			{
				returnWwiseObject = ResultToWwiseObject(x);
				return returnWwiseObject;
			}
		}
		else
		{
			//"Ak retunr Type not found";
		}
	}
	return returnWwiseObject;
}

bool WwiseConnectionHandler::LinkParentChildObjects(std::vector<WwiseObject>& objects)
{
	//get parent links
	for (auto &childobj : objects) {
		std::string parentID;
		//PrintToConsole(obj.properties.at("name"));
		if (childobj.properties.find("parent_id") != childobj.properties.end())
		{
			parentID = childobj.properties.at("parent_id");
		}
		else continue;
		
		for (auto &possibleparent : objects) {
			if (parentID == possibleparent.properties.at("id")) {
				childobj.parentObject = &possibleparent;
				possibleparent.childObjects.push_back(&childobj);
				break;
			}

		}
	}
	return true;
}

void WwiseConnectionHandler::SetOptionsFromConfig(config myConfig)
{
	MyCurrentWwiseConnection.port = myConfig.waapiPort;
	MyCurrentWwiseConnection.useAutomationMode = myConfig.useAutomationMode;
}

void WwiseConnectionHandler::SetWwiseAutomationMode(bool enable)
{
	waapi_SetAutomationMode(enable);
}

bool WwiseConnectionHandler::SetNotesForObject(std::string id, std::string notes,AK::WwiseAuthoringAPI::AkJson & results)
{
	return waapi_SetNotesForObject(id, notes, results);
}

bool WwiseConnectionHandler::SubscribeOnSelectionChanged
 (AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback,
	uint64_t &outsubscriptionID)
{
	return waapi_SetupSubscription(ak::wwise::ui::selectionChanged, in_callback, outsubscriptionID);
}

bool WwiseConnectionHandler::SubscribeOnProjectClosed
 (AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback,
	uint64_t &outsubscriptionID)
{
	return waapi_SetupSubscription(ak::wwise::core::project::preClosed, in_callback, outsubscriptionID);
}

WwiseObject WwiseConnectionHandler::CreateStructureFromPath(std::string path, std::string parent)
{
	if ((parent == "") or (path == ""))
	{
		//print("Error. Missing arguments")
		return WwiseObject();
	}
	bool isParentID = false;
	std::string nextParentID;
	//isParentID = isStringValidID(parent) // need to implement this regex check
	
	if (!isParentID)
	{
	//the parent param was not an ID, lets try to find it in the wwise project
		if (stringToLower(parent) == "actor-mixer hierarchy"){
			parent = "\\"+parent;}
		else if (stringToLower(parent) == "events"){
			parent = "\\"+parent;}

		ObjectGetArgs getArgs;
		getArgs.From = { "path",parent };
		getArgs.Select = "";
		getArgs.customReturnArgs.push_back("path");

		AK::WwiseAuthoringAPI::AkJson::Array results;
		std::vector<WwiseObject> MyWwiseObjects;
		try {
			MyWwiseObjects = GetWwiseObjects(false, getArgs, results);
		}
		catch (std::string e) {
			PrintToConsole(e);
		}
		if (MyWwiseObjects.size()==1)
		{
			nextParentID = MyWwiseObjects[0].properties["id"];
		}
		else if (MyWwiseObjects.size()>1)
		{
			PrintToConsole("Ambiguous parent argument. More than one possible parent found using arg: "+parent);
			PrintToConsole("Consider refining the argument or passing an explicit ID instead");
			return WwiseObject();
		}
		else if(MyWwiseObjects.empty())
		{
			PrintToConsole("Could not locate parent in wwise project. Attempting to create. Arg given = "+parent);
			//if "Actor-Mixer Hierarchy" in parent:
			size_t foundPos = stringToLower(parent).find("actor-mixer hierarchy");
			if (foundPos != parent.npos)
			{
				std::string p = parent.substr(foundPos+21,parent.npos);
				//p = parent.partition("Actor-Mixer Hierarchy")[2]
				WwiseObject res = CreateStructureFromPath(p,"\\Actor-Mixer Hierarchy");
				if (res.isEmpty)
				{
					PrintToConsole("Error creating parent object");
					return WwiseObject();
				}
				nextParentID = res.properties["id"];
			}
			else
			{//elif "Events" in parent:
				size_t foundPos = stringToLower(parent).find("events");
				if (foundPos != parent.npos)
				{
					std::string p = parent.substr(foundPos+6,parent.npos);
					//p = parent.partition("Actor-Mixer Hierarchy")[2]
					WwiseObject res = CreateStructureFromPath(p,"\\Events");
					if (res.isEmpty)
					{
						PrintToConsole("Error creating parent object");
						return WwiseObject();
					}
					nextParentID = res.properties["id"];
				}
			}
		}
	}
	else
	{
		ObjectGetArgs getArgs;
		getArgs.From = { "id",parent };
		getArgs.Select = "";
		getArgs.customReturnArgs.push_back("path");

		AK::WwiseAuthoringAPI::AkJson::Array results;
		std::vector<WwiseObject> MyWwiseObjects;
		try {
			MyWwiseObjects = GetWwiseObjects(false, getArgs, results);
		}
		catch (std::string e) {
			PrintToConsole(e);
		}
		if (!MyWwiseObjects.empty())
		{
			nextParentID = parent;
		}
		else{
			PrintToConsole("Error. Cannot find an object with matching ID from parent argument");
			return WwiseObject();
		}
	}
	
	
	WwiseObject lastChild;
	std::vector<std::string> pathlist = stringSplitToList(path, "\\");
	for (auto node : pathlist)
	{
		if (node == "") {continue;}
		std::string type = "";
		std::string name = "";
		size_t found = node.find("<");
		if (found != node.npos)
		{
			//if "<" in node:
			size_t found = node.find(">");
			if (found != node.npos)
			{
				type = node.substr(0,found);
				type = stringReplace(type, "<", "");
				name = node.substr(found+1,node.npos);
			}
		}
		else
		{
			type = "";
			name = node;
		}
		// check if there is already a child with the name under the parent
		ObjectGetArgs getArgs;
		getArgs.From = { "id",nextParentID };
		getArgs.Select = "children";
		getArgs.customReturnArgs.push_back("path");

		AK::WwiseAuthoringAPI::AkJson::Array results;
		std::vector<WwiseObject> MyWwiseObjects;
		try {
			MyWwiseObjects = GetWwiseObjects(false, getArgs, results);
		}
		catch (std::string e) {
			PrintToConsole(e);
		}
		bool foundMatch = false;
		if (!MyWwiseObjects.empty())
		{
			for (auto item : MyWwiseObjects)
			{
				if (stringToLower(item.properties["name"]) == stringToLower(name))
				{
					//node already exists in wwise
					foundMatch = true;
					nextParentID = item.properties["id"];
					lastChild = item;
					break;
				}
			}
		}
		if (!foundMatch)
		{
			if (!type.empty())
			{
			//node contains a type, and we didn't find an existing item so we try to create it
				CreateObjectArgs createArgs;
				createArgs.ParentID = nextParentID;
				createArgs.Type = type;
				createArgs.Name = name;
				
				AK::WwiseAuthoringAPI::AkJson::Array results;
				if (CreateWwiseObject(false, createArgs, results))
				{
					WwiseObject res = ResultToWwiseObject(results[0]);
					if (!res.isEmpty)
					{
						nextParentID = res.properties["id"];
						lastChild = res;
					}
				}
				else
				{
					PrintToConsole("Error! Could not create object and found no existing object named "+name+" underneath " + parent);
					return WwiseObject();
				}
			}
			else
			{
				PrintToConsole("Error! Could not create object and found no existing object named "+name+" underneath " + parent);
				return WwiseObject();
			}
			
		}
	}

	if (!lastChild.isEmpty)
	{
		return lastChild;
	}
	else{
		return WwiseObject();
	}
}

bool WwiseConnectionHandler::SetupTemplateObject(std::string templateObjID, std::string destinationParentID, std::string newName) { 
	
	using namespace AK::WwiseAuthoringAPI;
	ObjectGetArgs getChildrenArgs;
	getChildrenArgs.From = {"id",destinationParentID};
	getChildrenArgs.Select = "children";
	AK::WwiseAuthoringAPI::AkJson::Array results;
	std::vector<WwiseObject> MyWwiseObjects;
	try {
		MyWwiseObjects = GetWwiseObjects(false, getChildrenArgs, results);
	}
	catch (std::string e) {
		PrintToConsole(e);
		return false;
	}
	for (auto obj : MyWwiseObjects)
	{ //check if a named object already exists under the parent. We only want to create a template if we are adding a new audio file
		if (stringToLower(obj.properties["name"]) == stringToLower(newName))
		{
			return false;
		}
	}
	
	waapi_CopyPasteWwiseObject(templateObjID, destinationParentID, newName);
	
	return true;
}







ReaperRenderObj::ReaperRenderObj()
{
}

ReaperRenderObj::~ReaperRenderObj()
{
}

void ReaperRenderObj::RenderAllQues()
{
	Reaper_RenderAllQuedJobs();
}
