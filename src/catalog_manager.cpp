
#define _CRT_SECURE_NO_WARNINGS 
#include "catalog_manager.h"


void CatalogManager::createTable(std::string name, Attribute Attr, int primary, Index index) {
	//����Ѵ��ڸñ����쳣
	if (hasTable(name)) {
		throw table_exist();
	}
	//��������ȷ������Ϊunique
	if (primary >= 0)
		Attr.unique[primary] = true;
	//��¼ÿ����Ϣ���ַ��������������5����
	std::string str_tmp = "0000 ";
	//���name
	str_tmp += name;
	//���attribute������
	str_tmp = str_tmp + " " + num2str(Attr.num, 2);
	//���û��attribute����Ϣ��˳��Ϊ���ͣ����֣��Ƿ�ΪΨһ
	for (int i = 0; i < Attr.num; i++)
		str_tmp = str_tmp + " " + num2str(Attr.type[i], 3) + " " + Attr.name[i] + " " + (Attr.unique[i] == true ? "1" : "0");
	//���������Ϣ
	str_tmp = str_tmp + " " + num2str(primary, 2);
	//���index������, ;���������index�Ŀ�ʼ
	str_tmp = str_tmp + " ;" + num2str(index.num, 2);
	//���index����Ϣ��˳��Ϊ���λ�ú�����
	for (int i = 0; i < index.num; i++)
		str_tmp = str_tmp + " " + num2str(index.location[i], 2) + " " + index.indexname[i];
	//���к��ڽ�β����һ��#��ÿ������#��β
	str_tmp = str_tmp + "\n" + "#";
	//����ÿ����Ϣ�ĳ��ȵļ�¼
	std::string str_len = num2str((int)str_tmp.length() - 1, 4);
	str_tmp = str_len + str_tmp.substr(4, str_tmp.length() - 4);
	//����������
	/*int block_num=getBlockNum(TABLE_MANAGER_PATH)/PAGESIZE;*/
	int block_num = getBlockNum(TABLE_MANAGER_PATH);
	//�����������Ϊ0���������
	if (block_num <= 0)
		block_num = 1;
	//�������еĿ�
	for (int current_block = 0; current_block < block_num; current_block++) {
		char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, current_block);
		int page_id = buffer_manager.getPageId(TABLE_MANAGER_PATH, current_block);
		//Ѱ�Ҹ�block����Ч����
		int length = 0;
		for (length = 0; length < PAGESIZE&&buffer[length] != '\0'&&buffer[length] != '#'; length++) {}
		//ȷ����������Ϣ���ҳ���Ȳ��ᳬ��PAGESIZE
		if (length + (int)str_tmp.length() < PAGESIZE) {
			//ɾ��#
			if (length&&buffer[length - 1] == '#')
				buffer[length - 1] = '\0';
			else if (buffer[length] == '#')
				buffer[length] = '\0';
			//�ַ���ƴ��
			strcat(buffer, str_tmp.c_str());
			//���沢ˢ�¸�ҳ�󷵻�
			buffer_manager.modifyPage(page_id);
			buffer_manager.flushPage(page_id, "./src/catalog_file", block_num - 1);
			return;
		}
	}
	//���֮ǰ�Ŀ鲻���ã����½�һ���ֱ�Ӱ���Ϣ����
	char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, block_num);
	int page_id = buffer_manager.getPageId(TABLE_MANAGER_PATH, block_num);
	strcat(buffer, str_tmp.c_str());
	buffer_manager.modifyPage(page_id);

}

