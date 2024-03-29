
#include "WwiseConnectionHandler.h"
#include "ReaperRenderQueParser.h"
#include "reaperHelpers.h"
#include "gui_Transfer.h"
#include "platformhelpers.h"
#include <filesystem>
#include <thread>
#include <iostream>
#include <chrono>
#include <mutex>
#include <future>
#include <thread>
#include "workunithelper.h"
#include "wav_helper.h"
#include "Reaper_ExtState_Helper.h"


#include "ConfigFileHandler.h"

///Handles to UI elements
int tr_buttonConnect;
int tr_textConnectionStatus;
int tr_B_GetSelectedParent;
int tr_s_ImportParentID;
int tr_s_ImportParentNameType;
int tr_B_CreateObject;
int tr_c_CreateType;
std::string s_CreateType;
int tr_c_CreateNameConflict;
std::string s_CreateNameConflict;
int tr_txt_CreateName;
int tr_txt_CreateNotes;
int tr_Tree_RenderJobTree;
int tr_Progress_Import;
int B_RenderImport;
int B_RefreshTree;
int txt_status;
int check_IsVoice;
int txt_Language;
std::string defaultLanguage = "English(US)";
int check_OrigDirMatchWwise;
int txt_OriginalsSubDir;
int check_CreateEvent;
int l_eventOptions;
std::string importEventOption;

CreateObjectChoices myCreateChoices;

//std::vector<RenderQueJob> GlobalListOfRenderQueJobs;
//std::vector<std::string> RenderFilesBackup;

bool AllDone = false;
std::mutex mtx;
int numOfRendersDone = 0;
bool CreateImportWindow::isReaperRendering = false;

//=============================================================================

void CreateImportWindow::SetupPluginParent(WwiseConnectionHandler * parent)
{
	WwiseConnectionHnd = parent;
	thisPluginWindow = this;
}




CreateImportWindow::CreateImportWindow()
{
	WwiseConnectionHnd = new WwiseConnectionHandler();
	LoadSettingsFromExtState();
}

CreateImportWindow::~CreateImportWindow()
{
	SaveSettingsToExtState();
	SaveProject();
	owningGUIWindow = nullptr;
}


//=============================================================================
void CreateImportWindow::OnCommand(int id, int notifycode)
{

}
//=============================================================================
void CreateImportWindow::OnInitDlg()
{

	//config myConfig;
	ReadConfigFile(myConfig);
	//WwiseConnectionHnd->SetOptionsFromConfig(myConfig);
	
	LoadSettingsFromExtState();
	
	
	WwiseConnectionHnd->Settings_SetWaapiPort(TransferComponentSettings.waapiport);
	WwiseConnectionHnd->Settings_SetUseAutomationMode(TransferComponentSettings.useAtomationMode);
	

}
//=============================================================================
inline int CreateImportWindow::ErrMsg(const std::string& s)
{
	//return MessageBox(0, s.c_str(), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
	return 0;
}

//////////////////////////////////
////	Handle UI notifications 


void CreateImportWindow::handleUI_B_Connect()
{
	if (WwiseConnectionHnd->handle_GUI_Connect())
	{
		//SetStatusMessageText("Ready");
		//SetDlgItemText(m_hWindow, IDC_WwiseConnection, "Wwise Connection Established");
		//		SendMessage(hwnd_combo, CB_SETCURSEL, 0, 0);
		//		textConnectionStatus
	}
	else
	{
		//SetStatusMessageText("Error");
		//SetDlgItemText(m_hWindow, IDC_WwiseConnection, "!!Wwise Connection Missing!!");
	}
}

void CreateImportWindow::handleUI_B_CreateObject(CreateObjectArgs myCreateObjectArgs)
{

	/// Get selected wwise object first
	WwiseObject selectedParent = WwiseConnectionHnd->GetSelectedObject();

	//PrintToConsole("Creating New Wwise Object");

	

	if (selectedParent.properties.empty())
	{
		//PrintToConsole("No Wwise object selected..");
		return;
	}

	myCreateObjectArgs.ParentID = selectedParent.properties["id"];
	myCreateObjectArgs.ParentType = selectedParent.properties["type"];
	myCreateObjectArgs.Workunit = selectedParent.properties["workunit"];

	if (WwiseConnectionHnd->MyCurrentWwiseConnection.useAutomationMode)
	{
		if (!waapi_SetAutomationMode(true))
		{
			//PrintToConsole("Failed to set automation mode. Not supported in WAAPI 2017 or earlier");
		}
	}
	
	waapi_UndoHandler(Begin, "Create Objects");
	
	if (myCreateObjectArgs.count>1)
	{
		myCreateObjectArgs.Name += "_01";//if we are making multiple then set the first one to _01 so that wwise renaming takes care of the itteration
		//createArgs.onNameConflict = "rename";
	}
	for (int i=0; i < myCreateObjectArgs.count; i++)
	{
		AK::WwiseAuthoringAPI::AkJson::Array results;
		if (!WwiseConnectionHnd->CreateWwiseObject(false, myCreateObjectArgs, results,false))
		{
			//ERROR
			SetStatusMessageText("Error");
			waapi_UndoHandler(Cancel, "Create Objects");
			waapi_SetAutomationMode(false);
			return;
		}
		if (myCreateObjectArgs.createPlayEvent)
		{
			if (myCreateObjectArgs.Type != "ActorMixer")
			{
				//create play event for the object we just created
				std::string id = results[0]["id"].GetVariant();
				std::string name = results[0]["name"].GetVariant();
				WwiseObject createdObj = GetWwiseObjectFromID(id);
				WwiseObject parent = GetWwiseObjectFromID(createdObj.properties["parent_id"]);
				CreatePlayEventForID(id, name,"",parent.properties["path"]);
			}
		}
		if (myCreateObjectArgs.Notes != "")
		{
			std::string id = results[0]["id"].GetVariant();
			std::string notes = myCreateObjectArgs.Notes;
			AK::WwiseAuthoringAPI::AkJson results;
			WwiseConnectionHnd->SetNotesForObject(id, notes, results);
		}
	}
	
		waapi_UndoHandler(End, "Create Objects");
		waapi_SaveWwiseProject();
	//PrintToConsole("Failed to set automation mode. Not supported in WAAPI 2017 or earlier");
	if (WwiseConnectionHnd->MyCurrentWwiseConnection.useAutomationMode)
	{
		waapi_SetAutomationMode(false);
	}
	
	if (owningGUIWindow)
	{
		TransferToWwiseComponent* ownerAsTransferComponent = dynamic_cast<TransferToWwiseComponent*>(owningGUIWindow);
		if (ownerAsTransferComponent)
		{
			ownerAsTransferComponent->SaveEventsToExState();
		}
	}
	SetStatusMessageText("Ready");

	//waapi_CreateObjectFromArgs(myCreateObjectArgs, results);
}

