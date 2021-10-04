#pragma once

#include "JUCE/JuceHeader.h"
#include "waapi_structs.h"
#include "WwiseConnectionHandler.h"
#include "GUI.h"
#include "gui_RenderTree.h"


//class WwiseTemplateComponent : public juce::Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
class WwiseTemplateComponent : public BaseWwiseGuiComponent
{
	static WwiseTemplateComponent * currentWwiseTemplateComponent;

public:

	CreateImportWindow * thisCreateImportWindow;
	
	CurrentWwiseConnection * MyCurrentWwiseConnection;
	
	WwiseConnectionHandler * WwiseCntnHndlr;

	WwiseTemplateComponent();
	~WwiseTemplateComponent();

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
		}
	}





private:

	juce::TextButton * btn_ConnectToWwise = new TextButton("Connect To Wwise");

	std::vector<juce::Button*> buttons{
	btn_ConnectToWwise,
	};


	
	
	juce::Label * txt_ConnectionStatus = new Label(); // text

	juce::Label * debugLabel = new Label();
	
	juce::Label * selectedParentLabel = new Label();
	


	

	StringArray ToJuceStringArray(std::vector<std::string>strings);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WwiseTemplateComponent)
};





class TemplateWindow : public juce::DocumentWindow
{
	bool * mWindowState;
public:
	TemplateWindow(const juce::String& name, juce::Component* component, bool *windowStatus)
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
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateWindow)
};

