#pragma once

#ifdef _WIN32
//解决windows.h和winsock2.h下宏定义冲突
#define WIN32_LEAN_AND_MEAN
//使inet_ntoa()可用
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define FD_SETSIZE 1024
//错误理解：
//最大支持的客户端连接数量为FD_SETSIZE-1
//***注***
//最大支持的客户端数量为 
//CELLSERVER_THREADS_COUNT * (FD_SETSIZE)
//因为CellServer类中OnRun()中的FD_SET()注册的全是客户端

//服务端线程数量
//#define CELLSERVER_THREADS_COUNT 4

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

#include "MessageHeader_1.0.hpp"//改为100字节的数据包
#include "Timestamp.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>//mem_fn
#include <atomic>//原子操作
#include <map>

using namespace std;

#ifndef RECV_BUFFER_SIZE

#define RECV_BUFFER_SIZE 1024*10
#define SEND_BUFFER_SIZE 1024*10

#endif

//在服务端，由于要处理多个不同的客户端的数据
//因此每个客户端都应有其
//消息缓冲区还有指向消息缓冲区的数据尾部位置的变量
class ClientSocket
{
private:
	SOCKET m_client_sock;

	//自定义的接收缓冲区
	//char m_Recv[RECV_BUFFER_SIZE] = {};
	//不需要自定义的接收缓冲区，因为CellServer::RecvData()中recv()
	//每次能及时取数据放入消息缓冲区，接收缓冲区数据能得到及时清空
	//CellServer::RecvData()每次处理某个客户端的数据，
	//并会及时调用CellServer::OnNetMsg()，打印完该客户端发送的消息
	//每个客户端套接字句柄会唯一对应一个ClientSocket类指针，
	//相当于有自己的m_MsgBuf，select() 多路 I/O 作用下不会造成数据混乱

	//第二缓冲区 消息缓冲区
	char m_MsgBuf[RECV_BUFFER_SIZE] = {};
	//指向消息缓冲区的数据尾部位置
	int m_lastPos = 0;

	//第二缓冲区 发送缓冲区
	char m_SendBuf[SEND_BUFFER_SIZE] = {};
	//指向发送缓冲区的数据尾部位置
	int m_lastSendPos = 0;

public:
	ClientSocket(SOCKET m_client_sock = INVALID_SOCKET);

	SOCKET Get_m_client_sock();

	//char* Get_m_Recv();

	char* Get_m_MsgBuf();

	int Get_m_lastPos();

	void Set_m_lastPos(int NewPos);

	int SendData(DataHead* pHead);
};


//网络事件接口
//CellServer类的vec_client的元素数量减少时，通知EasyTcpServer类
//NetEvent类可以看成是个【代理类】，代理了EasyTcpServer类对象
class NetEvent
{
private:

public:
	//客户端加入事件
	virtual void NEOnNetJoin(ClientSocket* pClient) = 0;//纯虚函数

	//客户端离开事件
	virtual void NEOnNetLeave(ClientSocket* pClient) = 0;

	virtual void NEOnNetMsg(ClientSocket* client_sock, DataHead* pHead) = 0;

	//recv事件
	virtual void NERecv(ClientSocket* client_sock) = 0;
};


//作用：
//线程添加新客户端【到缓冲客户队列】、
//持续将缓冲客户队列内的新客户加入【正式客户队列】
//并持续监视是否存在待读取数据、
//接收数据、响应网络消息
class CellServer
{
private:
	SOCKET m_serv_sock;
	//自定义的接收缓冲区
	char m_Recv[RECV_BUFFER_SIZE] = {};
	//正式客户队列
	//vector<ClientSocket*> vec_client;
	//正式客户队列
	map<SOCKET, ClientSocket*> sock_pclient_pair;

	//缓冲客户队列
	vector<ClientSocket*> vec_client_buffer;
	//缓冲队列锁
	mutex m_mutex;
	//线程指针
	thread* m_pThread;
	//指向网络事件对象的指针
	//指向抽象基类的指针
	//（指向的对象在EasyTcpServer::StartThread()函数中创建，
	// 而且是个派生类对象，即指针指向派生类对象）
	NetEvent* m_pNetEvent;

	//备份 fdRead 中客户端socket
	fd_set m_fdReadBackUp;

	bool m_ClientsChange = false;

	SOCKET m_maxSocket;

public:
	//每次运行CellServer::OnNetMsg()，m_cnt增加1
	//被EasyTcpServer类内数据成员recvCnt使用，
	//体现在每过一秒对线程的m_cnt的值进行使用后，置m_cnt为0
	//atomic_int m_cnt;