void CreateImportWindow::handleUI_GetType(int notifCode)
{
	int x = 0;
	switch (notifCode)
	{
	case 0://CBN_SELCHANGE:
		//x = SendMessage(tr_c_CreateType, CB_GETCURSEL, 0, 0);
		s_CreateType = myCreateChoices.waapiCREATEchoices_TYPE[x];
		break;
	default:
		break;
	}
}

void CreateImportWindow::handleUI_GetImportEventOptions(int notifCode)
{
	int x = 0;
	switch (notifCode)
	{
	case 0://CBN_SELCHANGE:
		//x = SendMessage(l_eventOptions, CB_GETCURSEL, 0, 0);
		importEventOption = myCreateChoices.waapiCREATEchoices_EVENTOPTIONS[x];
		break;
	default:
		break;
	}
}

void CreateImportWindow::handleUI_GetNameConflict(int notifCode)
{
	int x = 0;
	switch (notifCode)
	{
		case 0://CBN_SELCHANGE:
		//x = SendMessage(tr_c_CreateNameConflict, CB_GETCURSEL, 0, 0);
		s_CreateNameConflict = myCreateChoices.waapiCREATEchoices_NAMECONFLICT[x];
		break;
	default:
		break;
	}
}

bool CreateImportWindow::UpdateProgressDuringRender(int numJobs)
{
	AllDone = false;
	numOfRendersDone = 0;
	bool finalCheckAfterRenderDone = false;
	
	while (!AllDone)
	{
		
		if (numOfRendersDone == numJobs) {
			AllDone = true;
			//PrintToConsole("ALL Renders complete");
			std::cout << "All renders complete" << std::endl;
			return true;
		}
		
		else
		{
			
		}

		int jobIndex = 0;
		for (auto job : GlobalListOfRenderQueJobs)
		{
			if (!GlobalListOfRenderQueJobs[jobIndex].hasRendered)
			{
				std::error_code ec;
				if (!std::filesystem::exists(job.RenderQueFilePath,ec))
				{
					//std::cout << "File not found!" << std::endl;
				
					if (!GlobalListOfRenderQueJobs[jobIndex].hasRendered)
						{
							//PostMessage(tr_Progress_Import, PBM_STEPIT, 0, 0);
							//SendMessage(tr_Progress_Import, PBM_STEPIT, 0, 0);

							GlobalListOfRenderQueJobs[jobIndex].hasRendered = true;
							numOfRendersDone++;
						}
				}
				jobIndex++;
			}
		
		}
		
		mtx.lock();
		if (CreateImportWindow::isReaperRendering == false)
		{
			mtx.unlock();
			if ((numOfRendersDone != numJobs) && (finalCheckAfterRenderDone))
			{
				std::cout << "Reaper stopped rendering, but not all render jobs are accounted for" << std::endl;
				return false;
			}
			finalCheckAfterRenderDone = true;
		}
		mtx.unlock();

	}
	
	return false;;
}



bool CreateImportWindow::handleUI_RenderImport()
{
	/// Render and import from que list

	//For each job in the global render que
	//check it has valid wwise parent and report errors/exit if not
	
	if (GlobalListOfRenderQueJobs.size() == 0)
	{
		//There are no render jobs!
		PrintToConsole("  ERROR! There are no render jobs! Add items to a render que to use them with this tool..   \n");
		SetStatusMessageText("Error");
		return false;
	}

	for (auto job : GlobalListOfRenderQueJobs)
	{
		if (job.parentWwiseObject.properties.size() == 0 && job.hasPerFileOverrides==false)
		{
			PrintToConsole("  ERROR! A render job has no import settings!   \n");
			PrintToConsole(job.RenderQueFilePath);
			SetStatusMessageText("Error");
			return false;
		}

	}

	SetStatusMessageText("Rendering from Reaper Render Que");

	int numJobs = GlobalListOfRenderQueJobs.size();

	//backup render que files in case of errors
	backupRenderQueFiles();

	bool success = false;

	//SendMessage(tr_Progress_Import, PBM_SETRANGE, 0, MAKELPARAM(0, numJobs));

	//SendMessage(tr_Progress_Import, PBM_SETSTEP, (WPARAM)1, 0);
	mtx.lock();
	CreateImportWindow::isReaperRendering = true;
	mtx.unlock();

	std::future<bool> fut = std::async(std::launch::async,&CreateImportWindow::UpdateProgressDuringRender,this,numJobs);

	//std::thread progressThread(&CreateImportWindow::UpdateProgressDuringRender, this, numJobs);

	ReaperRenderObj renderObj;
	renderObj.RenderAllQues();

	//std::this_thread::sleep_for(std::chrono::seconds(20));

	//PrintToConsole("Render done. Waiting for second thread");
	//SendMessage(tr_Progress_Import, PBM_SETPOS, numJobs, 0);
	mtx.lock();
	CreateImportWindow::isReaperRendering = false;
	mtx.unlock();


	//progressThread.join();
	
	std::future_status status;

	status = fut.wait_for(std::chrono::seconds(1));

	//PrintToConsole("Rejoined main");
	

	success = fut.get();

	//MSG msg;	//Clears the message que for the progress bar
	//PeekMessage(&msg,tr_Progress_Import,0,0,PM_REMOVE);
	
	if ((status == std::future_status::ready) && (success))
	{
		SetStatusMessageText("Importing into Wwise");
		//SendMessage(tr_Progress_Import, PBM_SETPOS, 0, 0);
		std::cout << "Render complete - attempting wwise import" << std::endl;
		
		if (!ImportJobsIntoWwise())
		{
			// something went wrong, restore render que files
			//PrintToConsole("Something went wrong, restoring Reaper Render Que files.....");
			std::cout << "Error in wwise import stage" << std::endl;
			restoreRenderQueFiles();
			return false;

		}
		return true;
	}
	else if ((status == std::future_status::timeout) || (!success))
	{
		//PrintToConsole("Timeout error. Something went wrong in Render. Reaper did not remove all render que files after processing.");
		std::cout << "Error in Reaper Render stage" << std::endl;
		SetStatusMessageText("Error");
		restoreRenderQueFiles();
		return false;
	}
	
	return false; // Shouldnt ever get here, if we do something has gone wrong so return false
}

