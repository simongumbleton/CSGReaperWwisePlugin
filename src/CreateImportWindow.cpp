
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
}

CreateImportWindow::~CreateImportWindow()
{
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
	WwiseConnectionHnd->SetOptionsFromConfig(myConfig);

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


	AK::WwiseAuthoringAPI::AkJson::Array results;
	if (!WwiseConnectionHnd->CreateWwiseObjects(false, myCreateObjectArgs, results))
	{
		//ERROR
		SetStatusMessageText("Error");
		return;
	}
	if (myCreateObjectArgs.createPlayEvent)
	{
		if (myCreateObjectArgs.Type != "ActorMixer")
		{
			//create play event for the object we just created
			std::string id = results[0]["id"].GetVariant();
			std::string name = results[0]["name"].GetVariant();
			CreatePlayEventForID(id, name);
		}
	}
	if (myCreateObjectArgs.Notes != "")
	{
		std::string id = results[0]["id"].GetVariant();
		std::string notes = myCreateObjectArgs.Notes;
		AK::WwiseAuthoringAPI::AkJson results;
		WwiseConnectionHnd->SetNotesForObject(id, notes, results);
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
					if (file.rfind(kPathSeparator) != file.npos)
					{
						file.erase(0, file.rfind(kPathSeparator)+1);
					}

					if (AudioFileExistsInWwise(file,fileOverride.second.parentWwiseObject, existingOriginalsPath, existingWwisePath))
					{
						//audio file already exists under this parent, so replace the originals path
					//If file is not SFX (is localised) then need to strip out the top level language dir from the existing originals path, as it is handled by the import Language
						if (job.ImportLanguage != "SFX")
						{
							
							if (existingOriginalsPath.find(kPathSeparator) != existingOriginalsPath.npos)
							{
								existingOriginalsPath.erase(0, existingOriginalsPath.find(kPathSeparator) + 1);
							}

						}
						fileOverride.second.OrigDirMatchesWwise = false;
						fileOverride.second.userOrigsSubDir = existingOriginalsPath;
					}



					std::vector<std::string> audiofile;
					audiofile.push_back(fileOverride.second.RenderJobFile);

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
						filenameFromPathString(job.ParentReaperProject)
						
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
				if (file.rfind(kPathSeparator) != file.npos)
				{
					file.erase(0, file.rfind(kPathSeparator)+1);
				}

				WwiseObject originalJobWwiseParent = job.parentWwiseObject;

				if (AudioFileExistsInWwise(file, originalJobWwiseParent, existingOriginalsPath, existingWwisePath))
				{
					//audio file already exists under this parent, so replace the originals path
					//If file is not SFX (is localised) then need to strip out the top level language dir from the existing originals path, as it is handled by the import Language
					if (job.ImportLanguage != "SFX")
					{
					
						if (existingOriginalsPath.find(kPathSeparator) != existingOriginalsPath.npos)
						{
							existingOriginalsPath.erase(0, existingOriginalsPath.find(kPathSeparator) + 1);
						}

					}
					//store the existing originals path, we might use it if we add extra files into this parent
					existingSiblingOriginalsPath = existingOriginalsPath;
					std::vector<std::string> importfiles;
					importfiles.push_back(fullPath);

					ImportObjectArgs curJobImportArgs = SetupImportArgs
					(
						originalJobWwiseParent,
						job.EimportType,
						job.ImportLanguage,
						false,
						existingOriginalsPath,
						importfiles,
						job.createEventOption,
						filenameFromPathString(job.ParentReaperProject)
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
				filenameFromPathString(job.ParentReaperProject)
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

void CreateImportWindow::CreatePlayEventForID(std::string id, std::string name,std::string notes,std::string path)
{
	//std::string remove = "\\Actor-Mixer Hierarchy";
	//path.erase(0, remove.length());
	CreateObjectArgs args;
	args.ParentID = stringReplace(path, "\\Actor-Mixer Hierarchy", "\\Events");;
	args.Type = "Event";
	args.Name = "Play_"+name;
	args.createPlayEvent = true;
	args.Notes = notes;

	EventCreateArgs eventArgs;
	eventArgs.target = id;
	
	args.eventArgs = eventArgs;


	AK::WwiseAuthoringAPI::AkJson::Array results;
	if (!WwiseConnectionHnd->CreateWwiseObjects(false, args, results))
	{
		//PrintToConsole("Error creating Play event");
		return;
	}
	if (owningGUIWindow)
	{
		TransferToWwiseComponent* parentAsTransferComponent = dynamic_cast<TransferToWwiseComponent*>(owningGUIWindow);
		if (parentAsTransferComponent)
		{
			parentAsTransferComponent->AddEventToSaveList(args.Name);
		}
	}
}

ImportObjectArgs CreateImportWindow::SetupImportArgs(WwiseObject parent, IMPORT_TYPE importType, std::string ImportLanguage,
													 bool OrigsDirMatchesWwise,
													 std::string userOrigSubDir,
													 std::vector<std::string> ImportFiles,
													 std::string eventCreateOption,
													 std::string SourceReaperProj,
													 std::string Notes
													 )
{
	std::string originalsPath = parent.properties["path"];
	std::string remove = "\\Actor-Mixer Hierarchy";
	originalsPath.erase(0, remove.length());

	ImportObjectArgs importArgs;
	importArgs.Notes = Notes;
	importArgs.SourceReaperProject = SourceReaperProj;
	importArgs.ImportLocation = parent.properties["path"];
	importArgs.ImportLanguage = ImportLanguage;
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
	if (eventCreateOption == "Play@Children") // "None", "Play@Children" , "Play@Parent"
	{
		importArgs.eventCreateOption = 1;
	}
	else if (eventCreateOption == "Play@Parent")
	{
		importArgs.eventCreateOption = 2;
	}
	else importArgs.eventCreateOption = 0;


	return importArgs;
}

bool CreateImportWindow::ImportCurrentRenderJob(ImportObjectArgs curJobImportArgs)
{
	bool success;
	AK::WwiseAuthoringAPI::AkJson::Array results;
	success = WwiseConnectionHnd->ImportAudioToWwise(false, curJobImportArgs, results);
	
	if (curJobImportArgs.eventCreateOption == 1)
	{
		for (auto obj : results)
		{
			std::string type = obj["type"].GetVariant();
			std::string name = obj["name"].GetVariant();
			if (type == "AudioFileSource") { continue; }
			std::string notes = "rpp:" + filenameFromPathString(curJobImportArgs.SourceReaperProject) + "\nNotes:";
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

			CreatePlayEventForID(obj["id"].GetVariant(), name, notes);
		}
	}
	else if (curJobImportArgs.eventCreateOption == 2)
	{
		// \Actor-Mixer Hierarchy\Default Work Unit\MyAM\AnotherAM\New Random Container
		std::string target = curJobImportArgs.ImportLocation;
		std::string name = curJobImportArgs.ImportLocation.erase(0,curJobImportArgs.ImportLocation.rfind("\\")+1);
		std::string notes = "rpp:" + filenameFromPathString(curJobImportArgs.SourceReaperProject) + "\nNotes:";
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
					thisSoundInfo.workUnitPath = cleanWwisePathsFromMac(obj.properties.at("filePath"));
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
	ObjectGetArgs getArgs;
	getArgs.From = { "id",guid };
	getArgs.Select = "";
	getArgs.customReturnArgs.push_back("path");
	getArgs.customReturnArgs.push_back("workunit");
	getArgs.customReturnArgs.push_back("filePath");
	getArgs.customReturnArgs.push_back("parent");

	AK::WwiseAuthoringAPI::AkJson::Array results;
	std::vector<WwiseObject> MyWwiseObjects;
	try {
		MyWwiseObjects = WwiseConnectionHnd->GetWwiseObjects(false, getArgs, results);
	}
	catch (std::string e) {
		//PrintToConsole(e);
	}
	if (MyWwiseObjects.empty())
	{
		return WwiseObject();
	}
	else
	{
		return MyWwiseObjects[0];
	}
}

std::vector<WwiseObject> CreateImportWindow::GetWwiseObjectsByName(std::string objectName,std::string type)
{
	ObjectGetArgs getArgs;
	getArgs.From = { "ofType",type };
	getArgs.Select = "";
	getArgs.Where = { "name:contains",objectName};

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



