
#include "gui_create.h"
#include "reaperHelpers.h"
#include <filesystem>
#include <mutex>

StringArray CreateWwiseComponent::ToJuceStringArray(std::vector<std::string> strings)
{
	StringArray output;
	for (auto string : strings)
	{
		output.add(string);
	}
	return output;
}

CreateWwiseComponent * CreateWwiseComponent::currentCreateComponent = nullptr;
std::mutex mx_c;

CreateWwiseComponent::CreateWwiseComponent() //constructor
{
	
	thisCreateImportWindow = new CreateImportWindow();
	if (thisCreateImportWindow)
	{
		thisCreateImportWindow->owningGUIWindow = this;
	}
	WwiseCntnHndlr = thisCreateImportWindow->WwiseConnectionHnd;
	
	MyCurrentWwiseConnection = &thisCreateImportWindow->WwiseConnectionHnd->MyCurrentWwiseConnection;
	
	CreateWwiseComponent::currentCreateComponent = this;
	
	
	// Init buttons and combo boxes
	InitAllButtons(buttons);
	
	InitComboBox(dd_CreateType,myCreateChoices.waapiCREATEchoices_TYPE,"Type..");
	addAndMakeVisible(info_CreateType);
	info_CreateType->attachToComponent(dd_CreateType, true);
	info_CreateType->setText("New Object Type: ", juce::NotificationType::dontSendNotification);
	
	InitComboBox(dd_OnNameConflict,myCreateChoices.waapiCREATEchoices_NAMECONFLICT,"On name conflict..");
	addAndMakeVisible(info_NameConflict);
	info_NameConflict->attachToComponent(dd_OnNameConflict, true);
	info_NameConflict->setText("On Name Conflict: ", juce::NotificationType::dontSendNotification);
	dd_OnNameConflict->setSelectedItemIndex(0, false);

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
	
	
	addAndMakeVisible(txt_ConnectionStatus);

	debugLabel->setText("debug", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(debugLabel);

	addAndMakeVisible(selectedParentLabel);
	selectedParentLabel->setText("Parent Name (Type):", juce::NotificationType::dontSendNotification);
	
	addAndMakeVisible(txt_ConnectionStatus);
	txt_ConnectionStatus->setText("Wwise Connection Status:", juce::NotificationType::dontSendNotification);
	
	addAndMakeVisible(statusLabel);
	statusLabel->setText("Status: ", juce::NotificationType::dontSendNotification);
	
	addAndMakeVisible(progressBar);

	setSize(1000, 600);
	
	thisCreateImportWindow->OnInitDlg();
	
	TryConnectToWwise();
	updateProgressValue(0);
	
	ReadConfigFile(myConfig);
	setTransferValuesFromConfig(myConfig);
	
	setStatusText("Ready..");
	

}

CreateWwiseComponent::~CreateWwiseComponent()
{
	WwiseCntnHndlr->UnsubscribeFromTopicByID(11);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(12);
	WwiseCntnHndlr->RemoveActiveComponent(this);
	CreateWwiseComponent::currentCreateComponent = nullptr;
}

void CreateWwiseComponent::InitAllButtons(std::vector<juce::Button *> buttons)
{
	for (auto button : buttons)
	{
		String name = button->getName();
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);
		
	}
}

void CreateWwiseComponent::InitComboBox(juce::ComboBox * comboBox, std::vector<std::string> choices,string displayText)
{
	comboBox->clear();
	comboBox->addItemList(ToJuceStringArray(choices),1);
	comboBox->setTextWhenNothingSelected(displayText);
	comboBox->addListener(this);
	comboBox->setSelectedItemIndex(0, false);
	addAndMakeVisible(comboBox);
}

