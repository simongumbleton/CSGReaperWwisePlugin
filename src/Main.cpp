
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
#include "gui_RegionMetadata.h"

#include "reaperHelpers.h"
#include "platformhelpers.h"

#include "EDL_Conformer.h"
#include "gui_reconformer.h"

REAPER_PLUGIN_HINSTANCE g_hInst;


#define GET_FUNC_AND_CHKERROR(x) if (!((*((void **)&(x)) = (void *)rec->GetFunc(#x)))) ++funcerrcnt
#define REGISTER_AND_CHKERROR(variable, name, info) if(!(variable = rec->Register(name, (void*)info))) ++regerrcnt

//define globals
HWND g_parentWindow;
char reaperProjectName[256];
std::string reaperResourcePath;
int WaapiPort = 8095;

char currentProject[256];

//std::unique_ptr<juce::DocumentWindow>currentActiveWindow;
bool transferWindowStatus = false;
bool templateWindowStatus = false;
bool edlWindowStatus = false;
bool assemblerWindowStatus = false;
juce::DocumentWindow * currentTransferWindow = nullptr;
juce::DocumentWindow * currentTemplateWindow = nullptr;
juce::DocumentWindow * currentEDLWindow = nullptr;
juce::DocumentWindow* currentAssemblerWindow = nullptr;

gaccel_register_t Transfer_To_Wwise = { { 0, 0, 0 }, "CSG Ext - Transfer To Wwise" };
gaccel_register_t Template_To_Wwise = { { 0, 0, 0 }, "CSG Ext - Region Metadata" };
gaccel_register_t EDL_Window = { { 0, 0, 0 }, "CSG Ext - EDL Conformer" };
gaccel_register_t Assembler_Window = { { 0, 0, 0 }, "CSG Ext - EDL Assembler" };


void LaunchTransferWindow();
void LaunchTemplateWindow();
void LaunchEDLWindow();
void LaunchAssemblerWindow();
bool HookCommandProc(int command, int flag);
static void menuHook(const char *name, HMENU handle, const int f);
static void AddCustomCSGMenuItems(HMENU parentMenuHandle = NULL);

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
		REGISTER_AND_CHKERROR(Template_To_Wwise.accel.cmd, "command_id", "CSG_Ext_RegionMetadata");
		REGISTER_AND_CHKERROR(EDL_Window.accel.cmd, "command_id", "CSG_Ext_EDLConformer");
		REGISTER_AND_CHKERROR(Assembler_Window.accel.cmd, "command_id", "CSG_Ext_EDLAssembler");
		
		//register our custom actions
		plugin_register("gaccel", &Transfer_To_Wwise.accel);
		plugin_register("gaccel", &Template_To_Wwise.accel);
		plugin_register("gaccel", &EDL_Window.accel);
		plugin_register("gaccel", &Assembler_Window.accel);
		
		//hook command is where we process command IDs and launch our custom actions
		rec->Register("hookcommand", (void*)HookCommandProc);
		//hook custom menu is where our custom actions are added to the main extensions submenu
		plugin_register("hookcustommenu", reinterpret_cast<void *>(menuHook));
		//Reaper API adds the Extensions main menu - we populate this in the hook custom menu
		AddExtensionsMainMenu();
		
#ifdef _WIN32
		// Create a custom menu to the main menu and add the available commands
		// only do this on windows, on Mac it seems to not work in the main menu,
		//so on mac we add them to the main extenstions menu only
		//(we add them to the main extensions menu anyway, but on windows its
		//nice to have them in the main menu too
		AddCustomCSGMenuItems();
#endif
		
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

std::string GetCurrentReaperProjectName()
{
	return PLATFORMHELPERS::filenameFromPathString(GetCurrentReaperProject());
}

ReaProject* GetCurrentReaProject()
{
	return EnumProjects(-1, NULL, NULL);
}

std::string GetCurrentReaperProjectPath()
{
	char projPath[256];
	EnumProjects(-1, projPath, MAX_PATH);
	return std::string(projPath);
}

void SaveProject(ReaProject* inProj)
{
	Main_SaveProject(inProj, false);
}
void SaveProject()
{
	Main_SaveProject(GetCurrentReaProject(), false);
}

void saveProjExState(std::string Key, std::string Value,std::string extName,ReaProject* inProj)
{
	if (inProj == nullptr){inProj = GetCurrentReaProject();}
	SetProjExtState(inProj, extName.c_str(), &Key[0], &Value[0]);
	SaveProject(inProj);
}

