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




class SettingsButton : public ImageButton
{
	Image cog = ImageFileFormat::loadFrom(cog::cog_png, cog::cog_pngSize);
public:
	
	SettingsButton(const String & name)
	{
		setImages(false, true, true, cog, 1.0f, Colours::transparentBlack, cog, 1.0f, Colours::transparentBlack, cog, 1.0f, Colours::transparentBlack);
	};
	
};

class SettingsComponent : public Component
{
public:
	
	Label* info = new Label("Settings");
	
	SettingsComponent(){
		info->setText("EDL Settings", NotificationType::dontSendNotification);
		addAndMakeVisible(info);
		setWantsKeyboardFocus(true);
		grabKeyboardFocus();
	};
	~SettingsComponent(){};
	
	void resized() override
	{
		auto area = getBoundsInParent();
		info->setBounds(area.removeFromTop(20));
		info->setJustificationType(Justification::centred);
	};
	
};

class SettingsWindow : public juce::DocumentWindow
{
	bool* mWindowState;
public:
	SettingsWindow(const juce::String& name,  juce::Component* component, bool* windowStatus)
		: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), juce::DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(true);
		setContentOwned(component, true);

		mWindowState = windowStatus;
		*mWindowState = true;

		setResizable(true, false);
		//setResizeLimits(500, 500, 10000, 10000);
		//setSize(750, 600);
		centreWithSize(getWidth(), getHeight());
		//setWantsKeyboardFocus(true);
		setVisible(true);
		//grabKeyboardFocus();
	}

	void closeButtonPressed() override
	{
		*mWindowState = false;
		delete this;
	}

	virtual void readSettingsFromParent(){};
	
	virtual void updateSettingsInParent(){};

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

class EDLSettingsCmp : public SettingsComponent
{
	

	
	
};

class EDLSettingsWnd : public SettingsWindow
{
public:
	EDLSettingsWnd (const juce::String name, bool* windowStatus)  : SettingsWindow(name, new SettingsComponent(), windowStatus)
	   {
	   }

	void closeButtonPressed() override
	   {
		   delete this;
	   }
	
	virtual void readSettingsFromParent()override
	{
		
	};
	
	virtual void updateSettingsInParent()override
	{
		
	};

   private:
	   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDLSettingsWnd)
};
