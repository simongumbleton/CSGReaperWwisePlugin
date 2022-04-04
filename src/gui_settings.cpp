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
	
};

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
