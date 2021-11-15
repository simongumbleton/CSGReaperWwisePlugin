
#include "gui_Transfer.h"
#include "reaperHelpers.h"
#include "platformhelpers.h"
#include "RegionMetadataHelper.h"
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

TransferToWwiseComponent::TransferToWwiseComponent(juce::Component* parentComp) //constructor
{
	parent = dynamic_cast<TransferTabComponent*>(parentComp);
	if (parent)
	{
		thisCreateImportWindow = parent->thisCreateImportWindow;// new CreateImportWindow();
		if (thisCreateImportWindow)
		{
			thisCreateImportWindow->owningGUIWindow = this;
		}
		WwiseCntnHndlr = thisCreateImportWindow->WwiseConnectionHnd;
		
		MyCurrentWwiseConnection = &thisCreateImportWindow->WwiseConnectionHnd->MyCurrentWwiseConnection;
	}
	
	TransferToWwiseComponent::currentTransferComponent = this;
	
	
	// Init buttons and combo boxes
	InitAllButtons(buttons);
	
	InitComboBox(dd_Language, MyCurrentWwiseConnection->projectGlobals.Languages, "Language..");
	
	
	InitComboBox(dd_EventOption, myCreateChoices.waapiCREATEchoices_EVENTOPTIONS,"Event Options..");
	addAndMakeVisible(info_EventOption);
	info_EventOption->attachToComponent(dd_EventOption, true);
	info_EventOption->setText("Create Events for: ", juce::NotificationType::dontSendNotification);
	
	
	InitComboBox(dd_ImportType, myCreateChoices.waapiCREATEchoices_IMPORTTYPE,"Sfx");
	addAndMakeVisible(info_ImportType);
	info_ImportType->attachToComponent(dd_ImportType, true);
	info_ImportType->setText("Import as: ", juce::NotificationType::dontSendNotification);
	
	
	// Init render job tree view
	InitTreeView();
	
	
	addAndMakeVisible(INtxt_OriginalsSubDir);
	INtxt_OriginalsSubDir->setEditable(true);
	INtxt_OriginalsSubDir->setColour(juce::Label::backgroundColourId, Colours::lightgrey);
	
		
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
	addAndMakeVisible(txt_pluginVersion);

	setSize(1000, 600);
	
	//thisCreateImportWindow->OnInitDlg();
	//TryConnectToWwise();
	CheckIsVoice();
	CheckOriginalsDirectory();
	RefreshRenderJobTree();
	updateProgressValue(0);
	
	ReadConfigFile(myConfig);
	setTransferValuesFromConfig(myConfig);
	
	setStatusText("Ready..");
}

