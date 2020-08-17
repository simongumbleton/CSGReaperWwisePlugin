#pragma once
#include <vector>

struct RenderQueJob;

//Test Functions

void ParseRenderQueFile(std::string pathToQueFile);

void TestReadRenderQue();


//Real functions

std::vector<std::string> GetListOfRenderQues();

RenderQueJob CreateRenderQueJobFromRenderQueFile(std::string pathToQueFile);
