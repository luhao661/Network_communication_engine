#if 0
#include "EasyTcpClient_1.3.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread(void);

int main()
{
	//由于EasyTcpClient类占用内存空间很大
	//所以放到堆中较好
	//EasyTcpClient* client1=new EasyTcpClient ;

	//EasyTcpClient* ClientArray = new EasyTcpClient[FD_SETSIZE-1]();

	const int Num_clients = 1000;

	shared_ptr < EasyTcpClient >ClientArray
	(new EasyTcpClient[Num_clients], default_delete<EasyTcpClient[]>());

	for (int i = 0; i < Num_clients; ++i)
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

	thread t1(cmdThread);//格式：函数名  参数
	t1.detach();

	for (int i = 0; i < Num_clients; ++i)
	{
		if (!g_bRun)
			return 0;

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


	while (g_bRun /*|| client2.isRun()*/)
	{
		for (int i = 0; i < Num_clients; ++i)
		{
			//ClientArray.get()[i].OnRun();
			(ClientArray.get()[i]).SendData(&login);
		}
	}

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).Close();
	}

	cout << "客户端已退出，任务结束。\n";

	return 0;
}

void cmdThread(void)
{
	char cmdBuf[256]{};

	//cin 和 scanf 都是阻塞式函数，
	// 而main()中的select()非阻塞，这样会造成运行逻辑冲突，所以要使用多线程

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
#endif
