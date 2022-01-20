#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "EDL_Conformer.h"


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
	
	
	juce::TextButton * btn_ChooseOldEDL = new TextButton("Load EDL File 01");
	
	juce::TextButton * btn_ChooseNewEDL = new TextButton("Load EDL File 02");
	
	void resized() override;

	void InitAllButtons(std::vector<juce::Button *> buttons);
	
	void buttonClicked(juce::Button* pButton)override;

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override;

	void labelTextChanged(Label* labelThatHasChanged)override;
	
	std::vector<juce::Button*> buttons{
		btn_ChooseOldEDL,
		btn_ChooseNewEDL
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