void CreateImportWindow::handleActiveSourceUpdatesForVersions()
{
	for (const auto& myPair : activeSourcesUpdateMap)
	{
		std::string workUnitPath = myPair.first;
		auto activeSources = myPair.second;

		std::unique_ptr<WUActiveSourceUpdater> workUnitUpdater(new WUActiveSourceUpdater);
		xml_parse_result result =  workUnitUpdater->LoadWorkUnit(workUnitPath);
		if (!result)continue;
		//work unit loaded successfully
		bool needsSave = false;
		for (const auto& activeSourceInfo : activeSources)
		{
			if (workUnitUpdater->UpdateActiveSource(activeSourceInfo.parentSoundID, activeSourceInfo.newActiveSourceName))
			{
				needsSave = true;
			}
		}
		if (needsSave)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			workUnitUpdater->SaveWorkUnit(workUnitPath);
			//workUnitUpdater->SaveWorkUnit("D:/save_file_output.xml");
			//WwiseConnectionHnd->CheckForProjectFileChanges(); // this is not working command id invalid??
		}
	}

}



bool CreateImportWindow::ImportJobsIntoWwise()
{
	SetStatusMessageText("Importing into Wwise");

	int jobIndex = 0;
	int importSuccesses = 0;
	double importProgress = 0;
	int numFilesToImport  = 0;
	
	//get the size of the list of render files in order to display progress
	for (auto &job : GlobalListOfRenderQueJobs)
	{
		numFilesToImport += job.RenderQueJobFileList.size();

		
	}
	
	
	for (auto &job : GlobalListOfRenderQueJobs)
	{
		if (job.hasRendered)
		{
			//Save the list of audio files we created

			SaveRenderOutputFilesToProjExState(job);


			///deal with file overrides here

			if (job.hasPerFileOverrides)
			{
				int fileOverrideIndex = 0;
				for (auto &fileOverride : job.perFileOverridesmap)
				{

					std::string file = fileOverride.second.RenderJobFile;

					// the RenderQueJob filelist is the whole path to the file, need to get this from the render job and use it for the import step

					for (auto &renderJobFilePath : job.RenderQueJobFileList)
					{
						std::size_t found = renderJobFilePath.rfind(file);
						if (found != renderJobFilePath.npos)
						{
							// we found a render file path matching our override file
							fileOverride.second.RenderJobFile = renderJobFilePath;

							//remove this overridden file from the main job render file list
							renderJobFilePath = "";

						}
					}

					std::string existingOriginalsPath = "";
					std::string existingWwisePath = "";
					//strip file at the os seperator to get just the filename
					std::string fullPath = file;
					if (file.rfind(PLATFORMHELPERS::kPathSeparator) != file.npos)
					{
						file.erase(0, file.rfind(PLATFORMHELPERS::kPathSeparator)+1);
					}

					if (AudioFileExistsInWwise(file,fileOverride.second.parentWwiseObject, existingOriginalsPath, existingWwisePath))
					{
						//audio file already exists under this parent, so replace the originals path
					//If file is not SFX (is localised) then need to strip out the top level language dir from the existing originals path, as it is handled by the import Language
						if (job.ImportLanguage != "SFX")
						{
							
							if (existingOriginalsPath.find(PLATFORMHELPERS::kPathSeparator) != existingOriginalsPath.npos)
							{
								existingOriginalsPath.erase(0, existingOriginalsPath.find(PLATFORMHELPERS::kPathSeparator) + 1);
							}

						}
						fileOverride.second.OrigDirMatchesWwise = false;
						fileOverride.second.userOrigsSubDir = existingOriginalsPath;
					}



					std::vector<std::string> audiofile;
					audiofile.push_back(fileOverride.second.RenderJobFile);

					std::string outtoken;
					if (IsAudioFileAVersion(file, outtoken))
					{
						fileOverride.second.Template = WwiseObject();
					}
					//deal with importing the overriden files
					ImportObjectArgs curFileOverrideImportArgs = SetupImportArgs
					(
						fileOverride.second.parentWwiseObject,
						fileOverride.second.EimportType,
						fileOverride.second.ImportLanguage,
						fileOverride.second.OrigDirMatchesWwise,
						fileOverride.second.userOrigsSubDir,
						audiofile,
						fileOverride.second.createEventOption,
						PLATFORMHELPERS::filenameFromPathString(job.ParentReaperProject),
						"",
						fileOverride.second.Template
						
					);
					if (ImportCurrentRenderJob(curFileOverrideImportArgs))
					{
						fileOverride.second.hasImported = true;
						importProgress++;
						if (owningGUIWindow)
						{
							owningGUIWindow->updateProgressValue(numFilesToImport/importProgress);
						}
					}

				}

			}


			// import the remaining files from the job that are not overrideen
			std::string existingSiblingOriginalsPath = "";

			for (auto file : job.RenderQueJobFileList)
			{
				if (file == "")
				{
					continue;
				}
				std::string existingOriginalsPath = "";
				std::string existingWwisePath = "";
				// file is the full path in this context, need to get the just the filename
				std::string fullPath = file;
				if (file.rfind(PLATFORMHELPERS::kPathSeparator) != file.npos)
				{
					file.erase(0, file.rfind(PLATFORMHELPERS::kPathSeparator)+1);
				}

				WwiseObject originalJobWwiseParent = job.parentWwiseObject;

				if (AudioFileExistsInWwise(file, originalJobWwiseParent, existingOriginalsPath, existingWwisePath))
				{
					//audio file already exists under this parent, so replace the originals path
					//If file is not SFX (is localised) then need to strip out the top level language dir from the existing originals path, as it is handled by the import Language
					if (job.ImportLanguage != "SFX")
					{
					
						if (existingOriginalsPath.find(PLATFORMHELPERS::kPathSeparator) != existingOriginalsPath.npos)
						{
							existingOriginalsPath.erase(0, existingOriginalsPath.find(PLATFORMHELPERS::kPathSeparator) + 1);
						}

					}
					//store the existing originals path, we might use it if we add extra files into this parent
					existingSiblingOriginalsPath = existingOriginalsPath;
					std::vector<std::string> importfiles;
					importfiles.push_back(fullPath);

					std::string outtoken;
					if (IsAudioFileAVersion(file, outtoken))
					{
						job.Template = WwiseObject();
					}
					ImportObjectArgs curJobImportArgs = SetupImportArgs
					(
						originalJobWwiseParent,
						job.EimportType,
						job.ImportLanguage,
						false,
						existingOriginalsPath,
						importfiles,
						job.createEventOption,
						PLATFORMHELPERS::filenameFromPathString(job.ParentReaperProject),
						"",
						job.Template
					);
					if (ImportCurrentRenderJob(curJobImportArgs))
					{
						for (auto &renderJobFile : job.RenderQueJobFileList)
						{
							if (renderJobFile == fullPath)
							{
								renderJobFile = "";
								importProgress++;
								if (owningGUIWindow)
								{
									owningGUIWindow->updateProgressValue(numFilesToImport/importProgress);
								}
							}
						}
					}

				}

			}

			// The job "should" be empty at this point, as we have handled all the overrides and looped through all the remaining import files. Not sure this last import step is needed here?
			ImportObjectArgs curJobImportArgs = SetupImportArgs
			(
				job.parentWwiseObject,
				job.EimportType,
				job.ImportLanguage,
				job.OrigDirMatchesWwise,
				job.userOrigsSubDir,
				job.RenderQueJobFileList,
				job.createEventOption,
				PLATFORMHELPERS::filenameFromPathString(job.ParentReaperProject),
				"",
				job.Template
			);
			if ((existingSiblingOriginalsPath != "") && (job.OrigDirMatchesWwise))
			{
				//We had some sibling audio in this parent already, so use the same originals path for new files
				curJobImportArgs.OriginalsSubFolder = existingSiblingOriginalsPath;
			}
			if (curJobImportArgs.ImportFileList.empty())
			{
				int numOfOverrides = job.perFileOverridesmap.size();
				int numOfImports = 0;
				// job list for importing was empty. Check if all overrides completed
				for (auto overrideJob : job.perFileOverridesmap)
				{
					if (overrideJob.second.hasImported)
					{
						numOfImports += 1;
					}
				}
				if (numOfImports == numOfOverrides)
				{
					GlobalListOfRenderQueJobs[jobIndex].hasImported = true;
					importSuccesses++;
					//SendMessage(tr_Progress_Import, PBM_SETPOS, importSuccesses, 0);
				}
			}
			else if (ImportCurrentRenderJob(curJobImportArgs))
			{
				GlobalListOfRenderQueJobs[jobIndex].hasImported = true;
				importSuccesses++;
				importProgress += curJobImportArgs.ImportFileList.size();
				if (owningGUIWindow)
				{
					owningGUIWindow->updateProgressValue(numFilesToImport/importProgress);
				}
				//SendMessage(tr_Progress_Import, PBM_SETPOS, importSuccesses, 0);
			}
			
		}
		jobIndex++;
		//owningGUIWindow->updateProgressValue(GlobalListOfRenderQueJobs.size()/importSuccesses);
	}

	if (importSuccesses == GlobalListOfRenderQueJobs.size())
	{
		//PrintToConsole("All jobs imported successfully");

		//Finished importing so if we have any version changes try to update the active sources
		if (!activeSourcesUpdateMap.empty())
		{
			if (WwiseConnectionHnd->SaveWwiseProject())//need to save changes so we can read the xml
			{
				handleActiveSourceUpdatesForVersions();
				activeSourcesUpdateMap.clear();
			}
		}

		SetStatusMessageText("All Import jobs complete");

		return true;
	}
	else
	{
		//PrintToConsole("Error! At least one import operation failed...");
		SetStatusMessageText("Error");
		return false;
	}



	
}

