#pragma once

#ifdef _WIN32
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

#include "MessageHeader_1.1.hpp"
#include <iostream>
#include <vector>

using namespace std;

class EasyTcpServer
{
private:
	SOCKET m_client_sock;
	SOCKET m_serv_sock;
	vector<SOCKET> vec_client;

public:
	EasyTcpServer();

	virtual ~EasyTcpServer();

	SOCKET initSocket();

	void Close();

	int Bind(const char* ip, unsigned short port);

	int Listen(int backlog);

	SOCKET Accept();


	//是否在正常工作中
	bool isRun()
	{
		return m_serv_sock != INVALID_SOCKET;
	}

	//处理网络数据
	//查询是否有待读取的数据
	bool OnRun();

	//接收数据
	//处理粘包 拆分包
	int RecvData(SOCKET client_sock);

	//响应网络消息
	//***注***
	//声明为virtual易于之后该类被继承，要重写响应网络消息函数时，
	//OnNetMsg()方法可获得多态性
	void OnNetMsg(SOCKET client_sock, DataHead* pHead);

	//发送数据
	int SendData(SOCKET client_sock, DataHead* pHead);

	void SendDataToAll(DataHead* pHead);
};

EasyTcpServer::EasyTcpServer()
{
	m_client_sock = m_serv_sock = INVALID_SOCKET;
}

EasyTcpServer:: ~EasyTcpServer()
{
	Close();
}

SOCKET EasyTcpServer::initSocket()
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
	if (m_serv_sock != INVALID_SOCKET)
	{
		cout << "<socket=" << m_serv_sock << ">关闭旧连接\n";
		Close();
	}

	//  建立一个socket
	m_serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//***注***
	// 在网络编程中，AF_INET 用于指定地址族（Address Family）为 IPv4，
	// 而 PF_INET 则用于指定协议族为 IPv4。
	// 在实际使用中，AF_INET 与 PF_INET 可以互换使用，
	// 而且在大多数情况下，它们是相等的。

	if (m_serv_sock == INVALID_SOCKET)
		cout << "建立socket失败\n";
	else
		cout << "建立服务端socket=<" << m_serv_sock << ">成功...\n";


	return m_serv_sock;
}

void EasyTcpServer::Close()
{
	if (m_serv_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		//关闭全部的客户端套接字
		for (int n = vec_client.size() - 1; n >= 0; --n)
		{
			closesocket(vec_client[n]);
			vec_client[n] = INVALID_SOCKET;
		}

		//  关闭套节字closesocket
		closesocket(m_serv_sock);
		m_serv_sock = INVALID_SOCKET;

		//注销
		WSACleanup();
#else

		//关闭全部的客户端套接字
		for (int n = (int)vec_client.size() - 1; n >= 0; --n)
		{
			close(vec_client[n]);
			vec_client[n] = INVALID_SOCKET;
		}

		//  关闭套节字closesocket
		close(m_serv_sock);
		m_serv_sock = INVALID_SOCKET;

#endif
	}
}

int EasyTcpServer::Bind(const char* ip, unsigned short port)
{
	if (INVALID_SOCKET == m_serv_sock)
		initSocket();

	//地址信息初始化
	//serv_adr结构体一定要进行初始化，原因见《TCP/IP网络编程读书笔记》
	sockaddr_in serv_adr = {};
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_port = htons(port);

	if (ip == nullptr)
	{
		//可以用常数 INADDR_ANY 来获取服务器端的 IP 地址
#ifdef _WIN32
		serv_adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
		serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
	}
	else
	{
#ifdef _WIN32
		serv_adr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		serv_adr.sin_addr.s_addr = inet_addr(ip);
#endif
	}

	//  bind 绑定用于接受客户端连接的服务端网络端口
	//Mac环境下将bind调用限定为使用全局命名空间中的函数
	int res = ::bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (res == SOCKET_ERROR)
		cout << "bind() ERROR，绑定网络端口<" << port << ">失败...\n";
	else
		cout << "绑定网络端口<" << port << ">成功...\n";

	return res;
}

