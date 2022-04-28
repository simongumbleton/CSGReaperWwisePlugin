//
//  gui_reconformer.cpp
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 10/01/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#include <stdio.h>
#include "gui_reconformer.h"


ConformerComponent::ConformerComponent()
{
	conformerComponent = new EDLconformer;
	InitAllButtons(buttons);
	addAndMakeVisible(btn_ChooseOldEDL);
	addAndMakeVisible(btn_ChooseNewEDL);
	addAndMakeVisible(dragDropTarget01);
	addAndMakeVisible(dragDropTarget02);
	//dragDropTarget01->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
	//dragDropTarget02->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(btn_DoConform);
	dragDropTarget01->addListener(this);
	dragDropTarget02->addListener(this);
	//dragDropTarget01->setBorderSize(BorderSize(20));
	//dragDropTarget02->setBorderSize(BorderSize(20));
	addAndMakeVisible(RegionPreview);
	txt_OldEdlTxt->setText("Choose Old EDL File..", juce::NotificationType::dontSendNotification);
	txt_NewEdlTxt->setText("Choose New EDL File..", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(txt_OldEdlTxt);
	addAndMakeVisible(txt_NewEdlTxt);
	addAndMakeVisible(txt_preview);
	addAndMakeVisible(txt_pluginVersion);
	txt_pluginVersion->setText(Reconformer::GetPluginVersionString(), juce::NotificationType::dontSendNotification);
	addAndMakeVisible(helpButton);
	
//	txt_assemblerTitle->setText("Auto Assembly of Audio from EDL Animation Clip Info...", juce::NotificationType::dontSendNotification);
//	txt_assemblerInfo->setText("Choose the location of audio files to use in assembly.", juce::NotificationType::dontSendNotification);
//	addAndMakeVisible(txt_assemblerTitle);
//	addAndMakeVisible(txt_assemblerInfo);
	//dragDropWavFolderTarget->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
//	addAndMakeVisible(dragDropWavFolderTarget);
//	addAndMakeVisible(btn_AssembleAudio);
	
	
	setSize(600, 350);
	
	addAndMakeVisible(btn_Settings);
	
};


void ConformerComponent::resized()
{
	auto area = getLocalBounds();
	auto titlearea = area.removeFromTop(20);
	helpButton->setBounds(titlearea.removeFromRight(40));
	txt_OldEdlTxt->setBounds(area.removeFromTop(25));
	auto oldedlArea = area.removeFromTop(50);
	btn_ChooseOldEDL->setBounds(oldedlArea.removeFromRight(100));
	dragDropTarget01->setBounds(oldedlArea);
	
	txt_NewEdlTxt->setBounds(area.removeFromTop(25));
	auto newedlArea = area.removeFromTop(50);
	btn_ChooseNewEDL->setBounds(newedlArea.removeFromRight(100));
	dragDropTarget02->setBounds(newedlArea);
	
	txt_preview->setBounds(area.removeFromTop(25));
	RegionPreview->setSize(getWidth(), 50);
	RegionPreview->setBounds(area.removeFromTop(50));
	auto buffer1 = area.removeFromTop(10);
	btn_DoConform->setBounds(area.removeFromTop(50));
	
	auto buffer2 = area.removeFromTop(20);

//	auto WavArea = area.removeFromTop(150);
//	txt_assemblerTitle->setBounds(WavArea.removeFromTop(25));
//	txt_assemblerInfo->setBounds(WavArea.removeFromTop(20));
//	dragDropWavFolderTarget->setBounds(WavArea.removeFromTop(50));
//	btn_AssembleAudio->setBounds(WavArea.removeFromTop(35));
	
	
	
	
	auto statusarea = area.removeFromBottom(40);
	txt_pluginVersion->setBounds(statusarea.removeFromRight(50).removeFromBottom(25));
	btn_Settings->setBounds(statusarea.removeFromLeft(40));
}

void ConformerComponent::buttonClicked(juce::Button *pButton) { 
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug

	if (pButton == btn_ChooseOldEDL)
	{
		oldEDLFilepath = PLATFORMHELPERS::askUserForFile("Choose OLD EDL File..");
		if (oldEDLFilepath.hasFileExtension("edl"))
		{
			dragDropTarget01->setText(oldEDLFilepath.getFullPathName(), juce::NotificationType::sendNotification);
		}
	}
	else if (pButton == btn_ChooseNewEDL)
	{
		newEDLFilepath = PLATFORMHELPERS::askUserForFile("Choose NEW EDL File..");
		if (newEDLFilepath.hasFileExtension("edl"))
		{
			dragDropTarget02->setText(newEDLFilepath.getFullPathName(), juce::NotificationType::sendNotification);
		}
	}
	else if (pButton == btn_DoConform)
	{
		TriggerConform();
	}
	/*
	else if (pButton == btn_AssembleAudio)
	{
		//wavFilepath = askUserForFile("Load wav","*.wav");
		//wavDirpath = PLATFORMHELPERS::askUserForDirectory("Choose WAV location..");
		wavDirpath = dragDropWavFolderTarget->getText().toStdString();
		if (PLATFORMHELPERS::isPathDirectory(wavDirpath))
		{
			conformerComponent->InitiateDialogueAssembly(wavDirpath);
		}
		else
		{
			PrintToConsole("Error! Assembly audio Location must be a valid folder path...");
			PrintToConsole(wavDirpath);
			dragDropWavFolderTarget->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
		}
		
	}*/
	else if (pButton == btn_Settings)
	{
		LaunchSettingsWindow();
	}
}

void ConformerComponent::InitAllButtons(std::vector<juce::Button *> buttons)
{
	for (auto button : buttons)
	{
		String name = button->getName();
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);
		
	}
}

