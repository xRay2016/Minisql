#ifndef DATA_STRUCTURE
#define DATA_STRUCTURE
#include "parser.h"
#include <queue>
#include <map>
#include "api.h"
#include "error.h"
using namespace std;

void gotoxy(int y, int x);
void myCout(string str);

struct boolTreeNode
{
	/*
	mode为1，表示and节点
	mode为2，表示or节点
	mode为3，表示为条件节点，不带some或者any
	mode为4，表示为带some的子查询的条件节点，是all节点
	mode为5，表示为带any的子查询的条件节点
	mode为6，表示不带some或any的子查询的条件节点
	*/
	int mode = 0;
	boolTreeNode* left = nullptr;
	boolTreeNode* right = nullptr;
	//field表示字段
	string field;
	/*
	type表示右操作数的类型，为_INT or _DOUBLE or _STRING;在子查询中，用他们的表示
	*/
	int type;
	/*
	op表示操作符，其中：
	<		1
	<=		2
	>		3
	>=		4
	!=		5
	=		6
	like	7
	in		8
	between and 9
	*/
	int op;
	//vals存放右操作数，如果为子查询则存放子查询语句
	vector<string> vals;
	//isNot说明该条件是否取非
	bool isNot = 0;
	//存放子查询对应的语法树根节点，不再使用vals存
	SelectTreeNode* subSelect = nullptr;
	//*****如果rightOperandField为真，说明右操作数是字段
	bool rightOperandField = false;
	boolTreeNode(int _mode = 0);
	void execSubselect();
};

class boolTree
{
	friend boolTreeNode* getBoolTreeRoot(boolTree* p);
	friend vector<string>& getBoolTreeAttributeList(boolTree* p);
public:
	boolTree(int _mode, const vector<boolTreeNode*>& v);
	boolTree();
	~boolTree();
	void pushInt(int a);
	void pushDouble(double a);
	void pushString(const string& s);
	void pushArrributes(const vector<Attribute>& v);
	bool getResult();
	string showTree();
	vector<string> getAttributeList();
	bool allAnd = true;//只有全部都是and才能优化
	void clear();
	bool isNull();
private:
	//给API提供的要添加的字段
	vector<string> attributeList;
	//存放字段的各种信息
	//其中，为<右字段名字， <右字段类型，右字段值>>
	map<string, pair<int, string>> m;
	int pushCnt = 0;//作为push系列函数的列表指针
	boolTreeNode* root;
	vector<boolTreeNode*> nodeList;
	//mode为1则是where子句，为2则是having子句，后期可以扩充支持check
	int mode;
	//函数
	string showTreeHelper(boolTreeNode* p);
	void buildTree(vector<boolTreeNode*>& v);
	boolTreeNode* buildTreeHelper(vector<boolTreeNode*> v, int begin, int end);
	//通配符匹配，通配符为%或_
	bool wildcardMatch(string s, string p);
	bool getResultHelper(boolTreeNode* p);
	template<typename T1, typename T2>
	bool cmp(int op, T1 x, T2 y);
};

enum Operation_node
{
	Op_Where, Op_Projection, Table_T, Op_Multiple, Op_Groupby, Op_Having, Op_Order, Op_Intersect,
	Op_Union, Op_Except, Op_Natural, Op_EqualJoin
};


struct SelectTreeNode {
	SelectTreeNode(Operation_node op) : nodeType(op) {}
	SelectTreeNode() {}
	string toString();
	//变量定义
	SelectTreeNode* left = nullptr;
	SelectTreeNode* right = nullptr;
	Operation_node nodeType;
	int projectionMode = 0;//0为all，1为distcinct
	vector<int> orderMode;//0为升序，1为降序
	boolTree* booltree = nullptr;//having和where子句条件
	//int tableMode;//0为单表或多表查询(与groupby共用)，1为单个子查询，2为单个自然连接
	SelectTreeNode* subSelect = nullptr;
	string tableName;//存表的名字
	vector<string> colName;//存列的名字
	string alias;//派生表的别名
	Table table;//计划对于Table_T节点，只用table来存表而不是表名
	/*
	对于Op_Where节点（只有一个条件），
	belongTo属性说明对应的表是什么，
	haveTableName说明该条件的字段是否带表名
	*/
	bool haveTableName = true;
	string belongTo;
	//对于Op_Multiple节点，存储孩子节点的表名
	vector<string> childTable;
	//对于Op_Multipls节点，存储该临时表的大小
	int size = 0;
};