void CatalogManager::dropTable(std::string name) {
	//��������ڸñ����쳣
	if (!hasTable(name)) {
		throw table_not_exist();
	}
	//Ѱ��table_name����Ӧ�Ŀ�ź��ڸÿ��λ��
	int suitable_block;
	int start_index = getTablePlace(name, suitable_block);
	//�õ�����Ӧ�����Ϣ
	char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, suitable_block);
	int page_id = buffer_manager.getPageId(TABLE_MANAGER_PATH, suitable_block);
	std::string buffer_check(buffer);
	//���Ӧɾ���Ŀ��index�Ŀ�ʼ�ͽ�β��ɾ��
	int end_index = start_index + str2num(buffer_check.substr(start_index, 4));
	int index = 0, current_index = 0;;
	do {
		if (index < start_index || index >= end_index)
			buffer[current_index++] = buffer[index];
		index++;
	} while (buffer[index] != '#');
	buffer[current_index++] = '#';
	buffer[current_index] = '\0';
	//ˢ��ҳ��
	buffer_manager.modifyPage(page_id);
	buffer_manager.flushPage(page_id,"./src/catalog_file", suitable_block);
	int block_num = getBlockNum("./src/" + name);
	for (int i = 0; i < block_num; i++)
	{
		int page_id = buffer_manager.getPageId("./src/" + name, i);
		buffer_manager.Frames[page_id].initialize();
	}
}

Attribute CatalogManager::getAttribute(std::string name) {
	//��������ڸñ����쳣
	if (!hasTable(name)) {
		throw attribute_not_exist();
	}
	//Ѱ��table_name��Ӧ�Ŀ���ڿ��е�λ��
	int suitable_block;
	int start_index = getTablePlace(name, suitable_block);
	//�õ�����Ӧ�Ŀ����Ϣ
	char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, suitable_block);
	std::string buffer_check(buffer);
	//end_index��¼�����б�����ֵ����һ���ַ���λ��
	int end_index = 0;
	std::string table_name = getTableName(buffer_check, start_index, end_index);
	Attribute table_attr;
	start_index = end_index + 1;
	//�õ�attribute�������ַ���
	std::string attr_num = buffer_check.substr(start_index, 2);
	table_attr.num = str2num(attr_num);
	start_index += 3;
	for (int index = 0; index < table_attr.num; index++) {
		//�����е�attribute�����ͺ�����
		if (buffer_check[start_index] == '-') {
			table_attr.type[index] = -1;
			start_index += 5;
			while (buffer_check[start_index] != ' ') {
				table_attr.name[index] += buffer_check[start_index++];
			}
			start_index += 1;
			table_attr.unique[index] = (buffer_check[start_index] == '1' ? true : false);
		}
		else if (str2num(buffer_check.substr(start_index, 3)) == 0) {
			table_attr.type[index] = 0;
			start_index += 4;
			while (buffer_check[start_index] != ' ') {
				table_attr.name[index] += buffer_check[start_index++];
			}
			start_index += 1;
			table_attr.unique[index] = (buffer_check[start_index] == '1' ? true : false);
		}
		else {
			table_attr.type[index] = str2num(buffer_check.substr(start_index, 3));
			start_index += 4;
			while (buffer_check[start_index] != ' ') {
				table_attr.name[index] += buffer_check[start_index++];
			}
			start_index += 1;
			table_attr.unique[index] = (buffer_check[start_index] == '1' ? true : false);
		}
		start_index += 2;
	}
	//��¼primary_key����Ϣ
	if (buffer_check[start_index] == '-')
		table_attr.primary_key = -1;
	else
		table_attr.primary_key = str2num(buffer_check.substr(start_index, 2));
	//����index����Ϣ
	Index index_record = getIndex(table_name);
	for (int i = 0; i < 32; i++)
		table_attr.has_index[i] = false;
	for (int i = 0; i < index_record.num; i++)
		table_attr.has_index[index_record.location[i]] = true;

	return table_attr;
}

//�ж������е�attribute���Ƿ�������Ե�����
bool CatalogManager::hasAttribute(std::string table_name, std::string attr_name) {
	//��������ڸñ����쳣
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	//�õ�attribute����Ϣ�󣬱������е�����
	Attribute find_attr = getAttribute(table_name);
	for (int index = 0; index < find_attr.num; index++) {
		if (attr_name == find_attr.name[index])
			return true;
	}
	return false;
}

