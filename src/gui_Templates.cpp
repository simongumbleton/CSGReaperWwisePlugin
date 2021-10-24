
#include "gui_Templates.h"
#include "reaperHelpers.h"
#include <filesystem>
#include <mutex>

StringArray RegionMetadataComponent::ToJuceStringArray(std::vector<std::string> strings)
{
	StringArray output;
	for (auto string : strings)
	{
		output.add(string);
	}
	return output;
}

RegionMetadataComponent * RegionMetadataComponent::currentWwiseTemplateComponent = nullptr;
std::mutex mx_tem;

RegionMetadataComponent::RegionMetadataComponent() //constructor
{
	
	thisCreateImportWindow = new CreateImportWindow();
	if (thisCreateImportWindow)
	{
		thisCreateImportWindow->owningGUIWindow = this;
	}
	WwiseCntnHndlr = thisCreateImportWindow->WwiseConnectionHnd;
	
	MyCurrentWwiseConnection = &thisCreateImportWindow->WwiseConnectionHnd->MyCurrentWwiseConnection;
	
	RegionMetadataComponent::currentWwiseTemplateComponent = this;
	
	// Init buttons and combo boxes
	InitAllButtons(buttons);
	
	//addAndMakeVisible(txt_ConnectionStatus);

	debugLabel->setText("debug", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(debugLabel);
	
	//addAndMakeVisible(txt_ConnectionStatus);
	//txt_ConnectionStatus->setText("Wwise Connection Status:", juce::NotificationType::dontSendNotification);
	
	addAndMakeVisible(statusLabel);
	statusLabel->setText("Status: ", juce::NotificationType::dontSendNotification);
	
	//int numRegions = 3;
	//RegionProperties = createProperties(numRegions);
	//int i = 0;
	//for (auto region : RegionProperties)
	//{
	//	addChildComponent(region);
	//	addAndMakeVisible(region);
	//	//region->SetRegionName(RegionNames[i]);
	//	region->SetPropertyValuesFromExState();
	//	i++;
	//}

	addAndMakeVisible(regionProperties);
	myViewport->setBounds(0, 0, 750, 300);
	myViewport->setViewedComponent(regionProperties);
	addAndMakeVisible(myViewport);



	setSize(1000, 500);
	
	TryConnectToWwise();
}

RegionMetadataComponent::~RegionMetadataComponent()
{
	
	WwiseCntnHndlr->UnsubscribeFromTopicByID(21);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(22);
	WwiseCntnHndlr->RemoveActiveComponent(this);
	RegionMetadataComponent::currentWwiseTemplateComponent = nullptr;
}

void RegionMetadataComponent::InitAllButtons(std::vector<juce::Button *> buttons)
{
	for (auto button : buttons)
	{
		String name = button->getName();
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);
		
	}
}

void RegionMetadataComponent::InitComboBox(juce::ComboBox * comboBox, std::vector<std::string> choices,string displayText)
{
	comboBox->clear();
	comboBox->addItemList(ToJuceStringArray(choices),1);
	comboBox->setTextWhenNothingSelected(displayText);
	comboBox->addListener(this);
	comboBox->setSelectedItemIndex(0, false);
	addAndMakeVisible(comboBox);
}



void RegionMetadataComponent::resized()
{
	auto border = 4;
	auto buttonHeight = 30;
	auto comboHeight = 30;
	auto treeHeight = 200;
	auto labelHeight = 30;
	auto titleHeight = 60;
	auto area = getLocalBounds();

	auto titleArea = area.removeFromTop(titleHeight);

	//auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);
	//auto RightHalf = area;

	auto viewportArea = area.removeFromTop(300);
	myViewport->setBounds(viewportArea);

	//auto TopRightQtr = RightHalf.removeFromTop(RightHalf.getHeight()/2);
	//auto CreateCorner = TopRightQtr.removeFromRight(TopRightQtr.getWidth()/1.5);
	
	auto buttonArea = area.removeFromTop(buttonHeight);

	auto savebuttonArea = buttonArea.removeFromLeft(300);
	
	btn_Save->setBounds(savebuttonArea.reduced(border));
	
	auto edgesize = buttonArea.getWidth()*0.1;
	auto offsetL = buttonArea.removeFromLeft(edgesize);
	auto offsetR = buttonArea.removeFromRight(edgesize);
	
	//auto optionsArea1 = LeftHalf.removeFromTop(buttonHeight).reduced(border);
	
	//auto optionsArea2 = LeftHalf.removeFromTop(buttonHeight).reduced(border);
	
	//auto o1qtrsize = optionsArea1.getWidth()/4;

	//auto RenderButtonArea = LeftHalf.removeFromTop(buttonHeight*2).reduced(border);
	
	//auto bottomRow = LeftHalf.removeFromBottom(buttonHeight).reduced(border);
	
	//btn_ConnectToWwise->setBounds(bottomRow.removeFromLeft(bottomRow.getWidth()/2));
	
	//txt_ConnectionStatus->setBounds(bottomRow);
	
	auto statusRow = area.removeFromBottom(labelHeight).reduced(border);
	
	statusLabel->setBounds(statusRow.removeFromLeft(statusRow.getWidth()/2).reduced(border));
	debugLabel->setBounds(statusRow);
}





