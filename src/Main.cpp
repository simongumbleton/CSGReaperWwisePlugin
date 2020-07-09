
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

#define REAPERAPI_IMPLEMENT
#define REAPERAPI_DECL

#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"

#include "GUI.h"
#include "gui_Transfer.h"

#include "reaperHelpers.h"

REAPER_PLUGIN_HINSTANCE g_hInst;


//define globals
HWND g_parentWindow;
char reaperProjectName[256];
std::string reaperResourcePath;
int WaapiPort = 8095;

char currentProject[256];



gaccel_register_t action01 = { { 0, 0, 0 }, "Do action 01." };

std::unique_ptr<BasicWindow> mainWindow;
//std::unique_ptr<MainWindow> mainWindow2;
std::unique_ptr<TransferWindow> mainWindow2;

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

		String wName = "JUCE test window";
		initialiseJuce_GUI();

		MessageManagerLock mml(Thread::getCurrentThread());
		//mainWindow.reset( new BasicWindow(wName,Colour::Colour(255,0,0),DocumentWindow::TitleBarButtons::allButtons));
		
        mainWindow2.reset(new TransferWindow(wName, new TransferToWwiseComponent));


		//mainWindow->setVisible(true);
		//mainWindow->centreWithSize(500, 500);
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
