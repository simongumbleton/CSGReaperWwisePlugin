
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


//static PluginWindow myPluginWindow = PluginWindow();
//static CreateImportWindow myCreateImportWindow = CreateImportWindow();
std::mutex mx_w;

WwiseConnectionHandler::WwiseConnectionHandler()
{
	
}

WwiseConnectionHandler::~WwiseConnectionHandler()
{
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
		if (MyCurrentWwiseConnection.useAutomationMode)
		{
			if (!waapi_SetAutomationMode(true))
			{
				//PrintToConsole("Failed to set automation mode. Not supported in WAAPI 2017 or earlier");
			}
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
		WwiseObject obj;
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
		throw std::string("Wwise connection not found!");
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
		PrintToConsole("ERROR. Get Object Call Failed. Exiting.");
		throw std::string("ERROR. Get Object Call Failed!");
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

bool WwiseConnectionHandler::CreateWwiseObjects(bool suppressOutputMessages, CreateObjectArgs & createArgs, AK::WwiseAuthoringAPI::AkJson::Array & Results)
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

	waapi_UndoHandler(Begin, "Create Object");

	AkJson MoreRawReturnResults;
	if (!waapi_CreateObjectFromArgs(createArgs, MoreRawReturnResults))
	{
		//Something went wrong!
		PrintToConsole("ERROR. Create Object Call Failed. Exiting.");
		waapi_UndoHandler(Cancel, "Create Object");
		return false;
	}
	waapi_GetWaapiResultsArray(Results, MoreRawReturnResults);

	waapi_UndoHandler(End, "Create Object");
	waapi_SaveWwiseProject();	
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


	waapi_UndoHandler(End, "Auto Import");
	waapi_SaveWwiseProject();
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
		WwiseProjGlobals.ProjectPath = string(Results[0]["filePath"].GetVariant());
		WwiseProjGlobals.ProjectName = string(Results[0]["name"].GetVariant());
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
			}
			else if (variant.IsNumber())
			{
				//push value into number results
				std::string key = stringKey;
				double value = variant.operator double();
				returnWwiseObject.numericProperties.emplace(std::make_pair(key, value));
			}
			else if (variant.GetType() == 11)//Type is bool
			{
				std::string key = stringKey;
				bool b_value = variant.GetBoolean();
				std::string value = std::to_string(b_value);
				returnWwiseObject.properties.emplace(std::make_pair(key, value));
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
				}
				else if (variant.IsNumber())
				{
					//push value into number results
					std::string key = first;
					double value = variant.operator double();
					returnWwiseObject.numericProperties.emplace(std::make_pair(stringKey + "_" + key, value));
				}
				else if (variant.GetType() == 11)//Type is bool
				{
					std::string key = first;
					bool b_value = variant.GetBoolean();
					std::string value = std::to_string(b_value);
					returnWwiseObject.properties.emplace(std::make_pair(stringKey + "_" + key, value));
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
	uint64_t subscriptionID)
{
	return waapi_SetupSubscription(ak::wwise::ui::selectionChanged, in_callback, subscriptionID);
}

bool WwiseConnectionHandler::SubscribeOnProjectClosed
 (AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback,
	uint64_t subscriptionID)
{
	return waapi_SetupSubscription(ak::wwise::core::project::preClosed, in_callback, subscriptionID);
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
