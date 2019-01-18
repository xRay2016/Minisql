#include "parser.h"
#include "basic.h"
Tokens::Tokens()
{
}

Tokens::Tokens(const Tokens& v) :
	tokens(v.tokens), index(v.index)
{

}

Tokens::Tokens(const vector<Token>& v) : tokens(v)
{
}

int Tokens::getIndex()
{
	return index;
}

bool Tokens::match(const string& s)
{
	if (index < (int)tokens.size() && tokens[index].str == s) {
		moveN(1);
		return true;
	}
	return false;
}

bool Tokens::_match(const string& s)
{
	if (index < (int)tokens.size() && tokens[index].str == s) {
		moveN(1);
		return true;
	}
	throw(InputError());
	return false;
}

bool Tokens::hasNth(int i)
{
	return index + i < (int)tokens.size();
}

bool Tokens::moveN(int n)
{
	index += n;
	return index < (int)tokens.size();
}

string Tokens::nthWord(int i)
{
	return (index + i < (int)tokens.size()) ? tokens[index + i].str : string("");
}

type_def Tokens::nthType(int i)
{
	return tokens[index + i].type;
}

//Parser代码

int Parser::subSelectCnt = 0;

vector<SelectTree*> Parser::subSelectList = {};

Parser::Parser()
{
}

Parser::Parser(const Parser& p) : tokens(p.tokens)
{
	dbinstance = Database::getInstance();
}

Parser::Parser(const string& input)
{
	dbinstance = Database::getInstance();
	sql = input;
	tokens = Tokens(lexical_analyzer::get_result(input));
	if (tokens.hasNth(0) && tokens.nthWord(0) == "exit")
	{
		state = -1;
		return;
	}
	else if (tokens.hasNth(0) && tokens.nthWord(0) == "quit")
	{
		state = 0;
		return;
	}
	else if (tokens.hasNth(0) && tokens.nthWord(0) == "showaudit")
	{
		state = 1;
		dbinstance->show_audit();
		return;
	}
	state = 1;
	execCmd();
}

authority Parser::parserS(string s)
{
	if (s == "select")
		return Select;
	if (s == "update")
		return Update;
	if (s == "delete")
		return Delete;
	if (s == "drop")
		return Drop;
	if (s == "insert")
		return Insert;
	if (s == "create table")
		return Create_Table;
	if (s == "create user")
		return Create_User;
	if (s == "create view")
		return Create_View;
	if (s == "alter")
		return Alter;
}

set<string> Parser::havingKeyword = { "sum", "count", "max", "min", "avg" };

set<string> Parser::specificOperator = { ">", "<", ">=" , "<=", "!=" ,"=", "like" };

//总入口
void Parser::execCmd()
{
	subSelectCnt = 0;
	subSelectList.clear();
	try
	{
		if (tokens.match("select")) {
			projectMode = 0;
			SelectTree* tree = new SelectTree(execSelect());
			cout << "优化前语法树:\n";
			tree->BFSDisplay();
			for (int i = 0; i < subSelectList.size(); i++) {
				cout << "子查询" << i << "优化后语法树\n";
				subSelectList[i]->BFSDisplay();
				cout << endl;
			}
			Table res;
			int execMode = 1;//0为直接查找，1为使用执行器并优化
			if (execMode == 0) {
				if (sp.from.size() > 1)
				{
					res = dbinstance->select_table(sp.from, sp.where, sp.gattr, sp.having,
						sp.oattr, sp.select, sp.mode);
				}
				else {
					res = dbinstance->select_table(sp.from, sp.where, sp.gattr, sp.having,
						sp.oattr, sp.select, 1);
				}
			}
			else {
				if (execMode) {
					Optimizer optimizer(tree);
					SelectTree* st = optimizer.getResult();
					cout << "优化后语法树\n";
					subSelectCnt = 0;
					st->BFSDisplay();
					res = st->execute();
				}
				else {
					res = tree->execute();
				}
			}
			if (projectMode) {
				res = dbinstance->api_op->distinct(res);
			}
			dbinstance->api_op->ShowTable(res);
			cout << res.tuple_.size() << " rows in set\n";
		}
		else if (tokens.match("insert")) {
			execInsert();
		}
		else if (tokens.match("delete")) {
			execDelete();
		}
		else if (tokens.match("update")) {
			execUpdate();
		}
		else if (tokens.match("create")) {
			if (tokens.match("table")) {
				execCreateTable();
			}
			else if (tokens._match("user")) {
				execCreateUser();
			}
		}
		else if (tokens.match("drop")) {
			execDrop();
		}
		else if (tokens.match("grant")) {
			execGrantOrRevoke();
		}
		else if (tokens.match("revoke")) {
			execGrantOrRevoke(false);
		}
		else if (tokens._match("execfile")) {
			if (tokens.hasNth(1) && tokens.nthType(0) == _STRING && tokens.nthWord(1) == ";") {
				string filename = tokens.nthWord(0);
				tokens.moveN(2);
				filename = filename.substr(1, filename.size() - 2);
				dbinstance->execfile(filename);
			}
			else {
				throw(InputError());
			}
		}
	}
	catch (InputError&) {
		std::cout << "Error: Input format error!\n";
	}
	catch (type_conflict&) {
		std::cout << "Error: Type conflict!\n";
	}
	catch (data_type_conflict&) {
		std::cout << "Error: Data type conflict\n";
	}
	catch (sub_select_fail&) {
		std::cout << "Error: Subselect failed\n";
	}
}

