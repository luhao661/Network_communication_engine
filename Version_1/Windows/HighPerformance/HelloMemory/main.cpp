#include <iostream>
#include "Allocator.h"

int main()
{
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


	return 0;
}