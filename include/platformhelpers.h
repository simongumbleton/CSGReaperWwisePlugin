//
//  platformhelpers.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 12/10/2020.
//  Copyright © 2020 My Company. All rights reserved.
//
#pragma once
#include <string>
#include <algorithm>
#include "JUCE/JuceHeader.h"
#include <cstdlib>
#include <regex>

#ifndef platformhelpers_h
#define platformhelpers_h

const char kPathSeparator =
#ifdef _WIN32
							'\\';
#else
							'/';
#endif

static std::string stringToLower(std::string input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c){ return std::tolower(c); });
	return result;
}

static std::string stringToUpper(std::string input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::toupper(c); });
	return result;
}

static bool stringIsNumber(std::string input)
{
	for (auto c : input)
	{
		if (!isdigit(c)) {return false;}
	}
	return true;
}

static bool stringIsFloat(std::string input)
{
	for (auto c : input)
	{
		if (!isdigit(c) and c != '.')
		{
			return false;
			
		}
	}
	return true;
}

static bool stringIsTimecode(std::string input)
{
	std::regex tcPattern("[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]");
	std::vector<std::string> matches;
	std::sregex_iterator iter(input.begin(), input.end(),tcPattern);
	std::sregex_iterator end;
	int count=0;
	while(iter != end)
	{
		for(unsigned i = 0; i < iter->size(); ++i)
		{
			matches.push_back((*iter)[i]);
		}
		++iter;
		count++;
	}
	return matches.size() == 1;
}


static std::string cleanWwisePathsFromMac(std::string input)
{
#ifndef _WIN32
	std::string home{std::getenv("HOME")};
	juce::String result = input;
	return result.replace("Y:", home).replace("\\", "/").toStdString();
#else
	return input;
#endif
}

static std::string stringReplace(std::string input, std::string from, std::string to)
{
	juce::String result = input;
	return result.replace(from, to).toStdString();
}

static std::string filenameFromPathString(std::string input)
{
	if (input.rfind(kPathSeparator) != input.npos)
	{
		input.erase(0, input.rfind(kPathSeparator) + 1);
	}
	return input;
}

static std::vector<std::string> stringSplitToList(std::string target, std::string delim)
{
	std::vector<std::string> v;
	if (!target.empty()) {
		std::string::size_type start = 0;
		do {
			size_t x = target.find(delim, start);
			if (x == target.npos)
			{
				//no more delimeters found so save the last remaining token
				std::string token = target.substr(start);
				v.push_back(token);
				break;
			}
			if (x != 0)
			{
				std::string token = target.substr(start, x-start);
				v.push_back(token);
				start += token.size() + delim.size();
			}
			else
			{//found delim at the start so skip adding an empty token and just move start forward by the delim size
				start += delim.size();
			}
			
			
		}
		while (true);
	}
	return v;
}
//...
inline bool ends_with(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

#endif /* platformhelpers_h */
