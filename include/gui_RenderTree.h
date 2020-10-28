//
//  gui_RenderTree.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 28/10/2020.
//  Copyright © 2020 My Company. All rights reserved.
//
#pragma once
#include "JUCE/JuceHeader.h"
#include "waapi_structs.h"

class renderJobAudioFileTreeItem : public TreeViewItem
{
	
public:
	std::string renderWav;
	std::string displayName;
	std::string importText;
	int textWidth = -1;
	
	renderJobAudioFileTreeItem(std::string audioFile)
	{
		renderWav = audioFile;
		displayName = audioFile.substr(audioFile.find_last_of("/\\") + 1);
	}
	
	bool mightContainSubItems() override
	{
		return getNumSubItems() != 0;
	}

	void paintItem(Graphics& g, int width, int height) override
	{
		//g.fillAll(Colours::grey);
		textWidth = g.getCurrentFont().getStringWidth(displayName);
		g.setColour(Colours::black);
		if (importText == "")
		{
			g.drawText(displayName, 0, 0, width, height, Justification::left);
		}
		else
		{
			g.drawText(displayName+" "+importText, 0, 0, width, height, Justification::left);
		}
	}
	int getItemWidth()
	{
		return textWidth;
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(renderJobAudioFileTreeItem)
};


class RenderQueJobTreeItem : public TreeViewItem
{
	
public:
	std::string renderJobPath;
	std::string displayName;
	std::string importText;
	
	RenderQueJobTreeItem(std::string renderJob)
	{
		renderJobPath = renderJob;
		displayName = renderJob.substr(renderJob.find_last_of("/\\") + 1);
	}
	
	void addAudioFileToRenderJobTree(renderJobAudioFileTreeItem * audioFile)
	{
		addSubItem(audioFile);
	}

	bool mightContainSubItems() override
	{
		return getNumSubItems() != 0;
	}

	void paintItem(Graphics& g, int width, int height) override
	{
		//g.fillAll(Colours::grey);
		g.setColour(Colours::black);
		if (importText == "")
		{
			g.drawText(displayName, 0, 0, width, height, Justification::left);
		}
		else
		{
			g.drawText(importText+" "+displayName, 0, 0, width, height, Justification::left);
		}
		
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RenderQueJobTreeItem)
};
class RenderQueTreeRoot : public TreeViewItem
{
public:
	RenderQueTreeRoot()
	{
	}
	
	void addRenderQueJobToTree(RenderQueJobTreeItem * renderJob,int index)
	{
		addSubItem(renderJob,index);
	}

	bool mightContainSubItems() override
	{
		return getNumSubItems() != 0;
	}

	void paintItem(Graphics& g, int width, int height) override
	{
		//g.fillAll(Colours::grey);
		g.setColour(Colours::black);
		g.drawText("ROOT", 0, 0, width, height, Justification::left);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RenderQueTreeRoot)
};
