
#ifndef _BPLUSTREE_H_
#define _BPLUSTREE_H_ 1
#pragma warning(disable:4996)  
#define ff "./src/"
#include<algorithm>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include "basic.h"
#include "const.h"
#include "error.h"
#include "buffer_manager.h"
#include "template_function.h"
using namespace std;

extern BufferManager buffer_manager;

//ģ����TreeNode�����ڴ��B+���Ľ�������Լ�������صĲ���
//ʹ��ģ���ౣֱ֤������int��float, string������������
//���ٴ����ģ
template <typename T>
class TreeNode {
public:
	//�ý����key����
	unsigned int num;
	//ָ�򸸽ڵ�ָ��
	TreeNode* parent;
	//���key����
	std::vector <T> keys;
	//ָ���ӽ���ָ������
	std::vector <TreeNode*> childs;
	std::vector <int> vals;
	//ָ����һ��Ҷ����ָ��
	TreeNode* nextLeafNode;
	//�˽���Ƿ���Ҷ���ı�־
	bool isLeaf;
	//�����Ķ�
	int degree;

public:
	//���캯��
	//���룺���Ķȣ��Ƿ�ΪҶ���
	//���ܣ���ʼ��һ���µĽ�㣬��Leaf=true�򴴽�һ��Ҷ��㣬���򴴽�һ��֦�ɽ��
	TreeNode(int in_degree, bool Leaf = false);
	//��������
	~TreeNode();
	//���ܣ��ж��Ƿ�Ϊ�����
	bool isRoot();
	//���룺keyֵ��(index)
	//�����bool
	//���ܣ��жϽ�����Ƿ����ĳkeyֵ����ͨ��index���÷��ظ�ֵindex
	//��ͨ����������boolֵ˵���Ƿ��ҵ�
	bool findKey(T key, unsigned int &index);
	//�����TreeNodeָ��
	//���ܣ���һ�������ѳ��������(��B+���ķ����й�)
	//�½��Ϊ��������һ�����
	//ͬʱ����key���÷���ȥ���ϲ��keyֵ
	TreeNode* splitNode(T &key);
	//���룺keyֵ
	//�������ӵ�position
	//���ܣ���֦�ɽ�������keyֵ����������ӵ�λ��
	unsigned int addKey(T &key);
	//���룺keyֵ
	//�������ӵ�position
	//���ܣ���Ҷ��������keyֵ����������ӵ�λ��
	unsigned int addKey(T &key, int val);
	//���룺keyֵ��Ӧ��index
	//�����bool
	//���ܣ��ڽ����ɾ��index��Ӧ��keyֵ
	bool deleteKeyByIndex(unsigned int index);
	//������һ������Ҷ���ָ��
	TreeNode* nextLeaf();
	//����:��ʼindex,��ֹkey�����ؽ��vector����
	//���:������ֹkey����true�����򷵻�flase
	//���ܣ�����һ����Χ��value����
	bool findRange(unsigned int index, T& key, std::vector<int>& vals);
	bool findRange2(unsigned int index, std::vector<int>& vals);

	void printl();
};

//ģ����BPlusTree�����ڶ�����B+���Ĳ���
//�ɴ�ģ�����TreeNodeģ�飬�����ɴ�ģ����buffer_managerģ����н���
//����ļ��Ķ�д����
template <typename T>
class BPlusTree {
private:
	//������TreeNodeָ�룬�����߽��в�������
	typedef TreeNode<T>* Tree;
	//����ṹ��������ʱ�洢���ҵ�keyֵ��������λ�ã�������в���
	struct searchNodeParse {
		Tree pNode; //������Ӧkey�Ľ��ָ��
		unsigned int index; //key�ڽ���е�index
		bool ifFound; //�Ƿ��ҵ���key
	};

private:
	//�ļ����������ļ���д����
	std::string file_name;
	//�����ָ��
	Tree root;
	//Ҷ������ͷָ��
	Tree leafHead;
	//����key������
	unsigned int key_num;
	//������level����
	unsigned int level;
	//������node����
	unsigned int node_num;
	//fileNode* file; // the filenode of this tree
	//ÿ��keyֵ��size������һ������˵������key��sizeӦ����ͬ��
	int key_size;
	//���Ķ�
	int degree;

public:
	//���캯��
	//���ڹ���һ���µ�����ȷ��m_name,key��size�����Ķ�
	//ͬʱ������������Ϊ���������ڴ�
	BPlusTree(std::string m_name, int key_size, int degree);
	//��������
	//�ͷ���Ӧ���ڴ�
	~BPlusTree();