SelectTreeNode* Parser::execColumns(int i)//i为1--select，i为2--groupBy，i为3--orderBy
{
	SelectTreeNode* root = nullptr;
	if (i == 1) {
		root = new SelectTreeNode(Op_Projection);
	}
	else if (i == 2) {
		root = new SelectTreeNode(Op_Groupby);
	}
	else {
		root = new SelectTreeNode(Op_Order);
	}
	//std::cout << "Enter execColumns()\n";
	//ok判断进入到子句中是否匹配到东西，如果没有的话最后要报错
	bool ok = false;
	while (tokens.hasNth(0) && (tokens.nthWord(0) == "*" || tokens.nthType(0) == VAR || havingKeyword.count(tokens.nthWord(0)))) {
		//*
		if (tokens.nthWord(0) == "*") {
			//在groupBy和order by子句中，不能出现*
			if (i == 2 || i == 3) {
				throw(InputError());
			}
			else {
				//std::cout << "Match: *\n";
				root->colName.push_back("*");
				tokens.moveN(1);
				ok = true;
			}
		}
		//Students.id型
		else if (tokens.hasNth(2) && tokens.nthType(0) == VAR && tokens.nthWord(1) == "."
			&& tokens.nthType(2) == VAR) {
			string tmp = tokens.nthWord(0);
			tokens.moveN(1);
			while (tokens.hasNth(1) && tokens.nthWord(0) == "." && tokens.nthType(1) == VAR) {
				tmp += "." + tokens.nthWord(1);
				tokens.moveN(2);
			}
			root->colName.push_back(tmp);
			//std::cout << "Match: " << tmp << endl;
			ok = true;
		}
		//id型
		else if (tokens.nthType(0) == VAR) {
			//std::cout << "Match: " << tokens.nthWord(0) << endl;
			root->colName.push_back(tokens.nthWord(0));
			tokens.moveN(1);
			ok = true;
		}
		//能够处理group by的
		else if ((i == 1 || i == 3) && havingKeyword.count(tokens.nthWord(0))) {
			//特殊处理count(*)
			if (tokens.hasNth(3) && tokens.nthWord(0) == "count" && tokens.nthWord(1) == "("
				&& tokens.nthWord(2) == "*") {
				//std::cout << "Match: count(*)\n";
				root->colName.push_back("count(*)");
				tokens.moveN(4);
				ok = true;
			}
			else if (tokens.hasNth(2) && tokens.nthWord(1) == "("
				&& tokens.nthType(2) == VAR) {
				string tmp = tokens.nthWord(0) + tokens.nthWord(1);
				tokens.moveN(2);
				while (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
					if (tokens.hasNth(1) && tokens.nthType(1) == VAR) {
						throw(InputError());
					}
					if (tokens.hasNth(1) && tokens.nthWord(1) == ".") {
						tmp += tokens.nthWord(0) + ".";
						tokens.moveN(2);
					}
					else if (tokens.hasNth(1) && tokens.nthWord(1) == ")") {
						tmp += tokens.nthWord(0) + ")";
						tokens.moveN(2);
						break;
					}
				}
				root->colName.push_back(tmp);
				ok = true;
			}
			else {
				throw(InputError());
			}
		}
		//不能处理的
		else {
			throw(InputError());
		}
		//处理asc或desc
		root->orderMode.push_back(0);
		if (i == 3 && tokens.hasNth(0) && (tokens.nthWord(0) == "asc" || tokens.nthWord(0) == "desc")) {
			//std::cout << "Match: " << tokens.nthWord(0) << endl;
			if (tokens.nthWord(0) == "desc") {
				root->orderMode.back() = 1;
			}
			tokens.moveN(1);
		}
		if (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
			throw(InputError());
		}
		tokens.match(",");
	}
	if (!ok) {
		throw(InputError());
	}
	//std::cout << "Leave execColumns()\n";
	return root;
}

