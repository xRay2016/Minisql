#pragma once
#ifndef ERROR
#define ERROR
#include<exception>
using namespace std;

extern class API;
extern class Database;
extern struct SelectTreeNode;
extern class SelectTree;
extern struct boolTreeNode;
extern class boolTree;
extern class RecordManager;
extern class BufferManager;
extern class CatalogManager;
extern class IndexManager;
extern class Optimizer;
//extern struct SelectParameter;

class sub_select_fail : public exception {

};

class file_not_exist : public exception {

};

class Insert_error :public exception {

};

class type_conflict :public exception
{

};

class Too_many_col :public exception
{

};

class InputError :public exception
{

};

class GrantError :public exception
{

};

class String_Illegal :public exception
{

};

class VarNotDefine :public exception
{

};

class VarAmbiguity :public exception
{

};

class table_exist : public std::exception {

};

class table_not_exist : public std::exception {

};

class attribute_not_exist : public std::exception {

};

class index_exist : public std::exception {

};

class index_not_exist : public std::exception {

};

class tuple_type_conflict : public std::exception {

};

class primary_key_conflict : public std::exception {

};

class data_type_conflict : public std::exception {

};

//Ôö¼Ó
class index_full : public std::exception {

};

class input_format_error : public std::exception {

};

class exit_command : public std::exception {

};

class unique_conflict :public std::exception {

};



#endif // !ERROR
