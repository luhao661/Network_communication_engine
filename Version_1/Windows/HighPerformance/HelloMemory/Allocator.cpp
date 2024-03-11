#if 1
#include "Allocator.h"
#include <stdlib.h>
#include "MemoryMgr.hpp"

void* operator new(size_t size)
{
	//由于静态资源在类初始化后就存储在静态存储区
	//可以通过类名来调用类的静态成员函数
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