SelectTreeNode* Parser::execFrom()
{
	//以下是老代码，不考虑可以派生表和普通表混在一起的
	/*Database* db = Database::getInstance();
	SelectTreeNode* root = new SelectTreeNode();
	//std::cout << "Enter execFrom()\n";
	if (tokens.hasNth(1) && tokens.nthWord(1) == "natural") {
		if (tokens.hasNth(3) && tokens.nthWord(2) == "join" && tokens.nthType(3) == VAR) {
			//std::cout << "Match: " << tokens.nthWord(0) << " natural join " << tokens.nthWord(3) << endl;
			root->nodeType = Op_Natural;
			root->left = new SelectTreeNode(Table_T);
			root->right = new SelectTreeNode(Table_T);
			root->left->tableName = tokens.nthWord(0);
			root->right->tableName = tokens.nthWord(3);
			sp.from.push_back(db->api_op->selectRecord1(tokens.nthWord(0)));
			sp.from.push_back(db->api_op->selectRecord1(tokens.nthWord(3)));
			sp.mode = 0;
			tokens.moveN(4);
		}
		else {
			throw(InputError());
		}
	}
	else if (tokens.hasNth(0) && tokens.nthWord(0) == "(") {
		if (tokens.hasNth(1) && tokens.nthWord(1) == "select") {
			tokens.moveN(2);
			if (!tokens.hasNth(0)) {
				throw(InputError());
			}
			//重入execSelect()
			//root->subSelect = execSelect(2);
			//直接用execSelect找到根据派生表生成的语法树的根节点，之后一定一定要把该表加上别名
			SelectTreeNode* subRoot = execSelect(2);
			SelectTree* subTree = new SelectTree(subRoot);
			//查看有没有as + 别名
			string alias;
			if (tokens.hasNth(1) && tokens.nthWord(0) == "as" && tokens.nthType(1) == VAR) {
				//std::cout << "Match: ... as " << tokens.nthWord(1) << endl;
				alias = tokens.nthWord(1);
				tokens.moveN(2);
			}
			else {
				throw(InputError());
			}
			//获得派生表的表对象，这里使用执行器去获得，执行器里面有调用DB的接口以检查权限
			Table subTable = subTree->execute();
			//给新表各属性加上别名.
			Attribute& attr = subTable.attr_;
			int len = attr.num;
			for (int i = 0; i < len; i++) {
				attr.name[i] = alias + "." + attr.name[i];
			}
			//给新表换名字
			subTable.title_ = alias;
			//修改好名字后，将该表视为普通的表
		}
		else {
			throw(InputError());
		}
	}
	else {
		if (tokens.hasNth(0) && tokens.nthType(0) == VAR)
		{
			vector<string> tmp;//存放表的名字，最后再生成节点
			while (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
				//std::cout << "Match: " << tokens.nthWord(0) << endl;
				tmp.push_back(tokens.nthWord(0));
				sp.from.push_back(db->api_op->selectRecord1(tokens.nthWord(0)));
				tokens.moveN(1);
				if (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
					throw(InputError());
				}
				tokens.match(",");
			}
			//from里面的表们不允许有相同的表
			int tmpSize = tmp.size();
			sort(tmp.begin(), tmp.end());
			auto itr = unique(tmp.begin(), tmp.end());
			if (itr - tmp.begin() != tmpSize) {
				throw(InputError());
			}
			sp.mode = 3;
			return helper(tmp, 0, tmp.size() - 1);
		}
		else {
			throw(InputError());
		}
	}
	//std::cout << "Leave execFrom()\n";
	return root;*/
	Database* db = Database::getInstance();
	SelectTreeNode* root = new SelectTreeNode();
	//std::cout << "Enter execFrom()\n";
	if (tokens.hasNth(1) && tokens.nthWord(1) == "natural") {
		if (tokens.hasNth(3) && tokens.nthWord(2) == "join" && tokens.nthType(3) == VAR) {
			//std::cout << "Match: " << tokens.nthWord(0) << " natural join " << tokens.nthWord(3) << endl;
			root->nodeType = Op_Natural;
			root->left = new SelectTreeNode(Table_T);
			root->right = new SelectTreeNode(Table_T);
			root->left->tableName = tokens.nthWord(0);
			root->right->tableName = tokens.nthWord(3);
			sp.from.push_back(db->api_op->selectRecord1(tokens.nthWord(0)));
			sp.from.push_back(db->api_op->selectRecord1(tokens.nthWord(3)));
			sp.mode = 0;
			tokens.moveN(4);
		}
		else {
			throw(InputError());
		}
	}
	else {
		//改为既能处理派生表又能处理一般的表
		vector<Table> curTable;//放派生表和一般表
		vector<string> tmp;//存放派生表表的名字，最后再生成节点
		while (tokens.hasNth(0) && (tokens.nthType(0) == VAR || tokens.nthWord(0) == "(")) {
			if (tokens.hasNth(0) && tokens.nthWord(0) == "(") {
				if (tokens.hasNth(1) && tokens.nthWord(1) == "select") {
					tokens.moveN(2);
					if (!tokens.hasNth(0)) {
						throw(InputError());
					}
					//重入execSelect()
					//root->subSelect = execSelect(2);
					//直接用execSelect找到根据派生表生成的语法树的根节点，之后一定一定要把该表加上别名
					SelectTreeNode* subRoot = execSelect(2);
					SelectTree* subTree = new SelectTree(subRoot);
					//查看有没有as + 别名
					string alias;
					if (tokens.hasNth(1) && tokens.nthWord(0) == "as" && tokens.nthType(1) == VAR) {
						//std::cout << "Match: ... as " << tokens.nthWord(1) << endl;
						alias = tokens.nthWord(1);
						tokens.moveN(2);
					}
					else {
						throw(InputError());
					}
					//获得派生表的表对象，这里使用执行器去获得，执行器里面有调用DB的接口以检查权限
					Table subTable = subTree->execute();
					//给新表各属性加上别名.
					Attribute& attr = subTable.attr_;
					int len = attr.num;
					for (int i = 0; i < len; i++) {
						attr.name[i] = alias + "." + attr.name[i];
					}
					//给新表换名字
					subTable.title_ = alias;
					//修改好名字后，将该表视为普通的表
					curTable.push_back(subTable);
					//在sp中加入该表
					sp.from.push_back(subTable);
				}
				else {
					throw(InputError());
				}
			}
			else if (tokens.hasNth(0) && tokens.nthType(0) == VAR)
			{
				/*while (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
					//std::cout << "Match: " << tokens.nthWord(0) << endl;
				}
				return helper(tmp, 0, tmp.size() - 1);*/
				tmp.push_back(tokens.nthWord(0));
				sp.from.push_back(db->api_op->selectRecord1(tokens.nthWord(0)));
				tokens.moveN(1);
				if (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
					throw(InputError());
				}
				tokens.match(",");
			}
			else
			{
				throw(InputError());
			}

		}
		//from里面的表们不允许有相同的表
		int tmpSize = tmp.size();
		sort(tmp.begin(), tmp.end());
		auto itr = unique(tmp.begin(), tmp.end());
		if (itr - tmp.begin() != tmpSize) {
			throw(InputError());
		}
		//此时派生表对象已经在curTable中，普通表名放在tmp中，将普通表放到curTable中
		for (auto i : tmp) {
			Table t = db->api_op->selectRecord1(i);
			//考虑权限问题
			Table t2 = db->select_table({ t }, nullptr, {}, nullptr, {}, { "*" }, 0);
			curTable.push_back(t);
		}
		//sp的模式为3，即笛卡儿积
		sp.mode = 3;
		//返回笛卡儿积节点或单表节点
		return helper(curTable, 0, curTable.size() - 1);
	}
	//std::cout << "Leave execFrom()\n";
	return root;
}

