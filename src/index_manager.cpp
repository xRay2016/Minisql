

#include "index_manager.h"
#include "const.h"
#include "basic.h"
#include "buffer_manager.h"
#include "bplustree.h"
#include "catalog_manager.h"
#include <string>
#include <vector>
#include <map>

IndexManager::IndexManager(std::string table_name)
{
	CatalogManager catalog;
	Attribute attr = catalog.getAttribute(table_name);

	for (int i = 0; i < attr.num; i++)
		if (attr.has_index[i])
			createIndex("INDEX_FILE_" + attr.name[i] + "_" + table_name, attr.type[i]);
}

IndexManager::~IndexManager()
{
	for (intMap::iterator itInt = indexIntMap.begin(); itInt != indexIntMap.end(); itInt++) {
		if (itInt->second) {
			itInt->second->writtenbackToDiskAll();
			delete itInt->second;
		}
	}
	for (stringMap::iterator itString = indexStringMap.begin(); itString != indexStringMap.end(); itString++) {
		if (itString->second) {
			itString->second->writtenbackToDiskAll();
			delete itString->second;
		}
	}
	for (floatMap::iterator itFloat = indexFloatMap.begin(); itFloat != indexFloatMap.end(); itFloat++) {
		if (itFloat->second) {
			itFloat->second->writtenbackToDiskAll();
			delete itFloat->second;
		}
	}
}

void IndexManager::createIndex(std::string file_path, int type)
{
	int key_size = getKeySize(type); //��ȡkey��size
	int degree = getDegree(type); //��ȡ��Ҫ��degree

								  //�����������Ͳ�ͬ���ö�Ӧ�ķ�������ӳ���ϵ
								  //�����ȳ�ʼ��һ��B+��
	if (type == TYPE_INT) {
		BPlusTree<int> *tree = new BPlusTree<int>(file_path, key_size, degree);
		indexIntMap.insert(intMap::value_type(file_path, tree));
	}
	else if (type == TYPE_FLOAT) {
		BPlusTree<float> *tree = new BPlusTree<float>(file_path, key_size, degree);
		indexFloatMap.insert(floatMap::value_type(file_path, tree));
	}
	else {
		BPlusTree<std::string> *tree = new BPlusTree<std::string>(file_path, key_size, degree);
		indexStringMap.insert(stringMap::value_type(file_path, tree));
	}

	return;
}

void IndexManager::dropIndex(std::string file_path, int type)
{
	//���ݲ�ͬ�������Ͳ��ö�Ӧ�Ĵ���ʽ
	if (type == TYPE_INT) {
		//����·����Ӧ�ļ�ֵ��
		intMap::iterator itInt = indexIntMap.find(file_path);
		if (itInt == indexIntMap.end()) { //δ�ҵ�
										  // cout << "Error:in drop index, no index " << file_path <<" exits" << endl;
			return;
		}
		else {
			//ɾ����Ӧ��B+��
			delete itInt->second;
			//��ոü�ֵ��
			indexIntMap.erase(itInt);
		}
	}
	else if (type == TYPE_FLOAT) { //ͬ��
		floatMap::iterator itFloat = indexFloatMap.find(file_path);
		if (itFloat == indexFloatMap.end()) {
			// cout << "Error:in drop index, no index " << file_path <<" exits" << endl;
			return;
		}
		else {
			delete itFloat->second;
			indexFloatMap.erase(itFloat);
		}
	}
	else {
		stringMap::iterator itString = indexStringMap.find(file_path);
		if (itString == indexStringMap.end()) { //ͬ��
												// cout << "Error:in drop index, no index " << file_path <<" exits" << endl;
			return;
		}
		else {
			delete itString->second;
			indexStringMap.erase(itString);
		}
	}

	return;
}

