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
		//detach()理解：
		//从 thread 对象分离执行线程，
		// 允许执行独立地持续。一旦该线程退出，则释放任何分配的资源。
		//调用了 detach() 方法，就意味着放弃了对线程的控制能力。
		//此后，主线程和分离的线程之间将相互独立运行，主线程不再等待分离的线程执行完成。
		//主线程不再需要关心该线程的状态或结束时机。
		//如：在分离后，不再能够对 t 进行 t1.join()
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


//锁
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


//锁消耗
#if 0
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


//自解锁
#if 0
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
		//自解锁
		lock_guard<mutex>lg(m);
		++g_bSum;
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


//原子操作
#if 1
#include <iostream>
#include <thread>
#include <mutex>//锁
#include <atomic>
#include "Timestamp.hpp"
using namespace std;

mutex m;
//int g_bSum;

atomic_int g_bSum;
//或写为
//atomic<int> g_bSum;

void f1(int number)
{
	for (int n = 0; n < 30000000; ++n)
	{
		//自解锁
		//lock_guard<mutex>lg(m);
		++g_bSum;
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