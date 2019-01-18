#include"audit.h"

ab::ab(string un, string s, int rs, op_object ob, op_type ot, bool r)
{
	current_un = un;
	str = s;
	rolestate = rs;
	object = ob;
	type = ot;
	b = r;
}

void ab::redefine(string un, string s, int rs, op_object ob, op_type ot, bool r)
{
	current_un = un;
	str = s;
	rolestate = rs;
	object = ob;
	type = ot;
	b = r;
}

void tr(vector<ab>v)
{
	for (int i = 0; i < v.size(); i++) {
		cout << v[i].current_un << endl << v[i].str << endl << v[i].rolestate << endl << v[i].object << endl << v[i].type << endl;
	}
}




void log_management::write_log(string log)
{
	ofstream outfile("./src/logs.txt", ios::out || ios::app);
	if (!outfile)
	{
		cout << "file not exist." << endl;
		return;
	}
	outfile << log;
	outfile << "\n";
	outfile.close();
}

void log_management::initial_()
{
	ofstream outfile("./src/audit.dat", ios::out || ios::binary||ios::trunc);
	int i = 0;
	outfile.write((char*)&i, sizeof(int));
	outfile.close();
}



log_management::log_management()
{
	//read_ab(sqls);
	//read_log(logs);
}

void log_management::write_log(vector<string> logs)
{
	ofstream outfile("./src/logs.txt", ios::out || ios::trunc);
	for (int i = 0; i < logs.size(); i++)
	{
		outfile << logs[i];
		outfile << "\n";
	}
	outfile.close();
}

void log_management::read_log(vector<string>& logs)
{
	logs.clear();
	string temp;
	ifstream infile("./src/logs.txt", ios::in);
	while (getline(infile, temp))
	{
		if (temp == "")
			continue;
		logs.push_back(temp);
	}
	infile.close();
}
void log_management::write_ab(ab sql) {
	int len;

	fstream write("./src/audit.dat", ios::in | ios::out | ios::binary);
	//string个数+1，写入
	write.seekg(0, ios::beg);
	write.read((char*)&len, sizeof(int));
	len++;
	write.seekp(0, ios::beg);
	write.write((char*)&len, sizeof(int));
	write.seekp(0, ios::end);
	//写该string
	int slen = sql.str.length();
	write.write((char*)&slen, sizeof(int));
	write.write(sql.str.c_str(), slen + 1);
	//写username
	int ulen = sql.current_un.length();
	write.write((char*)&ulen, sizeof(int));
	write.write(sql.current_un.c_str(), ulen + 1);
	//写类型
	write.write((char*)&sql.rolestate, sizeof(int));
	write.write((char*)&sql.object, sizeof(int));
	write.write((char*)&sql.type, sizeof(int));
	//写结果
	int b = sql.b;
	write.write((char*)&b, sizeof(int));

	write.close();

}

void log_management::write_ab(vector<ab> logs)
{
	ofstream outfile("./src/audit.dat", ios::out | ios::binary | ios::in);
	int length = logs.size();
	outfile.write((char*)&length, sizeof(int));
	for (int i = 0; i < length; i++)
	{
		string un = logs[i].current_un;
		string str = logs[i].str;
		//outfile.write((char*)&(str.length()), sizeof(int));
		int rs = logs[i].rolestate;
		op_object ob = logs[i].object;
		op_type t = logs[i].type;
		int ob_i = ob;
		int t_i = t;
		int b = logs[i].b;

		//写该string
		int slen = str.length();
		outfile.write((char*)&slen, sizeof(int));
		outfile.write(str.c_str(), slen + 1);
		//写username
		int ulen = un.length();
		outfile.write((char*)&ulen, sizeof(int));
		outfile.write(un.c_str(), ulen + 1);
		//写类型
		outfile.write((char*)&rs, sizeof(int));
		outfile.write((char*)&ob, sizeof(int));
		outfile.write((char*)&t, sizeof(int));
		//写结果
		outfile.write((char*)&b, sizeof(int));
	}
	outfile.close();
}
void log_management::read_ab(vector<ab>&v_sql)
{
	ifstream read("./src/audit.dat", ios::in | ios::binary);
	int len, slen;
	read.read((char*)&len, sizeof(int));

	char ch;
	for (int i = 0; i < len; i++) {
		string un;
		string str;

		int rs;
		op_object ob;
		op_type t;
		int b;
		read.read((char*)&slen, sizeof(int));
		for (int i = 0; i < slen + 1; i++)
		{
			read.read((char*)&ch, sizeof(char));
			str += ch;
		}
		read.read((char*)&slen, sizeof(int));
		for (int i = 0; i < slen + 1; i++)
		{
			read.read((char*)&ch, sizeof(char));
			un += ch;
		}
		read.read((char*)&rs, sizeof(int));
		read.read((char*)&ob, sizeof(int));
		read.read((char*)&t, sizeof(int));
		read.read((char*)&b, sizeof(int));
		ab s(un, str, rs, ob, t, b);
		v_sql.push_back(s);
	}
	read.close();
}
void show_ab(vector<ab>v)
{
	for (int i = 0; i < v.size(); i++) {
		cout << "username\t" << v[i].current_un << endl << "sql\t" << v[i].str << endl << "rolestate";
		switch (v[i].rolestate)
		{
		case 100:cout << "\tDBA" << endl; break;
		case 50:cout << "\tNormalUser" << endl; break;
		}
		cout << "op_object";
		switch (v[i].object)
		{
		case 0:cout << "\tTABLE" << endl; break;
		case 1:cout << "\tUSER" << endl; break;
		}
		cout << "op_type\t";
		switch (v[i].type)
		{
		case 0:cout << "\tCreate_t" << endl; break;
		case 1:cout << "\tCreate_u" << endl; break;
		case 2:cout << "\tUpdate_t" << endl; break;
		case 3:cout << "\tGrant_u" << endl; break;
		case 4:cout << "\tRevoke_u" << endl; break;
		case 5:cout << "\tInsert_t" << endl; break;
		case 6:cout << "\tDelete_t" << endl; break;
		case 7:cout << "\tSelect_t" << endl; break;
		case 8:cout << "\tDrop_t" << endl; break;
		}
		cout << "op_result";
		switch (v[i].b)
		{
		case true:cout << "\ttrue" << endl; break;
		case false:cout << "\tfalse" << endl; break;
		}
		cout << endl;
	}
}

ab::ab()
{
}