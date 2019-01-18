

#include "basic.h"
#include<string>

Tuple::Tuple(const Tuple &tuple_in) {

	for (int index = 0; index < tuple_in.data_.size(); index++)
	{
		this->data_.push_back(tuple_in.data_[index]);
	}
	this->isDeleted_ = tuple_in.isDeleted_;
}

inline int Tuple::getSize() {
	return (int)data_.size();
}

//��������
void Tuple::addData(Data data_in) {
	this->data_.push_back(data_in);
}

bool Tuple::isDeleted() {
	return isDeleted_;
}

void Tuple::setDeleted() {
	isDeleted_ = true;
}

//�õ�Ԫ���е�����
std::vector<Data> Tuple::getData() const {
	return this->data_;
}

void Tuple::setData(int index, Data data)
{
	if (data_[index].type != data.type)
	{
		throw data_type_conflict();
	}
	else
		data_[index] = data;
}

void Tuple::showTuple() {
	for (int index = 0; index < getSize(); index++) {
		if (data_[index].type == -1)
			std::cout << data_[index].datai << '\t';
		else if (data_[index].type == 0)
			std::cout << data_[index].dataf << '\t';
		else
			std::cout << data_[index].datas << '\t';
	}
	std::cout << std::endl;
}

bool Tuplecmp(Tuple& tuple1, Tuple& tuple2)
{
	if (tuple1.getSize() != tuple2.getSize())
	{
		return false;
	}
	std::vector<Data> datas1 = tuple1.getData();
	std::vector<Data> datas2 = tuple2.getData();

	for (int i = 0; i < tuple1.getSize(); i++)
	{
		if (!Datacmp(datas1[i], datas2[i]))
		{
			return false;
		};
	}
}



//table���캯��
Table::Table(std::string title, Attribute attr) {
	this->title_ = title;
	this->attr_ = attr;
	this->index_.num = 0;
}

//table�Ĺ��캯����������
Table::Table(const Table &table_in) {
	this->attr_ = table_in.attr_;
	this->index_ = table_in.index_;
	this->title_ = table_in.title_;
	for (int index = 0; index < table_in.tuple_.size(); index++)
		this->tuple_.push_back(table_in.tuple_[index]);
}

//�������Թ����
Table::Table(Attribute attr) {
	this->title_ = "tem_table";
	this->attr_ = attr;
}

//����Ԫ��
// int Table::addTuple(Tuple tuple_in){
//     if(tuple_in.getSize()!=attr_.num)
//     {
//         std::cout<<"Illegal Tuple Insert: The size of column is unequal."<<std::endl;
//         return 0;
//     }
//     for(int index=0;index<attr_.num;index++){
//         //�������tuple��typeΪint����floatʱ���������Ӧ��attr_.type��ͬ�����Ϊstring�Ļ�������Ҫ������attr_.type
//         if(tuple_in.getData()[index].type>attr_.type[index]||(tuple_in.getData()[index].type<=0&&tuple_in.getData()[index].type!=attr_.type[index]))
//         {
//             std::cout<<"Illegal Tuple Insert: The types of attributes are unequal."<<std::endl;
//             return 0;
//         }
//     }
//     tuple_.push_back(tuple_in);
//     return 1;
// }

//��������
int Table::setIndex(short index, std::string index_name) {
	short tmpIndex;
	for (tmpIndex = 0; tmpIndex < index_.num; tmpIndex++) {
		if (index == index_.location[tmpIndex])  //����Ԫ���Ѿ�������ʱ������
		{
			std::cout << "Illegal Set Index: The index has been in the table." << std::endl;
			return 0;
		}
	}
	for (tmpIndex = 0; tmpIndex < index_.num; tmpIndex++) {
		if (index_name == index_.indexname[tmpIndex])  //����Ԫ���Ѿ�������ʱ������
		{
			std::cout << "Illegal Set Index: The name has been used." << std::endl;
			return 0;
		}
	}
	index_.location[index_.num] = index;  //��������λ�ú��������֣���������������һ
	index_.indexname[index_.num] = index_name;
	index_.num++;
	return 1;
}

int Table::dropIndex(std::string index_name) {
	short tmpIndex;
	for (tmpIndex = 0; tmpIndex < index_.num; tmpIndex++) {
		if (index_name == index_.indexname[tmpIndex])  //����Ԫ���Ѿ�������ʱ������
			break;
	}
	if (tmpIndex == index_.num)
	{
		std::cout << "Illegal Drop Index: No such a index in the table." << std::endl;
		return 0;
	}

	//������������λ�ú����֣����ﵽɾ����Ч��
	index_.indexname[tmpIndex] = index_.indexname[index_.num - 1];
	index_.location[tmpIndex] = index_.location[index_.num - 1];
	index_.num--;
	return 1;
}

//����ÿ��attribute�Ĵ�С
// int Table::DataSize(){
//     int result=0;
//     for(int index=0;index<attr_.num;index++){
//         switch (attr_.type[index]) {
//             case -1:
//                 result+=sizeof(int);
//                 break;
//             case 0:
//                 result+=sizeof(float);
//                 break;
//             default:
//                 result+=attr_.type[index];
//                 break;
//         }
//     }
//     return result;
// }

//����һЩprivate��ֵ
std::string Table::getTitle() {
	return title_;
}
Attribute Table::getAttr() {
	return attr_;
}
std::vector<Tuple>& Table::getTuple() {
	return tuple_;
}
Index Table::getIndex() {
	return index_;
}


void Table::showTable() {
	for (int index = 0; index < attr_.num; index++)
		std::cout << attr_.name[index] << '\t';
	std::cout << std::endl;
	for (int index = 0; index < tuple_.size(); index++)
		tuple_[index].showTuple();
}

void Table::showTable(int limit) {
	for (int index = 0; index < attr_.num; index++)
		std::cout << attr_.name[index] << '\t';
	std::cout << std::endl;
	for (int index = 0; index < limit&&index < tuple_.size(); index++)
		tuple_[index].showTuple();
}

bool Datacmp(Data data1, Data data2)
{
	if (data1.type != data2.type)
	{
		std::cout << "���Բ���ͬ�������޷����бȽ�" << std::endl;
	}
	else {
		if (data1.type == -1)
		{
			return data1.datai == data2.datai;
		}
		else if (data1.type == 0)
		{
			return data1.dataf == data2.dataf;
		}
		else
		{
			return data1.datas == data2.datas;
		}
	}

}
