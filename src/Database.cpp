#include "Database.h"

int Database::is_recover = 0;
ab Database::temp_ab;

Database* Database::getInstance()
{
	if (singleton == NULL)
	{
		singleton = new Database();
		api_op = new API();
		log_management::initial_();
		return singleton;
	}
	else
		return singleton;
}

bool Database::delete_Instance()
{
	if (singleton == NULL)
		return false;
	else
	{
		delete singleton;
		ofstream write("./src/logs.txt", ios::trunc);
		write.close();
		singleton = NULL;
		return true;
	}
}

Database* Database::singleton = NULL;
API* Database::api_op = new API();


Database::Database()
{
	initial();
}

Database::Database(const Database&)
{

}

Database Database::operator=(const Database&)
{
	return *singleton;
}

vector<pair<string, string>> Database::get_table_name()
{
	vector<pair<string, string>> ans;
	return ans;
}


void Database::end()
{
	store_user(all_users);
}

int Database::find_user(string uname)
{
	int i;
	for (i = 0; i < all_users.size(); i++)
	{
		if (all_users[i]->get_user_name() == uname)
			break;
	}
	if (i == all_users.size())
		return -1;
	return i;
}

int Database::toInt(string str)
{
	const char* pStr = str.data();
	const int INTMAX = 2147483647;
	const int INTMIN = -2147483647;
	if (pStr == nullptr)
		return 0;
	while (isspace(*pStr))
		pStr++;
	bool bFlag = true;
	if (*pStr == '+')
	{
		bFlag = true; pStr++;
	}
	else if (*pStr == '-')
	{
		bFlag = false;
		pStr++;
	}
	int n = 0;
	while (pStr != nullptr)
	{
		if (isdigit(*pStr))
		{
			int c = *pStr - '0'; //�ж�n�Ƿ񳬳���Χ
			if ((bFlag == true) && ((n > INTMAX / 10) || ((n == INTMAX / 10) && (c > INTMAX % 10))))
			{
				return INTMAX;
			}
			else if ((bFlag == false) && ((n > (unsigned)INTMIN / 10) || ((n == (unsigned)INTMIN / 10) && (c > (unsigned)INTMIN % 10))))
			{
				return INTMIN;
			}
			//�ж�n�Ƿ񳬳���Χ
			n = 10 * n + c;
		}
		else
		{
			break;
		}
		pStr++;
	}
	if (bFlag == true)
		return n;
	else return -n;
}

double Database::toDouble(string operand)
{
	double val, power;
	int sign, index = 0;
	//operand = operand.trim();
	char first = operand[0];

	sign = (first == '-') ? -1 : 1;//�жϷ���
	if (first == '-' || first == '+')
		index = 1;//����ַ����ĵ�һ���ַ�Ϊ����,��� index=1����ʼѰ������
	for (val = 0.0; index < operand.length() && isdigit(operand[index]); index++)
		val = val * 10.0 + (operand[index] - '0');
	if (index < operand.length() && operand[index] == '.')
		index++;//����С����, ����С����Ѱ������
	for (power = 1.0; index < operand.length() && isdigit(operand[index]); index++)
	{
		val = val * 10.0 + (operand[index] - '0');
		power *= 10.0;//�൱�ڼ�¼С��������λ��
	}
	return sign * val / power;
}

string Database::toString(string str)
{
	int len = str.length();
	if (len > 0 && str[len - 1] == '"')
	{
		if (len - 1 > 0 && str[0] == '"')
		{
			str.erase(len - 1, 1);
			len--;
			str.erase(0, 1);
		}
	}
	return str;
}

bool Database::initial()
{
	bool flag;
	all_users = read_user(flag);
	return flag;
}