std::string CreateImportWindow::PrepareEventPathForCreation(std::string inPath, WwiseObject& OUT_parentObject) {

	if (WwiseConnectionHnd->IsValidWwiseGUID(inPath))
	{
		//inPath was an ID, so we dont need to do anything
		OUT_parentObject = WwiseConnectionHnd->GetWwiseObjectFromID(inPath);
		return inPath;
	}
	bool isPathInEventsAlready = (PLATFORMHELPERS::starts_with(inPath, "\\Events\\") or PLATFORMHELPERS::starts_with(inPath, "Events\\"));
	if (isPathInEventsAlready)
	{
		// inPath is already in the events hierarchy, so we dont need to do anything
		OUT_parentObject = WwiseConnectionHnd->GetWwiseObjectFromPath(inPath);
		return inPath;
	}

	bool isPathInActorMixer = (PLATFORMHELPERS::starts_with(inPath, "\\Actor-Mixer Hierarchy\\") or PLATFORMHELPERS::starts_with(inPath, "Actor-Mixer Hierarchy\\"));
	if (!isPathInActorMixer)
	{
		PrintToConsole("Error! When Preparing path for event creation, the input was not in Events or Actormixer paths!");
		PrintToConsole("InPath = " + inPath);
		return "";
	}

	std::string result = "";




	// Branch on different event options

	// Mirror only work units in actor mixer Path - 
	if (TransferComponentSettings.eEventCreationOption == ETransferEventCreationOption::E_MirrorActorMixerWorkUnits)
	{
		WwiseObject AMpaentObject = WwiseConnectionHnd->GetWwiseObjectFromPath(inPath); //this would be the actor mixer path of the object if we are doing mirroring behaviour
		if (!AMpaentObject.isEmpty)
		{
			while ((AMpaentObject.properties["name"] != "Actor-Mixer Hierarchy") or (AMpaentObject.properties["parent_id"] != ""))
			{
				if (AMpaentObject.properties["type"] == "WorkUnit")
				{
					//Try to find an existing WU in event structure
					auto foundObjects = GetWwiseObjectsByName(AMpaentObject.properties["name"] + TransferComponentSettings.eventWorkUnitSuffix, "WorkUnit", true, { "path" });
					if (foundObjects.size() > 0)
					{
						for (auto object : foundObjects)
						{
							if ((PLATFORMHELPERS::starts_with(object.properties["path"], "\\Events\\") or PLATFORMHELPERS::starts_with(object.properties["path"], "Events\\")))
							{
								OUT_parentObject = object;
								result = OUT_parentObject.properties["path"] + "\\" + result;
								result = PLATFORMHELPERS::stringReplace(result, "\\Events\\", "");
								return result;
							}
						}

					}

					result.insert(0, "<WorkUnit>" + (AMpaentObject.properties["name"] + TransferComponentSettings.eventWorkUnitSuffix) + "\\");

				}
				AMpaentObject = WwiseConnectionHnd->GetWwiseObjectFromID(AMpaentObject.properties["parent_id"]);
			}
			return result;
		}

	}
	
	else if (TransferComponentSettings.eEventCreationOption == ETransferEventCreationOption::E_UserEventWorkUnitPath)
	{
		// TO DO: Santize this in case user entered something bogus....
		result = TransferComponentSettings.UserEventPath;
		if (PLATFORMHELPERS::starts_with(result, "\\Events\\"))
		{
			result = PLATFORMHELPERS::stringReplace(result, "\\Events\\", "");
		}
		else if (PLATFORMHELPERS::starts_with(result, "Events\\"))
		{
			result = PLATFORMHELPERS::stringReplace(result, "Events\\", "");
		}
		return result;
	}

	return result;
}