int EasyTcpServer::Listen(int backlog)
{
	//  listen 监听网络端口
	int res = listen(m_serv_sock, backlog);

	if (res == SOCKET_ERROR)
		cout << "listen() ERROR，socket=<" << m_serv_sock << ">监听网络端口失败\n";
	else
		cout << "socket=<" << m_serv_sock << ">监听网络端口成功...\n";

	return res;
}

SOCKET EasyTcpServer::Accept()
{
	sockaddr_in client_adr = {};
	int client_adr_size = sizeof(client_adr);

	//  accept 等待接受客户端连接
#ifdef _WIN32
	m_client_sock = accept(m_serv_sock, (sockaddr*)&client_adr, &client_adr_size);
#else
	m_client_sock = accept(m_serv_sock, (sockaddr*)&client_adr, (socklen_t*)&client_adr_size);
#endif

	if (m_client_sock == INVALID_SOCKET)
		cout << "socket=<" << m_serv_sock << ">错误，接受到无效客户端SOCKET";
	else
	{
		//inet_ntoa()将网络字节序的整数型 IP 地址转换为字符串形式
		cout << "socket=<" << m_serv_sock << ">新客户端加入：IP = "
			<< inet_ntoa(client_adr.sin_addr)
			<< "  socket=<" << m_client_sock << ">" << endl;

		//通知该客户端之前的所有客户端有新用户加入
		NewUserJoin newuserjoin;
		SendDataToAll(&newuserjoin);

		vec_client.push_back(m_client_sock);
	}

	return m_client_sock;
}

static long long cnt = 0;

//处理网络数据
//查询是否有待读取的数据
bool EasyTcpServer::OnRun()
{
	if (!isRun())
		return false;

	// 4 创建timeval结构布局的结构变量timeout以作为select数的第五个实参
	//为了防止陷入无限阻塞的状态，使用 timeout 传递超时信息。
	struct timeval timeout;

	//设置5秒的超时时间
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	//或
	//struct timeval timeout = { 5,0 };

	//伯克利socket	 BSD socket

	// 1 创建fd_set结构体将要监视的套接字句柄集中到一起，以监视这些套接字句柄
	fd_set fdRead;//关注 是否存在待读取数据
	fd_set fdWrite;
	fd_set fdExp;

	//***注***
	// Windows的fd_set由成员 fd_count和fd_array构成， fd_count用于套接字句柄数，
	// fd_array是个数组集合，用于保存套接字句柄
	//基于Windows的套接字句柄不仅不能从零开始，
	//而且在生成的句柄的整数值之间也找不到规则，因此
	//需要一个数组来保存套接字的句柄以及一个记录句柄数的变量

	// 2 使用宏来完成对结构体中所有位都设置为0的操作
	//清空fdRead句柄集合，fdWrite句柄集合，fdExp句柄集合
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExp);

	// 3 使用宏来向结构体中注册套接字句柄serv_sock的信息
	// 将套接字句柄serv_sock对应的位设置为1，即
	// 将套接字句柄（注册）添加到fdRead集合、fdWrite集合、fdExp集合中
	// 需要监视serv_sock是否有读、写、异常
	FD_SET(m_serv_sock, &fdRead);
	FD_SET(m_serv_sock, &fdWrite);
	FD_SET(m_serv_sock, &fdExp);

	//创建maxSocket来存储所有客户端加入后的套接字的最大值
	SOCKET maxSocket = m_serv_sock;
	//***注***
	//Windows下不需要，因为
	//在 Windows 下使用 select()，实际上是依赖其他参数的设置来实现对文件描述符的监视，
	//如第二个参数 fdset，它指向一个 fd_set 结构体，其中包含要监视的文件描述符集合，
	//而不是依赖 nfds 参数即第一个参数来确定监视的文件描述符数量

	// 4 将客户端套接字句柄添加到fdRead集合
	// 这样做的目的是为了将这些已连接的客户端套接字加入到 fdRead 集合中进行监视，
	// 以便在调用 select() 函数时，能够监视这些套接字的读取操作。
	// 这意味着如果有任何已连接的客户端发送数据，select() 函数将返回并通知程序，
	// 使得程序可以在套接字可读的情况下进行相应的处理
	for (int n = (int)vec_client.size() - 1; n >= 0; --n)
	{
		FD_SET(vec_client[n], &fdRead);
		maxSocket = maxSocket < vec_client[n] ? vec_client[n] : maxSocket;
	}

	// cout << "fdRead.fd_count = " << fdRead.fd_count << endl;


	// 5 使用select函数
	// 对于第一个参数，int nfds，是指fd_set集合中所有套接字句柄的范围，
	// 而不是数量，是所有句柄的最大值+1，在windows中可以写0
	// 要写为socket值【最大的一个socket值再加一】
	int fd_num = select(maxSocket + 1, &fdRead, &fdWrite, &fdExp, &timeout);
	if (fd_num == -1)
	{
		cout << "select任务结束" << endl;
		Close();
		return false;
	}
	else if (fd_num == 0)
	{

	}
	cout <<  "fd_num="<< fd_num<<", cnt="<<cnt++ << endl;

	//用FD_ISSET宏筛选在fdRead集合中发生状态变化的句柄
	//检查服务器端套接字（serv_sock）是否处于待读取数据的状态
	//如果是服务器端套接字的变化，表示有客户端连接请求到达，将受理连接请求。
	if (FD_ISSET(m_serv_sock, &fdRead))
	{
		//从fdRead集合中删除该句柄
		//在后续的 I/O 多路复用操作中不再监听或检查该句柄的状态变化
		//（不写也没关系）
		FD_CLR(m_serv_sock, &fdRead);

		Accept();
	}

	for (int n = (int)vec_client.size() - 1; n >= 0; n--)
	{
		if (FD_ISSET(vec_client[n], &fdRead))
		{
			if (RecvData(vec_client[n]) == -1)
			{
				auto it = vec_client.begin() + n;
				if (it != vec_client.end())
					vec_client.erase(it);
			}
		}
	}

	return true;
}

