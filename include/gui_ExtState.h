#pragma once
#pragma once
#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "platformhelpers.h"





///  Component ///
class ExtStateGuiComponent : public Component, public juce::Button::Listener, public juce::ComboBox::Listener, public juce::Label::Listener
{
public:

	Component* owningParentComponent = nullptr;

	Label* info = new Label("Settings");

	ExtStateGuiComponent()
	{

	}

	~ExtStateGuiComponent()
	{

	}

	void resized() override
	{

	}

	void buttonClicked(juce::Button* pButton)override {};

	void comboBoxChanged(ComboBox* comboBoxThatHasChanged)override {};

	void labelTextChanged(Label* labelThatHasChanged)override {};
private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtStateGuiComponent)
};

/// Settings Window
class ExtStateGuiWindow : public juce::DocumentWindow
{
	bool* mWindowState;


public:

	ExtStateGuiComponent* childComponent = nullptr;

	ExtStateGuiWindow(const juce::String& name, bool* windowStatus)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		childComponent = new ExtStateGuiComponent();
		setContentOwned(childComponent, true);
		childComponent->owningParentComponent = this;

		mWindowState = windowStatus;
		setUsingNativeTitleBar(true);
		centreWithSize(getWidth(), getHeight());
		setVisible(true);
	}

	void closeButtonPressed() override
	{
		auto parent = getParentComponent();
		*mWindowState = false;
		delete this;
	}

private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtStateGuiWindow)
};

