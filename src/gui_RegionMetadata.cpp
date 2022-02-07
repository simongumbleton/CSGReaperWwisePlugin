
#include "gui_RegionMetadata.h"
#include "reaperHelpers.h"
#include <filesystem>
#include <mutex>
#include "RegionMetadataHelper.h"


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
	
	
	addAndMakeVisible(TitleRegions);
	TitleRegions->setText("Regions:", juce::NotificationType::dontSendNotification);
	//addAndMakeVisible(TitleTag);
	//TitleTag->setText("Tag:", juce::NotificationType::dontSendNotification);
	//addAndMakeVisible(TitleAttach);
	//TitleAttach->setText("Attach:", juce::NotificationType::dontSendNotification);
	
	
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

	addAndMakeVisible(regionPropertiesViewport);
	//myViewport->setBounds(0, 0, 750, 300);
	myViewport->setSize(3000, 1000);
	myViewport->setViewedComponent(regionPropertiesViewport);
	addAndMakeVisible(myViewport);

	txt_pluginVersion->setText(RegionMetadata::GetPluginVersionString(), juce::NotificationType::dontSendNotification);
	addAndMakeVisible(txt_pluginVersion);

	addAndMakeVisible(btn_SettingsMetadata);

	setSize(600, 400);
	
	//TryConnectToWwise();
	
	metadataHelper = std::make_unique<ProjectRegionMetadataHelper>();
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
	
	TitleRegions->setBounds(titleArea.removeFromLeft(320));
	//TitleTag->setBounds(titleArea.removeFromLeft(100));
	//TitleAttach->setBounds(titleArea.removeFromLeft(100));

	//auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);
	//auto RightHalf = area;

	auto viewportArea = area.removeFromTop(300);
	myViewport->setBounds(viewportArea);

	//auto TopRightQtr = RightHalf.removeFromTop(RightHalf.getHeight()/2);
	//auto CreateCorner = TopRightQtr.removeFromRight(TopRightQtr.getWidth()/1.5);
	
	auto buttonArea = area.removeFromTop(buttonHeight);

	auto savebuttonArea = buttonArea.removeFromLeft(300);
	
	btn_Save->setBounds(savebuttonArea.reduced(border));

	auto refreshButtonArea = buttonArea.removeFromLeft(300);

	btn_Refresh->setBounds(refreshButtonArea.reduced(border));
	
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
	
	txt_pluginVersion->setBounds(statusRow.removeFromRight(50));
	btn_SettingsMetadata->setBounds(statusRow.removeFromLeft(40));
	
	statusLabel->setBounds(statusRow.removeFromLeft(statusRow.getWidth()/2).reduced(border));
	debugLabel->setBounds(statusRow);
}





void RegionMetadataComponent::TryConnectToWwise() {
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
			isSubscribed = false;
		}
		WwiseCntnHndlr->DisconnectFromWwise();
		WwiseCntnHndlr->RemoveActiveComponent(this);
		setStatusText("Error");
		
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
	else if (pButton == btn_Refresh)
	{
		handle_OnButton_Refresh();
	}
	else if (pButton == btn_SettingsMetadata)
	{
		LaunchSettingsWindow();
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
	if (isSubscribed)
	{
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
		WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
		isSubscribed = false;
	}
	WwiseCntnHndlr->DisconnectFromWwise();
	std::string display = "Selected Parent: ";
	selectedParentLabel->setText(display, juce::NotificationType::dontSendNotification);
}

void RegionMetadataComponent::handle_OnButton_Saved()
{
	metadataHelper->fillProjectRegionInfos();
	//std::vector<std::string>nonMasterRegions = getNonMasterProjectRegions();
	saveProjExState("", "");
	for (auto region : regionPropertiesViewport->RegionProperties)
	{
		std::map<std::string, std::string> values = region->GetPropertyValues();
		metadataHelper->updateRegionGUIProperties(region->GetRegionName(), values);
		region->SaveRegionPropertiesToExState();
	}
	metadataHelper->createMasterRegionData();
	SaveProject();
	metadataHelper->writeMasterRegionInfoToJson();
	bringWindowsToFront();
}

void RegionMetadataComponent::handle_OnButton_Refresh()
{
	regionPropertiesViewport->refreshRegionsFromProject();
	//regionPropertiesViewport->repaint();
}

void RegionMetadataComponent::LaunchSettingsWindow()
{
	if (windowStatus)return;
	settingsWndHndl_meta = new MetadataSettingsWnd("settings", regionMetadataSettings, windowStatus);
	addAndMakeVisible(settingsWndHndl_meta);
	settingsWndHndl_meta->centreWithSize(300, 400);
}
