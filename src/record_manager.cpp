
#include "record_manager.h"

//���룺����
//�����void
//���ܣ��������ļ�
//�쳣�����쳣������catalog manager����
void RecordManager::createTableFile(std::string table_name) {
	table_name = PATHH + table_name;
	FILE* f = fopen(table_name.c_str(), "w");
	fclose(f);
}

//���룺����
//�����void
//���ܣ�ɾ�����ļ�
//�쳣�����쳣������catalog manager����
void RecordManager::dropTableFile(std::string table_name) {
	table_name = PATHH + table_name;
	remove(table_name.c_str());
}

//���룺������һ��Ԫ��
//�����void
//���ܣ����Ӧ���в���һ����¼
//�쳣�����Ԫ�����Ͳ�ƥ�䣬�׳�tuple_type_conflict�쳣�����
//������ͻ���׳�primary_key_conflict�쳣�����unique���Գ�ͻ��
//�׳�unique_conflict�쳣����������ڣ��׳�table_not_exist�쳣��
void RecordManager::insertRecord(std::string table_name, Tuple& tuple) {
	std::string tmp_name = table_name;
	table_name = PATHH + table_name;
	CatalogManager catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.hasTable(tmp_name)) {
		throw table_not_exist();
	}
	Attribute attr = catalog_manager.getAttribute(tmp_name);
	std::vector<Data> v = tuple.getData();
	//�������Ԫ��ĸ��������Ƿ�Ϸ�
	for (int i = 0; i < v.size(); i++) {
		if (v[i].type != attr.type[i])
			throw tuple_type_conflict();
	}
	Table table = selectRecord(tmp_name,tmp_name);
	std::vector<Tuple>& tuples = table.getTuple();
	//����Ƿ����������ͻ
	if (attr.primary_key >= 0) {
		if (isConflict(tuples, v, attr.primary_key) == true)
			throw primary_key_conflict();
	}
	//����Ƿ����unqiue��ͻ
	for (int i = 0; i < attr.num; i++) {
		if (attr.unique[i] == true) {
			if (isConflict(tuples, v, i) == true)
				throw unique_conflict();
		}
	}

	//�쳣������

	//��ȡ����ռ�Ŀ������
	// int block_num = getFileSize(table_name) / PAGESIZE;
	// ��Ϊ
	int block_num = getBlockNum(table_name);
	//������ļ���СΪ0���������
	if (block_num <= 0)
		block_num = 1;
	//��ȡ������һ��ľ��
	char* p = buffer_manager.getPage(table_name, block_num - 1);
	int i;
	//Ѱ�ҵ�һ����λ
	for (i = 0; p[i] != '\0' && i < PAGESIZE; i++);
	int j;
	int len = 0;
	//��������tuple�ĳ���
	for (j = 0; j < v.size(); j++) {
		Data d = v[j];
		switch (d.type) {
		case -1: {
			int t = getDataLength(d.datai);
			len += t;
		}; break;
		case 0: {
			float t = getDataLength(d.dataf);
			len += t;
		}; break;
		default: {
			len += d.datas.length();
		};
		}
	}
	len += v.size() + 7;
	int block_offset;//���ռ�¼������Ŀ�ı��
					 //���ʣ��Ŀռ��㹻�����tuple
	if (PAGESIZE - i >= len) {
		block_offset = block_num - 1;
		//�����Ԫ��
		insertRecord1(p, i, len, v);
		//д�ر��ļ�
		int page_id = buffer_manager.getPageId(table_name, block_num - 1);
		// buffer_manager.flushPage(page_id , table_name , block_num - 1);
		// ��Ϊ
		//buffer_manager.modifyPage(page_id);
	}
	//���ʣ��Ŀռ䲻��
	else {
		block_offset = block_num;
		//����һ����
		char* p = buffer_manager.getPage(table_name, block_num);
		//�������Ŀ��в����Ԫ��
		insertRecord1(p, 0, len, v);
		//д�ر��ļ�
		int page_id = buffer_manager.getPageId(table_name, block_num);
		// buffer_manager.flushPage(page_id , table_name , block_num);
		// ��Ϊ
		buffer_manager.modifyPage(page_id);
	}

	//��������
	IndexManager index_manager(tmp_name);
	for (int i = 0; i < attr.num; i++) {
		if (attr.has_index[i] == true) {
			std::string attr_name = attr.name[i];
			std::string file_path = "INDEX_FILE_" + attr_name + "_" + tmp_name;
			std::vector<Data> d = tuple.getData();
			index_manager.insertIndex(file_path, d[i], block_offset);

		}
	}
}

// ���룺����
// �����int(ɾ���ļ�¼��)
// ���ܣ�ɾ����Ӧ�������м�¼����ɾ�����ļ���
// �쳣����������ڣ��׳�table_not_exist�쳣
int RecordManager::deleteRecord(std::string table_name) {
	std::string tmp_name = table_name;
	table_name = PATHH + table_name;
	CatalogManager catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.hasTable(tmp_name)) {
		throw table_not_exist();
	}
	//��ȡ�ļ���ռ�������
	// int block_num = getFileSize(table_name) / PAGESIZE;
	// ��Ϊ
	int block_num = getBlockNum(table_name);
	//���ļ���СΪ0ʱֱ�ӷ���
	if (block_num <= 0)
		return 0;
	Attribute attr = catalog_manager.getAttribute(tmp_name);
	IndexManager index_manager(tmp_name);
	int count = 0;
	//�������п�
	for (int i = 0; i < block_num; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(table_name, i);
		char* t = p;
		//�����е�ÿһ��Ԫ���¼����Ϊ��ɾ��
		while (*p != '\0' && p < t + PAGESIZE) {
			//��������
			Tuple tuple = readTuple(p, attr);
			for (int j = 0; j < attr.num; j++) {
				if (attr.has_index[j] == true) {
					std::string attr_name = attr.name[i];
					std::string file_path = "INDEX_FILE_" + attr_name + "_" + tmp_name;
					std::vector<Data> d = tuple.getData();
					index_manager.deleteIndexByKey(file_path, d[j]);
				}
			}
			//ɾ����¼
			p = deleteRecord1(p);
			count++;
		}
		//����д�ر��ļ�
		int page_id = buffer_manager.getPageId(table_name, i);
		// buffer_manager.flushPage(page_id , table_name , i);
		// ��Ϊ
		buffer_manager.modifyPage(page_id);
	}
	return count;
}

//���룺������Ŀ�����ԣ�һ��Where���͵Ķ���
//�����int(ɾ���ļ�¼��)
//���ܣ�ɾ����Ӧ��������Ŀ������ֵ����Where�����ļ�¼
//�쳣����������ڣ��׳�table_not_exist�쳣��������Բ����ڣ��׳�attribute_not_exist�쳣��
//���Where�����е������������Ͳ�ƥ�䣬�׳�data_type_conflict�쳣��
//int RecordManager::deleteRecord(std::string table_name, std::string target_attr, Where where) {
//	std::string tmp_name = table_name;
//	table_name = PATHH + table_name;
//	CatalogManager catalog_manager;
//	//�����Ƿ����
//	if (!catalog_manager.hasTable(tmp_name)) {
//		throw table_not_exist();
//	}
//	Attribute attr = catalog_manager.getAttribute(tmp_name);
//	int index = -1;
//	bool flag = false;
//	//��ȡĿ�����Զ�Ӧ�ı��
//	for (int i = 0; i < attr.num; i++) {
//		if (attr.name[i] == target_attr) {
//			index = i;
//			if (attr.has_index[i] == true)
//				flag = true;
//			break;
//		}
//	}
//	//Ŀ�����Բ����ڣ��׳��쳣
//	if (index == -1) {
//		throw attribute_not_exist();
//	}
//	//where�����е��������ݵ����Ͳ�ƥ�䣬�׳��쳣
//	else if (attr.type[index] != where.data.type) {
//		throw data_type_conflict();
//	}
//
//	//�쳣�������
//
//	int count = 0;
//	//���Ŀ��������������
//	if (flag == true && where.relation_character != NOT_EQUAL) {
//		std::vector<int> block_ids;
//		//ͨ��������ȡ���������ļ�¼���ڵĿ��
//		searchWithIndex(tmp_name, target_attr, where, block_ids);
//		for (int i = 0; i < block_ids.size(); i++) {
//			count += conditionDeleteInBlock(tmp_name, block_ids[i], attr, index, where);
//		}
//	}
//	else {
//		//��ȡ�ļ���ռ�������
//		// int block_num = getFileSize(table_name) / PAGESIZE;
//		// ��Ϊ
//		int block_num = getBlockNum(table_name);
//		//�ļ���СΪ0��ֱ�ӷ���
//		if (block_num <= 0)
//			return 0;
//		//�������еĿ�
//		for (int i = 0; i < block_num; i++) {
//			count += conditionDeleteInBlock(tmp_name, i, attr, index, where);
//		}
//	}
//	return count;
//}

