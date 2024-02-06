#if 1
#include "EasyTcpClient_1.6.hpp"
#include <thread>
#include <vector>
bool g_bRun = true;
void cmdThread(void);
void SendThread(int id);

//全局变量存放在数据段的全局变量区域

//所有线程的客户端总量
const int Num_clients = 8;
//发送线程数量
const int T_cnt = 4;
//客户端数组
EasyTcpClient* pClientArray[Num_clients];

int main()
{
	//启动UI线程
	thread uiThread(cmdThread);//格式：函数名  参数
	uiThread.detach();

	//启动线程
	vector<thread> threads;
	for (int i = 0; i < T_cnt; ++i)
	{
		threads.emplace_back(SendThread, i + 1);
	}

	while (g_bRun)
	{
		Sleep(100);
	}

	cout << "客户端已退出，任务结束。\n";

	return 0;
}

void cmdThread(void)
{
	char cmdBuf[256]{};

	//cin 和 scanf 都是阻塞式函数，
	// 而select()非阻塞，这样会造成运行逻辑冲突，所以要使用多线程

	while (1)
	{
		cin.getline(cmdBuf, 256);

		// 处理请求
		if (!strcmp(cmdBuf, "exit"))
		{
			cout << "收到退出命令，退出cmdThread线程\n";
			g_bRun = false;
			break;
		}
		else
			cout << "未识别的命令，请重新输入！\n";
	}
}

//一种处理是在线程函数内动态分配存储空间来存储创建的客户端
//但由线程函数内的指针来管理
//缺点是其他线程无法取得客户端资源，无法访问客户端
#if 0
void SendThread(int id)
{
	//计算每个线程要承担创建的客户端数量
	int clients_per_thread = Num_clients / T_cnt;

	shared_ptr < EasyTcpClient >ClientArray
	(new EasyTcpClient[clients_per_thread], default_delete<EasyTcpClient[]>());

	for (int i = 0; i < clients_per_thread; ++i)
	{
		(ClientArray.get()[i]).initSocket();
	}

#if 1
	const char ip[] = "127.0.0.1";
#elif 0
	//通过cmd查到Linux虚拟机的IP地址为192.168.175.132
	const char ip[] = "192.168.175.132";
#else
	//通过cmd查到Mac虚拟机的IP地址为192.168.175.133
	const char ip[] = "192.168.175.133";
#endif

	for (int i = 0; i < clients_per_thread; ++i)
	{
		if (!g_bRun)
			return;

		(ClientArray.get()[i]).Connect(ip, 9190);
	}

	LogIn login{};

#ifdef _WIN32
	strncpy_s(login.username, 32, "Luhao", 6);
	strncpy_s(login.password, 32, "123456", 7);
#else
	strncpy(login.username, "Luhao", 6);
	strncpy(login.password, "123456", 7);
#endif

	while (g_bRun)
	{
		for (int i = 0; i < clients_per_thread; ++i)
		{
			//ClientArray.get()[i].OnRun();
			(ClientArray.get()[i]).SendData(&login);
		}
	}

	for (int i = 0; i < clients_per_thread; ++i)
	{
		(ClientArray.get()[i]).Close();
	}

	return;
}
#endif


//另一种处理是将要创建的客户端由全局创建的数组来管理
//这样客户端资源可以作为共享资源，被其他线程访问
#if 1
void SendThread(int id)
{
	printf("线程<%d>开始\n", id);

	//计算每个线程要承担创建的客户端数量
	int clients_per_thread = Num_clients / T_cnt;

	//计算每个线程动态创建的客户端对应管理其的
	//指针在pClientArray数组中的分布
	int theBegin = (id - 1) * clients_per_thread;
	int theEnd = id * clients_per_thread;

	//shared_ptr < EasyTcpClient >ClientArray
	//(new EasyTcpClient[clients_per_thread], default_delete<EasyTcpClient[]>());

	for (int i = theBegin; i < theEnd; ++i)
	{
		pClientArray[i] = new EasyTcpClient();
	}

	for (int i = theBegin; i < theEnd; ++i)
	{
		pClientArray[i]->initSocket();
	}

#if 1
	const char ip[] = "127.0.0.1";
#elif 0
	//通过cmd查到Linux虚拟机的IP地址为192.168.175.132
	const char ip[] = "192.168.175.132";
#else
	//通过cmd查到Mac虚拟机的IP地址为192.168.175.133
	const char ip[] = "192.168.175.133";
#endif

	for (int i = theBegin; i < theEnd; ++i)
	{
		if (!g_bRun)
			return;

		pClientArray[i]->Connect(ip, 9190);

		//printf("线程号<%d>, 客户端编号=%d 连接成功...\n",
		//	id, i);
	}

	printf("线程号<%d>, 客户端编号<begin=%d,  end=%d>连接成功...\n",
		id, theBegin, theEnd);

	//使用标准库提供的休眠
	chrono::seconds t(8);
	this_thread::sleep_for(t);
	//所有客户端都连接完成后，再一起发送消息

	const int n = 10;
	//LogIn login[10];
	LogIn login[n];

	for (int i = 0; i < n; ++i)
	{
#ifdef _WIN32
		strncpy_s(login[i].username, 32, "Luhao", 6);
		strncpy_s(login[i].password, 32, "123456", 7);
#else
		strncpy(login.username, "Luhao", 6);
		strncpy(login.password, "123456", 7);
#endif
	}

	int nLen = sizeof(login);

	while (g_bRun)
	{
		for (int i = theBegin; i < theEnd; ++i)
		{
			pClientArray[i]->OnRun();
			pClientArray[i]->SendData(login, nLen);
		}
	}

	for (int i = theBegin; i < theEnd; ++i)
	{
		pClientArray[i]->Close();
		delete pClientArray[i];
	}

	printf("线程<%d>退出\n", id);

	return;
}
#endif

#endif