	//����keyֵ���ض�Ӧ��Value
	int searchVal(T &key);
	//���룺keyֵ����value
	//�����bool
	//���ܣ������в���һ��keyֵ
	//�����Ƿ����ɹ�
	bool insertKey(T &key, int val);
	//���룺keyֵ
	//�����bool
	//���ܣ�������ɾ��һ��keyֵ
	//�����Ƿ�ɾ���ɹ�
	bool deleteKey(T &key);
	//���룺���ĸ����
	//���ܣ�ɾ�����������ͷ��ڴ�ռ䣬��Ҫ��������������
	void dropTree(Tree node);
	//���룺key1��key2������vals������
	//���ܣ����ط�Χ�����������value����vals������
	void searchRange(T &key1, T &key2, std::vector<int>& vals, int flag);

	//�Ӵ��̶�ȡ��������
	void readFromDiskAll();
	//��������д�����
	void writtenbackToDiskAll();
	//�ڴ����ж�ȡĳһ�������
	void readFromDisk(char *p, char* end);

	void printleaf();

private:
	//��ʼ��B+�����������ڴ�ռ�
	void initTree();
	//���ڲ���ĳkey����������������������е���
	bool adjustAfterinsert(Tree pNode);
	//����ɾ��ĳkey����ܳ��ֽ�������������������е���
	bool adjustAfterDelete(Tree pNode);
	//���ڲ���ĳkeyֵ������Ҷ���λ��
	void findToLeaf(Tree pNode, T key, searchNodeParse &snp);
	//��ȡ�ļ���С
	void getFile(std::string file_path);
	int getBlockNum(std::string table_name);
};

template <class T>
TreeNode<T>::TreeNode(int in_degree, bool Leaf) :
	num(0),
	parent(NULL),
	nextLeafNode(NULL),
	isLeaf(Leaf),
	degree(in_degree)
{
	for (unsigned i = 0; i < degree + 1; i++) {
		childs.push_back(NULL);
		keys.push_back(T());
		vals.push_back(int());
	}

	childs.push_back(NULL);
}

//��������ʹ��Ĭ��������������
template <class T>
TreeNode<T>::~TreeNode()
{

}

//���ܣ��ж��Ƿ�Ϊ�����
template <class T>
bool TreeNode<T>::isRoot()
{
	if (parent != NULL)
		return false;
	else
		return true;
}

//���룺keyֵ��(index)
//�����bool
//���ܣ��жϽ�����Ƿ����ĳkeyֵ����ͨ��index���÷��ظ�ֵindex
//�粻���ڣ��򷵻���ӽ���index
//��ͨ����������boolֵ˵���Ƿ��ҵ�
template <class T>
bool TreeNode<T>::findKey(T key, unsigned int &index)
{
	if (num == 0) { //�����key����Ϊ0
		index = 0;
		return false;
	}
	else {
		//�ж�keyֵ�Ƿ񳬹�����������ֵ(key���ڱ������)
		if (keys[num - 1] < key) {
			index = num;
			return false;
			//�ж�keyֵ�Ƿ�С�ڱ��������Сֵ(key���ڱ������)
		}
		else if (keys[0] > key) {
			index = 0;
			return false;
		}
		else if (num <= 20) {
			//�����key��������ʱֱ�����Ա�����������
			for (unsigned int i = 0; i < num; i++) {
				if (keys[i] == key) {
					index = i;
					return true;
				}
				else if (keys[i] < key)
					continue;
				else if (keys[i] > key) {
					index = i;
					return false;
				}
			}
		}
		else if (num > 20) {
			//�����key��������ʱ���ö�������
			unsigned int left = 0, right = num - 1, pos = 0;

			while (right > left + 1) {
				pos = (right + left) / 2;
				if (keys[pos] == key) {
					index = pos;
					return true;
				}
				else if (keys[pos] < key) {
					left = pos;
				}
				else if (keys[pos] > key) {
					right = pos;
				}
			}

			if (keys[left] >= key) {
				index = left;
				return (keys[left] == key);
			}
			else if (keys[right] >= key) {
				index = right;
				return (keys[right] == key);
			}
			else if (keys[right] < key) {
				index = right++;
				return false;
			}
		}//������������
	}

	return false;
}

