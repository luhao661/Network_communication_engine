#if 0
#include "EasyTcpServer_1.6.hpp"

ClientSocket::ClientSocket(SOCKET sock)
{
	m_client_sock = sock;
	memset(m_MsgBuf, 0, sizeof(m_MsgBuf));
}

SOCKET ClientSocket::Get_m_client_sock()
{
	return m_client_sock;
}

//char* ClientSocket::Get_m_Recv()
//{
//	return m_Recv;
//}

char* ClientSocket::Get_m_MsgBuf()
{
	return m_MsgBuf;
}

int ClientSocket::Get_m_lastPos()
{
	return m_lastPos;
}

void ClientSocket::Set_m_lastPos(int NewPos)
{
	m_lastPos = NewPos;
}

/**************************************************************************************/

void CellServer::Close(void)
{
	if (m_serv_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		//关闭全部的客户端套接字
		for (int n = vec_client.size() - 1; n >= 0; --n)
		{
			closesocket(vec_client[n]->Get_m_client_sock());
			delete vec_client[n];
		}

		//  关闭套节字closesocket
		closesocket(m_serv_sock);

		//注销
		//WSACleanup();
#else

		//关闭全部的客户端套接字
		for (int n = (int)vec_client.size() - 1; n >= 0; --n)
		{
			close(vec_client[n]->Get_m_client_sock());
			delete vec_client[n];
		}

		//  关闭套节字closesocket
		close(m_serv_sock);
		m_serv_sock = INVALID_SOCKET;

#endif

		vec_client.clear();
	}
}

void CellServer::Start()
{
	//或写成
	//thread (&CellServer::OnRun,this);

	//函数适配器mem_fn
	// 函数模板 std::mem_fn 生成指向成员指针的包装对象，
	// 它可以存储、复制及调用指向成员指针。
	// 到对象的引用和指针（含智能指针）
	// 相当于进行一个更安全的转换
	m_pThread = new thread(mem_fn(&CellServer::OnRun), this);
	//***理解***
	//mem_fn(&CellServer::OnRun)：mem_fn 是 C++ 标准库中的模板函数，用于
	// 将【成员函数】存储为可调用对象。在这里，&CellServer::OnRun 是 CellServer 类
	// 的成员函数 OnRun 的指针，通过 mem_fn 这个函数模板将其转换为可调用对象。
	//thread t(...)：创建一个新的线程对象 t，并将括号内的参数作为线程的执行函数。
	//这里传递了 mem_fn(&CellServer::OnRun)，因此线程将执行 CellServer 类的
	//  OnRun 成员函数。
	//this：表示当前对象的指针。在这个上下文中，它是指向 EasyTcpServer 对象的指针。
	// 该指针作为参数传递给 CellServer::OnRun 成员函数，
	// 在新线程中执行 CellServer::OnRun 时，
	// 可以通过 this 指针访问 EasyTcpServer 对象的成员变量和方法。
}

bool CellServer::OnRun()
{
	//if (!isRun())
	//	return false;

	while (isRun())
	{
		//将缓冲客户队列内的新客户加入正式客户队列
		//操作需要加锁解锁
		if (vec_client_buffer.size() > 0)
		{
			lock_guard<mutex>lg(m_mutex);

			for (auto pClient : vec_client_buffer)
				vec_client.push_back(pClient);

			vec_client_buffer.clear();
		}

		//如果没有需要处理的客户端，就跳到入口循环条件继续向下执行
		if (vec_client.empty())
		{
			//使用标准库提供的休眠
			chrono::milliseconds t(1);
			this_thread::sleep_for(t);

			continue;
		}

		// 4 创建timeval结构布局的结构变量timeout以作为select数的第五个实参
		//为了防止陷入无限阻塞的状态，使用 timeout 传递超时信息。
		//struct timeval timeout;

		//设置5秒的超时时间
		//timeout.tv_sec = 1;
		//timeout.tv_usec = 0;
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
		//FD_SET(m_serv_sock, &fdRead);
		//FD_SET(m_serv_sock, &fdWrite);
		//FD_SET(m_serv_sock, &fdExp);


		//创建maxSocket来存储所有客户端加入后的套接字的最大值
		SOCKET maxSocket = vec_client[0]->Get_m_client_sock();
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
			FD_SET(vec_client[n]->Get_m_client_sock(), &fdRead);
			maxSocket = maxSocket < (vec_client[n]->Get_m_client_sock()) ?
				(vec_client[n]->Get_m_client_sock()) : maxSocket;
		}

		// cout << "fdRead.fd_count = " << fdRead.fd_count << endl;

		// 5 使用select函数
		// 对于第一个参数，int nfds，是指fd_set集合中所有套接字句柄的范围，
		// 而不是数量，是所有句柄的最大值+1，在windows中可以写0
		// 要写为socket值【最大的一个socket值再加一】

		//select()第五个参数设为nullptr，因为CellServer只用来接收数据，
		//不干别的事情，
		// 所以只要阻塞在此处等待客户端传来数据就可以了
		int fd_num = select(m_serv_sock + 1, &fdRead, nullptr, nullptr, nullptr);
		if (fd_num == -1)
		{
			cout << "select任务结束" << endl;
			Close();
			return false;
		}
		else if (fd_num == 0)
		{

		}
		//cout << "fd_num=" << fd_num << ", cnt=" << cnt++ << endl;

		//用FD_ISSET宏筛选在fdRead集合中发生状态变化的句柄
		//检查服务器端套接字（serv_sock）是否处于待读取数据的状态
		//如果是服务器端套接字的变化，表示有客户端连接请求到达，将受理连接请求。
		/*
		if (FD_ISSET(m_serv_sock, &fdRead))
		{
			//从fdRead集合中删除该句柄
			//在后续的 I/O 多路复用操作中不再监听或检查该句柄的状态变化
			//（不写也没关系）
			FD_CLR(m_serv_sock, &fdRead);

			Accept();
		}
		*/

		for (int n = (int)vec_client.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(vec_client[n]->Get_m_client_sock(), &fdRead))
			{
				if (RecvData(vec_client[n]) == -1)
				{
					auto it = vec_client.begin() + n;
					if (it != vec_client.end())
					{
						//调用EasyTcpServer类对象的OnLeave()方法
						//EasyTcpServer类的vec_client元素得以减少
						if (m_pNetEvent)
							m_pNetEvent->OnLeave(vec_client[n]);

						delete *it;
						vec_client.erase(it);
					}
				}
			}
		}
	}
}

