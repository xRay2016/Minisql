#ifndef _PARSER_H
#define _PARSER_H
#include "lexical_analyzer.h"
#include "data_structure.h"
#include "error.h"
#include "Database.h"
#include "user.h"
#include <vector>
using namespace std;

using Token = str_pair;

class Tokens
{
private:
	vector<Token> tokens;
	int index = 0;
public:
	Tokens();
	Tokens(const Tokens& v);
	Tokens(const vector<Token>& v);
	int getIndex();
	//返回有无该下标起第i个单词
	bool hasNth(int i);
	//返回能否移进n个单词，不能的话抛出异常
	bool moveN(int n);
	//返回该下标前面第i个单词，没有则返回空串
	string nthWord(int i);
	//返回该下标前面第i个单词的类型，没有则返回-1
	type_def nthType(int i);
	//看是否能匹配该字符串，有的话移进且
		//返回真，没的话返回假
	bool match(const string& s);
	//看是否能匹配该字符串，有的话移进且返回真，
		//没有的话抛出异常
	bool _match(const string& s);
};

class Parser
{
	struct SelectParameter
	{
		vector<Table> from;
		boolTree* where;
		vector<string> gattr;
		boolTree* having;
		vector<string> select;
		vector<pair<string, UD>> oattr;
		int mode = 0;
	};
public:
	Parser();
	Parser(const Parser& p);
	Parser(const string& input);
	int state;
	static vector<SelectTree*> subSelectList;
	static int subSelectCnt;
private:
	int projectMode = 0;
	SelectParameter sp;
	authority parserS(string s);
	Database* dbinstance;
	string sql;
	static set<string> havingKeyword;
	static set<string> specificOperator;
	Tokens tokens;
	void execCmd();
	SelectTreeNode* execColumns(int i);
	SelectTreeNode* execFrom();
	boolTree* execCondition(bool having = false, bool check = false);
	void execConstraint(int& state, vector<string>& colname, vector<int>& type, int& primary, vector<int>unique_v);
	void execCol(int& state, vector<string>& colname, vector<int>& type);
	SelectTreeNode* execSelect(int type = 1);
	void execInsert();
	void execDelete();
	void execUpdate();
	void execCreateTable();
	void execDrop();
	void execCreateUser();
	void execGrantOrRevoke(bool g = true);
	SelectTreeNode* helper(const vector<Table>& v, int begin, int end);
};

#endif