//�����TreeNodeָ��
//���ܣ���һ�������ѳ��������(��B+���ķ����й�)
//�½��Ϊ��������һ�����
//ͬʱ����key���÷���ȥ���ϲ��keyֵ
template <class T>
TreeNode<T>* TreeNode<T>::splitNode(T &key)
{
	//��С�������
	unsigned int minmumNodeNum = (degree - 1) / 2;
	//�����½��
	TreeNode* newNode = new TreeNode(degree, this->isLeaf);

	/*
	if (newNode == NULL) {
	cout << "Problems in allocate momeory of TreeNode in splite node of " << key << endl;
	exit(2);
	}
	*/

	//��ǰ���ΪҶ������
	if (isLeaf) {
		key = keys[minmumNodeNum + 1];
		//���Ұ벿��keyֵ�������½����
		for (unsigned int i = minmumNodeNum + 1; i < degree; i++) {
			newNode->keys[i - minmumNodeNum - 1] = keys[i];
			keys[i] = T();
			newNode->vals[i - minmumNodeNum - 1] = vals[i];
			vals[i] = int();
		}
		//���½����õ�������ұ�
		newNode->nextLeafNode = this->nextLeafNode;
		this->nextLeafNode = newNode;
		newNode->parent = this->parent;

		//�����������key����
		newNode->num = minmumNodeNum;
		this->num = minmumNodeNum + 1;
	}
	else if (!isLeaf) {  //��Ҷ������
		key = keys[minmumNodeNum];
		//�����ӽ��ָ�����½��
		for (unsigned int i = minmumNodeNum + 1; i < degree + 1; i++) {
			newNode->childs[i - minmumNodeNum - 1] = this->childs[i];
			newNode->childs[i - minmumNodeNum - 1]->parent = newNode;
			this->childs[i] = NULL;
		}
		//����keyֵ���½��
		for (unsigned int i = minmumNodeNum + 1; i < degree; i++) {
			newNode->keys[i - minmumNodeNum - 1] = this->keys[i];
			this->keys[i] = T();
		}
		//��������໥λ�ù�ϵ
		this->keys[minmumNodeNum] = T();
		newNode->parent = this->parent;

		//���������key����
		newNode->num = minmumNodeNum;
		this->num = minmumNodeNum;
	}

	return newNode;
}

//���룺keyֵ
//�������ӵ�position
//���ܣ���֦�ɽ�������keyֵ����������ӵ�λ��
template <class T>
unsigned int TreeNode<T>::addKey(T &key)
{
	//���������key
	if (num == 0) {
		keys[0] = key;
		num++;
		return 0;
	}
	else {
		//�����Ƿ�Keyֵ�Ѿ�����
		unsigned int index = 0;
		bool exist = findKey(key, index);
		if (exist) {
			/*
			cout << "Error:In add(T &key),key has already in the tree!" << endl;
			exit(3);
			*/
		}
		else { //�����ڣ����Խ��в���
			   //��������keyֵ
			for (unsigned int i = num; i > index; i--)
				keys[i] = keys[i - 1];
			keys[index] = key;

			//�����ӽ��ָ�����
			for (unsigned int i = num + 1; i > index + 1; i--)
				childs[i] = childs[i - 1];
			childs[index + 1] = NULL;
			num++;

			return index;
		}
	}

	return 0;
}

//���룺keyֵ
//�������ӵ�position
//���ܣ���Ҷ��������keyֵ����������ӵ�λ��
template <class T>
unsigned int TreeNode<T>::addKey(T &key, int val)
{	//����Ҷ��㣬�޷�����
	if (!isLeaf) {
		/*
		cout << "Error:add(T &key,int val) is a function for leaf nodes" << endl;
		*/
		return -1;
	}

	//�����û��keyֵ
	if (num == 0) {
		keys[0] = key;
		vals[0] = val;
		num++;
		return 0;
	}
	else { //��������
		unsigned int index = 0;
		bool exist = findKey(key, index);
		if (exist) {
			/*
			cout << "Error:In add(T &key, int val),key has already in the tree!" << endl;
			exit(3);
			*/
		}
		else {
			//�������keyֵ
			for (unsigned int i = num; i > index; i--) {
				keys[i] = keys[i - 1];
				vals[i] = vals[i - 1];
			}
			keys[index] = key;
			vals[index] = val;
			num++;
			return index;
		}
	}

	return 0;
}

