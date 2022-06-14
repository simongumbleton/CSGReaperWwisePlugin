#pragma once
#include <unordered_map>
#include "reaperHelpers.h"

namespace EXTSTATE
{
	std::unordered_map<std::string, float> GetRenderOutputFilesFromProjExState(ReaProject* rProj);
}