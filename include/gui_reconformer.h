#pragma once
#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "EDL_Conformer.h"
#include "platformhelpers.h"
#include "gui_settings.h"


class DragDropHelper : public FileDragAndDropTarget, public Label
{
public:
	
	std::string fileExtensionToAccept = ".wav";
	std::string displayText = "Drag & drop here...";
	
	DragDropHelper(std::string fileExt){
		setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
		//if (!fileExt.empty()) {fileExtensionToAccept = fileExt;}
		fileExtensionToAccept = fileExt;
		resetDisplayText();
	};
	
	~DragDropHelper(){};
	
	//void paint (Graphics& g) override
	//{
		
		
		//g.setColour(Colours::lightblue);
		//g.fillRect(getLocalBounds().reduced(5));
	//}
	
	void resetDisplayText(std::string txtToDisplay = "")
	{
		if (!txtToDisplay.empty())
		{
			displayText = txtToDisplay;
		}
		setText(displayText, dontSendNotification);
	};

	bool isSet()
	{
		if (!fileExtensionToAccept.empty())
		{
			return PLATFORMHELPERS::ends_with(getText().toStdString(), fileExtensionToAccept);
		}
		//No ext specified
		else
		{
			if (getText().toStdString() == displayText || getText().isEmpty())
			{
				return false;
			}
		}
		return true;
	};
	
	bool isInterestedInFileDrag (const StringArray &files) override {
		if (files.isEmpty()) return false;
		if (fileExtensionToAccept.empty()) return true; // no ext specified, so as long as the array is not empty we are interested
		if (PLATFORMHELPERS::ends_with(PLATFORMHELPERS::stringToLower(files.begin()->toStdString()), fileExtensionToAccept))
		{
			return true;
		}
		return false;
	};
	 
	void fileDragEnter (const StringArray &files, int x, int y) override
	{
		if (isInterestedInFileDrag(files))
		{
			setColour(Label::outlineColourId,Colours::white);
		}
	};
	 
	void fileDragMove (const StringArray &files, int x, int y) override
	{
		
	};
	 
