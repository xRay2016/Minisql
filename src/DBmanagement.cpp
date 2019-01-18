#include "DBmanagement.h"

void front_service::show_info()
{
	cout << "---------------------------------------------------------------------------" << endl;
	cout << "-----------------------Micro Database Management---------------------------" << endl;
	cout << "------------Copyright @2018 SCUT 2016 CS1. All rights reserved.------------" << endl;
	cout << "------------------------------2018.11--------------------------------------" << endl;
	cout << endl;
}

void front_service::start_()
{

	bool flag = false;
	for (int i = 0; i < 10; i++)
	{
		string username, password;
		cout << "username: ";
		getline(cin, username);
		cout << "password: ";
		//cin >> password;

		getline(cin, password);
		cout << endl;
		//string username = "sys_dba";
		//string password = "oracle";
		if (db_instance->load(username, password))
		{
			flag = true;
			cout << "Welcome back," << username << "!" << endl;
			log_management::read_log(log_v);
			db_instance->is_recover = 1;
			for (int i = 0; i < log_v.size(); i++)
			{
				myParser = new Parser(log_v[i]);
			}
			db_instance->is_recover = 0;
			break;
		}
		else
		{
			cout << ">>>The username doesn't exit or the password is wrong." << endl << endl;
		}
	}
	if (!flag)
	{
		cout << ">>>You have tested too many times." << endl;
		exit(1);
	}
}

string front_service::read_sql()
{
	string temp1;
	vector<string> str_v(5);
	string finalsql = " ";

	//char temp;
	cout << "sql>> \n";
	/*for (int i = 0; i < 5; i++)
	{
		getline(cin, str_v[i]);
		cout << str_v[i]<<endl;
	}*/
	bool flag = false;
	//getline(cin, temp1);
	while (getline(cin, temp1))
	{
		if (temp1 == "")
			continue;
		int len = temp1.size();
		while (temp1.size() != 0 && temp1[len - 1] == ' ')
		{
			temp1.pop_back();
		}
		finalsql += " ";
		finalsql += temp1;
		flag = true;
		if (temp1[len - 1] == ';')
			break;
	}
	//cout << finalsql << endl;
	if (temp1 == "")
	{
		db_instance->delete_Instance();
		exit(0);
	}
	return finalsql;
	//讲sql语句提交给分析器处理
}



front_service::front_service(int no)
{
	id = no;
	db_instance = Database::getInstance();
}

void front_service::process()
{
	show_info();
	int state = 0;
	while (true)
	{
		if (state == 0)
		{
			start_();
			state = 1;
		}
		else if (state == -1)
		{
			db_instance->delete_Instance();
			exit(0);
		}
		string sql_ = read_sql();
		try {
			db_instance->temp_ab.redefine("", "", 0, USER, Create_u, false);
			myParser = new Parser(sql_);
			state = myParser->state;
		}
		catch (InputError error)
		{
			cout << endl;
			cout << ">>> Error: Input format error!\n";
		}
		catch (VarAmbiguity error)
		{
			cout << ">>> Erroe: Var Ambiguity!\n";
		}
		catch (VarNotDefine error)
		{
			cout << ">>> Error: Var NotDefine!\n";
		}
		catch (GrantError error)
		{
			cout << ">>> Error: Grant Error!\n";
		}
		catch (table_exist error)
		{
			cout << ">>> Error: Table Exist Error!\n";
		}
		catch (table_not_exist error)
		{
			cout << ">>> Error: Table Not Exist Error!\n";
		}
		catch (attribute_not_exist error)
		{
			cout << ">>> Error: Attribute Not Exist Error!\n";
		}
		catch (index_exist error)
		{
			cout << ">>> Error: Index Exist!\n";
		}
		catch (index_not_exist error)
		{
			cout << ">>> Error: Index Not Exist!\n";
		}
		catch (tuple_type_conflict error)
		{
			cout << ">>> Error: Tuple Type Conflict!\n";
		}
		catch (primary_key_conflict error)
		{
			cout << ">>> Primary Key Conflict!\n";
		}
		catch (data_type_conflict error)
		{
			cout << ">>> Data Type Conflict!\n";
		}
		catch (index_full error)
		{
			cout << ">>> Index Full!\n";
		}
		catch (input_format_error error)
		{
			cout << ">>> Input Format Error!\n";
		}
		catch (exit_command error)
		{
			cout << ">>> Exit Command Error!\n";
		}
		catch (unique_conflict error)
		{
			cout << ">>> Unique Conflict!\n";
		}
		if (db_instance->temp_ab.current_un != "")
		{
			log_management::write_ab(db_instance->temp_ab);
		}
		//if(rand()%10==0)
			//management_();
		cout << endl;
	}
}



void front_service::write_log(vector<pair<string, bool>>& log_str)
{
	ofstream file("log.dat", ios::out || ios::binary || ios::trunc);
	if (!file)
	{
		cout << "文件读写错误." << endl;
		return;
	}
	int len = log_str.size();
	file.write((char*)&len, sizeof(int));
	for (int i = 0; i < len; i++)
	{
		string str = log_str[i].first;
		int slen = str.length();
		bool flag = log_str[i].second;
		file.write((char*)&slen, sizeof(int));
		file.write(str.c_str(), sizeof(char)*(slen + 1));
		file.write((char*)&flag, sizeof(bool));
	}
	file.close();
}

void front_service::read_log(vector<pair<string, bool>>& log_str)
{
	ifstream infile("log.dat", ios::in | ios::binary);
	int len;
	if (!infile)
	{
		cout << "文件读写错误." << endl;
		return;
	}
	log_str.clear();
	infile.read((char*)&len, sizeof(int));
	for (int i = 0; i < len; i++)
	{
		int slen;
		infile.read((char*)& slen, sizeof(int));
		char* temp = new char[slen + 1];
		infile.read(&temp[0], sizeof(char)*(slen + 1));
		temp[len] = '\0';
		string str = string(temp);
		bool flag;
		infile.read((char*)& flag, sizeof(bool));
		pair<string, bool> var(str, flag);
		log_str.push_back(var);
	}

	infile.close();
}

void front_service::management_()
{
	vector<ab> ab_v;
	vector<ab> new_recording;
	log_management::initial_();
	log_management::read_ab(ab_v);
	if (ab_v.size() >= 20)
	{
		int len = ab_v.size();
		for (int i = 1; i <= 20; i++)
		{
			new_recording.push_back(ab_v[len - i]);
		}
		log_management::write_ab(new_recording);
	}
}
