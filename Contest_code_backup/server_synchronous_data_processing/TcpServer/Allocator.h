#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

//重载new 和new[]   delete和delete[]
void* operator new(size_t size);
void operator delete(void* p);

void* operator new[](size_t size);
void operator delete[](void* p);

//用新的函数去调用系统提供的malloc和free
void* mem_alloc(size_t size);
void mem_free(void* p);

 
#endif  /*_ALLOCATOR_H*/ 
