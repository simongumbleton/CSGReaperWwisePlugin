#pragma once
#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "platformhelpers.h"
#include "cog.h"
#include "settings_structs.h"




///// Settings Button	////////
class SettingsButton : public ImageButton
{
	
public:
	SettingsButton(const String & name);
	~SettingsButton();
private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsButton)
};

/// Settings Component ///
class SettingsComponent : public Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:

	Component* owningParentComponent = nullptr;
	
	Label* info = new Label("Settings");
	
	SettingsComponent();
	
	~SettingsComponent();

	void InitComboBox(juce::ComboBox* comboBox, std::vector<String> choices);
	
	void resized() override;
	
	void buttonClicked(juce::Button* pButton)override{};

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override{};

	void labelTextChanged(Label* labelThatHasChanged)override{};
private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};

/// Settings Window
class SettingsWindow : public juce::DocumentWindow
{
	bool* mWindowState;

	
public:

	SettingsComponent* childSettingsComponent = nullptr;

	SettingsWindow(const juce::String& name, bool* windowStatus);

	void closeButtonPressed() override
	{
		auto parent = getParentComponent();
		*mWindowState = false;
		delete this;
	}

	virtual void readSettingsFromParent(){};
	
	virtual void updateSettingsInParent(){};

private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};







class DynamicTextEntry : public Component, public juce::Label::Listener
{
public:
	//std::vector<TextEditor *> TextEditors;
	OwnedArray<TextEditor> TextEditors;
	Label* Info = new Label("info");
	
	
	DynamicTextEntry(std::vector<std::string>inList)
	{
		addAndMakeVisible(Info);
		Info->setText("List of properties to create for regions",dontSendNotification);
		if (!inList.empty())
		{
			for (auto txt : inList)
			{
				AddTextEditor(txt);
			}
			UpdateTextEditors();
		}
		setVisible(true);
	};
	
	~DynamicTextEntry(){};

	void AddTextEditor(std::string inText = "")
	{
		auto newEditor = new TextEditor(inText);
		addAndMakeVisible(newEditor);
		newEditor->setText(inText, dontSendNotification);
		TextEditors.add(newEditor);
	};

	void RemoveTextEditor() 
	{
		if (TextEditors.isEmpty()) return;
		TextEditors.removeLast();
	};

	std::vector<std::string> GetTextValues()
	{
		std::vector<std::string> textValues;
		for (auto editor : TextEditors)
		{
			textValues.push_back(editor->getText().toStdString());
		}
		return textValues;
	};
	
	void UpdateTextEditors()
	{
		auto area = getLocalBounds();
		auto height = 30;
		int i = 1;
		Info->setBounds(area.removeFromTop(30));
		for (auto editor : TextEditors)
		{
			editor->setBounds(area.removeFromTop(height));
			editor->setBorder(BorderSize(2));
			i++;
		}
	}


	void resized() override
	{
		UpdateTextEditors();
	};
	
	void labelTextChanged(Label* labelThatHasChanged)override{};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicTextEntry);
};


class MetadataSettingsCmp : public SettingsComponent
{
	MetadataSettingsStruct& rSettings;

	TextButton* btn_AddTextItem = new TextButton("AddTextItem");
	TextButton* btn_RemoveTextItem = new TextButton("RemoveTextItem");
	DynamicTextEntry* TextEntryCmpt;
	Viewport* myViewport = new Viewport();
	
	
public:
	MetadataSettingsCmp(MetadataSettingsStruct& inSettings) :rSettings(inSettings){

		String name = btn_AddTextItem->getName();
		btn_AddTextItem->setButtonText(btn_AddTextItem->getName());
		btn_AddTextItem->addListener(this);
		addAndMakeVisible(btn_AddTextItem);

		name = btn_RemoveTextItem->getName();
		btn_RemoveTextItem->setButtonText(btn_RemoveTextItem->getName());
		btn_RemoveTextItem->addListener(this);
		addAndMakeVisible(btn_RemoveTextItem);
		
		TextEntryCmpt = new DynamicTextEntry(rSettings.PropertyNames);

		TextEntryCmpt->setSize(200, 1000);
		//TextEntryCmpt->setBounds(0, 0, 200, 400);
		addAndMakeVisible(TextEntryCmpt);
		//myViewport->setSize(50, 50);
		myViewport->setViewedComponent(TextEntryCmpt, true);
		addAndMakeVisible(myViewport);

		setVisible(true);
	};
	
	~MetadataSettingsCmp() 
	{
		rSettings.PropertyNames = TextEntryCmpt->GetTextValues();
		if (owningParentComponent)
		{
			auto MainWindow = owningParentComponent->getParentComponent();
			MainWindow->postCommandMessage(11); //should be gui_metadatacomponent UpdateRegionPropertiesFromSettings()
		}
	};

	void resized() override {
		auto area = getLocalBounds();
		auto buttonArea = area.removeFromTop(30);
		btn_AddTextItem->setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2));
		btn_RemoveTextItem->setBounds(buttonArea);

		auto viewportArea = area.removeFromTop(200);
		myViewport->setBounds(viewportArea.reduced(10));
	}

	void buttonClicked(juce::Button* pButton)override 
	{
		if (pButton == btn_AddTextItem)
		{
			TextEntryCmpt->AddTextEditor();
			TextEntryCmpt->UpdateTextEditors();
			TextEntryCmpt->repaint();
		}else
		if (pButton == btn_RemoveTextItem)
		{
			TextEntryCmpt->RemoveTextEditor();
			TextEntryCmpt->UpdateTextEditors();
			TextEntryCmpt->repaint();
		}
	}

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override {}

	void labelTextChanged(Label* labelThatHasChanged)override {}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetadataSettingsCmp);
};

