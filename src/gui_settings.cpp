//
//  gui_settings.cpp
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 25/01/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#include "gui_settings.h"
#include "image_helper.h"


SettingsButton::SettingsButton(const String & name)
{
	Image cogImg = GetSettingsImage();
	setImages(false, true, true, cogImg, 1.0f, Colours::transparentBlack, cogImg, 1.0f, Colours::transparentBlack, cogImg, 1.0f, Colours::transparentBlack);
};

SettingsButton::~SettingsButton()
{
	
};



SettingsComponent::SettingsComponent(){
	info->setText("EDL Settings", NotificationType::dontSendNotification);
	addAndMakeVisible(info);
	setWantsKeyboardFocus(true);
	grabKeyboardFocus();
};

SettingsComponent::~SettingsComponent(){};

void SettingsComponent::resized()
{
	auto area = getBoundsInParent();
	info->setBounds(area.removeFromTop(20));
	info->setJustificationType(Justification::centred);
};

void SettingsComponent::InitComboBox(juce::ComboBox* comboBox, std::vector<String> choices)
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


SettingsWindow::SettingsWindow(const juce::String& name, bool* windowStatus): DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
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





EDLSettingsCmp::EDLSettingsCmp(EDLSettingsStruct& inSettings):rSettings(inSettings)
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
		if (!PLATFORMHELPERS::stringIsTimecode(txt_StartOffset->getText().toStdString()))
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

void EDLSettingsCmp::resized()
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
	
}

;

int EDLSettingsCmp::getSavedFramerateAsID(){
	int i = 1;
	for (auto choice : supportedFramerates)
	{
		auto choiceAsFloat = choice.getFloatValue();
		if (PLATFORMHELPERS::isEqual(choiceAsFloat,rSettings.framerate))
		{
			return i;
		}
		i++;
	}
	return 1;
};

/*
void EDLSettingsCmp::InitComboBox(juce::ComboBox * comboBox, std::vector<String> choices)
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
*/

void EDLSettingsCmp::buttonClicked(juce::Button* pButton)
{
	//PrintToConsole(pButton->getName().toStdString());
	if (pButton == btn_EDLRegions){
		rSettings.CreateEDLFileRegion = btn_EDLRegions->getToggleState();
	}else if (pButton == btn_ExistingRegions){
		rSettings.CopyExistingRegions = btn_ExistingRegions->getToggleState();
	}else if (pButton == btn_ChangedShots){
		rSettings.CreateRegionsForChangedShots = btn_ChangedShots->getToggleState();
	}
};

void EDLSettingsCmp::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == dd_Framerate)
	{
		auto value = dd_Framerate->getText().getFloatValue();
		rSettings.framerate = value;
	}
};

void EDLSettingsCmp::labelTextChanged(Label* labelThatHasChanged)
{
	//PrintToConsole(labelThatHasChanged->getName().toStdString());
	if (labelThatHasChanged == txt_StartOffset){
		rSettings.timeLineOffset = txt_StartOffset->getText().toStdString();
	}
};