boolTree* Parser::execCondition(bool having, bool check)
{
	vector<boolTreeNode*> conditions;
	//std::cout << "Enter execCondition()\n";
	//cnt用来标记是否为第一次匹配到字段，因为第一个字段前面不能有not 和 and
	int cnt = 0;
	while (tokens.hasNth(0)) {
		conditions.push_back(new boolTreeNode(3));
		boolTreeNode* cur = conditions[conditions.size() - 1];
		if (cnt++) {
			if (tokens.nthWord(0) == "or") {
				//std::cout << "Match: or\n";
				conditions.back() = new boolTreeNode(2);
				conditions.push_back(new boolTreeNode(3));
				cur = conditions.back();
				tokens.moveN(1);
				//conditions.push_back(new boolTreeNode(2));
			}
			else if (tokens.nthWord(0) == "and") {
				//std::cout << "Match: and\n";
				conditions.back() = new boolTreeNode(1);
				conditions.push_back(new boolTreeNode(3));
				cur = conditions.back();
				tokens.moveN(1);
				//conditions.push_back(new boolTreeNode(1));
			}
			else {//最多只能匹配到这里
				/*//std::cout << "Leave execCondition()\n";
				return;*/
				//goto到函数尾
				conditions.pop_back();
				goto L;
			}
		}
		if (tokens.hasNth(0) && tokens.nthWord(0) == "not") {
			//std::cout << "Match: not\n";
			tokens.moveN(1);
			cur->isNot = true;
		}
		//匹配聚集函数或字段，优先匹配聚集函数
		bool haveFun = false;//有无匹配到聚集函数
		bool haveField = false;//有无匹配到字段
		if (having && tokens.hasNth(0) && havingKeyword.count(tokens.nthWord(0))) {
			haveFun = true;
			//count(*)特殊判断
			if (tokens.hasNth(3) && tokens.nthWord(0) == "count" && tokens.nthWord(1) == "("
				&& tokens.nthWord(2) == "*" && tokens.nthWord(3) == ")") {
				//std::cout << "Match: count(*)\n";
				cur->vals.push_back("count(*)");
				tokens.moveN(4);
			}
			//其他聚集函数表达式，都是类似count(sc.col)
			//不能只考虑sc.grade的情况
			if (tokens.hasNth(2) && tokens.nthWord(1) == "(" && tokens.nthType(2) == VAR) {
				//std::cout << "Match: " << tokens.nthWord(0) << "(" << tokens.nthWord(2) << ")\n";
				string tmp = tokens.nthWord(0) + tokens.nthWord(1);
				tokens.moveN(2);
				while (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
					if (tokens.hasNth(1) && tokens.nthType(1) == VAR) {
						throw(InputError());
					}
					if (tokens.hasNth(1) && tokens.nthWord(1) == ".") {
						tmp += tokens.nthWord(0) + ".";
						tokens.moveN(2);
					}
					else if (tokens.hasNth(1) && tokens.nthWord(1) == ")") {
						tmp += tokens.nthWord(0) + ")";
						tokens.moveN(2);
						break;
					}
				}
				cur->field = tmp;
			}
			else {
				throw(InputError());
			}
		}
		//如果已经匹配了聚集函数表达式，那么不需要再匹配字段
		//匹配字段
		if (!haveFun && tokens.hasNth(0) && tokens.nthType(0) == VAR) {
			haveField = true;
			//Student.id型
			if (tokens.hasNth(2) && tokens.nthWord(1) == "."
				&& tokens.nthType(2) == VAR) {
				string tmp = tokens.nthWord(0);
				tokens.moveN(1);
				while (tokens.hasNth(1) && tokens.nthWord(0) == "." && tokens.nthType(1) == VAR) {
					tmp += "." + tokens.nthWord(1);
					tokens.moveN(2);
				}
				//std::cout << "Match: " << tmp << endl;
				cur->field = tmp;
			}
			//id型
			else {
				//std::cout << "Match: " << tokens.nthWord(0) << endl;
				cur->field = tokens.nthWord(0);
				tokens.moveN(1);
			}
		}
		//并没有出现字段
		else if (!haveFun && !haveField) {
			throw(InputError());
		}
		//开始匹配各种运算符
		//如果有字段后还有not
		if (tokens.hasNth(0) && tokens.nthWord(0) == "not") {
			//std::cout << "Match: not\n";
			//cur->isNot = 1;
			cur->isNot = cur->isNot ? 0 : 1;
			tokens.moveN(1);
		}
		//匹配between
		if (tokens.hasNth(0) && tokens.nthWord(0) == "between") {
			if (tokens.hasNth(3) && tokens.nthWord(2) == "and")
			{
				type_def t1 = tokens.nthType(1), t2 = tokens.nthType(3);
				if (!(t1 == t2 && (t1 == _INT || t1 == _DOUBLE || t1 == _STRING))) {
					throw(InputError());
				}
				//std::cout << "Match: between " << tokens.nthWord(1) << " and " << tokens.nthWord(3) << endl;
				cur->op = 9;
				cur->type = t1;
				cur->vals.push_back(tokens.nthWord(1));
				cur->vals.push_back(tokens.nthWord(3));
				tokens.moveN(4);
			}
			else {
				throw(InputError());
			}
		}
		//匹配in
		else if (tokens.hasNth(0) && tokens.nthWord(0) == "in") {
			//匹配in子查询
			if (tokens.hasNth(2) && tokens.nthWord(1) == "(" && tokens.nthWord(2) == "select") {
				//std::cout << "Match: in子查询\n";
				tokens.moveN(3);
				int begin = tokens.getIndex() - 1;
				cur->subSelect = execSelect(2);
				int end = tokens.getIndex() - 1;
				string tmp;
				for (int i = begin; i <= end; i++) {
					tmp += tokens.nthWord(i);
				}
				cur->mode = 6;
				cur->op = 8;
				cur->vals.push_back(tmp);
			}
			//类似course in (eng, chs)
			else if (tokens.hasNth(1) && tokens.nthWord(1) == "(") {
				tokens.moveN(2);
				//匹配 " eng, chs) "
				type_def firstTokenType;
				//检查括号里面的单词的种类，如果有不一样的要报错
				if (!tokens.hasNth(0)) {
					throw(InputError());
				}
				//std::cout << "Match: in(\n";
				firstTokenType = tokens.nthType(0);
				cur->op = 8;
				cur->type = firstTokenType;
				while (tokens.hasNth(0)) {
					if (tokens.nthType(0) != firstTokenType) {
						throw(InputError());
					}
					if (tokens.nthType(1)) {
						if (tokens.nthWord(1) == ")") {
							//std::cout << "Match: " << tokens.nthWord(0) << ")\n";
							cur->vals.push_back(tokens.nthWord(0));
							tokens.moveN(2);
							break;
						}
						else if (tokens.nthWord(1) == ",") {
							//std::cout << "Match: " << tokens.nthWord(0) << endl;
							cur->vals.push_back(tokens.nthWord(0));
							tokens.moveN(2);
						}
						else {
							throw(InputError());
						}
					}
				}
			}
			//没有左括号，报错
			else {
				throw(InputError());
			}
		}
		//匹配>,<.>=.<=.!=,=，like
		else if (tokens.hasNth(0) && specificOperator.count(tokens.nthWord(0))) {
			if (tokens.nthWord(0) == "<") {
				cur->op = 1;
			}
			else if (tokens.nthWord(0) == "<=") {
				cur->op = 2;
			}
			else if (tokens.nthWord(0) == ">") {
				cur->op = 3;
			}
			else if (tokens.nthWord(0) == ">=") {
				cur->op = 4;
			}
			else if (tokens.nthWord(0) == "!=") {
				cur->op = 5;
			}
			else if (tokens.nthWord(0) == "=") {
				cur->op = 6;
			}
			else if (tokens.nthWord(0) == "like") {
				cur->op = 7;
			}
			//等于号单独匹配需要匹配等值连接的情况
			//应该是匹配连接的情况，不只是等值连接
			if (tokens.nthWord(0) != "like" && tokens.hasNth(3) && tokens.nthType(1) == VAR && tokens.nthType(3) == VAR
				&& tokens.nthWord(2) == ".") {
				//匹配到了连接
				cur->op = 6;
				string tmp = tokens.nthWord(1);
				tokens.moveN(2);
				while (tokens.hasNth(1) && tokens.nthWord(0) == "." && tokens.nthType(1) == VAR) {
					tmp += "." + tokens.nthWord(1);
					tokens.moveN(2);
				}
				//std::cout << "Match: = " << tmp << endl;
				cur->vals.push_back(tmp);
				cur->rightOperandField = true;
				continue;
			}
			if (tokens.hasNth(1)) {
				//匹配some，any或者直接子查询的情况
				if (tokens.nthWord(1) == "all" || tokens.nthWord(1) == "any") {
					if (cur->op == 5 || cur->op == 6 || cur->op == 8) {
						throw(sub_select_fail());
					}
					if (tokens.hasNth(2) && tokens.nthWord(2) == "(") {
						if (!(tokens.hasNth(3) && tokens.nthWord(3) == "select")) {
							throw(InputError());
						}
						cur->mode = tokens.nthWord(1) == "all" ? 4 : 5;
						//std::cout << "Match: " << tokens.nthWord(0) << " " << tokens.nthWord(1) << " 子查询 \n";
						tokens.moveN(4);
						int begin = tokens.getIndex() - 1;
						cur->subSelect = execSelect(2);
						int end = tokens.getIndex() - 1;
						string tmp;
						for (int i = begin; i <= end; i++) {
							tmp += tokens.nthWord(i);
						}
						cur->vals.push_back(tmp);
					}
					else {
						throw(InputError());
					}
				}
				else if (tokens.nthWord(1) == "(") {
					if (cur->op == 5 || cur->op == 6 || cur->op == 8) {
						throw(sub_select_fail());
					}
					if (!(tokens.hasNth(2) && tokens.nthWord(2) == "select")) {
						throw(InputError());
					}
					//std::cout << "Match: " << tokens.nthWord(0) << " 子查询 \n";
					cur->mode = 6;
					tokens.moveN(3);
					int begin = tokens.getIndex() - 1;
					cur->subSelect = execSelect(2);
					int end = tokens.getIndex() - 1;
					string tmp;
					for (int i = begin; i <= end; i++) {
						tmp += tokens.nthWord(i);
					}
					cur->vals.push_back(tmp);
				}
				else {
					//std::cout << "Match: " << tokens.nthWord(0) << " " << tokens.nthWord(1) << endl;
					cur->vals.push_back(tokens.nthWord(1));
					cur->type = tokens.nthType(1);
					tokens.moveN(2);
				}
			}
			else {
				throw(InputError());
			}
		}
		//出现了字段，但并没有运算符可以匹配
		else {
			throw(InputError());
		}
	}
L:	boolTree* tree = new boolTree(having ? 2 : 1, conditions);
	//打印tree结果
	//tree->showTree();
	//std::cout << "Leave execCondition()\n";
	return tree;
}

