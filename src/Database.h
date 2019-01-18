#pragma once
#ifndef DATABASE
#define DATABASE
#include "user.h"
#include "api.h"
#include "lexical_analyzer.h"
#include "audit.h"
using namespace std;


class Database
{
private:
	static Database* singleton;
	vector<user*> all_users;
	vector<pair<string, string>> table_names;
	//vector<table*> all_tables;
	user* current_users;
	//vector<Table*> user_tables;
	vector<grant> grant_v;

	Database();
	Database(const Database&);
	Database operator=(const Database&);
	~Database();
	//初始化数据库
	bool initial();
	//正常结束的时候调用
	void end();
	int find_user(string uname);
	int toInt(string str);
	double toDouble(string operand);
	string toString(string str);

public:
	static ab temp_ab;
	static API* api_op;
	static Database* getInstance();
	static bool delete_Instance();
	static int is_recover;

	//获取有关表名以及所有者的信息
	vector<pair<string, string>> get_table_name();

	//Database登录时需要用到的函数
	bool load(string user_name, string password);

	void initial_users();

	//ok
	void create_user(int role_state, string unanme, string password, string sql);

	//void drop_user(string uname);

	//ok
	void add_grant(string uname, vector<grant> G, string sql);

	//ok
	void create_Table(string tname, vector<string>& colname, vector<int>& varType, int primary, vector<int>& unique_v, string sql);

	//ok
	void revoke_grant(string uname, vector<grant> G, string sql);


	//除了布尔树都ok
	void update_table(string tname, string attr_name, string value, int type, boolTree where, string sql);


	//暂时ok
	void delete_table(string tname, boolTree* where, string sql);

	//这个用来查中间结果
	Table select_table(vector<Table> from, boolTree* where, std::vector<std::string> gattr, boolTree* having, vector<pair<std::string, UD>> oattr, vector<string>select, int mode, string sql = "");

	//ok
	void drop_table(string tname, string sql);

	//ok
	void insert_table(string tname, vector<string> tuple, vector<int> type, string sql);

	Attribute getAttr(string table);
	Table setOperation(Table table1, Table table2, int operation);
	Table distinct(Table sourceTable);
	int TupleNum(string table_name);
	vector<string> getAllTable();
	//若要执行需要删掉里面的注释
	void execfile(string filen);

	void show_audit();
};

#endif // !DATABASE