//���룺keyֵ��Ӧ��index
//�����bool
//���ܣ��ڽ����ɾ��index��Ӧ��keyֵ
template <class T>
bool TreeNode<T>::deleteKeyByIndex(unsigned int index)
{
	//index���󣬳�������㷶Χ
	if (index > num) {
		/*
		cout << "Error:In removeAt(unsigned int index), index is more than num!" << endl;
		*/
		return false;
	}
	else { //��������ɾ��
		if (isLeaf) { //Ҷ������
					  //�������keyֵ
			for (unsigned int i = index; i < num - 1; i++) {
				keys[i] = keys[i + 1];
				vals[i] = vals[i + 1];
			}
			keys[num - 1] = T();
			vals[num - 1] = int();
		}
		else { //֦�ɽ�����
			   //����keyֵ
			for (unsigned int i = index; i < num - 1; i++)
				keys[i] = keys[i + 1];

			//�����ӽ��ָ��
			for (unsigned int i = index + 1; i < num; i++)
				childs[i] = childs[i + 1];

			keys[num - 1] = T();
			childs[num] = NULL;
		}

		num--;
		return true;
	}

	return false;
}

//������һ������Ҷ���ָ��
template <class T>
TreeNode<T>* TreeNode<T>::nextLeaf()
{
	return nextLeafNode;
}

//���룬��ʼindex,��ֹkey�����ؽ��vector����
//���:������ֹkey����true�����򷵻�flase
//���ܣ�����һ����Χ��value����
template <class T>
bool TreeNode<T>::findRange(unsigned int index, T& key, std::vector<int>& valsout)
{
	unsigned int i;
	for (i = index; i < num && keys[i] <= key; i++)
		valsout.push_back(vals[i]);

	if (keys[i] >= key)
		return true;
	else
		return false;
}

template <class T>
bool TreeNode<T>::findRange2(unsigned int index, std::vector<int>& valsout)
{
	unsigned int i;
	for (i = index; i < num; i++)
		valsout.push_back(vals[i]);

	return false;
}

/*   Class   BPlusTree   */

//���캯��
//���ڹ���һ���µ�����ȷ��m_name,key��size�����Ķ�
//��ʼ����������
//ͬʱ������������Ϊ���������ڴ�
template <class T>
BPlusTree<T>::BPlusTree(std::string in_name, int keysize, int in_degree) :
	file_name(in_name),
	key_num(0),
	level(0),
	node_num(0),
	root(NULL),
	leafHead(NULL),
	key_size(keysize),
	degree(in_degree)
{
	//��ʼ�������ڴ沢�Ӵ��̶�ȡ����
	//��������
	initTree();
	readFromDiskAll();
}

//��������
//�ͷ���Ӧ���ڴ�
template <class T>
BPlusTree<T>:: ~BPlusTree()
{
	dropTree(root);
	key_num = 0;
	root = NULL;
	level = 0;
}

//��ʼ��B+�����������ڴ�ռ�
template <class T>
void BPlusTree<T>::initTree()
{
	root = new TreeNode<T>(degree, true);
	key_num = 0;
	level = 1;
	node_num = 1;
	leafHead = root;
}

//���ڲ���ĳkeyֵ������Ҷ���λ��
template <class T>
void BPlusTree<T>::findToLeaf(Tree pNode, T key, searchNodeParse &snp)
{
	unsigned int index = 0;
	//�ڶ�Ӧ����ڲ���keyֵ
	if (pNode->findKey(key, index)) {
		//���˽����Ҷ��㣬����ҳɹ�
		if (pNode->isLeaf) {
			snp.pNode = pNode;
			snp.index = index;
			snp.ifFound = true;
		}
		else {
			//�˽�㲻���ӽ�㣬����������һ��
			pNode = pNode->childs[index + 1];
			while (!pNode->isLeaf) {
				pNode = pNode->childs[0];
			}
			//��Ϊ���ҵ�keyֵ����������ײ�Ҷ���index[0]��Ϊ��key
			snp.pNode = pNode;
			snp.index = 0;
			snp.ifFound = true;
		}

	}
	else { //�������δ�ҵ���key
		if (pNode->isLeaf) {
			//���˽���Ѿ���Ҷ��������ʧ��
			snp.pNode = pNode;
			snp.index = index;
			snp.ifFound = false;
		}
		else {
			//�ݹ�Ѱ����һ��
			findToLeaf(pNode->childs[index], key, snp);
		}
	}

	return;
}

