#include <iostream> 
#include <sstream>
#include <mutex>

#include "WaapiFunctions.h"
#include <AK/WwiseAuthoringAPI/AkVariantBase.h>
#include <AK/WwiseAuthoringAPI/waapi.h>
#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
//#include <AkAutobahn\JSONHelpers.h> - // TODO figure this out, not in the ak sdk anymore??
#include "reaperHelpers.h"

///Socket client for Waapi connection
AK::WwiseAuthoringAPI::Client * my_client = new AK::WwiseAuthoringAPI::Client();
CurrentWwiseConnection * myWwiseConnection;

std::mutex mx_waapi;

bool waapi_Connect(CurrentWwiseConnection &wwiseConnectionReturn)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	AkJson wwiseInfo;
	bool success = false;
	
	if ((success = my_client->Connect("127.0.0.1", wwiseConnectionReturn.port,waapi_DisconnectHandler)))
	{
		
		std::cout << "Connection Success" << std::endl;
		//Get Wwise info
		if ((success = my_client->Call(ak::wwise::core::getInfo,
									   AkJson(AkJson::Type::Map),
									   AkJson(AkJson::Type::Map),
									   wwiseInfo)))
		{
			wwiseConnectionReturn.Version = wwiseInfo["version"]["displayName"].GetVariant().GetString();
			wwiseConnectionReturn.year = wwiseInfo["version"]["year"].GetVariant().GetInt32();
			wwiseConnectionReturn.connected = true;
		}
	}
	if (!success)
	{
		std::cout << "Connection failed" << std::endl;
		//MessageBox(NULL, "! ERROR - Failed to Connect to Wwise. !", "Wwise Connection Status", MB_OK);
		wwiseConnectionReturn.connected = false;
		wwiseConnectionReturn.projectGlobals = WwiseProjectGlobals();//if not connected clear the project globals
	}
	myWwiseConnection = &wwiseConnectionReturn;
	
	return success;
}

void waapi_DisconnectHandler()
{
	std::cout << "Disconnected" << std::endl;
	waapi_Disconnect(*myWwiseConnection);
}

void waapi_Disconnect(CurrentWwiseConnection &wwiseConnectionReturn)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	
	if (wwiseConnectionReturn.connected)
	{
		my_client->Disconnect();
		wwiseConnectionReturn.Version = "";
		wwiseConnectionReturn.year = 0;
		wwiseConnectionReturn.connected = false;
		wwiseConnectionReturn.projectGlobals = WwiseProjectGlobals();
	}
	myWwiseConnection = &wwiseConnectionReturn;
}


bool waapi_SetAutomationMode(bool enable)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	AkJson automationMode = AkJson(AkJson::Map{ {"enable", AkVariant(enable)} });
	AkJson out = AkJson(AkJson::Map());
	AkJson res = AkJson(AkJson::Map());

	return my_client->Call(ak::wwise::debug::enableAutomationMode, automationMode, out, res);
}


bool waapi_GetSelectedWwiseObjects(AK::WwiseAuthoringAPI::AkJson & resultsOut, bool getNotes)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	AkJson ReturnResults;
	AkJson options(AkJson::Map{
		{ "return", AkJson::Array{
			AkVariant("id"),
			AkVariant("name"),
			AkVariant("type"),
			AkVariant("path"),
			AkVariant("parent"),
			AkVariant("workunit"),
			AkVariant("childrenCount"), } }
		});
	if (getNotes)
	{
		options["return"].GetArray().push_back(AkVariant("notes"));
	}
	return my_client->Call(ak::wwise::ui::getSelectedObjects, AkJson(AkJson::Map()), options, resultsOut);
}

bool waapi_GetChildrenFromGUID(const AK::WwiseAuthoringAPI::AkVariant &id,AK::WwiseAuthoringAPI::AkJson &results)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	AkJson query;
	AkJson args(AkJson::Map{
		{ "from", AkJson::Map{
			{ "id", AkJson::Array{ id } } } },
		{ "transform",
		{ AkJson::Array{ AkJson::Map{ { "select", AkJson::Array{ { "descendants" } } } } } }
		}
		});

	AkJson options(AkJson::Map{
		{ "return", AkJson::Array{
			AkVariant("id"),
			AkVariant("name"),
			AkVariant("path"),
			AkVariant("type"),
			AkVariant("parent"),
			AkVariant("childrenCount")
		} }
		});

	return my_client->Call(ak::wwise::core::object::get, args, options, results);
}