class SelectTree
{
	friend class Optimizer;
	struct SelectParameter
	{
		vector<Table> from;
		boolTree* where = nullptr;
		vector<string> gattr;
		boolTree* having = nullptr;
		vector<string> select;
		vector<pair<string, UD>> oattr;
		int mode = 0;
	};
private:
	//存储父指针
	map<SelectTreeNode*, SelectTreeNode*> parentRecord;
	SelectTreeNode* root;
	SelectTree::SelectParameter executeHelper(SelectTreeNode* t);
	//更新父指针表parentRecord
	void update();
	void updateHelper(SelectTreeNode* cur, SelectTreeNode* pre);
public:
	SelectTree(SelectTreeNode* p = nullptr) : root(p) {}
	void BFSDisplay();
	Table execute();
};

class Optimizer
{
private:
	int ok = 0;//0表示非正常退出，1表示优化正常结束
	SelectTreeNode* root;
	//某些函数里面会用到的成员变量
	SelectTreeNode* root1;//root1为where节点
	SelectTreeNode* root2;//root2为root1的父节点
	SelectTreeNode* root3;//root3为where节点的子节点
	//用来记录父指针，针对的是from子树
	map<SelectTreeNode*, SelectTreeNode*> parentRecord;
	//map	m	用来存储条件对应的左操作数和右操作数的表名
	map<boolTreeNode*, pair<string, string>> m;
	/*
	//查询优化对应函数
	*/
	//优化入口
	void exec();
	void resolveAndPushSelect();
	void pushDownProject();
	void makeJoin();
	/*
	//一些辅助函数
	*/
	bool projectOK(const vector<string>& v);
	void makeJoinHelper(SelectTreeNode* cur);
	bool isLinkedList(SelectTreeNode* cur);
	void pushDownProjectHelper(SelectTreeNode* cur, vector<string> v);
	//给定根节点，判断该子树是否包括某张表
	bool findHelper2(SelectTreeNode* root, string targetName);
	//更新父指针表parentRecord
	void update();
	void updateHelper(SelectTreeNode* cur, SelectTreeNode* pre);
	//找到两个表节点的最近公共祖先
	SelectTreeNode* getLCA(SelectTreeNode* root, string s1, string s2);
	//在选择树中找到某表对应的节点以及前继
	//返回值:找到的节点和前继
	pair<SelectTreeNode*, SelectTreeNode*> findHelper(SelectTreeNode* cur,
		SelectTreeNode* pre, string targetTable);
	//对于SC.SNO这样的字段，返回SC
	string getTableName(const string& s);
	//重新按照连接条件排列笛卡儿积
	/*SelectTreeNode* merge(
		vector<boolTreeNode*> nodeList3, vector<SelectTreeNode*> _res);*/
		//归并SelectTreeNode*
	SelectTreeNode* merge2(vector<SelectTreeNode*> v, int i, int j);
	//用来获得连接条件的布尔树对应两张表的名字
	pair<string, string> getTableNamePair(boolTreeNode* p);
	//用来获得原语法树中，表们的指针
	void getTablePtr(SelectTreeNode* src, vector<SelectTreeNode*>& res);
public:
	Optimizer(SelectTree* p) : root(p->root) { exec(); }
	Optimizer(SelectTreeNode* p) : root(p) { exec(); }
	SelectTree* getResult();
};




#endif // !DATASTRUCTURE


