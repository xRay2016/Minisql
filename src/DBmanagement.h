#pragma once
#ifndef _DBMANAGEMENT
#define _DBMANAGEMENT

#include "Database.h"
#include "parser.h"
#include "audit.h"

using namespace std;

class front_service
{
public:
	void start_();
	string read_sql();
	front_service(int no);
	void process();
	void show_info();
	void write_log(vector<pair<string, bool>>& log_str);
	void read_log(vector<pair<string, bool>>& log_str);

private:
	int id;
	void management_();
	Database* db_instance;
	vector<string> log_v;
	vector<ab> ab_v;
	Parser* myParser;
	vector<string> sql_v;
	//string temp;
};





#endif // !_DBMANAGEMENT
