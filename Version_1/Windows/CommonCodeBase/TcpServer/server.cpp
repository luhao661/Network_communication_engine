#if 1
#include "TcpServer.hpp"
#include <iostream>
#include <thread>

using std::cout;
using std::endl;
using std::thread;
using std::cin;

class MyServer :public EasyTcpServer
{
public:
	virtual void NEOnNetJoin(ClientSocket* pClient)
	{
		//以类名+作用域解析运算符的形式来调用基类的同名方法
		EasyTcpServer::NEOnNetJoin(pClient);
	}

	virtual void NEOnNetLeave(ClientSocket* pClient) 
	{ 
		EasyTcpServer::NEOnNetLeave(pClient);
		CellLog::Info("Clinet<%d>退出\n", pClient->Get_m_client_sock());
	}
 
	virtual void NEOnNetMsg(CellServer* pCellServer, ClientSocket* pclient_sock, DataHead* pHead)
	{
		EasyTcpServer::NEOnNetMsg(pCellServer, pclient_sock, pHead);

		//  处理请求
		switch (pHead->cmd)
		{
		case CMD_LOGIN: 
		{
			//将该消息视为心跳包
			pclient_sock->resetDTHeart();

			LogIn* login = reinterpret_cast<LogIn*>(pHead);

			//忽略判断用户密码是否正确的过程

			//***注***
			//写以下代码，会导致服务端和客户端在进行通信后不久就不动了
			//cout << "收到客户端<socket=" << client_sock << ">命令：CMD_LOGIN"
			//	<< " 数据长度：" << login->datalength << endl;
			//cout << "用户名：" << login->username << "登入" << endl;

			// 发送报文
			//LogInResult* res = new LogInResult{};
			//pclient_sock->SendData(res);

			LogInResult ret;

			//***注***
			//以下为select实现非阻塞发送数据的实现
			//如果发送缓冲区满了，消息没发送出去
			if (SOCKET_ERROR == pclient_sock->SendData(&ret))
			{
				CellLog::Info("<socket=%d> 发送缓冲区已满！当前消息未发送！\n", pclient_sock->Get_m_client_sock());

				//可以进行将数据暂存入磁盘等操作
				//...
			}


			//***注***
			//以下为阻塞发送数据，且使用CellTaskServer线程执行发送任务的实现
			//pCellServer->addSendTask(pclient_sock, &res);
		}
		break;

		/*
		case CMD_LOGOUT:
		{
			LogOut* logout = reinterpret_cast<LogOut*>(pHead);

			cout << "收到客户端<socket=" << client_sock << ">命令：CMD_LOGOUT"
				<< " 数据长度：" << logout->datalength << endl;
			cout << "用户名：" << logout->username << "登出" << endl;

			LogOutResult res{};
			send(client_sock, (const char*)&res, sizeof(LogOutResult), 0);
		}
		break;
		*/

		case CMD_c2s_HEART:
		{
			pclient_sock->resetDTHeart();

			s2c_Heart ret;

			pCellServer->addSendTask(pclient_sock, &ret);
		}
		break;

		default:
		{
			cout << "收到客户端<socket=" << pclient_sock->Get_m_client_sock() << ">未定义消息"
				<< " 数据长度：" << pHead->datalength << endl;

			//发送含有CMD_ERROR的数据包
			DataHead head;
			pclient_sock->SendData(&head);
		}
		}

	}

	virtual void NERecv(ClientSocket* client_sock)
	{
		EasyTcpServer::NERecv(client_sock);
	}
};


int main()
{
	CellLog::Instance().setLogPath("ServerLog.txt", "w");
	MyServer server;

	server.initSocket();
	server.Bind(nullptr, 9190);
	server.Listen(5);
	//server.Accept();	   //在OnRun()中已经包含Accept()，这样能处理多客户端的通信请求

	//启动多线程
	server.StartThread(4);

	while (true)
	{
		char cmdBuf[256]{};

		//cin 和 scanf 都是阻塞式函数，
		// 而main()中的select()非阻塞，这样会造成运行逻辑冲突，所以要使用多线程

		cin.getline(cmdBuf, 256);

		// 处理请求
		if (!strcmp(cmdBuf, "exit"))
		{
			server.Close();
			CellLog::Info("收到退出命令，退出cmdThread线程\n");
			break;
		}
		else
			CellLog::Info("未识别的命令，请重新输入！\n");
	}

	CellLog::Info("服务器端已退出，任务结束。\n");
	//while (true)
	//{
	//	Sleep(1); 
	//}

	return 0;
}
#endif
