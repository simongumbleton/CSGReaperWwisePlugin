#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "EDL_Conformer.h"
#include "platformhelpers.h"
#include "gui_settings.h"


class DragDropHelper : public FileDragAndDropTarget, public Label
{
public:
	
	DragDropHelper(){
		setColour(Label::backgroundColourId, Colours::lightseagreen.withAlpha(0.5f));
	};
	
	~DragDropHelper(){};
	
	//void paint (Graphics& g) override
	//{
		
		
		//g.setColour(Colours::lightblue);
		//g.fillRect(getLocalBounds().reduced(5));
	//}
	
	bool isSet()
	{
		return ends_with(getText().toStdString(), ".edl");
	};
	
	bool isInterestedInFileDrag (const StringArray &files) override {
		if (files.isEmpty()) return false;
		if (ends_with(stringToLower(files.begin()->toStdString()), ".edl"))
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
	float maxDuration;
	
	bool canPreview = false;
	
	juce::Array<juce::Rectangle<float>> UnchangedRegions;
	juce::Array<juce::Rectangle<float>> ChangedRegions;
	
	PreviewArea(){
		//setSize(100, 50);
	};
	
	~PreviewArea(){};
	
	void resized() override
	{
		
	};
	
	void paint (juce::Graphics& g) override
	{
		g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
		g.fillAll();
		
		g.setColour (juce::Colours::lightgreen);
		for (auto rectangle : UnchangedRegions)
		{
			juce::Rectangle<float> r = rectangle;
			r.setX(rectangle.getX()*getBoundsInParent().getWidth());
			r.setSize(rectangle.getWidth()*getBoundsInParent().getWidth(), 50);
			g.fillRect(r);
		}
		g.setColour(juce::Colours::yellow);
		for (auto rectangle : ChangedRegions)
		{
			juce::Rectangle<float> r = rectangle;
			r.setX(rectangle.getX()*getBoundsInParent().getWidth());
			r.setSize(rectangle.getWidth()*getBoundsInParent().getWidth(), 50);
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
		repaint();
	}
	
	void AddRegion(float startTime, float endTime,bool changed)
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
	
};


class ConformerComponent : public BaseWwiseGuiComponent
{
public:
	ConformerComponent();
	
	~ConformerComponent()
	{
		delete conformer;
	};

private:
	EDLconformer* conformer = nullptr;
	
	EDLSettingsWnd* settings = nullptr;

	bool windowStatus;
	
	juce::Label * txt_OldEdlTxt = new Label("Choose OLD EDL file...");
	
	juce::Label * txt_NewEdlTxt = new Label("Choose NEW EDL file...");
	
	juce::Label * txt_preview = new Label("Preview of conform...");
	
	juce::TextButton * btn_ChooseOldEDL = new TextButton("...");
	
	juce::TextButton * btn_ChooseNewEDL = new TextButton("...");
	
	juce::TextButton * btn_DoConform = new TextButton("Do Conform");
	
	SettingsButton * btn_Settings = new SettingsButton("settingsBtn");
	
	DragDropHelper * dragDropTarget01 = new DragDropHelper();
	
	DragDropHelper * dragDropTarget02 = new DragDropHelper();
	
	PreviewArea * RegionPreview = new PreviewArea();
	
	void resized() override;

	void InitAllButtons(std::vector<juce::Button *> buttons);
	
	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;
	
	void TriggerConform();
	
	void DrawPreviewConform();
	
	void LaunchSettings();
	
	void SaveSettings();
	
	void LoadSettings();
	
	std::vector<juce::Button*> buttons{
		btn_ChooseOldEDL,
		btn_ChooseNewEDL,
		btn_DoConform,
		helpButton
		,btn_Settings
	};
	
	std::unique_ptr<FileChooser> myChooser;
	
	File oldEDLFilepath;
	
	File newEDLFilepath;
	 
	File askUserForFile(std::string message = "Select EDL file",std::string extension = "*.edl")
	{
		myChooser = std::make_unique<FileChooser> (message,
												   File::getSpecialLocation (File::userHomeDirectory),
												   extension);
		myChooser->browseForFileToOpen();
		return myChooser->getResult();
	}
	
	
	
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
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConformerWindow)
};
