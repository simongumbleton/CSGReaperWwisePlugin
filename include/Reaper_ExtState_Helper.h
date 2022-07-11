#pragma once
#include <unordered_map>
#include "reaperHelpers.h"

namespace EXTSTATE
{

	extern std::string transferSettingsName;// = "CSGTransferSettings";

	extern std::string edlSettingsName;// = "CSGEDLSettings";

	extern std::string regionMetadataSettingsName;// = "CSGRegionMetadataSettings";

	extern std::string renderedFilesName;// = "CSGTransferRenderedFiles";

	extern std::string transferEventsName;// = "CSGTransferWwiseEvents";


	std::unordered_map<std::string, float> GetRenderOutputFilesFromProjExState(ReaProject* rProj);
}