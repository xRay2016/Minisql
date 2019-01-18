#pragma once
#ifndef PARAMETER
#define PARAMETER

#include <fstream>
#include <string>
#include <set>
using namespace std;

class word_divider
{
public:
	static set<string> keyword;
	static set<string>  Operator;
	static set<string> seperater;
	static set<string>  constraints_word;
};


#endif // !PARAMETER

