#pragma once
#include "WwiseConnectionHandler.h"
#include "ReaperRenderQueParser.h"
#include <filesystem>
#include <thread>
#include <iostream>
#include <chrono>
#include <mutex>
#include <future>

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

//int CreateImportWindow::m_hWindow = NULL;
long CreateImportWindow::m_lSaveThis = 0;

//std::vector<RenderQueJob> GlobalListOfRenderQueJobs;
//std::vector<std::string> RenderFilesBackup;

bool AllDone = false;
std::mutex mtx;
int numOfRendersDone = 0;

//=============================================================================

void CreateImportWindow::SetupPluginParent(WwiseConnectionHandler * parent)
{
	WwiseConnectionHnd = parent;
	thisPluginWindow = this;
}




CreateImportWindow::CreateImportWindow()
{
	WwiseConnectionHnd = new WwiseConnectionHandler();
	m_lSaveThis = (long)this;
	saveThis = this;
}

CreateImportWindow::~CreateImportWindow()
{
}


//=============================================================================
void CreateImportWindow::OnCommand(int id, int notifycode)
{
	//handles WM_COMMAND message of the modal dialogbox
	/*switch (id)
	{
	case ID_B_GO:        //RETURN key pressed or 'GO' button selected				 //MessageBox(NULL, "Ok buttor", _T("DEBUG"), MB_OK | MB_ICONEXCLAMATION);
		break;
	case IDC_B_ConnectWwise:
		handleUI_B_Connect();
		break;
	case IDC_B_CreateObject:
		handleUI_B_CreateObject();
		break;
	case IDC_B_GetSelectedParent:
		handleUI_B_GetSelectedParent();
		break;
	case IDC_C_Create_Type:
		handleUI_GetType(notifycode);
		break;
	case IDC_LIST_EventOptions:
		handleUI_GetImportEventOptions(notifycode);
		break;
	case IDC_C_CreateOnNameConflict:
		handleUI_GetNameConflict(notifycode);
		break;
	case IDC_B_RenderImport:
		handleUI_RenderImport();
		break;
	case ID_B_CANCEL:    //ESC key pressed or 'cancel' button selected
		SetWwiseAutomationMode(false);
		m_hWindow = NULL;
		//EndDialog(hwnd, id);
		break;
	case ID_B_OK:
		SetWwiseAutomationMode(false);
		m_hWindow = NULL;
		//EndDialog(hwnd, id);
		break;
	case IDC_B_RefreshTree:
		FillRenderQueList();
		break;
	case IDC_OrigsMatchWwise:
		GetOrigsDirMatchesWwise();
		break;
	case IDC_IsVoice:
		GetIsVoice();
		break;
	case IDC_B_Help:
		OpenHelp();
		break;
	}*/
}
//=============================================================================
void CreateImportWindow::OnInitDlg()
{

	config myConfig;
	ReadConfigFile(myConfig);
	WwiseConnectionHnd->SetOptionsFromConfig(myConfig);


	//init Wwise Connection
	handleUI_B_Connect();

	//Init options
	init_ALL_OPTIONS();
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
		SetStatusMessageText("Ready");
		//SetDlgItemText(m_hWindow, IDC_WwiseConnection, "Wwise Connection Established");
		//		SendMessage(hwnd_combo, CB_SETCURSEL, 0, 0);
		//		textConnectionStatus
	}
	else
	{
		SetStatusMessageText("Error");
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
		//create play event for the object we just created
		std::string id = results[0]["id"].GetVariant();
		std::string name = results[0]["name"].GetVariant();
		CreatePlayEventForID(id,name);
	}
	if (myCreateObjectArgs.Notes != "")
	{
		std::string id = results[0]["id"].GetVariant();
		std::string notes = myCreateObjectArgs.Notes;
		AK::WwiseAuthoringAPI::AkJson results;
		WwiseConnectionHnd->SetNotesForObject(id, notes, results);
	}

	SetStatusMessageText("Ready");

	//waapi_CreateObjectFromArgs(myCreateObjectArgs, results);
}

