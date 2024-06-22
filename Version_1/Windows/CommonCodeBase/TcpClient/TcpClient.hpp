#pragma once
//.hpp 文件扩展名通常用于 C++ 的头文件，与 .h 头文件的作用类似。
// 它们都包含了 C++ 程序中所需的声明、定义、函数原型、类、模板等信息，
// 供其他源文件引用和使用。
//.hpp 文件通常表示 C++ 头文件。C++ 头文件中可能包含 C++ 特定的内容，
// 例如模板、namespace 和其他 C++ 特性，而 .h 文件可能仅包含 C 的内容.
//C++ 编译器会默认将 .hpp 文件识别为 C++ 文件

#include <iostream>
#include "Cell.hpp"
#include "CellNetWork.hpp"
#include "CellClient.hpp"

using namespace std;

#ifndef RECV_BUFFER_SIZE 
#define RECV_BUFFER_SIZE 10240
#endif

class EasyTcpClient
{
private:
	//使用ClientSocket类，该类已封装了接收消息缓冲区和发送缓冲区
	ClientSocket* m_pClient=nullptr;

	//第二缓冲区 消息缓冲区
	//char m_MsgBuf[RECV_BUFFER_SIZE * 10] = {};
	//指向消息缓冲区的数据尾部位置
	//int m_lastPos = 0;

	bool m_isConnected = false;

public:

	EasyTcpClient()
	{}

	//在多态情况下避免局部销毁对象（《Effective C++》P41）
	virtual ~EasyTcpClient()
	{
		Close();
	}

	void initSocket()
	{
		CellNetWork::Init();

		if (m_pClient)
		{
			printf("<socket=%d>关闭旧连接...\n", m_pClient->Get_m_client_sock());
			Close();
		}

		// 1 建立一个socket
		SOCKET client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (client_sock == INVALID_SOCKET)
			printf("建立socket失败\n");
		else
		{
			m_pClient = new ClientSocket(client_sock);
		}
	}

	void Close()
	{
		if (m_pClient)
		{
			delete m_pClient;
			m_pClient = nullptr;
		}

		m_isConnected = false;
	}

	int Connect(const char* ip, unsigned short port)
	{
		//如果套接字还没被创建
		if (!m_pClient)
		{
			initSocket();
		}
		//***注***
		// 在网络编程中，AF_INET 用于指定地址族（Address Family）为 IPv4，
		// 而 PF_INET 则用于指定协议族为 IPv4。
		// 在实际使用中，AF_INET 与 PF_INET 可以互换使用，
		// 而且在大多数情况下，它们是相等的。

		//***注***
		//初始化值为【目标服务器端套接字】的 IP 和端口信息。
		//一定要进行初始化，原因见《TCP/IP网络编程读书笔记》
		sockaddr_in serv_adr = {};
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_port = htons(port);
		//serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		//127.0.0.1是IPv4地址空间中的一个特殊保留地址，
		//也称为回环地址或本地回环地址。
		//它通常被用作本地主机上的环回接口，
		//用于在计算机内部进行自我通信和测试网络功能。
		//当计算机尝试连接到127.0.0.1时，它实际上是在尝试与自己的网络接口进行通信。
		//或写为

#ifdef _WIN32
		serv_adr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		serv_adr.sin_addr.s_addr = inet_addr(ip);
#endif

		SOCKET client_sock = m_pClient->Get_m_client_sock();

		// 2 连接服务器 connect
		int res = connect(client_sock, (sockaddr*)&serv_adr, sizeof(sockaddr_in));
		if (res == SOCKET_ERROR)
			cout << "<socket=" << client_sock << ">连接服务器<"
			<< ip << " : " << port << ">失败...\n";
		else
		{
			//cout << "<socket=" << m_client_sock << ">连接服务器<"
			//<< ip << " : " << port << ">成功...\n";

			//printf("<socket=%d>连接服务器<%s : %hd>成功...\n",
				//m_client_sock, ip, (short)port);

			m_isConnected = true;
		}
		//***注***
		//客户端套接字在调用connect()时绑定（也叫分配）了客户端地址

		return res;
	}