int RecordManager::deleteRecord(std::string table_name, boolTree tree)
{
	std::string tmp_name = table_name;
	table_name = PATHH + table_name;
	CatalogManager catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.hasTable(tmp_name)) {
		throw table_not_exist();
	}
	Attribute attr = catalog_manager.getAttribute(tmp_name);
	vector<string> tree_attr = tree.getAttributeList();
	vector<Attribute> attrs;
	for (int i = 0; i < attr.num; i++) {
		attr.name[i] = tmp_name + "." + attr.name[i];
	}
	attrs.push_back(attr);
	tree.pushArrributes(attrs);
	vector<int> indexes;
	//��ȡĿ�����Զ�Ӧ�ı��,��¼����
	for (int i = 0; i < tree_attr.size(); i++) {
		int j;
		for (j = 0; j < attr.num; j++)
		{
			if (attr.name[j] == tree_attr[i])//�Ƚ�ʱҪע�����������ϱ���
			{
				indexes.push_back(j); break;
			}
		}
		if (j == attr.num)
			throw attribute_not_exist();

	}



	int count = 0;

	//��ȡ�ļ���ռ�������
	// int block_num = getFileSize(table_name) / PAGESIZE;
	// ��Ϊ
	int block_num = getBlockNum(table_name);
	//�ļ���СΪ0��ֱ�ӷ���
	if (block_num <= 0)
		return 0;
	//�������еĿ�
	for (int i = 0; i < block_num; i++) {
		count += conditionDeleteInBlock(tmp_name, i, attr, indexes, tree);
	}

	return count;
}

//���룺����
//�����Table���Ͷ���
//���ܣ��������ű�
//�쳣����������ڣ��׳�table_not_exist�쳣
Table RecordManager::selectRecord(std::string table_name, std::string result_table_name) {
	std::string tmp_name = table_name;
	table_name = PATHH + table_name;
	CatalogManager catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.hasTable(tmp_name)) {
		throw table_not_exist();
	}
	//��ȡ�ļ���ռ�Ŀ������
	// int block_num = getFileSize(table_name) / PAGESIZE;
	// ��Ϊ
	int block_num = getBlockNum(table_name);
	//�����ļ���СΪ0���������
	if (block_num <= 0)
		block_num = 1;
	//��ȡ�������
	Attribute attr = catalog_manager.getAttribute(tmp_name);
	for (int i = 0; i < attr.num; i++)
	{
		attr.name[i] = result_table_name + '.' + attr.name[i];
	}
	//����table���ʵ��
	Table table(result_table_name, attr);
	std::vector<Tuple>& v = table.getTuple();
	//�������п�

	for (int i = 0; i < block_num; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(table_name, i);
		char* t = p;
		//�����������м�¼
		while (*p != '\0' && p < t + PAGESIZE) {
			//��ȡ��¼
			Tuple tuple = readTuple(p, attr);
			//�����¼û�б�ɾ����������ӵ�table��
			if (tuple.isDeleted() == false)
				v.push_back(tuple);
			int len = getTupleLength(p);
			p = p + len;
		}
	}
	return table;
}

//���룺������Ŀ�����ԣ�һ��Where���͵Ķ���
//�����Table���Ͷ���
//���ܣ����ذ�������Ŀ����������Where�����ļ�¼�ı�
//�쳣����������ڣ��׳�table_not_exist�쳣��������Բ����ڣ��׳�attribute_not_exist�쳣��
//���Where�����е������������Ͳ�ƥ�䣬�׳�data_type_conflict�쳣��
Table RecordManager::selectRecord(std::string table_name, std::string target_attr, Where where, std::string result_table_name) {
	std::string tmp_name = table_name;
	table_name = PATHH + table_name;
	CatalogManager catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.hasTable(tmp_name)) {
		throw table_not_exist();
	}
	Attribute attr = catalog_manager.getAttribute(tmp_name);
	int index = -1;
	bool flag = false;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == target_attr) {
			index = i;
			if (attr.has_index[i] == true)
				flag = true;
			break;
		}
	}
	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		throw attribute_not_exist();
	}
	//where�����е��������ݵ����Ͳ�ƥ�䣬�׳��쳣
	else if (attr.type[index] != where.data.type) {
		throw data_type_conflict();
	}

	//�쳣������

	//����table
	Table table(result_table_name, attr);
	std::vector<Tuple>& v = table.getTuple();
	if (flag == true && where.relation_character != NOT_EQUAL) {
		std::vector<int> block_ids;
		//ʹ��������ȡ���������ļ�¼���ڿ��
		searchWithIndex(tmp_name, target_attr, where, block_ids);
		for (int i = 0; i < block_ids.size(); i++) {
			conditionSelectInBlock(tmp_name, block_ids[i], attr, index, where, v);
		}
	}
	else {
		//��ȡ�ļ���ռ�Ŀ������
		// int block_num = getFileSize(table_name) / PAGESIZE;
		// ��Ϊ
		int block_num = getBlockNum(table_name);
		//�����ļ���СΪ0���������
		if (block_num <= 0)
			block_num = 1;
		//�������п�
		for (int i = 0; i < block_num; i++) {
			conditionSelectInBlock(tmp_name, i, attr, index, where, v);
		}
	}
	return table;
}

