#pragma once

#include "JUCE/JuceHeader.h"
#include "waapi_structs.h"
#include "WwiseConnectionHandler.h"
#include "GUI.h"
#include "gui_RenderTree.h"
#include "gui_properties.h"
#include "reaperHelpers.h"
#include "gui_settings.h"

class ProjectRegionMetadataHelper;
//class WwiseTemplateComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener



class RegionMetadataComponent : public BaseWwiseGuiComponent
{
	static RegionMetadataComponent * currentWwiseTemplateComponent;

	MetadataSettingsStruct regionMetadataSettings;

	MetadataSettingsWnd* settingsWndHndl_meta = nullptr;

	bool windowStatus = false;

public:

	CreateImportWindow * thisCreateImportWindow;
	
	CurrentWwiseConnection * MyCurrentWwiseConnection;
	
	WwiseConnectionHandler * WwiseCntnHndlr;

	RegionMetadataComponent();
	~RegionMetadataComponent();

	void resized() override;
	
	void TryConnectToWwise();
	
	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;

	void InitAllButtons(std::vector<juce::Button *> buttons);

	void InitComboBox(juce::ComboBox * comboBox, std::vector<std::string> choices, string displayText);
	
	
	void handleUI_B_CreateObject();
	
	std::string GetDropDownValue(juce::ComboBox * dropdown);
	
	std::string GetLabelValue(juce::Label * label);
	
	bool GetToggleValue(juce::ToggleButton * btn);
	
	void handle_OnSelectedParentChanged();
	
	void handle_OnWwiseProjectClosed();
	
	void handle_OnButton_Saved();

	void handle_OnButton_Refresh();

	void LaunchSettingsWindow();
	
	void LoadSettingsFromExtState();
	
	void SaveSettingsToExtState();

	void UpdateRegionPropertiesFromSettings()
	{
		regionPropertiesViewport->UpdateRegionPropertyList(regionMetadataSettings.PropertyNames);
		//myViewport->setViewedComponent(nullptr);
		myViewport->setViewedComponent(regionPropertiesViewport);
		repaint();
		resized();
	};
	
	//void postCommandMessage(int commandId);
	//https://docs.juce.com/master/classComponent.html#a9ba6fa31d1397c7e90050b2cd4f2089b
	
	void handleCommandMessage (int commandId) override
	{
		switch(commandId) {
			case 1 : handle_OnSelectedParentChanged();
					 break;       // and exits the switch
			case 2 : handle_OnWwiseProjectClosed();
					 break;
			/// Sent from gui_settings
			case 11: UpdateRegionPropertiesFromSettings();
					 break;
		}
	}
	
	//Array<PropertiesComponent*> RegionProperties;
	Viewport* myViewport;
	PropertiesViewportComponent* regionPropertiesViewport;

private:
	std::unique_ptr<ProjectRegionMetadataHelper> metadataHelper;

	bool isSubscribed;
	uint64_t subscriptionID_selectionChanged =0;
	uint64_t subscriptionID_projectClosed=0;
	
	juce::TextButton * btn_ConnectToWwise = new TextButton("Connect To Wwise");
	
	juce::TextButton * btn_Save = new TextButton("Save Changes");

	juce::TextButton* btn_Refresh = new TextButton("Refresh Regions From Project");

	SettingsButton* btn_SettingsMetadata = new SettingsButton("settingsBtn");

	std::vector<juce::Button*> buttons{
//	btn_ConnectToWwise,
	btn_Save,
	btn_Refresh,
	btn_SettingsMetadata
	};

	juce::Label * txt_ConnectionStatus = new Label(); // text

	juce::Label * debugLabel = new Label();
	
	juce::Label * selectedParentLabel = new Label();
	
	juce::Label * TitleRegions = new Label();
	//juce::Label * TitleTag = new Label();
	//juce::Label * TitleAttach = new Label();
	
	
	
/*	Array<PropertiesComponent*> createProperties(int count)
	{// TO DO - count should be the number of regions
		Array<PropertiesComponent*> properties;
		for (auto region : getNonMasterProjectRegionNames())
		{
			properties.add(new PropertiesComponent(region, regionMetadataSettings.PropertyNames));
		}
		//for (int i = 0;i < count;i++)
		//{
		//	properties.add(new PropertiesComponent("Region"));
		//}
		return properties;
	}*/

	StringArray ToJuceStringArray(std::vector<std::string>strings);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionMetadataComponent)
};





class RegionMetadataWindow : public juce::DocumentWindow
{
	bool * mWindowState;
public:
	RegionMetadataWindow(const juce::String& name, juce::Component* component, bool *windowStatus)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		setContentOwned(component, true);
		setResizable(true, false);
		setResizeLimits(500, 500, 10000, 10000);
		centreWithSize(getWidth(), getHeight());
		mWindowState = windowStatus;
		*mWindowState = true;

		setVisible(true);
	}

	void closeButtonPressed() override
	{
		*mWindowState = false;
		delete this;
	}

	

private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionMetadataWindow)
};

