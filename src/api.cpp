#include "api.h"
#include "template_function.h"
#include <algorithm>
#include <vector>  
#include <iterator>
using namespace std;
Table API::selectRecord1(string name)
{
	return record->selectRecord(name, name);
}
//构造函数
API::API() {}

//析构函数
API::~API() {}

//输入：表名、Where条件属性名、Where条件值域
//输出：Table类型对象(包含对应的属性元组)
//功能：返回包含所有目标属性满足Where条件的记录的表
//在多条件查询情况下，根据Where下的逻辑条件进行Table的拼接
//异常：由底层处理
//如果表不存在，抛出table_not_exist异常
//如果属性不存在，抛出attribute_not_exist异常
//如果Where条件中的两个数据类型不匹配，抛出data_type_conflict异常
Attribute API::getAttr(string table)
{
	Attribute attr = record->getattr(table);
	return attr;
};



Table API::setOperation(Table table1, Table table2, int operation)
{
	Attribute attr = table1.getAttr();
	Table result_table(attr);
	std::vector<Tuple>& result_tuple = result_table.getTuple();
	std::vector<Tuple> tuple1 = table1.getTuple();
	std::vector<Tuple> tuple2 = table2.getTuple();

	switch (operation)
	{
		//交
	case 0:
		for (int i = 0; i < tuple2.size(); i++) {
			if (isInTable(table1, tuple2[i])) {
				result_tuple.push_back(tuple2[i]);
			}
		}
		break;
		//并
	case 1:
		result_tuple = tuple1;
		for (int i = 0; i < tuple2.size(); i++) {
			if (!isInTable(table1, tuple2[i])) {
				result_tuple.push_back(tuple2[i]);
			}
		}
		break;
		//差
	case 2:

		for (int i = 0; i < tuple1.size(); i++) {
			if (!isInTable(table2, tuple1[i])) {
				result_tuple.push_back(tuple1[i]);
			}
		}
		break;
	default:
		break;
	}
	return result_table;
}


int API::deleteRecord(std::string table_name, boolTree  tree)
{
	int result;
	result = record->deleteRecord(table_name, tree);
	return result;
}
//输入：表名、一个元组对象
//输出：void
//功能：向对应表内插入一条记录
//异常：由底层处理
//如果元组类型不匹配，抛出tuple_type_conflict异常
//如果主键冲突，抛出primary_key_conflict异常
//如果unique属性冲突，抛出unique_conflict异常
//如果表不存在，抛出table_not_exist异常
void API::insertRecord(std::string table_name, Tuple& tuple)
{
	record->insertRecord(table_name, tuple);
	return;
}
//输入：Table类型对象
//输出：是否创建成功
//功能：在数据库中插入一个表的元信息
//异常：由底层处理
//如果已经有相同表名的表存在，则抛出table_exist异常
bool API::createTable(std::string table_name, Attribute attribute, int primary, Index index)
{
	record->createTableFile(table_name);
	catalog->createTable(table_name, attribute, primary, index);

	return true;
}
//输入：表名
//输出：是否删除成功
//功能：在数据库中删除一个表的元信息，及表内所有记录(删除表文件)
//异常：由底层处理
//如果表不存在，抛出table_not_exist异常
bool API::dropTable(std::string table_name)
{
	record->dropTableFile(table_name);
	catalog->dropTable(table_name);

	return true;
}
//输入：表名，索引名，属性名
//输出：是否创建成功
//功能：在数据库中更新对应表的索引信息（在指定属性上建立一个索引）
//异常：由底层处理
//如果表不存在，抛出table_not_exist异常
//如果对应属性不存在，抛出attribute_not_exist异常
//如果对应属性已经有了索引，抛出index_exist异常
bool API::createIndex(std::string table_name, std::string index_name, std::string attr_name)
{
	IndexManager index(table_name);

	std::string file_path = "INDEX_FILE_" + attr_name + "_" + table_name;
	int type;

	catalog->createIndex(table_name, attr_name, index_name);
	Attribute attr = catalog->getAttribute(table_name);
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == attr_name) {
			type = (int)attr.type[i];
			break;
		}
	}
	index.createIndex(file_path, type);
	record->createIndex(index, table_name, attr_name);

	return true;
}
//输入：表名，索引名
//输出：是否删除成功
//功能：删除对应表的对应属性上的索引
//异常：由底层处理
//如果表不存在，抛出table_not_exist异常
//如果对应属性不存在，抛出attribute_not_exist异常
//如果对应属性没有索引，抛出index_not_exist异常
bool API::dropIndex(std::string table_name, std::string index_name)
{
	IndexManager index(table_name);

	std::string attr_name = catalog->IndextoAttr(table_name, index_name);
	std::string file_path = "INDEX_FILE_" + attr_name + "_" + table_name;
	int type;

	Attribute attr = catalog->getAttribute(table_name);
	for (int i = 0; i < attr.num; i++) {
		if (attr.name[i] == attr_name) {
			type = (int)attr.type[i];
			break;
		}
	}
	index.dropIndex(file_path, type);
	catalog->dropIndex(table_name, index_name);

	file_path = "./src/" + file_path;
	remove(file_path.c_str());
	return true;
}

