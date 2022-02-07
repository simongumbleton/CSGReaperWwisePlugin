#pragma once

#include "JUCE/JuceHeader.h"
#include <string>
#include <map>
#include <vector>
#include "reaperHelpers.h"
#include "RapidJsonUtils.h"
#include <algorithm>
#include "settings_structs.h"


//==============================================================================
class PropertiesComponent   : public Component
{
public:

	String sRegionName;

	MetadataSettingsStruct regionPropertySettings;

	PropertiesComponent(const juce::String& name,std::vector<std::string>inProperties)
	{
		setOpaque (true);
		
		addAndMakeVisible(regionName);
		regionName->setText(name, juce::NotificationType::dontSendNotification);
		sRegionName = name;
		
		properties = createTextEditors(inProperties);
		for (auto comp : properties)
		{
			addAndMakeVisible(comp);
		}
		
		setSize (3000, 30);
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
		//g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
		g.fillAll(juce::Colours::darkslategrey);
	}

	void resized() override
	{
		int x = 1;
		int y = 2;
		Font f = regionName->getFont();
		int textWidth = f.getStringWidth(regionName->getText());

		regionName->setBounds(x, y, MaxTextWidth, 28);
		x += MaxTextWidth + 20;
		auto area = getLocalBounds();
		for (auto comp : properties)
		{
			comp->setBounds(x, y, 100, 26);
			//auto borderSize = juce::BorderSize(25);
			//comp->setBoundsInset(borderSize);
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
	
	void SaveRegionPropertiesToExState()
	{
			std::string name = this->GetRegionName();
			std::map<std::string, std::string> values = this->GetPropertyValues();
			std::stringstream valuesToJson;
			//"{ 'id': 1234, 'name': 'nandini' }"
			valuesToJson << '{';
			for (auto value : values)
			{
				valuesToJson << "'";
				valuesToJson << value.first;
				valuesToJson << "'";
				valuesToJson << ":";
				valuesToJson << "'";
				valuesToJson << value.second;
				valuesToJson << "'";
				valuesToJson << ",";
			}
			//PrintToConsole(valuesToJson.str());
			valuesToJson.seekp(-1,valuesToJson.cur); valuesToJson << "}";
			saveProjExState(name, valuesToJson.str());
	}

private:
	
	juce::Label * regionName = new Label();
	
	rapidjson::MemoryPoolAllocator<> jsonAllocator;

	Array<TextEditor*> properties;

	int MaxTextWidth = 300;
	
	Array<TextEditor*> createTextEditors(std::vector<std::string> properties)
	{
		if (properties.empty())
		{
			properties = { "tag","attach" };//make sure there are some defaults
		}
		Array<TextEditor*> textEditors;
		for (auto property : properties)
		{
			auto editor = new TextEditor(property);
			textEditors.add(editor);
		}
		return textEditors;
	//	{
	//		new TextEditor("tag")
	//		,new TextEditor("attach")
			//,new TextEditor("Value")
	//	};
	}
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertiesComponent)
};


//==============================================================================

class PropertiesViewportComponent : public Component
{
	std::vector<std::string> regionPropertyList;
public:
	PropertiesViewportComponent(std::vector<std::string> inRegionPropertyList)
	{
		regionPropertyList = inRegionPropertyList;
		setOpaque(true);

		refreshRegionsFromProject();
	}

	void UpdateRegionPropertyList(std::vector<std::string> inList)
	{
		regionPropertyList = inList;
		refreshRegionsFromProject();
	}

	void paint(Graphics& g) override
	{
		//g.fillAll(juce::Colours::aquamarine);
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	}

	void refreshRegionsFromProject()
	{
		removeAllChildren();

		if (!RegionProperties.isEmpty())
		{
			RegionProperties.clear();
		}

		titles.clear();
		for (auto title : regionPropertyList)
		{
			Label* t = new Label(title);
			t->setText(title,dontSendNotification);
			addAndMakeVisible(t);
			titles.add(t);
		}


		auto createdProperties = createProperties();
		for (auto region : createdProperties)
		{
			RegionProperties.add(region);
			addChildComponent(region);
			addAndMakeVisible(region);
			region->repaint();
			region->resized();
			region->SetPropertyValuesFromExState();
		}
		int sizeY = gridHeight * RegionProperties.size();
		setSize(550, sizeY);
		//setBounds(0, 0, 750, sizeY);
		repaint();
		resized();
	}

	void resized() override
	{
		auto area = getLocalBounds();

		auto titleArea = area.removeFromTop(30);
		int x = 1;
		int y = 2;
		int MaxTextWidth = 300;
		x += MaxTextWidth + 20;

		for (auto title : titles)
		{
			title->setBounds(x, y, 100, 26);
			//auto borderSize = juce::BorderSize(25);
			//comp->setBoundsInset(borderSize);
			x += 105;
			//y += 80;
		}

		for (auto region : RegionProperties)
		{
			auto propertyArea = area.removeFromTop(gridHeight);
			region->setBounds(propertyArea.reduced(1));
			//auto borderSize = juce::BorderSize(1);
			//region->setBoundsInset(borderSize);
		}

	}



	OwnedArray<PropertiesComponent> RegionProperties;

private:
	
	int gridHeight = 30;

	juce::Label* TitleRegions = new Label();
	OwnedArray<Label> titles;

	std::vector<PropertiesComponent*> createProperties()
	{// TO DO - count should be the number of regions
	std::vector <PropertiesComponent*> properties;
		for (auto region : getNonMasterProjectRegionNames())
		{
			properties.push_back(new PropertiesComponent(region, regionPropertyList));
		}
		//for (int i = 0;i < count;i++)
		//{
		//	properties.add(new PropertiesComponent("Region"));
		//}
		return properties;
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PropertiesViewportComponent)
};
