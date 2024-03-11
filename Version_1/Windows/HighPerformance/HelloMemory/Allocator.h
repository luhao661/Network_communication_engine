#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

void* operator new(size_t size);
void operator delete(void* p);

void* operator new[](size_t size);
void operator delete[](void* p);

//用新的函数去包装重载的new等运算符
void* mem_alloc(size_t size);
void mem_free(void* p);


#endif  /*_ALLOCATOR_H*/