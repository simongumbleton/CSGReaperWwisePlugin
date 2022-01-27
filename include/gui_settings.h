#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "platformhelpers.h"
#include "cog.h"

struct TransferSettingsStruct
{
	
};

struct MetadataSettingsStruct
{
	std::vector<std::string>PropertyNames;
	
};

struct EDLSettingsStruct
{
	
};


class SettingsButton : public ImageButton
{
	Image cog = ImageFileFormat::loadFrom(cog::cog_png, cog::cog_pngSize);
public:
	
	SettingsButton(const String & name)
	{
		setImages(false, true, true, cog, 1.0f, Colours::transparentBlack, cog, 1.0f, Colours::transparentBlack, cog, 1.0f, Colours::transparentBlack);
	};
	
};
/*
class SettingsWindow : public DocumentWindow
{
public:
	SettingsWindow (juce::String name)  : DocumentWindow (name,
														 juce::Colours::lightgrey,
														 DocumentWindow::allButtons)
	   {
			setUsingNativeTitleBar(true);
			//setContentOwned(c, true);

			setResizable(true, false);
			//setResizeLimits(300, 250, 10000, 10000);
			//centreWithSize(getWidth(), getHeight());

			setVisible(true);
	   }

	   void closeButtonPressed() override
	   {
		   delete this;
	   }
	
	virtual void LoadSettings() {};
	
	virtual void SaveSettings() {};

   private:
	   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsWindow)
};
*/

class SettingsWindow : public juce::DocumentWindow
{
	bool* mWindowState;
public:
	SettingsWindow(const juce::String& name, /* juce::Component* component, */ bool* windowStatus)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		//setContentOwned(component, true);

		mWindowState = windowStatus;
		*mWindowState = true;

		setResizable(true, false);
		//setResizeLimits(500, 500, 10000, 10000);
		//setSize(750, 600);
		centreWithSize(getWidth(), getHeight());
		setWantsKeyboardFocus(true);
		setVisible(true);
		grabKeyboardFocus();
	}

	void closeButtonPressed() override
	{
		*mWindowState = false;
		delete this;
	}

	virtual void LoadSettings() {};

	virtual void SaveSettings() {};


private:

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};





class TransferSettingsWnd : public SettingsWindow
{
public:
};

class MetadataSettingsWnd : public SettingsWindow
{
public:
};

class EDLSettingsWnd : public SettingsWindow
{
public:
	EDLSettingsWnd (const juce::String name, bool* windowStatus)  : SettingsWindow(name, windowStatus)
	   {
	   }

	void closeButtonPressed() override
	   {
		   delete this;
	   }
	
	virtual void LoadSettings() override {};
	
	virtual void SaveSettings() override{};

   private:
	   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDLSettingsWnd)
};
