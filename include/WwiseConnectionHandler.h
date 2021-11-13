#pragma once
//#include <windows.h>
//#include <windowsx.h>
//#include <tchar.h>    //string and other mapping macros
#include <string>
#include <map>
#include "ConfigFileHandler.h"
#include "waapi_structs.h"
#include "GUI.h"
#include "WaapiFunctions.h"
//#include "PluginWindow.h"
#include <map>
#include <vector>

class BaseWwiseGuiComponent;


class WwiseConnectionHandler
{	
public:
	
	static std::vector<BaseWwiseGuiComponent*> ActiveComponents;
	
	static void AddActiveComponent(BaseWwiseGuiComponent* componentToAdd)
	{
		if (componentToAdd)
		{
			for (auto & element : ActiveComponents)
			{
				if (element == componentToAdd)
				{
					return;
				}
			}
			ActiveComponents.push_back(componentToAdd);
		}
	}
	static void RemoveActiveComponent(BaseWwiseGuiComponent* componentToRemove)
	{
		if (componentToRemove)
		{
			int i = 0;
			for (auto & element : ActiveComponents) {
				if (element == componentToRemove)
				{
					ActiveComponents.erase(ActiveComponents.begin()+i);
					return;
				}
				i++;
			}
		}
	}
	
	static void callback_OnSelectionChanged(uint64_t in_subscriptionId, const AK::WwiseAuthoringAPI::JsonProvider& in_jsonProvider)
	{
		for (auto & element : WwiseConnectionHandler::ActiveComponents)
		{
			if (element)
			{
				element->postCommandMessage(1);
			}
			//TransferToWwiseComponent::currentTransferComponent->postCommandMessage(1);
		}
	}

	static void callback_OnProjectClosed(uint64_t in_subscriptionId, const AK::WwiseAuthoringAPI::JsonProvider& in_jsonProvider)
	{
		for (auto & element : WwiseConnectionHandler::ActiveComponents)
		{
			if (element)
			{
				element->postCommandMessage(2);
			}
			//TransferToWwiseComponent::currentTransferComponent->postCommandMessage(2);
		}
	}
	
	WwiseConnectionHandler();
	~WwiseConnectionHandler();



	CurrentWwiseConnection MyCurrentWwiseConnection;

	//bool StartGUI_Get(HINSTANCE &myhInst);

	//bool StartGUI_Transfer(HINSTANCE &myhInst);

	bool handle_GUI_Connect();

	bool ConnectToWwise(bool suppressOuputMessages, int port);
	
	void DisconnectFromWwise();

	WwiseObject GetSelectedObject();

	std::vector<WwiseObject> GetWwiseObjects(bool suppressOuputMessages, ObjectGetArgs& getargs, AK::WwiseAuthoringAPI::AkJson::Array& Results);
	
	WwiseObject CreateStructureFromPath(std::string path,std::string parent = "Actor-Mixer Hierarchy");

	bool CreateWwiseObjects(bool suppressOutputMessages, CreateObjectArgs& createArgs, AK::WwiseAuthoringAPI::AkJson::Array& Results);

	bool ImportAudioToWwise(bool suppressOutputMessages, ImportObjectArgs& importArgs, AK::WwiseAuthoringAPI::AkJson::Array& Results);

	bool GetWwiseProjectGlobals(bool suppressOutputMessages, WwiseProjectGlobals& WwiseProjGlobals);

	WwiseObject ResultToWwiseObject(AK::WwiseAuthoringAPI::AkJson Result);

	bool LinkParentChildObjects(std::vector<WwiseObject>& objects);

	void SetOptionsFromConfig(config myConfig);

	void SetWwiseAutomationMode(bool enable);
	
	bool SetNotesForObject(std::string id, std::string notes,AK::WwiseAuthoringAPI::AkJson & results);
	
	bool SubscribeOnSelectionChanged(AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback,uint64_t &outsubscriptionID);
	
	bool SubscribeOnProjectClosed(AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback,uint64_t &outsubscriptionID);
	
	bool UnsubscribeFromTopicByID(uint64_t subscriptionID)
	{
		return waapi_Unsubscribe(subscriptionID);
	}

	bool SaveWwiseProject();

	bool CheckForProjectFileChanges();


private:

	void ReportConnectionError(CurrentWwiseConnection attemptedConnection);

};