	CellServer(SOCKET serv_sock = INVALID_SOCKET)
	{
		m_serv_sock = serv_sock;
		m_pThread = nullptr;
		//m_cnt = 0;
		m_pNetEvent = nullptr;
	}

	//***注***
	//此处不使用虚析构函数
	//因为该类不作为基类，没必要使用虚析构函数
	//而且一旦存在虚函数，就会有虚指针和虚表，在不同平台下数据结构会发生变化
	~CellServer()
	{
		delete m_pThread;
		Close();
		m_serv_sock = INVALID_SOCKET;
	}

	void Close(void);

	//被EasyTcpServer::StartThread()调用
	void Start();

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
	int RecvData(ClientSocket* pClientSocket);

	//响应网络消息
	void OnNetMsg(ClientSocket* pclient_sock, DataHead* pHead);

	//正式的和在缓冲区中的客户端总数
	//***注***
	//被EasyTcpServer::addClientToCellServer()内调用
	size_t getClientCnt();

	//被EasyTcpServer::addClientToCellServer()内调用
	void addClient(ClientSocket* pClient)
	{
		//也可以写成用自动锁
		//lock_guard<mutex>lg(mutex);

		m_mutex.lock();
		vec_client_buffer.push_back(pClient);
		m_mutex.unlock();
	}

	//设置m_pNetEvent指针指向EasyTcpServer类对象
	void setEventObj(NetEvent* event)
	{
		m_pNetEvent = event;
	}
};


//作用：
//服务端socket的创建，分配套接字地址、进入等待连接请求状态
//创建服务端线程并激活线程(serv->Start())、
//持续统计并显示所有线程每秒处理包数量、持续等待客户端的连接请求
//确保新的客户端被添加到当前客户端数量最少的线程服务端上，
//存储为【缓冲客户队列】
class EasyTcpServer : public NetEvent
{
private:
	SOCKET m_serv_sock;
	//vector<ClientSocket*> vec_client;
	vector<CellServer*> CellServers;

	Timestamp m_time;

	//自定义的接收缓冲区
	char m_Recv[RECV_BUFFER_SIZE] = {};

protected:
	//收到消息计数
	atomic_int m_MsgCnt;
	//客户端个数计数
	atomic_int m_clients_cnt;
	//RecvData()运行次数计数
	atomic_int m_RecvCnt;

public:
	EasyTcpServer();

	virtual ~EasyTcpServer();

	SOCKET initSocket();

	void Close();

	int Bind(const char* ip, unsigned short port);

	int Listen(int backlog);

	//创建服务端线程
	void StartThread(int CellServerThreadsCnt)
	{
		for (int n = 0; n < CellServerThreadsCnt; ++n)
		{
			auto serv = new CellServer(m_serv_sock);
			CellServers.push_back(serv);

			//注册网络事件
			//使CellServer的m_pNetEvent指向EasyTcpServer类对象
			serv->setEventObj(this);
			//***注***
			//正是由于继承了NetEvent类，setEventObj()的形参NetEvent*
			//才可被赋值为EasyTcpServer类对象的地址（向上强制转换）

			//启动消息处理线程
			serv->Start();
		}
	}

	//是否在正常工作中
	bool isRun()
	{
		return m_serv_sock != INVALID_SOCKET;
	}

	//统计所有线程每秒处理包数量
	void time4msg();

	//处理网络数据
	//查询是否有新客户端请求连接
	bool OnRun();

	//将调用addClientToCellServer()
	SOCKET Accept();

	//确保新的客户端被添加到当前客户端数量最少的CellServer上，
	//以分散服务器负载
	void addClientToCellServer(ClientSocket* pClient);

	virtual void NEOnNetJoin(ClientSocket* pClient);

	virtual void NEOnNetLeave(ClientSocket* pClient);

	virtual void NEOnNetMsg(ClientSocket* pclient_sock, DataHead* pHead);

	virtual void NERecv(ClientSocket* client_sock);

	//接收数据
	//处理粘包 拆分包
	//int RecvData(ClientSocket* pClientSocket);

	//响应网络消息
	//***注***
	//声明为virtual易于之后该类被继承，要重写响应网络消息函数时，
	//OnNetMsg()方法可获得多态性
	//void OnNetMsg(SOCKET client_sock, DataHead* pHead);

	//发送数据
	//int SendData(SOCKET client_sock, DataHead* pHead);

	//void SendDataToAll(DataHead* pHead);
};








