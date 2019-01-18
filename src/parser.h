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
	//�������޸��±����i������
	bool hasNth(int i);
	//�����ܷ��ƽ�n�����ʣ����ܵĻ��׳��쳣
	bool moveN(int n);
	//���ظ��±�ǰ���i�����ʣ�û���򷵻ؿմ�
	string nthWord(int i);
	//���ظ��±�ǰ���i�����ʵ����ͣ�û���򷵻�-1
	type_def nthType(int i);
	//���Ƿ���ƥ����ַ������еĻ��ƽ���
		//�����棬û�Ļ����ؼ�
	bool match(const string& s);
	//���Ƿ���ƥ����ַ������еĻ��ƽ��ҷ����棬
		//û�еĻ��׳��쳣
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