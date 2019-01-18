
#ifndef _TEMPLATE_FUNCTION_H_
#define _TEMPLATE_FUNCTION_H_ 1
#include <sstream>
#include "data_structure.h"
#include "error.h"
using namespace std;
//以下是几个模版函数，目的是为了简化record_manager中的代码

//extern class boolTree;

template <typename T>
int getDataLength(T data) {
	std::stringstream stream;
	stream << data;
	return stream.str().length();
}

template <typename T>
bool isSatisfied(T a, T b, WHERE relation) {
	switch (relation) {
	case LESS: {
		if (a < b)
			return true;
		else
			return false;
	}; break;
	case LESS_OR_EQUAL: {
		if (a <= b)
			return true;
		else
			return false;
	}; break;
	case EQUAL: {
		if (a == b)
			return true;
		else
			return false;
	}; break;
	case GREATER_OR_EQUAL: {
		if (a >= b)
			return true;
		else
			return false;
	}; break;
	case GREATER: {
		if (a > b)
			return true;
		else
			return false;
	}; break;
	case NOT_EQUAL: {
		if (a != b)
			return true;
		else
			return false;
	}; break;
	}
}

template <typename T>
void copyString(char* p, int& offset, T data) {
	std::stringstream stream;
	stream << data;
	std::string s1 = stream.str();
	for (int i = 0; i < s1.length(); i++, offset++)
		p[offset] = s1[i];
}


//判断元组是否满足条件,非模板函数
bool TuplesatisTree(Tuple & tuple, boolTree&  tree, vector<int> indexes);

#endif