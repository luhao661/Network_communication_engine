//�ڵ��߳��½��в���
#if 0
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
#endif


//�ڶ��߳��½��в���
#if 1
#include <iostream>
#include <thread>
#include "Timestamp.hpp"

#include "Allocator.h"

using namespace std;

const int tCnt = 4;
const int NumberOfApplications = 100000;
const int ApplicationsPerThread = NumberOfApplications / tCnt;

void f1(int number)
{
	char* data[ApplicationsPerThread]{nullptr};

	for (int i = 0; i < ApplicationsPerThread; ++i)
	{
		data[i] = new char[(rand()%128)+1];
	}

	for (int i = 0; i < ApplicationsPerThread; ++i)
	{
		delete[](data[i]);
	}
}

int main()
{
	thread* pThread[tCnt];
	for (int i = 0; i < tCnt; ++i)
	{
		pThread[i] = new thread(f1, i);
	}

	Timestamp timestamp;
	for (int i = 0; i < tCnt; ++i)
	{
		pThread[i]->join();
	}

	cout << timestamp.getElapsedTimeInMillisecond() << endl;

	return 0;
}
#endif