#include <iostream>
#include "Allocator.h"

int main()
{
	//动态分配内存空间的三种形式：
	//1.
	char* pData1 = new char[128];
	delete [] pData1;	
	//2.
	char* pData2 = new char;
	delete pData2;
	//3.
	char* pData3 = (char*)std::malloc(sizeof(char[128]));
	free(pData3);


	return 0;
}