void CreateImportWindow::handleUI_B_GetSelectedParent()
{
	WwiseObject selectedParent = WwiseConnectionHnd->GetSelectedObject();
	std::string parID = selectedParent.properties["id"];
	std::string parNameType = selectedParent.properties["name"] + " (" + selectedParent.properties["type"] + ")";
	//SetDlgItemText(m_hWindow, IDC_ImportParent_ID, parID.c_str());
	//SetDlgItemText(m_hWindow, IDC_ImportParent_NameType, parNameType.c_str());

	HandleUI_SetParentForRenderJob(selectedParent);

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
	
	while (!AllDone)
	{
		
		if (numOfRendersDone == numJobs) {
			AllDone = true;
			//PrintToConsole("ALL Renders complete");
			return true;
		}
/*
		int jobIndex = 0;
		for (auto job : GlobalListOfRenderQueJobs)
		{
			if (!GlobalListOfRenderQueJobs[jobIndex].hasRendered)
			{
				WIN32_FIND_DATA FindFileData;
				HANDLE hFind = FindFirstFile(job.RenderQueFilePath.c_str(), &FindFileData);

				if (hFind == INVALID_HANDLE_VALUE)
				{
					if (GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						if (!GlobalListOfRenderQueJobs[jobIndex].hasRendered)
						{
							PostMessage(tr_Progress_Import, PBM_STEPIT, 0, 0);
							//SendMessage(tr_Progress_Import, PBM_STEPIT, 0, 0);

							GlobalListOfRenderQueJobs[jobIndex].hasRendered = true;
							numOfRendersDone++;
						}
					}
				}
				jobIndex++;
			}
		
		}
*/
	}
	
	return false;;
}



void CreateImportWindow::handleUI_RenderImport()
{
	/// Render and import from que list

	//For each job in the global render que
	//check it has valid wwise parent and report errors/exit if not

	for (auto job : GlobalListOfRenderQueJobs)
	{
		if (job.parentWwiseObject.properties.size() == 0 && job.hasPerFileOverrides==false)
		{
			//PrintToConsole("  ERROR! A render job has no import settings!   \n");
			//PrintToConsole(job.RenderQueFilePath);
			SetStatusMessageText("Error");
			return;
		}

	}

	SetStatusMessageText("Rendering from Reaper Render Que");

	int numJobs = GlobalListOfRenderQueJobs.size();

	//backup render que files in case of errors
	backupRenderQueFiles();

	bool startedRender = false;

	//SendMessage(tr_Progress_Import, PBM_SETRANGE, 0, MAKELPARAM(0, numJobs));

	//SendMessage(tr_Progress_Import, PBM_SETSTEP, (WPARAM)1, 0);

	std::future<bool> fut = std::async(std::launch::async,&CreateImportWindow::UpdateProgressDuringRender,this,numJobs);

	ReaperRenderObj renderObj;
	renderObj.RenderAllQues();

	//PrintToConsole("Render done. Waiting for second thread");
	//SendMessage(tr_Progress_Import, PBM_SETPOS, numJobs, 0);

	std::future_status status;

	status = fut.wait_for(std::chrono::seconds(4));
	//bool ret = fut.get();

	//PrintToConsole("Rejoined main");

	//MSG msg;	//Clears the message que for the progress bar
	//PeekMessage(&msg,tr_Progress_Import,0,0,PM_REMOVE);
	
	if (status == std::future_status::ready)
	{
		SetStatusMessageText("Importing into Wwise");
		//SendMessage(tr_Progress_Import, PBM_SETPOS, 0, 0);
		
		if (!ImportJobsIntoWwise())
		{
			// something went wrong, restore render que files
			//PrintToConsole("Something went wrong, restoring Reaper Render Que files.....");
			restoreRenderQueFiles();
			return;

		}

		FillRenderQueList();
		return;
	}
	else if (status == std::future_status::timeout)
	{
		//PrintToConsole("Timeout error. Something went wrong in Render. Reaper did not remove all render que files after processing.");
		SetStatusMessageText("Error");
		restoreRenderQueFiles();
		return;
	}
	

}