void Parser::execConstraint(int& state, vector<string>& colname, vector<int>& type, int& primary, vector<int>unique_v)
{
	//std::cout << "Enter execConstraint()\n";
//匹配列级约束
	if (tokens.hasNth(2) && tokens.nthType(0) == VAR && (tokens.nthWord(1) == "int" || tokens.nthWord(1) == "string"
		|| tokens.nthWord(1) == "double") && (tokens.nthWord(2) == "primary key" || tokens.nthWord(2) == "unique")) {
		//std::cout << "Match: 列级约束\n";
		//printf_s("Match: %s Type: %s Constraint: %s\n", tokens.nthWord(0).c_str(), tokens.nthWord(1).c_str(),
				//tokens.nthWord(2).c_str());
		colname.push_back(tokens.nthWord(0));
		if (tokens.nthWord(1) == "int")
		{
			type.push_back(-1);
		}
		else if (tokens.nthWord(1) == "double")
		{
			type.push_back(0);
		}
		else if (tokens.nthWord(1) == "string")
		{
			type.push_back(254);
		}
		if (tokens.nthWord(2) == "primary key")
		{
			primary = colname.size() - 1;
		}
		else if (tokens.nthWord(2) == "unique")
		{
			unique_v.push_back(colname.size() - 1);
		}
		tokens.moveN(3);
		state = 2;
	}
	//std::cout << "Leave execConstraint()\n";
}

void Parser::execCol(int& state, vector<string>& colname, vector<int>& type)
{
	//std::cout << "Enter: execCol()\n";
	//匹配建表命令中的列名+数据类型
	if (tokens.hasNth(1) && (tokens.nthWord(1) == "int" || tokens.nthWord(1) == "double"
		|| tokens.nthWord(1) == "string") && tokens.nthType(0) == VAR) {
		//printf_s("Match: Column: %s  Type: %s\n", tokens.nthWord(0).c_str(), tokens.nthWord(1).c_str());
		colname.push_back(tokens.nthWord(0));
		if (tokens.nthWord(1) == "int")
		{
			type.push_back(-1);
		}
		else if (tokens.nthWord(1) == "double")
		{
			type.push_back(0);
		}
		else if (tokens.nthWord(1) == "string")
		{
			type.push_back(254);
		}
		tokens.moveN(2);
		state = 1;
	}
	//std::cout << "Leave execCol()\n";
}

//增查删改

