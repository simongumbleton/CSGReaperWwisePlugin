//
//  gui_assembler.cpp
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 10/01/2022.
//  Copyright © 2022 My Company. All rights reserved.
//

#include <stdio.h>
#include "gui_reconformer.h"

AssemblerComponent::AssemblerComponent()
{
	conformerComponent = new EDLconformer;
	InitAllButtons(buttons);
	addAndMakeVisible(btn_ChooseNewEDL);
	addAndMakeVisible(dragDropTarget01);
	//dragDropTarget01->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
	//dragDropTarget02->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
	dragDropTarget01->addListener(this);
	//dragDropTarget01->setBorderSize(BorderSize(20));
	//dragDropTarget02->setBorderSize(BorderSize(20));
	txt_NewEdlTxt->setText("Choose EDL File..", juce::NotificationType::dontSendNotification);

	addAndMakeVisible(txt_NewEdlTxt);
	addAndMakeVisible(txt_pluginVersion);
	txt_pluginVersion->setText(Reconformer::GetPluginVersionString(), juce::NotificationType::dontSendNotification);
	addAndMakeVisible(helpButton);

	txt_assemblerTitle->setText("Auto Assembly of Audio from EDL Animation Clip Info...", juce::NotificationType::dontSendNotification);
	txt_assemblerInfo->setText("Choose the location of audio files to use in assembly.", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(txt_assemblerTitle);
	addAndMakeVisible(txt_assemblerInfo);
	//dragDropWavFolderTarget->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(dragDropWavFolderTarget);
	addAndMakeVisible(btn_AssembleAudio);


	setSize(600, 350);

};


void AssemblerComponent::resized()
{
	auto area = getLocalBounds();
	auto titlearea = area.removeFromTop(20);
	helpButton->setBounds(titlearea.removeFromRight(40));

	txt_NewEdlTxt->setBounds(area.removeFromTop(25));
	auto newedlArea = area.removeFromTop(50);
	btn_ChooseNewEDL->setBounds(newedlArea.removeFromRight(100));
	dragDropTarget01->setBounds(newedlArea);


	auto buffer2 = area.removeFromTop(20);

	auto WavArea = area.removeFromTop(150);
	txt_assemblerTitle->setBounds(WavArea.removeFromTop(25));
	txt_assemblerInfo->setBounds(WavArea.removeFromTop(20));
	dragDropWavFolderTarget->setBounds(WavArea.removeFromTop(50));
	btn_AssembleAudio->setBounds(WavArea.removeFromTop(35));




	auto statusarea = area.removeFromBottom(40);
	txt_pluginVersion->setBounds(statusarea.removeFromRight(50).removeFromBottom(25));
}

void AssemblerComponent::buttonClicked(juce::Button* pButton) {
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug

	if (pButton == btn_ChooseNewEDL)
	{
		newEDLFilepath = PLATFORMHELPERS::askUserForFile("Choose an EDL File..");
		if (newEDLFilepath.hasFileExtension("edl"))
		{
			dragDropTarget01->setText(newEDLFilepath.getFullPathName(), juce::NotificationType::sendNotification);
		}
	}
	else if (pButton == btn_AssembleAudio)
	{
		//wavFilepath = askUserForFile("Load wav","*.wav");
		//wavDirpath = PLATFORMHELPERS::askUserForDirectory("Choose WAV location..");
		wavDirpath = dragDropWavFolderTarget->getText().toStdString();
		if (PLATFORMHELPERS::isPathDirectory(wavDirpath))
		{
			if (conformerComponent) {
				conformerComponent->InitiateDialogueAssembly(wavDirpath);
			}
			
		}
		else
		{
			PrintToConsole("Error! Assembly audio Location must be a valid folder path...");
			PrintToConsole(wavDirpath);
			dragDropWavFolderTarget->setText("Drag & drop here...", juce::NotificationType::dontSendNotification);
		}

	}
}

void AssemblerComponent::InitAllButtons(std::vector<juce::Button*> buttons)
{
	for (auto button : buttons)
	{
		String name = button->getName();
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);

	}
}

void AssemblerComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {

}

void AssemblerComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
	if (labelThatHasChanged == dragDropTarget01 )
	{
		if (dragDropTarget01->isSet())
		{
			//DrawPreviewConform();
			if (conformerComponent)
			{
				conformerComponent->filepath_New_EDL = dragDropTarget01->getText().toStdString();
			}
		}
	}
}