#if 1
#include "EasyTcpServer_1.1.hpp"

int main()
{
	EasyTcpServer server;

	server.initSocket();
	server.Bind(nullptr, 9190);
	server.Listen(5);
	//server.Accept();	   //在OnRun()中已经包含Accept()，这样能处理多客户端的通信请求

	while (server.isRun())
	{
		server.OnRun();
	}

	server.Close();

	cout << "服务器端已退出，任务结束。\n";
	cin.get();

	return 0;
}
#endif