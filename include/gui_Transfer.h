#pragma once

#include "JUCE/JuceHeader.h"

//==============================================================================
class TransferToWwiseComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:
	TransferToWwiseComponent();

	void resized() override;

	void buttonClicked(juce::Button* pButton);

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged);

	void labelTextChanged(Label* labelThatHasChanged);


private:

	

	juce::TextButton * btn_RenderAndImport = new TextButton("btn_RenderAndImport"); //button
	juce::TextButton * btn_RefreshJobList = new TextButton("btn_RefreshJobList");
	juce::TextButton * btn_CreateWwiseObject = new TextButton("btn_CreateWwiseObject");
	juce::TextButton * btn_ApplySettingsToJobs = new TextButton("btn_ApplySettingsToJobs");
	juce::TextButton * btn_ConnectToWwise = new TextButton("btn_ConnectToWwise");
	juce::ToggleButton * btn_isVoice = new ToggleButton("btn_isVoice");
	juce::ToggleButton * btn_OriginalsMatchesWwise = new ToggleButton("btn_OriginalsMatchesWwise");
	juce::ToggleButton * btn_CreatePlayEvent = new ToggleButton("btn_CreatePlayEvent");

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

	juce::ComboBox dd_Language; //drop down
	juce::ComboBox dd_EventOption;
	juce::ComboBox dd_CreateType;
	juce::ComboBox dd_OnNameConflict;


	
	juce::Label INtxt_OriginalsSubDir;	// INPUT text - editable label
	juce::Label INtxt_CreateName;
	juce::Label INtxt_CreateNotes;
	juce::Label txt_ConnectionStatus; // text

	juce::Label debugLabel;

	juce::TreeView tree_RenderJobTree;	//Tree view
	double transferProgress = 0.0f;


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferToWwiseComponent)
};

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