int IndexManager::findIndex(std::string file_path, Data data)
{
	//setKey(type, key);

	if (data.type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(file_path);
		if (itInt == indexIntMap.end()) { //δ�ҵ�
										  // cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return -1;
		}
		else
			//�ҵ��򷵻ض�Ӧ�ļ�ֵ
			return itInt->second->searchVal(data.datai);
	}
	else if (data.type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(file_path);
		if (itFloat == indexFloatMap.end()) { //ͬ��
											  // cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return -1;
		}
		else
			return itFloat->second->searchVal(data.dataf);
	}
	else {
		stringMap::iterator itString = indexStringMap.find(file_path);
		if (itString == indexStringMap.end()) { //ͬ��
												// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return -1;
		}
		else
			return itString->second->searchVal(data.datas);
	}
}

void IndexManager::insertIndex(std::string file_path, Data data, int block_id)
{
	//setKey(type, key);

	if (data.type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(file_path);
		if (itInt == indexIntMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itInt->second->insertKey(data.datai, block_id);
	}
	else if (data.type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(file_path);
		if (itFloat == indexFloatMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itFloat->second->insertKey(data.dataf, block_id);
	}
	else {
		stringMap::iterator itString = indexStringMap.find(file_path);
		if (itString == indexStringMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itString->second->insertKey(data.datas, block_id);
	}

	return;
}

void IndexManager::deleteIndexByKey(std::string file_path, Data data)
{
	//setKey(type, key);

	if (data.type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(file_path);
		if (itInt == indexIntMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itInt->second->deleteKey(data.datai);
	}
	else if (data.type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(file_path);
		if (itFloat == indexFloatMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itFloat->second->deleteKey(data.dataf);
	}
	else {
		stringMap::iterator itString = indexStringMap.find(file_path);
		if (itString == indexStringMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itString->second->deleteKey(data.datas);
	}
}

int IndexManager::getDegree(int type)
{
	int degree = (PAGESIZE - sizeof(int)) / (getKeySize(type) + sizeof(int));
	if (degree % 2 == 0)
		degree -= 1;
	return degree;
}

int IndexManager::getKeySize(int type)
{
	if (type == TYPE_FLOAT)
		return sizeof(float);
	else if (type == TYPE_INT)
		return sizeof(int);
	else if (type > 0)
		return type;
	else {
		// cout << "ERROR: in getKeySize: invalid type" << endl;
		return -100;
	}
}

void IndexManager::searchRange(std::string file_path, Data data1, Data data2, std::vector<int>& vals)
{
	int flag = 0;
	//������������Ƿ�ƥ��
	if (data1.type == -2) {
		flag = 1;
	}
	else if (data2.type == -2) {
		flag = 2;
	}
	/*
	else if (data1.type != data2.type) {
	// cout << "ERROR: in searchRange: Wrong data type!" << endl;
	return;
	}
	*/

	if (data1.type == TYPE_INT) {
		intMap::iterator itInt = indexIntMap.find(file_path);
		if (itInt == indexIntMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itInt->second->searchRange(data1.datai, data2.datai, vals, flag);
	}
	else if (data1.type == TYPE_FLOAT) {
		floatMap::iterator itFloat = indexFloatMap.find(file_path);
		if (itFloat == indexFloatMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itFloat->second->searchRange(data1.dataf, data2.dataf, vals, flag);
	}
	else {
		stringMap::iterator itString = indexStringMap.find(file_path);
		if (itString == indexStringMap.end()) {
			// cout << "Error:in search index, no index " << file_path <<" exits" << endl;
			return;
		}
		else
			itString->second->searchRange(data1.datas, data2.datas, vals, flag);
	}
}

/*
void IndexManager::setKey(int type, std::tring key)
{
stringstream ss;
ss << key;
if (type == this->TYPE_INT)
ss >> this->kt.intTmp;
else if (type == this->TYPE_FLOAT)
ss >> this->kt.floatTmp;
else if (type > 0)
ss >> this->kt.stringTmp;

else
cout << "Error: in getKey: invalid type" << endl;


return;
}
*/