//����index_name����attr_name
std::string CatalogManager::IndextoAttr(std::string table_name, std::string index_name) {
	if (!hasTable(table_name))
		throw table_not_exist();
	Index index_record = getIndex(table_name);
	int hasfind = -1;
	for (int i = 0; i < index_record.num; i++) {
		if (index_record.indexname[i] == index_name) {
			hasfind = i;
			break;
		}
	}
	if (hasfind == -1)
		throw index_not_exist();
	Attribute attr_record = getAttribute(table_name);
	return attr_record.name[index_record.location[hasfind]];
}

void CatalogManager::createIndex(std::string table_name, std::string attr_name, std::string index_name) {
	//��������ڸñ����쳣
	if (!hasTable(table_name))
		throw table_not_exist();
	//��������ڸ����ԣ����쳣
	if (!hasAttribute(table_name, attr_name))
		throw attribute_not_exist();
	//�õ��ñ��index����Ϣ
	Index index_record = getIndex(table_name);
	//���index�������Ѿ����ڵ���10�����쳣
	if (index_record.num >= 10)
		throw index_full();
	//�õ����Ե���Ϣ
	Attribute find_attr = getAttribute(table_name);
	//�����������е�index
	for (int i = 0; i < index_record.num; i++) {
		//�������ظ�����
		if (index_record.indexname[i] == index_name)
			throw index_exist();
		//�����ظ�
		if (find_attr.name[index_record.location[i]] == attr_name)
			throw index_exist();
	}
	//�������
	//�������������
	index_record.indexname[index_record.num] = index_name;
	//���������attribute��λ��
	for (int index = 0; index < find_attr.num; index++) {
		if (attr_name == find_attr.name[index])
		{
			index_record.location[index_record.num] = index;
			break;
		}
	}
	//��������һ
	index_record.num++;
	//��ԭ�б���ɾ���ñ���ٲ��룬ʵ��ˢ��
	dropTable(table_name);
	createTable(table_name, find_attr, find_attr.primary_key, index_record);
}

void CatalogManager::dropIndex(std::string table_name, std::string index_name) {
	//��������ڸ����������쳣
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}
	//�õ��ñ��index����Ϣ
	Index index_record = getIndex(table_name);
	//�õ����Ե���Ϣ
	Attribute attr_record = getAttribute(table_name);
	//�������е�index�������Ƿ��ж�Ӧ���ֵ�������������������쳣
	int hasindex = -1;
	for (int index = 0; index < index_record.num; index++) {
		if (index_record.indexname[index] == index_name) {
			hasindex = index;
			break;
		}
	}
	if (hasindex == -1) {
		throw index_not_exist();
	}
	//ͨ��������Ϣ�����λ�õ������滻�ķ�ʽ��ɾ������
	index_record.indexname[hasindex] = index_record.indexname[index_record.num - 1];
	index_record.location[hasindex] = index_record.location[index_record.num - 1];
	index_record.num--;
	//��ԭ�б���ɾ���ñ���ٲ��룬ʵ��ˢ��
	dropTable(table_name);
	createTable(table_name, attr_record, attr_record.primary_key, index_record);

}