void CreateImportWindow::CreatePlayEventForID(std::string id, std::string name,std::string notes,std::string path)
{
	//std::string remove = "\\Actor-Mixer Hierarchy";
	//path.erase(0, remove.length());

	CreateObjectArgs args;

	WwiseObject eventParentObj;
	std::string evPath = PrepareEventPathForCreation(path, eventParentObj);
	if (evPath.empty())
	{
		//There was some error in the preparation of the event path
		return;
	}

	if (TransferComponentSettings.eventMirroringDepth > 0)
	{
		//Handling cutting the path down to the required depth
		std::string cutEvPath = "";
		int targetdepth = TransferComponentSettings.eventMirroringDepth;
		int depthCount = 0;
		auto tokens = PLATFORMHELPERS::stringSplitToList(evPath, "\\");
		if (tokens.size() > targetdepth)
		{
			//PrintToConsole("Need to cull the path");
			for (auto token : tokens)
			{
				depthCount++;
				if (depthCount <= targetdepth)
				{
					cutEvPath.append(token + "\\");
					//result.insert(0, "<WorkUnit>" + (AMpaentObject.properties["name"] + TransferComponentSettings.eventWorkUnitSuffix) + "\\");
				}
				else
				{
					break;
				}
			}
			evPath = cutEvPath;
		}

	}



	///Need to seach for existing event work units before we try and create the structure
	/// Because if the structure is not mirrored exactly, we could fail to find the right parent, and then when trying to create it will fail if there is already a work unit with the same name!

	WwiseObject createdPathObj = WwiseConnectionHnd->CreateStructureFromPath(evPath, "Events");
	if (createdPathObj.isEmpty)
	{
		//failed to create the supplied event path, fallback to default work unit
		args.ParentID = "\\Events\\Default Work Unit";
	}
	else
	{
		createdPathObj = WwiseConnectionHnd->GetWwiseObjectFromID(createdPathObj.properties["id"]);// get a bunch of default properties for the new object, incl path
		args.ParentID = createdPathObj.properties["path"];
	}

	
	args.Type = "Event";
	args.Name = "Play_"+name;
	args.createPlayEvent = true;
	args.Notes = notes;

	EventCreateArgs eventArgs;
	eventArgs.target = id;
	
	args.eventArgs = eventArgs;

	
	
	
	
	AK::WwiseAuthoringAPI::AkJson::Array results;
	if (!WwiseConnectionHnd->CreateWwiseObject(false, args, results))
	{
		//PrintToConsole("Error creating Play event");
		return;
	}
	if (owningGUIWindow)
	{
		TransferToWwiseComponent* parentAsTransferComponent = dynamic_cast<TransferToWwiseComponent*>(owningGUIWindow);
		if (parentAsTransferComponent)
		{
			for (auto obj : results)
			{
				std::string name = obj["name"].GetVariant();
				parentAsTransferComponent->AddEventToSaveList(name);
			}
			
		}
	}
}

ImportObjectArgs CreateImportWindow::SetupImportArgs(WwiseObject parent,
													 IMPORT_TYPE importType,
													 std::string ImportLanguage,
													 bool OrigsDirMatchesWwise,
													 std::string userOrigSubDir,
													 std::vector<std::string> ImportFiles,
													 int eventCreateOption,
													 std::string SourceReaperProj,
													 std::string Notes,
													 WwiseObject Template
													 )
{
	std::string originalsPath = parent.properties["path"];
	std::string remove = "\\Actor-Mixer Hierarchy";
	originalsPath.erase(0, remove.length());

	ImportObjectArgs importArgs;
	importArgs.Notes = Notes;
	importArgs.SourceReaperProject = SourceReaperProj;
	importArgs.ImportLocation = parent.properties["path"];
	importArgs.ImportParentID = parent.properties["id"];
	importArgs.ImportLanguage = ImportLanguage;
	importArgs.templateObject = Template;
	switch (importType) {
		case Voice:
			importArgs.objectType = "<Sound Voice>";
			break;
		case Music:
			importArgs.objectType = "<MusicTrack>";
			break;
		default:
			importArgs.objectType = "<Sound SFX>";
			break;
	}
	if (OrigsDirMatchesWwise)
	{
		importArgs.OriginalsSubFolder = originalsPath;
	}
	else
	{
		importArgs.OriginalsSubFolder = userOrigSubDir;//"\\REAPER";
	}
	for (auto file : ImportFiles)
	{
		if (file == "")
		{
			continue;
		}
		std::string audiofile = file.substr(file.find_last_of("/\\")+1);
		std::string rawAudioFile = audiofile.substr(0, audiofile.find_last_of("."));
		std::string objectPath = parent.properties["path"] +"\\"+ importArgs.objectType + rawAudioFile;
		std::pair<std::string, std::string> imports;
		imports = std::make_pair(file, objectPath);
		importArgs.ImportFileList.push_back(imports);
	}

	importArgs.eventCreateOption = eventCreateOption;
	/*
	if (eventCreateOption == "Play@Children") // "None", "Play@Children" , "Play@Parent"
	{
		importArgs.eventCreateOption = 1;
	}
	else if (eventCreateOption == "Play@Parent")
	{
		importArgs.eventCreateOption = 2;
	}
	else importArgs.eventCreateOption = 0;
	*/

	return importArgs;
}

