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
	conformer = new EDLconformer;
	InitAllButtons(buttons);
	addAndMakeVisible(btn_ChooseOldEDL);
	addAndMakeVisible(btn_ChooseNewEDL);
	
	setSize(1000, 600);
};


void ConformerComponent::resized()
{
	auto area = getLocalBounds();
	btn_ChooseOldEDL->setBounds(area.removeFromTop(50));
	btn_ChooseNewEDL->setBounds(area.removeFromTop(50));
}

void ConformerComponent::buttonClicked(juce::Button *pButton) { 
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug

	if (pButton == btn_ChooseOldEDL)
	{
		oldEDLFilepath = askUserForFile("Choose OLD EDL File..");
	}
	else if (pButton == btn_ChooseNewEDL)
	{
		newEDLFilepath = askUserForFile("Choose NEW EDL File..");
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
	
}


