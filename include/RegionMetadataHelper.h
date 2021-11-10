#include <string>
#include <map>
#include <vector>
#include <set>

#include <json.hpp>

// for convenience
using json = nlohmann::json;

struct RegionInfo
{
	std::string name = "";//rInfo["name"] = name
	float start = 0.0f;//rInfo["start"] = pos
	float relativeStart = 0.0f; //rInfo["relativeStart"] = 0
	float end = 0.0f; //rInfo["end"] = rgnend
	int index = 0; //rInfo["index"]=markrgnindexnumber
	std::string master = ""; //rInfo["master"]="none"
	std::map<std::string,RegionInfo*> children; //rInfo["children"]={}
	std::string tag = ""; //rInfo["tag"]=""
	std::string attach = ""; //rInfo["attach"]=""
	std::set<std::string> events;
	~RegionInfo() { }
};

class ProjectRegionMetadataHelper
{
public:
	ProjectRegionMetadataHelper();
	
	~ProjectRegionMetadataHelper();
	
	
	std::map<std::string,RegionInfo> ProjectRegions;
	std::map<std::string,RegionInfo> MasterRegions;
	
	std::string projName;// = GetProjectName("","")
	std::string projPath;// = reaper.GetProjectPath("")
	//projPath = string.gsub(projPath, "Media","")
	std::string fullProjectFilepath;// = projPath..projName
	
	void fillProjectRegionInfos();
	
	bool isMasterRegion_Name(std::string regionName);
	
	bool isMasterRegion_Info(RegionInfo &regionInfo);
	
	void getProjectMasterRegions();
	
	void getMasterRegionForRegion(RegionInfo &rinfo);

	void getEventsForRegion(RegionInfo& rinfo);
	
	void getRelativeStartForChildrenInMaster(RegionInfo &master);
	
	void createMasterRegionData();

	void updateRegionGUIProperties(std::string RegionName, std::map<std::string, std::string> values);

	void writeMasterRegionInfoToJson();

	json RegionInfoToJson(RegionInfo& rinfo);

	bool CreateProjectRegionJsonFile(std::string filename,json data);

	static std::vector<std::string> GetEventListFromProjExtState();


	
};