//���룺������Ŀ��������
//�����void
//���ܣ��Ա����Ѿ����ڵļ�¼��������
//�쳣����������ڣ��׳�table_not_exist�쳣��������Բ����ڣ��׳�attribute_not_exist�쳣��
void RecordManager::createIndex(IndexManager& index_manager, std::string table_name, std::string target_attr) {
	std::string tmp_name = table_name;
	table_name = PATHH + table_name;
	CatalogManager catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.hasTable(tmp_name)) {
		throw table_not_exist();
	}
	Attribute attr = catalog_manager.getAttribute(tmp_name);
	int index = -1;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == target_attr) {
			index = i;
			break;
		}
	}
	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		throw attribute_not_exist();
	}
	//�쳣������

	//��ȡ�ļ���ռ�Ŀ������
	// int block_num = getFileSize(table_name) / PAGESIZE;
	// ��Ϊ
	int block_num = getBlockNum(table_name);
	//�����ļ���СΪ0���������
	if (block_num <= 0)
		block_num = 1;
	//��ȡ�������
	std::string file_path = "INDEX_FILE_" + target_attr + "_" + tmp_name;
	//�������п�
	for (int i = 0; i < block_num; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(table_name, i);
		char* t = p;
		//�����������м�¼
		while (*p != '\0' && p < t + PAGESIZE) {
			//��ȡ��¼
			Tuple tuple = readTuple(p, attr);
			if (tuple.isDeleted() == false) {
				std::vector<Data> v = tuple.getData();
				index_manager.insertIndex(file_path, v[index], i);
			}
			int len = getTupleLength(p);
			p = p + len;
		}
	}
}
Attribute RecordManager::getattr(string table_name) {
	std::string tmp_name = table_name;
	table_name = PATHH + table_name;
	CatalogManager catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.hasTable(tmp_name)) {
		throw table_not_exist();
	}
	//��ȡ�ļ���ռ�Ŀ������
	// int block_num = getFileSize(table_name) / PAGESIZE;
	// ��Ϊ
	int block_num = getBlockNum(table_name);
	//�����ļ���СΪ0���������
	if (block_num <= 0)
		block_num = 1;
	//��ȡ�������
	Attribute attr = catalog_manager.getAttribute(tmp_name);
	return attr;
}
vector<Table> RecordManager::group(Table table, std::vector<std::string> gattr) {

	vector<Table>m;
	if (gattr.size() == 0) { m.push_back(table); return m; }
	Attribute attr = table.getAttr();
	Table temp(table.getTitle(), attr);
	vector<Tuple>& v = temp.getTuple();
	vector<Tuple>tuple = table.getTuple();
	vector<int>mark;
	//�ҳ�����λ��
	for (int i = 0; i < gattr.size(); i++)
	{
		int index = -1;
		//��ȡĿ�����Եı��
		for (int j = 0; j < attr.num; j++) {
			if (attr.name[j] == gattr[i]) {
				index = j;
				break;
			}
		}
		//Ŀ�����Բ����ڣ��׳��쳣
		if (index == -1) {
			throw attribute_not_exist();
		}
		mark.push_back(index);
	}
	//����λ��

	for (int i = mark.size() - 1; i >= 0; i--)
	{
		switch (tuple[0].getData()[mark[i]].type)
		{
		case -1:
			for (int j = 0; j < tuple.size() - 1; j++)
				for (int k = 0; k < tuple.size() - j - 1; k++)
				{
					if (tuple[k].getData()[mark[i]].datai > tuple[k + 1].getData()[mark[i]].datai)
					{
						Tuple temp = tuple[k];
						tuple[k] = tuple[k + 1];
						tuple[k + 1] = temp;
					}
				}
			break;
		case 0: {for (int j = 0; j < tuple.size() - 1; j++)
			for (int k = 0; k < tuple.size() - j - 1; k++)
				if (tuple[k].getData()[mark[i]].dataf > tuple[k + 1].getData()[mark[i]].dataf)
				{
					Tuple temp = tuple[k];
					tuple[k] = tuple[k + 1];
					tuple[k + 1] = temp;
				}
				break; }
		default: {
			for (int j = 0; j < tuple.size() - 1; j++)
				for (int k = 0; k < tuple.size() - j - 1; k++)
					if (tuple[k].getData()[mark[i]].datas > tuple[k + 1].getData()[mark[i]].datas)
					{
						Tuple temp = tuple[k];
						tuple[k] = tuple[k + 1];
						tuple[k + 1] = temp;
					}
			break; }
		}
	}
	//�ֱ�
	for (int i = 0; i < tuple.size(); i++)
	{
		bool isPush = 1;
		if (i == 0) {
			v.push_back(tuple[i]);
			if (tuple.size() == 1)m.push_back(temp);
			continue;
		}
		else {
			vector<Data> data1 = v[v.size() - 1].getData();//not necessery
			vector<Data> data2 = tuple[i].getData();//not necessery

			for (int j = 0; j < mark.size(); j++)
			{

				if (data1[mark[j]].dataf == data2[mark[j]].dataf&&data1[mark[j]].datai == data2[mark[j]].datai&&data1[mark[j]].datas == data2[mark[j]].datas)
				{

					continue;
				}
				isPush = 0;

			}
			if (isPush) {
				v.push_back(tuple[i]); if (i == tuple.size() - 1) { m.push_back(temp); }
			}
			else {
				m.push_back(temp);
				v.clear();
				v.push_back(tuple[i]);
				if (i == tuple.size() - 1) m.push_back(temp);
			}
		}
	}
	return m;
}
vector<Table> RecordManager::order(vector<Table> table, vector<pair<std::string, UD>> oattr) {
	if (oattr.size() == 0)return table;
	vector<int>mark;
	Attribute attr = table[0].getAttr();
	//�ҳ�����λ��
	for (int i = 0; i < oattr.size(); i++)
	{
		int index = -1;
		//��ȡĿ�����Եı��
		for (int j = 0; j < attr.num; j++) {
			if (attr.name[j] == oattr[i].first) {
				index = j;
				break;
			}
		}
		//Ŀ�����Բ����ڣ��׳��쳣
		if (index == -1) {
			throw attribute_not_exist();
		}
		mark.push_back(index);
	}
	for (int q = 0; q < table.size(); q++)
	{
		vector<Tuple>&tuple = table[q].getTuple();
		if (q == 0 && tuple.size())return table;
		//����λ��
		for (int i = mark.size() - 1; i >= 0; i--)
		{
			switch (tuple[0].getData()[mark[i]].type)
			{
			case -1:
			{for (int j = 0; j < tuple.size() - 1; j++)
				for (int k = 0; k < tuple.size() - j - 1; k++)
					if (oattr[i].second == ASC) {
						if (tuple[k].getData()[mark[i]].datai > tuple[k + 1].getData()[mark[i]].datai)
						{
							Tuple temp = tuple[k];
							tuple[k] = tuple[k + 1];
							tuple[k + 1] = temp;
						}
					}
					else {
						if (tuple[k].getData()[mark[i]].datai < tuple[k + 1].getData()[mark[i]].datai)
						{
							Tuple temp = tuple[k];
							tuple[k] = tuple[k + 1];
							tuple[k + 1] = temp;
						}
					}
					break; }
			case 0: {for (int j = 0; j < tuple.size() - 1; j++)
				for (int k = 0; k < tuple.size() - j - 1; k++)
					if (oattr[i].second == ASC) {
						if (tuple[k].getData()[mark[i]].dataf > tuple[k + 1].getData()[mark[i]].dataf)
						{
							Tuple temp = tuple[k];
							tuple[k] = tuple[k + 1];
							tuple[k + 1] = temp;
						}
					}
					else {
						if (tuple[k].getData()[mark[i]].dataf < tuple[k + 1].getData()[mark[i]].dataf)
						{
							Tuple temp = tuple[k];
							tuple[k] = tuple[k + 1];
							tuple[k + 1] = temp;
						}
					}
					break; }
			default: {
				for (int j = 0; j < tuple.size() - 1; j++)
					for (int k = 0; k < tuple.size() - j - 1; k++)
						if (oattr[i].second == ASC) {
							if (tuple[k].getData()[mark[i]].datas > tuple[k + 1].getData()[mark[i]].datas)
							{
								Tuple temp = tuple[k];
								tuple[k] = tuple[k + 1];
								tuple[k + 1] = temp;
							}
						}
						else {
							if (tuple[k].getData()[mark[i]].datas < tuple[k + 1].getData()[mark[i]].datas)
							{
								Tuple temp = tuple[k];
								tuple[k] = tuple[k + 1];
								tuple[k + 1] = temp;
							}
						}
						break; }
			}
		}
	}
	return table;
};
vector<Table>RecordManager::having(vector<Table> table, boolTree tree, vector<string>select) {
	vector<string>e = tree.getAttributeList();
	if (e.size() == 0 || table[0].getTuple().size() == 0)return table;
	Attribute aa = table[0].getAttr();
	vector<Attribute>ee;
	ee.push_back(aa);
	tree.pushArrributes(ee);
	for (int i = 0; i < e.size(); i++)
	{
		string str = e[i];
		string sstr = str.substr(0, 4);
		string target;
		//���˾ۼ�����
		if (sstr == "sum(" || sstr == "avg(" || sstr == "max(" || sstr == "min(") {
			continue;
		}
		sstr = str.substr(0, 6);
		//count()
		if (sstr == "count(") {
			continue;
		}
		target = str;
		int index = -1;
		//��ȡĿ�����Եı��
		for (int i = 0; i < select.size(); i++) {
			if (select[i] == target) {
				index = i;
				break;
			}
		}
		//Ŀ�����Բ����ڣ��׳��쳣

		if (index == -1) {
			throw attribute_not_exist();
		}
	}
	vector<string>attributes = tree.getAttributeList();
	if (attributes.empty()) {
		return table;
	}
	vector<Table>::iterator it = table.begin();

	Attribute attr = (*it).getAttr();

	while (it != table.end())
	{
		Attribute attr = (*it).getAttr();
		vector<Tuple>&v = (*it).getTuple();
		tree.clear();
		vector<string>attributes = tree.getAttributeList();
		for (int k = 0; k < attributes.size(); k++)
		{
			string str = attributes[k];
			string sstr = str.substr(0, 4);
			string target;
			//sum()
			if (sstr == "sum(") {
				target = str.substr(4, str.size() - 5);
				tree.pushDouble(sum(*it, 0, target));
				continue;
			}
			//avg()
			if (sstr == "avg(") {
				target = str.substr(4, str.size() - 5);
				tree.pushDouble(avg(*it, 0, target));
				continue;
			}
			//max()
			if (sstr == "max(") {
				target = str.substr(4, str.size() - 5);
				tree.pushDouble(max(*it, target));
				continue;
			}
			//min()
			if (sstr == "min(") {
				target = str.substr(4, str.size() - 5);
				tree.pushDouble(min(*it, target));
				continue;
			}
			sstr = str.substr(0, 6);
			//count()
			if (sstr == "count(") {
				target = str.substr(6, str.size() - 7);
				tree.pushInt(count(*it, 0, target));
				continue;
			}
			//��ͨ����
			{target = str;
			//�ҵ�target����λ��;
			int index = -1;
			//��ȡĿ�����Եı��
			for (int i = 0; i < attr.num; i++) {
				if (attr.name[i] == target) {
					index = i;
					break;
				}
			}
			//Ŀ�����Բ����ڣ��׳��쳣
			if (index == -1) {
				throw attribute_not_exist();
			}
			switch (v[0].getData()[index].type)
			{
			case -1:tree.pushInt(v[0].getData()[index].datai); break;
			case 0:tree.pushDouble(v[0].getData()[index].dataf); break;
			default:tree.pushString(v[0].getData()[index].datas); break;
			}

			}
		}
		if (!tree.getResult()) it = table.erase(it);
		else it++;
	}
	return table;
}
Table RecordManager::select(vector<Table> table, vector<string>s) {
	Attribute attr = table[0].getAttr();
	Attribute mark;
	mark.num = s.size();
	int cnt = 0;
	for (int i = 0; i < s.size(); i++)
		mark.name[i] = s[i];
	if (table.size() == 1) {
		if (s[0] == "*") {
			Attribute attr = table[0].getAttr();
			Table final("final", attr);
			if (table[0].getTuple().size() == 0) return final;
			for (int j = 0; j < table[0].getTuple().size(); j++)
				final.getTuple().push_back(table[0].getTuple()[j]);
			return final;
		}
		//����
		for (int i = 0; i < s.size(); i++)
		{
			string str = s[i];
			string sstr = str.substr(0, 4);
			string target;
			//sum(),avg(),max(),min(),
			if (sstr == "sum(" || sstr == "avg(" || sstr == "max(" || sstr == "min(") {
				mark.type[i] = 256;
				cnt++;
				continue;
			}
			sstr = str.substr(0, 6);
			//count()
			if (sstr == "count(") {
				cnt++;
				mark.type[i] = 256;
				continue;
			}
			//��ͨ����
			{target = str;
			//�ҵ�target����λ��;
			int index = -1;
			//��ȡĿ�����Եı��
			for (int i = 0; i < attr.num; i++) {
				if (attr.name[i] == target) {
					index = i;
					break;
				}
			}
			//Ŀ�����Բ����ڣ��׳��쳣
			if (index == -1) {
				throw attribute_not_exist();
			}
			mark.type[i] = attr.type[index];
			}
		}
		Table final("final", mark);
		vector<Tuple> tuple;
		if (cnt == mark.num) {
			tuple.push_back(Tuple());
			for (int j = 0; j < mark.num; j++)
			{
				Data data;
				data.type = mark.type[j];

				switch (mark.type[j])
				{
				case -1: {

					string t = mark.name[j];
					//�ҵ�target����λ��;
					int index = -1;
					//��ȡĿ�����Եı��
					for (int i = 0; i < attr.num; i++) {
						if (attr.name[i] == t) {
							index = i;
							break;
						}
					}
					//Ŀ�����Բ����ڣ��׳��쳣
					if (index == -1) {
						throw attribute_not_exist();
					}

					data.datai = table[0].getTuple()[0].getData()[index].datai;
					data.type = -1;
					break; }
				case 0: {string t = mark.name[j];
					//�ҵ�target����λ��;
					int index = -1;
					//��ȡĿ�����Եı��
					for (int i = 0; i < attr.num; i++) {
						if (attr.name[i] == t) {
							index = i;
							break;
						}
					}
					//Ŀ�����Բ����ڣ��׳��쳣
					if (index == -1) {
						throw attribute_not_exist();
					}
					data.dataf = table[0].getTuple()[0].getData()[index].dataf;
					data.type = 1; break; }
				case 256: {

					string str = mark.name[j];
					string sstr = str.substr(0, 4);
					string target;
					//sum()
					if (sstr == "sum(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = sum(table[0], 0, target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					//avg()
					if (sstr == "avg(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = avg(table[0], 0, target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					//max()
					if (sstr == "max(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = max(table[0], target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					//min()
					if (sstr == "min(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = min(table[0], target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					sstr = str.substr(0, 6);
					//count()
					if (sstr == "count(") {
						target = str.substr(6, str.size() - 7);
						data.dataf = count(table[0], 0, target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					break;
				}
				default: {
					string t = mark.name[j];
					//�ҵ�target����λ��;
					int index = -1;
					//��ȡĿ�����Եı��
					for (int i = 0; i < attr.num; i++) {
						if (attr.name[i] == t) {
							index = i;
							break;
						}
					}
					//Ŀ�����Բ����ڣ��׳��쳣
					if (index == -1) {
						throw attribute_not_exist();
					}
					data.datas = table[0].getTuple()[0].getData()[index].datas;
					data.type = mark.type[j]; break; }
				}
				tuple[0].addData(data);
			}
			final.tuple_ = tuple;
			return final;
		}
		for (int i = 0; i < table[0].getTuple().size(); i++)
		{
			tuple.push_back(Tuple());
			for (int j = 0; j < mark.num; j++)
			{
				Data data;
				data.type = mark.type[j];

				switch (mark.type[j])
				{
				case -1: {

					string t = mark.name[j];
					//�ҵ�target����λ��;
					int index = -1;
					//��ȡĿ�����Եı��
					for (int i = 0; i < attr.num; i++) {
						if (attr.name[i] == t) {
							index = i;
							break;
						}
					}
					//Ŀ�����Բ����ڣ��׳��쳣
					if (index == -1) {
						throw attribute_not_exist();
					}

					data.datai = table[0].getTuple()[i].getData()[index].datai;
					data.type = -1;
					break; }
				case 0: {string t = mark.name[j];
					//�ҵ�target����λ��;
					int index = -1;
					//��ȡĿ�����Եı��
					for (int i = 0; i < attr.num; i++) {
						if (attr.name[i] == t) {
							index = i;
							break;
						}
					}
					//Ŀ�����Բ����ڣ��׳��쳣
					if (index == -1) {
						throw attribute_not_exist();
					}
					data.dataf = table[0].getTuple()[i].getData()[index].dataf;
					data.type = 0; break; }
				case 256: {

					string str = mark.name[j];
					string sstr = str.substr(0, 4);
					string target;
					//sum()
					if (sstr == "sum(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = sum(table[0], 0, target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					//avg()
					if (sstr == "avg(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = avg(table[0], 0, target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					//max()
					if (sstr == "max(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = max(table[0], target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					//min()
					if (sstr == "min(") {
						target = str.substr(4, str.size() - 5);
						data.dataf = min(table[0], target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					sstr = str.substr(0, 6);
					//count()
					if (sstr == "count(") {
						target = str.substr(6, str.size() - 7);
						data.dataf = count(table[0], 0, target);
						final.attr_.type[j] = 0;
						data.type = 0;
						break;
					}
					break;
				}
				default: {
					string t = mark.name[j];
					//�ҵ�target����λ��;
					int index = -1;
					//��ȡĿ�����Եı��
					for (int i = 0; i < attr.num; i++) {
						if (attr.name[i] == t) {
							index = i;
							break;
						}
					}
					//Ŀ�����Բ����ڣ��׳��쳣
					if (index == -1) {
						throw attribute_not_exist();
					}
					data.datas = table[0].getTuple()[i].getData()[index].datas;
					data.type = mark.type[j]; break; }
				}
				tuple[i].addData(data);
			}
		}

		final.tuple_ = tuple;
		return final;
	}

	for (int i = 0; i < s.size(); i++)
	{
		string str = s[i];
		string sstr = str.substr(0, 4);
		string target;
		//sum(),avg(),max(),min(),
		if (sstr == "sum(" || sstr == "avg(" || sstr == "max(" || sstr == "min(") {
			mark.type[i] = 256;
			continue;
		}
		sstr = str.substr(0, 6);
		//count()
		if (sstr == "count(") {
			mark.type[i] = 256;
			continue;
		}
		//��ͨ����
		{target = str;
		//�ҵ�target����λ��;
		int index = -1;
		//��ȡĿ�����Եı��
		for (int i = 0; i < attr.num; i++) {
			if (attr.name[i] == target) {
				index = i;
				break;
			}
		}
		//Ŀ�����Բ����ڣ��׳��쳣
		if (index == -1) {
			throw attribute_not_exist();
		}
		mark.type[i] = attr.type[index];
		}
	}
	Table final("final", mark);
	vector<Tuple> tuple;
	if (table[0].getTuple().size() == 0) return final;
	for (int i = 0; i < table.size(); i++)
	{
		tuple.push_back(Tuple());
		for (int j = 0; j < mark.num; j++)
		{
			Data data;
			data.type = mark.type[j];

			switch (mark.type[j])
			{
			case -1: {

				string t = mark.name[j];
				//�ҵ�target����λ��;
				int index = -1;
				//��ȡĿ�����Եı��
				for (int i = 0; i < attr.num; i++) {
					if (attr.name[i] == t) {
						index = i;
						break;
					}
				}
				//Ŀ�����Բ����ڣ��׳��쳣
				if (index == -1) {
					throw attribute_not_exist();
				}

				data.datai = table[i].getTuple()[0].getData()[index].datai;
				data.type = -1;
				break; }
			case 0: {string t = mark.name[j];
				//�ҵ�target����λ��;
				int index = -1;
				//��ȡĿ�����Եı��
				for (int i = 0; i < attr.num; i++) {
					if (attr.name[i] == t) {
						index = i;
						break;
					}
				}
				//Ŀ�����Բ����ڣ��׳��쳣
				if (index == -1) {
					throw attribute_not_exist();
				}
				data.dataf = table[i].getTuple()[0].getData()[index].dataf; data.type = 0; break; }
			case 256: {

				string str = mark.name[j];
				string sstr = str.substr(0, 4);
				string target;
				//sum()
				if (sstr == "sum(") {
					target = str.substr(4, str.size() - 5);
					data.dataf = sum(table[i], 0, target);
					data.type = 0;
					final.attr_.type[j] = 0;
					break;
				}
				//avg()
				if (sstr == "avg(") {
					target = str.substr(4, str.size() - 5);
					data.dataf = avg(table[i], 0, target);
					data.type = 0;
					final.attr_.type[j] = 0;
					break;
				}
				//max()
				if (sstr == "max(") {
					target = str.substr(4, str.size() - 5);
					data.dataf = max(table[i], target);
					data.type = 0;
					final.attr_.type[j] = 0;
					break;
				}
				//min()
				if (sstr == "min(") {
					target = str.substr(4, str.size() - 5);
					data.dataf = min(table[i], target);
					data.type = 0;
					final.attr_.type[j] = 0;
					break;
				}
				sstr = str.substr(0, 6);
				//count()
				if (sstr == "count(") {
					target = str.substr(6, str.size() - 7);
					data.dataf = count(table[i], 0, target);
					final.attr_.type[j] = 0;
					data.type = 0;
					break;
				}
				break;
			}
			default: {
				string t = mark.name[j];
				//�ҵ�target����λ��;
				int index = -1;
				//��ȡĿ�����Եı��
				for (int i = 0; i < attr.num; i++) {
					if (attr.name[i] == t) {
						index = i;
						break;
					}
				}
				//Ŀ�����Բ����ڣ��׳��쳣
				if (index == -1) {
					throw attribute_not_exist();
				}
				data.datas = table[i].getTuple()[0].getData()[index].datas;
				data.type = mark.type[j];
				break;
			}
			}
			tuple[i].addData(data);
		}


	}

	final.tuple_ = tuple;
	return final;

}


Table RecordManager::CartProduct(std::vector<Table> tables) {

	Table result = tables[0];//ȡ��һ������Ϊ�����ÿ�����ӶԵ�һ���������չ
	std::vector<Tuple>& result_tuple = result.getTuple();
	//�ѿ�����
	for (int k = 1; k < tables.size(); k++) {
		Table table1 = result;
		Table table2 = tables[k];
		Attribute attr1 = table1.getAttr();
		Attribute attr2 = table2.getAttr();
		Attribute result_attr = attr1;

		//�ϲ�������
		for (int i = 0; i < attr1.num; i++) {
			if (k <= 1) {
				//result_attr.name[i] = table1.title_ + "." + attr1.name[i];
				result_attr.name[i] =  attr1.name[i];
			}
			else {
				result_attr.name[i] = attr1.name[i];
			}

		}
		for (int j = 0; j < attr2.num; j++) {
			//result_attr.name[result_attr.num] = table2.title_ + "." + attr2.name[j];
			result_attr.name[result_attr.num] = attr2.name[j];
			result_attr.num++;
		}


		//�����µ������й����±�����result��
		Table result_table("tmp_table",result_attr);
		string s = result.title_;

		result = result_table;
		result.title_ = s;
		std::vector<Tuple> tuple1 = table1.getTuple();
		std::vector<Tuple> tuple2 = table2.getTuple();


		for (int i = 0; i < tuple1.size(); i++) {

			for (int j = 0; j < tuple2.size(); j++) {
				Tuple temp1 = tuple1[i];
				std::vector<Data>data = tuple2[j].getData();
				for (int k = 0; k < data.size(); k++) {
					temp1.addData(data[k]);
				}

				result_tuple.push_back(temp1);
			}

		}
	}

	return result;
}

//Table RecordManager::EqualJoin(vector<string> table_name, boolTree tree)
//{
//	Table result = selectRecord(table_name[0], table_name[0]);//ȡ��һ������Ϊ�����ÿ�����ӶԵ�һ���������չ
//	std::vector<Tuple>& result_tuple = result.getTuple();
//	for (int k = 1; k < table_name.size(); k++) {
//		Table table1 = result;
//		Table table2 = selectRecord(table_name[k], table_name[k]);
//		Attribute attr1 = table1.getAttr();
//		Attribute attr2 = table2.getAttr();
//		Attribute result_attr = attr1;
//
//		//�ϲ�������
//		for (int i = 0; i < attr1.num; i++) {
//			if (k <= 1) {
//				result_attr.name[i] = table1.title_ + "." + attr1.name[i];
//			}
//			else {
//				result_attr.name[i] = attr1.name[i];
//			}
//
//		}
//		for (int j = 0; j < attr2.num; j++) {
//			result_attr.name[result_attr.num] = table2.title_ + "." + attr2.name[j];
//			result_attr.num++;
//		}
//
//
//		//�����µ������й����±�����result��
//		Table result_table(result_attr);
//		string s = result.title_;
//
//		result = result_table;
//		result.title_ = s;
//		std::vector<Tuple> tuple1 = table1.getTuple();
//		std::vector<Tuple> tuple2 = table2.getTuple();
//
//
//		for (int i = 0; i < tuple1.size(); i++) {
//
//			for (int j = 0; j < tuple2.size(); j++) {
//				Tuple temp1 = tuple1[i];
//				std::vector<Data>data = tuple2[j].getData();
//				for (int k = 0; k < data.size(); k++) {
//					temp1.addData(data[k]);
//				}
//				Attribute attr = result.getAttr();
//				tree.clear();
//				vector<string> attrlist = tree.getAttributeList();
//				for (int x = 0; x < attrlist.size(); x++) {
//					string str = attrlist[x];
//					int index = -1;
//					for (int y = 0; y < attr.num; y++) {
//						if (attr.name[y] == str) {
//							index = y;
//							break;
//						}
//					}
//					if (index == -1) {
//						throw attribute_not_exist();
//					}
//					switch (temp1.getData()[index].type)
//					{
//					case -1:tree.pushInt(temp1.getData()[index].datai); break;
//					case 0:tree.pushDouble(temp1.getData()[index].dataf); break;
//					default:tree.pushString(temp1.getData()[index].datas); break;
//					}
//				}
//				if (!tree.getResult()) {
//					result_tuple.push_back(temp1);
//				}
//
//			}
//
//		}
//	}
//
//	return result;
//
//}
Table RecordManager::EqualJoin(vector<Table>tables, boolTree tree)
{
	Table result = tables[0];//ȡ��һ������Ϊ�����ÿ�����ӶԵ�һ���������չ
	std::vector<Tuple>& result_tuple = result.getTuple();
	for (int k = 1; k < tables.size(); k++) {
		Table table1 = result;
		Table table2 = tables[k];
		Attribute attr1 = table1.getAttr();
		Attribute attr2 = table2.getAttr();
		Attribute result_attr = attr1;

		//�ϲ�������
		for (int i = 0; i < attr1.num; i++) {
			if (k <= 1) {
				//result_attr.name[i] = table1.title_ + "." + attr1.name[i];
				result_attr.type[i] = attr1.type[i];
				result_attr.name[i] =  attr1.name[i];
			}
			else {
				result_attr.type[i] = attr1.type[i];
				result_attr.name[i] = attr1.name[i];
			}

		}
		for (int j = 0; j < attr2.num; j++) {
			//result_attr.name[result_attr.num] = table2.title_ + "." + attr2.name[j];
			result_attr.type[result_attr.num] = attr2.type[j];
			result_attr.name[result_attr.num] = attr2.name[j];
			result_attr.num++;
		}
		vector<Attribute> attrs;
		attrs.push_back(result_attr);
		tree.pushArrributes(attrs);

		//�����µ������й����±�����result��
		Table result_table("tmp_table",result_attr);
		string s = result.title_;

		result = result_table;
		result.title_ = s;
		std::vector<Tuple> tuple1 = table1.getTuple();
		std::vector<Tuple> tuple2 = table2.getTuple();


		for (int i = 0; i < tuple1.size(); i++) {

			for (int j = 0; j < tuple2.size(); j++) {
				Tuple temp1 = tuple1[i];
				std::vector<Data>data = tuple2[j].getData();
				for (int k = 0; k < data.size(); k++) {
					temp1.addData(data[k]);
				}
				Attribute attr = result.getAttr();
				tree.clear();
				vector<string> attrlist = tree.getAttributeList();
				for (int x = 0; x < attrlist.size(); x++) {
					string str = attrlist[x];
					int index = -1;
					for (int y = 0; y < attr.num; y++) {
						if (attr.name[y] == str) {
							index = y;
							break;
						}
					}
					if (index == -1) {
						throw attribute_not_exist();
					}
					switch (temp1.getData()[index].type)
					{
					case -1:tree.pushInt(temp1.getData()[index].datai); break;
					case 0:tree.pushDouble(temp1.getData()[index].dataf); break;
					default:tree.pushString(temp1.getData()[index].datas); break;
					}
				}
				if (tree.getResult()) {
					result_tuple.push_back(temp1);
				}

			}

		}
	}

	return result;

}

//Table RecordManager::naturalJoin(std::string table_name1, std::string table_name2)
//{
//	Table table1 = selectRecord(table_name1);
//	Table table2 = selectRecord(table_name2);
//	Attribute attr1 = table1.getAttr();
//	Attribute attr2 = table2.getAttr();
//	Attribute result_attr = attr1;
//
//
//	int sameAttrs[32][2];//��¼������ͬ���У�[t][0]��ʾ��1�е�λ�ã�[t][1]��ʾ��2�е�λ��
//	int t = 0;//��¼������ͬ�Ķ���-1
//			  //������Ȼ���ӵ�����
//	for (int i = 0; i < attr2.num; i++) {
//		for (int j = 0; j < attr1.num; j++) {
//			if (attr2.name[i] == attr1.name[j]) {
//				result_attr.name[i] = attr1.name[i];
//				sameAttrs[t][0] = j;
//				sameAttrs[t][1] = i;
//				t++;
//
//				break;
//			}
//			else {
//				if (j == attr1.num - 1) {
//					result_attr.name[result_attr.num] = attr2.name[i];
//					result_attr.num++;
//
//				}
//			}
//		}
//
//	}
//
//
//	//������Ȼ�������յļ�¼
//	Table result_table(result_attr);
//	std::vector<Tuple> tuple1 = table1.getTuple();
//	std::vector<Tuple> tuple2 = table2.getTuple();
//	std::vector<Tuple>& result_tuple = result_table.getTuple();
//	bool ok = true;//�ж��Ƿ�������ͬ�����Զ���ͬ
//	for (int i = 0; i < tuple1.size(); i++) {//��1��ÿ����¼
//
//		for (int j = 0; j < tuple2.size(); j++) {//��2��ÿ����¼
//			Tuple temp1 = tuple1[i];
//			std::vector<Data>data1 = tuple1[i].getData();
//			std::vector<Data>data2 = tuple2[j].getData();//��2ÿ��Ԫ�������
//
//			for (int x = 0; x < t; x++)
//			{
//				if (!Datacmp(data1[sameAttrs[x][0]], data2[sameAttrs[x][1]]))
//				{
//					ok = false;
//					break;
//				}
//			}
//
//			if (ok)
//			{
//				int x = 0;
//				for (int k = 0; k < data2.size(); k++) {
//					if (k == sameAttrs[x][1]) x++;
//					else
//					{
//						temp1.addData(data2[k]);
//					}
//				}
//			}
//			else
//			{
//				ok = true;
//				break;
//			}
//			result_tuple.push_back(temp1);
//		}
//
//	}
//
//	return result_table;
//
//}
Table RecordManager::naturalJoin(Table table1, Table table2,boolTree tree)
{
		
		Attribute attr1 = table1.getAttr();
		string s1 = table1.getTitle();
		Attribute attr2 = table2.getAttr();
		string s2 = table2.getTitle();
		//ȥ����������
		for (int i = 0; i < attr1.num; i++)
		{
			attr1.name[i] = attr1.name[i].substr(s1.length()+1, attr1.name[i].length() - s1.length()-1);
		}
		for (int i = 0; i < attr2.num; i++)
		{
			attr2.name[i] = attr2.name[i].substr(s2.length()+1, attr2.name[i].length() - s2.length()-1);
		}

		Attribute result_attr = attr1;
		//string s = table1.getTitle();
		
		
	
		int sameAttrs[32][2];//��¼������ͬ���У�[t][0]��ʾ��1�е�λ�ã�[t][1]��ʾ��2�е�λ��
		int t = 0;//��¼������ͬ�Ķ���-1
				  //������Ȼ���ӵ�����
		for (int i = 0; i < attr2.num; i++) {
			for (int j = 0; j < attr1.num; j++) {
				if (attr2.name[i] == attr1.name[j]) {
					result_attr.name[i] = attr1.name[i];
					result_attr.type[i] = attr1.type[i];
					sameAttrs[t][0] = j;
					sameAttrs[t][1] = i;
					t++;
	
					break;
				}
				else {
					if (j == attr1.num - 1) {
						result_attr.name[result_attr.num] = attr2.name[i];
						result_attr.type[result_attr.num] = attr2.type[i];
						result_attr.num++;
	
					}
				}
			}
	
		}
	
	    
		//��������ձ����֣�������
		Table result_table("tmp_table",result_attr);

		//�ҵ�boolTree��Ӧ������������
		vector<int> indexes;
		for (int i = 0; i < tree.getAttributeList().size(); i++)
		{
			for (int j = 0; j < result_attr.num; j++)
			{
				if (result_attr.name[j] == tree.getAttributeList()[i])
					indexes.push_back(j);
			}
		}

		

		vector<Attribute> result_attrs;
		result_attrs.push_back(result_attr);
		tree.pushArrributes(result_attrs);
		std::vector<Tuple> tuple1 = table1.getTuple();
		std::vector<Tuple> tuple2 = table2.getTuple();
		std::vector<Tuple>& result_tuple = result_table.getTuple();
		bool ok = true;//�ж��Ƿ�������ͬ�����Զ���ͬ
		for (int i = 0; i < tuple1.size(); i++) {//��1��ÿ����¼
	
			for (int j = 0; j < tuple2.size(); j++) {//��2��ÿ����¼
				Tuple temp1 = tuple1[i];
				std::vector<Data>data1 = tuple1[i].getData();
				std::vector<Data>data2 = tuple2[j].getData();//��2ÿ��Ԫ�������
	
				for (int x = 0; x < t; x++)
				{
					if (!Datacmp(data1[sameAttrs[x][0]], data2[sameAttrs[x][1]]))
					{
						ok = false;
						break;
					}
				}
	
				if (ok)
				{
					int x = 0;
					for (int k = 0; k < data2.size(); k++) {
						if (k == sameAttrs[x][1]) x++;
						else
						{
							temp1.addData(data2[k]);
						}
					}
				}
				else
				{
					ok = true;
					continue;
				}
				if(TuplesatisTree(temp1,tree,indexes))
				result_tuple.push_back(temp1);

			}
	
		}
	
		return result_table;
}

double RecordManager::count(Table table, bool isDistinct, std::string target)
{

	if (target == "*")return table.getTuple().size();
	//�ظ�Ҳ��
	Attribute attr = table.getAttr();
	//�ҵ�target����λ��;
	int index = -1;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == target) {
			index = i;
			break;
		}
	}
	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		throw attribute_not_exist();
	}
	if (!isDistinct)return table.getTuple().size();
	//�ظ�����
	else {

		double count = 0;
		vector<Tuple> tuple = table.getTuple();
		switch (attr.type[index])
		{
		case -1: {
			vector<int>mark;
			for (int i = 0; i < tuple.size(); i++)
				if (find(mark.begin(), mark.end(), tuple[i].getData()[index].datai) == mark.end())
				{
					count++;
					mark.push_back(tuple[i].getData()[index].datai);
				}
			break;
		}
		case 0: {
			vector<double>mark;
			for (int i = 0; i < tuple.size(); i++)
				if (find(mark.begin(), mark.end(), tuple[i].getData()[index].dataf) == mark.end())
				{
					count++;
					mark.push_back(tuple[i].getData()[index].dataf);
				}
			break;
		}
		default:
		{
			vector<string>mark;
			for (int i = 0; i < tuple.size(); i++)
				if (find(mark.begin(), mark.end(), tuple[i].getData()[index].datas) == mark.end())
				{
					count++;
					mark.push_back(tuple[i].getData()[index].datas);
				}
			break;
		}
		}
		return count;
	}


}
double RecordManager::sum(Table table, bool isDistinct, std::string target)
{
	Attribute attr = table.getAttr();
	//�ҵ�target����λ��;
	int index = -1;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == target) {
			index = i;
			break;
		}
	}
	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		throw attribute_not_exist();
	}
	vector <double> mark;
	vector<Tuple> tuple = table.getTuple();
	switch (attr.type[index])
	{
	case -1: {
		for (int i = 0; i < tuple.size(); i++)
			if (isDistinct)
			{
				if (std::count(mark.begin(), mark.end(), tuple[i].getData()[index].datai) == 0)
				{
					mark.push_back(tuple[i].getData()[index].datai);
				}
			}
			else mark.push_back(tuple[i].getData()[index].datai);
		break;
	}
	case 0: {
		for (int i = 0; i < tuple.size(); i++)
			if (isDistinct)
			{
				if (std::count(mark.begin(), mark.end(), tuple[i].getData()[index].dataf) == 0)
				{
					mark.push_back(tuple[i].getData()[index].dataf);
				}
			}
			else mark.push_back(tuple[i].getData()[index].dataf);
	}
			break;
	default:
	{   throw  data_type_conflict();
	break;
	}
	}
	double sum = 0;
	for (int i = 0; i < mark.size(); i++)
	{
		sum += mark[i];
	}
	return sum;
};
double RecordManager::avg(Table table, bool isDistinct, std::string target) {
	return sum(table, isDistinct, target) / table.getTuple().size();
}
double  RecordManager::max(Table table, std::string target) {
	Attribute attr = table.getAttr();
	//�ҵ�target����λ��;
	int index = -1;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == target) {
			index = i;
			break;
		}
	}
	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		throw attribute_not_exist();
	}
	vector<Tuple>tuple = table.getTuple();
	double max;
	switch (attr.type[index])
	{
	case -1: {
		max = tuple[0].getData()[index].datai;
		for (int i = 1; i < tuple.size(); i++)
		{
			if (tuple[i].getData()[index].datai > max)max = tuple[i].getData()[index].datai;
		}
		break;
	}
	case 0: {
		max = tuple[0].getData()[index].dataf;
		for (int i = 1; i < tuple.size(); i++)
		{
			if (tuple[i].getData()[index].dataf > max)max = tuple[i].getData()[index].dataf;
		}
		break;
	}
	default:
	{   throw  data_type_conflict();
	break;
	}
	}
	return max;
}
double  RecordManager::min(Table table, std::string target) {
	Attribute attr = table.getAttr();
	//�ҵ�target����λ��;
	int index = -1;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == target) {
			index = i;
			break;
		}
	}
	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		throw attribute_not_exist();
	}
	vector<Tuple>tuple = table.getTuple();
	double min;
	switch (attr.type[index])
	{
	case -1: {
		min = tuple[0].getData()[index].datai;
		for (int i = 1; i < tuple.size(); i++)
		{
			if (tuple[i].getData()[index].datai < min)min = tuple[i].getData()[index].datai;
		}
		break;
	}
	case 0: {
		min = tuple[0].getData()[index].dataf;
		for (int i = 1; i < tuple.size(); i++)
		{
			if (tuple[i].getData()[index].dataf < min)min = tuple[i].getData()[index].dataf;
		}
		break;
	}
	default:
	{   throw  data_type_conflict();
	break;
	}
	}
	return min;
}
//�����Ǽ�����������������ϸע����

//��ȡ�ļ���С
int RecordManager::getBlockNum(std::string table_name) {
	char* p;
	int block_num = -1;
	do {
		p = buffer_manager.getPage(table_name, block_num + 1);
		block_num++;
	} while (p[0] != '\0');
	return block_num;
}

//insertRecord�ĸ�������
void RecordManager::insertRecord1(char* p, int offset, int len, const std::vector<Data>& v) {
	std::stringstream stream;
	stream << len;
	std::string s = stream.str();
	while (s.length() < 4)
		s = "0" + s;
	for (int j = 0; j < s.length(); j++, offset++)
		p[offset] = s[j];
	for (int j = 0; j < v.size(); j++) {
		p[offset] = ' ';
		offset++;
		Data d = v[j];
		switch (d.type) {
		case -1: {
			copyString(p, offset, d.datai);
		}; break;
		case 0: {
			copyString(p, offset, d.dataf);
		}; break;
		default: {
			copyString(p, offset, d.datas);
		};
		}
	}
	p[offset] = ' ';
	p[offset + 1] = '0';
	p[offset + 2] = '\n';
}

//deleteRecord�ĸ�������
char* RecordManager::deleteRecord1(char* p) {
	int len = getTupleLength(p);
	p = p + len;
	*(p - 2) = '1';
	return p;
}