class MetadataSettingsWnd : public SettingsWindow
{
	
public:
	MetadataSettingsCmp* settingsComp;

	MetadataSettingsWnd(const juce::String name, MetadataSettingsStruct& inSettings, bool& windowStatus) : SettingsWindow(name, &windowStatus)
	{
		
		settingsComp = new MetadataSettingsCmp(inSettings);
		setContentOwned(settingsComp, true);
		childSettingsComponent = settingsComp;
		childSettingsComponent->owningParentComponent = this;
		
		
		setUsingNativeTitleBar(true);
		centreWithSize(getWidth(), getHeight());
		setVisible(true);



	}
	
//	void resized()override
//	{
//		auto viewportArea = getLocalBounds().removeFromTop(400);
//		myViewport->setBounds(viewportArea.reduced(50));
//	};
	
	virtual void readSettingsFromParent()override
	{

	};

	virtual void updateSettingsInParent()override
	{

	};

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetadataSettingsWnd)
};

class EDLSettingsCmp : public SettingsComponent
{
	EDLSettingsStruct& rSettings;
	
	std::vector<String> supportedFramerates
	{
		"30",//default first
		"24",
		"25" ,
		"29.97",
		"59.97",
		"60",
		"120",
	};
	
	int getSavedFramerateAsID();
	
public:
	TooltipWindow tooltipWindow {nullptr,750};
	Label * info_StartOffset = new Label("info_StartOffset");
	Label * txt_StartOffset = new Label("txt_StartOffset");
	Label * info_Framerate = new Label("info_Framerate");
	Label * txt_Framerate = new Label("txt_Framerate");
	ComboBox * dd_Framerate = new ComboBox("dd_Framerate");
	ToggleButton * btn_EDLRegions = new ToggleButton("Create EDL Regions");
	ToggleButton * btn_ExistingRegions = new ToggleButton("Copy Existing Regions");
	ToggleButton * btn_ChangedShots = new ToggleButton("Create Regions for Changed Shots");
	
	//void InitComboBox(juce::ComboBox * comboBox, std::vector<String> choices);
	
	
	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;

	EDLSettingsCmp(EDLSettingsStruct& inSettings);
	
	~EDLSettingsCmp(){};
	
	void resized() override;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDLSettingsCmp)
};

class EDLSettingsWnd : public SettingsWindow
{
public:
	EDLSettingsCmp * settingsComp;
	
	EDLSettingsWnd (const juce::String name, EDLSettingsStruct& inSettings, bool &windowStatus)  : SettingsWindow(name, &windowStatus)
	   {
		   settingsComp = new EDLSettingsCmp(inSettings);
		   setContentOwned(settingsComp, true);
		   childSettingsComponent = settingsComp;
		   childSettingsComponent->owningParentComponent = this;
	   }
	
	virtual void readSettingsFromParent()override
	{
		
	};
	
	virtual void updateSettingsInParent()override
	{
		
	};

 private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDLSettingsWnd)

};


class TransferSettingsCmp : public SettingsComponent
{
	TransferSettingsStruct& rSettings;

public:
	TooltipWindow tooltipWindow{ nullptr,750 };

	Label* info_waapiPort = new Label("info_waapiPort");
	Label* txt_waapiPort = new Label("txt_waapiPort");

	Label* info_useAtomationMode = new Label("info_useAtomationMode");
	ToggleButton* btn_useAtomationMode = new ToggleButton("useAtomationMode");

	Label* info_userorigsubdir = new Label("info_userorigsubdir");
	Label* txt_userorigsubdir = new Label("txt_userorigsubdir");

	Label* info_versionToken = new Label("info_versionToken");
	Label* txt_versionToken = new Label("txt_versionToken");

	Label* info_templatePath = new Label("info_templatePath");
	Label* txt_templatePath = new Label("txt_templatePath");

	Label* info_eventWorkUnitSuffix = new Label("info_eventWorkUnitSuffix");
	Label* txt_eventWorkUnitSuffix = new Label("txt_eventWorkUnitSuffix");

	Label* info_UserEventPath = new Label("info_UserEventPath");
	Label* txt_UserEventPath = new Label("txt_UserEventPath");


	Label* info_eventcreationoption = new Label("info_eventcreationoption");
	//Label* txt_eventcreationoption = new Label("txt_eventcreationoption");
	ComboBox* dd_eventcreationoption = new ComboBox("dd_eventcreationoption");

	std::vector<String> eventCreationOptions;
	

	//void InitComboBox(juce::ComboBox* comboBox, std::vector<String> choices);


	//void buttonClicked(juce::Button* pButton)override;

	//void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	//void labelTextChanged(Label* labelThatHasChanged)override;

	TransferSettingsCmp(TransferSettingsStruct& inSettings);

	~TransferSettingsCmp() {};

	//void resized() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferSettingsCmp)
};



class TransferSettingsWnd : public SettingsWindow
{
public:
	TransferSettingsCmp* settingsComp;

	TransferSettingsWnd(const juce::String name, TransferSettingsStruct& inSettings, bool& windowStatus) : SettingsWindow(name, &windowStatus)
	{
		settingsComp = new TransferSettingsCmp(inSettings);
		setContentOwned(settingsComp, true);
		childSettingsComponent = settingsComp;
		childSettingsComponent->owningParentComponent = this;
	}

	virtual void readSettingsFromParent()override
	{

	};

	virtual void updateSettingsInParent()override
	{

	};

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferSettingsWnd)
};
