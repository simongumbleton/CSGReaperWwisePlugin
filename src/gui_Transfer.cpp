
#include "gui_Transfer.h"
#include "reaperHelpers.h"
#include <filesystem>
#include <mutex>

StringArray TransferToWwiseComponent::ToJuceStringArray(std::vector<std::string> strings)
{
	StringArray output;
	for (auto string : strings)
	{
		output.add(string);
	}
	return output;
}

TransferToWwiseComponent * TransferToWwiseComponent::currentTransferComponent = nullptr;
std::mutex mx_t;

TransferToWwiseComponent::TransferToWwiseComponent() //constructor
{
	
	thisCreateImportWindow = new CreateImportWindow();
	if (thisCreateImportWindow)
	{
		thisCreateImportWindow->owningGUIWindow = this;
	}
	WwiseCntnHndlr = thisCreateImportWindow->WwiseConnectionHnd;
	
	MyCurrentWwiseConnection = &thisCreateImportWindow->WwiseConnectionHnd->MyCurrentWwiseConnection;
	
	TransferToWwiseComponent::currentTransferComponent = this;
	
	// Init buttons and combo boxes
	InitAllButtons(buttons);
	
	InitComboBox(dd_Language, MyCurrentWwiseConnection->projectGlobals.Languages, "Language..");
	
	
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
	dd_OnNameConflict->setSelectedItemIndex(0, false);

	// Init render job tree view
	InitTreeView();
	
	
	addAndMakeVisible(INtxt_OriginalsSubDir);
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

	
	addAndMakeVisible(btn_isVoice);
	addAndMakeVisible(dd_Language);
	
	addAndMakeVisible(btn_OriginalsMatchesWwise);
	
	addAndMakeVisible(dd_EventOption);
	addAndMakeVisible(info_EventOption);
	info_EventOption->setText("Create events?", juce::NotificationType::dontSendNotification);
	//info_EventOption->attachToComponent(dd_EventOption, true);
	addAndMakeVisible(INtxt_OriginalsSubDir);

	
	addAndMakeVisible(btn_RenderAndImport);
	
	addAndMakeVisible(btn_RefreshJobList);
	
	addAndMakeVisible(btn_ApplySettingsToJobs);
	
	addAndMakeVisible(selectedParentLabel);
	selectedParentLabel->setText("Parent Name (Type):", juce::NotificationType::dontSendNotification);
	
	addAndMakeVisible(txt_ConnectionStatus);
	txt_ConnectionStatus->setText("Wwise Connection Status:", juce::NotificationType::dontSendNotification);
	
	addAndMakeVisible(statusLabel);
	statusLabel->setText("Status: ", juce::NotificationType::dontSendNotification);
	
	addAndMakeVisible(progressBar);

	setSize(1000, 500);
	
	thisCreateImportWindow->OnInitDlg();
	
	TryConnectToWwise();
	CheckIsVoice();
	CheckOriginalsDirectory();
	RefreshRenderJobTree();
	updateProgressValue(0);
	setStatusText("Ready..");
	

}