//���룺keyֵ����value
//�����bool
//���ܣ������в���һ��keyֵ
//�����Ƿ����ɹ�
template <class T>
bool BPlusTree<T>::insertKey(T &key, int val)
{
	searchNodeParse snp;
	//����㲻����
	if (!root)
		initTree();
	//���Ҳ���ֵ�Ƿ����
	findToLeaf(root, key, snp);
	if (snp.ifFound) { //�Ѵ���
					   /*
					   cout << "Error:in insert key to index: the duplicated key!" << endl;
					   */
		return false;
	}
	else { //�����ڣ����Բ���
		snp.pNode->addKey(key, val);
		//�������������Ҫ���е���
		if (snp.pNode->num == degree) {
			adjustAfterinsert(snp.pNode);
		}
		key_num++;
		return true;
	}

	return false;
}

//���ڲ���ĳkey����������������������е���
template <class T>
bool BPlusTree<T>::adjustAfterinsert(Tree pNode)
{
	T key;
	Tree newNode = pNode->splitNode(key);
	node_num++;

	//��ǰ���Ϊ��������
	if (pNode->isRoot()) {
		Tree root = new TreeNode<T>(degree, false);
		if (root == NULL) {
			/*
			cout << "Error: can not allocate memory for the new root in adjustAfterinsert" << endl;
			exit(1);
			*/
		}
		else {
			level++;
			node_num++;
			this->root = root;
			pNode->parent = root;
			newNode->parent = root;
			root->addKey(key);
			root->childs[0] = pNode;
			root->childs[1] = newNode;
			return true;
		}
	}
	else { //��ǰ���Ǹ����
		Tree parent = pNode->parent;
		unsigned int index = parent->addKey(key);

		parent->childs[index + 1] = newNode;
		newNode->parent = parent;
		//�ݹ���е���
		if (parent->num == degree)
			return adjustAfterinsert(parent);

		return true;
	}

	return false;
}

template <class T>
int BPlusTree<T>::searchVal(T& key)
{
	if (!root)
		return -1;
	searchNodeParse snp;
	findToLeaf(root, key, snp);

	if (!snp.ifFound)
		return -1;
	else
		return snp.pNode->vals[snp.index];
}

//���룺keyֵ
//�����bool
//���ܣ�������ɾ��һ��keyֵ
//�����Ƿ�ɾ���ɹ�
template <class T>
bool BPlusTree<T>::deleteKey(T &key)
{
	searchNodeParse snp;
	//����㲻����
	if (!root) {
		/*
		cout << "ERROR: In deleteKey, no nodes in the tree " << fileName << "!" << endl;
		*/
		return false;
	}
	else { //�������в���
		   //����λ��
		findToLeaf(root, key, snp);
		if (!snp.ifFound) { //�Ҳ�����key
							/*
							cout << "ERROR: In deleteKey, no keys in the tree " << fileName << "!" << endl;
							*/
			return false;
		}
		else { //�����ҵ�����ɾ��
			if (snp.pNode->isRoot()) { //��ǰΪ�����
				snp.pNode->deleteKeyByIndex(snp.index);
				key_num--;
				return adjustAfterDelete(snp.pNode);
			}
			else {
				if (snp.index == 0 && leafHead != snp.pNode) {
					//key������֦�ɽ����
					//����һ��ȥ����֦�ɲ�
					unsigned int index = 0;

					Tree now_parent = snp.pNode->parent;
					bool if_found_inBranch = now_parent->findKey(key, index);
					while (!if_found_inBranch) {
						if (now_parent->parent)
							now_parent = now_parent->parent;
						else
							break;
						if_found_inBranch = now_parent->findKey(key, index);
					}

					now_parent->keys[index] = snp.pNode->keys[1];

					snp.pNode->deleteKeyByIndex(snp.index);
					key_num--;
					return adjustAfterDelete(snp.pNode);

				}
				else { //ͬʱ��Ȼ������Ҷ���
					snp.pNode->deleteKeyByIndex(snp.index);
					key_num--;
					return adjustAfterDelete(snp.pNode);
				}
			}
		}
	}

	return false;
}