void deleteProjExtState(std::string Key, std::string extName,ReaProject* inProj)
{
	saveProjExState(Key.c_str(),"",extName.c_str(),inProj);
}

std::string getProjExState(std::string Key, std::string extName,ReaProject* inProj)
{
	//std::string KeyOutValue;
	//char bufferK[256] = "x";
	//char* K;
	//int KeyOutSize = 256;
	//std::string OutValue;
	//char* V;
	//char bufferV[256] = "a";
	//int OutSize = 256;

	//int i = 0;
	//while (EnumProjExtState(GetCurrentReaProject(), "CSGREGIONPROPERTIES", i, bufferK, KeyOutSize, bufferV, OutSize))
	//{
	//	KeyOutValue = std::string(bufferK);
	//	OutValue = std::string(bufferV);
	//	PrintToConsole(KeyOutValue);
	//	PrintToConsole(OutValue);
	//	i++;
	//}
	const int ValOutSize = 25600;/// Need to find a safer way of doing this!
	char bufferValue[ValOutSize] = "";
	
	//std::string OutValue;
	//int OutSize;
	std::string keyUC = PLATFORMHELPERS::stringToUpper(Key);
	//needs a char* for outvalue
	if (inProj == nullptr){inProj = GetCurrentReaProject();}
	GetProjExtState(inProj, extName.c_str(), &keyUC[0], bufferValue, ValOutSize);
	return std::string(bufferValue);
}

void saveGlobalExtState(std::string Key, std::string Value,bool persist, std::string extName)
{
	SetExtState(extName.c_str(),Key.c_str(), Value.c_str(), persist);
}

void deleteGlobalExtState(std::string Key,bool persist, std::string extName)
{
	DeleteExtState(extName.c_str(),Key.c_str(),persist);
}

