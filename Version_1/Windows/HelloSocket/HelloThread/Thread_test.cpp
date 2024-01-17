#if 0
#include <iostream>
#include <thread>
using namespace std;

void f1(int number)
{
	for(int n=0;n<number;++n)
	cout << "Hello, this is other thread." << endl;
}

int main()
{
	//thread t(f1,5);

	thread* pThread[3];
	for (int i = 0; i < 3; ++i)
	{
		pThread[i] = new thread(f1, i);
		pThread[i]->detach();
	}


	//t.join();

	for (int n = 0; n < 5; ++n)
	cout << "Hello, this is main thread." << endl;

	return 0;
}
#endif


#if 0
#include <iostream>
#include <thread>
using namespace std;

void f1(int number)
{
	for (int n = 0; n < 5; ++n)
		cout <<number<< " Hello, this is other thread."
		<< endl;
}

int main()
{
	//thread t(f1,5);

	thread* pThread[3];
	for (int i = 0; i < 3; ++i)
	{
		pThread[i] = new thread(f1, i);
	}

	for (int i = 0; i < 3; ++i)
	{
		pThread[i]->join();
	}

	for (int n = 0; n < 5; ++n)
		cout << "Hello, this is main thread." << endl;

	return 0;
}
#endif


#if 0
#include <iostream>
#include <thread>
#include <mutex>//锁
using namespace std;

mutex m;

void f1(int number)
{
	for (int n = 0; n < 30; ++n)
	{
		m.lock();//临界区域——开始
		cout << number << " Hello, this is other thread."
			<< endl;
		m.unlock();//临界区域——结束
	}
}

int main()
{
	//thread t(f1,5);

	thread* pThread[3];
	for (int i = 0; i < 3; ++i)
	{
		pThread[i] = new thread(f1, i);
	}

	for (int i = 0; i < 3; ++i)
	{
		pThread[i]->join();
	}

	for (int n = 0; n < 5; ++n)
		cout << "Hello, this is main thread." << endl;

	return 0;
}
#endif


#if 1
#include <iostream>
#include <thread>
#include <mutex>//锁
#include "Timestamp.hpp"
using namespace std;

mutex m;
int g_bSum;

void f1(int number)
{
	for (int n = 0; n < 30000000; ++n)
	{
		m.lock();//临界区域——开始
		++g_bSum;
		m.unlock();//临界区域——结束
	}
}

int main()
{
	//thread t(f1,5);

	thread* pThread[3];
	for (int i = 0; i < 3; ++i)
	{
		pThread[i] = new thread(f1, i);
	}

	Timestamp timestamp;
	for (int i = 0; i < 3; ++i)
	{
		pThread[i]->join();
	}

	//for (int n = 0; n < 5; ++n)
	//	cout << "Hello, this is main thread." << endl;

	cout << timestamp.getElapsedTimeInMillisecond() << endl;
	cout << g_bSum << endl;

	g_bSum = 0;
	timestamp.update();
	for (int i = 0; i < 90000000; ++i)
		++g_bSum;

	cout << timestamp.getElapsedTimeInMillisecond() << endl;
	cout << g_bSum << endl;

	return 0;
}
#endif