void CatalogManager::showTable(std::string table_name) {
	//��������ڸñ����쳣
	if (!hasTable(table_name)) {
		throw table_not_exist();
	}

	//��ӡ�������
	std::cout << "Table name:" << table_name << std::endl;
	Attribute attr_record = getAttribute(table_name);
	Index index_record = getIndex(table_name);
	//Ѱ�����index_name����Ϣ����֮��Ĵ�ӡ����л��õ�
	int longest = -1;
	for (int index = 0; index < attr_record.num; index++) {
		if ((int)attr_record.name[index].length() > longest)
			longest = (int)attr_record.name[index].length();
	}
	//��ӡ����
	std::string type;
	std::cout << "Attribute:" << std::endl;
	std::cout << "Num|" << "Name" << std::setw(longest + 2) << "|Type" << type << std::setw(6) << "|" << "Unique|Primary Key" << std::endl;
	for (int index_out = 0; index_out < longest + 35; index_out++)
		std::cout << "-";
	std::cout << std::endl;
	for (int index = 0; index < attr_record.num; index++) {
		switch (attr_record.type[index]) {
		case -1:
			type = "int";
			break;
		case 0:
			type = "float";
			break;
		default:
			type = "char(" + num2str(attr_record.type[index] - 1, 3) + ")";
			break;
		}
		std::cout << index << std::setw(3 - index / 10) << "|" << attr_record.name[index] << std::setw(longest - (int)attr_record.name[index].length() + 2) << "|" << type << std::setw(10 - (int)type.length()) << "|";
		if (attr_record.unique[index])
			std::cout << "unique" << "|";
		else
			std::cout << std::setw(7) << "|";
		if (attr_record.primary_key == index)
			std::cout << "primary key";
		std::cout << std::endl;
	}

	for (int index_out = 0; index_out < longest + 35; index_out++)
		std::cout << "-";

	std::cout << std::endl;

	//��ӡ����
	std::cout << "Index:" << std::endl;
	std::cout << "Num|Location|Name" << std::endl;
	longest = -1;
	for (int index_out = 0; index_out < index_record.num; index_out++) {
		if ((int)index_record.indexname[index_out].length() > longest)
			longest = (int)index_record.indexname[index_out].length();
	}
	for (int index_out = 0; index_out < ((longest + 14) > 18 ? (longest + 14) : 18); index_out++)
		std::cout << "-";
	std::cout << std::endl;
	for (int index_out = 0; index_out < index_record.num; index_out++) {
		std::cout << index_out << std::setw(3 - index_out / 10) << "|" << index_record.location[index_out] << std::setw(8 - index_record.location[index_out] / 10) << "|" << index_record.indexname[index_out] << std::endl;
	}
	for (int index_out = 0; index_out < ((longest + 14) > 18 ? (longest + 14) : 18); index_out++)
		std::cout << "-";
	std::cout << std::endl << std::endl;
}

//�ж��Ƿ����������ı��
bool CatalogManager::hasTable(std::string table_name) {
	//����������
	/* int block_num=getBlockNum(TABLE_MANAGER_PATH)/PAGESIZE;*/
	int block_num = getBlockNum(TABLE_MANAGER_PATH);
	if (block_num <= 0)
		block_num = 1;
	//�������еĿ�
	for (int current_block = 0; current_block < block_num; current_block++) {
		char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, current_block);
		std::string buffer_check(buffer);
		std::string str_tmp = "";
		int start_index = 0, end_index = 0;
		do {
			//���һ��ʼ����#��������һ��
			if (buffer_check[0] == '#')
				break;
			//�õ�table�����֣�����������������ͬ����return true
			else if (getTableName(buffer, start_index, end_index) == table_name) {
				return true;
			}
			else {
				//ͨ���ַ�������������ȷ����һ��table��λ��
				start_index += str2num(buffer_check.substr(start_index, 4));
				//�ų����ĵ�����������
				if (!start_index)
					break;
			}
		} while (buffer_check[start_index] != '#');  //�ж��Ƿ�ͷ
	}
	return false;
}