	void fileDragExit (const StringArray &files) override
	{
		setColour(Label::outlineColourId,getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	};
	 
	void filesDropped (const StringArray &files, int x, int y) override
	{
		if (files.isEmpty()) return;
		if (not isInterestedInFileDrag(files)) return;
		
		PrintToConsole(files.begin()->toStdString());
		setText(files.begin()->toStdString(), juce::NotificationType::sendNotification);
//		if (isSet())
//		{
//			setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
//			setColour(Label::textColourId, Colours::white);
//		}
//		else
//		{
//			setColour(Label::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
//			setColour(Label::textColourId, Colours::white);
//		}
		setColour(Label::outlineColourId,getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
		return;
		
	};
};


class PreviewArea : public Component
{
public:
	float maxDuration = std::numeric_limits<float>::max();
	
	bool canPreview = false;
	
	juce::Array<juce::Rectangle<float>> UnchangedRegions;
	juce::Array<juce::Rectangle<float>> ChangedRegions;
	juce::Array<juce::Rectangle<float>> AnimChangedRegions;
	
	Colour unchangedRegionColour = juce::Colours::lightgreen;
	Colour changedRegionColour = juce::Colours::yellow;
	Colour AnimChangedRegionColour = juce::Colours::mediumvioletred;
	
	PreviewArea(){
		//setSize(100, 50);
	};
	
	PreviewArea(Colour unchanged, Colour changed, Colour animchange)
	{
		unchangedRegionColour = unchanged;
		changedRegionColour = changed;
		AnimChangedRegionColour = animchange;
	}
	
	~PreviewArea(){};
	
	void resized() override
	{
		
	};
	
	void paint (juce::Graphics& g) override
	{
		g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
		g.fillAll();
		
		g.setColour (unchangedRegionColour);
		for (auto rectangle : UnchangedRegions)
		{
			juce::Rectangle<float> r = rectangle;
			r.setX(rectangle.getX()*getBoundsInParent().getWidth());
			r.setSize(rectangle.getWidth()*getBoundsInParent().getWidth(), 50);
			g.fillRect(r);
		}
		g.setColour(changedRegionColour);
		for (auto rectangle : ChangedRegions)
		{
			juce::Rectangle<float> r = rectangle;
			r.setX(rectangle.getX()*getBoundsInParent().getWidth());
			r.setSize(rectangle.getWidth()*getBoundsInParent().getWidth(), 50);
			g.fillRect(r);
		}
		g.setColour(AnimChangedRegionColour);
		for (auto rectangle : AnimChangedRegions)
		{
			juce::Rectangle<float> r = rectangle;
			r.setX(rectangle.getX() * getBoundsInParent().getWidth());
			r.setSize(rectangle.getWidth() * getBoundsInParent().getWidth(), 10);
			g.fillRect(r);
		}
	}
	
	void DrawRegions()
	{
	}
	
	void ClearRegions()
	{
		UnchangedRegions.clear();
		ChangedRegions.clear();
		AnimChangedRegions.clear();
		repaint();
	}
	
	void AddRegion(float startTime, float endTime,bool changed,bool animChanged=false)
	{
		float width = getBoundsInParent().getWidth();
		float normStart = (startTime/maxDuration);
		float normEnd = (endTime/maxDuration);
		juce::Rectangle<float> rec;
		rec.setPosition(normStart, 0);
		rec.setSize(normEnd-normStart, 50);
		if (changed)
		{
			ChangedRegions.add(rec);
		}else
		{
			UnchangedRegions.add(rec);
		}
	};
	void AddRegionForAnimChange(float startTime, float endTime)
	{
		float width = getBoundsInParent().getWidth();
		float normStart = (startTime/maxDuration);
		float normEnd = (endTime/maxDuration);
		juce::Rectangle<float> rec;
		rec.setPosition(normStart, 0);
		rec.setSize(normEnd-normStart, 10);
		AnimChangedRegions.add(rec);
	};
	
};


class ConformerComponent : public BaseWwiseGuiComponent
{
public:
	ConformerComponent();
	
	~ConformerComponent()
	{
		delete conformerComponent;
	};
	EDLconformer* conformerComponent = nullptr;
	
	EDLSettingsWnd* settingsWndHndl = nullptr;

	bool windowStatus = false;
	
private:
	
	
	juce::Label * txt_OldEdlTxt = new Label("Choose OLD EDL file...");
	
	juce::Label * txt_NewEdlTxt = new Label("Choose NEW EDL file...");
	
	juce::Label * txt_preview = new Label("Preview of conform...");

//	juce::Label* txt_assemblerTitle = new Label("Auto Assembly of Audio from EDL Animation Clip Info...");

//	juce::Label* txt_assemblerInfo = new Label("Location of audio files to use in assembly....");
	
	juce::TextButton * btn_ChooseOldEDL = new TextButton("...");
	
	juce::TextButton * btn_ChooseNewEDL = new TextButton("...");
	
	juce::TextButton * btn_DoConform = new TextButton("Do Conform");
	
	SettingsButton * btn_Settings = new SettingsButton("settingsBtn");
	
	DragDropHelper * dragDropTarget01 = new DragDropHelper(".edl");
	
	DragDropHelper * dragDropTarget02 = new DragDropHelper(".edl");
	
//	DragDropHelper * dragDropWavFolderTarget = new DragDropHelper("");
	
//	juce::TextButton * btn_AssembleAudio = new TextButton("Assemble Audio From EDL Anim Clip Info");
	
	PreviewArea * RegionPreview = new PreviewArea();
	
	void resized() override;

	void InitAllButtons(std::vector<juce::Button *> buttons);
	
	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;
	
	void TriggerConform();
	
	void DrawPreviewConform();
	
	void LaunchSettingsWindow();
	
	void SaveSettings();
	
	void GetCurrentSettings();
	
	std::vector<juce::Button*> buttons{
		btn_ChooseOldEDL,
		btn_ChooseNewEDL,
		btn_DoConform,
		helpButton,
//		btn_AssembleAudio,
		btn_Settings
	};
	
	File oldEDLFilepath;
	
	File newEDLFilepath;
	
//	std::string wavDirpath;
	 

};

class AssemblerComponent : public BaseWwiseGuiComponent
{
public:
	AssemblerComponent();

	~AssemblerComponent()
	{
		delete conformerComponent;
	};
	EDLconformer* conformerComponent = nullptr;

	EDLSettingsWnd* settingsWndHndl = nullptr;

	bool windowStatus = false;

private:
	juce::Label* txt_assemblerTitle = new Label("Auto Assembly of Audio from EDL Animation Clip Info...");

	juce::Label* txt_assemblerInfo = new Label("Location of audio files to use in assembly....");

	DragDropHelper* dragDropWavFolderTarget = new DragDropHelper("");

	juce::TextButton* btn_AssembleAudio = new TextButton("Assemble Audio From EDL Anim Clip Info");

	juce::Label* txt_NewEdlTxt = new Label("Choose NEW EDL file...");

	juce::TextButton* btn_ChooseNewEDL = new TextButton("...");

	DragDropHelper* dragDropTarget01 = new DragDropHelper(".edl");


	void resized() override;

	void InitAllButtons(std::vector<juce::Button*> buttons);

	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;

	void TriggerConform();

	void DrawPreviewConform();

	void LaunchSettingsWindow();

	void SaveSettings();

	void GetCurrentSettings();

	std::vector<juce::Button*> buttons{
		helpButton,
		btn_AssembleAudio
	};

	File newEDLFilepath;

	std::string wavDirpath;
};

class ConformerWindow : public juce::DocumentWindow
{
	bool * mWindowState;
public:
	ConformerWindow(const juce::String& name, juce::Component* component, bool *windowStatus)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		setContentOwned(component, true);
		setResizable(true, false);
		setResizeLimits(500, 600, 10000, 10000);
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
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConformerWindow)
};
