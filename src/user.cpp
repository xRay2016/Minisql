#include "user.h"

void user::show() const
{
	cout << "username:" << user_name <<"\t";
	cout << "password:" << password<<"\t";
	if (role_state == DBA_ROLE)
		cout << "DBA";
	else
		cout << "normal_user";
	cout << endl;
	for (int i=0;i<grant_set.size();i++)
	{
		print_a(grant_set[i].val.first);
		if (grant_set[i].val.second != DEFAULT_name)
		{
			cout << "\t"<<grant_set[i].val.second;
		}
		cout << endl;
	}
}

bool store_user(vector<user*> temp)
{
	ofstream infile;
	infile.open("./src/users.dat", ios::out | ios::trunc | ios::binary);
	if (!infile)
	{
		cerr << "文件打开错误" << endl;
		return false;
	}
	int size = temp.size();
	infile.write((char*)&size, sizeof(int));
	for (int i = 0; i < size; i++)
	{
		string user_name = temp[i]->get_user_name();
		string password = temp[i]->get_password();
		int role_state = temp[i]->get_role_state();
		vector<grant> a = temp[i]->get_grant();

		int len = user_name.length();
		infile.write((char*)&len, sizeof(int));
		infile.write((char*)user_name.c_str(), sizeof(char)*(len + 1));

		len = password.length();
		infile.write((char*)&len, sizeof(int));
		infile.write((char*)password.c_str(), sizeof(char)*(len + 1));

		infile.write((char*)&role_state, sizeof(int));

		int size = a.size();
		infile.write((char*)&size, sizeof(int));

		for (int i = 0; i < size; i++)
		{
			string tablename = a[i].val.second;
			authority Au = a[i].val.first;
			len = tablename.length();
			infile.write((char*)&len, sizeof(int));
			infile.write((char*)tablename.c_str(), sizeof(char)*(len + 1));
			infile.write((char*)&Au, sizeof(authority));
		}

	}
}


vector<user*> read_user(bool& flag)
{
	fstream infile;
	vector<user*> ans;
	infile.open("./src/users.dat", ios::in | ios::binary);
	if (!infile)
	{
		flag = false;
		return ans;
	}
	flag = true;
	int size;
	infile.read((char*)&size, sizeof(int));
	int len;
	//Array<user*> ans;
	for (int i = 0; i < size; i++)
	{
		string user_name;
		string password;
		int rolestate;

		infile.read((char*)&len, sizeof(int));
		char* temp = new char[len + 1];
		infile.read(&temp[0], sizeof(char)*(len + 1));
		temp[len] = '\0';
		user_name = string(temp);

		infile.read((char*)&len, sizeof(int));
		temp = new char[len + 1];
		infile.read(&temp[0], sizeof(char)*(len + 1));
		temp[len] = '\0';
		password = string(temp);

		infile.read((char*)&rolestate, sizeof(int));
		user* U;
		switch (rolestate)
		{
		case DBA_ROLE:
			U = new DBA(user_name, password);
			ans.push_back(U);
			break;
		case NORMAL_USER:
			U = new normal_user(user_name, password);
			ans.push_back(U);
			break;
		default:
			flag = false;
			return ans;
		}
		int Asize = 0;
		infile.read((char*)& Asize, sizeof(int));
		for (int k = 0; k < Asize; k++)
		{
			infile.read((char*)&len, sizeof(int));
			char* tempname = new char[len + 1];
			infile.read(&tempname[0], sizeof(char)*(len + 1));
			tempname[len] = '\0';
			string tablen = string(tempname);
			authority A;
			infile.read((char*)&A, sizeof(authority));
			grant tempG(A,tablen);
			ans[i]->add_grant(tempG);
		}
	}
	return ans;
}

/*
---	以下为DBA函数实现 ---
*/

bool DBA::contains_grant(grant a)
{
	return true;
}


bool DBA::delete_grant(grant a)
{
	return false;
}

void DBA::add_grant(grant g)
{

}


bool DBA::is_DBA()
{
	return true;
}

bool DBA::is_NORMAL_USER()
{
	return false;
}

/*
---	以下为normal_user函数实现 ---
*/
bool normal_user::contains_grant(grant a)
{
	for (int i = 0; i < grant_set.size(); i++)
	{
		if (grant_set[i] == a)
		{
			return true;
		}
	}
	return false;
}



void normal_user::add_grant(grant g)
{
	if (g.is_global())
	{
		g.val.second = DEFAULT_name;
	}
	if (!contains_grant(g))
	{
		grant_set.push_back(g);
	}
}



bool normal_user::delete_grant(grant g)
{
	for (int i = 0; i < grant_set.size(); i++)
	{
		if (grant_set[i] == g)
		{
			grant_set.erase(grant_set.begin()+i);
			return true;
		}
	}
	return false;
}

bool normal_user::is_DBA()
{
	return false;
}

bool normal_user::is_NORMAL_USER()
{
	return true;
}


void print_a(authority a)
{
	switch (a)
	{
	case Update:
		cout << "Update";
		break;
	case Select:
		cout << "Select";
		break;
	case Delete:
		cout << "Delete";
		break;
	case Insert:
		cout << "Insert";
		break;
	case Alter:
		cout << "Alter";
		break;
	case Drop:
		cout << "Drop";
		break;
	case Create_Table:
		cout << "Create_Table";
		break;
	case Create_User:
		cout << "Create_User";
		break;
	case Create_View:
		cout << "Create_View";
		break;
	}
}