TransferToWwiseComponent::~TransferToWwiseComponent()
{
	tree_RenderJobTree->deleteRootItem();
	WwiseCntnHndlr->RemoveActiveComponent(this);
	parent = nullptr;
	TransferToWwiseComponent::currentTransferComponent = nullptr;
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

	//auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);
	//auto RightHalf = area;

	
	
	//auto TopRightQtr = RightHalf.removeFromTop(RightHalf.getHeight()/2);
	//auto CreateCorner = TopRightQtr.removeFromRight(TopRightQtr.getWidth()/1.5);
	
	//auto buttonArea = TopRightQtr.removeFromTop(buttonHeight).reduced(border);
	//auto edgesize = buttonArea.getWidth()*0.1;
	//auto offsetL = buttonArea.removeFromLeft(edgesize);
	//auto offsetR = buttonArea.removeFromRight(edgesize);
	
	//auto TopLeftQtr = LeftHalf.removeFromTop(LeftHalf.getHeight()/2);
	auto titleArea = area.removeFromTop(titleHeight).reduced(border);
	//txt_pluginVersion->setBounds(titleArea.removeFromRight(50));
	Title_RenderImport->setBounds(titleArea);
	
	selectedParentLabel->setBounds(area.removeFromTop(labelHeight).reduced(border));
	
	auto optionsArea1 = area.removeFromTop(buttonHeight).reduced(border);
	
	auto optionsArea2 = area.removeFromTop(buttonHeight).reduced(border);
	
	auto o1qtrsize = optionsArea1.getWidth()/4;
	
	//btn_isVoice->setBounds(optionsArea1.removeFromLeft(o1qtrsize).reduced(border/2));
	
	auto buffer = optionsArea1.removeFromLeft(75).reduced(border/2);
	
	dd_ImportType->setBounds(optionsArea1.removeFromLeft(o1qtrsize/2).reduced(border/2));
	
	dd_Language->setBounds(optionsArea1.removeFromLeft(o1qtrsize).reduced(border/2));
	
	info_EventOption->setBounds(optionsArea1.removeFromLeft(o1qtrsize).reduced(border/2));
	
	dd_EventOption->setBounds(optionsArea1.reduced(border/2));
	
	btn_OriginalsMatchesWwise->setBounds(optionsArea2.removeFromLeft(o1qtrsize * 1.5).reduced(border/2));
	
	auto originalsTextArea = optionsArea2.removeFromLeft(optionsArea2.getWidth()*0.7);
	
	INtxt_OriginalsSubDir->setBounds(originalsTextArea.reduced(border/2));
	//INtxt_OriginalsSubDir->setText("ImportedFromReaper/", juce::NotificationType::dontSendNotification);
	
	btn_ChooseWwiseOriginalsDir->setBounds(optionsArea2);
	
	btn_ApplySettingsToJobs->setBounds(area.removeFromTop(labelHeight*1.5).reduced(border));
	
	auto statusRow = area.removeFromBottom(labelHeight).reduced(border);
	txt_pluginVersion->setBounds(statusRow.removeFromRight(50));
	
	auto statusLeft = statusRow.removeFromLeft(statusRow.getWidth()/2);
	
	statusLabel->setBounds(statusRow.removeFromLeft(statusRow.getWidth()/2).reduced(border));
	debugLabel->setBounds(statusRow);
	
	btn_ConnectToWwise->setBounds(statusLeft.removeFromLeft(statusLeft.getWidth()/2));
	
	txt_ConnectionStatus->setBounds(statusLeft);
	
	progressBar->setBounds(area.removeFromBottom(buttonHeight).reduced(border));
	
	auto RenderButtonArea = area.removeFromBottom(buttonHeight*1.5).reduced(border);
	
	btn_RefreshJobList->setBounds(RenderButtonArea.removeFromLeft(RenderButtonArea.getWidth()/2).reduced(border));
	
	btn_RenderAndImport->setBounds(RenderButtonArea.reduced(border));
	
	//tree_RenderJobTree->setBounds(LeftHalf.removeFromTop(treeHeight).reduced(border));
	
	tree_RenderJobTree->setBounds(area.reduced(border));

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

	if (GetImportType()==IMPORT_TYPE::Music)
	{
		if (parentWwisePath.find("Interactive Music Hierarchy\\") == parentWwisePath.npos)
		{
			PrintToConsole("Import parent must be in Interactive Music hierarchy for Music imports");
			return;	// Invalid wwise parent selection!
		}
	}
	else{
		if (parentWwisePath.find("Actor-Mixer Hierarchy\\") == parentWwisePath.npos)
		{
			PrintToConsole("Import parent must be in Actor-Mixer hierarchy for Sfx or Voice imports");
			return;	// Invalid wwise parent selection!
		}
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
					fileOverride.EimportType = GetImportType();
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
					renderJob.EimportType = GetImportType();

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
	this->toFront(true);


}

void TransferToWwiseComponent::TryConnectToWwise() {


	if (!MyCurrentWwiseConnection->connected)
	{
		thisCreateImportWindow->handleUI_B_Connect();
	}
	
	bool connected = MyCurrentWwiseConnection->connected;
	if ((connected)&&(MyCurrentWwiseConnection->projectGlobals.ProjectPath != ""))
	{
		String text = ("Wwise Connected: " + MyCurrentWwiseConnection->projectGlobals.ProjectName);
		txt_ConnectionStatus->setText(text, juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->AddActiveComponent(this);
		if (!isSubscribed)
		{
			WwiseCntnHndlr->SubscribeOnSelectionChanged(WwiseConnectionHandler::callback_OnSelectionChanged, subscriptionID_selectionChanged);
			WwiseCntnHndlr->SubscribeOnProjectClosed(WwiseConnectionHandler::callback_OnProjectClosed, subscriptionID_projectClosed);
			isSubscribed = true;
		}
		InitComboBox(dd_Language, MyCurrentWwiseConnection->projectGlobals.Languages, "Language..");
		handle_OnSelectedParentChanged();
		setStatusText("Ready");
	}
	else
	{
		txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
		if (isSubscribed)
		{
			WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
			WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
			subscriptionID_selectionChanged = 0;
			subscriptionID_projectClosed = 0;
			isSubscribed = false;
		}
		WwiseCntnHndlr->DisconnectFromWwise();
		WwiseCntnHndlr->RemoveActiveComponent(this);
		setStatusText("Error");
		
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
	else if (pButton == btn_OriginalsMatchesWwise)
	{
		CheckOriginalsDirectory();
	}
	else if (pButton == btn_RenderAndImport)
	{
		//thisCreateImportWindow->backupRenderQueFiles();
		if (thisCreateImportWindow->handleUI_RenderImport())
		{
			SaveEventsToExState();
			RefreshRenderJobTree();
		}
		//thisCreateImportWindow->restoreRenderQueFiles();
		this->toFront(true);
		handle_OnBecameActiveTab();
	}
	else if (pButton == btn_ChooseWwiseOriginalsDir)
	{
		std::string userOriginalsSubDir = INtxt_OriginalsSubDir->getText().toStdString();
		if (askUserForWwiseSubDir(userOriginalsSubDir))
		{
			INtxt_OriginalsSubDir->setText(userOriginalsSubDir, juce::NotificationType::dontSendNotification);
		}
	}
	
}

void TransferToWwiseComponent::comboBoxChanged(ComboBox * comboBoxThatHasChanged)
{
	String text = ("SELECTED: " + comboBoxThatHasChanged->getText());
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);
	if (comboBoxThatHasChanged == dd_ImportType)
	{
		CheckIsVoice();
	}
}

void TransferToWwiseComponent::labelTextChanged(Label * labelThatHasChanged)
{
}

bool TransferToWwiseComponent::CheckIsVoice() {
	
	if (dd_ImportType->getText().toStdString() == "Voice")
	{
		dd_Language->setEnabled(true);
		return true;
	}
	else
	{
		dd_Language->setEnabled(false);
		return false;
	}
	/*
	if (btn_isVoice->getToggleState())
	{
		dd_Language->setEnabled(true);
		return true;
	}
	else{
		dd_Language->setEnabled(false);
		return false;
	}*/
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
	if (isSubscribed)
	{
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
		subscriptionID_selectionChanged = 0;
		subscriptionID_projectClosed = 0;
		isSubscribed = false;
	}
	WwiseCntnHndlr->DisconnectFromWwise();
	std::string display = "Selected Parent: ";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}

bool TransferToWwiseComponent::askUserForWwiseSubDir(std::string &OutSubDir)
{
	juce::String wwiseOriginalsRoot = MyCurrentWwiseConnection->projectGlobals.OriginalsPath;
	File fWwiseRoot = File(wwiseOriginalsRoot);
	myChooser = std::make_unique<FileChooser> ("Please select the location for the Wwise originals",fWwiseRoot,"");
	if (!fWwiseRoot.isDirectory())
	{
		return false;
	}
	if (myChooser->browseForDirectory())
	{
		File selectedFile (myChooser->getResult());
		std::string subDir = selectedFile.getRelativePathFrom(fWwiseRoot).toStdString()+kPathSeparator;
		std::string subDirLower = stringToLower(subDir);
		if (subDir.empty() or subDir==".")
		{
			PrintToConsole("Wwise originals location must be inside SFX or Voices folder..");
			return false;
		}
		if (!(subDirLower.find("voices") != subDirLower.npos or subDirLower.find("sfx") != subDirLower.npos))
		{
			PrintToConsole("Wwise originals location must be inside SFX or Voices folder..");
			return false;
		}
		//strip the first dir off because it will either be SFX or Voices
		if (subDir.find(kPathSeparator) != subDir.npos)
		{
			subDir.erase(0, subDir.find(kPathSeparator) + 1);
		}
		//if subDir contained voices, then we must then strip the language folder too
		if (subDirLower.find("voices") != subDirLower.npos)
		{
			if (subDir.find(kPathSeparator) != subDir.npos)
			{
				subDir.erase(0, subDir.find(kPathSeparator) + 1);
			}
		}
		//PrintToConsole(subDir);
		OutSubDir = subDir;
		return true;
	}
	return false;
}

void TransferToWwiseComponent::setTransferValuesFromConfig(config c)
{
	if (!c.userOrigDir.empty())
	{
		INtxt_OriginalsSubDir->setText(c.userOrigDir, juce::NotificationType::dontSendNotification);
	}
	
}

void TransferToWwiseComponent::handle_OnBecameActiveTab()
{
	bool connected = MyCurrentWwiseConnection->connected;
	if ((connected)&&(MyCurrentWwiseConnection->projectGlobals.ProjectPath != ""))
	{
		WwiseCntnHndlr->AddActiveComponent(this);
		if (!isSubscribed)
		{
			WwiseCntnHndlr->SubscribeOnSelectionChanged(WwiseConnectionHandler::callback_OnSelectionChanged, subscriptionID_selectionChanged);
			WwiseCntnHndlr->SubscribeOnProjectClosed(WwiseConnectionHandler::callback_OnProjectClosed, subscriptionID_projectClosed);
			isSubscribed = true;
		}
		handle_OnSelectedParentChanged();
		setStatusText("Ready");
		String text = ("Wwise Connected: " + MyCurrentWwiseConnection->projectGlobals.ProjectName);
		txt_ConnectionStatus->setText(text, juce::NotificationType::dontSendNotification);
		InitComboBox(dd_Language, MyCurrentWwiseConnection->projectGlobals.Languages, "Language..");
	}
	else
	{
		setStatusText("Error");
		txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->RemoveActiveComponent(this);
	}

}

void TransferToWwiseComponent::handle_OnTabBecameInactive()
{
	if (isSubscribed)
	{
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
		subscriptionID_selectionChanged = 0;
		subscriptionID_projectClosed = 0;
		isSubscribed = false;
	}
	//WwiseCntnHndlr->DisconnectFromWwise();
	WwiseCntnHndlr->RemoveActiveComponent(this);

//	WwiseCntnHndlr->DisconnectFromWwise();
}

IMPORT_TYPE TransferToWwiseComponent::GetImportType() { 
	if (dd_ImportType->getText().toStdString() == "Voice")
	{
		return IMPORT_TYPE::Voice;
	}
	else if (dd_ImportType->getText().toStdString() == "Music")
	{
		return IMPORT_TYPE::Music;
	}
	else
	{
		return IMPORT_TYPE::Sfx;
	}
}

void TransferToWwiseComponent::AddEventToSaveList(std::string eventName)
{
	if (eventName != "")
	{
		EventsToSave.insert(eventName);
	}
}

void TransferToWwiseComponent::SaveEventsToExState()
{
	GetExistingEventsFromExState();
	if (EventsToSave.empty())
	{
		return;
	}
	std::string name = "CSGTransferWwiseEvents";
	std::stringstream valuesToJson;
	//"{ 'id': 1234, 'name': 'nandini' }"
	valuesToJson << '{';
	for (auto value : EventsToSave)
	{
		valuesToJson << "'";
		valuesToJson << value;
		valuesToJson << "'";
		valuesToJson << ",";
	}
	//PrintToConsole(valuesToJson.str());
	valuesToJson.seekp(-1, valuesToJson.cur); valuesToJson << "}";
	saveProjExState("Events", valuesToJson.str(), name);
}

void TransferToWwiseComponent::GetExistingEventsFromExState()
{
	std::vector<std::string> existingEvents = ProjectRegionMetadataHelper::GetEventListFromProjExtState();
	for (auto event : existingEvents)
	{
		if (thisCreateImportWindow)
		{
			if (!thisCreateImportWindow->GetWwiseObjectsByName(event, "Event").empty())
			{
				EventsToSave.insert(event);
			}
		}
		
	}
}