//���ڴ��ж�ȡһ��tuple
Tuple RecordManager::readTuple(const char* p, Attribute attr) {
	Tuple tuple;
	p = p + 5;
	for (int i = 0; i < attr.num; i++) {
		Data data;
		data.type = attr.type[i];
		char tmp[100];
		int j;
		for (j = 0; *p != ' '; j++, p++) {
			tmp[j] = *p;
		}
		tmp[j] = '\0';
		p++;
		std::string s(tmp);
		switch (data.type) {
		case -1: {
			std::stringstream stream(s);
			stream >> data.datai;
		}; break;
		case 0: {
			std::stringstream stream(s);
			stream >> data.dataf;
		}; break;
		default: {
			data.datas = s;
		}
		}
		tuple.addData(data);
	}
	if (*p == '1')
		tuple.setDeleted();
	return tuple;
}

//��ȡһ��tuple�ĳ���
int RecordManager::getTupleLength(char* p) {
	char tmp[10];
	int i;
	for (i = 0; p[i] != ' '; i++)
		tmp[i] = p[i];
	tmp[i] = '\0';
	std::string s(tmp);
	int len = stoi(s);
	return len;
}

//�жϲ���ļ�¼�Ƿ��������¼��ͻ
bool RecordManager::isConflict(std::vector<Tuple>& tuples, std::vector<Data>& v, int index) {
	for (int i = 0; i < tuples.size(); i++) {
		if (tuples[i].isDeleted() == true)
			continue;
		std::vector<Data> d = tuples[i].getData();
		switch (v[index].type) {
		case -1: {
			if (v[index].datai == d[index].datai)
				return true;
		}; break;
		case 0: {
			if (v[index].dataf == d[index].dataf)
				return true;
		}; break;
		default: {
			if (v[index].datas == d[index].datas)
				return true;
		};
		}
	}
	return false;
}

