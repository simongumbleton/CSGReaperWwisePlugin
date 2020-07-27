#pragma once
#include "gui_Transfer.h"



StringArray TransferToWwiseComponent::ToJuceStringArray(std::vector<std::string> strings)
{
	StringArray output;
	for (auto string : strings)
	{
		output.add(string);
	}
	return output;
}


TransferToWwiseComponent::TransferToWwiseComponent() //constructor
{
	
	thisCreateImportWindow = new CreateImportWindow();
	thisCreateImportWindow->handleUI_B_Connect();
	
	MyCurrentWwiseConnection = &thisCreateImportWindow->WwiseConnectionHnd->MyCurrentWwiseConnection;
	
	// Init buttons and combo boxes
	InitAllButtons(buttons);
	
	InitComboBox(dd_Language, MyCurrentWwiseConnection->projectGlobals.Languages, "Language..");
	addAndMakeVisible(info_Language);
	info_Language->attachToComponent(dd_Language, true);
	info_Language->setText("Language: ", juce::NotificationType::dontSendNotification);
	
	InitComboBox(dd_EventOption, myCreateChoices.waapiCREATEchoices_EVENTOPTIONS,"Event Options..");
	addAndMakeVisible(info_EventOption);
	info_EventOption->attachToComponent(dd_EventOption, true);
	info_EventOption->setText("Create Events for: ", juce::NotificationType::dontSendNotification);
	
	InitComboBox(dd_CreateType,myCreateChoices.waapiCREATEchoices_TYPE,"Type..");
	addAndMakeVisible(info_CreateType);
	info_CreateType->attachToComponent(dd_CreateType, true);
	info_CreateType->setText("New Object Type: ", juce::NotificationType::dontSendNotification);
	
	InitComboBox(dd_OnNameConflict,myCreateChoices.waapiCREATEchoices_NAMECONFLICT,"On name conflict..");
	addAndMakeVisible(info_NameConflict);
	info_NameConflict->attachToComponent(dd_OnNameConflict, true);
	info_NameConflict->setText("On Name Conflict: ", juce::NotificationType::dontSendNotification);

	// Init render job tree view
	InitTreeView();
	
	
	addAndMakeVisible(INtxt_OriginalsSubDir);
	addAndMakeVisible(info_OriginalsSubDir);
	info_OriginalsSubDir->attachToComponent(INtxt_OriginalsSubDir, true);
	info_OriginalsSubDir->setText("Originals Sub Directory: ", juce::NotificationType::dontSendNotification);
	INtxt_OriginalsSubDir->setEditable(true);
	INtxt_OriginalsSubDir->setColour(juce::Label::backgroundColourId, Colours::lightgrey);
	
	addAndMakeVisible(INtxt_CreateName);
	addAndMakeVisible(info_CreateName);
	info_CreateName->attachToComponent(INtxt_CreateName, true);
	info_CreateName->setText("New Object Name:", juce::NotificationType::dontSendNotification);
	INtxt_CreateName->setEditable(true);
	INtxt_CreateName->setColour(juce::Label::backgroundColourId, Colours::lightgrey);
	
	addAndMakeVisible(INtxt_CreateNotes);
	addAndMakeVisible(info_CreateNotes);
	info_CreateNotes->attachToComponent(INtxt_CreateNotes, true);
	info_CreateNotes->setText("Notes:", juce::NotificationType::dontSendNotification);
	INtxt_CreateNotes->setEditable(true);
	INtxt_CreateNotes->setColour(juce::Label::backgroundColourId, Colours::lightgrey);
	
	
	
	addAndMakeVisible(txt_ConnectionStatus);

	debugLabel->setText("debug", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(debugLabel);


	setSize(1000, 500);
	
	CheckIsVoice();

}

TransferToWwiseComponent::~TransferToWwiseComponent()
{
	tree_RenderJobTree->deleteRootItem();
}

void TransferToWwiseComponent::InitAllButtons(std::vector<juce::Button *> buttons)
{
	for (auto button : buttons)
	{
		String name = button->getName();
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);
		
	}
}

void TransferToWwiseComponent::InitComboBox(juce::ComboBox * comboBox, std::vector<std::string> choices,string displayText)
{
	comboBox->addItemList(ToJuceStringArray(choices),1);
	comboBox->setTextWhenNothingSelected(displayText);
	comboBox->addListener(this);
	addAndMakeVisible(comboBox);
}

void TransferToWwiseComponent::InitTreeView()
{
	addAndMakeVisible(tree_RenderJobTree);
	tree_RenderJobTree->setColour(juce::TreeView::backgroundColourId, juce::Colours::lightgreen);
	tree_RenderJobTree->setColour(juce::TreeView::linesColourId, juce::Colours::red);
	tree_RenderJobTree->setColour(juce::TreeView::selectedItemBackgroundColourId, juce::Colours::lightcoral);
	tree_RenderJobTree->setDefaultOpenness(true);
	tree_RenderJobTree->setMultiSelectEnabled(true);
	tree_RenderJobTree->setRootItem(nullptr);
	tree_RenderJobTree->setRootItem(new RenderQueTreeRoot());
	tree_RenderJobTree->setRootItemVisible(false);
}

