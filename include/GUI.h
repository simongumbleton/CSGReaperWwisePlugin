#pragma once
#include "JUCE/JuceHeader.h"


/** Just a simple window that deletes itself when closed. */
class BasicWindow : public DocumentWindow
{
public:
	BasicWindow(const String& name, Colour backgroundColour, int buttonsNeeded)
		: DocumentWindow(name, backgroundColour, buttonsNeeded)
	{
	}

	void closeButtonPressed()
	{
		delete this;
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicWindow)
};

std::unique_ptr<BasicWindow> mainWindow;