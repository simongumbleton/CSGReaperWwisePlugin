#include "wav_helper.h"
#include "WavData.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <bit>

void WAV::ReadBextChunk(std::string inFile)
{
    uint32_t timelow = 0;
    uint32_t timehigh = 0;
    uint64_t timeTotal = 0;

    WavData* wav = new WavData();
    wav->read(inFile);
    auto bextChunk = wav->getChunk("bext");
    for (auto field : bextChunk->getAllFields())
    {
        auto name = field->name;
        auto val = field->val;// e.g. "zçf\x1" is hex. Need to read right to left e.g 01-66-8d-7a and convert to decimal to get the sample count e.g. 23498106

        if (name == "TimeReferenceLow")
        {
            if (val.empty())
            {
                continue;
            }
            std::memcpy(&timelow, val.c_str(), 4);
        }
        if (name == "TimeReferenceHigh")
        {
            if (val.empty())
            {
                continue;
            }
            std::memcpy(&timehigh, val.c_str(), 4);
        } 
    }

    timeTotal = (uint64_t)timehigh << 32 | timelow;
}

float WAV::GetBWFTimecode_Seconds(std::string inFile)
{
    uint32_t timelow = 0;
    uint32_t timehigh = 0;
    uint64_t timeTotal = 0;
    int sampleRate = GetBWFSampleRate(inFile);

    WavData* wav = new WavData();
    wav->read(inFile);
    auto bextChunk = wav->getChunk("bext");
    for (auto field : bextChunk->getAllFields())
    {
        auto name = field->name;
        auto val = field->val;// e.g. "zçf\x1" is hex. Need to read right to left e.g 01-66-8d-7a and convert to decimal to get the sample count e.g. 23498106

        if (name == "TimeReferenceLow")
        {
            if (val.empty())
            {
                continue;
            }
            std::memcpy(&timelow, val.c_str(), field->nBytes);
        }
        if (name == "TimeReferenceHigh")
        {
            if (val.empty())
            {
                continue;
            }
            std::memcpy(&timehigh, val.c_str(), field->nBytes);
        }
    }
    timeTotal = (uint64_t)timehigh << 32 | timelow;
    float seconds = (float)timeTotal / (float)sampleRate;
    return seconds;
}

uint32_t WAV::GetBWFSampleRate(std::string inFile)
{
    uint32_t sampleRate = 0;
    WavData* wav = new WavData();
    wav->read(inFile);
    auto fmtChunk = wav->getChunk("fmt "); // here "fmt " must include a space as this is how its defined in WavData?????
    for (auto field : fmtChunk->getAllFields())
    {
        if (field->name == "SamplesPerSec")
        {
            std::memcpy(&sampleRate, field->val.c_str(), field->nBytes);
            return sampleRate;
        }
    }
    return 48000; //didn't find a sample rate, return default
}