bool CreateImportWindow::ImportJobsIntoWwise()
{
	SetStatusMessageText("Importing into Wwise");

	int jobIndex = 0;
	int importSuccesses = 0;
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

					std::string file = fileOverride.second.RenderJobFile;	// file is just the filename here

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

					if (AudioFileExistsInWwise(file,fileOverride.second.parentWwiseObject, existingOriginalsPath, existingWwisePath))
					{
						//audio file already exists under this parent, so replace the originals path
					//If file is not SFX (is localised) then need to strip out the top level language dir from the existing originals path, as it is handled by the import Language
						if (job.ImportLanguage != "SFX")
						{

							if (existingOriginalsPath.find("\\") != existingOriginalsPath.npos)
							{
								existingOriginalsPath.erase(0, existingOriginalsPath.find("\\") + 1);
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
						fileOverride.second.isVoice,
						fileOverride.second.ImportLanguage,
						fileOverride.second.OrigDirMatchesWwise,
						fileOverride.second.userOrigsSubDir,
						audiofile,
						fileOverride.second.createEventOption
						
					);
					if (ImportCurrentRenderJob(curFileOverrideImportArgs))
					{
						fileOverride.second.hasImported = true;
					}

				}

			}


			// import the remaining files from the job that are not overrideen

			for (auto file : job.RenderQueJobFileList)
			{
				std::string existingOriginalsPath = "";
				std::string existingWwisePath = "";
				// file is the full path in this context, need to get the just the filename
				std::string fullPath = file;
				if (file.rfind("\\") != file.npos)
				{
					file.erase(0, file.rfind("\\")+1);
				}

				WwiseObject originalJobWwiseParent = job.parentWwiseObject;

				if (AudioFileExistsInWwise(file, originalJobWwiseParent, existingOriginalsPath, existingWwisePath))
				{
					//audio file already exists under this parent, so replace the originals path
					//If file is not SFX (is localised) then need to strip out the top level language dir from the existing originals path, as it is handled by the import Language
					if (job.ImportLanguage != "SFX")
					{
					
						if (existingOriginalsPath.find("\\") != existingOriginalsPath.npos)
						{
							existingOriginalsPath.erase(0, existingOriginalsPath.find("\\") + 1);
						}

					}
					std::vector<std::string> importfiles;
					importfiles.push_back(fullPath);

					ImportObjectArgs curJobImportArgs = SetupImportArgs
					(
						originalJobWwiseParent,
						job.isVoice,
						job.ImportLanguage,
						false,
						existingOriginalsPath,
						importfiles,
						job.createEventOption
					);
					if (ImportCurrentRenderJob(curJobImportArgs))
					{
						for (auto &renderJobFile : job.RenderQueJobFileList)
						{
							if (renderJobFile == fullPath)
							{
								renderJobFile = "";
							}
						}
					}

				}

			}

			ImportObjectArgs curJobImportArgs = SetupImportArgs
			(
				job.parentWwiseObject,
				job.isVoice,
				job.ImportLanguage,
				job.OrigDirMatchesWwise,
				job.userOrigsSubDir,
				job.RenderQueJobFileList,
				job.createEventOption
			);
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
				//SendMessage(tr_Progress_Import, PBM_SETPOS, importSuccesses, 0);
			}
			
		}
		jobIndex++;
	}

	if (importSuccesses == GlobalListOfRenderQueJobs.size())
	{
		//PrintToConsole("All jobs imported successfully");
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

void CreateImportWindow::CreatePlayEventForID(std::string id, std::string name)
{
	CreateObjectArgs args;
	args.ParentID = "\\Events\\Default Work Unit";
	args.Type = "Event";
	args.Name = "Play_"+name;
	args.createPlayEvent = true;

	EventCreateArgs eventArgs;
	eventArgs.target = id;
	
	args.eventArgs = eventArgs;


	AK::WwiseAuthoringAPI::AkJson::Array results;
	if (!WwiseConnectionHnd->CreateWwiseObjects(false, args, results))
	{
		//PrintToConsole("Error creating Play event");
	}

}

ImportObjectArgs CreateImportWindow::SetupImportArgs(WwiseObject parent, bool isVoice, std::string ImportLanguage, bool OrigsDirMatchesWwise, std::string userOrigSubDir,std::vector<std::string> ImportFiles,std::string eventCreateOption)
{
	std::string originalsPath = parent.properties["path"];
	std::string remove = "\\Actor-Mixer Hierarchy";
	originalsPath.erase(0, remove.length());

	ImportObjectArgs importArgs;
	importArgs.ImportLocation = parent.properties["path"];
	importArgs.ImportLanguage = ImportLanguage;
	if (isVoice)
	{
		importArgs.objectType = "<Sound Voice>";
	}
	else
	{
		importArgs.objectType = "<Sound SFX>";
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
			if (type == "AudioFileSource") { continue; }

			CreatePlayEventForID(obj["id"].GetVariant(), obj["name"].GetVariant());
		}
	}
	else if (curJobImportArgs.eventCreateOption == 2)
	{
		// \Actor-Mixer Hierarchy\Default Work Unit\MyAM\AnotherAM\New Random Container
		std::string target = curJobImportArgs.ImportLocation;
		std::string name = curJobImportArgs.ImportLocation.erase(0,curJobImportArgs.ImportLocation.rfind("\\")+1);
		CreatePlayEventForID(target, name);
	}

	return success;
}

