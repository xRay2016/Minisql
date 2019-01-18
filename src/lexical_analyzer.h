#pragma once
#ifndef LEXICAL_ANALYZER
#define LEXICAL_ANALYZER

#include "string_dealer.h"
#include "parameter.h"
#include "error.h"
using namespace std;

enum type_def {
	KEYWORD, OPERATOR, SEPERATER, CONSTRAINT_WORD, VAR, _STRING, _INT, _DOUBLE
};

struct str_pair {
	string str;
	type_def type;

	str_pair(string s, type_def t)
	{
		str = s;
		type = t;
	}

	str_pair()
	{

	}
};

class lexical_analyzer
{
public:
	static vector<str_pair> get_result(string str);

private:
	static bool match_var(string str);
	static bool match_const_string(string str);
	static bool match_const_int(string str);
	static bool match_const_double(string str);
};



#endif // !lexical_analyzer