int CellServer::RecvData(ClientSocket* pClientSocket)
{
	//接收客户端数据存到【服务端的】自定义接收缓冲区m_Recv
	int len = (int)recv(pClientSocket->Get_m_client_sock(),
		m_Recv, RECV_BUFFER_SIZE, 0);

	//cout << "len = " << len << endl;

	if (len <= 0)
	{
		cout << "客户端<socket=" << pClientSocket->Get_m_client_sock()
			<< ">已退出。\n";
		return -1;
	}

	//将【服务端的】自定义接收缓冲区m_Recv的数据
	//拷贝到【某个客户端对应的】消息缓冲区
	memcpy(pClientSocket->Get_m_MsgBuf() + pClientSocket->Get_m_lastPos(),
		m_Recv, len);
	//表示消息缓冲区的数据尾部的位置的变量m_lastPos后移
	pClientSocket->Set_m_lastPos(pClientSocket->Get_m_lastPos() + len);

	//判断消息缓冲区的数据长度是否大于消息头的长度
	//用while循环，解决【粘包】
	while (pClientSocket->Get_m_lastPos() >= sizeof(DataHead))
	{
		//指向某个客户端对应的m_MsgBuf的指针解释为DataHead*类型的指针，
		//用于访问DataHead的数据成员
		DataHead* pHead = reinterpret_cast<DataHead*>(pClientSocket->Get_m_MsgBuf());

		//判断消息缓冲区的数据长度是否大于消息长度
		//解决【少包】的问题
		if (pClientSocket->Get_m_lastPos() >= pHead->datalength)
		{
			//处理网络消息
			OnNetMsg(pClientSocket->Get_m_client_sock(), pHead);

			//暂存表示自定义的消息缓冲区中剩余未处理的数据的长度的变量
			int unprocessed = pClientSocket->Get_m_lastPos() - pHead->datalength;

			//将m_MsgBuf中已经处理过的消息数据用其后面的未处理的数据进行
			//数据覆盖
			memcpy(pClientSocket->Get_m_MsgBuf(),
				pClientSocket->Get_m_MsgBuf() + pHead->datalength, unprocessed);

			//更新m_MsgBuf中数据尾部的位置
			pClientSocket->Set_m_lastPos(unprocessed);
		}
		else
			break;
	}

	return 0;
}