bool CreateImportWindow::AudioFileExistsInWwise(std::string audioFile, WwiseObject& parent, std::string& existingOriginalDir, std::string& existingWwisePath)
{
	ObjectGetArgs getArgs;
	std::string id = parent.properties["id"];
	getArgs.From = { "id",id };
	getArgs.Select = "descendants";
	getArgs.Where = { "type:isIn","Sound" };
	getArgs.customReturnArgs.push_back("sound:originalWavFilePath"); 
	getArgs.customReturnArgs.push_back("path");

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
		if (name == audioFile)
		{
			std::string wwisePath = obj.properties.at("path");
			std::string fullPath = obj.properties.at("sound:originalWavFilePath");


			ObjectGetArgs getPArgs;
			getPArgs.From = { "path",wwisePath };
			getPArgs.Select = "parent";
			getPArgs.customReturnArgs.push_back("type");
			getPArgs.customReturnArgs.push_back("path");

			AK::WwiseAuthoringAPI::AkJson::Array results;
			std::vector<WwiseObject> MyWwiseObjects;
			try {
				MyWwiseObjects = WwiseConnectionHnd->GetWwiseObjects(false, getPArgs, results);
			}
			catch (std::string e) {
				//PrintToConsole(e);
			}

			parent = MyWwiseObjects[0];





			fullPath.erase(0, fullPath.find("Originals\\"));

			if (fullPath.find("Originals\\Voices\\") != fullPath.npos)
			{
				fullPath.erase(0, 17);
			}
			if (fullPath.find("Originals\\SFX\\") != fullPath.npos)
			{
				fullPath.erase(0, 14);
			}

			size_t pos = fullPath.find(audioFile);
			fullPath.erase(pos, audioFile.length());

			pos = wwisePath.find(obj.properties.at("name"));
			wwisePath.erase(pos, obj.properties.at("name").length());

			existingOriginalDir = fullPath;
			existingWwisePath = wwisePath;


			
			
			return true;
		}
	}

	return false;
}


