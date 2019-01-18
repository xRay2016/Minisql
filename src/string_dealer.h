#pragma once
#ifndef STRING_DEALER
#define STRING_DEALER

#include <string>
#include <iostream>
#include <vector>
#include <regex>
#include <fstream>
using namespace std;



class string_dealer {
public:
	static string get_string(string str);//ȥ�����з����ҰѴ�д����ĸ��ΪСд

	static vector<string> divide_string(string str);

private:
	static vector<string> splitEx(const string& src, string sep);

	static string& trim(string &str);

	static bool is_number(char c);
};

#endif // !STRING_DEALER

