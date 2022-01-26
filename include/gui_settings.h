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

class SettingsWindow : public DocumentWindow
{
public:
	SettingsWindow (juce::String name)  : DocumentWindow (name,
														 juce::Colours::lightgrey,
														 DocumentWindow::allButtons)
	   {
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
	EDLSettingsWnd (juce::String name)  : SettingsWindow(name)
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