//����������
void RecordManager::searchWithIndex(std::string table_name, std::string target_attr, Where where, std::vector<int>& block_ids) {
	IndexManager index_manager(table_name);
	Data tmp_data;
	std::string file_path = "INDEX_FILE_" + target_attr + "_" + table_name;
	if (where.relation_character == LESS || where.relation_character == LESS_OR_EQUAL) {
		if (where.data.type == -1) {
			tmp_data.type = -1;
			tmp_data.datai = -INF;
		}
		else if (where.data.type == 0) {
			tmp_data.type = 0;
			tmp_data.dataf = -INF;
		}
		else {
			tmp_data.type = 1;
			tmp_data.datas = "";
		}
		index_manager.searchRange(file_path, tmp_data, where.data, block_ids);
	}
	else if (where.relation_character == GREATER || where.relation_character == GREATER_OR_EQUAL) {
		if (where.data.type == -1) {
			tmp_data.type = -1;
			tmp_data.datai = INF;
		}
		else if (where.data.type == 0) {
			tmp_data.type = 0;
			tmp_data.dataf = INF;
		}
		else {
			tmp_data.type = -2;
		}
		index_manager.searchRange(file_path, where.data, tmp_data, block_ids);
	}
	else {
		index_manager.searchRange(file_path, where.data, where.data, block_ids);
	}
}

