#pragma once

#include "JUCE/JuceHeader.h"
#include "waapi_structs.h"
#include "WwiseConnectionHandler.h"
#include "GUI.h"
#include "gui_RenderTree.h"


//class TransferToWwiseComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
class TransferToWwiseComponent : public BaseWwiseGuiComponent
{
	static TransferToWwiseComponent * currentTransferComponent;
	
	static void callback_OnSelectionChanged(uint64_t in_subscriptionId, const AK::WwiseAuthoringAPI::JsonProvider& in_jsonProvider)
	{
		if (TransferToWwiseComponent::currentTransferComponent)
		{
			TransferToWwiseComponent::currentTransferComponent->postCommandMessage(1);
		}
	}
	
	static void callback_OnProjectClosed(uint64_t in_subscriptionId, const AK::WwiseAuthoringAPI::JsonProvider& in_jsonProvider)
	{
		
		if (TransferToWwiseComponent::currentTransferComponent)
		{
			TransferToWwiseComponent::currentTransferComponent->postCommandMessage(2);
		}
	}
	
	
	
public:

	CreateImportWindow * thisCreateImportWindow;
	
	CurrentWwiseConnection * MyCurrentWwiseConnection;
	
	WwiseConnectionHandler * WwiseCntnHndlr;

	TransferToWwiseComponent();
	~TransferToWwiseComponent();

	void resized() override;
	
	void TryConnectToWwise();
	
	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;

	void InitAllButtons(std::vector<juce::Button *> buttons);

	void InitComboBox(juce::ComboBox * comboBox, std::vector<std::string> choices, string displayText);
	
	void InitTreeView();
	
	void RefreshRenderJobTree();
	
	bool CheckIsVoice();
	
	bool CheckOriginalsDirectory();
	
	void ApplySettingsToSelectedJobs();
	
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
		}
	}





private:

	CreateObjectChoices myCreateChoices;

	juce::TextButton * btn_RenderAndImport = new TextButton("Render And Import"); //button
	juce::TextButton * btn_RefreshJobList = new TextButton("Refresh Job List");
	juce::TextButton * btn_CreateWwiseObject = new TextButton("Create A Wwise Object");
	juce::TextButton * btn_ApplySettingsToJobs = new TextButton("Apply Import Settings To Selection");
	juce::TextButton * btn_ConnectToWwise = new TextButton("Connect To Wwise");
	juce::ToggleButton * btn_isVoice = new ToggleButton("Is Voice?");
	juce::ToggleButton * btn_OriginalsMatchesWwise = new ToggleButton("Originals Dir Matches Wwise?");
	juce::ToggleButton * btn_CreatePlayEvent = new ToggleButton("Create Play Event?");

	std::vector<juce::Button*> buttons{
	btn_RenderAndImport,
	btn_RefreshJobList,
	btn_CreateWwiseObject,
	btn_ApplySettingsToJobs,
	btn_ConnectToWwise,
	btn_isVoice,
	btn_OriginalsMatchesWwise,
	btn_CreatePlayEvent
	};

	juce::ComboBox * dd_Language = new ComboBox("dd_Language"); //drop down
	juce::ComboBox * dd_EventOption = new ComboBox("dd_EventOption");
	juce::Label * info_EventOption = new Label();
	juce::ComboBox * dd_CreateType = new ComboBox("dd_CreateType");
	juce::Label * info_CreateType = new Label();
	juce::ComboBox * dd_OnNameConflict = new ComboBox("dd_OnNameConflict");
	juce::Label * info_NameConflict = new Label();

	std::vector<juce::ComboBox *> comboBoxes{
		dd_Language,
		dd_EventOption,
		dd_CreateType,
		dd_OnNameConflict
	};
	
	juce::Label * Title_CreateWwiseObject = new Label();
	juce::Label * Title_RenderImport = new Label();

	juce::Label * INtxt_OriginalsSubDir = new Label();
	
	juce::Label * INtxt_CreateName = new Label();
	juce::Label * info_CreateName = new Label();
	
	juce::Label * INtxt_CreateNotes = new Label();
	juce::Label * info_CreateNotes = new Label();
	
	juce::Label * txt_ConnectionStatus = new Label(); // text

	juce::Label * debugLabel = new Label();
	
	juce::Label * selectedParentLabel = new Label();
	
//	juce::Label * statusLabel = new Label();

	//std::unique_ptr<TreeView> tree_RenderJobTree; //= new TreeView("tree_RenderJobTree");	//Tree view
	juce::TreeView * tree_RenderJobTree = new TreeView("tree_RenderJobTree");
	
	std::vector<juce::TreeViewItem *> RenderTreeSelectedItems;

//	double transferProgress = 0.0f;
//	juce::ProgressBar * progressBar = new ProgressBar(transferProgress);
	

	StringArray ToJuceStringArray(std::vector<std::string>strings);


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferToWwiseComponent)
};


static void callback_OnSelectionChanged(uint64_t in_subscriptionId, const AK::WwiseAuthoringAPI::JsonProvider& in_jsonProvider);


class TransferWindow : public juce::DocumentWindow
{
public:
	TransferWindow(const juce::String& name, juce::Component* component)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		setContentOwned(component, true);
		
		

		setResizable(true, false);
		setResizeLimits(500, 500, 10000, 10000);
		centreWithSize(getWidth(), getHeight());

		setVisible(true);
	}

	void closeButtonPressed() override
	{
		delete this;
	}

private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferWindow)
};