void CreateWwiseComponent::resized()
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
	
	selectedParentLabel->setBounds(LeftHalf.removeFromTop(labelHeight).reduced(border));
	
	auto optionsArea1 = LeftHalf.removeFromTop(buttonHeight).reduced(border);
	
	auto optionsArea2 = LeftHalf.removeFromTop(buttonHeight).reduced(border);
	
	auto o1qtrsize = optionsArea1.getWidth()/4;
		
	auto originalsTextArea = optionsArea2.removeFromLeft(optionsArea2.getWidth()*0.7);
		
	auto bottomRow = LeftHalf.removeFromBottom(buttonHeight).reduced(border);
	
	btn_ConnectToWwise->setBounds(bottomRow.removeFromLeft(bottomRow.getWidth()/2));
	
	txt_ConnectionStatus->setBounds(bottomRow);
	
	progressBar->setBounds(LeftHalf.removeFromBottom(buttonHeight).reduced(border));
	
	
	auto RenderButtonArea = LeftHalf.removeFromBottom(buttonHeight*1.5).reduced(border);
	
	auto statusRow = RightHalf.removeFromBottom(labelHeight).reduced(border);
	
	statusLabel->setBounds(statusRow.removeFromLeft(statusRow.getWidth()/2).reduced(border));
	debugLabel->setBounds(statusRow);
}


void CreateWwiseComponent::TryConnectToWwise() {
	thisCreateImportWindow->handleUI_B_Connect();
	bool connected = MyCurrentWwiseConnection->connected;
	if ((connected)&&(MyCurrentWwiseConnection->projectGlobals.ProjectPath != ""))
	{
		String text = ("Wwise Connected: " + MyCurrentWwiseConnection->projectGlobals.ProjectName);
		txt_ConnectionStatus->setText(text, juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->AddActiveComponent(this);
		WwiseCntnHndlr->SubscribeOnSelectionChanged(WwiseConnectionHandler::callback_OnSelectionChanged, 11);
		WwiseCntnHndlr->SubscribeOnProjectClosed(WwiseConnectionHandler::callback_OnProjectClosed, 12);
		handle_OnSelectedParentChanged();
	}
	else
	{
		txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(11);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(12);
		WwiseCntnHndlr->DisconnectFromWwise();
		WwiseCntnHndlr->RemoveActiveComponent(this);
		
	}
}

void CreateWwiseComponent::buttonClicked(juce::Button * pButton)
{
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);

	if (pButton == btn_ConnectToWwise)
	{
		TryConnectToWwise();
	}
	else if (pButton == btn_CreateWwiseObject)
	{
		handleUI_B_CreateObject();
	}
}

void CreateWwiseComponent::comboBoxChanged(ComboBox * comboBoxThatHasChanged)
{
	String text = ("SELECTED: " + comboBoxThatHasChanged->getText());
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);
}

void CreateWwiseComponent::labelTextChanged(Label * labelThatHasChanged)
{
}


void CreateWwiseComponent::handleUI_B_CreateObject()
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


std::string CreateWwiseComponent::GetDropDownValue(juce::ComboBox * dropdown)
{
	int Index = dropdown->getSelectedItemIndex();
	return dropdown->getItemText(Index).toStdString();
}

std::string CreateWwiseComponent::GetLabelValue(juce::Label * label)
{
	return label->getText().toStdString();
}

bool CreateWwiseComponent::GetToggleValue(juce::ToggleButton * btn)
{
	return btn->getToggleState();
}

void CreateWwiseComponent::handle_OnSelectedParentChanged()
{
	const std::lock_guard<std::mutex> lock(mx_c);
	//std::cout << "_____Callback 1______" << std::endl;
	WwiseObject newParent = WwiseCntnHndlr->GetSelectedObject();
	//std::cout << newParent.properties["name"] << std::endl;
	//std::string display = "Selected Parent: " + newParent.properties["name"];
	//MessageManagerLock mml(Thread::getCurrentThread());
	std::string display = "Selected Parent: " +newParent.properties["name"] + " ("+newParent.properties["type"]+")";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}

void CreateWwiseComponent::handle_OnWwiseProjectClosed()
{
	const std::lock_guard<std::mutex> lock(mx_c);
	std::cout << "_____Callback 2______" << std::endl;
	txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(11);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(12);
	WwiseCntnHndlr->DisconnectFromWwise();
	std::string display = "Selected Parent: ";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}


void CreateWwiseComponent::setTransferValuesFromConfig(config c)
{
	//if (!c.userOrigDir.empty())
	//{
	//}
}
