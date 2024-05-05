#pragma once 

#include "CellServer.hpp"

#include<thread>
#include<mutex>
#include <atomic>//原子操作
#include "CellNetWork.hpp"

using std::vector;
using std::atomic_int;
using std::cout;
using std::endl;

//作用：
//服务端socket的创建，分配套接字地址、进入等待连接请求状态
//创建服务端线程并激活线程(serv->Start())、
//持续统计并显示所有线程每秒处理包数量、持续等待客户端的连接请求
//确保新的客户端被添加到当前客户端数量最少的线程服务端上，
//存储为【缓冲客户队列】
class EasyTcpServer : public NetEvent
{
private:
	//创建线程管理类
	CellThread m_thread;

	SOCKET m_serv_sock=INVALID_SOCKET;
	//vector<ClientSocket*> vec_client;
	vector<CellServer*> CellServers;

	Timestamp m_time;

	//自定义的接收缓冲区
	//char m_Recv[RECV_BUFFER_SIZE] = {};

protected:
	//收到消息计数
	atomic_int m_MsgCnt;
	//客户端个数计数
	atomic_int m_clients_cnt;
	//RecvData()运行次数计数
	atomic_int m_RecvCnt;

public:
	EasyTcpServer()
	{
		atomic_init(&m_MsgCnt, 0);
		atomic_init(&m_RecvCnt, 0);
		atomic_init(&m_clients_cnt, 0);
	}

	virtual ~EasyTcpServer()
	{
		Close();
	}

	SOCKET initSocket()
	{
		CellNetWork::Init();

		//如果当前对象的套接字已经创建了，不允许重复创建套接字
		//那就关闭了，再重新创建一个
		if (m_serv_sock != INVALID_SOCKET)
		{
			CellLog::Info("<socket=%d>关闭旧连接\n",m_serv_sock);
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
			CellLog::Info("建立socket失败\n");
		else
			CellLog::Info("建立服务端socket=<%d>成功...\n", m_serv_sock);

		return m_serv_sock;
	}

	void Close()
	{
		CellLog::Info("EasyTcpServer Close() begin\n");

		m_thread.Close();

		if (m_serv_sock != INVALID_SOCKET)
		{
			//释放每个CellServer占用的内存空间
			for (auto x : CellServers)
				delete x;
			//容器元素清空
			CellServers.clear();

			// 关闭套节字
#ifdef _WIN32
			closesocket(m_serv_sock);
			//WSACleanup();    不再使用，而是交给~CellNetWork()
#else
			close(m_serv_sock);
#endif

			m_serv_sock = INVALID_SOCKET;
		}

		CellLog::Info("EasyTcpServer Close() end\n");
	}


	int Bind(const char* ip, unsigned short port)
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

		//bind 绑定用于接受客户端连接的服务端网络端口
		//Mac环境下将bind调用限定为使用全局命名空间中的函数
		int res = ::bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (res == SOCKET_ERROR)
			CellLog::Info("bind() ERROR，绑定网络端口<%d>失败...\n", port);
		else
			CellLog::Info("绑定网络端口<%d>成功...\n", port);

		return res;
	}


	int Listen(int backlog)
	{
		//  listen 监听网络端口
		int res = listen(m_serv_sock, backlog);

		if (res == SOCKET_ERROR)
			CellLog::Info("listen() ERROR，socket=<%d>监听网络端口失败\n",m_serv_sock);
		else
			CellLog::Info("socket=<%d>监听网络端口成功...\n",m_serv_sock);

		return res;
	}