bool CreateImportWindow::ImportCurrentRenderJob(ImportObjectArgs curJobImportArgs)
{
	bool success;
	AK::WwiseAuthoringAPI::AkJson::Array results;
	success = WwiseConnectionHnd->ImportAudioToWwise(false, curJobImportArgs, results);
	
	if (curJobImportArgs.eventCreateOption == 2)//play event per sound file
	{
		for (auto obj : results)
		{
			std::string type = obj["type"].GetVariant();
			std::string name = obj["name"].GetVariant();
			if (type == "AudioFileSource")
			{	// If the type is audiosource we are replacing an existing sound
				WwiseObject wwiseObject = GetWwiseObjectFromID(obj["id"].GetVariant());
				auto parentID = wwiseObject.properties["parent_id"];
				WwiseObject parentObj = GetWwiseObjectFromID(parentID);
				std::vector<WwiseObject> events = WwiseConnectionHnd->FindPlayEventsForID(parentID);
				if (events.empty())
				{
					if (!parentObj.isEmpty)
					{
						auto pathParent = GetWwiseObjectFromID(parentObj.properties["parent_id"]);
						CreatePlayEventForID(parentObj.properties["id"],parentObj.properties["name"], "",pathParent.properties["path"]);
					}
					
				}
				else
				{
					if (owningGUIWindow)
					{
						TransferToWwiseComponent* parentAsTransferComponent = dynamic_cast<TransferToWwiseComponent*>(owningGUIWindow);
						if (parentAsTransferComponent)
						{
							for (auto event : events)
							{
								if (event.isEmpty) continue;
								parentAsTransferComponent->AddEventToSaveList(event.properties["name"]);
							}
						
						}
					}
				}
				continue;
				
			}
			std::string notes = "rpp:" + PLATFORMHELPERS::filenameFromPathString(curJobImportArgs.SourceReaperProject) + "\nNotes:";
			std::string versionToken = "";
			if (IsAudioFileAVersion(name, versionToken))
			{
				size_t found = name.rfind(versionToken);
				if (found != name.npos)
				{
					name = name.erase(found);
				}
			}
			WwiseObject wobj = GetWwiseObjectFromID(obj["id"].GetVariant());
			WwiseObject pwobj = GetWwiseObjectFromID(wobj.properties["parent_id"]);

			CreatePlayEventForID(obj["id"].GetVariant(), name, notes,pwobj.properties["path"]);
		}
	}
	else if (curJobImportArgs.eventCreateOption == 1)// play event for parent container
	{
		// \Actor-Mixer Hierarchy\Default Work Unit\MyAM\AnotherAM\New Random Container
		std::string target = curJobImportArgs.ImportLocation;
		std::string name = curJobImportArgs.ImportLocation.erase(0,curJobImportArgs.ImportLocation.rfind("\\")+1);
		std::string notes = "rpp:" + PLATFORMHELPERS::filenameFromPathString(curJobImportArgs.SourceReaperProject) + "\nNotes:";
		CreatePlayEventForID(target, name, notes);
	}
	if (success)
	{
		waapi_UndoHandler(End, "Auto Import");
	}

	return success;
}

bool CreateImportWindow::AudioFileExistsInWwise(std::string audioFile, WwiseObject& parent, std::string& existingOriginalDir, std::string& existingWwisePath)
{
	ObjectGetArgs getArgs;
	std::string id = parent.properties["id"];
	getArgs.From = { "id",id };
	getArgs.Select = "descendants";
	getArgs.Where = { "type:isIn","AudioFileSource" };
	getArgs.customReturnArgs.push_back("sound:originalWavFilePath"); 
	getArgs.customReturnArgs.push_back("path");
	getArgs.customReturnArgs.push_back("workunit");
	getArgs.customReturnArgs.push_back("filePath");

	AK::WwiseAuthoringAPI::AkJson::Array results;
	std::vector<WwiseObject> MyWwiseObjects;
	try {
		MyWwiseObjects = WwiseConnectionHnd->GetWwiseObjects(false, getArgs, results);
	}
	catch (std::string e) {
		//PrintToConsole(e);
	}

	for (const auto obj : MyWwiseObjects) {
		//PrintToConsole("");
		//PrintToConsole(obj.properties.at("name"));
		std::string name = obj.properties.at("name")+".wav";
		std::string nameForComparison = name;
		std::string audioFileNameForComparison = audioFile;
		
		// TODO Could maybe do the versioning here?
		std::string versionToken = myConfig.versionToken;
		bool isVersion = false;
		
		std::string foundVersionToken = "";
		if (IsAudioFileAVersion(audioFile, foundVersionToken))
		{
			isVersion = true;
			std::size_t pos = audioFileNameForComparison.find(foundVersionToken);
			if (pos != audioFileNameForComparison.npos)
			{
				audioFileNameForComparison = audioFileNameForComparison.erase(pos);
			}
			//audioFileNameForComparison = audioFileNameForComparison.erase(audioFileNameForComparison.length() - lengthToRemove, lengthToRemove);
			size_t verTokenPos = nameForComparison.length() - (foundVersionToken.length()+4);// last 8 chars of a version will be _v**.wav
			std::size_t found = nameForComparison.find(versionToken.c_str(), verTokenPos, 2);
			if (found != nameForComparison.npos)
			{
				nameForComparison = nameForComparison.erase(nameForComparison.length() - (foundVersionToken.length() + 4), foundVersionToken.length() + 4);
			}
			else
			{
				nameForComparison = nameForComparison.erase(nameForComparison.length() - 4, 4);
			}
		}

		/*
		size_t lengthToRemove = versionToken.length()+4;
		
		if (audioFile.length() >= lengthToRemove)
		{
			size_t verTokenPos = audioFile.length()-lengthToRemove;// last 8 chars of a version will be _v**.wav
			std::size_t found = audioFile.find(versionToken.c_str(),verTokenPos,2);//match the first 2 chars of the token (_v)
			if (found != audioFile.npos)
			{
				int i = 0;
				bool doesPatternMatch = false;
				for (auto c : versionToken)
				{
					if((isdigit(c)) != (isdigit(audioFile[found+i])))
					{
						break;
					}
					i++;
				}
				if (i == versionToken.length())
				{
					doesPatternMatch = true;
				}
				char c = audioFile[found+versionToken.length()];
				if (c != '.')
				{
					doesPatternMatch = false;
				}
				
				if (doesPatternMatch)
				{
					isVersion = true;
					audioFileNameForComparison = audioFileNameForComparison.erase(audioFileNameForComparison.length()- lengthToRemove, lengthToRemove);
					size_t verTokenPos = nameForComparison.length()- lengthToRemove;// last 8 chars of a version will be _v**.wav
					std::size_t found = nameForComparison.find(versionToken.c_str(),verTokenPos,2);
					if (found != nameForComparison.npos)
					{
						nameForComparison = nameForComparison.erase(nameForComparison.length()- lengthToRemove, lengthToRemove);
					}
					else
					{
						nameForComparison = nameForComparison.erase(nameForComparison.length()-4,4);
					}
				}
			}
		
		}
		*/
		
		if (nameForComparison == audioFileNameForComparison)
		{
			std::string wwisePath = obj.properties.at("path");
			std::string fullPath = obj.properties.at("sound:originalWavFilePath");

			ObjectGetArgs getPArgs;
			getPArgs.From = { "path",wwisePath };
			getPArgs.Select = "parent";
			getPArgs.customReturnArgs.push_back("type");
			getPArgs.customReturnArgs.push_back("path");
			getPArgs.customReturnArgs.push_back("parent");

			AK::WwiseAuthoringAPI::AkJson::Array results;
			std::vector<WwiseObject> MyWwiseObjects;
			try {
				MyWwiseObjects = WwiseConnectionHnd->GetWwiseObjects(false, getPArgs, results);
			}
			catch (std::string e) {
				PrintToConsole(e);
			}

			parent = MyWwiseObjects[0];


			if (isVersion)
			{
				//Need to save off the details so we can replace the active source in the XML later
				activeSourceUpdateInfo thisSoundInfo;
				if (obj.properties.find("filePath") != obj.properties.end())
				{
#ifndef _WIN32
					thisSoundInfo.workUnitPath = PLATFORMHELPERS::cleanWwisePathsFromMac(obj.properties.at("filePath"));
#else
					thisSoundInfo.workUnitPath = obj.properties.at("filePath");
#endif // !_WIN32

					thisSoundInfo.parentSoundID = parent.properties.at("id");
					thisSoundInfo.newActiveSourceName = audioFile;

					size_t found = audioFile.find(".wav");
					if (found != audioFile.npos)
					{
						thisSoundInfo.newActiveSourceName.erase(thisSoundInfo.newActiveSourceName.length() - 4, 4);
					}

					std::vector sourcesForThisWU = activeSourcesUpdateMap[thisSoundInfo.workUnitPath];
					sourcesForThisWU.push_back(thisSoundInfo);
					activeSourcesUpdateMap[thisSoundInfo.workUnitPath] = sourcesForThisWU;
				}
				
			}


			fullPath.erase(0, fullPath.find("Originals\\"));

			if (fullPath.find("Originals\\Voices\\") != fullPath.npos)
			{
				fullPath.erase(0, 17);
			}
			if (fullPath.find("Originals\\SFX\\") != fullPath.npos)
			{
				fullPath.erase(0, 14);
			}

			size_t pos = fullPath.rfind(name);
			if (pos != fullPath.npos)
			{
				fullPath.erase(pos, audioFile.length());
				
			}
			pos = wwisePath.rfind(obj.properties.at("name"));
			if (pos != wwisePath.npos)
			{
				wwisePath.erase(pos, obj.properties.at("name").length());
			}
			

			existingOriginalDir = fullPath;
			existingWwisePath = wwisePath;


			
			
			return true;
		}
	}

	return false;
}


