
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
	INtxt_CreateName->setColour(juce::Label::backgroundColourId, Colours::darkgrey);
	
	addAndMakeVisible(INtxt_CreateNotes);
	addAndMakeVisible(info_CreateNotes);
	info_CreateNotes->attachToComponent(INtxt_CreateNotes, true);
	info_CreateNotes->setText("Notes:", juce::NotificationType::dontSendNotification);
	INtxt_CreateNotes->setEditable(true);
	INtxt_CreateNotes->setColour(juce::Label::backgroundColourId, Colours::darkgrey);
	
	
	addAndMakeVisible(Title_CreateWwiseObject);
	Title_CreateWwiseObject->setFont(Font(16.0f,Font::bold));
	Title_CreateWwiseObject->setText("Create a new Wwise object under current selection", dontSendNotification);
	Title_CreateWwiseObject->setColour(Label::textColourId, Colours::white);
	Title_CreateWwiseObject->setJustificationType(Justification::centred);
	
	addAndMakeVisible(Title_RenderImport);
	Title_RenderImport->setFont(Font(16.0f,Font::bold));
	Title_RenderImport->setText("Select a Wwise parent, apply settings to jobs and render/import", dontSendNotification);
	Title_RenderImport->setColour(Label::textColourId, Colours::white);
	Title_RenderImport->setJustificationType(Justification::centred);
	
	addAndMakeVisible(txt_ConnectionStatus);

	debugLabel->setText("debug", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(debugLabel);


	setSize(1000, 500);
	
	CheckIsVoice();
	RefreshRenderJobTree();

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
	auto buttonHeight = 30;
	auto comboHeight = 30;
	auto treeHeight = 250;
	auto labelHeight = 30;
	auto titleHeight = 60;
	auto area = getLocalBounds();

	auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);
	auto RightHalf = area;

	
	
	auto TopRightQtr = RightHalf.removeFromTop(RightHalf.getHeight()/2);
	//auto CreateCorner = TopRightQtr.removeFromRight(TopRightQtr.getWidth()/1.5);
	
	Title_CreateWwiseObject->setBounds(TopRightQtr.removeFromTop(titleHeight).reduced(border));
	
	
	dd_CreateType->setBounds(TopRightQtr.removeFromTop(comboHeight).reduced(border).removeFromRight(TopRightQtr.getWidth()/2));
	//dd_CreateType->setJustificationType(Justification::right);
	
	INtxt_CreateName->setBounds(TopRightQtr.removeFromTop(labelHeight).reduced(border).removeFromRight(TopRightQtr.getWidth()/2));
	//INtxt_CreateName->setJustificationType(Justification::right);
	
	INtxt_CreateNotes->setBounds(TopRightQtr.removeFromTop(labelHeight).reduced(border).removeFromRight(TopRightQtr.getWidth()/2));
	//INtxt_CreateNotes->setJustificationType(Justification::right);
	dd_OnNameConflict->setBounds(TopRightQtr.removeFromTop(comboHeight).reduced(border).removeFromRight(TopRightQtr.getWidth()/2));
	//dd_OnNameConflict->setJustificationType(Justification::right);
	
	
	btn_CreatePlayEvent->setBounds(TopRightQtr.removeFromTop(buttonHeight).reduced(border).removeFromRight(TopRightQtr.getWidth()/2));
	
	auto buttonArea = TopRightQtr.removeFromTop(buttonHeight).reduced(border);
	auto edgesize = buttonArea.getWidth()*0.1;
	auto offsetL = buttonArea.removeFromLeft(edgesize);
	auto offsetR = buttonArea.removeFromRight(edgesize);
	btn_CreateWwiseObject->setBounds(buttonArea);
	btn_CreateWwiseObject->setSize(buttonArea.getWidth(), buttonHeight);
	
	//auto TopLeftQtr = LeftHalf.removeFromTop(LeftHalf.getHeight()/2);
	
	Title_RenderImport->setBounds(LeftHalf.removeFromTop(titleHeight).reduced(border));
	

	

	tree_RenderJobTree->setBounds(LeftHalf.removeFromTop(treeHeight).reduced(border));

	
	txt_ConnectionStatus->setBounds(RightHalf.removeFromBottom(labelHeight));
	debugLabel->setBounds(RightHalf.removeFromBottom(labelHeight));
}



void TransferToWwiseComponent::ApplySettingsToSelectedJobs() {
	RenderTreeSelectedItems.clear();
	int selectedCount = tree_RenderJobTree->getNumSelectedItems();
	for (int i = 0; i<selectedCount;i++)
	{
		RenderTreeSelectedItems.push_back(tree_RenderJobTree->getSelectedItem(i));
	}
	
	String text = ("Num of selected items: " + String(RenderTreeSelectedItems.size()));
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);




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
		ApplySettingsToSelectedJobs();
	}
	else if (pButton == btn_CreateWwiseObject)
	{
		handleUI_B_CreateObject();
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

void TransferToWwiseComponent::handleUI_B_CreateObject()
{

	//PrintToConsole("Creating New Wwise Object");
	CreateObjectArgs myCreateObjectArgs;
	
	myCreateObjectArgs.Type = GetDropDownValue(dd_CreateType);
	myCreateObjectArgs.onNameConflict = GetDropDownValue(dd_OnNameConflict);
	
	myCreateObjectArgs.Name = GetLabelValue(INtxt_CreateName);
	myCreateObjectArgs.Notes = GetLabelValue(INtxt_CreateNotes);

	myCreateObjectArgs.createPlayEvent = GetToggleValue(btn_CreatePlayEvent);
	
	//TODO notes doesnt work needs changing in waapi code
	//TODO creating a voice object doesnt work either
	thisCreateImportWindow->handleUI_B_CreateObject(myCreateObjectArgs);

}


std::string TransferToWwiseComponent::GetDropDownValue(juce::ComboBox * dropdown)
{
	int Index = dropdown->getSelectedItemIndex();
	return dropdown->getItemText(Index).toStdString();
}

std::string TransferToWwiseComponent::GetLabelValue(juce::Label * label)
{
	return label->getText().toStdString();
}

bool TransferToWwiseComponent::GetToggleValue(juce::ToggleButton * btn)
{
	return btn->getToggleState();
}