std::vector<std::string> CatalogManager::getAllTable() {
	int block_num = getBlockNum(TABLE_MANAGER_PATH);
	if (block_num <= 0)
		block_num = 1;
	vector<string> allTable;
	//�������еĿ�
	for (int current_block = 0; current_block < block_num; current_block++) {
		char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, current_block);
		std::string buffer_check(buffer);
		std::string str_tmp = "";
		int start_index = 0, end_index = 0;
		do {
			//���һ��ʼ����#��������һ��
			if (buffer_check[0] == '#')
				break;

			else {
				allTable.push_back(getTableName(buffer, start_index, end_index));
				//ͨ���ַ�������������ȷ����һ��table��λ��
				start_index += str2num(buffer_check.substr(start_index, 4));
				//�ų����ĵ�����������
				if (!start_index)
					break;
			}
		} while (buffer_check[start_index] != '#');  //�ж��Ƿ�ͷ
	}
	return allTable;
}

//����ת�ַ�����bitΪ���ֵ�λ��
std::string CatalogManager::num2str(int num, short bit) {
	std::string str = "";
	if (num < 0) {
		num = -num;
		str += "-";
	}
	int divisor = pow(10, bit - 1);
	for (int i = 0; i < bit; i++) {
		str += (num / divisor % 10 + '0');
		divisor /= 10;
	}
	return str;
}

//�ַ���ת����
int CatalogManager::str2num(std::string str) {
	return atoi(str.c_str());
}

//�õ����еı������
std::string CatalogManager::getTableName(std::string buffer, int start, int &rear) {
	std::string str_tmp = "";
	rear = 0;
	if (buffer == "")
		return buffer;
	while (buffer[start + rear + 5] != ' ') {
		rear++;
	}
	str_tmp = buffer.substr(start + 5, rear);
	rear = start + 5 + rear;
	return str_tmp;
}

//�õ��ñ��λ�ã����ô����ñ�����ڵĿ��λ�ã������ڿ��е�λ�ã����δ�ҵ����򷵻�-1
int CatalogManager::getTablePlace(std::string name, int &suitable_block) {
	int block_num = getBlockNum(TABLE_MANAGER_PATH);
	if (block_num <= 0)
		block_num = 1;
	//�������еĿ�
	for (suitable_block = 0; suitable_block < block_num; suitable_block++) {
		char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, suitable_block);
		std::string buffer_check(buffer);
		std::string str_tmp = "";
		int start = 0, rear = 0;
		do {
			//���һ��ʼ����#��������һ��
			if (buffer_check[0] == '#')
				break;
			if (getTableName(buffer, start, rear) == name) {
				return start;
			}
			else {
				//ͨ���ַ�������������ȷ��start
				start += str2num(buffer_check.substr(start, 4));
				if (!start)
					break;
			}
		} while (buffer_check[start] != '#');  //�ж��Ƿ�ͷ
	}
	return -1;
}

Index CatalogManager::getIndex(std::string table_name) {
	Index index_record;
	//�õ��ñ��λ�úͶ�Ӧ�Ŀ�
	int suitable_block;
	int start_index = getTablePlace(table_name, suitable_block);
	char* buffer = buffer_manager.getPage(TABLE_MANAGER_PATH, suitable_block);
	//��start_index����������Ϣ��λ��
	std::string buffer_check(buffer);
	while (buffer_check[start_index] != ';')
		start_index++;
	start_index++;
	//�õ�����������
	index_record.num = str2num(buffer_check.substr(start_index, 2));
	//�õ�������������Ϣ
	for (int times = 0; times < index_record.num; times++) {
		start_index += 3;
		index_record.location[times] = str2num(buffer_check.substr(start_index, 2));
		start_index += 3;
		while (buffer_check[start_index] != ' '&&buffer_check[start_index] != '#'&&buffer_check[start_index] != '\n') {
			index_record.indexname[times] += buffer_check[start_index++];
		}
		start_index -= 2;
	}
	return index_record;
}

//��ȡ�ļ���С
int CatalogManager::getBlockNum(std::string table_name) {
	char* p;
	int block_num = -1;
	do {
		p = buffer_manager.getPage(table_name, block_num + 1);
		block_num++;
	} while (p[0] != '\0');
	return block_num;
}