TransferSettingsCmp::TransferSettingsCmp(TransferSettingsStruct& inSettings) :rSettings(inSettings)
{

	for (const auto e : all_EventCreationOptions)
	{
		eventCreationOptions.push_back(e);
	}

	info->setText("Transfer To Wwise Settings", NotificationType::dontSendNotification);

	info_waapiPort->setText("Waapi Port:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_waapiPort);
	info_waapiPort->setTooltip("WAMP port to use for Wwise connection");
	txt_waapiPort->setEditable(true);
	txt_waapiPort->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	addAndMakeVisible(txt_waapiPort);
	info_waapiPort->attachToComponent(txt_waapiPort, true);
	txt_waapiPort->addListener(this);
	txt_waapiPort->setText(String(rSettings.waapiport), NotificationType::dontSendNotification);

	info_useAtomationMode->setText("Use Wwise Automation Mode:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_useAtomationMode);
	info_useAtomationMode->setTooltip("Use Wwise Automation mode for transfers");
	btn_useAtomationMode->addListener(this);
	addAndMakeVisible(btn_useAtomationMode);
	btn_useAtomationMode->setTooltip("Create a region with the name of the new EDL file.");
	btn_useAtomationMode->setToggleState(inSettings.useAtomationMode, NotificationType::dontSendNotification);
	info_useAtomationMode->attachToComponent(btn_useAtomationMode, true);

	info_userorigsubdir->setText("Default Originals Subfolder:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_userorigsubdir);
	info_userorigsubdir->setTooltip("Loaction for audio files inside Originals");
	txt_userorigsubdir->setEditable(true);
	txt_userorigsubdir->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	addAndMakeVisible(txt_userorigsubdir);
	info_userorigsubdir->attachToComponent(txt_userorigsubdir, true);
	txt_userorigsubdir->addListener(this);
	txt_userorigsubdir->setText(String(rSettings.userorigsubdir), NotificationType::dontSendNotification);

	info_versionToken->setText("Version Token:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_versionToken);
	info_versionToken->setTooltip("Suffix to indicate audio file version");
	txt_versionToken->setEditable(true);
	txt_versionToken->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	addAndMakeVisible(txt_versionToken);
	info_versionToken->attachToComponent(txt_versionToken, true);
	txt_versionToken->addListener(this);
	txt_versionToken->setText(String(rSettings.versionToken), NotificationType::dontSendNotification);
	
	info_templatePath->setText("Wwise Template Actor-Mixer Path:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_templatePath);
	info_templatePath->setTooltip("Location of Wwise template root");
	txt_templatePath->setEditable(true);
	txt_templatePath->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	addAndMakeVisible(txt_templatePath);
	info_templatePath->attachToComponent(txt_templatePath, true);
	txt_templatePath->addListener(this);
	txt_templatePath->setText(String(rSettings.templatePath), NotificationType::dontSendNotification);

	info_eventWorkUnitSuffix->setText("Event Work Unit Suffix:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_eventWorkUnitSuffix);
	info_eventWorkUnitSuffix->setTooltip("When mirroring the event structure, append an optional suffix to the event work units that are created");
	txt_eventWorkUnitSuffix->setEditable(true);
	txt_eventWorkUnitSuffix->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	addAndMakeVisible(txt_eventWorkUnitSuffix);
	info_eventWorkUnitSuffix->attachToComponent(txt_eventWorkUnitSuffix, true);
	txt_eventWorkUnitSuffix->addListener(this);
	txt_eventWorkUnitSuffix->setText(String(rSettings.eventWorkUnitSuffix), NotificationType::dontSendNotification);
	
	info_UserEventPath->setText("Custom/Default Event Path:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_UserEventPath);
	info_UserEventPath->setTooltip("Root path for events to be created in if using Default/Custom path mode");
	txt_UserEventPath->setEditable(true);
	txt_UserEventPath->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	addAndMakeVisible(txt_UserEventPath);
	info_UserEventPath->attachToComponent(txt_UserEventPath, true);
	txt_UserEventPath->addListener(this);
	txt_UserEventPath->setText(String(rSettings.UserEventPath), NotificationType::dontSendNotification);

	info_eventcreationoption->setText("Event Creation Method:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_eventcreationoption);
	info_eventcreationoption->setTooltip("How to organise events created by the Transfer");
	
	addAndMakeVisible(dd_eventcreationoption);
	info_eventcreationoption->attachToComponent(dd_eventcreationoption, true);
	InitComboBox(dd_eventcreationoption, eventCreationOptions);
	dd_eventcreationoption->setSelectedId(rSettings.eEventCreationOption + 1);//combo box index starts at 1
	
	info_eventMirrorDepth->setText("Event Mirroring Depth:", NotificationType::dontSendNotification);
	addAndMakeVisible(info_eventMirrorDepth);
	info_eventMirrorDepth->setTooltip("Depth limit when mirroring work units in the event hierarchy");
	txt_eventMirrorDepth->setEditable(true);
	txt_eventMirrorDepth->setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	addAndMakeVisible(txt_eventMirrorDepth);
	info_eventMirrorDepth->attachToComponent(txt_eventMirrorDepth, true);
	txt_eventMirrorDepth->addListener(this);
	txt_eventMirrorDepth->setText(String(rSettings.eventMirroringDepth), NotificationType::dontSendNotification);


}

void TransferSettingsCmp::resized()
{
	auto area = getBoundsInParent();
	auto width = getWidth();
	info->setBounds(area.removeFromTop(20));
	info->setJustificationType(Justification::centred);
	auto buffer = area.removeFromTop(30);
	auto area1 = area.removeFromTop(30);
	info_waapiPort->setBounds(area1.removeFromLeft(width/2));
	txt_waapiPort->setBounds(area1.reduced(3));
	
	buffer = area.removeFromTop(5);
	auto area2 = area.removeFromTop(30);
	info_useAtomationMode->setBounds(area2.removeFromLeft(width/2));
	btn_useAtomationMode->setBounds(area2.reduced(3));
	
	buffer = area.removeFromTop(5);
	auto area3 = area.removeFromTop(30);
	info_templatePath->setBounds(area3.removeFromLeft(width/2));
	txt_templatePath->setBounds(area3.reduced(3));
	
	buffer = area.removeFromTop(5);
	auto area4 = area.removeFromTop(30);
	info_versionToken->setBounds(area4.removeFromLeft(width/2));
	txt_versionToken->setBounds(area4.reduced(3));

	buffer = area.removeFromTop(5);
	auto area5 = area.removeFromTop(30);
	info_userorigsubdir->setBounds(area5.removeFromLeft(width/2));
	txt_userorigsubdir->setBounds(area5.reduced(3));
	
	buffer = area.removeFromTop(5);
	auto area6 = area.removeFromTop(30);
	info_eventcreationoption->setBounds(area6.removeFromLeft(width/2));
	dd_eventcreationoption->setBounds(area6.reduced(3));
	
	buffer = area.removeFromTop(5);
	auto area7 = area.removeFromTop(30);
	info_UserEventPath->setBounds(area7.removeFromLeft(width/2));
	txt_UserEventPath->setBounds(area7.reduced(3));
	
	buffer = area.removeFromTop(5);
	auto area8 = area.removeFromTop(30);
	info_eventWorkUnitSuffix->setBounds(area8.removeFromLeft(width/2));
	txt_eventWorkUnitSuffix->setBounds(area8.reduced(3));
	
	buffer = area.removeFromTop(5);
	auto area9 = area.removeFromTop(30);
	info_eventMirrorDepth->setBounds(area9.removeFromLeft(width/2));
	txt_eventMirrorDepth->setBounds(area9.reduced(3));
};

void TransferSettingsCmp::buttonClicked(juce::Button* pButton)
{
	//PrintToConsole(pButton->getName().toStdString());
	if (pButton == btn_useAtomationMode){
		rSettings.useAtomationMode = btn_useAtomationMode->getToggleState();
	}
};

void TransferSettingsCmp::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == dd_eventcreationoption)
	{
		int value = dd_eventcreationoption->getSelectedId()-1;
		rSettings.eEventCreationOption = (ETransferEventCreationOption)value;
	}
};

void TransferSettingsCmp::labelTextChanged(Label* labelThatHasChanged)
{
	//PrintToConsole(labelThatHasChanged->getName().toStdString());
	if (labelThatHasChanged == txt_waapiPort){
		rSettings.waapiport = txt_waapiPort->getText().getIntValue();
	}
	else if (labelThatHasChanged == txt_userorigsubdir)
	{
		rSettings.userorigsubdir = txt_userorigsubdir->getText().toStdString();
	}
	else if (labelThatHasChanged == txt_templatePath)
	{
		rSettings.templatePath = txt_templatePath->getText().toStdString();
	}
	else if (labelThatHasChanged == txt_versionToken)
	{
		rSettings.templatePath = txt_versionToken->getText().toStdString();
	}
	else if (labelThatHasChanged == txt_UserEventPath)
	{
		rSettings.UserEventPath = txt_UserEventPath->getText().toStdString();
	}
	else if (labelThatHasChanged == txt_eventWorkUnitSuffix)
	{
		rSettings.eventWorkUnitSuffix = txt_eventWorkUnitSuffix->getText().toStdString();
	}
	else if (labelThatHasChanged == txt_eventMirrorDepth)
	{
		if (txt_eventMirrorDepth->getText().isNotEmpty())
		{
			rSettings.eventMirroringDepth = std::stoi(txt_eventMirrorDepth->getText().toStdString());
		}
	}
};

void TransferSettingsCmp::handle_OnBecameActiveTab()
{
	//PrintToConsole("Settings active");
}
void TransferSettingsCmp::handle_OnTabBecameInactive()
{
	//PrintToConsole("Settings closed");
	
}
void TransferSettingsCmp::LoadSettingsFromExtState()
{
}
void TransferSettingsCmp::SaveSettingsToExtState()
{
}

void TransferSettingsCmp::UpdateSettingsValues(TransferSettingsStruct& inSettings)
{

}
