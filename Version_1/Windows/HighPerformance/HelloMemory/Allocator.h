#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

//����new ��new[]   delete��delete[]
void* operator new(size_t size);
void operator delete(void* p);

void* operator new[](size_t size);
void operator delete[](void* p);

//���µĺ���ȥ����ϵͳ�ṩ��malloc��free
void* mem_alloc(size_t size);
void mem_free(void* p);


#endif  /*_ALLOCATOR_H*/