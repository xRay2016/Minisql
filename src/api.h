#pragma once

#ifndef _API_H_
#define _API_H_ 1

#include "basic.h"
#include "record_manager.h"
#include "buffer_manager.h"
#include "error.h"
using namespace std;
//API接口。作为Interpreter层操作和底层Manager连接的接口
//包含程序的所有功能接口
//API只做初步的程序功能判断，具体的异常抛出仍由各底层Manager完成
class API {
public:
	//根据表名获得表对象
	Table selectRecord1(string name);
	//构造函数
	API();
	//析构函数
	~API();
	//根据表名获得属性
	Attribute getAttr(string table);
	//mode为0表示自然连接，为1表示普通连接，为其他表示笛卡尔积
	Table selectRecord(std::vector<Table> from, boolTree where,
		std::vector<std::string> gattr, boolTree having,
		vector<pair<std::string, UD>> oattr, vector<string>select, int mode);
	//交，并，差运算
	Table setOperation(Table table1, Table table2, int operation);
	//删除记录
	int deleteRecord(std::string table_name, boolTree tree);
    //插入记录
	void insertRecord(std::string table_name, Tuple& tuple);
	//创建表
	bool createTable(std::string table_name, Attribute attribute, int primary, Index index);
	//删除表
	bool dropTable(std::string table_name);
	//创建索引
	bool createIndex(std::string table_name, std::string index_name, std::string attr_name);
	//删除索引
	bool dropIndex(std::string table_name, std::string index_name);
	//消除具有重复元组的表
	Table distinct(Table soureTable);
	//修改表记录
	void update(std::string table_name, string attr_name, Data attr_value, boolTree where);

	//获取表中元组数目
	int TupleNum(std::string table_name);

	//获得表的输出
	void ShowTable(Table& table);

	//获得所有表名
	vector<string> getAllTable();


private:
	//私有函数，用于多条件查询时的and条件合并
	Table unionTable(Table &table1, Table &table2, std::string target_attr, Where where);
	//私有函数，用于多条件查询时的or条件合并
	Table joinTable(Table &table1, Table &table2, std::string target_attr, Where where);
	//单表连接
	Table selectRecordSingle(Table table, boolTree tree);

	RecordManager* record;
	CatalogManager* catalog;
};

//用于对vector的sort时排序
bool sortcmp(const Tuple &tuple1, const Tuple &tuple2);
//用于对vector对合并时对排序
bool calcmp(const Tuple &tuple1, const Tuple &tuple2);

bool isSatisfied(Tuple& tuple, int target_attr, Where where);
//判断元组是否在表中
bool isInTable(Table table, Tuple t);
//判断元组是否相等
bool isEqual(vector<Data>data1, vector<Data>data2);
//输出：返回一个整数的位数
int getBits(int num);
//输出：返回一个浮点数的位数（保留小数点后4位）
int getBits(float num);
#endif