//�ڿ��н�������ɾ��
int RecordManager::conditionDeleteInBlock(std::string table_name, int block_id, Attribute attr, int index, Where where) {
	//��ȡ��ǰ��ľ��
	table_name = PATHH + table_name;//����
	char* p = buffer_manager.getPage(table_name, block_id);
	char* t = p;
	int count = 0;
	//�����������м�¼
	while (*p != '\0' && p < t + PAGESIZE) {
		//��ȡ��¼
		Tuple tuple = readTuple(p, attr);
		std::vector<Data> d = tuple.getData();
		//������������ִ�в�ͬ����
		switch (attr.type[index]) {
		case -1: {
			//�������where����
			if (isSatisfied(d[index].datai, where.data.datai, where.relation_character) == true) {
				//����¼ɾ��
				p = deleteRecord1(p);
				count++;
			}
			//���������where�����������ü�¼
			else {
				int len = getTupleLength(p);
				p = p + len;
			}
		}; break;
			//ͬcase1
		case 0: {
			if (isSatisfied(d[index].dataf, where.data.dataf, where.relation_character) == true) {
				p = deleteRecord1(p);
				count++;
			}
			else {
				int len = getTupleLength(p);
				p = p + len;
			}
		}; break;
			//ͬcase1
		default: {
			if (isSatisfied(d[index].datas, where.data.datas, where.relation_character) == true) {
				p = deleteRecord1(p);
				count++;
			}
			else {
				int len = getTupleLength(p);
				p = p + len;
			}
		}
		}
	}
	//����ǰ��д���ļ�
	int page_id = buffer_manager.getPageId(table_name, block_id);
	// buffer_manager.flushPage(page_id , table_name , block_id);
	// ��Ϊ
	buffer_manager.modifyPage(page_id);
	return count;
}