void CellServer::OnNetMsg(SOCKET client_sock, DataHead* pHead)
{
	++m_cnt;

	//处理包数量加一
	/*
	++m_RecvCnt;

	auto t = m_time.getElapsedTimeInSecond();
	if (t >= 1.0)
	{
		printf("Duration time<%lf>, socket<%d>, \
连接数=%d, 每秒处理包数量=%d\n",
t, (int)client_sock, vec_client.size(), m_RecvCnt);

			m_RecvCnt = 0;
			m_time.update();
		}
		*/
		//  处理请求
	switch (pHead->cmd)
	{
	case CMD_LOGIN:
	{
		LogIn* login = reinterpret_cast<LogIn*>(pHead);

		//忽略判断用户密码是否正确的过程

		//***注***
		//写以下代码，会导致服务端和客户端在进行通信后不久就不动了
		//cout << "收到客户端<socket=" << client_sock << ">命令：CMD_LOGIN"
		//	<< " 数据长度：" << login->datalength << endl;
		//cout << "用户名：" << login->username << "登入" << endl;

		// 发送报文
		//LogInResult res{};
		//send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
		//可以改写成
		//SendData(client_sock, &res);
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
		cout << "收到客户端<socket=" << client_sock << ">未定义消息"
			<< " 数据长度：" << pHead->datalength << endl;

		//发送含有CMD_ERROR的数据包
		DataHead head;
		send(client_sock, (const char*)&head, sizeof(DataHead), 0);
	}
	}
}

size_t CellServer::getClientCnt()
{
	return vec_client.size() + vec_client_buffer.size();
}

/**************************************************************************************/
EasyTcpServer::EasyTcpServer()
	:m_serv_sock(INVALID_SOCKET)
{}

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
		//for (int n = vec_client.size() - 1; n >= 0; --n)
		//{
		//	closesocket(vec_client[n]->Get_m_client_sock());
		//	delete vec_client[n];
		//}

		//  关闭套节字closesocket
		closesocket(m_serv_sock);

		//注销
		WSACleanup();
#else

		//关闭全部的客户端套接字
		for (int n = (int)vec_client.size() - 1; n >= 0; --n)
		{
			close(vec_client[n]->Get_m_client_sock());
			delete vec_client[n];
		}

		//  关闭套节字closesocket
		close(m_serv_sock);
		m_serv_sock = INVALID_SOCKET;

#endif

		vec_client.clear();
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

	//bind 绑定用于接受客户端连接的服务端网络端口
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

void EasyTcpServer::time4msg()
{
	auto t = m_time.getElapsedTimeInSecond();
	if (t >= 1.0)
	{
		int recvCnt = 0;

		for (auto ser : CellServers)
		{
			recvCnt += ser->m_cnt;
			ser->m_cnt = 0;
		}

		printf("线程数<%d>, 持续时间<%lf>, 服务端套接字<%d>, \
客户端连接数<%d>, 所有线程每秒处理包数量=%d\n",
(int)CellServers.size(), t, m_serv_sock, (int)vec_client.size(), static_cast<int>(recvCnt / t));

		m_time.update();
	}
}

//处理网络数据
//查询是否有待读取的数据
bool EasyTcpServer::OnRun()
{
	if (!isRun())
		return false;

	time4msg();

	// 4 创建timeval结构布局的结构变量timeout以作为select数的第五个实参
	//为了防止陷入无限阻塞的状态，使用 timeout 传递超时信息。
	struct timeval timeout;

	//设置超时时间
	timeout.tv_sec = 0;
	timeout.tv_usec = 10;
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
		cout << "select任务结束" << endl;
		Close();
		return false;
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

	return true;
}

SOCKET EasyTcpServer::Accept()
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
		cout << "socket=<" << m_serv_sock << ">错误，接受到无效客户端SOCKET";
	else
	{
		//inet_ntoa()将网络字节序的整数型 IP 地址转换为字符串形式
		/*
		cout << "socket=<" << m_serv_sock << ">新客户端加入：IP = "
			<< inet_ntoa(client_adr.sin_addr)
			<< "  socket=<" << client_sock << ">" << endl;
		*/

		//printf()更快
		printf("socket=<%d>新客户端加入：IP = %s   client_sock=<%d> ,共%d个客户端加入\n",
			m_serv_sock, inet_ntoa(client_adr.sin_addr), client_sock, vec_client.size() + 1);

		//通知该客户端之前的所有客户端有新用户加入
		//NewUserJoin newuserjoin;
		//SendDataToAll(&newuserjoin);

		//vec_client.push_back(new ClientSocket(client_sock));

		addClientToCellServer(new ClientSocket(client_sock));
	}

	return client_sock;
}

