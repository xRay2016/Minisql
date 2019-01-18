#include "data_structure.h"
#include <windows.h>
#include <regex>
#include "error.h"

void gotoxy(int y, int x) {
	//y�� x��
	//Y�� X��
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void myCout(string str)
{
	CONSOLE_SCREEN_BUFFER_INFO bInfo; // ���ڻ�������Ϣ
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // ��ȡ��׼����豸���
	GetConsoleScreenBufferInfo(hOut, &bInfo); // ��ȡ���ڻ�������Ϣ
	int curPos = bInfo.dwCursorPosition.X;
	int oldPos = curPos;
	while (str.size()) {
		while (curPos < bInfo.dwMaximumWindowSize.X) {
			COORD rcd = { curPos, bInfo.dwCursorPosition.Y };
			char text;
			DWORD read;
			ReadConsoleOutputCharacter(hOut, &text, 1, rcd, &read);
			if (text != ' ') {
				break;
			}
			curPos++;
		}
		if (curPos - oldPos - 1 > str.size()) {
			cout << str;
			return;
		}
		else {
			cout << str.substr(0, curPos - oldPos - 1);
			str = str.substr(curPos - oldPos - 1);
			gotoxy(bInfo.dwCursorPosition.Y + 1, oldPos + 1);
		}
	}
}

boolTreeNode::boolTreeNode(int _mode) : mode(_mode), op(0), isNot(false),
left(nullptr), right(nullptr), type(0)
{

}

boolTreeNode* getBoolTreeRoot(boolTree* p)
{
	return p->root;
}

boolTree::boolTree(int _mode, const vector<boolTreeNode*>& v)
	: mode(_mode), root(nullptr), nodeList(v) {
	buildTree(nodeList);
	for (auto i : v) {
		if (i->mode >= 3 && i->mode <= 6) {
			attributeList.push_back(i->field);
		}
		if (i->mode == 3 && i->rightOperandField) {
			attributeList.push_back(i->vals[0]);
		}
	}
}

boolTree::boolTree()
{
}

boolTree::~boolTree()
{
}

void boolTree::pushInt(int a)
{
	string curFieldName = attributeList[pushCnt];
	if (!m.count(curFieldName) || m[curFieldName].first != -1) {
		throw(InputError());
	}
	m[curFieldName].second = to_string(a);
	pushCnt++;
}

void boolTree::pushDouble(double a)
{
	string curFieldName = attributeList[pushCnt];
	if (!m.count(curFieldName) || m[curFieldName].first != 0) {
		throw(InputError());
	}
	m[curFieldName].second = to_string(a);
	pushCnt++;
}

void boolTree::pushString(const string& s)
{
	string curFieldName = attributeList[pushCnt];
	if (!m.count(curFieldName) || m[curFieldName].first <= 0) {
		throw(InputError());
	}
	m[curFieldName].second = s;
	pushCnt++;
}

void boolTree::pushArrributes(const vector<Attribute>& v)
{
	//����attributeList�еĸ��ֶΣ���v���ҵ������û���򱨴�
	for (int i = 0; i < attributeList.size(); i++) {
		//���ھۼ��������ͣ�����Ҫ��v���ҵ���������Ҫ����m�У�����Ϊ��double���͵�
		string tmp = attributeList[i];
		if (tmp.size() > 5 && tmp.substr(0, 5) == "count") {
			m[attributeList[i]] = make_pair(0, " ");
			continue;
		}
		else if (tmp.size() > 3) {
			if (tmp.substr(0, 3) == "avg" || tmp.substr(0, 3) == "max" || tmp.substr(0, 3) == "min" ||
				tmp.substr(0, 3) == "sum") {
				m[attributeList[i]] = make_pair(0, " ");
				continue;
			}
		}
		int ok = 0;
		for (int j = 0; !ok && j < v.size(); j++) {
			for (int k = 0; !ok && k < v[j].name->size(); k++) {
				if (v[j].name[k] == attributeList[i]) {
					ok = 1;
					m[attributeList[i]] = make_pair(v[j].type[k], "");
				}
			}
		}
		if (!ok) {
			throw(InputError());
		}
	}
}

bool boolTree::getResult()
{
	return getResultHelper(root);
}

string boolTree::showTree()
{
	return showTreeHelper(root);
}

vector<string> boolTree::getAttributeList()
{
	return attributeList;
}

void boolTree::clear()
{
	pushCnt = 0;
}

bool boolTree::isNull()
{
	return attributeList.empty();
}

string boolTree::showTreeHelper(boolTreeNode* p)
{
	if (!p) {
		return "";
	}
	string res;
	if (!p->left && !p->right) {
		res += " " + (p->isNot ? string("!(") : string()) + p->field;
		//���ڵ㣬����some����any
		if (p->mode == 3) {
			switch (p->op)
			{
			case 1:
				res += " <";
				break;
			case 2:
				res += " <=";
				break;
			case 3:
				res += " >";
				break;
			case 4:
				res += " >=";
				break;
			case 5:
				res += " !=";
				break;
			case 6:
				res += " =";
				break;
			case 7:
				res += " like";
				break;
			case 8:
				res += " in(";
				break;
			case 9:
				res += " between";
				break;
			}
			if (p->op == 1 || p->op == 2 || p->op == 3 ||
				p->op == 4 || p->op == 5 || p->op == 6 ||
				p->op == 7) {
				res += " " + p->vals[0];
			}
			else if (p->op == 9) {
				res += " " + p->vals[0] + " and " + p->vals[1];
			}
			else {
				res += p->vals[0];
				for (int i = 1; i < (int)p->vals.size(); i++) {
					res += ", " + p->vals[i];
				}
				res += ")";
			}
		}
		//�Ӳ�ѯ�ڵ㣬ֻ��ʾ�Ӳ�ѯ������
		else if (p->mode == 4 || p->mode == 5 || p->mode == 6) {
			switch (p->op)
			{
			case 1:
				res += " <";
				break;
			case 2:
				res += " <=";
				break;
			case 3:
				res += " >";
				break;
			case 4:
				res += " >=";
				break;
			case 5:
				res += " !=";
				break;
			case 6:
				res += " =";
				break;
			case 7:
				res += " like";
				break;
			case 8:
				res += " in";
				break;
			}
			res += " �Ӳ�ѯ " + to_string(Parser::subSelectCnt++);
		}
		res += (p->isNot ? ")" : "");
	}
	res += showTreeHelper(p->left);
	res += (p->mode == 1 ? " and" : (p->mode == 2 ? " or" : ""));
	res += showTreeHelper(p->right);
	return res;
}

void boolTree::buildTree(vector<boolTreeNode*>& v)
{
	root = buildTreeHelper(v, 0, v.size() - 1);
}

boolTreeNode* boolTree::buildTreeHelper(vector<boolTreeNode*> v, int begin, int end)
{
	/*
	�ݹ齨����˼·��
	�ݹ���ֹ�������б���ֻ�����һ���ڵ�������ڵ�ʱ����ֹ�ݹ�
	ÿ��˳��������飬�ҵ���һ�����ȼ��ϵ͵Ľڵ�(or�ڵ�)��Ϊ�ָ�ڵ㣬Ȼ���б��Ϊ��������ݹ�
	��û���ҵ������Եڶ����ڵ���Ϊ�ָ�ڵ㣬�ֿ��ݹ�
	*/
	if (begin > end) {
		return nullptr;
	}
	else if (begin == end) {
		//�����Ӳ�ѯ�ڵ����⴦��
		if (v[begin]->subSelect) {
			v[begin]->execSubselect();
		}
		return v[begin];
	}
	else if (end - begin == 2) {
		v[begin + 1]->left = v[begin];
		if (v[begin]->subSelect) {
			v[begin]->execSubselect();
		}
		if (v[end]->subSelect) {
			v[end]->execSubselect();
		}
		v[begin + 1]->right = v[end];
		return v[begin + 1];
	}
	int i = -1;
	for (i = begin + 1; i <= end; i += 2) {
		if (v[i]->mode == 2) {
			break;
		}
	}
	//�ҵ�or�ڵ�
	if (i <= end) {
		allAnd = false;
		v[i]->left = buildTreeHelper(v, begin, i - 1);
		v[i]->right = buildTreeHelper(v, i + 1, end);
		return v[i];
	}
	//����and�ڵ㣬�Ե�һ��and�ڵ㿪ʼ�ָ�
	else {
		v[begin + 1]->left = v[begin];
		v[begin + 1]->right = buildTreeHelper(v, begin + 2, end);
		return v[begin + 1];
	}
}

vector<string>& getBoolTreeAttributeList(boolTree* p)
{
	return p->attributeList;
}

bool boolTree::wildcardMatch(string m, string str)
{
	swap(m, str);
	if (str[str.length() - 1] == '"')
		str.erase(str.length() - 1, 1);
	if (str[0] == '"')
		str.erase(0, 1);
	string temp = m;
	string temp1 = m;
	int j = 0;
	string t1 = "(.?)";
	string t2 = "(.*?)";
	for (int i = 0; i < (int)m.length(); i++)
	{
		if (m[i] == '_')
		{
			temp.erase(j, 1);
			temp.insert(j, t1);
			j = j + 3;
		}
		else if (m[i] == '%')
		{
			temp.erase(j, 1);
			temp.insert(j, t2);
			j = j + 4;
		}
		j++;
	}
	regex reg(temp);
	smatch s1;
	return regex_match(str, s1, reg);
}

template<typename T1, typename T2>
bool boolTree::cmp(int op, T1 x, T2 y)
{
	switch (op)
	{
	case 1:
		return x < y;
	case 2:
		return x <= y;
	case 3:
		return x > y;
	case 4:
		return x >= y;
	}
	return false;
}

bool boolTree::getResultHelper(boolTreeNode* p)
{
	if (p == nullptr) {
		return true;
	}
	else if (p->mode == 1) {
		return getResultHelper(p->left) && getResultHelper(p->right);
	}
	else if (p->mode == 2) {
		return getResultHelper(p->left) || getResultHelper(p->right);
	}
	//������Ҷ�ӽڵ㡣�ȴ�����ͨ�������ڵ�
	//�����Ӳ�ѯ��Ҷ�ӽڵ�
	//�������Ӳ�ѯ�ڵ�Ҳ����
	else {
		//��res�������ڴ���������ȡ�ǵ�����
		bool res = false;
		int op = p->op;
		string leftName = p->field;
		int leftType = m[leftName].first;
		string rightName = p->rightOperandField ? p->vals[0] : "";
		int rightType = p->rightOperandField ? m[rightName].first : p->type;
		//���Ҳ������������ֶΣ������͸ĵ�
		if (!p->rightOperandField && !p->subSelect) {
			if (rightType == _INT) {
				rightType = -1;
			}
			else if (rightType == _DOUBLE) {
				rightType = 0;
			}
			else {
				//��������������������������������������
				rightType = 254;
			}
		}
		string leftVal = m[leftName].second;
		string rightVal = p->rightOperandField ? m[rightName].second : p->vals[0];
		if (leftType > 0 && leftVal.size() > 0 && leftVal[0] == '"') {
			leftVal = leftVal.substr(1, leftVal.size() - 2);
		}
		if (rightType > 0 && rightVal.size() > 0 && rightVal[0] == '"') {
			rightVal = rightVal.substr(1, rightVal.size() - 2);
		}
		switch (op)
		{
			//	<	<=	>	>=	�κ�һ����������������string
		case 1: case 2: case 3: case 4: {
			if (leftType > 0 || rightType > 0) {
				throw(data_type_conflict());
			}
			double left = stod(leftVal), right = stod(rightVal);
			res = cmp(op, left, right);
			break;
		}
				//like �������������붼��string
		case 7: {
			if (!(leftType > 0 && rightType > 0)) {
				throw(data_type_conflict());
			}
			res = wildcardMatch(leftVal, rightVal);
			break;
		}
				//between and :����������һ������execCondition������ֻ���鶼Ϊ������
		case 9: {
			if (leftType > 0 || rightType > 0) {
				throw(data_type_conflict());
			}
			double val1 = stod(leftVal), val2 = stod(p->vals[0]), val3 = stod(p->vals[1]);
			res = cmp(2, val2, val1) && cmp(2, val1, val3);
			break;
		}
				//= �� != ������������������
		case 5: case 6: {
			if (leftType > 0 || rightType > 0) {
				if (!(leftType > 0 && rightType > 0)) {
					throw(data_type_conflict());
				}
				res = op == 5 ? leftVal != rightVal : leftVal == rightVal;
			}
			//���������ͣ�ȫ��ת����double���бȽ�
			else {
				double _left = stod(leftVal), _right = stod(rightVal);
				res = op == 5 ? _left != _right : _left == _right;
			}
			break;
		}
				//in ���ߵĲ��������ͱ�����ͬ
		case 8: {
			if (!(leftType == -1 && rightType == -1 || leftType == 0 && rightType == 0 ||
				leftType > 0 && rightType > 0)) {
				throw(data_type_conflict());
			}
			//����������ת���ˣ�ȫ����string���ͽ��бȽ�
			bool flag = 0;
			for (auto i : p->vals) {
				//ȥ��˫����
				//i = i.substr(1, i.size() - 2);
				if (i.size() && i[0] == '\"') {
					i = i.substr(1, i.size() - 2);
				}
				if (i == leftVal) {
					res = true;
					flag = 1;
					break;
				}
			}
			res = !flag ? false : true;
		}
		}
		return p->isNot ? !res : res;
	}
	return false;
}


void boolTreeNode::execSubselect()
{
	//�ú����ǽ��Ӳ�ѯ��ֵ���ʵ�ʵ�ֵ����vals��
	//�ȼ��ͶӰ�����ͶӰ�в�ֹһ�����򱨴�
	SelectTreeNode* cur = subSelect;
	while (cur && cur->nodeType != Op_Projection) {
		cur = cur->left;
	}
	if (!cur || cur->colName.size() != 1) {
		throw(sub_select_fail());
	}
	//�޸�ͶӰ
	//�޸�all�ڵ�
	string curName = cur->colName[0];
	if (mode == 4) {
		if (op == 1 || op == 2 || op == 3 || op == 4) {
			switch (op)
			{
			case 1: case 2: {
				curName = "min(" + curName + ")";
				break;
			}
			case 3: case 4: {
				curName = "max(" + curName + ")";
			}
			}
		}
		else {
			throw(sub_select_fail());
		}
	}
	//����any�ڵ�
	else if (mode == 5) {
		if (op == 1 || op == 2 || op == 3 || op == 4) {
			switch (op)
			{
			case 1: case 2: {
				curName = "max(" + curName + ")";
				break;
			}
			case 3: case 4: {
				curName = "min(" + curName + ")";
			}
			}
		}
		else {
			throw(sub_select_fail());
		}
	}
	cur->colName[0] = curName;
	//ִ�в�ѯ
	SelectTree st(subSelect);
	Optimizer o(&st);
	SelectTree* s1 = o.getResult();
	//�����Ӳ�ѯ�б�
	Parser::subSelectList.push_back(s1);
	//Table res = st.execute();
	Table res = s1->execute();
	//���ڵ�ֵ�ģ�����in����ֻ����һ��Ԫ��
	if (mode == 6 && op != 8 && res.tuple_.size() != 1) {
		throw(sub_select_fail());
	}
	//�������ֶ�Ϊ0�����vals
	rightOperandField = 0;
	vals.clear();
	//����in�ڵ�
	if (mode == 6 && op == 8) {
		type = res.attr_.type[0];
		for (int i = 0; i < res.tuple_.size(); i++) {
			Data data = res.tuple_[i].getData()[0];
			switch (type)
			{
			case -1: {
				vals.push_back(to_string(data.datai));
				break;
			}
			case 0: {
				vals.push_back(to_string(data.dataf));
				break;
			}
			default: {
				vals.push_back(data.datas);
			}
			}
		}
	}
	//��������ֵ
	else {
		Data data = res.tuple_[0].getData()[0];
		switch (type)
		{
		case -1: {
			vals.push_back(to_string(data.datai));
			break;
		}
		case 0: {
			vals.push_back(to_string(data.dataf));
			break;
		}
		default: {
			vals.push_back(data.datas);
		}
		}
	}
	//
	subSelect = nullptr;
	switch (type)
	{
	case -1:
		type = _INT;
		break;
	case 0:
		type = _DOUBLE;
		break;
	case 1:
		type = _STRING;
	}
}

string SelectTreeNode::toString()
{
	switch (nodeType)
	{
	case Op_Where: {
		return "Where:" + booltree->showTree();
	}
	case Op_Projection: {
		string res;
		res += colName[0];
		for (int i = 1; i < (int)colName.size(); i++) {
			res += ", " + colName[i];
		}
		return "Projection: " + res;
	}
	case Table_T: {
		return "Table: " + table.getTitle();
	}
	case Op_Multiple: {
		return "Multiple";
	}
	case Op_Groupby: {
		string res = colName[0];
		for (int i = 1; i < (int)colName.size(); i++) {
			res += ", " + colName[i];
		}
		return "GroupBy:" + res;
	}
	case Op_Having: {
		return "Having: " + booltree->showTree();
	}
	case Op_Order: {
		string res = colName[0] + (orderMode[0] == 0 ? " asc" : " desc");
		for (int i = 1; i < (int)colName.size(); i++) {
			res += ", " + colName[i] + (orderMode[i] == 0 ? " asc" : " desc");
		}
		return "OrderBy: " + res;
	}
	case Op_Intersect: {
		return "Intersect";
	}
	case Op_Union: {
		return "Union";
	}
	case Op_Except: {
		return "Except";
	}
	case Op_Natural: {
		return "Natural";
	}
	case Op_EqualJoin: {
		return "Join:" + booltree->showTree();
	}
	}
	return "";
}

void SelectTree::update()
{
	parentRecord.clear();
	updateHelper(root, nullptr);
}

void SelectTree::updateHelper(SelectTreeNode* cur, SelectTreeNode* pre)
{
	if (!cur) {
		return;
	}
	parentRecord[cur] = pre;
	if (cur->left) {
		updateHelper(cur->left, cur);
	}
	if (cur->right) {
		updateHelper(cur->right, cur);
	}
}

void SelectTree::BFSDisplay()
{
	if (!root) {
		return;
	}
	queue<SelectTreeNode*> que;
	map<SelectTreeNode*, int> m1;
	map<SelectTreeNode*, SelectTreeNode*> m2;
	CONSOLE_SCREEN_BUFFER_INFO bInfo; // ���ڻ�������Ϣ
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // ��ȡ��׼����豸���
	GetConsoleScreenBufferInfo(hOut, &bInfo); // ��ȡ���ڻ�������Ϣ
	int line = bInfo.dwCursorPosition.Y;
	m1[root] = bInfo.dwMaximumWindowSize.X / 2 - 10;
	m2[root->left] = root;
	//�ȴ�ӡ���ڵ�
	gotoxy(line, m1[root]);
	cout << root->toString();
	if (root->right) {
		que.push(root->right);
		m2[root->right] = root;
	}
	que.push(root->left);
	while (!que.empty()) {
		cout << endl << endl << endl;
		line += 3;
		int n = que.size();
		while (n--) {
			SelectTreeNode* cur = que.front();
			que.pop();
			int parentPos = m1[m2[cur]];
			//���ڵ�ֻ��һ���ӽڵ�
			if (cur == m2[cur]->left && !m2[cur]->right) {
				gotoxy(line - 2, parentPos);
				cout << "|";
				gotoxy(line - 1, parentPos);
				cout << "|";
				m1[cur] = parentPos;
				gotoxy(line, m1[cur]);
				//cout << cur->toString();
				myCout(cur->toString());
			}
			//���ڵ��������ӽڵ㣬�Ҹýڵ����ҽڵ�
			else if (cur == m2[cur]->right) {
				//parent: (line - 3, parentPos)
				gotoxy(line - 2, parentPos + m2[cur]->toString().size());
				cout << "\\";
				gotoxy(line - 1, parentPos + m2[cur]->toString().size() + 1);
				cout << "\\";
				m1[cur] = parentPos + m2[cur]->toString().size() + 2;
				gotoxy(line, m1[cur]);
				//cout << cur->toString();
				myCout(cur->toString());
			}
			//���ڵ��������ӽڵ㣬�Ҹýڵ�����ڵ�
			else {
				//parent: (line - 3, parentPos)
				gotoxy(line - 2, parentPos);
				cout << "/";
				gotoxy(line - 1, parentPos - 1);
				cout << "/";
				m1[cur] = parentPos - cur->toString().size();
				gotoxy(line, m1[cur]);
				//cout << cur->toString();
				myCout(cur->toString());
			}
			if (cur->right) {
				que.push(cur->right);
				m2[cur->right] = cur;
			}
			if (cur->left) {
				que.push(cur->left);
				m2[cur->left] = cur;
			}
		}
	}
	cout << endl << endl;
}

Table SelectTree::execute()
{
	if (!root) {
		cerr << "û�в�ѯ�ƻ�����ִ��\n";
		return Table();
	}
	update();
	SelectParameter p;
	p = executeHelper(root);
	Table res;
	Database* db = Database::getInstance();
	//��ý����
	//��������
	if (p.mode >= 10) {
		if (p.from.size() > 1) {
			res = db->setOperation(p.from[0], p.from[1], p.mode - 10);
		}
		//��release�¿���������
		else {
			res = db->setOperation(p.from[0], p.from[0], p.mode - 10);
		}
		res = db->api_op->distinct(res);
	}
	else {
		res = db->select_table(p.from, p.where, p.gattr, p.having, p.oattr, p.select, p.mode);
	}
	//db->api_op->ShowTable(res);
	//cout << "��ѡ��" << res.tuple_.size() << "��\n";
	return res;
}

//ִ������������
SelectTree::SelectParameter SelectTree::executeHelper(SelectTreeNode* t)
{
	Database* db = Database::getInstance();
	SelectParameter p;//����ʱ��
	SelectParameter p1, p2;//������ʱ��
	if (t->nodeType == Op_Natural) {
		p1 = executeHelper(t->left);
		p2 = executeHelper(t->right);
		Table table1, table2;
		if (p1.from.size() > 1 || p1.where || p1.gattr.size() || p1.having || p1.select.size() || p1.oattr.size()) {
			if (p1.select.size() == 0) {
				p1.select.push_back("*");
			}
			table1 = db->select_table(p1.from, p1.where, p1.gattr, p1.having, p1.oattr, p1.select, p1.mode);
		}
		else {
			table1 = p1.from[0];
		}
		if (p2.from.size() > 1 || p2.where || p2.gattr.size() || p2.having || p2.select.size() || p2.oattr.size()) {
			if (p2.select.size() == 0) {
				p2.select.push_back("*");
			}
			table2 = db->select_table(p2.from, p2.where, p2.gattr, p2.having, p2.oattr, p2.select, p2.mode);
		}
		p.mode = 0;
		p.from.push_back(table1);
		p.from.push_back(table2);
		return p;
	}
	else if (t->nodeType == Op_EqualJoin) {
		p1 = executeHelper(t->left);
		p2 = executeHelper(t->right);
		Table table1, table2;
		if (p1.from.size() > 1 || p1.where || p1.gattr.size() || p1.having || p1.select.size() || p1.oattr.size()) {
			if (p1.select.size() == 0) {
				p1.select.push_back("*");
			}
			table1 = db->select_table(p1.from, p1.where, p1.gattr, p1.having, p1.oattr, p1.select, p1.mode);
		}
		else {
			table1 = p1.from[0];
		}
		if (p2.from.size() > 1 || p2.where || p2.gattr.size() || p2.having || p2.select.size() || p2.oattr.size()) {
			if (p2.select.size() == 0) {
				p2.select.push_back("*");
			}
			table2 = db->select_table(p2.from, p2.where, p2.gattr, p2.having, p2.oattr, p2.select, p2.mode);
		}
		else {
			table2 = p2.from[0];
		}
		p.where = t->booltree;
		p.mode = 1;
		p.from.push_back(table1);
		p.from.push_back(table2);
		return p;
	}
	else if (t->nodeType == Op_Union || t->nodeType == Op_Intersect || t->nodeType == Op_Except) {
		p1 = executeHelper(t->left);
		p2 = executeHelper(t->right);
		Table table1, table2;
		if (p1.from.size() > 1 || p1.where || p1.gattr.size() || p1.having || p1.select.size() || p1.oattr.size()) {
			table1 = db->select_table(p1.from, p1.where, p1.gattr, p1.having, p1.oattr, p1.select, p1.mode);
		}
		else {
			table1 = p1.from[0];
		}
		if (p2.from.size() > 1 || p2.where || p2.gattr.size() || p2.having || p2.select.size() || p2.oattr.size()) {
			table2 = db->select_table(p2.from, p2.where, p2.gattr, p2.having, p2.oattr, p2.select, p2.mode);
		}
		else {
			table2 = p2.from[0];
		}
		//ִ�м�������
		int op = -1;
		if (t->nodeType == Op_Intersect) {
			op = 0 + 10;
		}
		else if (t->nodeType == Op_Union) {
			op = 1 + 10;
		}
		else {
			op = 2 + 10;
		}
		//Table res = db->setOperation(table1, table2, op);
		p.from.push_back(table1);
		p.from.push_back(table2);
		p.mode = op;
		return p;
	}
	else if (t->nodeType == Op_Order) {
		p = executeHelper(t->left);
		p.oattr.resize(t->colName.size());
		for (int i = 0; i < t->colName.size(); i++) {
			p.oattr[i].first = t->colName[i];
			p.oattr[i].second = t->orderMode[i] == 0 ? ASC : DESC;
		}
		return p;
	}
	else if (t->nodeType == Op_Projection) {
		p = executeHelper(t->left);
		p.select = t->colName;
		return p;
	}
	else if (t->nodeType == Op_Having) {
		p = executeHelper(t->left);
		p.having = t->booltree;
		return p;
	}
	else if (t->nodeType == Op_Groupby) {
		p = executeHelper(t->left);
		p.gattr = t->colName;
		return p;
	}
	else if (t->nodeType == Op_Where) {
		p = executeHelper(t->left);
		p.where = t->booltree;
		return p;
	}
	else if (t->nodeType == Op_Multiple) {
		p1 = executeHelper(t->left);
		p2 = executeHelper(t->right);
		Table table1, table2;
		if (p1.from.size() > 1 || p1.where || p1.gattr.size() || p1.having || p1.select.size() || p1.oattr.size()) {
			if (p1.select.size() == 0) {
				p1.select.push_back("*");
			}
			table1 = db->select_table(p1.from, p1.where, p1.gattr, p1.having, p1.oattr, p1.select, p1.mode);
		}
		else {
			table1 = p1.from[0];
		}
		if (p2.from.size() > 1 || p2.where || p2.gattr.size() || p2.having || p2.select.size() || p2.oattr.size()) {
			if (p2.select.size() == 0) {
				p2.select.push_back("*");
			}
			table2 = db->select_table(p2.from, p2.where, p2.gattr, p2.having, p2.oattr, p2.select, p2.mode);
		}
		//�ѿ�����
		p.from.push_back(table1);
		p.from.push_back(table2);
		p.mode = 1;
		return p;
	}
	else if (t->nodeType == Table_T) {
		//Table res = db->api_op->selectRecord1(t->tableName);
		//p.from.push_back(res);
		//���ñ�
		Table res2 = db->select_table({ t->table }, nullptr, {}, nullptr, {}, { "*" }, 0);
		p.from.push_back(t->table);
		return p;
	}
}


//�Ż���ϵ��

void Optimizer::exec()
{
	if (root->nodeType == Op_Union || root->nodeType == Op_Except || root->nodeType == Op_Intersect) {
		Optimizer o1(root->left), o2(root->right);
		root->left = o1.getResult()->root;
		root->right = o2.getResult()->root;
		ok = 1;
		return;
	}
	/*��һ�������ù���4�ֽ�ѡ������
	�ڶ����������·�ͶӰ����
	���ڹ���4	��F1(��F2(E))=��(F1 and F2)(E)	��Ժ�ȡ
	���Ե�һ����ֻ��ȫΪand�Ĳ����������Ż�
	*/
	//���ҵ�where�ڵ㼰ǰ��ڵ�
	root1 = root;//root1Ϊwhere�ڵ�
	root2 = nullptr;//root2Ϊroot1�ĸ��ڵ�
	while (root1 && root1->nodeType != Op_Where) {
		root2 = root1;
		root1 = root1->left;
	}
	//����root1Ϊwhere�ڵ㣬�����һ���ڵ�����Ȼ���ӵģ���ô�㷨��ֹ
	if (root1 && root1->left->nodeType == Op_Natural) {
		return;
	}
	root3 = root1 ? root1->left : nullptr;//root3Ϊwhere�ڵ���ӽڵ�
	update();
	//�ֽ�ѡ����·�ѡ��ֻ���ѡ������Ϊ��ȡʽʱ��������where�Ӿ�
	if (root1 && root1->booltree->allAnd) {
		resolveAndPushSelect();
	}
	//SelectTree st1(root);
	//st1.BFSDisplay();
	//���������·�ͶӰ
	//���ѡ������к���*����ô�����·�ͶӰ
	update();
	SelectTreeNode* cur = root;
	while (cur && cur->nodeType != Op_Projection) {
		cur = cur->left;
	}
	vector<string> v = cur->colName;
	//ͶӰ�У������having�Ӿ䣬Ҳ�����оۼ�����
	cur = root;
	while (cur && cur->nodeType != Op_Having) {
		cur = cur->left;
	}
	if (cur) {
		vector<string> hav = cur->booltree->getAttributeList();
		for (auto i : hav) {
			v.push_back(i);
		}
	}
	if (projectOK(v)) {
		pushDownProject();
	}
	//SelectTree st1(root);
	//st1.BFSDisplay();
	//���Ĳ�����ѡ��͵ѿ������������
	update();
	makeJoin();
	//�Ż�����
	ok = 1;
}

void Optimizer::resolveAndPushSelect()
{
	boolTree* bTree = root1->booltree;
	boolTreeNode* bTreeNode = getBoolTreeRoot(bTree);
	vector<boolTreeNode*> nodeList;
	//������������������Ҷ�ӽڵ�
	queue<boolTreeNode*> que;
	que.push(bTreeNode);
	while (!que.empty()) {
		boolTreeNode* cur = que.front();
		que.pop();
		if (cur->left) {
			que.push(cur->left);
		}
		if (cur->right) {
			que.push(cur->right);
		}
		if (!cur->left && !cur->right) {
			nodeList.push_back(cur);
		}
	}
	//nodeList1��Ž�ԭ���ĸ���������ɢ��Ĳ�������ɵ�SelectTreeNode*
	//������ֱ�ӶԲ��������в������ݲ��ϳ�SelectTreeNode*
	//vector<SelectTreeNode*> nodeList1(nodeList.size());
	/*
	�������ڵ���з���
	��һ�࣬������һ�����������ģ����ұ����ֶε�������Ȼ���Ӻ�����ԣ�������������nodeList2
	�ڶ��࣬���Ҷ��������ģ����������������ŵѿ�����ǰ����������ӣ�����nodeList3
	�����࣬��ߴ������ģ����Ҳ����������ֶεģ�ֱ���·ŵ��������nodeList4
	*/
	vector<boolTreeNode*> nodeList2;
	vector<boolTreeNode*> nodeList3;
	vector<boolTreeNode*> nodeList4;
	//map	m1	���������һ�ű�������ڵ��Ӧ����Ϣ
	map<string, vector<boolTreeNode*>> m1;
	//v1	�����������ڵ����漰���ı�
	vector<string> v1;
	//�ȷ���
	for (int i = 0; i < nodeList.size(); i++) {
		/*nodeList1[i] = new SelectTreeNode(Op_Where);
		boolTree* tmp = new boolTree(1, { nodeList[i] });
		nodeList1[i]->booltree = tmp;*/
		//��ȡ�������ڵ��Ӧ�ı����������Ҳ������Ľڵ㣬���ܵĻ���
		//ͬʱ����ڵ���Թ���
		string leftFieldTableName = getTableName(nodeList[i]->field);
		bool rightFieldHaveTableName = false;
		string rightFieldTableName = "";
		if (nodeList[i]->rightOperandField) {
			string tmp = nodeList[i]->vals[0];
			string tmp1 = getTableName(tmp);
			if (tmp != tmp1) {
				rightFieldTableName = tmp1;
				rightFieldHaveTableName = true;
			}
		}
		//������
		if (leftFieldTableName.size() != nodeList[i]->field.size() && !nodeList[i]->rightOperandField) {
			nodeList4.push_back(nodeList[i]);
			m1[leftFieldTableName].push_back(nodeList[i]);
			v1.push_back(leftFieldTableName);
		}
		//�ڶ���
		else if (leftFieldTableName.size() != nodeList[i]->field.size() && rightFieldHaveTableName) {
			nodeList3.push_back(nodeList[i]);
			m[nodeList[i]] = { leftFieldTableName, rightFieldTableName };
		}
		else {
			nodeList2.push_back(nodeList[i]);
		}
	}
	/*
	Ϊ���·�ѡ������ĵڶ���ڵ㣬�������Ӵ������·ֲ��ѿ�����
	*/
	//_resΪ�����SelectTreeNode*
	/*vector<SelectTreeNode*> _res;
	getTablePtr(root3, _res);
	SelectTreeNode* res = merge(nodeList3, _res);
	//�滻ԭ���ı�
	root3 = root1->left = res;
	parentRecord[res] = root1;
	update();*/
	/*
	�·�ѡ������
	������ڵ㣬ֱ�����ҵ���Ӧ�ı�ڵ��Լ�ǰ�̣���ϳ��µ�boolTree*����ϳ�SelectTreeNode*
	*/
	sort(v1.begin(), v1.end());
	auto it = unique(v1.begin(), v1.end());
	v1.resize(it - v1.begin());
	//SelectTree st(root);
	//st.BFSDisplay();
	for (int i = 0; i < v1.size(); i++) {
		vector<boolTreeNode*> tempV = m1[v1[i]];
		vector<boolTreeNode*> andNode(tempV.size() - 1);
		for (int i = 0; i < andNode.size(); i++) {
			andNode[i] = new boolTreeNode(1);
		}
		vector<boolTreeNode*> tmp;
		for (int i = 0; i < tempV.size() + andNode.size(); i++) {
			if (i % 2 == 0) {
				tmp.push_back(tempV[i / 2]);
			}
			else {
				tmp.push_back(andNode[(i - 1) / 2]);
			}
		}
		tempV = tmp;
		boolTree* newBoolTree = new boolTree(1, tempV);
		SelectTreeNode* newSelectNode = new SelectTreeNode(Op_Where);
		newSelectNode->belongTo = v1[i];
		newSelectNode->booltree = newBoolTree;
		pair<SelectTreeNode*, SelectTreeNode*> p = findHelper(root1->left, root1, v1[i]);
		if (!p.first && !p.second) {
			throw(InputError());
		}
		SelectTreeNode* tableNode = p.first;
		//������������⣬��һ�����ڵѿ���������߼���ѡ��
		//p.second->left = newSelectNode;
		if (findHelper2(p.second->left, v1[i])) {
			p.second->left = newSelectNode;
		}
		else if (findHelper2(p.second->right, v1[i])) {
			p.second->right = newSelectNode;
		}
		newSelectNode->left = tableNode;
	}
	update();
	/*
	�·�ѡ������
	�ڶ���ڵ㣬���������������ҵ������������������ȣ�����ѡ������
	ע�⣬����������������������������ϵ���ȡ������򵥵�����O(n^2)���㷨�ҵ���ͬ�࣬���Һϲ��ɲ�����
	*/
	vector<int> mark(nodeList3.size(), 1);
	for (int i = 0; i < mark.size(); i++) {
		if (mark[i] == 0) {
			continue;
		}
		string leftTableName = m[nodeList3[i]].first;
		string rightTableName = m[nodeList3[i]].second;
		vector<boolTreeNode*> v;
		v.push_back(nodeList3[i]);
		for (int j = i + 1; j < mark.size(); j++) {
			if (mark[j] == 0) {
				continue;
			}
			string leftTableName1 = m[nodeList3[j]].first;
			string rightTableName1 = m[nodeList3[j]].second;
			if (leftTableName == leftTableName1 && rightTableName == rightTableName1) {
				//��Ҫ�������and�ڵ�
				v.push_back(new boolTreeNode(1));
				v.push_back(nodeList3[j]);
				mark[j] = 0;
			}
		}
		SelectTreeNode* parent = getLCA(root, leftTableName, rightTableName);
		//�ҵ��ѿ������ĸ��ڵ�
		SelectTreeNode* pParent = parentRecord[parent];
		SelectTreeNode* newSelect = new SelectTreeNode(Op_Where);
		newSelect->booltree = new boolTree(1, v);
		pParent->left = newSelect;
		newSelect->left = parent;
	}
	//SelectTree st1(root);
	//st1.BFSDisplay();
	update();
	/*
	������һ��ڵ㣬����ԭwhere����root1����left���ϣ�ԭwhereҪɾ��
	*/
	if (nodeList2.size()) {
		SelectTreeNode* pre = parentRecord[root1];
		SelectTreeNode* next = root1->left;
		SelectTreeNode * _1Kind = new SelectTreeNode(Op_Where);
		_1Kind->booltree = new boolTree(1, nodeList2);
		pre->left = _1Kind;
		_1Kind->left = next;
	}
	else {
		SelectTreeNode* pre = parentRecord[root1];
		SelectTreeNode* next = root1->left;
		pre->left = next;
	}
}

void Optimizer::pushDownProject()
{
	//���ҵ�ͶӰ�Ľڵ�
	SelectTreeNode* cur = root;
	while (cur && cur->nodeType != Op_Projection) {
		cur = cur->left;
	}
	pushDownProjectHelper(cur, {});
}

void Optimizer::makeJoin()
{
	//���ҵ�where�Ľڵ�
	SelectTreeNode* cur = root;
	while (cur && cur->nodeType != Op_Where) {
		cur = cur->left;
	}
	makeJoinHelper(cur);
}

bool Optimizer::projectOK(const vector<string>& v)
{
	for (auto i : v) {
		if (i == "*") {
			return false;
		}
		if (i.size() > 3 && (i.substr(0, 3) == "avg" || i.substr(0, 3) == "sum" || i.substr(0, 3) ==
			"max" || i.substr(0, 3) == "min")) {
			return false;
		}
		if (i.size() > 5 && i.substr(0, 5) == "count") {
			return false;
		}
	}
	return true;
}

void Optimizer::makeJoinHelper(SelectTreeNode* cur)
{
	if (!cur || isLinkedList(cur)) {
		return;
	}
	if (cur->nodeType != Op_Where) {
		makeJoinHelper(cur->left);
		makeJoinHelper(cur->right);
	}
	else {
		//��ǰ�ڵ�Ϊ�����ڵ㣬��һ���ڵ�Ϊ�ѿ������ڵ�
		SelectTreeNode* newCur = nullptr;
		if (cur->left && cur->left->nodeType == Op_Multiple) {
			//�ȱ���ѿ���������������ڵ�������ڵ�Ĳ�����
			SelectTreeNode* pLeft = cur->left->left;
			SelectTreeNode* pRight = cur->left->right;
			boolTree* bTree = cur->booltree;
			//�����ڵ��ǰһ���ڵ�
			SelectTreeNode* parent = parentRecord[cur];
			//���ɣ���ֵ�����ӵĽڵ�
			newCur = new SelectTreeNode(Op_EqualJoin);
			newCur->booltree = bTree;
			//�޸�ָ��
			parent->left = newCur;
			newCur->left = pLeft;
			newCur->right = pRight;
			update();
		}
		//makeJoinHelper(cur->left);
		if (newCur == nullptr) {
			makeJoinHelper(cur->left);
			makeJoinHelper(cur->right);
		}
		else {
			makeJoinHelper(newCur->left);
			makeJoinHelper(newCur->right);
		}
	}
}

bool Optimizer::isLinkedList(SelectTreeNode * cur)
{
	SelectTreeNode* p = cur;
	bool ok = true;
	while (p) {
		if (p->right) {
			ok = false;
			break;
		}
		if (!p->left) {
			break;
		}
		p = p->left;
	}
	return ok;
}

void Optimizer::pushDownProjectHelper(SelectTreeNode* cur, vector<string> v)
{
	//����Ըø��ڵ�cur������ʵ�����˻��������������·�ͶӰ���㷨��ֹ
	if (isLinkedList(cur)) {
		return;
	}
	//���v���ǿյģ���ô��vȥ��
	if (v.size()) {
		sort(v.begin(), v.end());
		auto it = unique(v.begin(), v.end());
		v.resize(it - v.begin());
	}
	//����Op_order, Op_having, Op_groupBy,��Op_natural�������·�ͶӰ
	if (cur->nodeType == Op_Order || cur->nodeType == Op_Having ||
		cur->nodeType == Op_Groupby) {
		pushDownProjectHelper(cur->left, v);
		return;
	}
	//��ͶӰ�ڵ��У�ֻ����Щ�������������ֶ��·�
	if (cur->nodeType == Op_Projection) {
		for (auto i : cur->colName) {
			string tmp = getTableName(i);
			if (tmp != i) {
				v.push_back(i);
			}
		}
		pushDownProjectHelper(cur->left, v);
	}
	/*
	��ѡ��ڵ��У�ֻ������sc.sno = s.sno��c.cpno = '5'�������ֶ��·�
	���������������ֶ�
	*/
	else if (cur->nodeType == Op_Where) {
		//�Ȼ��boolTree���漰���������ֶΣ��ٽ���ɸѡ����v��
		boolTree* bTree = cur->booltree;
		vector<string> fields = getBoolTreeAttributeList(bTree);
		for (auto i : fields) {
			string tmp = getTableName(i);
			if (tmp != i) {
				v.push_back(i);
			}
		}
		pushDownProjectHelper(cur->left, v);
	}
	/*
	���ڵѿ������ڵ㣬��v�е��ֶηָ���ͬ�ķ�֧��
	*/
	else if (cur->nodeType == Op_Multiple) {
		vector<string> left, right;
		for (auto i : v) {
			string tableName = getTableName(i);
			//leftΪ�·ŵ���ߵ��ֶΣ�rightΪ�·ŵ��ұߵ��ֶ�
			if (findHelper2(cur->left, tableName)) {
				left.push_back(i);
			}
			else if (findHelper2(cur->right, tableName)) {
				right.push_back(i);
			}
		}
		//�޸�cur�����Һ���ָ�룬����ͶӰ
		SelectTreeNode* pLeft = cur->left;
		SelectTreeNode* pRight = cur->right;
		if (left.size()) {
			SelectTreeNode* newLeft = new SelectTreeNode(Op_Projection);
			for (auto i : left) {
				newLeft->colName.push_back(i);
			}
			cur->left = newLeft;
			newLeft->left = pLeft;
		}
		if (right.size()) {
			SelectTreeNode* newRight = new SelectTreeNode(Op_Projection);
			for (auto i : right) {
				newRight->colName.push_back(i);
			}
			cur->right = newRight;
			newRight->left = pRight;
		}
		//�ݹ��޸�
		pushDownProjectHelper(cur->left, left);
		pushDownProjectHelper(cur->right, right);
	}
	//�����Ӳ����������������в������������ֶ��·�
	//��������������費���ܳ������Ӳ���
}

bool Optimizer::findHelper2(SelectTreeNode * root, string targetName)
{
	if (root == nullptr) {
		return false;
	}
	if (root->nodeType == Table_T && root->tableName == targetName) {
		return true;
	}
	if (root->nodeType == Table_T && root->table.getTitle() == targetName) {
		return true;
	}
	return findHelper2(root->left, targetName) ||
		findHelper2(root->right, targetName);
}

void Optimizer::update()
{
	parentRecord.clear();
	updateHelper(root, nullptr);
}

void Optimizer::updateHelper(SelectTreeNode* cur, SelectTreeNode* pre)
{
	if (!cur) {
		return;
	}
	parentRecord[cur] = pre;
	if (cur->left) {
		updateHelper(cur->left, cur);
	}
	if (cur->right) {
		updateHelper(cur->right, cur);
	}
}

SelectTreeNode* Optimizer::getLCA(SelectTreeNode* root, string s1, string s2)
{
	if (root == NULL || root->table.getTitle() == s1 || root->table.getTitle() == s2) return root;
	SelectTreeNode* left = getLCA(root->left, s1, s2);
	SelectTreeNode* right = getLCA(root->right, s1, s2);
	if (left && right) {
		return root;
	}
	else {
		return left == NULL ? right : left;
	}
}

pair<SelectTreeNode*, SelectTreeNode*> Optimizer::findHelper
(SelectTreeNode* cur, SelectTreeNode* pre, string targetTable)
{
	/*
	�ݹ���Ҷ�������Ѱ�ұ�����Ӧ�ı�ѡ�����ڵ�
	�������û�ҵ����򷵻�{nullptr, nullptr}
	*/
	if (cur && cur->tableName == targetTable) {
		return { cur, pre };
	}
	else if (cur && cur->table.getTitle() == targetTable) {
		return { cur, pre };
	}
	if (cur && cur->left) {
		pair<SelectTreeNode*, SelectTreeNode*> res = findHelper(cur->left, cur, targetTable);
		if (res.first && res.first->tableName == targetTable ||
			res.first && res.first->table.getTitle() == targetTable) {
			return res;
		}
	}
	if (cur && cur->right) {
		pair<SelectTreeNode*, SelectTreeNode*> res = findHelper(cur->right, cur, targetTable);
		if (res.first && res.first->tableName == targetTable ||
			res.first && res.first->table.getTitle() == targetTable) {
			return res;
		}
	}
	//û�ҵ�
	return { nullptr, nullptr };
}

string Optimizer::getTableName(const string& s)
{
	int k = 0;
	while (k < s.size() && s[k] != '.') {
		k++;
	}
	return k != s.size() ? s.substr(0, k) : s;
}

int getSize(string name)
{
	return 0;
}

/*SelectTreeNode* Optimizer::merge(
	vector<boolTreeNode*> nodeList3, vector<SelectTreeNode*> _res)
{
	/*
	�ú��������ϲ������������Ӳ�ѯ�ı�ʹ��̰���㷨
	ÿ�δ�����������ѡ�������С�Ľ��кϲ��ɵѿ�����
	����û�����������ı�������ɵѿ���������󷵻��ܽ��
	*
	//parentRecord.clear();
	//res����Ѿ��ϲ��Ľ��
	vector<SelectTreeNode*> res;
	//res_����Ѿ���ѡ�ϵı�
	set<string> res_;
	while (nodeList3.size()) {
		int cost = INT_MAX;
		boolTreeNode* deleted;
		SelectTreeNode* cur = nullptr;
		SelectTreeNode *p3 = nullptr, *p4 = nullptr;
		string name1, name2;
		//�Ҵ�����С�����ű�
		for (int i = 0; i < nodeList3.size(); i++) {
			pair<string, string> names = getTableNamePair(nodeList3[i]);
			//�����õ����������У�������һ�����Ѿ���ѡ���ˣ�������������
			if (res_.count(names.first) || res_.count(names.second)) {
				continue;
			}
			//�ɱ�����ñ��ָ��
			SelectTreeNode* p1 = nullptr, *p2 = nullptr;
			for (int j = 0; j < _res.size(); j++) {
				if (p1 && p2) {
					break;
				}
				if (_res[j]->tableName == names.first) {
					p1 = _res[j];
				}
				if (_res[j]->tableName == names.second) {
					p2 = _res[j];
				}
			}
			if (!p1 || !p2) {
				throw(InputError());
			}
			//�ұ�ڵ�����ȵ�ǰ�̣�������������ϲ�ĵѿ��������Ȼ�����ȵ�ǰ������left�������
			if (parentRecord.count(p1) && !(p1->nodeType == Op_Multiple || p1->nodeType == Table_T)) {
				p1 = parentRecord[p1];
			}
			if (parentRecord.count(p2) && !(p2->nodeType == Op_Multiple || p2->nodeType == Table_T)) {
				p2 = parentRecord[p2];
			}
			p1 = p1->left;
			p2 = p2->left;
			int cost1 = p1->nodeType == Op_Multiple ? p1->size : getSize(p1->tableName);
			int cost2 = p2->nodeType == Op_Multiple ? p1->size : getSize(p2->tableName);
			if (cost1 * cost2 < cost) {
				deleted = nodeList3[i];
				cost = cost1 * cost2;
				p3 = p1;
				p4 = p2;
				name1 = names.first;
				name2 = names.second;
			}
		}
		//�����µ�multiple�ڵ�
		cur = new SelectTreeNode(Op_Multiple);
		cur->left = p3;
		cur->right = p4;
		res.push_back(cur);
		//��parentRecord
		parentRecord[p3] = cur;
		parentRecord[p4] = cur;
		auto it1 = find(res.begin(), res.end(), p3);
		//res
		if (it1 != res.end()) {
			res.erase(it1);
		}
		auto it2 = find(res.begin(), res.end(), p4);
		if (it2 != res.end()) {
			res.erase(it2);
		}
		cur->size = cost;
		res_.insert(name1);
		res_.insert(name2);
		//ȥ���������
		nodeList3.erase(find(nodeList3.begin(), nodeList3.end(), deleted));
	}
	for (auto i = _res.begin(); i != _res.end(); ) {
		SelectTreeNode* cur = *i;
		auto j = find(res_.begin(), res_.end(), cur);
		if (j != res_.end()) {
			i = _res.erase(i);
		}
		else {
			i++;
		}
	}
	//_res��Ϊû��ѡ��ı�����Щ��������ɵѿ�����
	SelectTreeNode* res1 = merge2(_res, 0, _res.size() - 1);//δ���ӵı�
	SelectTreeNode* res2 = merge2(res, 0, res.size() - 1);//�����ӵı�
	//���ؽ��
	if (!res2) {
		return res1;
	}
	else if (!res1) {
		return res2;
	}
	SelectTreeNode* res3 = new SelectTreeNode(Op_Multiple);
	res3->left = res1;
	res3->right = res2;
	return res3;
}*/

SelectTreeNode* Optimizer::merge2(vector<SelectTreeNode*> v, int i, int j)
{
	if (i > j) {
		return nullptr;
	}
	if (i == j) {
		return v[i];
	}
	SelectTreeNode* root = new SelectTreeNode(Op_Multiple);
	root->left = merge2(v, i, (i + j) / 2);
	root->right = merge2(v, (i + 2) / 2 + 1, j);
	return root;
}

pair<string, string> Optimizer::getTableNamePair(boolTreeNode* p)
{
	return { getTableName(p->field), getTableName(p->vals[0]) };
}

void Optimizer::getTablePtr(SelectTreeNode* ptr, vector<SelectTreeNode*>& res)
{
	if (!ptr) {
	}
	else if (ptr->nodeType == Op_Multiple) {
		getTablePtr(ptr->left, res);
		getTablePtr(ptr->right, res);
	}
	else {
		res.push_back(ptr);
	}
}

SelectTree* Optimizer::getResult()
{
	return ok ? new SelectTree(root) : nullptr;
}