Table API::distinct(Table  soureTable)
{
	/*Table distinctTable(soureTable);
	vector<Tuple>::iterator it1 = distinctTable.getTuple().begin();
	vector<Tuple>::iterator it2 = distinctTable.getTuple().begin() + 1;
	while (it1 != distinctTable.getTuple().end())
	{
		while (it2 != distinctTable.getTuple().end())
		{
			if (Tuplecmp((*it1), (*it2)))
			{
				it2 = distinctTable.getTuple().erase(it2);
				
			}
			else
			{
				it2++;
			}
		}
		it1++;
		it2 = it1 + 1;

	}
	return distinctTable;*/
	Attribute attr = soureTable.getAttr();
	Table result_table(attr);
	std::vector<Tuple>& result_tuple = result_table.getTuple();
	vector<Tuple>tuple1 = soureTable.getTuple();
	for (int i = 0; i < tuple1.size(); i++) {
		if (!isInTable(result_table, tuple1[i])) {
			result_tuple.push_back(tuple1[i]);
		}
	}
	return result_table;
}
void API::update(std::string table_name, string attr_name, Data attr_value, boolTree tree)
{
	Table temptable = record->selectRecord(table_name, table_name);
	Table target_table = selectRecordSingle(temptable, tree);
	deleteRecord(table_name, tree);

	vector<Attribute>attrs;
	//Table target_table = record->selectRecord(table_name, table_name);
	Attribute attr = target_table.getAttr();

	attrs.push_back(attr);
	tree.pushArrributes(attrs);

	vector<Tuple> tuples = target_table.getTuple();

	//找到要修改的属性列索引
	int index = -1;
	for (int i = 0; i < attr.num; i++)
	{
		if (attr.name[i] == attr_name)
		{
			index = i;
			break;
		}
	}
	//对每条记录，如果满足条件就把数据修改,并写入
	for (int i = 0; i < tuples.size(); i++)
	{
		tuples[i].setData(index, attr_value);
		record->insertRecord(table_name, tuples[i]);
	}

}

int API::TupleNum(std::string table_name)
{
	Table table = record->selectRecord(table_name, table_name);
	return table.getTuple().size();
}

