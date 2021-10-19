
#include "gui_create.h"
#include "gui_Transfer.h"
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

CreateWwiseComponent::CreateWwiseComponent(juce::Component* parentComp) //constructor
{
	parent = dynamic_cast<TransferTabComponent*>(parentComp);
	if (parent)
	{
		thisCreateImportWindow = parent->thisCreateImportWindow;// new CreateImportWindow();
		WwiseCntnHndlr = thisCreateImportWindow->WwiseConnectionHnd;
		
		MyCurrentWwiseConnection = &thisCreateImportWindow->WwiseConnectionHnd->MyCurrentWwiseConnection;
	}
	
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

	//auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);
	//auto RightHalf = area;

	
	
	//auto TopRightQtr = RightHalf.removeFromTop(RightHalf.getHeight()/2);
	//auto CreateCorner = TopRightQtr.removeFromRight(TopRightQtr.getWidth()/1.5);
	
	Title_CreateWwiseObject->setBounds(area.removeFromTop(titleHeight).reduced(border));
	
	
	dd_CreateType->setBounds(area.removeFromTop(comboHeight).reduced(border).removeFromRight(area.getWidth()/2));
	//dd_CreateType->setJustificationType(Justification::right);
	
	INtxt_CreateName->setBounds(area.removeFromTop(labelHeight).reduced(border).removeFromRight(area.getWidth()/2));
	//INtxt_CreateName->setJustificationType(Justification::right);
	
	INtxt_CreateNotes->setBounds(area.removeFromTop(labelHeight).reduced(border).removeFromRight(area.getWidth()/2));
	//INtxt_CreateNotes->setJustificationType(Justification::right);
	dd_OnNameConflict->setBounds(area.removeFromTop(comboHeight).reduced(border).removeFromRight(area.getWidth()/2));
	//dd_OnNameConflict->setJustificationType(Justification::right);
	
	
	btn_CreatePlayEvent->setBounds(area.removeFromTop(buttonHeight).reduced(border).removeFromRight(area.getWidth()/2));
	
	auto buttonArea = area.removeFromTop(buttonHeight).reduced(border);
	auto edgesize = buttonArea.getWidth()*0.1;
	auto offsetL = buttonArea.removeFromLeft(edgesize);
	auto offsetR = buttonArea.removeFromRight(edgesize);
	btn_CreateWwiseObject->setBounds(buttonArea);
	btn_CreateWwiseObject->setSize(buttonArea.getWidth(), buttonHeight);
	
	//auto TopLeftQtr = LeftHalf.removeFromTop(LeftHalf.getHeight()/2);
	
	auto optionsArea1 = area.removeFromTop(buttonHeight).reduced(border);
	
	auto optionsArea2 = area.removeFromTop(buttonHeight).reduced(border);
	
	auto o1qtrsize = optionsArea1.getWidth()/4;
		
	auto originalsTextArea = optionsArea2.removeFromLeft(optionsArea2.getWidth()*0.7);
		
	auto statusRow = area.removeFromBottom(labelHeight).reduced(border);
	
	auto statusLeft = statusRow.removeFromLeft(statusRow.getWidth()/2);
	
	statusLabel->setBounds(statusRow.removeFromLeft(statusRow.getWidth()/2).reduced(border));
	debugLabel->setBounds(statusRow);
	
	btn_ConnectToWwise->setBounds(statusLeft.removeFromLeft(statusLeft.getWidth()/2));
	
	txt_ConnectionStatus->setBounds(statusLeft);
	
	progressBar->setBounds(area.removeFromBottom(buttonHeight).reduced(border));
	
	
	auto RenderButtonArea = area.removeFromBottom(buttonHeight*1.5).reduced(border);
	

}


void CreateWwiseComponent::TryConnectToWwise() {
	thisCreateImportWindow->handleUI_B_Connect();
	bool connected = MyCurrentWwiseConnection->connected;
	if ((connected)&&(MyCurrentWwiseConnection->projectGlobals.ProjectPath != ""))
	{
		String text = ("Wwise Connected: " + MyCurrentWwiseConnection->projectGlobals.ProjectName);
		txt_ConnectionStatus->setText(text, juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->AddActiveComponent(this);
		WwiseCntnHndlr->SubscribeOnSelectionChanged(WwiseConnectionHandler::callback_OnSelectionChanged, subscriptionID_selectionChanged);
		WwiseCntnHndlr->SubscribeOnProjectClosed(WwiseConnectionHandler::callback_OnProjectClosed, subscriptionID_projectClosed);
		handle_OnSelectedParentChanged();
	}
	else
	{
		txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
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
	WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
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

void CreateWwiseComponent::handle_OnBecameActiveTab()
{
	bool connected = MyCurrentWwiseConnection->connected;
	if ((connected)&&(MyCurrentWwiseConnection->projectGlobals.ProjectPath != ""))
	{
		WwiseCntnHndlr->SubscribeOnSelectionChanged(WwiseConnectionHandler::callback_OnSelectionChanged, subscriptionID_selectionChanged);
		WwiseCntnHndlr->SubscribeOnProjectClosed(WwiseConnectionHandler::callback_OnProjectClosed, subscriptionID_projectClosed);
		handle_OnSelectedParentChanged();
	}
}

void CreateWwiseComponent::handle_OnTabBecameInactive()
{
	WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
//	WwiseCntnHndlr->DisconnectFromWwise();
}
