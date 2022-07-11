#include "Reaper_ExtState_Helper.h"


std::string EXTSTATE::transferSettingsName = "CSGTransferSettings";

std::string EXTSTATE::edlSettingsName = "CSGEDLSettings";

std::string EXTSTATE::regionMetadataSettingsName = "CSGRegionMetadataSettings";

std::string EXTSTATE::renderedFilesName = "CSGTransferRenderedFiles";

std::string EXTSTATE::transferEventsName = "CSGTransferWwiseEvents";




std::unordered_map<std::string, float> EXTSTATE::GetRenderOutputFilesFromProjExState(ReaProject* rProj)
{
	std::vector<std::string> tempListValues;
	std::unordered_map<std::string, float> results;
	//auto rProj = GetReaProjectFromProjectName(job.ParentReaperProject);
	if (!rProj)
	{
		rProj = GetCurrentReaProject();
	}

	if (rProj != nullptr) {

		std::string svalue = "";

		std::string name = EXTSTATE::renderedFilesName;// "CSGTransferRenderedFiles";
		//svalue = getProjExState("Transfer", "CSGTransferSettings");
		svalue = getProjExState("RENDEREDFILES", name, rProj);

		if (svalue.empty()) { return results; }

		char* pch;
		printf("Splitting string \"%s\" into tokens:\n", svalue.c_str());
		//char delims[] = "\n !@#$%^&*)(_+-=][}{|:;'<>?,./\"\\";
		char delims[] = "{,:}";
		pch = strtok(&svalue[0], delims);

		while (pch != NULL)
		{
			printf("%s\n", pch);
			std::string value = std::string(pch);
			value.erase(std::remove(value.begin(), value.end(), '\''), value.end());


			tempListValues.push_back(value);
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
			results.emplace(key, std::stof(val));
			index += 2;
		}


	}
	return results;
}