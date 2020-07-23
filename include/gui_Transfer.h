#pragma once

#include "JUCE/JuceHeader.h"
#include "WaapiFunctions.h"

class CurrentWwiseConnection;
class CreateObjectChoices;

class renderJobAudioFile : public TreeViewItem
{
	std::string renderWav;
public:
	renderJobAudioFile(std::string audioFile)
	{
		renderWav = audioFile;
	}
	
	bool mightContainSubItems() override
	{
		return getNumSubItems() != 0;
	}

	void paintItem(Graphics& g, int width, int height) override
	{
		//g.fillAll(Colours::grey);
		g.setColour(Colours::black);
		g.drawText(renderWav, 0, 0, width, height, Justification::left);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(renderJobAudioFile)
};


class RenderQueTreeItem : public TreeViewItem
{
	std::string renderJobPath;
public:
	RenderQueTreeItem(std::string renderJob)
	{
		renderJobPath = renderJob;
		
	}
	
	void addAudioFileToRenderJobTree(std::string audioFile)
	{
		addSubItem(new renderJobAudioFile(audioFile));
	}

	bool mightContainSubItems() override
	{
		return getNumSubItems() != 0;
	}

	void paintItem(Graphics& g, int width, int height) override
	{
		//g.fillAll(Colours::grey);
		g.setColour(Colours::black);
		g.drawText(renderJobPath, 0, 0, width, height, Justification::left);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RenderQueTreeItem)
};

//==============================================================================
class TransferToWwiseComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:

	CurrentWwiseConnection MyCurrentWwiseConnection;

	TransferToWwiseComponent();

	void resized() override;

	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;

	void InitAllButtons(std::vector<juce::Button *> buttons);

	void InitComboBox(juce::ComboBox * comboBox, std::vector<std::string> choices);


private:

	CreateObjectChoices myCreateChoices;

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

	juce::ComboBox * dd_Language = new ComboBox("dd_Language"); //drop down
	juce::ComboBox * dd_EventOption = new ComboBox("dd_EventOption");
	juce::ComboBox * dd_CreateType = new ComboBox("dd_CreateType");
	juce::ComboBox * dd_OnNameConflict = new ComboBox("dd_OnNameConflict");

	std::vector<juce::ComboBox *> comboBoxes{
		dd_Language,
		dd_EventOption,
		dd_CreateType,
		dd_OnNameConflict
	};


	juce::Label * INtxt_OriginalsSubDir = new Label("INtxt_OriginalsSubDir");	// INPUT text - editable label
	juce::Label * INtxt_CreateName = new Label("INtxt_OriginalsSubDir");
	juce::Label * INtxt_CreateNotes = new Label("INtxt_OriginalsSubDir");
	juce::Label * txt_ConnectionStatus = new Label("INtxt_OriginalsSubDir"); // text

	juce::Label * debugLabel = new Label("INtxt_OriginalsSubDir");

	std::unique_ptr<TreeView> tree_RenderJobTree; //= new TreeView("tree_RenderJobTree");	//Tree view
	
	std::unique_ptr<TreeView> treeView;

	double transferProgress = 0.0f;

	StringArray ToJuceStringArray(std::vector<std::string>strings);


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

