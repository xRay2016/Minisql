#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
enum op_object {
	TABLE, USER
};
enum op_type {
	Create_t, Create_u, Update_t, Grant_u, Revoke_u, Insert_t, Delete_t, Select_t, Drop_t
};

class ab {
public:
	ab(string un, string s, int rs, op_object ob, op_type ot, bool r = false);
	ab();
	void redefine(string un, string s, int rs, op_object ob, op_type ot, bool r = false);
	string current_un;
	int rolestate;
	op_object object;
	op_type type;
	string str;
	bool b;
};

void show_ab(vector<ab>ab_v);

class log_management {
public:
	log_management();
	//我写好了，不用管
	static void write_log(vector<string> logs);


	static void write_ab(ab sql);
	static void write_ab(vector<ab> sqls);

	//我写好了，不用管
	static void write_log(string log);
	//我写好了，不用管
	static void initial_();

	static void read_ab(vector<ab>&v_sql);

	//我写好了，不用管
	static void read_log(vector<string>& logs);

};