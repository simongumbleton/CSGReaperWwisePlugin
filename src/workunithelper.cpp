//
//  workunithelper.cpp
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 21/10/2021.
//  Copyright © 2021 My Company. All rights reserved.
//

#include <stdio.h>
#include "workunithelper.h"

using namespace pugi;

	
xml_parse_result WorkUnitHelper::LoadWorkUnit(std::string workUnitPath)
{
	return doc.load_string(workUnitPath.c_str());
};
	
void WorkUnitHelper::SaveWorkUnit(std::string workUnitPath)
{
	doc.save_file(workUnitPath.c_str());
};


xml_node WorkUnitHelper::GetNodeByWwiseID(std::string objectID,std::string nodeType,xml_node searchRoot=xml_node())
{
	xml_node root;
	if (searchRoot.empty())
	{
		root = doc.document_element();
	}
	else{
		root = searchRoot;
	}
	// Search for the first / last child entry with the given hint attribute
	std::string searchStr = nodeType+"[@ID="+objectID+"]";
	xpath_node xpathNode = root.select_node(searchStr.c_str());
	if (xpathNode)
	{
		xml_node selectedNode = xpathNode.node();
			// now access found node
			// ...
		return selectedNode;
	}

	return xml_node();
};

xml_node WorkUnitHelper::GetNodeByWwiseName(std::string name,std::string nodeType,xml_node searchRoot=xml_node())
{
	xml_node root;
	if (searchRoot.empty())
	{
		root = doc.document_element();
	}
	else
	{
		root = searchRoot;
	}
	// Search for the first / last child entry with the given hint attribute
	std::string searchStr = nodeType+"[@Name="+name+"]";
	xpath_node xpathNode = root.select_node(searchStr.c_str());
	if (xpathNode)
	{
		xml_node selectedNode = xpathNode.node();
			// now access found node
			// ...
		return selectedNode;
	}

	return xml_node();
};

xml_attribute WorkUnitHelper::GetAttribute(xml_node node,std::string attributeName)
{
	return node.attribute(attributeName.c_str());
};

void WorkUnitHelper::SetAttributeValue(xml_attribute attribute,std::string attributeValue)
{
	attribute.set_value(attributeValue.c_str());
};

void WorkUnitHelper::SetAttributeValue(xml_attribute attribute,int attributeValue)
{
	attribute.set_value(attributeValue);
};

void WorkUnitHelper::SetAttributeValue(xml_attribute attribute,bool attributeValue)
{
	attribute.set_value(attributeValue);
};

void WorkUnitHelper::SetAttributeValue(xml_attribute attribute,float attributeValue)
{
	attribute.set_value(attributeValue);
};


bool WUActiveSourceUpdater::UpdateActiveSource(std::string workUnit,std::string soundID,std::string newActiveSourceName)
{
	xml_node soundNode = GetNodeByWwiseID(soundID, "Sound");
	if (!soundNode.empty())
	{
		//need to first find the audio source child matching the name, so we can get the ID
		xml_node audioSource = GetNodeByWwiseName(newActiveSourceName, "AudioFileSource",soundNode);
		if (!audioSource.empty())
		{
			xml_attribute newActiveSourceIDAttr = GetAttribute(audioSource, "ID");
			std::string newActiveSourceID = newActiveSourceIDAttr.value();
			
			xml_node activeSource = GetNodeByWwiseName(newActiveSourceName, "ActiveSource",soundNode);
			if (!activeSource.empty())
			{
				xml_attribute nameAttr = GetAttribute(activeSource, "Name");
				SetAttributeValue(nameAttr, newActiveSourceName);
				xml_attribute IDAttr = GetAttribute(activeSource, "ID");
				SetAttributeValue(IDAttr, newActiveSourceID);
				return true;
			}
		}
		//then find the active source node and update the name AND id values
	}
	return false;
};