TransferToWwiseComponent::~TransferToWwiseComponent()
{
	tree_RenderJobTree->deleteRootItem();
	WwiseCntnHndlr->UnsubscribeFromTopicByID(1);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(2);
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
	comboBox->clear();
	comboBox->addItemList(ToJuceStringArray(choices),1);
	comboBox->setTextWhenNothingSelected(displayText);
	comboBox->addListener(this);
	comboBox->setSelectedItemIndex(0, false);
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
	auto treeHeight = 200;
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
	
	selectedParentLabel->setBounds(LeftHalf.removeFromTop(labelHeight).reduced(border));
	
	auto optionsArea1 = LeftHalf.removeFromTop(buttonHeight).reduced(border);
	
	auto optionsArea2 = LeftHalf.removeFromTop(buttonHeight).reduced(border);
	
	auto o1qtrsize = optionsArea1.getWidth()/4;
	
	btn_isVoice->setBounds(optionsArea1.removeFromLeft(o1qtrsize).reduced(border/2));
	
	dd_Language->setBounds(optionsArea1.removeFromLeft(o1qtrsize).reduced(border/2));
	
	info_EventOption->setBounds(optionsArea1.removeFromLeft(o1qtrsize).reduced(border/2));
	
	dd_EventOption->setBounds(optionsArea1.reduced(border/2));
	
	btn_OriginalsMatchesWwise->setBounds(optionsArea2.removeFromLeft(o1qtrsize * 1.5).reduced(border/2));
	
	INtxt_OriginalsSubDir->setBounds(optionsArea2.reduced(border/2));
	INtxt_OriginalsSubDir->setText("ImportedFromReaper/", juce::NotificationType::dontSendNotification);
	
	
	btn_ApplySettingsToJobs->setBounds(LeftHalf.removeFromTop(labelHeight).reduced(border));
	
	
	auto bottomRow = LeftHalf.removeFromBottom(buttonHeight).reduced(border);
	
	btn_ConnectToWwise->setBounds(bottomRow.removeFromLeft(bottomRow.getWidth()/2));
	
	txt_ConnectionStatus->setBounds(bottomRow);
	
	progressBar->setBounds(LeftHalf.removeFromBottom(buttonHeight).reduced(border));
	
	
	auto RenderButtonArea = LeftHalf.removeFromBottom(buttonHeight*2).reduced(border);
	
	btn_RefreshJobList->setBounds(RenderButtonArea.removeFromLeft(RenderButtonArea.getWidth()/2).reduced(border));
	
	btn_RenderAndImport->setBounds(RenderButtonArea.reduced(border));
	
	//tree_RenderJobTree->setBounds(LeftHalf.removeFromTop(treeHeight).reduced(border));
	
	tree_RenderJobTree->setBounds(LeftHalf.reduced(border));
	
	
	
	
	auto statusRow = RightHalf.removeFromBottom(labelHeight).reduced(border);
	
	statusLabel->setBounds(statusRow.removeFromLeft(statusRow.getWidth()/2).reduced(border));
	debugLabel->setBounds(statusRow);
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
	
	WwiseObject selectedParent = WwiseCntnHndlr->GetSelectedObject();
	std::string parentWwiseID = selectedParent.properties["id"];
	std::string parentWwiseName = selectedParent.properties["name"];
	std::string parentWwiseType = selectedParent.properties["type"];
	std::string parentWwisePath = selectedParent.properties["path"];

	if (parentWwiseID == "")
	{
		PrintToConsole("Error - No valid Wwise Parent");
		return;	// Invalid wwise parent selection!
	}

	if (parentWwisePath.find("Actor-Mixer Hierarchy\\") == parentWwisePath.npos)
	{
		PrintToConsole("Import parent must be in Actor-Mixer hierarchy");
		return;	// Invalid wwise parent selection!
	}
	if (RenderTreeSelectedItems.size() == 0) // Nothing selected
	{
		PrintToConsole("No render job or file selected");
		return;
	}
	
	for (auto selectedItem : RenderTreeSelectedItems)
	{
		//RenderQueJobTreeItem
		//renderJobAudioFileTreeItem
		bool isItemWav;
		std::string currentName;
		RenderQueJobTreeItem *jobItem = dynamic_cast<RenderQueJobTreeItem *>(selectedItem);
		renderJobAudioFileTreeItem *wavItem = dynamic_cast<renderJobAudioFileTreeItem *>(selectedItem);
		if (jobItem)
		{
			isItemWav = false;
			currentName = jobItem->renderJobPath;
			jobItem->importText = "";
		}
		else if (wavItem)
		{
			isItemWav = true;
			currentName = wavItem->renderWav;
			wavItem->importText = "";
		}
		else
		{
			PrintToConsole("Error");
			return;
		}
		
		std::string jobName = "";
		if (isItemWav) {
			RenderQueJobTreeItem *parent = dynamic_cast<RenderQueJobTreeItem *>(wavItem->getParentItem());
			if (parent)
			{
				jobName = parent->renderJobPath;
			}
			
		}
		else {
			jobName = jobItem->renderJobPath;
		}
		int count = 0;
		for (auto &renderJob : thisCreateImportWindow->GlobalListOfRenderQueJobs)
		{
			std::filesystem::path filePath = renderJob.RenderQueFilePath;
			std::string filename = filePath.filename().string();
			if (jobName.find(filename) != jobName.npos)
			{
				//Found a match
				//PrintToConsole("Found a match");
				if (isItemWav) {
					renderJob.hasPerFileOverrides = true;
					RenderJobFileOverride fileOverride;
					fileOverride.RenderJobFile = currentName;
					fileOverride.parentWwiseObject = selectedParent;
					fileOverride.isVoice = CheckIsVoice();
					fileOverride.createEventOption = dd_EventOption->getText().toStdString();
					fileOverride.OrigDirMatchesWwise = CheckOriginalsDirectory();
					if (!fileOverride.OrigDirMatchesWwise)
					{
						fileOverride.userOrigsSubDir = INtxt_OriginalsSubDir->getText().toStdString();
					}

					std::string language;
					if (fileOverride.isVoice)
					{
						fileOverride.ImportLanguage = dd_Language->getText().toStdString();
						language = fileOverride.ImportLanguage;
					}
					else
					{
						fileOverride.ImportLanguage = "SFX";
						language = "SFX";
					}

					renderJob.perFileOverridesmap[fileOverride.RenderJobFile] = fileOverride;
					
					wavItem->importText = " -> " + parentWwiseName + " (" + parentWwiseType + ")";
				}
				else
				{

					renderJob.parentWwiseObject = selectedParent;

					renderJob.isVoice = CheckIsVoice();

					renderJob.createEventOption = dd_EventOption->getText().toStdString();

					renderJob.OrigDirMatchesWwise = CheckOriginalsDirectory();

					if (!renderJob.OrigDirMatchesWwise)
					{
						renderJob.userOrigsSubDir = INtxt_OriginalsSubDir->getText().toStdString();
					}

					std::string language;
					if (renderJob.isVoice)
					{
						renderJob.ImportLanguage = dd_Language->getText().toStdString();
						language = renderJob.ImportLanguage;
					}
					else
					{
						renderJob.ImportLanguage = "SFX";
						language = "SFX";
					}
					jobItem->importText = parentWwiseName + "(" + parentWwiseType + ") - ";
				}
			}
		}
		
	}
	
	//Set status ready to import
	tree_RenderJobTree->clearSelectedItems();


}

