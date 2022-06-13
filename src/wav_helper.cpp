#include "wav_helper.h"
#include "WavData.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <iomanip>

void ReadBextChunk(std::string inFile)
{
    WavData* wav = new WavData();
    wav->read(inFile);
    auto bextChunk = wav->getChunk("bext");
    for (auto field : bextChunk->getAllFields())
    {
        auto name = field->name;
        auto val = field->val;// e.g. "zçf\x1" is hex. Need to read right to left e.g 01-66-8d-7a and convert to decimal to get the sample count e.g. 23498106

        if (name == "TimeReferenceLow")
        {
            std::vector<unsigned char>values;
            std::stringstream ss;

            for (const auto& item : val) {
                
                values.push_back(item);
            }
            ss << std::dec << std::hex << std::setw(2) << values[3] << std::hex << std::setw(2) << values[2] << std::hex << std::setw(2) << values[1] << std::hex << std::setw(2) << values[0];
            auto asHex = ss.str();
        }

        
    }
}
