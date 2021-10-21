#pragma once

#include "JUCE/JuceHeader.h"
#include "waapi_structs.h"
#include "WwiseConnectionHandler.h"
#include "GUI.h"
#include "gui_RenderTree.h"
#include <stdio.h>
#include "reaperHelpers.h"
#include "gui_create.h"

class TransferTabComponent;
//class TransferToWwiseComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
class TransferToWwiseComponent : public BaseWwiseGuiComponent
{
	static TransferToWwiseComponent * currentTransferComponent;

public:

	CreateImportWindow * thisCreateImportWindow;
	
	CurrentWwiseConnection * MyCurrentWwiseConnection;
	
	WwiseConnectionHandler * WwiseCntnHndlr;

	TransferToWwiseComponent(juce::Component* parentComp);
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
			case 30: handle_OnBecameActiveTab();
					break;
			case 31: handle_OnTabBecameInactive();
					break;
		}
	}

	void handle_OnBecameActiveTab();
	
	void handle_OnTabBecameInactive();

	void handle_Shutdown()
	{
		if (isSubscribed)
		{
			if ((MyCurrentWwiseConnection) && (MyCurrentWwiseConnection->connected) && (WwiseCntnHndlr))
			{
				WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_selectionChanged);
				WwiseCntnHndlr->UnsubscribeFromTopicByID(subscriptionID_projectClosed);
				isSubscribed = false;
			}
		}
	}
	 
	bool askUserForWwiseSubDir(std::string &OutSubDir);

	void setTransferValuesFromConfig(config c);

private:

	bool isSubscribed = false;
	CreateObjectChoices myCreateChoices;
	config myConfig;

	uint64_t subscriptionID_selectionChanged =0;
	uint64_t subscriptionID_projectClosed=0;
	TransferTabComponent* parent = nullptr;

	juce::TextButton * btn_RenderAndImport = new TextButton("Render And Import"); //button
	juce::TextButton * btn_RefreshJobList = new TextButton("Refresh Job List");

	juce::TextButton * btn_ApplySettingsToJobs = new TextButton("Apply Import Settings To Selection");
	juce::TextButton * btn_ConnectToWwise = new TextButton("Connect To Wwise");
	juce::TextButton * btn_ChooseWwiseOriginalsDir = new TextButton("Choose Originals Dir");
	juce::ToggleButton * btn_isVoice = new ToggleButton("Is Voice?");
	juce::ToggleButton * btn_OriginalsMatchesWwise = new ToggleButton("Originals Dir Matches Wwise?");

	std::vector<juce::Button*> buttons{
	btn_RenderAndImport,
	btn_RefreshJobList,
	btn_ApplySettingsToJobs,
	btn_ConnectToWwise,
	btn_isVoice,
	btn_OriginalsMatchesWwise,
	btn_ChooseWwiseOriginalsDir
	};

	juce::ComboBox * dd_Language = new ComboBox("dd_Language"); //drop down
	juce::ComboBox * dd_EventOption = new ComboBox("dd_EventOption");
	juce::Label * info_EventOption = new Label();

	std::vector<juce::ComboBox *> comboBoxes{
		dd_Language,
		dd_EventOption,
	};
	
	juce::Label * Title_RenderImport = new Label();

	juce::Label * INtxt_OriginalsSubDir = new Label();
	
	juce::Label * txt_ConnectionStatus = new Label(); // text

	juce::Label * debugLabel = new Label();
	
	juce::Label * selectedParentLabel = new Label();
	
//	juce::Label * statusLabel = new Label();

	//std::unique_ptr<TreeView> tree_RenderJobTree; //= new TreeView("tree_RenderJobTree");	//Tree view
	juce::TreeView * tree_RenderJobTree = new TreeView("tree_RenderJobTree");
	
	std::vector<juce::TreeViewItem *> RenderTreeSelectedItems;

//	double transferProgress = 0.0f;
//	juce::ProgressBar * progressBar = new ProgressBar(transferProgress);
	
	std::unique_ptr<FileChooser> myChooser;
	

	StringArray ToJuceStringArray(std::vector<std::string>strings);
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferToWwiseComponent)
};





class TransferWindow : public juce::DocumentWindow
{
	bool * mWindowState;
public:
	TransferWindow(const juce::String& name, juce::Component* component, bool *windowStatus)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		setContentOwned(component, true);
		
		mWindowState = windowStatus;
		*mWindowState = true;

		setResizable(true, false);
		setResizeLimits(500, 500, 10000, 10000);
		setSize(750, 600);
		centreWithSize(getWidth(), getHeight());

		setVisible(true);
	}

	void closeButtonPressed() override
	{
		*mWindowState = false;
		delete this;
	}

private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferWindow)
};

class TransferTabComponent : public juce::TabbedComponent
{
public:
	
	TransferToWwiseComponent* transferComp = nullptr;
	CreateWwiseComponent* createComp = nullptr;

	CreateImportWindow* thisCreateImportWindow = nullptr;
	CurrentWwiseConnection* MyCurrentWwiseConnection = nullptr;
	WwiseConnectionHandler* WwiseCntnHndlr = nullptr;

	TransferTabComponent(juce::TabbedButtonBar::Orientation orientation) : juce::TabbedComponent(orientation)
	{

		thisCreateImportWindow = new CreateImportWindow();

		WwiseCntnHndlr = thisCreateImportWindow->WwiseConnectionHnd;
		MyCurrentWwiseConnection = &thisCreateImportWindow->WwiseConnectionHnd->MyCurrentWwiseConnection;

		transferComp = new TransferToWwiseComponent(this);
		createComp = new CreateWwiseComponent(this);

		thisCreateImportWindow->OnInitDlg();

		if (!MyCurrentWwiseConnection->connected)
		{
			thisCreateImportWindow->handleUI_B_Connect();
		}
		
		addTab("Transfer",juce::Colours::darkslategrey,transferComp,true,0);
		addTab("Create",juce::Colours::darkslategrey,createComp,true,1);


		
	};




	~TransferTabComponent()
	{
		if ((thisCreateImportWindow)&&(thisCreateImportWindow->WwiseConnectionHnd))
		{
			if ((MyCurrentWwiseConnection)&&(MyCurrentWwiseConnection->connected))
			{
			//	thisCreateImportWindow->WwiseConnectionHnd->DisconnectFromWwise();
			}
		}

		if (transferComp)
		{
			transferComp->handle_Shutdown();
		}
		if (createComp)
		{
			createComp->handle_Shutdown();
		}


	};
	
	void currentTabChanged(int newCurrentTabIndex,const String & newCurrentTabName)
	{
		if (currentActiveTabIndex != newCurrentTabIndex)
		{
			if (getTabContentComponent(currentActiveTabIndex))
			{
				getTabContentComponent(currentActiveTabIndex)->handleCommandMessage(31);
			}
			if (getTabContentComponent(newCurrentTabIndex))
			{
				getTabContentComponent(newCurrentTabIndex)->handleCommandMessage(30);
			}
			currentActiveTabIndex = newCurrentTabIndex;
		}
		//PrintToConsole(newCurrentTabIndex);
	}
private:
	int currentActiveTabIndex = -1;
	
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferTabComponent)
};