bool waapi_GetParentFromGUID(const AK::WwiseAuthoringAPI::AkVariant & id, AK::WwiseAuthoringAPI::AkJson & results)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	AkJson query;
	AkJson args(AkJson::Map{
		{ "from", AkJson::Map{
			{ "id", AkJson::Array{ id } } } },
		{ "transform",
		{ AkJson::Array{ AkJson::Map{ { "select", AkJson::Array{ { "parent" } } } } } }
		}
		});

	AkJson options(AkJson::Map{
		{ "return", AkJson::Array{
			AkVariant("id"),
			AkVariant("name"),
			AkVariant("path"),
			AkVariant("type"),
			AkVariant("parent"),
			AkVariant("childrenCount")
		} }
		});

	return my_client->Call(ak::wwise::core::object::get, args, options, results);
}

bool waapi_GetObjectFromArgs(ObjectGetArgs & getArgs, AK::WwiseAuthoringAPI::AkJson & results)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;

	//Check for missing inputs
	if (getArgs.From[0] == "" || getArgs.From[1] == "")
	{
		PrintToConsole("!ERROR! - One or more required inputs are missing from GetObjectFromArgs call");
		return false;
	}
	AkVariant from0 = getArgs.From[0];
	AkVariant from1 = getArgs.From[1];
	AkVariant Akselect = getArgs.Select;

	AkJson args;

	if (getArgs.Where[0] == "" || getArgs.Where[1] == "") // If both Where args are empty
	{
		if (getArgs.Select == "")	// If Select is empty (we are running with no transform
		{
			args = (AkJson::Map{{ "from", AkJson::Map{ { from0, AkJson::Array{ from1 } } } } });
		}
		else
		{
			args = (AkJson::Map{
				{ "from", AkJson::Map{ { from0, AkJson::Array{ from1 } } } },
				{ "transform",{ AkJson::Array
				{ AkJson::Map{ { "select",AkJson::Array{ { Akselect } } } } },
				} } });
		}
	}
	else
	{
		AkVariant where0 = getArgs.Where[0];
		std::string s_where1 = getArgs.Where[1];	// When using name:contains AK complains about expecting a string type argument, but seemingly this args needs to be AkVariant???
		AkVariant where1 = getArgs.Where[1];
		int type = where1.GetType();

		////////// FIGURED IT OUT!!	///// Type and Category needs array args, name search just needs a string!

		if (getArgs.Where[0] == "name:contains" || getArgs.Where[0] == "name:matches")
		{
			args = (AkJson::Map{
				{ "from", AkJson::Map{ { from0, AkJson::Array{ from1 } } } },
				{ "transform",{ AkJson::Array
				{ { AkJson::Map{ { "select",AkJson::Array{ { Akselect } } } } },
				{ AkJson::Map{ { "where", AkJson::Array{ { where0,  where1 } } } } } }
				} } });
		}
		else
		{
			args = (AkJson::Map{
				{ "from", AkJson::Map{ { from0, AkJson::Array{ from1 } } } },
				{ "transform",{ AkJson::Array
				{ { AkJson::Map{ { "select",AkJson::Array{ { Akselect } } } } },
				{ AkJson::Map{ { "where", AkJson::Array{ { where0,  AkJson::Array{ where1 } } } } } } }
				} } });

		}
	}


//	using namespace AK::WwiseAuthoringAPI::JSONHelpers;
//	std::string argsToString = GetAkJsonString(args);
//	PrintToConsole(argsToString);

	AkJson options(AkJson::Map{
		{ "return", AkJson::Array{
			AkVariant("id"),
			AkVariant("name"),
			//AkVariant("path"),
			//AkVariant("type"),
			//AkVariant("parent"),
			//AkVariant("childrenCount")
		} }
		});

	if (!getArgs.customReturnArgs.empty())
	{
		for (auto i : getArgs.customReturnArgs)
		{
			if (i != "")
			{
				options["return"].GetArray().push_back(AkVariant(i));
			}
		}
	}

	return my_client->Call(ak::wwise::core::object::get, args, options, results);
}

std::string GetPropertyFromGUID(const AK::WwiseAuthoringAPI::AkVariant & id, std::string property, bool usePath)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	AkJson query;
	AkJson args;

	if (usePath)
	{
		args = (AkJson::Map{
	{ "from", AkJson::Map{
		{ "path", AkJson::Array{ id } } } }
			});
	}
	else {
		args = (AkJson::Map{
	{ "from", AkJson::Map{
		{ "id", AkJson::Array{ id } } } }
			});
	}

	AkJson options(AkJson::Map{
		{ "return", AkJson::Array{
			AkVariant(property)
		} }
		});

	AkJson results;

	my_client->Call(ak::wwise::core::object::get, args, options, results);

	return std::string();
}

