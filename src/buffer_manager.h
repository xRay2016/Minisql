
#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_ 1
#define _SCL_SECURE_NO_WARNINGS 

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include "const.h"
using namespace std;

// Page�ࡣ�����ļ��е�ÿһ���Ӧ�ڴ��е�һ��ҳ��page)
class Page {
public:
	// ���캯����һЩ��ȡ���ƺ������ɺ��ԡ�
	Page();
	void initialize();
	void setFileName(std::string file_name);
	std::string getFileName();
	void setBlockId(int block_id);
	int getBlockId();
	void setPinCount(int pin_count);
	int getPinCount();
	void setDirty(bool dirty);
	bool getDirty();
	void setRef(bool ref);
	bool getRef();
	void setAvaliable(bool avaliable);
	bool getAvaliable();
	char* getBuffer();
private:
	char buffer_[PAGESIZE];//ÿһҳ����һ����СΪPAGESIZE�ֽڵ�����
	std::string file_name_;//ҳ����Ӧ���ļ���
	int block_id_;//ҳ�������ļ��еĿ��(������ͨ���п�)
	int pin_count_;//��¼����ס�Ĵ���������ס����˼���ǲ����Ա��滻
	bool dirty_;//dirty��¼ҳ�Ƿ��޸�
	bool ref_;//ref��������ʱ���滻����
	bool avaliable_;//avaliable��ʾҳ�Ƿ���Ա�ʹ��(�������̿�load����ҳ)
};

// BufferManager�ࡣ�����ṩ�����������Ľӿڡ�
class BufferManager {
public:
	//  ���캯��
	BufferManager();
	BufferManager(int frame_size);
	// ��������
	~BufferManager();
	// ͨ��ҳ�ŵõ�ҳ�ľ��(һ��ҳ��ͷ��ַ)
	char* getPage(std::string file_name, int block_id);
	// ���page_id����Ӧ��ҳ�Ѿ����޸�
	void modifyPage(int page_id);
	// ��סһ��ҳ
	void pinPage(int page_id);
	// ���һ��ҳ�Ķ�ס״̬(��Ҫע�����һ��ҳ���ܱ���ζ�ס���ú���ֻ�ܽ��һ��)
	// �����Ӧҳ��pin_count_Ϊ0���򷵻�-1
	int unpinPage(int page_id);
	// ����Ӧ�ڴ�ҳд���Ӧ�ļ��Ķ�Ӧ�顣����ķ���ֵΪint�����о���ʵûʲô�ã�������Ϊvoid
	int flushPage(int page_id, std::string file_name, int block_id);
	// ��ȡ��Ӧ�ļ��Ķ�Ӧ�����ڴ��е�ҳ�ţ�û���ҵ�����-1
	int getPageId(std::string file_name, int block_id);
	Page * Frames;//����أ�ʵ���Ͼ���һ��Ԫ��ΪPage�����飬ʵ���ڴ�ռ佫�����ڶ���
private:
	//Page * Frames;//����أ�ʵ���Ͼ���һ��Ԫ��ΪPage�����飬ʵ���ڴ�ռ佫�����ڶ���
	int frame_size_;//��¼��ҳ��
	int current_position_;//ʱ���滻������Ҫ�õ��ı���
	void initialize(int frame_size);//ʵ�ʳ�ʼ������
									// ��ȡһ�����õ�ҳ��ҳ��(�ڲ���װ��ʱ���滻���ԣ���ʹ���߲���Ҫ֪����Щ)
	int getEmptyPageId();
	// ����Ӧ�ļ��Ķ�Ӧ�������Ӧ�ڴ�ҳ�������ļ������ڷ���-1�����򷵻�0
	int loadDiskBlock(int page_id, std::string file_name, int block_id);
};

#endif