void RegionMetadataComponent::TryConnectToWwise() {
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

void RegionMetadataComponent::buttonClicked(juce::Button * pButton)
{
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);

	if (pButton == btn_ConnectToWwise)
	{
		TryConnectToWwise();
	}
	else if (pButton == btn_Save)
	{
		handle_OnButton_Saved();
	}
	
}

void RegionMetadataComponent::comboBoxChanged(ComboBox * comboBoxThatHasChanged)
{
	String text = ("SELECTED: " + comboBoxThatHasChanged->getText());
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);
}

void RegionMetadataComponent::labelTextChanged(Label * labelThatHasChanged)
{
}

std::string RegionMetadataComponent::GetDropDownValue(juce::ComboBox * dropdown)
{
	int Index = dropdown->getSelectedItemIndex();
	return dropdown->getItemText(Index).toStdString();
}

std::string RegionMetadataComponent::GetLabelValue(juce::Label * label)
{
	return label->getText().toStdString();
}

bool RegionMetadataComponent::GetToggleValue(juce::ToggleButton * btn)
{
	return btn->getToggleState();
}

void RegionMetadataComponent::handle_OnSelectedParentChanged()
{
	const std::lock_guard<std::mutex> lock(mx_tem);
	//std::cout << "_____Callback 1______" << std::endl;
	WwiseObject newParent = WwiseCntnHndlr->GetSelectedObject();
	//std::cout << newParent.properties["name"] << std::endl;
	//std::string display = "Selected Parent: " + newParent.properties["name"];
	//MessageManagerLock mml(Thread::getCurrentThread());
	std::string display = "Selected Parent: " +newParent.properties["name"] + " ("+newParent.properties["type"]+")";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}

void RegionMetadataComponent::handle_OnWwiseProjectClosed()
{
	const std::lock_guard<std::mutex> lock(mx_tem);
	std::cout << "_____Callback 2______" << std::endl;
	txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(21);
	WwiseCntnHndlr->UnsubscribeFromTopicByID(22);
	WwiseCntnHndlr->DisconnectFromWwise();
	std::string display = "Selected Parent: ";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}

void RegionMetadataComponent::handle_OnButton_Saved() { 
	//std::vector<std::string>nonMasterRegions = getNonMasterProjectRegions();
	saveProjExState("", "");
	std::map< std::string, std::map<std::string, std::string> > savedRegionsWithProperties;
	for (auto region : regionProperties->RegionProperties)
	{
		std::string name = region->GetRegionName();
		std::map<std::string, std::string> values = region->GetPropertyValues();
		savedRegionsWithProperties.emplace(name,values);
		std::stringstream valuesToJson;
		//"{ 'id': 1234, 'name': 'nandini' }"
		valuesToJson << '{';
		for (auto value : values)
		{
			valuesToJson << "'";
			valuesToJson << value.first;
			valuesToJson << "'";
			valuesToJson << ":";
			valuesToJson << "'";
			valuesToJson << value.second;
			valuesToJson << "'";
			valuesToJson << ",";
		}
		//PrintToConsole(valuesToJson.str());
		valuesToJson.seekp(-1,valuesToJson.cur); valuesToJson << "}";
		saveProjExState(name, valuesToJson.str());
	}
	SaveProject();
}