bool Database::load(string username, string password)
{
	bool ans = false;
	for (int i = 0; i < all_users.size(); i++)
	{
		if (all_users[i]->get_user_name() == username)
		{
			if (all_users[i]->get_password() == password)
			{
				current_users = all_users[i];
				grant_v = current_users->get_grant();
				//��ȡ����û���ص���Ϣ
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

void Database::initial_users()
{
	all_users.clear();
	user* temp = new DBA();
	all_users.push_back(temp);
	store_user(all_users);
}

Database::~Database()
{
	delete api_op;
}

void Database::create_user(int role_state, string username, string password, string sql)
{
	bool flag = false;
	//���Ȩ�ޣ�û�еĻ�������Ȩ�޵Ĵ���

	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, USER, Create_u, false);

	grant g(Create_User);
	if (!current_users->contains_grant(g))
	{
		throw GrantError();
	}
	if (current_users->is_NORMAL_USER() && role_state == DBA_ROLE)
	{
		throw GrantError();
	}
	//����Ƿ�����
	for (int i = 0; i < all_users.size(); i++)
	{
		if (all_users[i]->get_user_name() == username)
		{
			throw(VarAmbiguity());
		}
	}

	if (password.length() > 0 && password[0] == '"')
		password.erase(0, 1);
	if (password.length() > 1 && password[password.length() - 1] == '"')
		password.erase(password.length() - 1, 1);


	//ִ��ǰ�Ȱ��������д����־�ļ���

	user* tempuser;
	switch (role_state)
	{
	case DBA_ROLE:
		tempuser = new DBA(username, password);
		all_users.push_back(tempuser);
		break;
	case NORMAL_USER:
		tempuser = new normal_user(username, password);
		all_users.push_back(tempuser);
	}
	store_user(all_users);

	temp_ab.b = true;
	//log_management::write_ab(*temp_ab);
	cout << endl;
	if (is_recover == 0)
		cout << ">>>Create user successfully." << endl;
}

////�Դ�Ϊ��,�쳣��������������
//void Database::drop_user(string uname)
//{
//	//�ȼ������dropȨ��,��Ȩ���׳�GrantError
//	//�ټ��ɾ���û��Ƿ���DBA��DBA���ܱ�ɾ�������쳣�׳�GrantError
//	//�ټ������û����Ƿ����,�������׳�VarNotDefine
//	//�Ѿ���д����־�ļ�
//	//��user��all_users��ɾ��,��all_usersд��
//	//���Drop user successfully."
//}

void Database::add_grant(string uname, vector<grant> G, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, USER, Grant_u, false);

	int j = find_user(uname);
	vector<string> all_tables = api_op->getAllTable();
	if (j == -1)
	{
		throw VarNotDefine();
	}
	for (int i = 0; i < G.size(); i++)
	{//�������д
		if (!current_users->contains_grant(G[i]))
		{
			throw GrantError();
		}
		//�����ȫ��Ȩ�ޣ���ô������ͨ�û���˵�����Ȩ���ǲ��ɴ��ݵ�
		if (G[i].is_global() && current_users->is_NORMAL_USER())
		{
			throw GrantError();
		}
		if (!G[i].is_global())
		{
			bool flag = false;
			for (int i = 0; i < all_tables.size(); i++)
			{
				if (all_tables[i] == G[i].val.second)
				{
					flag = true;
					break;
				}
			}
			if (!flag)
			{
				throw table_not_exist();
			}
		}

		//д����־�ļ�
		//vector<user*>::iterator iter = all_users.begin();

		all_users[j]->add_grant(G[i]);
	}
	store_user(all_users);
	temp_ab.b = true;
	if (is_recover == 0)
		cout << ">>> Grant authority successfully." << endl;
}

void Database::create_Table(string tname, vector<string>& colname, vector<int>& varType, int primary, vector<int>& unique_v, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, TABLE, Create_t, false);
	//�������Ȩ��
	bool flag = false;
	//���Ȩ�ޣ�û�еĻ�������Ȩ�޵Ĵ���
	grant g(Create_Table);
	if (!current_users->contains_grant(g))
	{
		throw GrantError();
	}
	//�����ɵײ���
	Attribute attribute;
	attribute.num = colname.size();
	if (colname.size() > 32)
	{
		throw Too_many_col();
	}
	for (int i = 0; i < colname.size(); i++)
	{
		attribute.name[i] = colname[i];
		attribute.type[i] = varType[i];
	}
	for (int j = 0; j < unique_v.size(); j++)
	{
		attribute.unique[unique_v[j]] = true;
	}
	attribute.primary_key = primary;
	Index index;
	if (primary != -1)
	{
		attribute.has_index[primary] = true;
		index.indexname[0] = "primary_key index";
		index.num = 1;
		index.location[0] = primary;
	}
	else
	{
		index.num = 0;
	}
	//�Ѿ���д����־�ļ�
	//write_log(sql);
	//��������ȿ���(���Ű���ص�Ȩ�����ȥ)
	int i = 0;

	api_op->createTable(tname, attribute, primary, index);
	grant g1(Select, tname);
	grant g2(Update, tname);
	grant g3(Delete, tname);
	grant g4(Insert, tname);
	grant g5(Drop, tname);
	current_users->add_grant(g1);
	current_users->add_grant(g2);
	current_users->add_grant(g3);
	current_users->add_grant(g4);
	current_users->add_grant(g5);
	store_user(all_users);
	temp_ab.b = true;
	if (is_recover == 0)
		cout << ">>> Create Table successfully.\n";
}

void Database::revoke_grant(string uname, vector<grant> G, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, USER, Revoke_u, false);
	int j = find_user(uname);
	vector<string> all_tables = api_op->getAllTable();
	if (j == -1)
	{
		throw VarNotDefine();
	}
	bool flag = true;
	for (int i = 0; i < G.size(); i++)
	{
		//�����Լ�ӵ�е�Ȩ�޲���revoke
		if (!current_users->contains_grant(G[i]))
		{
			throw GrantError();
		}
		//�����ȫ��Ȩ�ޣ���ô������ͨ�û���˵�����Ȩ���ǳ��ص�
		if (G[i].is_global() && current_users->is_NORMAL_USER())
		{
			throw GrantError();
		}

		bool flag1 = false;
		if (!G[i].is_global())
		{
			bool flag = false;
			for (int i = 0; i < all_tables.size(); i++)
			{
				if (all_tables[i] == G[i].val.second)
				{
					flag1 = true;
					break;
				}
			}
			if (!flag1)
			{
				throw table_not_exist();
			}
		}
		if (!all_users[j]->delete_grant(G[i]))
			flag = false;
	}
	//д��־
	store_user(all_users);
	temp_ab.b = true;
	if (is_recover == 0)
		cout << ">>> Revoke  authority successfully." << endl;

	//�����û���
}

