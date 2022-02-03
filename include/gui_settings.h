#pragma once
#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "platformhelpers.h"
#include "cog.h"
#include "settings_structs.h"




///// Settings Button	////////
class SettingsButton : public ImageButton
{
	
public:
	SettingsButton(const String & name);
	~SettingsButton();
private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsButton)
};

/// Settings Component ///
class SettingsComponent : public Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:
	
	Label* info = new Label("Settings");
	
	SettingsComponent();
	
	~SettingsComponent();
	
	void resized() override;
	
	void buttonClicked(juce::Button* pButton)override{};

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override{};

	void labelTextChanged(Label* labelThatHasChanged)override{};
private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};

/// Settings Window
class SettingsWindow : public juce::DocumentWindow
{
	bool* mWindowState;
public:
	SettingsWindow(const juce::String& name, bool* windowStatus);

	void closeButtonPressed() override
	{
		
		*mWindowState = false;
		delete this;
	}

	virtual void readSettingsFromParent(){};
	
	virtual void updateSettingsInParent(){};

private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};





class TransferSettingsWnd : public SettingsWindow
{
public:
};



class MetadataSettingsCmp : public SettingsComponent
{
	MetadataSettingsStruct& rSettings;
public:
	MetadataSettingsCmp(MetadataSettingsStruct& inSettings) :rSettings(inSettings){};
	
	~MetadataSettingsCmp() {};

	void resized() override {}

	void buttonClicked(juce::Button* pButton)override {}

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override {}

	void labelTextChanged(Label* labelThatHasChanged)override {}

};

class MetadataSettingsWnd : public SettingsWindow
{
public:
	MetadataSettingsCmp* settingsComp;

	MetadataSettingsWnd(const juce::String name, MetadataSettingsStruct& inSettings, bool& windowStatus) : SettingsWindow(name, &windowStatus)
	{
		settingsComp = new MetadataSettingsCmp(inSettings);
		setContentOwned(settingsComp, true);
	}
	
	virtual void readSettingsFromParent()override
	{

	};

	virtual void updateSettingsInParent()override
	{

	};

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetadataSettingsWnd)
};

class EDLSettingsCmp : public SettingsComponent
{
	EDLSettingsStruct& rSettings;
	
	std::vector<String> supportedFramerates
	{
		"30",//default first
		"24",
		"25" ,
		"29.97",
		"59.97",
		"60",
		"120",
	};
	
	int getSavedFramerateAsID();
	
public:
	TooltipWindow tooltipWindow {nullptr,750};
	Label * info_StartOffset = new Label("info_StartOffset");
	Label * txt_StartOffset = new Label("txt_StartOffset");
	Label * info_Framerate = new Label("info_Framerate");
	Label * txt_Framerate = new Label("txt_Framerate");
	ComboBox * dd_Framerate = new ComboBox("dd_Framerate");
	ToggleButton * btn_EDLRegions = new ToggleButton("Create EDL Regions");
	ToggleButton * btn_ExistingRegions = new ToggleButton("Copy Existing Regions");
	ToggleButton * btn_ChangedShots = new ToggleButton("Create Regions for Changed Shots");
	
	void InitComboBox(juce::ComboBox * comboBox, std::vector<String> choices);
	
	
	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;

	EDLSettingsCmp(EDLSettingsStruct& inSettings);
	
	~EDLSettingsCmp(){};
	
	void resized() override;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDLSettingsCmp)
};

class EDLSettingsWnd : public SettingsWindow
{
public:
	EDLSettingsCmp * settingsComp;
	
	EDLSettingsWnd (const juce::String name, EDLSettingsStruct& inSettings, bool &windowStatus)  : SettingsWindow(name, &windowStatus)
	   {
		   settingsComp = new EDLSettingsCmp(inSettings);
		   setContentOwned(settingsComp, true);
	   }
	
	virtual void readSettingsFromParent()override
	{
		
	};
	
	virtual void updateSettingsInParent()override
	{
		
	};

 private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDLSettingsWnd)

};