int RecordManager::conditionDeleteInBlock(std::string table_name, int block_id, Attribute attr, vector<int> indexes, boolTree & tree)
{
	//��ȡ��ǰ��ľ��
	table_name = PATHH + table_name;//����
	char* p = buffer_manager.getPage(table_name, block_id);
	//cout<<buffer_manager.getPageId(table_name, block_id);
	char* t = p;
	int count = 0;
	//�����������м�¼
	while (*p != '\0' && p < t + PAGESIZE) {
		//��ȡ��¼
		Tuple tuple = readTuple(p, attr);

		//������������ִ�в�ͬ����
		if (TuplesatisTree(tuple, tree, indexes))
		{
			p = deleteRecord1(p);
			count++;
		}
		else {
			int len = getTupleLength(p);
			p = p + len;
		}

	}
	//����ǰ��д���ļ�
	//cout << buffer_manager.Frames[2].getBuffer();
	int page_id = buffer_manager.getPageId(table_name, block_id);
	// buffer_manager.flushPage(page_id , table_name , block_id);
	// ��Ϊ
	buffer_manager.modifyPage(page_id);
	return count;
}

//�ڿ��н���������ѯ
void RecordManager::conditionSelectInBlock(std::string table_name, int block_id, Attribute attr, int index, Where where, std::vector<Tuple>& v) {
	//��ȡ��ǰ��ľ��
	table_name = PATHH + table_name;//����
	char* p = buffer_manager.getPage(table_name, block_id);
	char* t = p;
	//�������м�¼
	while (*p != '\0' && p < t + PAGESIZE) {
		//��ȡ��¼
		Tuple tuple = readTuple(p, attr);
		//�����¼�ѱ�ɾ���������ü�¼
		if (tuple.isDeleted() == true) {
			int len = getTupleLength(p);
			p = p + len;
			continue;
		}
		std::vector<Data> d = tuple.getData();
		//������������ѡ��
		switch (attr.type[index]) {
		case -1: {
			//�����������򽫸�Ԫ����ӵ�table
			if (isSatisfied(d[index].datai, where.data.datai, where.relation_character) == true) {
				v.push_back(tuple);
			}
			//�����������������ü�¼
		}; break;
			//ͬcase1
		case 0: {
			if (isSatisfied(d[index].dataf, where.data.dataf, where.relation_character) == true) {
				v.push_back(tuple);
			}
		}; break;
			//ͬcase1
		default: {
			if (isSatisfied(d[index].datas, where.data.datas, where.relation_character) == true) {
				v.push_back(tuple);
			}
		};
		}
		int len = getTupleLength(p);
		p = p + len;
	}
}