//��Щʵ�ֵĺ����ӿڻ�û��д�ã��Ȱ�Ȩ�޼��д��
void Database::update_table(string tname, string attr, string values, int type, boolTree where, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, TABLE, Update_t, false);
	grant g(Update, tname);
	if (!current_users->contains_grant(g))
	{
		throw GrantError();
	}
	Data attr_value;
	switch (type)
	{
	case -1:
		attr_value.type = -1;
		attr_value.datai = toInt(values);
		break;
	case 0:
		attr_value.type = 0;
		attr_value.dataf = toDouble(values);
		break;
	default:
		attr_value.type = 254;
		attr_value.datas = toString(values);
		break;
	}
	//��¼��־
	api_op->update(tname, attr, attr_value, where);
	temp_ab.b = true;

	if (is_recover == 0)
	{
		log_management::write_log(sql);
		cout << ">>> Update record successfully\n.";
	}
}

void Database::delete_table(string tname, boolTree* condi, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, TABLE, Delete_t, false);
	grant g(Delete, tname);
	if (!current_users->contains_grant(g))
	{
		throw GrantError();
	}
	//д��־
	api_op->deleteRecord(tname, *condi);
	temp_ab.b = true;
	if (is_recover == 0)
	{
		log_management::write_log(sql);
		cout << ">>> Delete successfully.\n";
	}
}

Table Database::select_table(vector<Table> from, boolTree* where, std::vector<string> gattr, boolTree* having, vector<pair<string, UD>> oattr, vector<string>select, int mode, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, TABLE, Select_t, false);
	bool where_is_null = false;
	bool having_is_null = false;
	boolTree* final_where = NULL;
	boolTree* final_having = NULL;
	if (where == NULL)
	{
		where_is_null = true;
		final_where = new boolTree();
	}
	if (having == NULL)
	{
		having_is_null = true;
		final_having = new boolTree();
	}

	vector<string> check_t;
	vector<string> all_table = api_op->getAllTable();
	for (int i = 0; i < from.size(); i++)
	{
		bool flag = false;
		for (int j = 0; j < all_table.size(); j++)
		{
			if (all_table[j] == from[i].getTitle() && all_table[j] != "tmp_table")
			{
				flag = true;
				break;
			}
		}
		if (flag)
		{
			check_t.push_back(from[i].getTitle());
		}
	}
	for (int i = 0; i < check_t.size(); i++)
	{
		grant g(Select, check_t[i]);
		if (!current_users->contains_grant(g))
		{
			throw GrantError();
		}
	}
	if (where_is_null&&having_is_null)
	{
		Table temp = api_op->selectRecord(from, *final_where, gattr, *final_having, oattr, select, mode);
		return temp;
	}
	else if (where_is_null && !having_is_null)
	{
		Table temp = api_op->selectRecord(from, *final_where, gattr, *having, oattr, select, mode);
		return temp;
	}
	else if (!where_is_null && having_is_null)
	{
		Table temp = api_op->selectRecord(from, *where, gattr, *final_having, oattr, select, mode);
		return temp;
	}
	else if (!where_is_null && !having_is_null)
	{
		Table temp = api_op->selectRecord(from, *where, gattr, *having, oattr, select, mode);
		return temp;
	}

	if (sql != "")
	{
		temp_ab.b = true;
		//д��־
	}

	//cout << ">>> Select successfully."<<endl;
	//api_op->ShowTable(*temp);
