#pragma once
#include "WwiseConnectionHandler.h"


//Test Functions

void ParseRenderQueFile(std::string pathToQueFile);

void TestReadRenderQue();


//Real functions

std::vector<std::string> GetListOfRenderQues();

RenderQueJob CreateRenderQueJobFromRenderQueFile(std::string pathToQueFile);