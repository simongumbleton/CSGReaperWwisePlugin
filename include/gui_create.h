#pragma once

#include "JUCE/JuceHeader.h"
#include "waapi_structs.h"
#include "WwiseConnectionHandler.h"
#include "GUI.h"
#include "gui_RenderTree.h"
#include <stdio.h>
#include "reaperHelpers.h"

class TransferTabComponent;
//class TransferToWwiseComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
class CreateWwiseComponent : public BaseWwiseGuiComponent
{
	static CreateWwiseComponent * currentCreateComponent;

public:

	CreateImportWindow * thisCreateImportWindow;
	
	CurrentWwiseConnection * MyCurrentWwiseConnection;
	
	WwiseConnectionHandler * WwiseCntnHndlr;

	CreateWwiseComponent(juce::Component* parentComp);
	~CreateWwiseComponent();

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
	
	//void postCommandMessage(int commandId);
	//https://docs.juce.com/master/classComponent.html#a9ba6fa31d1397c7e90050b2cd4f2089b
	
	void handleCommandMessage (int commandId) override
	{
		switch(commandId) {
			case 1 : handle_OnSelectedParentChanged();
					 break;       // and exits the switch
			case 2 : handle_OnWwiseProjectClosed();
					 break;
			case 30: handle_OnBecameActiveTab();
					break;
			case 31: handle_OnTabBecameInactive();
					break;
		}
	}

	void setTransferValuesFromConfig(config c);
	
	void handle_OnBecameActiveTab();
	
	void handle_OnTabBecameInactive();

private:

	CreateObjectChoices myCreateChoices;
	TransferTabComponent* parent;
	config myConfig;
	uint64_t subscriptionID_selectionChanged =0;
	uint64_t subscriptionID_projectClosed=0;

	juce::TextButton * btn_CreateWwiseObject = new TextButton("Create A Wwise Object");
	juce::TextButton * btn_ConnectToWwise = new TextButton("Connect To Wwise");
	juce::ToggleButton * btn_CreatePlayEvent = new ToggleButton("Create Play Event?");

	std::vector<juce::Button*> buttons{
	btn_CreateWwiseObject,
	btn_ConnectToWwise,
	btn_CreatePlayEvent
	};

	juce::ComboBox * dd_CreateType = new ComboBox("dd_CreateType");
	juce::Label * info_CreateType = new Label();
	juce::ComboBox * dd_OnNameConflict = new ComboBox("dd_OnNameConflict");
	juce::Label * info_NameConflict = new Label();

	std::vector<juce::ComboBox *> comboBoxes{
		dd_CreateType,
		dd_OnNameConflict
	};
	
	juce::Label * Title_CreateWwiseObject = new Label();
	
	juce::Label * INtxt_CreateName = new Label();
	juce::Label * info_CreateName = new Label();
	
	juce::Label * INtxt_CreateNotes = new Label();
	juce::Label * info_CreateNotes = new Label();
	
	juce::Label * txt_ConnectionStatus = new Label(); // text

	juce::Label * debugLabel = new Label();
	
	juce::Label * selectedParentLabel = new Label();

	StringArray ToJuceStringArray(std::vector<std::string>strings);
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateWwiseComponent)
};