//����ɾ��ĳkey����ܳ��ֽ�������������������е���
template <class T>
bool BPlusTree<T>::adjustAfterDelete(Tree pNode)
{
	unsigned int minmumKeyNum = (degree - 1) / 2;
	//���ֲ���Ҫ���������
	if (((pNode->isLeaf) && (pNode->num >= minmumKeyNum)) ||
		((degree != 3) && (!pNode->isLeaf) && (pNode->num >= minmumKeyNum - 1)) ||
		((degree == 3) && (!pNode->isLeaf) && (pNode->num < 0))) {
		return  true;
	}
	if (pNode->isRoot()) { //��ǰ���Ϊ�����
		if (pNode->num > 0) //����Ҫ����
			return true;
		else { //������Ҫ����
			if (root->isLeaf) { //����Ϊ�������
				delete pNode;
				root = NULL;
				leafHead = NULL;
				level--;
				node_num--;
			}
			else { //���ڵ㽫��Ϊ��ͷ��
				root = pNode->childs[0];
				root->parent = NULL;
				delete pNode;
				level--;
				node_num--;
			}
		}
	}
	else { //�Ǹ��ڵ����
		Tree parent = pNode->parent, brother = NULL;
		if (pNode->isLeaf) { //��ǰΪҶ�ڵ�
			unsigned int index = 0;
			parent->findKey(pNode->keys[0], index);

			//ѡ�����ֵ�
			if ((parent->childs[0] != pNode) && (index + 1 == parent->num)) {
				brother = parent->childs[index];
				if (brother->num > minmumKeyNum) {
					for (unsigned int i = pNode->num; i > 0; i--) {
						pNode->keys[i] = pNode->keys[i - 1];
						pNode->vals[i] = pNode->vals[i - 1];
					}
					pNode->keys[0] = brother->keys[brother->num - 1];
					pNode->vals[0] = brother->vals[brother->num - 1];
					brother->deleteKeyByIndex(brother->num - 1);

					pNode->num++;
					parent->keys[index] = pNode->keys[0];
					return true;

				}
				else {
					parent->deleteKeyByIndex(index);

					for (int i = 0; i < pNode->num; i++) {
						brother->keys[i + brother->num] = pNode->keys[i];
						brother->vals[i + brother->num] = pNode->vals[i];
					}
					brother->num += pNode->num;
					brother->nextLeafNode = pNode->nextLeafNode;

					delete pNode;
					node_num--;

					return adjustAfterDelete(parent);
				}

			}
			else {
				if (parent->childs[0] == pNode)
					brother = parent->childs[1];
				else
					brother = parent->childs[index + 2];
				if (brother->num > minmumKeyNum) {
					pNode->keys[pNode->num] = brother->keys[0];
					pNode->vals[pNode->num] = brother->vals[0];
					pNode->num++;
					brother->deleteKeyByIndex(0);
					if (parent->childs[0] == pNode)
						parent->keys[0] = brother->keys[0];
					else
						parent->keys[index + 1] = brother->keys[0];
					return true;

				}
				else {
					for (int i = 0; i < brother->num; i++) {
						pNode->keys[pNode->num + i] = brother->keys[i];
						pNode->vals[pNode->num + i] = brother->vals[i];
					}
					if (pNode == parent->childs[0])
						parent->deleteKeyByIndex(0);
					else
						parent->deleteKeyByIndex(index + 1);
					pNode->num += brother->num;
					pNode->nextLeafNode = brother->nextLeafNode;
					delete brother;
					node_num--;

					return adjustAfterDelete(parent);
				}
			}

		}
		else { //֦�ɽڵ����
			unsigned int index = 0;
			parent->findKey(pNode->childs[0]->keys[0], index);
			if ((parent->childs[0] != pNode) && (index + 1 == parent->num)) {
				brother = parent->childs[index];
				if (brother->num > minmumKeyNum - 1) {
					pNode->childs[pNode->num + 1] = pNode->childs[pNode->num];
					for (unsigned int i = pNode->num; i > 0; i--) {
						pNode->childs[i] = pNode->childs[i - 1];
						pNode->keys[i] = pNode->keys[i - 1];
					}
					pNode->childs[0] = brother->childs[brother->num];
					pNode->keys[0] = parent->keys[index];
					pNode->num++;

					parent->keys[index] = brother->keys[brother->num - 1];

					if (brother->childs[brother->num])
						brother->childs[brother->num]->parent = pNode;
					brother->deleteKeyByIndex(brother->num - 1);

					return true;

				}
				else {
					brother->keys[brother->num] = parent->keys[index];
					parent->deleteKeyByIndex(index);
					brother->num++;

					for (int i = 0; i < pNode->num; i++) {
						brother->childs[brother->num + i] = pNode->childs[i];
						brother->keys[brother->num + i] = pNode->keys[i];
						brother->childs[brother->num + i]->parent = brother;
					}
					brother->childs[brother->num + pNode->num] = pNode->childs[pNode->num];
					brother->childs[brother->num + pNode->num]->parent = brother;

					brother->num += pNode->num;

					delete pNode;
					node_num--;

					return adjustAfterDelete(parent);
				}

			}
			else {
				if (parent->childs[0] == pNode)
					brother = parent->childs[1];
				else
					brother = parent->childs[index + 2];
				if (brother->num > minmumKeyNum - 1) {

					pNode->childs[pNode->num + 1] = brother->childs[0];
					pNode->keys[pNode->num] = brother->keys[0];
					pNode->childs[pNode->num + 1]->parent = pNode;
					pNode->num++;

					if (pNode == parent->childs[0])
						parent->keys[0] = brother->keys[0];
					else
						parent->keys[index + 1] = brother->keys[0];

					brother->childs[0] = brother->childs[1];
					brother->deleteKeyByIndex(0);

					return true;
				}
				else {

					pNode->keys[pNode->num] = parent->keys[index];

					if (pNode == parent->childs[0])
						parent->deleteKeyByIndex(0);
					else
						parent->deleteKeyByIndex(index + 1);

					pNode->num++;

					for (int i = 0; i < brother->num; i++) {
						pNode->childs[pNode->num + i] = brother->childs[i];
						pNode->keys[pNode->num + i] = brother->keys[i];
						pNode->childs[pNode->num + i]->parent = pNode;
					}
					pNode->childs[pNode->num + brother->num] = brother->childs[brother->num];
					pNode->childs[pNode->num + brother->num]->parent = pNode;

					pNode->num += brother->num;

					delete brother;
					node_num--;

					return adjustAfterDelete(parent);
				}

			}

		}

	}

	return false;
}

