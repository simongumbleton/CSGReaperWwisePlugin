
#ifdef _WIN32
#include <windows.h>
#include <Commctrl.h>
#else
#include "WDL/swell/swell.h"
#endif

#include <string>
#include "stdio.h"
#include "stdlib.h"

#include <iostream> 
#include <sstream>
#include <thread>

#define REAPERAPI_IMPLEMENT
#define REAPERAPI_DECL

#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"

#include "GUI.h"
#include "gui_Transfer.h"
#include "gui_Templates.h"

#include "reaperHelpers.h"

REAPER_PLUGIN_HINSTANCE g_hInst;


#define GET_FUNC_AND_CHKERROR(x) if (!((*((void **)&(x)) = (void *)rec->GetFunc(#x)))) ++funcerrcnt
#define REGISTER_AND_CHKERROR(variable, name, info) if(!(variable = rec->Register(name, (void*)info))) ++regerrcnt

//define globals
HWND g_parentWindow;
char reaperProjectName[256];
std::string reaperResourcePath;
int WaapiPort = 8095;

char currentProject[256];



gaccel_register_t Transfer_To_Wwise = { { 0, 0, 0 }, "CSG Ext - Transfer To Wwise" };
gaccel_register_t Template_To_Wwise = { { 0, 0, 0 }, "CSG Ext - Transfer To Wwise - Template" };

//std::unique_ptr<BasicWindow> mainWindow;
//std::unique_ptr<MainWindow> mainWindow2;
//std::unique_ptr<TransferWindow> mainWindow2;

void LaunchTransferWindow();
void LaunchTemplateWindow();
bool HookCommandProc(int command, int flag);

extern "C"
{
REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
{
	g_hInst = hInstance;
	if (rec)
	{
		rec->Register;
		HWND main = rec->hwnd_main;
		//MessageBox(main, "Hello World","Reaper Basic Extension", MB_OK);

		// load all Reaper API functions in one go, byebye ugly IMPAPI macro!
		int error_count = REAPERAPI_LoadAPI(rec->GetFunc);
		if (error_count > 0)
		{
			char errbuf[256];
			sprintf(errbuf, "Failed to load %d expected API function(s)", error_count);
			MessageBox(main, errbuf, "MRP extension error", MB_OK);
			return 0;
		}
		int regerrcnt = 0;
		//Register a custom command ID for an action
		REGISTER_AND_CHKERROR(Transfer_To_Wwise.accel.cmd, "command_id", "CSG_Ext_TransferToWwise");
		REGISTER_AND_CHKERROR(Template_To_Wwise.accel.cmd, "command_id", "CSG_Ext_TransferToWwise_Template");
		
		//register actions
		plugin_register("gaccel", &Transfer_To_Wwise.accel);
		plugin_register("gaccel", &Template_To_Wwise.accel);
		//plugin_register("custom_action",&Transfer_To_Wwise.accel.cmd);

		rec->Register("hookcommand", (void*)HookCommandProc);

		AddExtensionsMainMenu();

		HMENU hMenu = GetSubMenu(GetMenu(GetMainHwnd()), 9);
		
		//SWELL_Menu_AddMenuItem(hMenu, "CSG", 0, 999);
		
		
		{
			MENUITEMINFO mi = { sizeof(MENUITEMINFO), };
			mi.fMask = MIIM_TYPE | MIIM_ID;
			mi.fType = MFT_STRING;
			mi.wID = Transfer_To_Wwise.accel.cmd;
			mi.dwTypeData = (char *)"CSG - Transfer To Wwise";
			InsertMenuItem(hMenu, 0, true, &mi);
		}
		{
			MENUITEMINFO mi = { sizeof(MENUITEMINFO), };
			mi.fMask = MIIM_TYPE | MIIM_ID;
			mi.fType = MFT_STRING;
			mi.wID = Template_To_Wwise.accel.cmd;
			mi.dwTypeData = (char *)"CSG - Transfer To Wwise - Template";
			InsertMenuItem(hMenu, 0, true, &mi);
		}
		
		
		
		//LaunchTransferWindow();

		
	}
	return 1;
}
}

std::string GetReaperResourcePath()
{
	return GetResourcePath();
}

std::string GetCurrentReaperProject()
{
	char projName[256];
	EnumProjects(-1, currentProject, MAX_PATH);
	GetProjectName(EnumProjects(-1, nullptr, 0), projName, 256);
	return std::string(projName);
}

void Reaper_RenderAllQuedJobs()
{
	Main_OnCommand(41207, 0);
}

void GetReaperGlobals()
{
	//get open project and compare
	EnumProjects(-1, currentProject, MAX_PATH);
	GetProjectName(EnumProjects(-1, nullptr, 0), reaperProjectName, 256);
	reaperResourcePath = GetResourcePath();
}

void PrintToConsole(std::string text)
{

		std::string debugText = text + "\n";
		ShowConsoleMsg(debugText.c_str());

}

void PrintToConsole(int text)
{

		std::string debugText = std::to_string(text) + "\n";
		ShowConsoleMsg(debugText.c_str());

}

void LaunchTransferWindow()
{
	String wName = "CSG Reaper Transfer to Wwise Extension";
	initialiseJuce_GUI();
	MessageManagerLock mml(Thread::getCurrentThread());
	TransferWindow* mainWindow2 = new TransferWindow(wName, new TransferToWwiseComponent);
}

void LaunchTemplateWindow()
{
	String wName = "CSG Reaper Transfer to Wwise Extension";
	initialiseJuce_GUI();
	MessageManagerLock mml(Thread::getCurrentThread());
	TemplateWindow* mainWindow3 = new TemplateWindow(wName, new WwiseTemplateComponent);
}

bool HookCommandProc(int command, int flag)
{
	GetReaperGlobals();
	
	if (command == Transfer_To_Wwise.accel.cmd)
	{
		LaunchTransferWindow();
		return true;
	}
	else if (command == Template_To_Wwise.accel.cmd)
	{
		LaunchTemplateWindow();
		return true;
	}
	return false;
}
