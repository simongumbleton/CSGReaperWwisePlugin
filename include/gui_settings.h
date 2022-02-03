#pragma once
#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "platformhelpers.h"
#include "cog.h"
#include "gui_reconformer.h"


struct TransferSettingsStruct
{
	
};

struct MetadataSettingsStruct
{
	std::vector<std::string>PropertyNames;
	
};




class SettingsButton : public ImageButton
{
	Image cog = ImageFileFormat::loadFrom(cog::cog_png, cog::cog_pngSize);
public:
	
	SettingsButton(const String & name)
	{
		setImages(false, true, true, cog, 1.0f, Colours::transparentBlack, cog, 1.0f, Colours::transparentBlack, cog, 1.0f, Colours::transparentBlack);
	};
	
};

class SettingsComponent : public Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:
	
	Label* info = new Label("Settings");
	
	
	SettingsComponent(){
		info->setText("EDL Settings", NotificationType::dontSendNotification);
		addAndMakeVisible(info);
		setWantsKeyboardFocus(true);
		grabKeyboardFocus();
	};
	
	~SettingsComponent(){};
	
	void resized() override
	{
		auto area = getBoundsInParent();
		info->setBounds(area.removeFromTop(20));
		info->setJustificationType(Justification::centred);
	};
	
	void buttonClicked(juce::Button* pButton)override{};

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override{};

	void labelTextChanged(Label* labelThatHasChanged)override{};
	
};

class SettingsWindow : public juce::DocumentWindow
{
	bool* mWindowState;
public:
	SettingsWindow(const juce::String& name, bool* windowStatus)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		

		mWindowState = windowStatus;
		*mWindowState = true;

		setResizable(true, false);
		//setResizeLimits(500, 500, 10000, 10000);
		//setSize(750, 600);
		centreWithSize(getWidth(), getHeight());
		//setWantsKeyboardFocus(true);
		setVisible(true);
		//grabKeyboardFocus();
	}

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

class MetadataSettingsWnd : public SettingsWindow
{
public:
};

class EDLSettingsCmp : public SettingsComponent
{
	EDLSettingsStruct& rSettings;
	
	std::vector<String> supportedFramerates
	{
		"30",
		"24",
		"25" ,
		"29.97",
		"59.97",
		"60",
		"120",
	};
	
	int getSavedFramerateAsID()
	{
		int i = 1;
		for (auto choice : supportedFramerates)
		{
			auto choiceAsFloat = choice.getFloatValue();
			if (choiceAsFloat == rSettings.framerate)
			{
				return i;
			}
			i++;
		}
		return 1;
	};
	
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
	
	
	
	
	
	
	void InitComboBox(juce::ComboBox * comboBox, std::vector<String> choices)
	{
		comboBox->clear();
		int i = 1;
		for (auto choice : choices)
		{
			comboBox->addItem(choice, i);
			i++;
		}
		comboBox->addListener(this);
		comboBox->setSelectedItemIndex(0, false);
		addAndMakeVisible(comboBox);
	}
	
	
	void buttonClicked(juce::Button* pButton)override{
		//PrintToConsole(pButton->getName().toStdString());
		if (pButton == btn_EDLRegions){
			rSettings.CreateEDLFileRegion = btn_EDLRegions->getToggleState();
		}else if (pButton == btn_ExistingRegions){
			rSettings.CopyExistingRegions = btn_ExistingRegions->getToggleState();
		}else if (pButton == btn_ChangedShots){
			rSettings.CreateRegionsForChangedShots = btn_ChangedShots->getToggleState();
		}
	};

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override{
		if (comboBoxThatHasChanged == dd_Framerate)
		{
			auto value = dd_Framerate->getText().getFloatValue();
			rSettings.framerate = value;
		}
		
	};

	void labelTextChanged(Label* labelThatHasChanged)override{
		//PrintToConsole(labelThatHasChanged->getName().toStdString());
		if (labelThatHasChanged == txt_StartOffset){
			rSettings.timeLineOffset = txt_StartOffset->getText().toStdString();
		}
	};

	EDLSettingsCmp(EDLSettingsStruct& inSettings):rSettings(inSettings)
	{
		info->setText("EDL Conform Settings", NotificationType::dontSendNotification);
		
		info_StartOffset->setText("EDL Project Offset:", NotificationType::dontSendNotification);
		addAndMakeVisible(info_StartOffset);
		info_StartOffset->setTooltip("Location to copy the original content to when conforming.");
		txt_StartOffset->setEditable(true);
		txt_StartOffset->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
		addAndMakeVisible(txt_StartOffset);
		info_StartOffset->attachToComponent(txt_StartOffset, true);
	
		txt_StartOffset->addListener(this);
		txt_StartOffset->setText(rSettings.timeLineOffset, NotificationType::dontSendNotification);
		
		
		
		txt_StartOffset->onTextChange = [this]
		{
			if (!stringIsTimecode(txt_StartOffset->getText().toStdString()))
			{
				txt_StartOffset->setText(rSettings.timeLineOffset, sendNotification);
			}
		};
		
		
		
		
		info_Framerate->setText("Framerate:", NotificationType::dontSendNotification);
		addAndMakeVisible(info_Framerate);
		info_Framerate->setTooltip("Framerate to use during EDL conform.");

		addAndMakeVisible(dd_Framerate);
		info_Framerate->attachToComponent(dd_Framerate, true);
		InitComboBox(dd_Framerate, supportedFramerates);
		dd_Framerate->setSelectedId(getSavedFramerateAsID());
		
		
		
		btn_EDLRegions->addListener(this);
		addAndMakeVisible(btn_EDLRegions);
		btn_EDLRegions->setTooltip("Create a region with the name of the new EDL file.");
		btn_EDLRegions->setToggleState(inSettings.CreateEDLFileRegion, NotificationType::dontSendNotification);
		
		btn_ExistingRegions->addListener(this);
		addAndMakeVisible(btn_ExistingRegions);
		btn_ExistingRegions->setTooltip("Maintain existing project regions in place.");
		btn_ExistingRegions->setToggleState(inSettings.CopyExistingRegions, NotificationType::dontSendNotification);
		
		btn_ChangedShots->addListener(this);
		addAndMakeVisible(btn_ChangedShots);
		btn_ChangedShots->setTooltip("Create new regions for shots that have changed.");
		btn_ChangedShots->setToggleState(inSettings.CreateRegionsForChangedShots, NotificationType::dontSendNotification);
	};
	~EDLSettingsCmp(){};
	
	void resized() override
	{
		auto area = getBoundsInParent();
		auto width = getWidth();
		info->setBounds(area.removeFromTop(20));
		info->setJustificationType(Justification::centred);
		auto buffer = area.removeFromTop(30);
		auto area1 = area.removeFromTop(30);
		info_StartOffset->setBounds(area1.removeFromLeft(width/2));
		txt_StartOffset->setBounds(area1.reduced(3));
		auto buffer2 = area.removeFromTop(5);
		auto area2 = area.removeFromTop(30);
		info_Framerate->setBounds(area2.removeFromLeft(width/2));
		dd_Framerate->setBounds(area2.reduced(3));
		auto buffer3 = area.removeFromTop(5);
		btn_EDLRegions->setBounds(area.removeFromTop(20));
		auto buffer4 = area.removeFromTop(5);
		btn_ExistingRegions->setBounds(area.removeFromTop(20));
		auto buffer5 = area.removeFromTop(5);
		btn_ChangedShots->setBounds(area.removeFromTop(20));
		
	};
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

	void closeButtonPressed() override
	   {
		   delete this;
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