//���룺���ĸ����
//���ܣ�ɾ�����������ͷ��ڴ�ռ䣬��Ҫ��������������
template <class T>
void BPlusTree<T>::dropTree(Tree node)
{
	//����
	if (!node)
		return;
	//��Ҷ�ڵ�
	if (!node->isLeaf) {
		for (unsigned int i = 0; i <= node->num; i++) {
			dropTree(node->childs[i]);
			node->childs[i] = NULL;
		}
	}
	delete node;
	node_num--;
	return;
}

//���룺key1��key2������vals������
//���ܣ����ط�Χ�����������value����vals������
template <class T>
void BPlusTree<T>::searchRange(T& key1, T& key2, std::vector<int>& vals, int flag)
{
	//����
	if (!root)
		return;

	if (flag == 2) {
		searchNodeParse snp1;
		findToLeaf(root, key1, snp1);

		bool finished = false;
		Tree pNode = snp1.pNode;
		unsigned int index = snp1.index;

		do {
			finished = pNode->findRange2(index, vals);
			index = 0;
			if (pNode->nextLeafNode == NULL)
				break;
			else
				pNode = pNode->nextLeaf();
		} while (!finished);
	}
	else if (flag == 1) {
		searchNodeParse snp2;
		findToLeaf(root, key2, snp2);

		bool finished = false;
		Tree pNode = snp2.pNode;
		unsigned int index = snp2.index;

		do {
			finished = pNode->findRange2(index, vals);
			index = 0;
			if (pNode->nextLeafNode == NULL)
				break;
			else
				pNode = pNode->nextLeaf();
		} while (!finished);
	}
	else {
		searchNodeParse snp1, snp2;
		findToLeaf(root, key1, snp1);
		findToLeaf(root, key2, snp2);
		bool finished = false;
		unsigned int index;

		if (key1 <= key2) {
			Tree pNode = snp1.pNode;
			index = snp1.index;
			do {
				finished = pNode->findRange(index, key2, vals);
				index = 0;
				if (pNode->nextLeafNode == NULL)
					break;
				else
					pNode = pNode->nextLeaf();
			} while (!finished);
		}
		else {
			Tree pNode = snp2.pNode;
			index = snp2.index;
			do {
				finished = pNode->findRange(index, key1, vals);
				index = 0;
				if (pNode->nextLeafNode == NULL)
					break;
				else
					pNode = pNode->nextLeaf();
			} while (!finished);
		}
	}
	//}

	std::sort(vals.begin(), vals.end());
	vals.erase(unique(vals.begin(), vals.end()), vals.end());
	return;
}

