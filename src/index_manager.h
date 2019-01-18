

#ifndef _INDEX_MANAGER_H_
#define _INDEX_MANAGER_H_ 1

#include <sstream>
#include <string>
#include <map>
#include "basic.h"
#include "buffer_manager.h"
#include "bplustree.h"
using namespace std;

class IndexManager {
public:
	//���캯��
	IndexManager(std::string table_name);
	//��������
	~IndexManager();
	//���룺Index�ļ���(·��)��������key������
	//�����void
	//���ܣ����������ļ���B+��
	//�쳣��
	void createIndex(std::string file_path, int type);
	//���룺Index�ļ���(·��)��������key������
	//�����void
	//���ܣ�ɾ��������B+�����ļ�
	//�쳣��
	void dropIndex(std::string file_path, int type);
	//���룺Index�ļ���(·��)��������key(��������)
	//�����void
	//���ܣ����������ļ���B+��
	//�쳣��
	int findIndex(std::string file_path, Data data);
	//���룺Index�ļ���(·��)��������key(��������)��block_id
	//�����void
	//���ܣ���ָ�������в���һ��key
	//�쳣��
	void insertIndex(std::string file_path, Data data, int block_id);
	//���룺Index�ļ���(·��)��������key(��������)
	//�����void
	//���ܣ���������ɾ����Ӧ��Key
	//�쳣��
	void deleteIndexByKey(std::string file_path, Data data);
	//���룺Index�ļ���(·��)��������key1(��������)��������key2(��������)�����ص�vector����
	//�����void
	//���ܣ���Χ���ң�����һ����Χ�ڵ�value
	//�쳣��
	void searchRange(std::string file_path, Data data1, Data data2, std::vector<int>& vals);

private:
	typedef std::map<std::string, BPlusTree<int> *> intMap;
	typedef std::map<std::string, BPlusTree<std::string> *> stringMap;
	typedef std::map<std::string, BPlusTree<float> *> floatMap;

	int static const TYPE_FLOAT = 0;
	int static const TYPE_INT = -1;

	intMap indexIntMap;
	stringMap indexStringMap;
	floatMap indexFloatMap;

	// struct keyTmp{
	// 	int intTmp;
	// 	float floatTmp;
	// 	string stringTmp;
	// };
	// struct keyTmp kt;

	//����B+���ʺϵ�degree
	int getDegree(int type);

	//���㲻ͬ����Key��size
	int getKeySize(int type);

	//void setKey(int type, std::string key);
};

#endif
#pragma once