void API::ShowTable(Table& table)
{
	Attribute attr_record = table.getAttr();
	
	int use[32] = { 0 };

	for (int i = 0; i < attr_record.num; i++)
		use[i] = i;

	std::vector<Tuple> output_tuple = table.getTuple();
	int longest = -1;
	for (int index = 0; index < attr_record.num; index++) {
		if ((int)attr_record.name[use[index]].length() > longest)
			longest = (int)attr_record.name[use[index]].length();
	}
	for (int index = 0; index < attr_record.num; index++) {
		int type = attr_record.type[use[index]];
		if (type == -1) {
			for (int i = 0; i < output_tuple.size(); i++) {
				if (longest < getBits(output_tuple[i].getData()[use[index]].datai)) {
					longest = getBits(output_tuple[i].getData()[use[index]].datai);
				}
			}
		}
		if (type == 0) {
			for (int i = 0; i < output_tuple.size(); i++) {
				if (longest < getBits(output_tuple[i].getData()[use[index]].dataf)) {
					longest = getBits(output_tuple[i].getData()[use[index]].dataf);
				}
			}
		}
		if (type > 0) {
			for (int i = 0; i < output_tuple.size(); i++) {
				if (longest < output_tuple[i].getData()[use[index]].datas.length()) {
					longest = (int)output_tuple[i].getData()[use[index]].datas.length();
				}
			}
		}
	}
	longest += 1;
	for (int index = 0; index < attr_record.num; index++) {
		if (index != attr_record.num - 1) {
			for (int i = 0; i < (longest - attr_record.name[use[index]].length()) / 2; i++)
				printf(" ");
			printf("%s", attr_record.name[use[index]].c_str());
			for (int i = 0; i < longest - (longest - attr_record.name[use[index]].length()) / 2 - attr_record.name[use[index]].length(); i++)
				printf(" ");
			printf("|");
		}
		else {
			for (int i = 0; i < (longest - attr_record.name[use[index]].length()) / 2; i++)
				printf(" ");
			printf("%s", attr_record.name[use[index]].c_str());
			for (int i = 0; i < longest - (longest - attr_record.name[use[index]].length()) / 2 - attr_record.name[use[index]].length(); i++)
				printf(" ");
			printf("\n");
		}
	}
	for (int index = 0; index < attr_record.num*(longest + 1); index++) {
		std::cout << "-";
	}
	std::cout << std::endl;
	for (int index = 0; index < output_tuple.size(); index++) {
		for (int i = 0; i < attr_record.num; i++)
		{
			switch (output_tuple[index].getData()[use[i]].type) {
			case -1:
				if (i != attr_record.num - 1) {
					int len = output_tuple[index].getData()[use[i]].datai;
					len = getBits(len);
					for (int i = 0; i < (longest - len) / 2; i++)
						printf(" ");
					printf("%d", output_tuple[index].getData()[use[i]].datai);
					for (int i = 0; i < longest - (longest - len) / 2 - len; i++)
						printf(" ");
					printf("|");
				}
				else {
					int len = output_tuple[index].getData()[use[i]].datai;
					len = getBits(len);
					for (int i = 0; i < (longest - len) / 2; i++)
						printf(" ");
					printf("%d", output_tuple[index].getData()[use[i]].datai);
					for (int i = 0; i < longest - (longest - len) / 2 - len; i++)
						printf(" ");
					printf("\n");
				}
				break;
			case 0:
				if (i != attr_record.num - 1) {
					float num = output_tuple[index].getData()[use[i]].dataf;
					int len = getBits(num);
					for (int i = 0; i < (longest - len) / 2; i++)
						printf(" ");
					printf("%.2f", output_tuple[index].getData()[use[i]].dataf);
					for (int i = 0; i < longest - (longest - len) / 2 - len; i++)
						printf(" ");
					printf("|");
				}
				else {
					float num = output_tuple[index].getData()[use[i]].dataf;
					int len = getBits(num);
					for (int i = 0; i < (longest - len) / 2; i++)
						printf(" ");
					printf("%.2f", output_tuple[index].getData()[use[i]].dataf);
					for (int i = 0; i < longest - (longest - len) / 2 - len; i++)
						printf(" ");
					printf("\n");
				}
				break;
			default:
				std::string tmp = output_tuple[index].getData()[use[i]].datas;
				if (i != attr_record.num - 1) {
					for (int i = 0; i < (longest - tmp.length()) / 2; i++)
						printf(" ");
					printf("%s", tmp.c_str());
					for (int i = 0; i < longest - (longest - (int)tmp.length()) / 2 - (int)tmp.length(); i++)
						printf(" ");
					printf("|");
				}
				else {
					std::string tmp = output_tuple[index].getData()[i].datas;
					for (int i = 0; i < (longest - tmp.length()) / 2; i++)
						printf(" ");
					printf("%s", tmp.c_str());
					for (int i = 0; i < longest - (longest - (int)tmp.length()) / 2 - (int)tmp.length(); i++)
						printf(" ");
					printf("\n");
				}
				break;
			}
		}
	}
//table.showTable();
}

vector<string> API::getAllTable()
{
	return catalog->getAllTable();
}



