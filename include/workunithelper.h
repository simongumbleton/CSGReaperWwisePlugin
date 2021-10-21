#include "pugixml.hpp"
//https://pugixml.org/docs/quickstart.html


struct activeSourceUpdateInfo
{
	std::string parentSoundID;
	std::string newActiveSourceName;
	std::string workUnitPath;
};


using namespace pugi;

class WorkUnitHelper
{
public:
	xml_document doc;
	
	xml_parse_result LoadWorkUnit(std::string workUnitPath);
	
	void SaveWorkUnit(std::string workUnitPath);
	
	xml_node GetNodeByWwiseID(std::string objectID,std::string nodeType,xml_node searchRoot);
	
	xml_node GetNodeByWwiseName(std::string name,std::string nodeType,xml_node searchRoot);
	
	xml_attribute GetAttribute(xml_node node,std::string attributeName);
	
	void SetAttributeValue(xml_attribute attribute,std::string attributeValue);
	
	void SetAttributeValue(xml_attribute attribute,int attributeValue);
	
	void SetAttributeValue(xml_attribute attribute,bool attributeValue);
	
	void SetAttributeValue(xml_attribute attribute,float attributeValue);
};

class WUActiveSourceUpdater : WorkUnitHelper
{
	bool UpdateActiveSource(std::string workUnit,std::string soundID,std::string newActiveSourceName);
	
};
