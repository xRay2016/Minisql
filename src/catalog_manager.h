#pragma once


#ifndef _CATALOG_MANAGER_H_
#define _CATALOG_MANAGER_H_ 1


#include <iostream>
#include <cmath>
#include <cstring>
#include <iomanip>
#include "buffer_manager.h"
#include "basic.h"
#include "error.h"
#include "const.h"
using namespace std;

#ifndef TABLE_MANAGER_PATH
#define TABLE_MANAGER_PATH "./src/catalog_file"
#endif

extern BufferManager buffer_manager;

class CatalogManager {
public:
	//���룺���������Զ���������ţ���������
	//�����void
	//���ܣ���catalog�ļ��в���һ�����Ԫ��Ϣ
	//�쳣������Ѿ�����ͬ�����ı���ڣ����׳�table_exist�쳣
	void createTable(std::string table_name, Attribute attribute, int primary, Index index);
	//���룺����
	//�����void
	//���ܣ���catalog�ļ���ɾ��һ�����Ԫ��Ϣ
	//�쳣����������ڣ��׳�table_not_exist�쳣
	void dropTable(std::string table_name);
	//���룺����
	//�����bool
	//���ܣ����Ҷ�Ӧ���Ƿ���ڣ����ڷ���true�������ڷ���false
	//�쳣�����쳣
	bool hasTable(std::string table_name);
	//���룺������������
	//�����bool
	//���ܣ����Ҷ�Ӧ�����Ƿ���ĳһ���ԣ�����з���true�����û�з���false
	//�쳣����������ڣ��׳�table_not_exist�쳣
	bool hasAttribute(std::string table_name, std::string attr_name);
	//���룺����
	//��������Զ���
	//���ܣ���ȡһ���������
	//�쳣����������ڣ��׳�table_not_exist�쳣
	Attribute getAttribute(std::string table_name);
	//���룺��������������������
	//�����void
	//���ܣ���catalog�ļ��и��¶�Ӧ���������Ϣ����ָ�������Ͻ���һ��������
	//�쳣����������ڣ��׳�table_not_exist�쳣�������Ӧ���Բ����ڣ��׳�attribute_not_exist�쳣��
	//�����Ӧ�����Ѿ������������׳�index_exist�쳣��
	void createIndex(std::string table_name, std::string attr_name, std::string index_name);
	//���룺������������
	//�������������Ӧ��������
	//���ܣ�ͨ����������λ������
	//�쳣����������ڣ��׳�table_not_exist�쳣�������Ӧ���������ڣ��׳�index_not_exist�쳣��
	std::string IndextoAttr(std::string table_name, std::string index_name);
	//���룺������������
	//�����void
	//���ܣ�ɾ����Ӧ��Ķ�Ӧ�����ϵ�����
	//�쳣����������ڣ��׳�table_not_exist�쳣�������Ӧ���Բ����ڣ��׳�attribute_not_exist�쳣��
	//�����Ӧ����û���������׳�index_not_exist�쳣��
	void dropIndex(std::string table_name, std::string index_name);
	//���룺����
	//�����void
	//���ܣ���ʾ�����Ϣ
	//�쳣����������ڣ��׳�table_not_exist�쳣
	void showTable(std::string table_name);
	//������б���
	std::vector<std::string> getAllTable();
private:
	//����ת�ַ�����bitΪ����λ��
	std::string num2str(int num, short bit);
	//�ַ���ת����
	int str2num(std::string str);
	//�õ����еı������
	std::string getTableName(std::string buffer, int start, int &rear);
	//���ر����ļ��е�λ��,���ؾ���λ�ã����ô����������ڵĿ���Ϣ
	int getTablePlace(std::string name, int &suitable_block);
	//���ظñ��index
	Index getIndex(std::string table_name);
	//��ȡ�ļ���С
	int getBlockNum(std::string table_name);
};

#endif /* catalog_hpp */