//type为	1：普通的select调用		2：子查询select调用		3：集合中或create view中的select调用
SelectTreeNode* Parser::execSelect(int type)
{
	//先更新sp变量
	sp = SelectParameter();
	//std::cout << "Enter execSelect():\n";
	//投影参数，all或distinct
	if (tokens.match("distinct")) {
		Parser::projectMode = 1;
	}
	//处理字段，必须有的，并先生成语法树的根节点
	SelectTreeNode* root = nullptr;
	SelectTreeNode* ptr = nullptr;
	SelectTreeNode* select = execColumns(1);
	sp.select = select->colName;
	SelectTreeNode* from = nullptr;
	SelectTreeNode* where = nullptr;
	SelectTreeNode* groupBy = nullptr;
	SelectTreeNode* having = nullptr;
	SelectTreeNode* orderBy = nullptr;
	SelectTreeNode* setOp = nullptr;
	//处理from，必须有的
	if (tokens._match("from")) {
		from = execFrom();
		//sp.from = from->colName;
	}
	//处理条件
	if (tokens.match("where")) {
		where = new SelectTreeNode(Op_Where);
		where->booltree = execCondition();
		sp.where = where->booltree;
	}
	//处理group by
	bool _groupBy = false;//判断有无成功匹配groupBy子句，避免出现只匹配了having子句的情况
	if (tokens.match("group")) {
		if (tokens._match("by")) {
			groupBy = execColumns(2);
			_groupBy = true;
			sp.gattr = groupBy->colName;
			//groupBy出现的一定要在select中出现，反过来也是
			//只是针对字段才可以，对于聚集函数和*不需要
			vector<string> groupByVector;
			vector<string> selectVector;
			for (auto i : groupBy->colName) {
				if (i == "*" || i.size() > 5 && i.substr(0, 5) == "count") {
					continue;
				}
				if (i.size() > 3 && (i.substr(0, 3) == "avg" || i.substr(0, 3) == "max" || i.substr(0, 3) == "max"
					|| i.substr(0, 3) == "min" || i.substr(0, 3) == "sum")) {
					continue;
				}
				groupByVector.push_back(i);
			}
			for (auto i : select->colName) {
				if (i == "*" || i.size() > 5 && i.substr(0, 5) == "count") {
					continue;
				}
				if (i.size() > 3 && (i.substr(0, 3) == "avg" || i.substr(0, 3) == "max" || i.substr(0, 3) == "max"
					|| i.substr(0, 3) == "min" || i.substr(0, 3) == "sum")) {
					continue;
				}
				selectVector.push_back(i);
			}
			sort(groupByVector.begin(), groupByVector.end());
			sort(selectVector.begin(), selectVector.end());
			if (selectVector != groupByVector) {
				throw(InputError());
			}
		}
	}
	//处理having
	if (tokens.match("having")) {
		if (!groupBy) {
			throw(InputError());
		}
		having = new SelectTreeNode(Op_Having);
		having->booltree = execCondition(true);
		sp.having = having->booltree;
	}
	//处理order by
	if (tokens.match("order")) {
		if (tokens._match("by")) {
			//std::cout << "Match: order by\n";
			orderBy = execColumns(3);
			sp.oattr.resize(orderBy->colName.size());
			for (int i = 0; i < orderBy->colName.size(); i++) {
				sp.oattr[i].first = orderBy->colName[i];
				sp.oattr[i].second = orderBy->orderMode[i] == 0 ? ASC : DESC;
			}
		}
	}
	//先生成不包括集合运算节点的语法树
	vector<SelectTreeNode*> v{ orderBy, select, having, groupBy,
		where, from };
	for (int i = 0; i < (int)v.size(); i++) {
		if (!v[i]) {
			continue;
		}
		else if (v[i] && !root) {
			root = v[i];
		}
		else {
			ptr->left = v[i];
		}
		ptr = v[i];
	}
	//处理集合运算
	if (tokens.hasNth(0) && ((tokens.nthWord(0) == "intersect")
		|| tokens.nthWord(0) == "union" || tokens.nthWord(0) == "except")) {
		if (tokens.hasNth(1) && tokens.nthWord(1) == "select") {
			//std::cout << "Match: " << tokens.nthWord(0) << endl;
			setOp = new SelectTreeNode();
			if (tokens.nthWord(0) == "intersect") {
				setOp->nodeType = Op_Intersect;
			}
			else if (tokens.nthWord(0) == "union") {
				setOp->nodeType = Op_Union;
			}
			else {
				setOp->nodeType = Op_Except;
			}
			tokens.moveN(2);
			SelectTreeNode* another = execSelect(3);
			setOp->right = another;
			setOp->left = root;
			root = setOp;
		}
		else {
			throw(InputError());
		}
	}
	//处理结尾分号
	switch (type) {
	case 1: {
		if (!tokens._match(";")) {
			throw(InputError());
		}
		else {
			//std::cout << "Match: ;" << endl;
		}
		break;
	}
	case 2: {
		if (!tokens._match(")")) {
			throw(InputError());
		}
		else {
			//std::cout << "Match: )" << endl;
		}
		break;
	}
	default: {
		break;
	}
	}
	//std::cout << "Leave execSelect()\n";
	return root;
}

void Parser::execInsert()
{
	string tname;
	vector<string> values;
	vector<int> type;
	//std::cout << "Enter execInsert()\n";
	bool subSelect = false;
	if (tokens.hasNth(2) && tokens.nthWord(0) == "into" && tokens.nthType(1) == VAR
		&& (tokens.nthWord(2) == "values" || tokens.nthWord(2) == "(")) {
		//std::cout << "Match: into \n";
		tname = tokens.nthWord(1);
		subSelect = (tokens.nthWord(2) == "(") ? true : false;
		tokens.moveN(3);
	}
	else {
		throw(InputError());
	}
	if (subSelect) {
		tokens._match("select");
		execSelect(2);
	}
	else {
		tokens._match("(");
		while (tokens.hasNth(2) && (tokens.nthType(0) == _INT || tokens.nthType(0) == _STRING || tokens.nthType(0) == _DOUBLE)) {
			//std::cout << "Match: " << tokens.nthWord(0) << endl;
			switch (tokens.nthType(0))
			{
			case _INT:
				values.push_back(tokens.nthWord(0));
				type.push_back(-1);
				break;
			case _STRING:
				values.push_back(tokens.nthWord(0));
				type.push_back(1);
				break;
			case _DOUBLE:
				values.push_back(tokens.nthWord(0));
				type.push_back(0);
				break;
			}
			tokens.moveN(1);
			if (tokens.nthWord(0) == ")") {
				break;
			}
			else {
				tokens._match(",");
			}
		}
		tokens._match(")");
	}
	tokens._match(";");
	dbinstance->insert_table(tname, values, type, sql);
	//std::cout << "Leave execInsert()\n";
}