bool waapi_CreateObjectFromArgs(CreateObjectArgs & createArgs, AK::WwiseAuthoringAPI::AkJson & results)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;

	//Check for missing inputs
	if (createArgs.ParentID == "" || createArgs.Name == "" || createArgs.Type == "")
	{
		PrintToConsole("!ERROR! - One or more required inputs are missing from Create Objects call");
		return false;
	}
	bool autoAddSC = true;

	// Do Source control operations
	waapi_DoWorkgoupOperation(CheckoutWWU, createArgs.ParentID);


	AkJson args; //"@RandomOrSequence"
	args = (AkJson::Map{
		{ "parent",AkVariant(createArgs.ParentID)},
		{ "type", AkVariant(createArgs.Type) },
		{ "name", AkVariant(createArgs.Name)},
		{ "onNameConflict", AkVariant(createArgs.onNameConflict)},
		{ "notes", AkVariant(createArgs.Notes)},
		//{ "@Volume", AkVariant(-6)}	// Add properties like this
		});

	if (createArgs.Type == "RandomSequenceContainer")
	{
		args.GetMap().insert(std::make_pair("@RandomOrSequence", AkVariant(createArgs.RandomOrSequence)));
	}
	if (myWwiseConnection->year > 2017 && createArgs.Type !="Event") // Events cant have this property
	{
		args.GetMap().insert(std::make_pair("autoAddToSourceControl", AkVariant(autoAddSC)));
	}
	if (createArgs.Type == "Event")
	{
		// GetPropertyFromGUID(createArgs.eventArgs.target, "type", true);  /// TODO - Need to finish this. Check type of event target

		AkJson::Array eventArgs;
		eventArgs.push_back(AkJson::Map{
			{"name",AkVariant("0")},
			{"type",AkVariant("Action")},
			{"@ActionType",AkVariant(createArgs.eventArgs.action)},
			{"@Target",AkVariant(createArgs.eventArgs.target)},

			});
		args.GetMap().insert(std::make_pair("children", eventArgs));
	}


	AkJson options = AkJson(AkJson::Map());
	return my_client->Call(ak::wwise::core::object::create, args, options, results);
}

bool waapi_SetNotesForObject(std::string id, std::string notes,AK::WwiseAuthoringAPI::AkJson & results)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;

	//Check for missing inputs
	if (id == "" || notes == "")
	{
		PrintToConsole("!ERROR! - One or more required inputs are missing from Set Notes call");
		return false;
	}

	// Do Source control operations
	waapi_DoWorkgoupOperation(CheckoutWWU, id);


	AkJson args; //"@RandomOrSequence"
	args = (AkJson::Map{
		{ "object",AkVariant(id)},
		{ "value", AkVariant(notes)}
		});
	
	AkJson options = AkJson(AkJson::Map());
	return my_client->Call(ak::wwise::core::object::setNotes, args, options, results);
}


bool wappi_ImportFromArgs(ImportObjectArgs & importArgs, AK::WwiseAuthoringAPI::AkJson & results)
{
	//const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;

	// Do Source control operations
	waapi_DoWorkgoupOperation(CheckoutWWU, importArgs.ImportLocation);

	AkJson::Array items;

	for (auto importFile : importArgs.ImportFileList)
	{
		std::string notes = "rpp:"+ importArgs.SourceReaperProject + "\nNotes:" +importArgs.Notes;
		AkJson importItem = AkJson(AkJson::Map{
			{ "audioFile", AkVariant(importFile.first) },
			{ "objectPath", AkVariant(importFile.second) },
			{ "notes",AkVariant(notes)}
			});
		items.push_back(importItem);
	}
	bool autoAddSC = true;
	AkJson args;
	args = (AkJson::Map{
		{ "importOperation", AkVariant(importArgs.importOperation) },
		{ "default", AkJson::Map{
			{ "importLanguage", AkVariant(importArgs.ImportLanguage) },
			{ "importLocation", AkVariant(importArgs.ImportLocation) },
		//	{ "objectType", AkVariant(importArgs.objectType) },
			{ "originalsSubFolder", AkVariant(importArgs.OriginalsSubFolder) }
		} },
		{ "imports", items }
		});

	if (myWwiseConnection->year > 2017)
	{
		args.GetMap().insert(std::make_pair("autoAddToSourceControl", AkVariant(autoAddSC)));
	}

	AkJson options(AkJson::Map{
	{ "return", AkJson::Array{
		AkVariant("id"),
		AkVariant("name"),
		AkVariant("path"),
		AkVariant("type"),
		//AkVariant("parent"),
		//AkVariant("childrenCount")
	} }
		});

	return my_client->Call(ak::wwise::core::audio::import, args, options, results);
}

