#pragma once
#include <string>


namespace WAV
{
	void ReadBextChunk(std::string inFile);

	float GetBWFTimecode_Seconds(std::string inFile);

	uint32_t GetBWFSampleRate(std::string inFile);
}

