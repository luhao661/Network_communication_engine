#include <iostream>
#include "Allocator.h"

int main()
{
#if 0
	//��̬�����ڴ�ռ��������ʽ��
	//1.
	char* pData1 = new char[128];
	delete [] pData1;	
	//2.
	char* pData2 = new char;
	delete pData2;
	//3.
	char* pData3 = new char[64];
	delete [] pData3;
#endif

#if 0
	//һ������ͷ�
	char* data[12];
	for (int i = 0; i < 12; ++i)
	{
		data[i] = new char[60];
		delete[](data[i]);
	}
#endif

#if 1
	//���������ͳһ�ͷ�
	char* data[1000];
	for (int i = 0; i < 1000; ++i)
	{
		data[i] = new char[1+i];
	}

	for (int i = 0; i < 1000; ++i)
	{
		delete[](data[i]);
	}
#endif

	return 0;
}