//
}



void Database::drop_table(string tname, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, TABLE, Drop_t, false);
	grant g(Drop, tname);
	if (!current_users->contains_grant(g))
	{
		throw GrantError();
	}
	//д��־�ļ�
	api_op->dropTable(tname);
	grant g1(Select, tname);
	grant g2(Update, tname);
	grant g3(Delete, tname);
	grant g4(Insert, tname);
	grant g5(Drop, tname);
	for (int i = 0; i < all_users.size(); i++)
	{
		all_users[i]->delete_grant(g1);
		all_users[i]->delete_grant(g2);
		all_users[i]->delete_grant(g3);
		all_users[i]->delete_grant(g4);
		all_users[i]->delete_grant(g5);
	}
	store_user(all_users);
	temp_ab.b = true;
	cout << ">>> Drop table successfully." << endl;
	//��¼����

}

void Database::insert_table(string tname, vector<string> tuple, vector<int> type, string sql)
{
	string un11 = current_users->get_user_name();
	int rs = current_users->get_role_state();
	temp_ab.redefine(un11, sql, rs, TABLE, Insert_t, false);
	grant g(Insert, tname);
	Tuple tp;
	vector<Data> data_v(tuple.size());
	if (!current_users->contains_grant(g))
	{
		throw GrantError();
	}
	for (int i = 0; i < tuple.size(); i++)
	{
		switch (type[i])
		{
		case -1:
			data_v[i].type = -1;
			data_v[i].datai = toInt(tuple[i]);
			break;
		case 0:
			data_v[i].type = 0;
			data_v[i].dataf = toDouble(tuple[i]);
			break;
		default:
			string data_str = toString(tuple[i]);
			if (data_str.length() <= 254)
			{
				data_v[i].type = 254;
				data_v[i].datas = data_str;
			}
			else
			{
				throw String_Illegal();
			}
			break;
		}
	}
	for (int i = 0; i < data_v.size(); i++)
	{
		tp.addData(data_v[i]);
	}
	//д��־�ļ�
	api_op->insertRecord(tname, tp);
	temp_ab.b = true;
	if (is_recover == 0)
	{
		log_management::write_log(sql);
		cout << ">>> Insert successfully.\n";
	}
}

Attribute Database::getAttr(string table)
{
	return api_op->getAttr(table);
}

Table Database::setOperation(Table table1, Table table2, int operation)
{
	return api_op->setOperation(table1, table2, operation);
}

Table Database::distinct(Table sourceTable)
{
	return api_op->distinct(sourceTable);
}

int Database::TupleNum(string table_name)
{
	return api_op->TupleNum(table_name);
}

vector<string> Database::getAllTable()
{
	return api_op->getAllTable();
}

void Database::execfile(string filen)
{
	Parser* myparser;
	string str;
	ifstream infile(filen, ios::in);
	if (!infile)
	{
		throw file_not_exist();
	}
	string sql = "";
	while (getline(infile, str))
	{
		if (str.empty())
		{
			continue;
		}
		str.erase(0, str.find_first_not_of(" "));
		str.erase(str.find_last_not_of(" ") + 1);
		if (str.empty())
		{
			continue;
		}
		sql = sql + " ";
		sql += str;
		if (str[str.size() - 1] == ';')
		{
			cout << sql << endl;
			myparser = new Parser(sql);
			sql.clear();
		}
	}
	cout << "Exec file successfully.\n";
}

void Database::show_audit()
{
	if (current_users->is_NORMAL_USER())
	{
		throw GrantError();
	}
	vector<ab> ab_v;
	log_management::read_ab(ab_v);
	show_ab(ab_v);
	cout << "There are " << ab_v.size() << " recordings." << endl;
}
