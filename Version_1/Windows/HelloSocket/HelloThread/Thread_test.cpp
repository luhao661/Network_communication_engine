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
		//detach()��⣺
		//�� thread �������ִ���̣߳�
		// ����ִ�ж����س�����һ�����߳��˳������ͷ��κη������Դ��
		//������ detach() ����������ζ�ŷ����˶��̵߳Ŀ���������
		//�˺����̺߳ͷ�����߳�֮�佫�໥�������У����̲߳��ٵȴ�������߳�ִ����ɡ�
		//���̲߳�����Ҫ���ĸ��̵߳�״̬�����ʱ����
		//�磺�ڷ���󣬲����ܹ��� t ���� t1.join()
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


//��
#if 0
#include <iostream>
#include <thread>
#include <mutex>//��
using namespace std;

mutex m;

void f1(int number)
{
	for (int n = 0; n < 30; ++n)
	{
		m.lock();//�ٽ����򡪡���ʼ
		cout << number << " Hello, this is other thread."
			<< endl;
		m.unlock();//�ٽ����򡪡�����
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


//������
#if 0
#include <iostream>
#include <thread>
#include <mutex>//��
#include "Timestamp.hpp"
using namespace std;

mutex m;
int g_bSum;

void f1(int number)
{
	for (int n = 0; n < 30000000; ++n)
	{
		m.lock();//�ٽ����򡪡���ʼ
		++g_bSum;
		m.unlock();//�ٽ����򡪡�����
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


//�Խ���
#if 0
#include <iostream>
#include <thread>
#include <mutex>//��
#include "Timestamp.hpp"
using namespace std;

mutex m;
int g_bSum;

void f1(int number)
{
	for (int n = 0; n < 30000000; ++n)
	{
		//�Խ���
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


//ԭ�Ӳ���
#if 1
#include <iostream>
#include <thread>
#include <mutex>//��
#include <atomic>
#include "Timestamp.hpp"
using namespace std;

mutex m;
//int g_bSum;

atomic_int g_bSum;
//��дΪ
//atomic<int> g_bSum;

void f1(int number)
{
	for (int n = 0; n < 30000000; ++n)
	{
		//�Խ���
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