char RecvBuff[409600] = {};// 400 KB
//接收数据
//处理粘包 拆分包
int EasyTcpServer::RecvData(SOCKET client_sock)
{

	//  接收客户端数据
	int len = (int)recv(client_sock, (char*)&RecvBuff, 409600, 0);

	if (len <= 0)
	{
		cout << "客户端<socket=" << client_sock << ">已退出。\n";
		return -1;
	}

	cout << "收到数据的长度len=" << len << endl;

	/*
	DataHead* pHead = reinterpret_cast<DataHead*>(RecvBuff);

	recv(client_sock, (char*)&RecvBuff + sizeof(DataHead),
		pHead->datalength - sizeof(DataHead), 0);

	OnNetMsg(client_sock, pHead);
	*/

	// 发送报文
	LogInResult res{};
	//send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
	//可以改写成
	SendData(client_sock, &res);

	return 0;
}

//响应网络消息
void EasyTcpServer::OnNetMsg(SOCKET client_sock, DataHead* pHead)
{
	//  处理请求
	switch (pHead->cmd)
	{
	case CMD_LOGIN:
	{
		LogIn* login = reinterpret_cast<LogIn*>(pHead);

		//忽略判断用户密码是否正确的过程

		cout << "收到客户端<socket=" << client_sock << ">命令：CMD_LOGIN"
			<< " 数据长度：" << login->datalength << endl;
		cout << "用户名：" << login->username << "登入" << endl;

		// 发送报文
		LogInResult res{};
		//send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
		//可以改写成
		SendData(client_sock, &res);
	}
	break;

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

	default:
	{
		DataHead head = { sizeof(DataHead),CMD_ERROR };
		send(client_sock, (const char*)&head, sizeof(DataHead), 0);

		cout << "Error!" << endl;
	}
	}
}

//给指定客户端发送数据
int EasyTcpServer::SendData(SOCKET client_sock, DataHead* pHead)
{
	if (isRun() && pHead)
	{
		return send(client_sock, (const char*)pHead, pHead->datalength, 0);
	}

	return SOCKET_ERROR;
}

//对所有客户端进行广播
void EasyTcpServer::SendDataToAll(DataHead* pHead)
{
	for (int n = (int)vec_client.size() - 1; n >= 0; --n)
	{
		SendData(vec_client[n], pHead);
	}
}