///////////////////////////////////
/////Create Import Window - Implements Object Create and Import
//////////////////////////////////

struct activeSourceUpdateInfo
{
	std::string parentSoundID="";
	std::string newActiveSourceName="";
	std::string workUnitPath="";//wwise returns the workunit_id so we will need to get the actual path later with a get call asking for filePath
};

class CreateImportWindow
{
	static bool isReaperRendering;

public:
	
	WwiseConnectionHandler * WwiseConnectionHnd = nullptr;
	std::vector<RenderQueJob> GlobalListOfRenderQueJobs;
	std::vector<std::string> RenderFilesBackup;
	
	//TransferToWwiseComponent * owningGUIWindow;
	BaseWwiseGuiComponent * owningGUIWindow = nullptr;
	//juce::Component * owningGUIWindow;
	
	CreateImportWindow();
	~CreateImportWindow();

	config myConfig;

	//static INT_PTR CALLBACK DialogProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	
	
	void SetupPluginParent(WwiseConnectionHandler *parent);

	////=============================================================================
	//int CreateTransferWindow(HINSTANCE hInst, HINSTANCE, LPSTR, int);

	//HWND m_hParent;
	int m_nResId;

	CreateImportWindow* thisPluginWindow = nullptr;


	////=============================================================================
	////message processing function declarations
	void handleUI_B_Connect();
	void handleUI_B_CreateObject(CreateObjectArgs myCreateObjectArgs);
	void handleUI_B_GetSelectedParent();
	void handleUI_GetType(int notifCode);
	void handleUI_GetImportEventOptions(int notifCode);
	void handleUI_GetNameConflict(int notifCode);
	bool handleUI_RenderImport();

	void handleActiveSourceUpdatesForVersions();

	////non-message function declarations
	////=============================================================================
	void OnCommand(int id, int notifycode);
	////=============================================================================
	void OnInitDlg();
	////=============================================================================
	inline int ErrMsg(const std::string& s);

	/////Handle the various UsI elements


	/////Initialise dialogue boxes
	bool init_ComboBox_A(std::vector<std::string> choices);
	bool init_ListBox_A(std::vector<std::string> choices);

	void FillRenderQueList();
	void UpdateRenderJob_TreeView();
	bool GetCreateEvent();
	std::string GetLanguage();
	std::string GetImportEventOption();
	std::string GetUserOriginalsSubDir();
	void SetStatusMessageText(std::string message);

	void OpenHelp();

	bool UpdateProgressDuringRender(int numJobs);

	bool ImportJobsIntoWwise();
	void CreatePlayEventForID(std::string id,std::string name,std::string notes="",std::string path="\\Events\\Default Work Unit");
	
	ImportObjectArgs SetupImportArgs(WwiseObject parent, IMPORT_TYPE importType, std::string ImportLanguage,
									 bool OrigsDirMatchesWwise, std::string userOrigSubDir,
									 std::vector<std::string> ImportFiles,
									 std::string eventCreateOption,
									 std::string SourceReaperProj = "",
									 std::string Notes = "");
	
	ImportObjectArgs SetupImportArgs(WwiseObject parent, IMPORT_TYPE importType, std::string ImportLanguage,
									 bool OrigsDirMatchesWwise,std::string userOrigSubDir,
									 std::vector<std::string> ImportFiles);
	
	bool ImportCurrentRenderJob(ImportObjectArgs curJobImportArgs);
	
	bool AudioFileExistsInWwise(std::string audioFile, WwiseObject& parent, std::string& originalsPath, std::string& existingWwisePath);

	void backupRenderQueFiles();
	void restoreRenderQueFiles();

	void SetWwiseAutomationMode(bool enable);

	bool IsAudioFileAVersion(std::string input, std::string& outVarientToken);
	
	std::map<std::string, std::vector<activeSourceUpdateInfo>> activeSourcesUpdateMap;
	
	WwiseObject GetWwiseObjectFromID(std::string guid);

	std::vector<WwiseObject> GetWwiseObjectsByName(std::string objectName, std::string type);
	
	std::string PrepareEventPathForCreation(std::string inPath);

	//INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


class ReaperRenderObj
{
public:
	ReaperRenderObj();
	~ReaperRenderObj();
	void RenderAllQues();
};