void CreateImportWindow::backupRenderQueFiles()
{
	std::string resourcePath = "";// GetReaperResourcePath();
	std::filesystem::path backupPath = resourcePath + "\\QueuedRenders\\_backup";

	for (auto RenderJob : GlobalListOfRenderQueJobs)
	{
		RenderFilesBackup.push_back(RenderJob.RenderQueFilePath);
		std::filesystem::path source = RenderJob.RenderQueFilePath;
		std::filesystem::path target = backupPath / source.filename();

		try // If you want to avoid exception handling, then use the error code overload of the following functions.
		{
			std::filesystem::create_directories(backupPath); // Recursively create target directory if not existing.
			std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing);
		}
		catch (std::exception& e) // Not using fs::filesystem_error since std::bad_alloc can throw too.  
		{
			std::cout << e.what();
		}
	}

}

void CreateImportWindow::restoreRenderQueFiles()
{
	std::string resourcePath = "";//GetReaperResourcePath();
	std::filesystem::path backupPath = resourcePath + "\\QueuedRenders\\_backup";
	std::filesystem::path restorePath = resourcePath + "\\QueuedRenders";

	for (auto RenderQueFile : RenderFilesBackup)
	{
		std::filesystem::path source = RenderQueFile;
		std::filesystem::path sourceFile = source.filename();
		source = backupPath / sourceFile;
		std::filesystem::path target = restorePath / sourceFile;

		try // If you want to avoid exception handling, then use the error code overload of the following functions.
		{
			std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing);
			remove(source);
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



/// INIT ALL OPTIONS

bool CreateImportWindow::init_ALL_OPTIONS()
{/*
	tr_buttonConnect = GetDlgItem(hwnd, IDC_B_ConnectWwise);
	tr_textConnectionStatus = GetDlgItem(hwnd, IDC_WwiseConnection);
	tr_B_GetSelectedParent = GetDlgItem(hwnd, IDC_B_GetSelectedParent);
	tr_s_ImportParentID = GetDlgItem(hwnd, IDC_ImportParent_ID);
	tr_s_ImportParentNameType = GetDlgItem(hwnd, IDC_ImportParent_NameType);
	tr_B_CreateObject = GetDlgItem(hwnd, IDC_B_CreateObject);
	tr_c_CreateType = GetDlgItem(hwnd, IDC_C_Create_Type);
	tr_c_CreateNameConflict = GetDlgItem(hwnd, IDC_C_CreateOnNameConflict);
	tr_txt_CreateName = GetDlgItem(hwnd, IDC_text_CreateName);
	tr_txt_CreateNotes = GetDlgItem(hwnd, IDC_Text_CreateNotes);
	tr_Tree_RenderJobTree = GetDlgItem(hwnd, IDC_TREE_RenderJobTree);

	UINT mask = TVS_EX_MULTISELECT;
	DWORD dWord = TVS_EX_MULTISELECT;
	TreeView_SetExtendedStyle(tr_Tree_RenderJobTree, dWord, mask);

	tr_Progress_Import = GetDlgItem(hwnd, IDC_PROGRESS_Import);
	B_RenderImport = GetDlgItem(hwnd, IDC_B_RenderImport);
	txt_status = GetDlgItem(hwnd, IDC_Txt_Status);
	txt_Language = GetDlgItem(hwnd, IDC_Language);
	//Edit_SetText(txt_Language, defaultLanguage.c_str());
	check_IsVoice = GetDlgItem(hwnd, IDC_IsVoice);
	SendDlgItemMessage(m_hWindow, IDC_IsVoice, BM_SETCHECK, BST_UNCHECKED, 0);
	check_OrigDirMatchWwise = GetDlgItem(hwnd, IDC_OrigsMatchWwise);
	SendDlgItemMessage(m_hWindow, IDC_OrigsMatchWwise, BM_SETCHECK, BST_CHECKED, 0);
	B_RefreshTree = GetDlgItem(hwnd, IDC_B_RefreshTree);
	txt_OriginalsSubDir = GetDlgItem(hwnd, IDC_txt_OrigsDir);
	Edit_SetText(txt_OriginalsSubDir, "ImportedFromReaper/");
	check_CreateEvent = GetDlgItem(hwnd, IDC_Create_Event);
	l_eventOptions = GetDlgItem(hwnd, IDC_LIST_EventOptions);

	init_ComboBox_A(tr_c_CreateType, myCreateChoices.waapiCREATEchoices_TYPE);
	init_ComboBox_A(tr_c_CreateNameConflict, myCreateChoices.waapiCREATEchoices_NAMECONFLICT);
	init_ComboBox_A(l_eventOptions, myCreateChoices.waapiCREATEchoices_EVENTOPTIONS);

	init_ComboBox_A(
		txt_Language,
		parentWwiseConnectionHnd->MyCurrentWwiseConnection.projectGlobals.Languages
	);
*/
	GetOrigsDirMatchesWwise();
	GetIsVoice();

	FillRenderQueList();

	SetStatusMessageText("Ready");

	return true;
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

void CreateImportWindow::UpdateRenderJob_TreeView()
{
	FillRenderQueList();
	
	//TreeView_DeleteAllItems(tr_Tree_RenderJobTree);
/*
	for (auto RenderJob : GlobalListOfRenderQueJobs)
	{
		std::filesystem::path filePath = RenderJob.RenderQueFilePath;
		std::string filename = filePath.filename().string();

		TV_INSERTSTRUCT tvInsert;
		HTREEITEM Parent;
		HTREEITEM Child;

		tvInsert.hParent = NULL;
		tvInsert.hInsertAfter = TVI_ROOT;
		tvInsert.item.mask = TVIF_TEXT;	// tvinsert.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		tvInsert.item.pszText = &filename[0];	//(LPARAM)choice.c_str()
		Parent = (HTREEITEM)SendDlgItemMessage(m_hWindow, IDC_TREE_RenderJobTree, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);

		//Children = WAV files in this render que job
		for (auto renderFile : RenderJob.RenderQueJobFileList)
		{
			std::filesystem::path filePath = renderFile;
			std::string filename = filePath.filename().string();
			tvInsert.hParent = Parent;
			tvInsert.hInsertAfter = TVI_LAST;
			tvInsert.item.pszText = &filename[0];
			Child = (HTREEITEM)SendDlgItemMessage(m_hWindow, IDC_TREE_RenderJobTree, TVM_INSERTITEM, 0, (LPARAM)&tvInsert);
		}


	}
*/
}


void CreateImportWindow::HandleUI_SetParentForRenderJob(WwiseObject selectedParent)
{
	std::string parentWwiseID = selectedParent.properties["id"];
	std::string parentWwiseName = selectedParent.properties["name"];
	std::string parentWwiseType = selectedParent.properties["type"];
	std::string parentWwisePath = selectedParent.properties["path"];
	
/*
	TVITEM item;
	TVITEM parentItem;

	if (parentWwiseID == "")
	{
		PrintToConsole("Error - No valid Wwise Parent");
		return;	// Invalid wwise parent selection!
	}

	if (parentWwisePath.find("Actor-Mixer Hierarchy\\") == parentWwisePath.npos)
	{
		PrintToConsole("Import parent must be in Actor-Mixer hierarchy");
		return;	// Invalid wwise parent selection!
	}

	// if type is Work Unit - Need to check if it's a folder

	int selectedcount = TreeView_GetSelectedCount(tr_Tree_RenderJobTree);

	HTREEITEM hSelectedItem = TreeView_GetSelection(tr_Tree_RenderJobTree);
	if (hSelectedItem == NULL) // Nothing selected
	{
		PrintToConsole("No render job or file selected");
		return;
	}
	std::vector<HTREEITEM> selectedItems;
	selectedItems.push_back(hSelectedItem);

	if (selectedcount > 1)
	{	
		for (int i = 1; i < selectedcount; i++)
		{
			HTREEITEM nextSelected = TreeView_GetNextSelected(tr_Tree_RenderJobTree, hSelectedItem);
			selectedItems.push_back(nextSelected);
			hSelectedItem = nextSelected;
		}
	}

	for (auto &selectedItem : selectedItems)
	{

		TCHAR buffer[256];
		item.hItem = selectedItem;
		item.mask = TVIF_TEXT | TVIF_CHILDREN;
		item.cchTextMax = 256;
		item.pszText = buffer;
		bool isItemWav = false;
		if (TreeView_GetItem(tr_Tree_RenderJobTree, &item))
		{
			/// need to clean the nammes IF they contain % (they have been double set)

			std::string curName = item.pszText;
			size_t pos = curName.find("%");
			if (pos != curName.npos)
			{
				curName.erase(curName.begin(), curName.begin() + pos+1);
				std::string newItemName = curName;
				//item.mask = TVIF_TEXT;
				item.pszText = &newItemName[0];
				TreeView_SetItem(tr_Tree_RenderJobTree, &item);
			}
			
			TreeView_GetItem(tr_Tree_RenderJobTree, &item);

			if (item.cChildren != 1)
			{
				// Check if selected thing is wav?
				// Support overriding Wwise imports for child wavs of a render job?
				std::string itemName = item.pszText;
				if (itemName.find(".wav") != itemName.npos)
				{
					//Selected thing is a wav file! This render job has overrides

					isItemWav = true;


				}
				else {
					PrintToConsole("Render Job selected has no children");
					return;

				}

			}
			//Find the matching Render Que Job that we selected OR find the parent job of the selected wav
			if (isItemWav) {

				HTREEITEM hparentItem = TreeView_GetParent(tr_Tree_RenderJobTree, item.hItem);
				if (hparentItem == NULL) // Nothing selected
				{
					return;
				}
				TCHAR buffer[256];
				parentItem.hItem = hparentItem;
				parentItem.mask = TVIF_TEXT;
				parentItem.cchTextMax = 256;
				parentItem.pszText = buffer;
				if (TreeView_GetItem(tr_Tree_RenderJobTree, &parentItem))
				{
					std::string itemName = parentItem.pszText;
				}


			}
			std::string jobName = "";
			if (isItemWav) {
				jobName = parentItem.pszText;
			}
			else {
				jobName = item.pszText;
			}
			int count = 0;
			for (auto &renderJob : GlobalListOfRenderQueJobs)
			{
				std::filesystem::path filePath = renderJob.RenderQueFilePath;
				std::string filename = filePath.filename().string();

				if (jobName.find(filename) != jobName.npos)
				{
					//Found a match
					//PrintToConsole("Found a match");

					if (isItemWav) {
						renderJob.hasPerFileOverrides = true;
						RenderJobFileOverride fileOverride;
						fileOverride.RenderJobFile = item.pszText;
						fileOverride.parentWwiseObject = selectedParent;
						fileOverride.isVoice = GetIsVoice();
						fileOverride.createEventOption = GetImportEventOption();
						fileOverride.OrigDirMatchesWwise = GetOrigsDirMatchesWwise();
						if (!fileOverride.OrigDirMatchesWwise)
						{
							fileOverride.userOrigsSubDir = GetUserOriginalsSubDir();
						}

						std::string language;
						if (fileOverride.isVoice)
						{
							fileOverride.ImportLanguage = GetLanguage();
							language = fileOverride.ImportLanguage;
						}
						else
						{
							fileOverride.ImportLanguage = "SFX";
							language = "SFX";
						}

						renderJob.perFileOverridesmap[fileOverride.RenderJobFile] = fileOverride;


						//Set the display Text to include wwise parent name and type
						std::string newItemName = parentWwiseName + "(" + parentWwiseType + ")  - " + "%" + fileOverride.RenderJobFile;
						item.mask = TVIF_TEXT;
						item.pszText = &newItemName[0];
						TreeView_SetItem(tr_Tree_RenderJobTree, &item);
						PrintToConsole(fileOverride.RenderJobFile + " Imports into " + renderJob.parentWwiseObject.properties["name"]);

					}
					else {

						renderJob.parentWwiseObject = selectedParent;

						renderJob.isVoice = GetIsVoice();

						renderJob.createEventOption = GetImportEventOption();

						renderJob.OrigDirMatchesWwise = GetOrigsDirMatchesWwise();

						if (!renderJob.OrigDirMatchesWwise)
						{
							renderJob.userOrigsSubDir = GetUserOriginalsSubDir();
						}

						std::string language;
						if (renderJob.isVoice)
						{
							renderJob.ImportLanguage = GetLanguage();
							language = renderJob.ImportLanguage;
						}
						else
						{
							renderJob.ImportLanguage = "SFX";
							language = "SFX";
						}
						//Set the display Text to include wwise parent name and type
						std::string newItemName = parentWwiseName + "(" + parentWwiseType + " : " + ")  - " + "%" + filename;
						item.mask = TVIF_TEXT;
						item.pszText = &newItemName[0];
						TreeView_SetItem(tr_Tree_RenderJobTree, &item);
						PrintToConsole(renderJob.RenderQueFilePath + " Imports into " + renderJob.parentWwiseObject.properties["name"]);
					}

				}
				count++;
			}

		}
	}


	for (auto &selectedItem : selectedItems)
	{
		TVITEM clearitem;
		clearitem.hItem = selectedItem;
		clearitem.stateMask = TVIS_SELECTED;
		clearitem.state = 0;
		if (TreeView_GetItem(tr_Tree_RenderJobTree, &item))
		{
			TreeView_SetItemState(tr_Tree_RenderJobTree, clearitem.hItem, clearitem.state, TVIS_SELECTED);
		}

	}
	TreeView_SelectItem(tr_Tree_RenderJobTree, NULL);

	SetStatusMessageText("Ready");
*/
}

bool CreateImportWindow::GetIsVoice()
{

	if (0)//SendDlgItemMessage(m_hWindow, IDC_IsVoice, BM_GETCHECK, 0, 0))
	{
		//Edit_Enable(txt_Language, true);
		return true;
	}
	else
	{
		//Edit_Enable(txt_Language, false);
		return false;
	}
}
bool CreateImportWindow::GetCreateEvent()
{

	if (0)//SendDlgItemMessage(m_hWindow, IDC_Create_Event, BM_GETCHECK, 0, 0))
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::string CreateImportWindow::GetLanguage()
{
	///Get the par ID text
	char buffer[256];
	//GetDlgItemTextA(m_hWindow, IDC_Language, buffer, 256);
	int x = 0;//SendMessage(txt_Language, CB_GETCURSEL, 0, 0);
	std::string lang = WwiseConnectionHnd->MyCurrentWwiseConnection.projectGlobals.Languages[x];
	return lang;
}

std::string CreateImportWindow::GetImportEventOption()
{
	int x = 0;//SendMessage(l_eventOptions, CB_GETCURSEL, 0, 0);
	return myCreateChoices.waapiCREATEchoices_EVENTOPTIONS[x];
}

bool CreateImportWindow::GetOrigsDirMatchesWwise()
{
	if (0)//SendDlgItemMessage(m_hWindow, IDC_OrigsMatchWwise, BM_GETCHECK, 0, 0))
	{
		//IDC_txt_OrigsDir
		//Edit_Enable(txt_OriginalsSubDir, false);
		return true;
	}
	else
	{
		//Edit_Enable(txt_OriginalsSubDir, true);
		return false;
	}
}

std::string CreateImportWindow::GetUserOriginalsSubDir()
{
	char buffer[256];

	//GetDlgItemTextA(m_hWindow, IDC_txt_OrigsDir, buffer, 256);
	std::string userOrigsDir = buffer;
	return userOrigsDir;
}

void CreateImportWindow::SetStatusMessageText(std::string message)
{
	//SetDlgItemText(m_hWindow, IDC_Txt_Status, message.c_str());
}


void CreateImportWindow::OpenHelp()
{
	std::string help = "https://github.com/simongumbleton/SimonsReaperPlugin/raw/master/help/SimonsReaperPlugin_Help.docx";
	//PrintToConsole("Help wanted");
	//ShellExecute(NULL, "open", help.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
