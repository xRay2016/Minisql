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
	modeΪ1����ʾand�ڵ�
	modeΪ2����ʾor�ڵ�
	modeΪ3����ʾΪ�����ڵ㣬����some����any
	modeΪ4����ʾΪ��some���Ӳ�ѯ�������ڵ㣬��all�ڵ�
	modeΪ5����ʾΪ��any���Ӳ�ѯ�������ڵ�
	modeΪ6����ʾ����some��any���Ӳ�ѯ�������ڵ�
	*/
	int mode = 0;
	boolTreeNode* left = nullptr;
	boolTreeNode* right = nullptr;
	//field��ʾ�ֶ�
	string field;
	/*
	type��ʾ�Ҳ����������ͣ�Ϊ_INT or _DOUBLE or _STRING;���Ӳ�ѯ�У������ǵı�ʾ
	*/
	int type;
	/*
	op��ʾ�����������У�
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
	//vals����Ҳ����������Ϊ�Ӳ�ѯ�����Ӳ�ѯ���
	vector<string> vals;
	//isNot˵���������Ƿ�ȡ��
	bool isNot = 0;
	//����Ӳ�ѯ��Ӧ���﷨�����ڵ㣬����ʹ��vals��
	SelectTreeNode* subSelect = nullptr;
	//*****���rightOperandFieldΪ�棬˵���Ҳ��������ֶ�
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
	bool allAnd = true;//ֻ��ȫ������and�����Ż�
	void clear();
	bool isNull();
private:
	//��API�ṩ��Ҫ��ӵ��ֶ�
	vector<string> attributeList;
	//����ֶεĸ�����Ϣ
	//���У�Ϊ<���ֶ����֣� <���ֶ����ͣ����ֶ�ֵ>>
	map<string, pair<int, string>> m;
	int pushCnt = 0;//��Ϊpushϵ�к������б�ָ��
	boolTreeNode* root;
	vector<boolTreeNode*> nodeList;
	//modeΪ1����where�Ӿ䣬Ϊ2����having�Ӿ䣬���ڿ�������֧��check
	int mode;
	//����
	string showTreeHelper(boolTreeNode* p);
	void buildTree(vector<boolTreeNode*>& v);
	boolTreeNode* buildTreeHelper(vector<boolTreeNode*> v, int begin, int end);
	//ͨ���ƥ�䣬ͨ���Ϊ%��_
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
	//��������
	SelectTreeNode* left = nullptr;
	SelectTreeNode* right = nullptr;
	Operation_node nodeType;
	int projectionMode = 0;//0Ϊall��1Ϊdistcinct
	vector<int> orderMode;//0Ϊ����1Ϊ����
	boolTree* booltree = nullptr;//having��where�Ӿ�����
	//int tableMode;//0Ϊ��������ѯ(��groupby����)��1Ϊ�����Ӳ�ѯ��2Ϊ������Ȼ����
	SelectTreeNode* subSelect = nullptr;
	string tableName;//��������
	vector<string> colName;//���е�����
	string alias;//������ı���
	Table table;//�ƻ�����Table_T�ڵ㣬ֻ��table���������Ǳ���
	/*
	����Op_Where�ڵ㣨ֻ��һ����������
	belongTo����˵����Ӧ�ı���ʲô��
	haveTableName˵�����������ֶ��Ƿ������
	*/
	bool haveTableName = true;
	string belongTo;
	//����Op_Multiple�ڵ㣬�洢���ӽڵ�ı���
	vector<string> childTable;
	//����Op_Multipls�ڵ㣬�洢����ʱ��Ĵ�С
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
	//�洢��ָ��
	map<SelectTreeNode*, SelectTreeNode*> parentRecord;
	SelectTreeNode* root;
	SelectTree::SelectParameter executeHelper(SelectTreeNode* t);
	//���¸�ָ���parentRecord
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
	int ok = 0;//0��ʾ�������˳���1��ʾ�Ż���������
	SelectTreeNode* root;
	//ĳЩ����������õ��ĳ�Ա����
	SelectTreeNode* root1;//root1Ϊwhere�ڵ�
	SelectTreeNode* root2;//root2Ϊroot1�ĸ��ڵ�
	SelectTreeNode* root3;//root3Ϊwhere�ڵ���ӽڵ�
	//������¼��ָ�룬��Ե���from����
	map<SelectTreeNode*, SelectTreeNode*> parentRecord;
	//map	m	�����洢������Ӧ������������Ҳ������ı���
	map<boolTreeNode*, pair<string, string>> m;
	/*
	//��ѯ�Ż���Ӧ����
	*/
	//�Ż����
	void exec();
	void resolveAndPushSelect();
	void pushDownProject();
	void makeJoin();
	/*
	//һЩ��������
	*/
	bool projectOK(const vector<string>& v);
	void makeJoinHelper(SelectTreeNode* cur);
	bool isLinkedList(SelectTreeNode* cur);
	void pushDownProjectHelper(SelectTreeNode* cur, vector<string> v);
	//�������ڵ㣬�жϸ������Ƿ����ĳ�ű�
	bool findHelper2(SelectTreeNode* root, string targetName);
	//���¸�ָ���parentRecord
	void update();
	void updateHelper(SelectTreeNode* cur, SelectTreeNode* pre);
	//�ҵ�������ڵ�������������
	SelectTreeNode* getLCA(SelectTreeNode* root, string s1, string s2);
	//��ѡ�������ҵ�ĳ���Ӧ�Ľڵ��Լ�ǰ��
	//����ֵ:�ҵ��Ľڵ��ǰ��
	pair<SelectTreeNode*, SelectTreeNode*> findHelper(SelectTreeNode* cur,
		SelectTreeNode* pre, string targetTable);
	//����SC.SNO�������ֶΣ�����SC
	string getTableName(const string& s);
	//���°��������������еѿ�����
	/*SelectTreeNode* merge(
		vector<boolTreeNode*> nodeList3, vector<SelectTreeNode*> _res);*/
		//�鲢SelectTreeNode*
	SelectTreeNode* merge2(vector<SelectTreeNode*> v, int i, int j);
	//����������������Ĳ�������Ӧ���ű������
	pair<string, string> getTableNamePair(boolTreeNode* p);
	//�������ԭ�﷨���У����ǵ�ָ��
	void getTablePtr(SelectTreeNode* src, vector<SelectTreeNode*>& res);
public:
	Optimizer(SelectTree* p) : root(p->root) { exec(); }
	Optimizer(SelectTreeNode* p) : root(p) { exec(); }
	SelectTree* getResult();
};




#endif // !DATASTRUCTURE