Table API::selectRecord(std::vector<Table> from, boolTree where,
	std::vector<std::string> gattr, boolTree having,
	vector<pair<std::string, UD>> oattr, vector<string>select, int mode)
{
		Table temp;
		vector<Table> temp1;
	
		if (from.size() == 1)temp = selectRecordSingle(from[0],where);
		else {
			if (mode == 0)
			{
				temp = record->naturalJoin(from[0], from[1],where);//自然连接，只针对两张表，没有where条件
			}
			else 
			{
				temp = record->EqualJoin(from, where);//进行连接，
			}
			
		}
		temp1 = record->group(temp, gattr);
		temp1 = record->having(temp1, having,select);
		temp1 = record->order(temp1, oattr);
		temp = record->select(temp1, select);
	
		//return distinct(temp);
		return temp;
}
//私有函数，用于多条件查询时的or条件合并
Table API::unionTable(Table &table1, Table &table2, std::string target_attr, Where where)
{
	Table result_table(table1);
	std::vector<Tuple>& result_tuple = result_table.getTuple();
	std::vector<Tuple> tuple1 = table1.getTuple();
	std::vector<Tuple> tuple2 = table2.getTuple();
	result_tuple = tuple1;

	//std::vector<Tuple>().swap(result_tuple);

	int i;
	Attribute attr = table1.getAttr();
	for (i = 0; i < 32; i++)
		if (attr.name[i] == target_attr)
			break;

	for (int j = 0; j < tuple2.size(); j++)
		if (!isSatisfied(tuple2[j], i, where))
			result_tuple.push_back(tuple2[j]);

	std::sort(result_tuple.begin(), result_tuple.end(), sortcmp);
	return result_table;
	//std::merge(tuple1.begin(), tuple1.end(), tuple2.begin(), tuple2.end(), std::back_inserter(result_tuple), sortcmp);
	//result_tuple.erase(unique(result_tuple.begin(), result_tuple.end(), calcmp), result_tuple.end());

	//    int i;
	//    for (i = 0; i < tuple1.size() && i < tuple2.size(); i++) {
	//        std::vector<Data> data1 = tuple1[i].getData();
	//        std::vector<Data> data2 = tuple2[i].getData();
	//        
	//        int j;
	//        for (j = 0; j < data1.size(); j++) {
	//            int flag = 0;
	//
	//            switch (data1[i].type) {
	//                case -1: {
	//                    if (data1[i].datai != data2[i].datai) {
	//                        if (data1[i].datai < data2[i].datai)
	//                            flag = 1;
	//                        else
	//                            flag = -1;
	//                    }
	//                } break;
	//                case 0: {
	//                    if (data1[i].dataf != data2[i].dataf) {
	//                        if (data1[i].dataf < data2[i].dataf)
	//                            flag = 1;
	//                        else
	//                            flag = -1;
	//                    }
	//                } break;
	//                default: {
	//                    if (data1[i].datas < data2[i].datas)
	//                        flag = 1;
	//                    else
	//                        flag = -1;
	//                } break;
	//            }
	//            
	//            if (flag)
	//                break;
	//        }
	//        
	//        switch (flag) {
	//            case 1:
	//        }
	//    }
}

//私有函数，用于多条件查询时的and条件合并
Table API::joinTable(Table &table1, Table &table2, std::string target_attr, Where where)
{
	Table result_table(table1);
	std::vector<Tuple>& result_tuple = result_table.getTuple();
	std::vector<Tuple> tuple1 = table1.getTuple();
	std::vector<Tuple> tuple2 = table2.getTuple();

	for (int index = 0; index < table1.tuple_.size(); index++)
		result_table.tuple_.pop_back();

	int i;
	Attribute attr = table1.getAttr();
	for (i = 0; i < 32; i++)
		if (attr.name[i] == target_attr)
			break;

	for (int j = 0; j < tuple2.size(); j++)
		if (isSatisfied(tuple2[j], i, where))
			result_tuple.push_back(tuple2[j]);

	std::sort(result_tuple.begin(), result_tuple.end(), sortcmp);
	return result_table;
	//	std::vector<Tuple>().swap(result_tuple);
	//	std::sort(tuple1.begin(), tuple1.end(), sortcmp);
	//	std::sort(tuple2.begin(), tuple2.end(), sortcmp);

	//std::set_intersection(tuple1.begin(), tuple1.end(), tuple2.begin(), tuple2.end(), std::back_inserter(result_tuple), calcmp);

	//return result_table;
}
Table API::selectRecordSingle(Table table, boolTree tree)
{
	Table target_table = table;
	vector<Tuple>& tuples = target_table.getTuple();
	vector<int> indexes;

	vector<Attribute>attrs;

	Attribute attr = target_table.getAttr();

	attrs.push_back(attr);
	tree.pushArrributes(attrs);
	/*for (int i = 0; i < attr.num; i++)
	{
		attr.name[i] = table.getTitle() + "." + attr.name[i] ;
	}*/
	vector<string> tree_attr = tree.getAttributeList();
		//获取目标属性对应的编号,记录下来
	for (int i = 0; i < tree_attr.size(); i++) {
		int j;
		for (j = 0; j < attr.num; j++)
		{
			if (attr.name[j] == tree_attr[i])//比较时要注意属性名带上表名
			{
				indexes.push_back(j); break;
			}
		}
		if (j == attr.num)throw attribute_not_exist();

	}

	vector<Tuple>::iterator it = tuples.begin();
	//vector<Tuple>::iterator ithelp = tuples.begin();
	while (it != tuples.end())
	{
		if (!TuplesatisTree(*it, tree, indexes))
		{
			
			it = tuples.erase(it);
			
		}
		else it++;
	}
	target_table.attr_ = attr;
	return target_table;

}
//用于对vector的sort时排序
bool sortcmp(const Tuple &tuple1, const Tuple &tuple2)
{
	std::vector<Data> data1 = tuple1.getData();
	std::vector<Data> data2 = tuple2.getData();

	switch (data1[0].type) {
	case -1: return data1[0].datai < data2[0].datai;
	case 0: return data1[0].dataf < data2[0].dataf;
	default: return data1[0].datas < data2[0].datas;
	}
}