void ConformerComponent::comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) { 
	
}

void ConformerComponent::labelTextChanged(juce::Label *labelThatHasChanged) {
	if (labelThatHasChanged == dragDropTarget01 or labelThatHasChanged == dragDropTarget02)
	{
		if (dragDropTarget01->isSet() && dragDropTarget02->isSet())
		{
			DrawPreviewConform();
		}
	}
}

void ConformerComponent::TriggerConform() {
	if (dragDropTarget01->getText().isEmpty() or dragDropTarget02->getText().isEmpty())
	{
		PrintToConsole("Warning! One or more EDL file paths not set. Make sure to provide both OLD and NEW edl files");
		return;
	}
	
	if (conformerComponent)
	{
		conformerComponent->filepath_Old_EDL = dragDropTarget01->getText().toStdString();
		conformerComponent->filepath_New_EDL = dragDropTarget02->getText().toStdString();
		conformerComponent->DoConform();
	}
}

void ConformerComponent::DrawPreviewConform() { 
	if (conformerComponent)
	{
		conformerComponent->filepath_Old_EDL = dragDropTarget01->getText().toStdString();
		conformerComponent->filepath_New_EDL = dragDropTarget02->getText().toStdString();
		txt_preview->setText("", juce::NotificationType::dontSendNotification);
		repaint();
		RegionPreview->ClearRegions();
		RegionPreview->repaint();
		if (conformerComponent->SetupConform())
		{
			float newEndTime = conformerComponent->GetNewEndTime();
			if (newEndTime == 0.0f) return;
			PrintToConsole("Previewing conform.." + std::to_string(newEndTime));
			RegionPreview->maxDuration = newEndTime;
			for (auto region : conformerComponent->changedSections)
			{
				
				RegionPreview->AddRegion(
										 conformerComponent->TimecodeToSeconds(region.destStartTC),
										 conformerComponent->TimecodeToSeconds(region.destEndTC),
										 true);
			}
			for (auto region : conformerComponent->unchangedSections)
			{
				
				RegionPreview->AddRegion(
										 conformerComponent->TimecodeToSeconds(region.destStartTC),
										 conformerComponent->TimecodeToSeconds(region.destEndTC),
										 false);
			}
			for (auto region : conformerComponent->animClipChangedSections)
			{
				RegionPreview->AddRegionForAnimChange(
										 conformerComponent->TimecodeToSeconds(region.destStartTC),
										 conformerComponent->TimecodeToSeconds(region.destEndTC));
			}
			txt_preview->setText("Preview of conform...", juce::NotificationType::dontSendNotification);
			RegionPreview->repaint();
		}
	}
}

void ConformerComponent::GetCurrentSettings() {
	
}


void ConformerComponent::SaveSettings() { 
	
}


void ConformerComponent::LaunchSettingsWindow() {
	if (windowStatus) return;
	settingsWndHndl = new EDLSettingsWnd("settings", conformerComponent->EdlCompSettings,windowStatus);
	addAndMakeVisible(settingsWndHndl);
	settingsWndHndl->centreWithSize(300, 400);
	
}





