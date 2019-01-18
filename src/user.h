#pragma once
#ifndef USER_H
#define USER_H
#define DBA_ROLE 100
#define NORMAL_USER 50
#define DEFAULT_name "@default"
#define DBA_NAME "sys_dba"
#define DBA_PASSWORD "oracle"
#include <cstring>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

/*
这里可能会用到两层以上的继承关系
--- 定义用户类，设置用户权限 ---
*/

enum authority
{
	Update, Select, Delete, Insert,Alter, Drop, Create_Table,Create_View,Create_User
};

void print_a(authority a);


class grant {
public:
	pair<authority, string> val;
	grant(authority a, string name=DEFAULT_name):val(a,name)
	{
		
	}
	bool is_global()
	{
		if (val.first == Create_View || val.first == Create_Table || val.first == Create_User)
		{
			return true;
		}
		return false;
	}

	bool operator==(grant& g)
	{
		if (val.first == g.val.first&&val.second == g.val.second)
		{
			return true;
		}
		return false;
	}
};

class user {
public:
	user():grant_set()
	{

	}
	user(const user& obj)
	{
		user_name = obj.get_user_name();
		password = obj.get_password();
		grant_set = obj.get_grant();
		role_state = obj.get_role_state();
	}
	virtual bool contains_grant(grant g) = 0;
	virtual void add_grant(grant g) = 0;
	virtual bool delete_grant(grant g) = 0;
	virtual bool is_DBA() = 0;
	virtual bool is_NORMAL_USER() = 0;
	vector<grant> get_grant()const
	{
		return grant_set;
	}
	string get_user_name()const
	{
		return user_name;
	}
	string get_password()const
	{
		return password;
	}
	int get_role_state()const
	{
		return role_state;
	}

	void show() const;

	friend bool store_user(vector<user*> temp);//存入user信息

protected:
	vector<grant> grant_set;
	int role_state;
	string user_name;
	string password;
};

class DBA :public user
{
public:
	DBA(string name = DBA_NAME, string pw = DBA_PASSWORD)
	{
		user_name = name;
		password = pw;
		role_state = DBA_ROLE;
	}
	bool contains_grant(grant g);
	//DBA已经具有所有权限
	void add_grant(grant g);
	//DBA不能被删除权限
	bool delete_grant(grant g);
	bool is_DBA();
	bool is_NORMAL_USER();

};


//权限自定义
//当
class normal_user :public user
{
public:
	normal_user(string name, string p)
	{
		user_name = name;
		password = p;
		role_state = NORMAL_USER;
	}
	bool contains_grant(grant a);
	void add_grant(grant g);
	bool delete_grant(grant g);
	bool is_DBA();
	bool is_NORMAL_USER();

};

vector<user*> read_user(bool& flag);//读入user信息



#endif