void Parser::execDelete()
{
	string tname;
	boolTree* condi = NULL;
	int type;
	//std::cout << "Enter execDelete()\n";
	if (tokens.hasNth(2) && tokens.nthWord(0) == "from" && tokens.nthWord(2) == "where"
		|| tokens.nthType(1) == VAR) {
		//std::cout << "Match: " << tokens.nthWord(1) << endl;
		tname = tokens.nthWord(1);
		tokens.moveN(3);
		condi = execCondition();
	}
	else {
		throw(InputError());
	}
	//execCondition();
	/*if (tokens.hasNth(0) && tokens.nthType(0) == VAR)
	{
		attr = tokens.nthWord(0);
		tokens.moveN(1);
	}
	else
	{
		throw InputError();
	}
	if (tokens.hasNth(1) && tokens.nthType(0) == OPERATOR && (tokens.nthType(1) == _INT || tokens.nthType(1)
		== _DOUBLE || tokens.nthType(1) == _STRING))
	{
		values = tokens.nthWord(1);
		switch (tokens.nthType(1))
		{
		case _INT:
			type = -1;
			break;
		case _DOUBLE:
			type = 0;
			break;
		case _STRING:
			type = 254;
			break;
		}
		if (tokens.nthWord(0) == "<")
		{
			rela = LESS;
		}
		else if (tokens.nthWord(0) == "<=")
		{
			rela = LESS_OR_EQUAL;
		}
		else if (tokens.nthWord(0) == ">")
		{
			rela = GREATER;
		}
		else if (tokens.nthWord(0) == ">=")
		{
			rela = GREATER_OR_EQUAL;
		}
		else if (tokens.nthWord(0) == "=")
		{
			rela = EQUAL;
		}
		else if (tokens.nthWord(0) == "!=")
		{
			rela = NOT_EQUAL;
		}
	}
	else
	{
		throw InputError();
	}*/
	tokens._match(";");
	dbinstance->delete_table(tname, condi, sql);
	std::cout << "Leave execDelete()\n";
}

void Parser::execUpdate()
{
	string tname;
	string attr;
	string values;
	int type;
	boolTree* bt = new boolTree();
	//std::cout << "Enter execUpdate()\n";
	if (tokens.hasNth(4) && tokens.nthType(0) == VAR && tokens.nthWord(1) == "set" &&
		tokens.nthType(2) == VAR && tokens.nthWord(3) == "="
		&& (tokens.nthType(4) == _INT || tokens.nthType(4) == _DOUBLE || tokens.nthType(4)
			== _STRING)) {
		/*printf_s("Match: set %s %s %s\n", tokens.nthWord(2).c_str(), tokens.nthWord(3).c_str(),
			tokens.nthWord(4).c_str());*/
		tname = tokens.nthWord(0);
		attr = tokens.nthWord(2);
		values = tokens.nthWord(4);
		switch (tokens.nthType(4))
		{
		case _INT:
			type = -1;
			break;
		case _DOUBLE:
			type = 0;
			break;
		case _STRING:
			type = 254;
			break;
		default:
			break;
		}
		tokens.moveN(5);
	}
	else if (tokens.hasNth(2) && tokens.nthType(0) == VAR && tokens.nthWord(1) == "set")
	{
		tname = tokens.nthWord(0);
		tokens.moveN(2);
		if (tokens.hasNth(4) && tokens.nthType(0) == VAR && tokens.nthWord(1) == "."&&
			tokens.nthType(2) == VAR && tokens.nthWord(3) == "=" &&
			(tokens.nthType(4) == _INT || tokens.nthType(4) == _DOUBLE || tokens.nthType(4)
				== _STRING))
		{
			attr = tokens.nthWord(0) + "." + tokens.nthWord(2);
			values = tokens.nthWord(4);
			switch (tokens.nthType(4))
			{
			case _INT:
				type = -1;
				break;
			case _DOUBLE:
				type = 0;
				break;
			case _STRING:
				type = 254;
				break;
			default:
				break;
			}
			tokens.moveN(5);
		}
		else
		{
			throw InputError();
		}
	}
	else
	{
		throw(InputError());
	}
	tokens._match("where");
	//赋值boolTree
	bt = execCondition();
	tokens._match(";");
	dbinstance->update_table(tname, attr, values, type, *bt, sql);
	//std::cout << "Leave execUpdate()\n";
}

//建立与删除

void Parser::execCreateTable()
{
	string tname;
	vector<string> colname;
	vector<int> varType;
	vector<int> unique_v;
	int primary = -1;
	std::cout << "Enter execCreateTable()\n";
	if (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
		//std::cout << "Match: Table " << tokens.nthWord(0) << endl;
		tname = tokens.nthWord(0);
		tokens.moveN(1);
	}
	else {
		throw(InputError());
	}
	tokens._match("(");
	//先匹配 列名 数据类型 [列级完整性约束条件]
	//ok为真表示可以开始解析表级约束条件，列--1，列约束--2，表约束--3
	//实质上是一个有穷自动机模型，保证了表约束放在后面
	bool ok = false;
	int state = 0;
	while (tokens.hasNth(0)) {
		state = 0;
		execConstraint(state, colname, varType, primary, unique_v);
		if (state == 0) {
			execCol(state, colname, varType);
		}
		if (!ok) {
			if (state == 3) {
				ok = true;
			}
		}
		else {
			if (state == 1 || state == 2) {
				throw(InputError());
			}
		}
		if (tokens.hasNth(0) && tokens.nthWord(0) == ")") {
			break;
		}
		else if (tokens.hasNth(0) && tokens.nthWord(0) == ",") {
			tokens.moveN(1);
		}
		else {
			throw(InputError());
		}
	}
	tokens._match(")");
	tokens._match(";");
	dbinstance->create_Table(tname, colname, varType, primary, unique_v, sql);
	//std::cout << "Leave execCreateTable()\n";
}

void Parser::execDrop()
{
	string tname;
	//std::cout << "Enter execDrop()\n";
	if (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
		//std::cout << "Match: " << tokens.nthWord(0) << endl;
		tname = tokens.nthWord(0);
		tokens.moveN(1);
	}
	else {
		throw(InputError());
	}
	tokens._match(";");
	dbinstance->drop_table(tname, sql);
	std::cout << "Leave execDrop()\n";
}

//权限控制

void Parser::execCreateUser()
{
	string username, password;
	int rolestate;
	//std::cout << "Enter execCreateUser()\n";
	if (tokens.hasNth(3) && tokens.nthType(0) == VAR && tokens.nthType(3) == _STRING &&
		tokens.nthWord(1) == "identified" && tokens.nthWord(2) == "by") {
		username = tokens.nthWord(0);
		password = tokens.nthWord(3);
		//std::cout << "Match: " << tokens.nthWord(0) << " password " << tokens.nthWord(3) << endl;
		tokens.moveN(4);
	}
	else {
		throw(InputError());
	}
	bool with = tokens.match("with");
	if (with) {
		if (tokens.hasNth(0) && (tokens.nthWord(0) == "dba" || tokens.nthWord(0) == "resource"))
		{
			if (tokens.nthWord(0) == "dba")
				rolestate = DBA_ROLE;
			else if (tokens.nthWord(0) == "resource")
				rolestate = NORMAL_USER;
			//std::cout << "Match: " << tokens.nthWord(0) << endl;
			tokens.moveN(1);
		}
		else {
			throw(InputError());
		}
	}
	tokens._match(";");
	dbinstance->create_user(rolestate, username, password, sql);
	//std::cout << "Leave execCreateUser()\n";
}

