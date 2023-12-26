#pragma once
//.hpp 文件扩展名通常用于 C++ 的头文件，与 .h 头文件的作用类似。
// 它们都包含了 C++ 程序中所需的声明、定义、函数原型、类、模板等信息，
// 供其他源文件引用和使用。
//.hpp 文件通常表示 C++ 头文件。C++ 头文件中可能包含 C++ 特定的内容，
// 例如模板、namespace 和其他 C++ 特性，而 .h 文件可能仅包含 C 的内容.
//C++ 编译器会默认将 .hpp 文件识别为 C++ 文件

#ifdef   _WIN32
	//解决windows.h和winsock2.h下宏定义冲突
	#define WIN32_LEAN_AND_MEAN
	//使inet_ntoa()可用
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	//包含windows下的API
	#include <windows.h>
	//包含windows下的socket的API
	#include <winsock2.h>
	//无法解析的外部符号 imp WSAStartup，函数 main 中引用了该符号
	//解决：要添加静态链接库文件
	//#pragma comment(lib,"ws2_32.lib")
	//#pragma comment(lib, "library_name")
	//#pragma 是一个编译器指令，用于向编译器传达特定的指令或控制信息
	//在编译时指示链接器引入特定的库文件。
	//方法二：在项目的属性->链接器->输入->附加依赖项->添加ws2_32.lib
#else
	//UNIX 下标准C语言头文件
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>

	//将SOCKET类型宏定义为int类型
	//因为Linux下套接字文件描述符就是int类型
	#define SOCKET int
	//继续定义Linux下没有的宏定义
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <iostream>
#include "MessageHeader.hpp"
using namespace std;


class EasyTcpClient
{
private:
	SOCKET m_client_sock;

public:

	EasyTcpClient();

	//在多态情况下避免局部销毁对象（《Effective C++》P41）
	virtual ~EasyTcpClient();

	void initSocket();

	int Connect(const char* ip, unsigned short port);

	void Close();

	//是否在正常工作中
	bool isRun()
	{
		return m_client_sock != INVALID_SOCKET;
	}

	//处理网络数据
	//查询是否有待读取的数据
	bool OnRun();

	//接收数据
	//处理粘包 拆分包
	int RecvData();

	//响应网络消息
	void OnNetMsg(DataHead* pHead);

	//发送数据
	int SendData(DataHead* pHead);
};

EasyTcpClient::EasyTcpClient()
{
	m_client_sock = INVALID_SOCKET;
}
EasyTcpClient::~EasyTcpClient()
{
	Close();
}
void EasyTcpClient::initSocket()
{
	{
#ifdef _WIN32
		//初始化

		//创建版本号
		WORD ver = MAKEWORD(2, 2);
		//创建Windows Sockets API数据
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		//如果当前对象的套接字已经创建了，不允许重复创建套接字
		//那就关闭了，再重新创建一个
		if (m_client_sock != INVALID_SOCKET)
		{
			cout << "<socket=" << m_client_sock << ">关闭旧连接\n";
			Close();
		}

		// 1 建立一个socket
		m_client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (m_client_sock == INVALID_SOCKET)
			cout << "建立socket失败\n";
		else
			cout << "建立socket=<"<< m_client_sock <<">成功...\n";
	}

}
int EasyTcpClient::Connect(const char* ip, unsigned short port)
{
	//如果套接字还没被创建
	if (m_client_sock == INVALID_SOCKET)
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

	// 2 连接服务器 connect
	int res = connect(m_client_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (res == SOCKET_ERROR)
		cout << "<socket=" << m_client_sock <<  ">连接服务器<"
		<< ip << " : " << port << ">失败...\n";
	else
		cout << "<socket=" << m_client_sock << ">连接服务器<"
		<<ip<<" : "<<port<<">成功...\n";
	//***注***
	//客户端套接字在调用connect()时绑定（也叫分配）了客户端地址

	return res;
}

void EasyTcpClient::Close()
{
	//如果主动关闭客户端套接字，那么析构函数不需要再关闭一次连接
	if (m_client_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		// 7 关闭套节字closesocket
		closesocket(m_client_sock);

		//注销
		WSACleanup();

#else
		close(m_client_sock);
#endif
		//避免重复关闭
		m_client_sock = INVALID_SOCKET;
	}
}

bool EasyTcpClient::OnRun()
{
	if (!isRun())
		return false;

	fd_set fdRead;

	FD_ZERO(&fdRead);

	FD_SET(m_client_sock, &fdRead);

	struct timeval timeout = { 3,0 };

	int fd_num = select(m_client_sock + 1, &fdRead, 0, 0, &timeout);
	if (fd_num == -1)
	{
		cout << "<socket=" << m_client_sock << ">select任务结束_1" << endl;
		//防止检测到返回值-1后还仍在持续运行OnRun()
		Close();
		return false;
	}
	else if (fd_num == 0)
	{
		//cout << "空闲时间处理其他业务" << endl;
	}

	if (FD_ISSET(m_client_sock, &fdRead))
	{
		FD_CLR(m_client_sock, &fdRead);

		if (-1 == RecvData())
		{
			cout << "<socket=INVALID_SOCKET" << ">select任务结束_2" << endl;
		}
	}

	return true;
}

int EasyTcpClient::RecvData()
{
	char RecvBuff[4096] = {};

	//先接收包头
	int len = (int)recv(m_client_sock, (char*)&RecvBuff, sizeof(DataHead), 0);
	if (len <= 0)
	{
		cout << "<socket=" << m_client_sock << 
			">与服务器断开连接，任务结束" << endl;
		return -1;
	}

	//DataHead* pHead = (DataHead*)RecvBuff;
	DataHead* pHead = reinterpret_cast<DataHead*>(RecvBuff);

	//再根据数据包长度，继续接收数据
	recv(m_client_sock, (char*)RecvBuff + sizeof(DataHead),
		pHead->datalength - sizeof(DataHead), 0);

	//***理解***
	//一个指向RecvBuff存储空间的指针转换为 DataHead* 类型的指针
	//这种转换被称为重新解释转换
	//那么后面的switch语句块
	//LogInResult* loginresult =reinterpret_cast<LogInResult*>(pHead)
	//同理将DataHead* 类型的指针解释为LogInResult*类型的指针
	//得以访问datalength数据成员

	OnNetMsg(pHead);

	return 0;
}

void EasyTcpClient::OnNetMsg(DataHead* pHead)
{
	switch (pHead->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		LogInResult* loginresult = reinterpret_cast<LogInResult*>(pHead);

		cout << "<socket=" << m_client_sock << 
			">收到服务端消息：CMD_LOGIN_RESULT"
			<< " 数据长度：" << loginresult->datalength << endl;
	}
	break;

	case CMD_LOGOUT_RESULT:
	{
		LogOutResult* logoutresult = reinterpret_cast<LogOutResult*>(pHead);

		cout << "<socket=" << m_client_sock << 
			">收到服务端消息：CMD_LOGOUT_RESULT"
			<< " 数据长度：" << logoutresult->datalength << endl;
	}
	break;

	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin* newuserjoin = reinterpret_cast<NewUserJoin*>(pHead);
		cout << "\n<socket=" << m_client_sock << ">收到服务端消息：CMD_NEW_USER_JOIN"
			<< " 数据长度：" << newuserjoin->datalength << endl;
	}
	break;
	}

}

int EasyTcpClient::SendData(DataHead* pHead)
{
	if (isRun() && pHead)
	{
		return send(m_client_sock, (const char*)pHead, pHead->datalength, 0);
	}

	return SOCKET_ERROR;
}