void CreateImportWindow::backupRenderQueFiles()
{
	std::string resourcePath = GetReaperResourcePath();
	std::filesystem::path backupPath = resourcePath + "/QueuedRenders/_backup";

	for (auto RenderJob : GlobalListOfRenderQueJobs)
	{
		RenderFilesBackup.push_back(RenderJob.RenderQueFilePath);
		std::filesystem::path source = RenderJob.RenderQueFilePath;
		std::filesystem::path target = backupPath / source.filename();
		std::error_code ec;

		try // If you want to avoid exception handling, then use the error code overload of the following functions.
		{
			std::filesystem::create_directories(backupPath,ec); // Recursively create target directory if not existing.
			std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing,ec);
		}
		catch (std::exception& e) // Not using fs::filesystem_error since std::bad_alloc can throw too.  
		{
			std::cout << e.what();
		}
	}

}

void CreateImportWindow::restoreRenderQueFiles()
{
	std::string resourcePath = GetReaperResourcePath();
	std::filesystem::path backupPath = resourcePath + "/QueuedRenders/_backup";
	std::filesystem::path restorePath = resourcePath + "/QueuedRenders";

	for (auto RenderQueFile : RenderFilesBackup)
	{
		std::filesystem::path source = RenderQueFile;
		std::filesystem::path sourceFile = source.filename();
		source = backupPath / sourceFile;
		std::filesystem::path target = restorePath / sourceFile;
		std::error_code ec;

		try // If you want to avoid exception handling, then use the error code overload of the following functions.
		{
			std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing,ec);
			remove(source,ec);
		}
		catch (std::exception& e) // Not using fs::filesystem_error since std::bad_alloc can throw too.  
		{
			std::cout << e.what();
		}
	}
}

void CreateImportWindow::SetWwiseAutomationMode(bool enable)
{
	WwiseConnectionHnd->SetWwiseAutomationMode(enable);
}

bool CreateImportWindow::IsAudioFileAVersion(std::string input, std::string& outVarientToken)
{
	std::string versionToken = myConfig.versionToken;
	bool isVersion = false;
	size_t lengthToRemove = versionToken.length();
	outVarientToken = "";

	//check if ".wav" in input
	std::size_t found = input.rfind(".wav");
	if (found != input.npos)
	{
		lengthToRemove += 4;
	}

	if (input.length() >= lengthToRemove)
	{
		size_t verTokenPos = input.length() - lengthToRemove;// last 8 chars of a version will be _v**.wav
		std::size_t found = input.find(versionToken.c_str(), verTokenPos, 2);//match the first 2 chars of the token (_v)
		if (found != input.npos)
		{
			int i = 0;
			bool doesPatternMatch = false;
			for (auto c : versionToken)
			{
				if ((isdigit(c)) != (isdigit(input[found + i])))
				{
					break;
				}
				i++;
			}
			if (i == versionToken.length())
			{
				doesPatternMatch = true;
			}
			if (input.length() > (found + versionToken.length()))
			{
				char c = input[found + versionToken.length()];
				if (c != '.')
				{
					doesPatternMatch = false;
				}
			}

			if (doesPatternMatch)
			{
				isVersion = true;
				outVarientToken = input.substr(verTokenPos, versionToken.length());
			}
		}

	}
	return isVersion;
}






void CreateImportWindow::FillRenderQueList()
{
	GlobalListOfRenderQueJobs.clear();
	std::vector<std::string> ListOfRenderQueFiles;
	ListOfRenderQueFiles = GetListOfRenderQues();

	for (auto RenderQueFile : ListOfRenderQueFiles)
	{
		RenderQueJob MyrenderQueJob = CreateRenderQueJobFromRenderQueFile(RenderQueFile);
		if (!MyrenderQueJob.RenderQueJobFileList.empty())
		{
			GlobalListOfRenderQueJobs.push_back(MyrenderQueJob);
		}
	}
}


void CreateImportWindow::SetStatusMessageText(std::string message)
{
	//SetDlgItemText(m_hWindow, IDC_Txt_Status, message.c_str());
	if (owningGUIWindow)
	{
		owningGUIWindow->setStatusText(message);
	}
	
	
}



