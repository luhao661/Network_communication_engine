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

#include <iostream>
#include <vector>
#include "MessageHeader.hpp"

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

	void initSocket();

	int Bind(const char* ip, unsigned short port);

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

EasyTcpServer::EasyTcpServer()
{

}

EasyTcpServer:: ~EasyTcpServer()
{

}

void EasyTcpServer::initSocket()
{
#ifdef _WIN32
	//初始化

	//创建版本号
	WORD ver = MAKEWORD(2, 2);
	//创建Windows Sockets API数据
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	//  建立一个socket
	m_serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//***注***
	// 在网络编程中，AF_INET 用于指定地址族（Address Family）为 IPv4，
	// 而 PF_INET 则用于指定协议族为 IPv4。
	// 在实际使用中，AF_INET 与 PF_INET 可以互换使用，
	// 而且在大多数情况下，它们是相等的。


}

int EasyTcpServer::Bind(const char* ip, unsigned short port)
{
	//地址信息初始化
	//serv_adr结构体一定要进行初始化，原因见《TCP/IP网络编程读书笔记》
	sockaddr_in serv_adr = {};
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_port = htons(port);

	if (ip==NULL)
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
}

void EasyTcpServer::Close()
{

}

//处理网络数据
//查询是否有待读取的数据
bool EasyTcpServer::OnRun()
{

}

//接收数据
//处理粘包 拆分包
int EasyTcpServer::RecvData()
{

}

//响应网络消息
void EasyTcpServer::OnNetMsg(DataHead* pHead)
{

}

//发送数据
int EasyTcpServer::SendData(DataHead* pHead)
{

}