void waapi_GetWaapiResultsArray(AK::WwiseAuthoringAPI::AkJson::Array & arrayIn, AK::WwiseAuthoringAPI::AkJson & results)
{
	const std::lock_guard<std::mutex> lock(mx_waapi);
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
		else if (results.HasKey("id") && (results.HasKey("name")))
		{
			arrayIn.push_back(results);
		}
		else
		{
			//MessageBox(NULL,"!Error! Malformed Results array","Waapi Results Array Error", MB_OK);
			PrintToConsole("Waapi results array empty");
			return;
		}
	} break;
	default:
		//MessageBox(NULL, "!Error! Results array is not of type AkJson::Map", "Waapi Results Array Error", MB_OK);
		PrintToConsole("Waapi results are not of type AkJson::Map");
		return;
	}
}

void waapi_HELPER_Print_AkJson_Array(AK::WwiseAuthoringAPI::AkJson::Array & printResults)	// TODO figure out JSON helpers, not in the AK SDK anymore?
{
//	using namespace AK::WwiseAuthoringAPI::JSONHelpers;
//	std::string argsToString = GetAkJsonString(printResults);
//	PrintToConsole(argsToString);
}

void waapi_HELPER_Print_AkJson_Map(AK::WwiseAuthoringAPI::AkJson::Map & printResults)
{
//	using namespace AK::WwiseAuthoringAPI::JSONHelpers;
//	std::string argsToString = GetAkJsonString(printResults);
//	PrintToConsole(argsToString);
}

bool waapi_TranslateJSONResults(std::map<std::string,std::string>& INstringResults, std::map<std::string, double>& INnumberResults, AK::WwiseAuthoringAPI::AkJson result, std::string stringKey)
{
	const std::lock_guard<std::mutex> lock(mx_waapi);
	using namespace AK::WwiseAuthoringAPI;
	int resultCount = 1;
	//std::string argsToString = JSONHelpers::GetAkJsonString(result);


	for (const auto i : result.GetMap()) {
		AkJson::Type type;
		std::string stringKey = i.first;
		type = i.second.GetType();

		///Type is already AK Variant
		if (type == AkJson::Type::Variant)
		{
			AkVariant variant = result[stringKey].GetVariant();
			if (variant.IsString())
			{
				//push value into string results
				std::string key = stringKey;
				std::string value = variant.GetString();
				INstringResults[key] = value;
			}
			else if (variant.IsNumber())
			{
				//push value into number results
				std::string key = stringKey;
				double value = variant.operator double();
				INnumberResults[key] = value;
			}
			else if (variant.GetType() == 11)//Type is bool
			{
				std::string key = stringKey;
				bool b_value = variant.GetBoolean();
				std::string value = std::to_string(b_value);
				INstringResults[key] = value;
			}
		}
		else if (type == AK::WwiseAuthoringAPI::AkJson::Type::Map)
		{
			for (const auto x : result[stringKey].GetMap())
			{
				std::string first = x.first;
				AkVariant variant = x.second.GetVariant();
				if (variant.IsString())
				{
					//push value into string results
					std::string key = first;
					std::string value = variant.GetString();
					INstringResults[key] = value;
				}
				else if (variant.IsNumber())
				{
					//push value into number results
					std::string key = first;
					double value = variant.operator double();
					INnumberResults[key] = value;
				}
				else if (variant.GetType() == 11)//Type is bool
				{
					std::string key = stringKey;
					bool b_value = variant.GetBoolean();
					std::string value = std::to_string(b_value);
					INstringResults[key] = value;
				}
			}
		}
		else if (type == AK::WwiseAuthoringAPI::AkJson::Type::Array)
		{
			///Not implemented
			if (result.HasKey("objects"))
			{
				for (const auto obj : result["objects"].GetArray())
				{
					for (const auto x : obj.GetMap())
					{
						std::string first = x.first;
						AkVariant variant = x.second.GetVariant();
						if (variant.IsString())
						{
							//push value into string results
							std::string key = first;
							std::string value = variant.GetString();
							INstringResults[key] = value;
						}
						else if (variant.IsNumber())
						{
							//push value into number results
							std::string key = first;
							double value = variant.operator double();
							INnumberResults[key] = value;
						}
						else if (variant.GetType() == 11)//Type is bool
						{
							std::string key = stringKey;
							bool b_value = variant.GetBoolean();
							std::string value = std::to_string(b_value);
							INstringResults[key] = value;
						}
					}
				}
			}

		}
		else
		{
			//"Ak retunr Type not found";
			return false;
		}
	}

	return true;
}


