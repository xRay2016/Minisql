#define _CRT_SECURE_NO_WARNINGS 
#include "buffer_manager.h"


// Page���ʵ��
Page::Page() {
	initialize();
}
// ��ʼ��
void Page::initialize() {
	file_name_ = "";
	block_id_ = -1;
	pin_count_ = -1;
	dirty_ = false;
	ref_ = false;
	avaliable_ = true;
	for (int i = 0; i < PAGESIZE; i++)
		buffer_[i] = '\0';
}

// ������һЩ��ȡ���ƺ�������Ϊ�򵥾Ͳ�׸����
inline void Page::setFileName(std::string file_name) {
	file_name_ = file_name;
}

inline std::string Page::getFileName() {
	return file_name_;
}

inline void Page::setBlockId(int block_id) {
	block_id_ = block_id;
}

inline int Page::getBlockId() {
	return block_id_;
}

inline void Page::setPinCount(int pin_count) {
	pin_count_ = pin_count;
}

inline int Page::getPinCount() {
	return pin_count_;
}

inline void Page::setDirty(bool dirty) {
	dirty_ = dirty;
}

inline bool Page::getDirty() {
	return dirty_;
}

inline void Page::setRef(bool ref) {
	ref_ = ref;
}

inline bool Page::getRef() {
	return ref_;
}

inline void Page::setAvaliable(bool avaliable) {
	avaliable_ = avaliable;
}

inline bool Page::getAvaliable() {
	return avaliable_;
}

inline char* Page::getBuffer() {
	return buffer_;
}

// BufferManager���ʵ��
// ���캯��������ʵ�ʳ�ʼ��������ɳ�ʼ��
BufferManager::BufferManager() {
	initialize(MAXFRAMESIZE);
}

BufferManager::BufferManager(int frame_size) {
	initialize(frame_size);
}

// ���������ǳ���Ҫ���ڳ������ʱ��Ҫ��������������ҳд�ش��̡�
BufferManager::~BufferManager() {
	for (int i = 0; i < frame_size_; i++) {
		std::string file_name;
		int block_id;
		file_name = Frames[i].getFileName();
		block_id = Frames[i].getBlockId();
		flushPage(i, file_name, block_id);
	}
}

// ʵ�ʳ�ʼ������
void BufferManager::initialize(int frame_size) {
	Frames = new Page[frame_size];//�ڶ��Ϸ����ڴ�
	frame_size_ = frame_size;
	current_position_ = 0;
}

// ���漸��������Ϊ�򵥣�Ҳ��׸����
char* BufferManager::getPage(std::string file_name, int block_id) {
	int page_id = getPageId(file_name, block_id);
	if (page_id == -1) {
		page_id = getEmptyPageId();
		loadDiskBlock(page_id, file_name, block_id);
	}
	Frames[page_id].setRef(true);
	return Frames[page_id].getBuffer();
}

void BufferManager::modifyPage(int page_id) {
	Frames[page_id].setDirty(true);
}

void BufferManager::pinPage(int page_id) {
	int pin_count = Frames[page_id].getPinCount();
	Frames[page_id].setPinCount(pin_count + 1);
}

int BufferManager::unpinPage(int page_id) {
	int pin_count = Frames[page_id].getPinCount();
	if (pin_count <= 0)
		return -1;
	else
		Frames[page_id].setPinCount(pin_count - 1);
	return 0;
}

// ���ĺ���֮һ���ڴ�ʹ��̽����Ľӿڡ�
int BufferManager::loadDiskBlock(int page_id, std::string file_name, int block_id) {
	// ��ʼ��һ��ҳ
	Frames[page_id].initialize();
	// �򿪴����ļ�
	FILE* f = fopen((file_name).c_str(), "r");
	// ��ʧ�ܷ���-1
	if (f == NULL)
		return -1;
	// ���ļ�ָ�붨λ����Ӧλ��
	fseek(f, PAGESIZE * block_id, SEEK_SET);
	// ��ȡҳ�ľ��
	char* buffer = Frames[page_id].getBuffer();
	// ��ȡ��Ӧ���̿鵽�ڴ�ҳ
	fread(buffer, PAGESIZE, 1, f);
	// �ر��ļ�
	fclose(f);
	// ���������ҳ������Ӧ����
	Frames[page_id].setFileName(file_name);
	Frames[page_id].setBlockId(block_id);
	Frames[page_id].setPinCount(1);
	Frames[page_id].setDirty(false);
	Frames[page_id].setRef(true);
	Frames[page_id].setAvaliable(false);
	return 0;
}

// ���ĺ���֮һ���ڴ�ʹ��̽����Ľӿڡ�
int BufferManager::flushPage(int page_id, std::string file_name, int block_id) {
	// ���ļ�
	FILE* f = fopen((file_name).c_str(), "r+");
	// ��ʵ������д���࣬��Ϊ��һ���ļ��������ܳɹ���
	if (f == NULL)
		return -1;
	// ���ļ�ָ�붨λ����Ӧλ��
	fseek(f, PAGESIZE * block_id, SEEK_SET);
	// ��ȡҳ�ľ��
	char* buffer = Frames[page_id].getBuffer();
	// ���ڴ�ҳ������д����̿�
	fwrite(buffer, PAGESIZE, 1, f);
	// �ر��ļ�
	fclose(f);
	return 0;
}

// �򵥱�����ȡҳ��
int BufferManager::getPageId(std::string file_name, int block_id) {
	for (int i = 0; i < frame_size_; i++) {
		std::string tmp_file_name = Frames[i].getFileName();
		int tmp_block_id = Frames[i].getBlockId();
		if (tmp_file_name == file_name && tmp_block_id == block_id)
			return i;
	}
	return -1;
}

// Ѱ��һ�����õ�ҳ
int BufferManager::getEmptyPageId() {
	// �ȼ򵥵ı���һ�飬��������õ�ֱ�ӷ�����ҳ��
	for (int i = 0; i < frame_size_; i++) {
		if (Frames[i].getAvaliable() == true)
			return i;
	}
	// �������ҳ���Ѿ���ʹ�ã���ô��Ҫ�ҵ�һ��ҳ������ɾ������
	// ������Ҫʹ��һЩ������ѡ����һ��ҳӦ�ñ�ɾ����
	// �������в���ʱ���滻���ԡ�
	while (1) {
		// ���ҳ��refΪtrue��������Ϊfalse
		if (Frames[current_position_].getRef() == true) {
			Frames[current_position_].setRef(false);
		}
		// �������ҳ��Ӧ��pin_countΪ0����ҳû�б���ס����ô��һҳ��
		// �ᱻɾ����
		else if (Frames[current_position_].getPinCount() == 0) {
			// �����һҳ���Ķ�������Ҫ����д�ش��̣�Ȼ��ɾ��
			if (Frames[current_position_].getDirty() == true) {
				std::string file_name = Frames[current_position_].getFileName();
				int block_id = Frames[current_position_].getBlockId();
				flushPage(current_position_, file_name, block_id);
			}
			// ɾ��ҳ
			Frames[current_position_].initialize();
			// ����ҳ��
			return current_position_;
		}
		// ʱ��ָ��˳ʱ��ת��
		current_position_ = (current_position_ + 1) % frame_size_;
	}
}