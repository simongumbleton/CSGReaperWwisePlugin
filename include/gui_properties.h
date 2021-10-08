#pragma once

#include "JUCE/JuceHeader.h"


//==============================================================================
class PropertiesComponent   : public Component
{
public:
	PropertiesComponent(const juce::String& name)
	{
		setOpaque (true);
		
		addAndMakeVisible(regionName);
		regionName->setText(name, juce::NotificationType::dontSendNotification);
		
		properties = createTextEditors();
		for (auto comp : properties)
		{
			addAndMakeVisible(comp);
		}
		
		setSize (500, 30);
	}
	void SetRegionName(const juce::String& name)
	{
		regionName->setText(name, juce::NotificationType::dontSendNotification);
	}

	void paint (Graphics& g) override
	{
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	}

	void resized() override
	{
		int x = 0;
		int y = 0;
		Font f = regionName->getFont();
		int textWidth = f.getStringWidth(regionName->getText());
		regionName->setBounds(x, y, textWidth+20, 25);
		x += textWidth+20;
		for (auto comp : properties)
		{
			comp->setBounds(x, y, 100, 25);
			x += 105;
			//y += 80;
		}
	}

private:
	
	juce::Label * regionName = new Label();
	
	Array<TextEditor*> properties;
	
	Array<TextEditor*> createTextEditors()
	{
		return
		{
			new TextEditor("Tag")
			,new TextEditor("Attach")
			//,new TextEditor("Value")
		};
	}
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertiesComponent)
};