bool waapi_SaveWwiseProject()
{
	using namespace AK::WwiseAuthoringAPI;
	AkJson in = AkJson(AkJson::Map());
	AkJson out = AkJson(AkJson::Map());
	AkJson res = AkJson(AkJson::Map());
	bool result = my_client->Call(ak::wwise::core::project::save, in, out, res);
	if (!result)
	{
		std::string error = res["message"].GetVariant().GetString();
		PrintToConsole(error);
	}
	return result;
}

bool waapi_OpenWwiseProject(std::string proj)
{
	return false;
}

bool waapi_UndoHandler(undoStep undoStep, std::string undoTag)
{
	using namespace AK::WwiseAuthoringAPI;
	AkJson in = AkJson(AkJson::Map());
	AkJson akj_undoTag = AkJson(AkJson::Map{ {"displayName", AkVariant(undoTag)} });
	AkJson out = AkJson(AkJson::Map());
	AkJson res = AkJson(AkJson::Map());
	std::string s_res;
	switch (undoStep)
	{
	case Begin:
		return my_client->Call(ak::wwise::core::undo::beginGroup, in, out, res);
		break;
	case End:
		return my_client->Call(ak::wwise::core::undo::endGroup, akj_undoTag, out, res);
		break;
	case Cancel:
		return my_client->Call(ak::wwise::core::undo::cancelGroup, in, out, res);
		break;
	default:
		break;
	}
	return false;
}

bool waapi_DoWorkgoupOperation(SourceControlOperation operation, std::string target)
{
	using namespace AK::WwiseAuthoringAPI;

	std::string s_operation;
	switch (operation)
	{
	case CheckoutWWU:
		s_operation = "WorkgroupCheckoutWWU";
		break;
	case RevertWWU:
		s_operation = "WorkgroupRevertWWU";
		break;
	case UpdateWWU:
		s_operation = "WorkgroupUpdateWWU";
		break;
	case CommitWWU:
		s_operation = "WorkgroupCommitWWU";
		break;
	default:
		s_operation = "WorkgroupCheckoutWWU";
		break;
	}


	AkJson args;
	args = AkJson::Map{
		{ "command", AkVariant(s_operation)},
		{ "objects", AkJson::Array{
			AkVariant(target),
		} }};

	AkJson options = AkJson(AkJson::Map());
	AkJson results = AkJson(AkJson::Map());


	return my_client->Call(ak::wwise::ui::commands::execute, args, options, results);
}

bool waapi_SetupSubscription(const char *subscription,
							 AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback,
							 uint64_t &subscriptionID)
{
	using namespace AK::WwiseAuthoringAPI;
	
	AkJson options = AkJson(AkJson::Map());
	AkJson results = AkJson(AkJson::Map());
	
	return my_client->Subscribe(subscription, options, in_callback, subscriptionID, results);
}

bool waapi_Unsubscribe(const uint64_t& in_subscriptionId)
{
	using namespace AK::WwiseAuthoringAPI;
	
	AkJson results = AkJson(AkJson::Map());
	
	return my_client->Unsubscribe(in_subscriptionId, results);
}

bool waapi_CheckForProjectFileChanges()
{
	//TODO - this is only supported in 2021.1.4 wwise sdk!
	using namespace AK::WwiseAuthoringAPI;
	AkJson args;
	args = AkJson::Map{
		{ "command", AkVariant("CheckProjectFiles")},
		{ "objects", AkJson::Array{
		} } };

	AkJson options = AkJson(AkJson::Map());
	AkJson results = AkJson(AkJson::Map());
	bool result = my_client->Call(ak::wwise::ui::commands::execute, args, options, results);
	if (!result)
	{
		std::string error = results["message"].GetVariant().GetString();
		PrintToConsole(error);
	}
	return result;
	
}
//bool Unsubscribe(const uint64_t& in_subscriptionId, AkJson& out_result, int in_timeoutMs = -1);
