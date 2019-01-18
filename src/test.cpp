#include "DBmanagement.h"
#include "parser.h"
#include "data_structure.h"
#include <Windows.h>
using namespace std;
BufferManager buffer_manager;


void test_sql()
{
	fstream file;
	file.open("test_sql.txt");
	string str = "";
	char c;
	str = "";
	while (file.get(c))
	{
		str = "";
		while (file.get(c))
		{
			str = str + c;
			if (c == ';')
			{
				break;
			}
		}
		cout << str << endl;
		vector<str_pair> ans = lexical_analyzer::get_result(str);
		for (int i = 0; i <ans.size(); i++)
		{
			cout << "  [" << ans[i].str << " " << ans[i].type << "] " << endl;
		}
		cout << endl;
	}
	file.close();
}

bool match_test(string str)
{
	regex reg("^[\u4e00-\u9fa5_a-zA-Z0-9]+$");
	smatch s1;
	return regex_match(str, s1, reg);
}

void test_frontend()
{
	front_service front_test(1);
	front_test.process();
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) {
	if (CTRL_CLOSE_EVENT == dwCtrlType || CTRL_SHUTDOWN_EVENT == dwCtrlType) {
		// 控制台将要被关闭，这里添加你的处理代码 ...
		Database::delete_Instance();
	}
	return false;
}


int main()
{
	/*string str = "SELECT s.ID\nfrom students\nwhere score>=60;";
	vector<string> ans = string_dealer::divide_string(str);
	for (int i = 0; i < ans.size(); i++)
	{
		cout << "  [" << ans[i] << "]  ";
	}
	cout << endl;*/

	/*string str = "update set grade=60 where id is not null; ";
	string test = "select   iD";
	cout << string_dealer::get_string(str) << endl;
	vector<str_pair> ans = lexical_analyzer::get_result(str);
	for (int i = 0; i < ans.size(); i++)
	{
		cout << "  [" << ans[i].str << " " << ans[i].type << "] " << endl;
	}
	cout << endl;*/

	/*cout << 1 << endl;
	system("pause");
	exit(1);
	cout << 1 << endl;
	system("pause");*/

	/*Database* db = Database::getInstance();
	db->initial_users();*/

	SetConsoleCtrlHandler(HandlerRoutine, true);
	test_frontend();

	/*SelectTreeNode* root = new SelectTreeNode();
	SelectTreeNode* table1 = new SelectTreeNode(Table_T, "students");
	SelectTreeNode* table2 = new SelectTreeNode(Table_T, "sc");
	SelectTreeNode* multiple = new SelectTreeNode(Op_Multiple, " ");
	multiple->left = table1;
	multiple->right = table2;
	SelectTreeNode* select = new SelectTreeNode(Op_Select, "sc.cno,students.sno");
	select->left = multiple;
	Tree_show(select);
*/

	//test_sql();


	/*string str = "男";
	cout << match_test(str);*/

	/*string word = "AAAAAAAAAA1AAAAAAAAAA21A";
	cout << match_var(word) << endl;*/

	/*Trie* root = new Trie();
	root->insert("select");
	root->insert("update");

	cout << root->search("select") << endl;
	cout << root->search("drop") << endl;
	*/

	/*vector<user*> user_v(2);
	user_v[0] = new DBA();
	user_v[1] = new normal_user("xdl", "12345");
	grant g(Create_Table, "students");
	grant g1(Insert, "students");
	user_v[1]->add_grant(g);
	user_v[1]->add_grant(g1);
	user_v[0]->add_grant(g);
	cout << user_v[0]->delete_grant(g);
	user_v[0]->show();
	cout << endl;
	user_v[1]->show();
	cout << endl;
	
	store_user(user_v);

	bool flag;
	vector<user*> ans = read_user(flag);
	for (int i = 0; i < ans.size(); i++)
	{
		ans[i]->show();
		cout << endl;
	}*/


	system("pause");
}