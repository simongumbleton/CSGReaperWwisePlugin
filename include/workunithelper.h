#pragma once
#include "pugixml.hpp"
//https://pugixml.org/docs/quickstart.html


using namespace pugi;

class WorkUnitHelper
{
public:

	WorkUnitHelper()
	{
	}
	~WorkUnitHelper()
	{
	}

	xml_document doc;
	
	xml_parse_result LoadWorkUnit(std::string workUnitPath);
	
	void SaveWorkUnit(std::string workUnitPath);
	
	xml_node GetNodeByWwiseID(std::string objectID,std::string nodeType,xml_node searchRoot);
	
	xml_node GetNodeByWwiseName(std::string name,std::string nodeType,xml_node searchRoot);

	xml_node GetFirstNodeOfTypeUnderParent(std::string nodeType, xml_node searchRoot);
	
	xml_attribute GetAttribute(xml_node node,std::string attributeName);
	
	void SetAttributeValue(xml_attribute attribute,std::string attributeValue);
	
	void SetAttributeValue(xml_attribute attribute,int attributeValue);
	
	void SetAttributeValue(xml_attribute attribute,bool attributeValue);
	
	void SetAttributeValue(xml_attribute attribute,float attributeValue);
};

class WUActiveSourceUpdater : public WorkUnitHelper
{
public:
	WUActiveSourceUpdater()
	{
	}
	~WUActiveSourceUpdater()
	{
	}
	bool UpdateActiveSource(std::string soundID,std::string newActiveSourceName);
	
};