//��ȡ�ļ���С
template <class T>
void BPlusTree<T>::getFile(std::string fname) {
	FILE* f = fopen(fname.c_str(), "r");
	if (f == NULL) {
		f = fopen(fname.c_str(), "w+");
		fclose(f);
		f = fopen(fname.c_str(), "r");
	}
	fclose(f);
	return;
}

template <class T>
int BPlusTree<T>::getBlockNum(std::string table_name)
{
	char* p;
	int block_num = -1;
	do {
		p = buffer_manager.getPage(table_name, block_num + 1);
		block_num++;
	} while (p[0] != '\0');
	return block_num;
}

template <class T>
void BPlusTree<T>::readFromDiskAll()
{
	std::string fname = ff + file_name;
	//std::string fname = file_name;
	getFile(fname);
	int block_num = getBlockNum(fname);

	if (block_num <= 0)
		block_num = 1;

	for (int i = 0; i < block_num; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(fname, i);
		//char* t = p;
		//�����������м�¼

		readFromDisk(p, p + PAGESIZE);
	}
}


template <class T>
void BPlusTree<T>::readFromDisk(char* p, char* end)
{
	T key;
	int value;

	for (int i = 0; i < PAGESIZE; i++)
		if (p[i] != '#')
			return;
		else {
			i += 2;
			char tmp[100];
			int j;

			for (j = 0; i < PAGESIZE && p[i] != ' '; i++)
				tmp[j++] = p[i];
			tmp[j] = '\0';
			std::string s(tmp);
			std::stringstream stream(s);
			stream >> key;

			memset(tmp, 0, sizeof(tmp));

			i++;
			for (j = 0; i < PAGESIZE && p[i] != ' '; i++)
				tmp[j++] = p[i];
			tmp[j] = '\0';
			std::string s1(tmp);
			std::stringstream stream1(s1);
			stream1 >> value;

			insertKey(key, value);
		}
}


template <class T>
void BPlusTree<T>::writtenbackToDiskAll()
{
	std::string fname = ff + file_name;
	//std::string fname = file_name;
	getFile(fname);
	int block_num = getBlockNum(fname);

	Tree ntmp = leafHead;
	int i, j;

	for (j = 0, i = 0; ntmp != NULL; j++) {
		char* p = buffer_manager.getPage(fname, j);
		int offset = 0;

		memset(p, 0, PAGESIZE);

		for (i = 0; i < ntmp->num; i++) {
			p[offset++] = '#';
			p[offset++] = ' ';

			copyString(p, offset, ntmp->keys[i]);
			p[offset++] = ' ';
			copyString(p, offset, ntmp->vals[i]);
			p[offset++] = ' ';
		}

		p[offset] = '\0';

		int page_id = buffer_manager.getPageId(fname, j);
		buffer_manager.modifyPage(page_id);

		ntmp = ntmp->nextLeafNode;
	}

	while (j < block_num) {
		char* p = buffer_manager.getPage(fname, j);
		memset(p, 0, PAGESIZE);

		int page_id = buffer_manager.getPageId(fname, j);
		buffer_manager.modifyPage(page_id);

		j++;
	}

	return;

}

template <class T>
void BPlusTree<T>::printleaf()
{
	Tree p = leafHead;
	while (p != NULL) {
		p->printl();
		p = p->nextLeaf();
	}

	return;
}

template <class T>
void TreeNode<T>::printl()
{
	for (int i = 0; i < num; i++)
		std::cout << "->" << keys[i];
	std::cout << std::endl;

}
#endif