void TransferToWwiseComponent::resized()
{
	auto border = 4;
	auto area = getLocalBounds();

	auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);
	auto RightHalf = area;

	auto buttonHeight = 30;
	auto comboHeight = 30;
	auto treeHeight = 200;
	auto labelHeight = 30;
	
	auto TopRightQtr = RightHalf.removeFromTop(RightHalf.getHeight()/2);
	auto CreateCorner = TopRightQtr.removeFromRight(TopRightQtr.getWidth()/2);
	
	dd_CreateType->setBounds(CreateCorner.removeFromTop(comboHeight).reduced(border));
	INtxt_CreateName->setBounds(CreateCorner.removeFromTop(labelHeight).reduced(border));
	
	INtxt_CreateNotes->setBounds(CreateCorner.removeFromTop(labelHeight).reduced(border));
	dd_OnNameConflict->setBounds(CreateCorner.removeFromTop(comboHeight).reduced(border));
	
	btn_CreatePlayEvent->setBounds(CreateCorner.removeFromTop(buttonHeight).reduced(border));
	btn_CreateWwiseObject->setBounds(CreateCorner.removeFromTop(buttonHeight).reduced(border));
	
	
	
	
	for (auto button : buttons)
	{
		//button->setBounds(LeftHalf.removeFromTop(buttonHeight).reduced(border));
	}

	
	for (auto cb : comboBoxes)
	{
	//	cb->setBounds(RightHalf.removeFromTop(comboHeight).reduced(border));
	}

	

	tree_RenderJobTree->setBounds(LeftHalf.removeFromTop(treeHeight).reduced(border));

	
	txt_ConnectionStatus->setBounds(RightHalf.removeFromBottom(labelHeight));
	debugLabel->setBounds(RightHalf.removeFromBottom(labelHeight));
}



void TransferToWwiseComponent::buttonClicked(juce::Button * pButton)
{
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);

	if (pButton == btn_ConnectToWwise)
	{
		thisCreateImportWindow->handleUI_B_Connect();
		bool connected = MyCurrentWwiseConnection->connected;
		if (connected)
		{
			String text = ("Wwise Connected: " + MyCurrentWwiseConnection->Version);
			txt_ConnectionStatus->setText(text, juce::NotificationType::dontSendNotification);
		}
		else
		{
			txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
		}
	}
	else if (pButton == btn_RefreshJobList)
	{
		RefreshRenderJobTree();
	}
	else if (pButton == btn_isVoice)
	{
		CheckIsVoice();
	}
	else if (pButton == btn_ApplySettingsToJobs)
	{
		RenderTreeSelectedItems.clear();
		int selectedCount = tree_RenderJobTree->getNumSelectedItems();
		for (int i = 0; i<selectedCount;i++)
		{
			RenderTreeSelectedItems.push_back(tree_RenderJobTree->getSelectedItem(i));
		}
		
		String text = ("Num of selected items: " + String(RenderTreeSelectedItems.size()));
		debugLabel->setText(text, juce::NotificationType::dontSendNotification);
	}
	
	
}

void TransferToWwiseComponent::comboBoxChanged(ComboBox * comboBoxThatHasChanged)
{
	String text = ("SELECTED: " + comboBoxThatHasChanged->getText());
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);
}

void TransferToWwiseComponent::labelTextChanged(Label * labelThatHasChanged)
{
}

void TransferToWwiseComponent::CheckIsVoice() {
	if (btn_isVoice->getToggleState())
	{
		dd_Language->setEnabled(true);
	}
	else{
		dd_Language->setEnabled(false);
	}
}

void TransferToWwiseComponent::RefreshRenderJobTree()
{
	
	RenderQueTreeRoot* root = dynamic_cast<RenderQueTreeRoot*>(tree_RenderJobTree->getRootItem());
	root->clearSubItems();
	
	thisCreateImportWindow->FillRenderQueList();
	
	
	int jobIndex = 0;
	for (auto RenderJob : thisCreateImportWindow->GlobalListOfRenderQueJobs)
	{
		//Add the job to the tree
		string jobPath = RenderJob.RenderQueFilePath;
		RenderQueJobTreeItem* job = new RenderQueJobTreeItem(jobPath);
		
		for (auto renderFile : RenderJob.RenderQueJobFileList)
		{
			renderJobAudioFileTreeItem* wav = new renderJobAudioFileTreeItem(renderFile);
			job->addAudioFileToRenderJobTree(wav);
		}
		root->addRenderQueJobToTree(job,jobIndex);
		jobIndex++;
	}
}
