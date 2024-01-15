#if 0
#include "EasyTcpServer_1.3.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread(void);

int main()
{
	EasyTcpServer server;

	server.initSocket();
	server.Bind(nullptr, 9190);
	server.Listen(5);
	server.Accept();	   //在OnRun()中已经包含Accept()，这样能处理多客户端的通信请求

	thread t1(cmdThread);//格式：函数名  参数
	t1.detach();

	while (server.isRun() && g_bRun)
	{
		server.OnRun();
	}

	server.Close();

	cout << "服务器端已退出，任务结束。\n";
	cin.get();

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