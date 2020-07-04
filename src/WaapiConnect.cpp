#include <windows.h>
#include <Wincodec.h>
#include <Commctrl.h>
#include "stdio.h"
//#include "stdlib.h"
//#include <string>
#include <iostream> 
#include <sstream>

#include "waapiconnect.h"
#include <AK/WwiseAuthoringAPI/AkVariantBase.h>
#include <AK/WwiseAuthoringAPI/waapi.h>



//WAAPIConnect::WAAPIConnect(HWND window, int treeId, int statusTextid, int transferWindowId)

	//: hwnd(window)
	//	, m_wwiseViewId(treeId)
	//	, m_statusTextId(statusTextid)
	//	, m_transferWindowId(transferWindowId)
	//	, m_progressWindow(0)
	//	, m_client()
WAAPIConnect::WAAPIConnect()	/// This is the constructor, gets called with every instance of WAAPIConnect
	{
		//Connect();	/// Connect doesn't need to be called in the constructor
	}


bool WAAPIConnect::Connect(bool suppressOuputMessages)	//Connect to Wwise. Optionally pass bool true to supress message boxes from wwise connection
{
	using namespace AK::WwiseAuthoringAPI;
	AkJson wwiseInfo;
	bool success = false;

	if (success = m_client.Connect("127.0.0.1", g_Waapi_Port))
	{
		//Get Wwise info
		if (success = m_client.Call(ak::wwise::core::getInfo,
			AkJson(AkJson::Type::Map),
			AkJson(AkJson::Type::Map),
			wwiseInfo))
		{

			//create a status text string and set it
			std::stringstream status;
			status << "Connected on port " + std::to_string(g_Waapi_Port) + ": ";
			status << wwiseInfo["displayName"].GetVariant().GetString();
			status << " - " + wwiseInfo["version"]["displayName"].GetVariant().GetString();
			std::string WwiseConnectionStatus = status.str();
			if (!suppressOuputMessages)
			{
				MessageBox(NULL, WwiseConnectionStatus.c_str(), "Wwise Connection Status", MB_OK);
			}
		}
	}

	if (!success)
	{
		if (!suppressOuputMessages)
		{
			MessageBox(NULL, "! Failed to Connect !", "Wwise Connection Status", MB_OK);
		}
	}

	return success;
}

bool WAAPIConnect::GetSelectedWwiseObject()
{
	using namespace AK::WwiseAuthoringAPI;
	AkJson ReturnResults;
	AkJson options(AkJson::Map{
		{"return", AkJson::Array
		{
			AkVariant("id"),
			AkVariant("name"),
			AkVariant("type"),
			AkVariant("parent"),
			}
		}
		}
	);

	bool success = false;

	if (success = m_client.Connect("127.0.0.1", g_Waapi_Port))
	{
		if (success = m_client.Call(ak::wwise::ui::getSelectedObjects, AkJson(AkJson::Map()), options, ReturnResults))
		{
			AkJson::Array SelectedObjectReturn;
			GetWaapiResultsArray(SelectedObjectReturn,ReturnResults);

			for (const auto &result : SelectedObjectReturn)
			{
				const std::string wwiseObjectGuid = result["id"].GetVariant().GetString();
				const std::string wwiseObjectName = result["name"].GetVariant().GetString();
				const std::string wwiseObjectType = result["type"].GetVariant().GetString();
//				const std::string wwiseObjectParent = result["parent"].GetVariant().GetString();

				//create a status text string and set it
				std::stringstream status;
				status << wwiseObjectGuid + " Named: " +wwiseObjectName;
				status << " is type " + wwiseObjectType;
//				status << " Parent =  " + wwiseObjectParent;
				std::string SelectedObject = status.str();

				MessageBox(NULL, SelectedObject.c_str(), "Wwise Objects Selected", MB_OK);
			}
		}
	}
	else
	{
		MessageBox(NULL, "No Wwise Connection Found!", "Wwise Objects Selected", MB_OK);
	}


	return false;
}

void WAAPIConnect::GetWaapiResultsArray(AK::WwiseAuthoringAPI::AkJson::Array & arrayIn, AK::WwiseAuthoringAPI::AkJson & results)
{
	using namespace AK::WwiseAuthoringAPI;
	switch (results.GetType())
	{
	case AkJson::Type::Map:
	{
		if (results.HasKey("objects"))
		{
			arrayIn = results["objects"].GetArray();
			return;
		}
		else if (results.HasKey("return"))
		{
			arrayIn = results["return"].GetArray();
			return;
		}
		else
		{
			//assert(!"Not implemented.");
			return;
		}

	} break;


	default:
		//assert(!"Not implemented.");
		return;
	}
}