void CreateImportWindow::OpenHelp()
{
	std::string help = "https://github.com/simongumbleton/SimonsReaperPlugin/raw/master/help/SimonsReaperPlugin_Help.docx";
	//PrintToConsole("Help wanted");
	//ShellExecute(NULL, "open", help.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

WwiseObject CreateImportWindow::GetWwiseObjectFromID(std::string guid)
{
	return WwiseConnectionHnd->GetWwiseObjectFromID(guid);
}

std::vector<WwiseObject> CreateImportWindow::GetWwiseObjectsByName(std::string objectName,std::string type, bool exactMatch, std::vector<std::string> returnProperties)
{
	ObjectGetArgs getArgs;
	getArgs.From = { "ofType",type };
	getArgs.Select = "";
	if (exactMatch)
	{
		getArgs.Where = { "name:matches", "^"+objectName+"$"};
	}
	else
	{
		getArgs.Where = { "name:contains",objectName };
	}

	for (auto property : returnProperties)
	{
		getArgs.customReturnArgs.push_back(property);
	}

	AK::WwiseAuthoringAPI::AkJson::Array results;
	std::vector<WwiseObject> MyWwiseObjects;
	try {
		return WwiseConnectionHnd->GetWwiseObjects(false, getArgs, results);
	}
	catch (std::string e) {
		//PrintToConsole(e);
	}
	return std::vector<WwiseObject>();
}

std::vector<WwiseObject> CreateImportWindow::GetTemplateWwiseObjects(std::string templateWwisePath) {
	std::vector<WwiseObject> templateObjects;
	if (templateWwisePath.empty())
	{
		return templateObjects;
	}
	if (!WwiseConnectionHnd)
	{
		return templateObjects;
	}
	if (!DoesTemplatePathExist(templateWwisePath))
	{
		return templateObjects;
	}
	ObjectGetArgs getArgs;
	getArgs.From = {"path",templateWwisePath};
	getArgs.Select = "children";
	getArgs.Where = { "type:isIn","Sound" };
	AK::WwiseAuthoringAPI::AkJson::Array Results;
	templateObjects = WwiseConnectionHnd->GetWwiseObjects(false, getArgs, Results);
	return templateObjects;
}

bool CreateImportWindow::DoesTemplatePathExist(std::string templateWwisePath) { 
	AK::WwiseAuthoringAPI::AkJson MoreRawReturnResults;
	ObjectGetArgs getArgs;
	getArgs.From = {"path",templateWwisePath};
	getArgs.Select = "";
	if (!waapi_GetObjectFromArgs(getArgs, MoreRawReturnResults))
	{
		return false;
	}
	return true;
}

void CreateImportWindow::SaveSettingsToExtState() {
	std::string name = EXTSTATE::transferSettingsName;// "CSGTransferSettings";
	std::stringstream valuesToJson;
	//"{ 'id': 1234, 'name': 'nandini' }"
	valuesToJson << '{';
	valuesToJson << "'" << TransferComponentSettings.waapiport << "'" << ",";
	valuesToJson << "'" << std::boolalpha << TransferComponentSettings.useAtomationMode << "'" << ",";
	valuesToJson << "'" << TransferComponentSettings.userorigsubdir << "'" << ",";
	valuesToJson << "'" << TransferComponentSettings.templatePath << "'" << ",";
	valuesToJson << "'" << TransferComponentSettings.versionToken << "'" << ",";
	valuesToJson << "'" << TransferComponentSettings.eEventCreationOption << "'" << ",";
	valuesToJson << "'" << TransferComponentSettings.eventWorkUnitSuffix << "'" << ",";
	valuesToJson << "'" << TransferComponentSettings.UserEventPath << "'" << ",";
	valuesToJson << "'" << TransferComponentSettings.eventMirroringDepth << "'" << ",";
	valuesToJson << "}";
	//saveProjExState("Transfer", valuesToJson.str(), name);
	saveGlobalExtState(name, valuesToJson.str(),true);
}


void CreateImportWindow::LoadSettingsFromExtState() {
	
	std::string svalue = "";
	std::vector<std::string> tempListValues;
	std::string name = EXTSTATE::transferSettingsName;// "CSGTransferSettings";
	//svalue = getProjExState("Transfer", "CSGTransferSettings");
	svalue = getGlobalExtState(name);
	
	if (svalue.empty()) {return;}
	
	char* pch;
	printf("Splitting string \"%s\" into tokens:\n", svalue.c_str());
	//char delims[] = "\n !@#$%^&*)(_+-=][}{|:;'<>?,./\"\\";
	char delims[] = "{,}";
	pch = strtok(&svalue[0], delims);
	
	while (pch != NULL)
	{
		printf("%s\n", pch);
		std::string value = std::string(pch);
		value.erase(std::remove(value.begin(), value.end(), '\''), value.end());


		tempListValues.push_back(value);
		pch = strtok(NULL, delims);
	}
	
	if (tempListValues.size() != 9)//number of settings in the struct
	{
		printf("Warning! Mismatch in number of settings retrived from extstate");
		return;
	}
	
	TransferComponentSettings.waapiport = std::stoi(tempListValues[0]);
	std::istringstream(tempListValues[1]) >> std::boolalpha >> TransferComponentSettings.useAtomationMode;
	TransferComponentSettings.userorigsubdir = tempListValues[2];
	TransferComponentSettings.templatePath = tempListValues[3];
	TransferComponentSettings.versionToken = tempListValues[4];
	TransferComponentSettings.eEventCreationOption = ETransferEventCreationOption(std::stoi(tempListValues[5]));
	TransferComponentSettings.eventWorkUnitSuffix = tempListValues[6];
	TransferComponentSettings.UserEventPath = tempListValues[7];
	TransferComponentSettings.eventMirroringDepth = std::stoi(tempListValues[8]);
}



void CreateImportWindow::UpdateSettings()
{

}

void CreateImportWindow::SaveRenderOutputFilesToProjExState(RenderQueJob& job)
{
	auto rProj = GetReaProjectFromProjectName(job.ParentReaperProject);
	if (rProj)
	{
		std::string name = "CSGTransferRenderedFiles";
		std::stringstream valuesToJson;
		//"{ 'id': 1234, 'name': 'nandini' }"
		std::unordered_map<std::string, float> keyValues = EXTSTATE::GetRenderOutputFilesFromProjExState(rProj);

		for (auto file : job.RenderQueJobFileList)
		{
			float timeRef = WAV::GetBWFTimecode_Seconds(file);// WIP
			auto fileName = PLATFORMHELPERS::filenameFromPathString(file);
			keyValues[fileName] = timeRef;
		}
		valuesToJson << '{';
		for (auto entry : keyValues)
		{
			valuesToJson << "'" << entry.first << "'";
			valuesToJson << ":";
			valuesToJson << "'" << entry.second << "'";
			valuesToJson << ",";
		}
		//PrintToConsole(valuesToJson.str());
		valuesToJson.seekp(-1, valuesToJson.cur); valuesToJson << "}";
		saveProjExState("RENDEREDFILES", valuesToJson.str(), name, rProj);
	}
}