//用于对vector对合并时排序
bool calcmp(const Tuple &tuple1, const Tuple &tuple2)
{
	int i;

	std::vector<Data> data1 = tuple1.getData();
	std::vector<Data> data2 = tuple2.getData();

	for (i = 0; i < data1.size(); i++) {
		bool flag = false;
		switch (data1[0].type) {
		case -1: {
			if (data1[0].datai != data2[0].datai)
				flag = true;
		}break;
		case 0: {
			if (data1[0].dataf != data2[0].dataf)
				flag = true;
		}break;
		default: {
			if (data1[0].datas != data2[0].datas)
				flag = true;
		}break;
		}
		if (flag)
			break;
	}


	if (i == data1.size())
		return true;
	else
		return false;
}

bool isSatisfied(Tuple& tuple, int target_attr, Where where)
{
	std::vector<Data> data = tuple.getData();

	switch (where.relation_character) {
	case LESS: {
		switch (where.data.type) {
		case -1: return (data[target_attr].datai < where.data.datai); break;
		case 0: return (data[target_attr].dataf < where.data.dataf); break;
		default: return (data[target_attr].datas < where.data.datas); break;
		}
	} break;
	case LESS_OR_EQUAL: {
		switch (where.data.type) {
		case -1: return (data[target_attr].datai <= where.data.datai); break;
		case 0: return (data[target_attr].dataf <= where.data.dataf); break;
		default: return (data[target_attr].datas <= where.data.datas); break;
		}
	} break;
	case EQUAL: {
		switch (where.data.type) {
		case -1: return (data[target_attr].datai == where.data.datai); break;
		case 0: return (data[target_attr].dataf == where.data.dataf); break;
		default: return (data[target_attr].datas == where.data.datas); break;
		}
	} break;
	case GREATER_OR_EQUAL: {
		switch (where.data.type) {
		case -1: return (data[target_attr].datai >= where.data.datai); break;
		case 0: return (data[target_attr].dataf >= where.data.dataf); break;
		default: return (data[target_attr].datas >= where.data.datas); break;
		}
	} break;
	case GREATER: {
		switch (where.data.type) {
		case -1: return (data[target_attr].datai > where.data.datai); break;
		case 0: return (data[target_attr].dataf > where.data.dataf); break;
		default: return (data[target_attr].datas > where.data.datas); break;
		}
	} break;
	case NOT_EQUAL: {
		switch (where.data.type) {
		case -1: return (data[target_attr].datai != where.data.datai); break;
		case 0: return (data[target_attr].dataf != where.data.dataf); break;
		default: return (data[target_attr].datas != where.data.datas); break;
		}
	} break;
	default:break;
	}

	return false;
}
bool isInTable(Table table, Tuple t) {
	std::vector<Tuple> t1 = table.getTuple();
	int length = t.getSize();
	for (int i = 0; i < t1.size(); i++) {
		std::vector<Data>data1 = t1[i].getData();
		std::vector<Data>data2 = t.getData();
		if (isEqual(data1, data2)) {
			return true;
		}

	}
	return false;
}
bool isEqual(vector<Data>data1, vector<Data>data2) {
	for (int i = 0; i < data1.size(); i++) {
		if (data1[i].type != data2[i].type) {
			return false;
		}
		else {
			switch (data1[i].type)
			{
			case -1:
				if (data1[i].datai != data2[i].datai)
					return false;
				break;
			case 0:
				if (data1[i].dataf != data2[i].dataf)
					return false;
				break;
			default:
				if (data1[i].datas != data2[i].datas)
					return false;
				break;
			}
		}
	}
	return true;
}

//根据除法得到整形的数字的长度
int getBits(int num) {
	int bit = 0;
	if (num == 0)
		return 1;
	if (num < 0) {
		bit++;
		num = -num;
	}
	while (num != 0) {
		num /= 10;
		bit++;
	}
	return bit;
}

//根据除法得到小数的数字的长度
int getBits(float num) {
	int bit = 0;
	if ((int)num == 0)
		return 4;
	if (num < 0) {
		bit++;
		num = -num;
	}
	int integer_part = num;
	while (integer_part != 0) {
		bit++;
		integer_part /= 10;
	}
	return bit + 3;//为了保留小数点的后几位
}

