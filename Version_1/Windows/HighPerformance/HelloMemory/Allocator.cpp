#if 1
#include "Allocator.h"
#include <stdlib.h>
#include "MemoryMgr.hpp"

void* operator new(size_t size)
{
	//���ھ�̬��Դ�����ʼ����ʹ洢�ھ�̬�洢��
	//����ͨ��������������ľ�̬��Ա����
	return MemoryMgr::Instance().allocMem(size);
}

void operator delete(void* p)
{
	MemoryMgr::Instance().freeMem(p);

	return;
}


void* operator new[](size_t size)
{
	return MemoryMgr::Instance().allocMem(size);
}

void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMem(p);

	return;
}

void* mem_alloc(size_t size)
{
	return malloc(size);
}
void mem_free(void* p)
{
	free(p);

	return;
}

#endif