	//是否在正常工作中
	bool isRun()
	{
		return (m_pClient) && m_isConnected;
	}

	//处理网络数据
	//查询是否有待读取和待写入的数据
	bool OnRun()
	{
		if (isRun())
		{
			SOCKET client_sock = m_pClient->Get_m_client_sock();

			fd_set fdRead;

			FD_ZERO(&fdRead);
			FD_SET(client_sock, &fdRead);

			fd_set fdWrite;
			FD_ZERO(&fdWrite);

			timeval timeout = { 0,1 };
			int ret = 0;
			//提升性能的写法：
			if(m_pClient->needWrite())
			{
				FD_SET(client_sock, &fdWrite);
				ret = select(client_sock + 1, &fdRead, &fdWrite, nullptr, &timeout);
			}
			else
			{
				ret= select(client_sock + 1, &fdRead, nullptr, nullptr, &timeout);
			}

			if (ret < 0)
			{
				cout << "<socket=" << client_sock << ">select任务结束_1" << endl;
				//防止检测到返回值-1后还仍在持续运行OnRun()
				Close();
				return false;
			}
			//cout << "fd_num=" << fd_num << ", cnt=" << cnt++ << endl;

			if (FD_ISSET(client_sock, &fdRead))
			{
				if (-1 == RecvData())
				{
					printf("<socket=%d> select任务结束_2\n", client_sock);
					Close();
					return false;
				}
			}

			if (FD_ISSET(client_sock, &fdWrite))
			{
				if (-1 == m_pClient->SendDataImmediately())
				{
					printf("<socket=%d> select任务结束_2\n", client_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//接收数据
	//处理粘包 拆分包
	int RecvData()
	{
		if(isRun())
		{
			int len = m_pClient->RecvData();
			if (len > 0)
			{
				//判断消息缓冲区的数据长度是否大于消息头的长度
				//用while循环，解决【粘包】
				while (m_pClient->hasMsg())//hasMsg()解决【少包】
				{
					OnNetMsg(m_pClient->frontMsg());

					m_pClient->popMsg();
				}
			}

			return len;
		}
		return 0;
	}

	//响应网络消息
	void OnNetMsg(DataHead* pHead)
	{
		SOCKET client_sock = m_pClient->Get_m_client_sock();

		switch (pHead->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LogInResult* loginresult = reinterpret_cast<LogInResult*>(pHead);

			//cout << "<socket=" << m_client_sock <<
			//	">收到服务端消息：CMD_LOGIN_RESULT"
			//	<< " 数据长度：" << loginresult->datalength << endl;
		}
		break;

		case CMD_LOGOUT_RESULT:
		{
			LogOutResult* logoutresult = reinterpret_cast<LogOutResult*>(pHead);

			cout << "<socket=" << client_sock <<
				">收到服务端消息：CMD_LOGOUT_RESULT"
				<< " 数据长度：" << logoutresult->datalength << endl;
		}
		break;

		case CMD_NEW_USER_JOIN:
		{
			//NewUserJoin* newuserjoin = reinterpret_cast<NewUserJoin*>(pHead);
			//cout << "\n<socket=" << m_client_sock <<
			//	">收到服务端消息：CMD_NEW_USER_JOIN"
			//	<< " 数据长度：" << newuserjoin->datalength << endl;
		}
		break;

		case CMD_s2c_HEART:
		{

		}
		break;

		case CMD_ERROR:
		{
			cout << "\n<socket=" << client_sock <<
				">收到服务端消息：CMD_ERROR"
				<< " 数据长度：" << pHead->datalength << endl;
		}
		break;

		default:
		{
			cout << "\n<socket=" << client_sock <<
				">收到未知服务端消息" << endl;
		}
		}
	}

	//发送数据
	int SendData(DataHead* pHead)
	{
		if(isRun())
			return m_pClient->SendData(pHead);

		return 0;
	}

	//发送数据
	int SendData(const char* pData,int len)
	{
		if(isRun())
			return m_pClient->SendData(pData,len);

		return 0;
	}
};