std::string getGlobalExtState(std::string Key, std::string extName)
{
	std::string value = "";
	std::string keyUC = PLATFORMHELPERS::stringToUpper(Key);
	bool HasState = HasExtState(extName.c_str(), keyUC.c_str());
	if (HasState)
	{
		value = GetExtState(extName.c_str(), keyUC.c_str());
	}
	return value;
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

int countRegions()
{
	int markerCount;
	int regionCount;
	CountProjectMarkers(GetCurrentReaProject(), &markerCount, &regionCount);
	return regionCount;
}

int countMarkers()
{
	int markerCount;
	int regionCount;
	CountProjectMarkers(GetCurrentReaProject(), &markerCount, &regionCount);
	return markerCount;
}



std::vector<std::string> getNonMasterProjectRegionNames()
{
	int total = countRegions() + countMarkers();
	std::vector<std::string> results;
	int i = 0;
	while (i < total)
	{
		bool isRegion;
		double pos;
		double regEnd;
		//std::string name;
		const char* name;
		int index;
		EnumProjectMarkers(i, &isRegion,&pos,&regEnd,&name,&index);
		if (isRegion)
		{
			std::string namestr = name;
			if (!namestr.empty())
			{
				if (!PLATFORMHELPERS::ends_with(PLATFORMHELPERS::stringToLower(name), "_master"))
				{
					results.push_back(namestr);
				}
			}
		}
		i++;
	}
	return results;
}

void PrintToConsole(int text)
{

		std::string debugText = std::to_string(text) + "\n";
		ShowConsoleMsg(debugText.c_str());

}

void LaunchEDLWindow()
{
	initialiseJuce_GUI();
	MessageManagerLock mml(Thread::getCurrentThread());

	if (edlWindowStatus)
	{
		currentEDLWindow->toFront(true);
	}
	else
	{
		currentEDLWindow = new ConformerWindow("Unreal EDL Conformer", new ConformerComponent(),&edlWindowStatus);
	}
}

void LaunchAssemblerWindow()
{
	initialiseJuce_GUI();
	MessageManagerLock mml(Thread::getCurrentThread());

	if (assemblerWindowStatus)
	{
		currentAssemblerWindow->toFront(true);
	}
	else
	{
		currentAssemblerWindow = new ConformerWindow("Unreal EDL Assembler", new AssemblerComponent(), &assemblerWindowStatus);
	}
}

void LaunchTransferWindow()
{
	initialiseJuce_GUI();
	MessageManagerLock mml(Thread::getCurrentThread());
		
	
	bool useTab = true;
	String wName = "CSG Reaper Transfer to Wwise Extension";
	
	//int commandID = NamedCommandLookup("_S&M_HIDECCLANES_ME");
	//reaper.Main_OnCommand(commandID, 0)
	//std::string name = ReverseNamedCommandLookup(commandID);
	//int commandID = AddRemoveReaScript(true, 0, reaper.GetResourcePath().."/Scripts/12000.eel", true);
	
	if (useTab)
	{
		if (transferWindowStatus)
		{
			currentTransferWindow->toFront(true);
		}
		else
		{
			TransferWindow* mainWindow2 = new TransferWindow(wName, new TransferTabComponent(juce::TabbedButtonBar::Orientation::TabsAtTop),&transferWindowStatus);
			currentTransferWindow = mainWindow2;
		}
		return;
	}
	
	
	if (transferWindowStatus)
	{
		currentTransferWindow->toFront(true);
	}
	else
	{
		TransferWindow* mainWindow2 = new TransferWindow(wName, new TransferToWwiseComponent(nullptr),&transferWindowStatus);
		currentTransferWindow = mainWindow2;
	}
}

void LaunchTemplateWindow()
{
	//Testing set markers
	//SetProjectMarker(1,true,10.0,110.0,"TestRegion");
	
	String wName = "CSG Reaper Region Metadata";
	initialiseJuce_GUI();
	MessageManagerLock mml(Thread::getCurrentThread());
	if (templateWindowStatus)
	{
		currentTemplateWindow->toFront(true);
	}
	else
	{
		RegionMetadataWindow* mainWindow3 = new RegionMetadataWindow(wName, new RegionMetadataComponent,&templateWindowStatus);
		currentTemplateWindow = mainWindow3;
	}
	
}

void bringWindowsToFront()
{
	if (currentTransferWindow && transferWindowStatus)
	{
		currentTransferWindow->toFront(true);
	}
	if (currentTemplateWindow && templateWindowStatus)
	{
		currentTemplateWindow->toFront(true);
	}
	if (currentEDLWindow && edlWindowStatus)
	{
		currentEDLWindow->toFront(true);
	}
	if (currentAssemblerWindow && assemblerWindowStatus)
	{
		currentAssemblerWindow->toFront(true);
	}
}



void ClearCurrentWindowPtr()
{
	currentTransferWindow = nullptr;
	currentTemplateWindow = nullptr;
	currentEDLWindow = nullptr;
	currentAssemblerWindow = nullptr;
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
	else if (command == EDL_Window.accel.cmd)
	{
		LaunchEDLWindow();
		return true;
	}
	else if (command == Assembler_Window.accel.cmd)
	{
		LaunchAssemblerWindow();
		return true;
	}
	return false;
}

static void menuHook(const char *name, HMENU handle, const int f)
{
	if(strcmp(name, "Main extensions")==0 and f == 0)
	{
		// Create a custom menu and add the available commands
		AddCustomCSGMenuItems(handle);
	}
}

static void AddCustomCSGMenuItems(HMENU parentMenuHandle)
{
	HMENU hMenu = CreatePopupMenu();
	
	MENUITEMINFO mi = { sizeof(MENUITEMINFO), };
	mi.fMask = MIIM_TYPE | MIIM_ID;
	mi.fType = MFT_STRING;
	
	// add each command to the popupmenu
	mi.wID = Transfer_To_Wwise.accel.cmd;
	mi.dwTypeData = (char *)"Transfer To Wwise";
	InsertMenuItem(hMenu, 0, true, &mi);
	
	mi.wID = Template_To_Wwise.accel.cmd;
	mi.dwTypeData = (char *)"Region Metadata";
	InsertMenuItem(hMenu, 0, true, &mi);
	
	mi.wID = EDL_Window.accel.cmd;
	mi.dwTypeData = (char *)"Unreal EDL Conformer";
	InsertMenuItem(hMenu, 0, true, &mi);

	mi.wID = Assembler_Window.accel.cmd;
	mi.dwTypeData = (char*)"Unreal EDL Assembler";
	InsertMenuItem(hMenu, 0, true, &mi);

	if (!parentMenuHandle)
	{
		parentMenuHandle = GetMenu(GetMainHwnd());
	}
	
	mi.fMask = MIIM_SUBMENU | MIIM_TYPE;
	mi.hSubMenu = hMenu;
	mi.dwTypeData = (char*)"CSG";
	InsertMenuItem(parentMenuHandle, GetMenuItemCount(parentMenuHandle) - 1, TRUE, &mi);
}