void Parser::execGrantOrRevoke(bool g)
{
	string uname;
	vector<string> tname_v;
	bool all = false;
	vector<authority> aut_v;
	vector<grant> grant_v;
	//std::cout << (grant ? "Enter execGrant()\n" : "Enter execRevoke()\n");
	//先匹配权限
	bool needOn = false;
	while (tokens.hasNth(0))
	{
		if (tokens.nthWord(0) == "all")
		{
			//std::cout << "Match: all privileges\n";
			tokens.moveN(1);
			tokens.match("privileges");
			all = true;
			break;
		}
		else if (tokens.nthWord(0) == "select" || tokens.nthWord(0) == "update"
			|| tokens.nthWord(0) == "insert" || tokens.nthWord(0) == "delete" || tokens.nthWord(0) == "drop")
		{
			//std::cout << "Match: " << tokens.nthWord(0) << endl;
			authority temp = parserS(tokens.nthWord(0));
			aut_v.push_back(temp);
			tokens.moveN(1);
			needOn = true;
			if (tokens.nthWord(0) == "select" || tokens.nthWord(0) == "update"
				|| tokens.nthWord(0) == "insert" || tokens.nthWord(0) == "delete" || tokens.nthWord(0) == "drop"
				|| tokens.nthWord(0) == "alter" || tokens.nthWord(0) == "create")
			{
				throw(InputError());
			}
			tokens.match(",");
		}
		else if (tokens.nthWord(0) == "create" && tokens.hasNth(1))
		{
			if (tokens.nthWord(1) == "table") {
				//std::cout << "Match: create table\n";
				authority temp = parserS("create table");
				aut_v.push_back(temp);
				tokens.moveN(2);
				if (tokens.nthWord(0) == "select" || tokens.nthWord(0) == "update"
					|| tokens.nthWord(0) == "insert" || tokens.nthWord(0) == "delete" || tokens.nthWord(0) == "drop"
					|| tokens.nthWord(0) == "alter" || tokens.nthWord(0) == "create")
				{
					throw(InputError());
				}
				tokens.match(",");
			}
			else if (tokens.nthWord(1) == "user")
			{
				//std::cout << "Match: create user\n";
				authority temp = parserS("create user");
				aut_v.push_back(temp);
				tokens.moveN(2);
				if (tokens.nthWord(0) == "select" || tokens.nthWord(0) == "update"
					|| tokens.nthWord(0) == "insert" || tokens.nthWord(0) == "delete" || tokens.nthWord(0) == "drop"
					|| tokens.nthWord(0) == "alter" || tokens.nthWord(0) == "create")
				{
					throw(InputError());
				}
				tokens.match(",");
			}
			else
			{
				throw(InputError());
			}
		}
		else if (tokens.hasNth(1) && tokens.nthWord(0) == "alter" && tokens.nthWord(1) == "table") {
			//std::cout << "Match: alter table\n";
			authority temp = parserS("alter");
			aut_v.push_back(temp);
			tokens.moveN(2);
		}
		else
		{
			break;
		}
	}
	//匹配on
	if (needOn && tokens.hasNth(0) && tokens.nthWord(0) == "on") {
		//std::cout << "Match: on\n";
		tokens.moveN(1);
	}
	else if (needOn) {
		throw(InputError());
	}
	//匹配表名或视图名
	while (needOn && tokens.hasNth(0) && tokens.nthType(0) == VAR) {
		//std::cout << "Match: " << tokens.nthWord(0) << endl;
		tname_v.push_back(tokens.nthWord(0));
		tokens.moveN(1);
		if (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
			throw(InputError());
		}
		tokens.match(",");
	}
	if (g) {
		tokens._match("to");
	}
	else {
		tokens._match("from");
	}
	//匹配用户
	while (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
		//std::cout << "Match: User: " << tokens.nthWord(0) << endl;
		uname = tokens.nthWord(0);
		tokens.moveN(1);
		if (tokens.hasNth(0) && tokens.nthType(0) == VAR) {
			throw(InputError());
		}
		tokens.match(",");
	}
	tokens._match(";");
	//std::cout << (g ? "Leave execGrant()\n" : "Leave execRevoke()\n");
	if (all)
	{
		for (int i = 0; i < tname_v.size(); i++)
		{
			grant g1(Select, tname_v[i]);
			grant g2(Update, tname_v[i]);
			grant g3(Insert, tname_v[i]);
			grant g4(Delete, tname_v[i]);
			grant g5(Drop, tname_v[i]);
			grant_v.push_back(g1);
			grant_v.push_back(g2);
			grant_v.push_back(g3);
			grant_v.push_back(g4);
			grant_v.push_back(g5);
		}
		if (g)
			dbinstance->add_grant(uname, grant_v, sql);
		else
			dbinstance->revoke_grant(uname, grant_v, sql);
	}
	else
	{
		for (int i = 0; i < aut_v.size(); i++)
		{
			if (aut_v[i] == Create_Table || Create_User || Create_View)
			{
				grant temp(aut_v[i]);
				grant_v.push_back(temp);
			}
			else
			{
				for (int j = 0; j < tname_v.size(); j++)
				{
					grant temp(aut_v[i], tname_v[j]);
					grant_v.push_back(temp);
				}
			}
		}
		if (g)
			dbinstance->add_grant(uname, grant_v, sql);
		else
			dbinstance->revoke_grant(uname, grant_v, sql);
	}

}

SelectTreeNode * Parser::helper(const vector<Table>& v, int begin, int end)
{
	if (begin == end) {
		SelectTreeNode* root = new SelectTreeNode(Table_T);
		//root->tableName = v[begin];
		root->table = v[begin];
		return root;
	}
	SelectTreeNode* root = new SelectTreeNode(Op_Multiple);
	root->left = helper(v, begin, (end + begin) / 2);
	root->right = helper(v, (end + begin) / 2 + 1, end);
	return root;
}

//其他