void EasyTcpServer::addClientToCellServer(ClientSocket* pClient)
{
	vec_client.push_back(pClient);

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

void EasyTcpServer::OnLeave(ClientSocket* pClient)
{
	for (int n = (int)vec_client.size() - 1; n >= 0; --n)
	{
		if (vec_client[n] == pClient)
		{
			auto it = vec_client.begin() + n;
			if (it != vec_client.end())
				vec_client.erase(it);
		}
	}
}

//接收数据
//处理粘包 拆分包
#if 0
int EasyTcpServer::RecvData(ClientSocket* pClientSocket)
{
	//  接收客户端数据存到【某客户端对应的的】自定义接收缓冲区m_Recv
	int len = (int)recv(pClientSocket->Get_m_client_sock(),
		pClientSocket->Get_m_Recv(), RECV_BUFFER_SIZE, 0);

	if (len <= 0)
	{
		cout << "客户端<socket=" << pClientSocket->Get_m_client_sock()
			<< ">已退出。\n";
		return -1;
	}

	//将【某客户端对应的】自定义接收缓冲区m_Recv的数据
	//拷贝到【某个客户端对应的】消息缓冲区
	memcpy(pClientSocket->Get_m_MsgBuf() + pClientSocket->Get_m_lastPos(),
		pClientSocket->Get_m_Recv(), len);
	//表示消息缓冲区的数据尾部的位置的变量m_lastPos后移
	pClientSocket->Set_m_lastPos(pClientSocket->Get_m_lastPos() + len);

	//判断消息缓冲区的数据长度是否大于消息头的长度
	//用while循环，解决【粘包】
	while (pClientSocket->Get_m_lastPos() >= sizeof(DataHead))
	{
		//指向某个客户端对应的m_MsgBuf的指针解释为DataHead*类型的指针，
		//用于访问DataHead的数据成员
		DataHead* pHead = reinterpret_cast<DataHead*>(pClientSocket->Get_m_MsgBuf());

		//判断消息缓冲区的数据长度是否大于消息长度
		//解决【少包】的问题
		if (pClientSocket->Get_m_lastPos() >= pHead->datalength)
		{
			//处理网络消息
			OnNetMsg(pClientSocket->Get_m_client_sock(), pHead);

			//暂存表示自定义的消息缓冲区中剩余未处理的数据的长度的变量
			int unprocessed = pClientSocket->Get_m_lastPos() - pHead->datalength;

			//将m_MsgBuf中已经处理过的消息数据用其后面的未处理的数据进行
			//数据覆盖
			memcpy(pClientSocket->Get_m_MsgBuf(),
				pClientSocket->Get_m_MsgBuf() + pHead->datalength, unprocessed);

			//更新m_MsgBuf中数据尾部的位置
			pClientSocket->Set_m_lastPos(unprocessed);
		}
		else
			break;
	}

	return 0;
}
#endif

#if 0
int EasyTcpServer::RecvData(ClientSocket* pClientSocket)
{
	//接收客户端数据存到【服务端的】自定义接收缓冲区m_Recv
	int len = (int)recv(pClientSocket->Get_m_client_sock(),
		m_Recv, RECV_BUFFER_SIZE, 0);

	//cout << "len = " << len << endl;

	if (len <= 0)
	{
		cout << "客户端<socket=" << pClientSocket->Get_m_client_sock()
			<< ">已退出。\n";
		return -1;
	}

	//将【服务端的】自定义接收缓冲区m_Recv的数据
	//拷贝到【某个客户端对应的】消息缓冲区
	memcpy(pClientSocket->Get_m_MsgBuf() + pClientSocket->Get_m_lastPos(),
		m_Recv, len);
	//表示消息缓冲区的数据尾部的位置的变量m_lastPos后移
	pClientSocket->Set_m_lastPos(pClientSocket->Get_m_lastPos() + len);

	//判断消息缓冲区的数据长度是否大于消息头的长度
	//用while循环，解决【粘包】
	while (pClientSocket->Get_m_lastPos() >= sizeof(DataHead))
	{
		//指向某个客户端对应的m_MsgBuf的指针解释为DataHead*类型的指针，
		//用于访问DataHead的数据成员
		DataHead* pHead = reinterpret_cast<DataHead*>(pClientSocket->Get_m_MsgBuf());

		//判断消息缓冲区的数据长度是否大于消息长度
		//解决【少包】的问题
		if (pClientSocket->Get_m_lastPos() >= pHead->datalength)
		{
			//处理网络消息
			OnNetMsg(pClientSocket->Get_m_client_sock(), pHead);

			//暂存表示自定义的消息缓冲区中剩余未处理的数据的长度的变量
			int unprocessed = pClientSocket->Get_m_lastPos() - pHead->datalength;

			//将m_MsgBuf中已经处理过的消息数据用其后面的未处理的数据进行
			//数据覆盖
			memcpy(pClientSocket->Get_m_MsgBuf(),
				pClientSocket->Get_m_MsgBuf() + pHead->datalength, unprocessed);

			//更新m_MsgBuf中数据尾部的位置
			pClientSocket->Set_m_lastPos(unprocessed);
		}
		else
			break;
	}

	return 0;
}
#endif

/*
void EasyTcpServer::OnNetMsg(SOCKET client_sock, DataHead* pHead);
{}

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
		SendData(vec_client[n]->Get_m_client_sock(), pHead);
	}
}
*/
#endif