	//创建服务端线程
	void StartThread(int CellServerThreadsCnt)
	{
		for (int n = 0; n < CellServerThreadsCnt; ++n)
		{
			auto serv = new CellServer(n+1);//传id值
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

		//运行StartThread()的主线程将运行完该函数，子线程继续执行OnRun()
		m_thread.Start(
			//onCreate
			nullptr,
			//onRun
			[this](CellThread* pThread) {
				OnRun(pThread);
			},
			//OnDestory
			nullptr);
	}

	//是否在正常工作中
	//bool isRun()
	//{
	//	return m_serv_sock != INVALID_SOCKET;
	//}

	//统计所有线程每秒处理包数量
	void time4msg()
	{
		auto t = m_time.getElapsedTimeInSecond();
		if (t >= 1.0)
		{
			//int recvCnt = 0;

			//for (auto ser : CellServers)
			//{
			//	recvCnt += ser->m_cnt;
			//	ser->m_cnt = 0;
			//}

			//***注***
			//这样写报错：将类"std::atomic<int>"作为可变参数函数的参数的非标准用法
			//因为std::atomic<int>对象是不可复制的
			/*
			printf("线程数<%d>, 持续时间<%lf>, 服务端套接字<%d>, \
			客户端连接数<%d>, 所有线程每秒处理包数量=%d\n",
			(int)CellServers.size(), t, m_serv_sock, m_clients_cnt,
			static_cast<int>(m_RecvCnt / t));
			*/

			//解决：
			//使用std::atomic<T>::load()方法，返回原子变量的当前值。
			CellLog::Info("线程数<%d>, 持续时间<%lf>, 服务端套接字<%d>, \
客户端连接数<%d>, 所有线程每秒运行RecvData()次数=%d, 所有线程每秒处理包数量=%d\n",
(int)CellServers.size(), t, m_serv_sock, m_clients_cnt.load(),
static_cast<int>(m_RecvCnt.load() / t), static_cast<int>(m_MsgCnt.load() / t));

			m_MsgCnt = m_RecvCnt = 0;
			m_time.update();
		}
	}


	//处理网络数据
	//查询是否有新客户端请求连接
	void OnRun(CellThread* pThread)
	{
		while (pThread->isRun())
		{
			time4msg();

			// 4 创建timeval结构布局的结构变量timeout以作为select数的第五个实参
			//为了防止陷入无限阻塞的状态，使用 timeout 传递超时信息。
			timeval timeout;

			//设置超时时间
			timeout.tv_sec = 0;
			timeout.tv_usec = 1;
			//或
			//struct timeval timeout = { 5,0 };

			//伯克利socket	 BSD socket

			// 1 创建fd_set结构体将要监视的套接字句柄集中到一起，以监视这些套接字句柄
			fd_set fdRead;//关注 是否存在待读取数据
			//fd_set fdWrite;
			//fd_set fdExp;

			//***注***
			// Windows的fd_set由成员 fd_count和fd_array构成， fd_count用于套接字句柄数，
			// fd_array是个数组集合，用于保存套接字句柄
			//基于Windows的套接字句柄不仅不能从零开始，
			//而且在生成的句柄的整数值之间也找不到规则，因此
			//需要一个数组来保存套接字的句柄以及一个记录句柄数的变量

			// 2 使用宏来完成对结构体中所有位都设置为0的操作
			//清空fdRead句柄集合，fdWrite句柄集合，fdExp句柄集合
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);

			// 3 使用宏来向结构体中注册套接字句柄serv_sock的信息
			// 将套接字句柄serv_sock对应的位设置为1，即
			// 将套接字句柄（注册）添加到fdRead集合、fdWrite集合、fdExp集合中
			// 需要监视serv_sock是否有读、写、异常
			FD_SET(m_serv_sock, &fdRead);
			//FD_SET(m_serv_sock, &fdWrite);
			//FD_SET(m_serv_sock, &fdExp);

			//创建maxSocket来存储所有客户端加入后的套接字的最大值
			//SOCKET maxSocket = m_serv_sock;
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
			/*
			for (int n = (int)vec_client.size() - 1; n >= 0; --n)
			{
				//FD_SET(vec_client[n]->Get_m_client_sock(), &fdRead);
				maxSocket = maxSocket < (vec_client[n]->Get_m_client_sock()) ?
					(vec_client[n]->Get_m_client_sock()) : maxSocket;
			}
			*/

			// cout << "fdRead.fd_count = " << fdRead.fd_count << endl;

			// 5 使用select函数
			// 对于第一个参数，int nfds，是指fd_set集合中所有套接字句柄的范围，
			// 而不是数量，是所有句柄的最大值+1，在windows中可以写0
			// 要写为socket值【最大的一个socket值再加一】
			int fd_num = select(m_serv_sock + 1, &fdRead, nullptr, nullptr, &timeout);
			if (fd_num == -1)
			{
				CellLog::Info("EasyTcpServer.OnRun select error exit\n");
				pThread->Exit();
				break;
			}
			else if (fd_num == 0)
			{

			}
			//cout << "fd_num=" << fd_num << ", cnt=" << cnt++ << endl;

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

			/*
			for (int n = (int)vec_client.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(vec_client[n]->Get_m_client_sock(), &fdRead))
				{
					if (RecvData(vec_client[n]) == -1)
					{
						auto it = vec_client.begin() + n;
						if (it != vec_client.end())
						{
							delete* it;
							vec_client.erase(it);
						}
					}
				}
			}
			*/

		}

		return;
	}

