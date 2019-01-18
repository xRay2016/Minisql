#pragma once

#ifndef _API_H_
#define _API_H_ 1

#include "basic.h"
#include "record_manager.h"
#include "buffer_manager.h"
#include "error.h"
using namespace std;
//API�ӿڡ���ΪInterpreter������͵ײ�Manager���ӵĽӿ�
//������������й��ܽӿ�
//APIֻ�������ĳ������жϣ�������쳣�׳����ɸ��ײ�Manager���
class API {
public:
	//���ݱ�����ñ����
	Table selectRecord1(string name);
	//���캯��
	API();
	//��������
	~API();
	//���ݱ����������
	Attribute getAttr(string table);
	//modeΪ0��ʾ��Ȼ���ӣ�Ϊ1��ʾ��ͨ���ӣ�Ϊ������ʾ�ѿ�����
	Table selectRecord(std::vector<Table> from, boolTree where,
		std::vector<std::string> gattr, boolTree having,
		vector<pair<std::string, UD>> oattr, vector<string>select, int mode);
	//��������������
	Table setOperation(Table table1, Table table2, int operation);
	//ɾ����¼
	int deleteRecord(std::string table_name, boolTree tree);
    //�����¼
	void insertRecord(std::string table_name, Tuple& tuple);
	//������
	bool createTable(std::string table_name, Attribute attribute, int primary, Index index);
	//ɾ����
	bool dropTable(std::string table_name);
	//��������
	bool createIndex(std::string table_name, std::string index_name, std::string attr_name);
	//ɾ������
	bool dropIndex(std::string table_name, std::string index_name);
	//���������ظ�Ԫ��ı�
	Table distinct(Table soureTable);
	//�޸ı��¼
	void update(std::string table_name, string attr_name, Data attr_value, boolTree where);

	//��ȡ����Ԫ����Ŀ
	int TupleNum(std::string table_name);

	//��ñ�����
	void ShowTable(Table& table);

	//������б���
	vector<string> getAllTable();


private:
	//˽�к��������ڶ�������ѯʱ��and�����ϲ�
	Table unionTable(Table &table1, Table &table2, std::string target_attr, Where where);
	//˽�к��������ڶ�������ѯʱ��or�����ϲ�
	Table joinTable(Table &table1, Table &table2, std::string target_attr, Where where);
	//��������
	Table selectRecordSingle(Table table, boolTree tree);

	RecordManager* record;
	CatalogManager* catalog;
};

//���ڶ�vector��sortʱ����
bool sortcmp(const Tuple &tuple1, const Tuple &tuple2);
//���ڶ�vector�Ժϲ�ʱ������
bool calcmp(const Tuple &tuple1, const Tuple &tuple2);

bool isSatisfied(Tuple& tuple, int target_attr, Where where);
//�ж�Ԫ���Ƿ��ڱ���
bool isInTable(Table table, Tuple t);
//�ж�Ԫ���Ƿ����
bool isEqual(vector<Data>data1, vector<Data>data2);
//���������һ��������λ��
int getBits(int num);
//���������һ����������λ��������С�����4λ��
int getBits(float num);
#endif