void TransferToWwiseComponent::TryConnectToWwise() {
	thisCreateImportWindow->handleUI_B_Connect();
	bool connected = MyCurrentWwiseConnection->connected;
	if ((connected)&&(MyCurrentWwiseConnection->projectGlobals.ProjectPath != ""))
	{
		String text = ("Wwise Connected: " + MyCurrentWwiseConnection->projectGlobals.ProjectName);
		txt_ConnectionStatus->setText(text, juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->SubscribeOnSelectionChanged(TransferToWwiseComponent::callback_OnSelectionChanged, 1);
		WwiseCntnHndlr->SubscribeOnProjectClosed(TransferToWwiseComponent::callback_OnProjectClosed, 2);
		InitComboBox(dd_Language, MyCurrentWwiseConnection->projectGlobals.Languages, "Language..");
		handle_OnSelectedParentChanged();
	}
	else
	{
		txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(1);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(2);
		WwiseCntnHndlr->DisconnectFromWwise();
		
	}
}

void TransferToWwiseComponent::buttonClicked(juce::Button * pButton)
{
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);

	if (pButton == btn_ConnectToWwise)
	{
		TryConnectToWwise();
	}
	else if (pButton == btn_RefreshJobList)
	{
		RefreshRenderJobTree();
		updateProgressValue(0);
		setStatusText("Ready..");
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
	else if (pButton == btn_OriginalsMatchesWwise)
	{
		CheckOriginalsDirectory();
	}
	else if (pButton == btn_RenderAndImport)
	{
		//thisCreateImportWindow->backupRenderQueFiles();
		if (thisCreateImportWindow->handleUI_RenderImport())
		{
			RefreshRenderJobTree();
		}
		//thisCreateImportWindow->restoreRenderQueFiles();
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

bool TransferToWwiseComponent::CheckIsVoice() {
	if (btn_isVoice->getToggleState())
	{
		dd_Language->setEnabled(true);
		return true;
	}
	else{
		dd_Language->setEnabled(false);
		return false;
	}
}

bool TransferToWwiseComponent::CheckOriginalsDirectory() {
	if (btn_OriginalsMatchesWwise->getToggleState())
	{
		INtxt_OriginalsSubDir->setEnabled(false);
		INtxt_OriginalsSubDir->setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
		return true;
	}
	else{
		INtxt_OriginalsSubDir->setEnabled(true);
		INtxt_OriginalsSubDir->setColour(juce::Label::backgroundColourId, juce::Colours::dimgrey);
		return false;
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

void TransferToWwiseComponent::handle_OnSelectedParentChanged()
{
	const std::lock_guard<std::mutex> lock(mx_t);
	//std::cout << "_____Callback 1______" << std::endl;
	WwiseObject newParent = WwiseCntnHndlr->GetSelectedObject();
	//std::cout << newParent.properties["name"] << std::endl;
	//std::string display = "Selected Parent: " + newParent.properties["name"];
	//MessageManagerLock mml(Thread::getCurrentThread());
	std::string display = "Selected Parent: " +newParent.properties["name"] + " ("+newParent.properties["type"]+")";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}

void TransferToWwiseComponent::handle_OnWwiseProjectClosed()
{
	const std::lock_guard<std::mutex> lock(mx_t);
	std::cout << "_____Callback 2______" << std::endl;
	txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(1);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(2);
	WwiseCntnHndlr->DisconnectFromWwise();
	std::string display = "Selected Parent: ";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}