	//将调用addClientToCellServer()
	SOCKET Accept()
	{
		SOCKET client_sock = INVALID_SOCKET;
		sockaddr_in client_adr = {};
		int client_adr_size = sizeof(client_adr);

		//  accept 等待接受客户端连接
#ifdef _WIN32
		client_sock = accept(m_serv_sock, (sockaddr*)&client_adr, &client_adr_size);
#else
		client_sock = accept(m_serv_sock, (sockaddr*)&client_adr, (socklen_t*)&client_adr_size);
#endif

		if (client_sock == INVALID_SOCKET)
			CellLog::Info("socket=<%d>错误，接受到无效客户端SOCKET", m_serv_sock);
		else
		{
			//inet_ntoa()将网络字节序的整数型 IP 地址转换为字符串形式
			/*
			cout << "socket=<" << m_serv_sock << ">新客户端加入：IP = "
				<< inet_ntoa(client_adr.sin_addr)
				<< "  socket=<" << client_sock << ">" << endl;
			*/

			//printf()更快
			/*
			printf("socket=<%d>新客户端加入：IP = %s   client_sock=<%d> ,共%d个客户端加入\n",
				m_serv_sock, inet_ntoa(client_adr.sin_addr), client_sock, m_clients_cnt + 1);
			*/
			//通知该客户端之前的所有客户端有新用户加入
			//NewUserJoin newuserjoin;
			//SendDataToAll(&newuserjoin);

			//vec_client.push_back(new ClientSocket(client_sock));

			addClientToCellServer(new ClientSocket(client_sock));
		}

		return client_sock;
	}


	//确保新的客户端被添加到当前客户端数量最少的CellServer上，
	//以分散服务器负载
	void addClientToCellServer(ClientSocket* pClient)
	{
		//vec_client.push_back(pClient);

		//查找客户数量最少的CellServer
		auto pMinServer = CellServers[0];

		//错误写法：
		//for (auto pCellServer : CellServers)
		//	pMinServer = std::min(pMinServer->getClientCnt(),pCellServer->getClientCnt());

		for (auto pCellServer : CellServers)
			if (pMinServer->getClientCnt() > pCellServer->getClientCnt())
				pMinServer = pCellServer;

		pMinServer->addClient(pClient);
	}


	virtual void NEOnNetJoin(ClientSocket* pClient)
	{
		++m_clients_cnt;
	}

	virtual void NEOnNetLeave(ClientSocket* pClient)
	{
		--m_clients_cnt;
	}

	virtual void NEOnNetMsg(CellServer* pCellServer, ClientSocket* client_sock, DataHead* pHead)
	{
		++m_MsgCnt;
	}

	virtual void NERecv(ClientSocket* client_sock)
	{
		++m_RecvCnt;
	}

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
