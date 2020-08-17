#pragma once
//#include <windows.h>
//#include <windowsx.h>
//#include <tchar.h>    //string and other mapping macros
#include <string>
#include <map>
#include "ConfigFileHandler.h"
#include "waapi_structs.h"

#include "WaapiFunctions.h"
//#include "PluginWindow.h"



class WwiseConnectionHandler
{	

public:
	WwiseConnectionHandler();
	~WwiseConnectionHandler();



	CurrentWwiseConnection MyCurrentWwiseConnection;

	//bool StartGUI_Get(HINSTANCE &myhInst);

	//bool StartGUI_Transfer(HINSTANCE &myhInst);

	bool handle_GUI_Connect();

	bool ConnectToWwise(bool suppressOuputMessages, int port);

	bool GetSelectedWwiseObjects(bool suppressOuputMessages);

	WwiseObject GetSelectedObject();

	void GetChildrenFromSelectedParent(bool suppressOuputMessages);

	std::vector<WwiseObject> GetWwiseObjects(bool suppressOuputMessages, ObjectGetArgs& getargs, AK::WwiseAuthoringAPI::AkJson::Array& Results);

	bool CreateWwiseObjects(bool suppressOutputMessages, CreateObjectArgs& createArgs, AK::WwiseAuthoringAPI::AkJson::Array& Results);

	bool ImportAudioToWwise(bool suppressOutputMessages, ImportObjectArgs& importArgs, AK::WwiseAuthoringAPI::AkJson::Array& Results);

	bool GetWwiseProjectGlobals(bool suppressOutputMessages, WwiseProjectGlobals& WwiseProjGlobals);

	WwiseObject ResultToWwiseObject(AK::WwiseAuthoringAPI::AkJson Result);

	bool LinkParentChildObjects(std::vector<WwiseObject>& objects);

	void SetOptionsFromConfig(config myConfig);

	void SetWwiseAutomationMode(bool enable);


private:

	void ReportConnectionError(CurrentWwiseConnection attemptedConnection);

};



///////////////////////////////////
/////Create Import Window - Implements Object Create and Import
//////////////////////////////////

class CreateImportWindow
{
	

public:
	
	WwiseConnectionHandler * WwiseConnectionHnd;
	std::vector<RenderQueJob> GlobalListOfRenderQueJobs;
	std::vector<std::string> RenderFilesBackup;
	
	CreateImportWindow();
	~CreateImportWindow();

	//static HWND m_hWindow;
	static long m_lSaveThis;
	CreateImportWindow* saveThis;


	//static INT_PTR CALLBACK DialogProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	
	void SetupPluginParent(WwiseConnectionHandler *parent);

	////=============================================================================
	//int CreateTransferWindow(HINSTANCE hInst, HINSTANCE, LPSTR, int);

	//HWND m_hParent;
	int m_nResId;

	CreateImportWindow* thisPluginWindow;


	////=============================================================================
	////message processing function declarations
	void handleUI_B_Connect();
	void handleUI_B_CreateObject(CreateObjectArgs myCreateObjectArgs);
	void handleUI_B_GetSelectedParent();
	void handleUI_GetType(int notifCode);
	void handleUI_GetImportEventOptions(int notifCode);
	void handleUI_GetNameConflict(int notifCode);
	void handleUI_RenderImport();

	////non-message function declarations
	////=============================================================================
	void OnCommand(int id, int notifycode);
	////=============================================================================
	void OnInitDlg();
	////=============================================================================
	inline int ErrMsg(const std::string& s);

	/////Handle the various UsI elements


	/////Initialise dialogue boxes
	bool init_ALL_OPTIONS();
	bool init_ComboBox_A(std::vector<std::string> choices);
	bool init_ListBox_A(std::vector<std::string> choices);

	void FillRenderQueList();
	void UpdateRenderJob_TreeView();
	void HandleUI_SetParentForRenderJob(WwiseObject selectedParent);
	bool GetCreateEvent();
	bool GetIsVoice();
	std::string GetLanguage();
	std::string GetImportEventOption();
	bool GetOrigsDirMatchesWwise();
	std::string GetUserOriginalsSubDir();
	void SetStatusMessageText(std::string message);

	void OpenHelp();

	bool UpdateProgressDuringRender(int numJobs);

	bool ImportJobsIntoWwise();
	void CreatePlayEventForID(std::string id,std::string name);
	ImportObjectArgs SetupImportArgs(WwiseObject parent, bool isVoice, std::string ImportLanguage, bool OrigsDirMatchesWwise, std::string userOrigSubDir, std::vector<std::string> ImportFiles, std::string eventCreateOption);
	ImportObjectArgs SetupImportArgs(WwiseObject parent, bool isVoice, std::string ImportLanguage, bool OrigsDirMatchesWwise,std::string userOrigSubDir, std::vector<std::string> ImportFiles);
	bool ImportCurrentRenderJob(ImportObjectArgs curJobImportArgs);
	bool AudioFileExistsInWwise(std::string audioFile, WwiseObject& parent, std::string& originalsPath, std::string& existingWwisePath);

	void backupRenderQueFiles();
	void restoreRenderQueFiles();

	void SetWwiseAutomationMode(bool enable);


	//INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


class ReaperRenderObj
{
public:
	ReaperRenderObj();
	~ReaperRenderObj();
	void RenderAllQues();
};
