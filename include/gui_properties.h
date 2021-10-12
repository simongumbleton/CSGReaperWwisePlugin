#pragma once

#include "JUCE/JuceHeader.h"
#include <string>
#include <map>
#include <vector>
#include "reaperHelpers.h"
#include "RapidJsonUtils.h"
#include <algorithm>

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
	
	std::string GetRegionName()
	{
		return regionName->getText().toStdString();
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
	std::map<std::string,std::string> GetPropertyValues()
	{
		std::map<std::string, std::string> results;
		for (auto property : properties)
		{
			results.emplace(property->getName().toStdString(),property->getText().toStdString());
		}
		return results;
	}

	std::map<std::string, std::string> ParseExtStateStringForValues(std::string ExtStateString)
	{
		std::map<std::string, std::string> results;
		//parse string
		// example svalue:  "{\"Attach\":\"value\",\"Tag\":\"new\"}"
		std::string str = ExtStateString;
		char* pch;
		printf("Splitting string \"%s\" into tokens:\n", str.c_str());
		//char delims[] = "\n !@#$%^&*)(_+-=][}{|:;'<>?,./\"\\";
		char delims[] = "!@#$%^&*)(+-=][}{|:;<>?,./\\";
		pch = strtok(&str[0], delims);
		std::vector<std::string> tempListValues;
		while (pch != NULL)
		{
			printf("%s\n", pch);
			tempListValues.push_back(std::string(pch));
			pch = strtok(NULL, delims);
		}
		
		int size = static_cast<int>(tempListValues.size());
		if ((size % 2) != 0)
		{
			PrintToConsole("Warning! Got an odd number of values when reading GetExtState!");
			return results;
		}
		int index = 0;
		while (index <= size - 2)
		{
			std::string key = tempListValues[index];
			std::string val = tempListValues[index + 1];
			key.erase(std::remove(key.begin(), key.end(), '\''), key.end());
			val.erase(std::remove(val.begin(), val.end(), '\''), val.end());
			results.emplace(key, val);
			index += 2;
		}
		return results;
	}
	
	void SetPropertyValuesFromExState()
	{
		// ANOTHERNEWREGION {'Attach':'','Tag':''}
		std::string RegionKey = this->GetRegionName();
		std::string svalue = "";
		int size;
		svalue = getProjExState(RegionKey);
		std::map<std::string, std::string> results;
		if (!svalue.empty())
		{	
			results = ParseExtStateStringForValues(svalue);
		}
		for (auto property : properties)
		{
			std::string svalue = "";
			int size;
			std::map<std::string, std::string>::iterator it;
			it = results.find(property->getName().toStdString());
			if (it != results.end())
			{
				std::string textValue = results[property->getName().toStdString()];
				if (!textValue.empty())
				{
					property->setText(textValue, false);
				}
			}
		}
	}


private:
	
	juce::Label * regionName = new Label();
	
	rapidjson::MemoryPoolAllocator<> jsonAllocator;

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
