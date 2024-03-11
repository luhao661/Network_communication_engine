#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

void* operator new(size_t size);
void operator delete(void* p);

void* operator new[](size_t size);
void operator delete[](void* p);

//���µĺ���ȥ��װ���ص�new�������
void* mem_alloc(size_t size);
void mem_free(void* p);


#endif  /*_ALLOCATOR_H*/