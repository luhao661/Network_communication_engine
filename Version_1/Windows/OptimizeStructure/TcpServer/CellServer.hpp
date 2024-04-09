#pragma once

#include "NetEvent.hpp"

#include<map>
#include<vector>
#include <thread>
#include <functional>//mem_fn
#include <chrono>

using std::mutex, std::vector, std::map, std::mem_fn, std::lock_guard;
using std::thread;
using std::cout,std::endl;


//作用：
//线程添加新客户端【到缓冲客户队列】、
//持续将缓冲客户队列内的新客户加入【正式客户队列】
//并持续监视是否存在待读取数据、
//接收数据、响应网络消息
class CellServer
{
private:
	//SOCKET m_serv_sock;//m_serv_sock暂时是没有意义的，故注释掉
	//要调试观察是哪个CellServer关闭了，每个CellServer对象就要有个专属的id值
	int m_id = -1;

	//是否在正常工作中
	bool m_isRun = false;

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
	//thread* m_pThread;
	//指向网络事件对象的指针
	//指向抽象基类的指针
	//（指向的对象在EasyTcpServer::StartThread()函数中创建，
	// 而且是个派生类对象，即指针指向派生类对象）
	NetEvent* m_pNetEvent;

	//备份 fdRead 中客户端socket
	fd_set m_fdReadBackUp;

	bool m_ClientsChange = false;

	SOCKET m_maxSocket=INVALID_SOCKET;

	//每个CellServer对应一个CellTaskServer类对象
	CellTaskServer m_CellTaskServer;

	//旧的时间戳
	time_t OldTime = CellTime::getNowInMillisecond();

public:
	//每次运行CellServer::OnNetMsg()，m_cnt增加1
	//被EasyTcpServer类内数据成员recvCnt使用，
	//体现在每过一秒对线程的m_cnt的值进行使用后，置m_cnt为0
	//atomic_int m_cnt;

	CellServer(int id)
	{
		m_id = id;
		m_CellTaskServer.m_CellServer_id = id;
		//m_pThread = nullptr;
		//m_cnt = 0;
		m_pNetEvent = nullptr;
	}

	//***注***
	//此处不使用虚析构函数
	//因为该类不作为基类，没必要使用虚析构函数
	//而且一旦存在虚函数，就会有虚指针和虚表，在不同平台下数据结构会发生变化
	~CellServer()
	{
		printf("CellServer %d ~CellServer() begin\n", m_id);
		//delete m_pThread;
		Close();
		//m_serv_sock = INVALID_SOCKET;
		printf("CellServer %d ~CellServer() end\n", m_id);
	}

	void Close(void)
	{
		printf("CellServer %d Close() begin\n",m_id);

		m_isRun = false;

		//关闭执行任务的服务类对象
		m_CellTaskServer.Close();


		//关闭客户端套接字
		for (auto pair : sock_pclient_pair)
		{
			delete pair.second;
		}

		//错误：
		// 创建CellServer类对象时传入构造函数的实参是
		// EasyTcpServer类创建的服务端套接字
		// CellServer没有权利去关闭EasyTcpServer类创建的服务端套接字
		//closesocket(m_serv_sock);

		//在该函数中也不再进行关闭客户端套接字的操作
		//而是CellClient类中	~ClientSocket()方法来关闭客户端套接字

		//再次优化：直接不要m_serv_sock，因为CellServer目前没有对该变量有打印上的使用

		//清空缓冲区客户队列，关闭客户端套接字
		for (auto x : vec_client_buffer)
		{
			delete x;
		}

		sock_pclient_pair.clear();
		vec_client_buffer.clear();

		//m_serv_sock = INVALID_SOCKET;
		//***注***
		//此处可以将m_serv_sock 设置为 INVALID_SOCKET
		//因为该m_serv_sock是EasyTcpServer类创建的服务端套接字的拷贝


		printf("CellServer %d Close() end\n", m_id);
	}

	//被EasyTcpServer::StartThread()调用
	void Start()
	{
		if (!m_isRun)
		{
			m_isRun = true;

			//或写成
			//thread (&CellServer::OnRun,this);

			//函数适配器mem_fn
			// 函数模板 std::mem_fn 生成指向成员指针的包装对象，
			// 它可以存储、复制及调用指向成员指针。
			// 到对象的引用和指针（含智能指针）
			// 相当于进行一个更安全的转换
			thread Thread = thread(mem_fn(&CellServer::OnRun), this);
			Thread.detach();
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

			//调用CellTaskServer类对象的Start()方法，不断等待任务的到来，并添加具体任务到list容器
			m_CellTaskServer.Start();		
		}
	}

	//是否在正常工作中
	//bool m_isRun()
	//{
	//	return m_serv_sock != INVALID_SOCKET;
	//}

	//处理网络数据
	//查询是否有待读取的数据
	bool OnRun()
	{
		//if (!m_isRun())
		//	return false;

		while (m_isRun)
		{
			//将缓冲客户队列内的新客户加入正式客户队列
			//操作需要加锁解锁
			if (vec_client_buffer.size() > 0)
			{
				lock_guard<mutex>lg(m_mutex);

				for (auto pClient : vec_client_buffer)
				{
					sock_pclient_pair.insert
					(std::make_pair(pClient->Get_m_client_sock(), pClient));

					pClient->m_CellServer_id = m_id;

					if(m_pNetEvent)
						m_pNetEvent->NEOnNetJoin(pClient);
				}

				vec_client_buffer.clear();

				//表示有新客户端加入
				m_ClientsChange = true;
			}

			//如果没有需要处理的客户端，就跳到入口循环条件继续向下执行
			if (sock_pclient_pair.empty())
			{
				//使用标准库提供的休眠
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);

				//旧的时间戳
				time_t OldTime = CellTime::getNowInMillisecond();

				continue;
			}

			// 4 创建timeval结构布局的结构变量timeout以作为select数的第五个实参
			//为了防止陷入无限阻塞的状态，使用 timeout 传递超时信息。
			struct timeval timeout;

			//设置1秒的超时时间
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
			//FD_SET(m_serv_sock, &fdRead);
			//FD_SET(m_serv_sock, &fdWrite);
			//FD_SET(m_serv_sock, &fdExp);

			if (m_ClientsChange)
			{
				m_ClientsChange = false;

				m_maxSocket = sock_pclient_pair.begin()->second->Get_m_client_sock();
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
				for (auto it = sock_pclient_pair.begin(); it != sock_pclient_pair.end(); ++it)
				{
					FD_SET(it->second->Get_m_client_sock(), &fdRead);

					m_maxSocket = m_maxSocket < (it->second->Get_m_client_sock()) ?
						(it->second->Get_m_client_sock()) : m_maxSocket;
				}

				memcpy(&m_fdReadBackUp, &fdRead, sizeof(fd_set));
			}
			else
				memcpy(&fdRead, &m_fdReadBackUp, sizeof(fd_set));


			// cout << "fdRead.fd_count = " << fdRead.fd_count << endl;

			// 5 使用select函数
			// 对于第一个参数，int nfds，是指fd_set集合中所有套接字句柄的范围，
			// 而不是数量，是所有句柄的最大值+1，在windows中可以写0
			// 要写为socket值【最大的一个socket值再加一】

			//select()第五个参数设为nullptr，因为CellServer只用来接收数据，
			//不干别的事情，
			// 所以只要阻塞在此处等待客户端传来数据就可以了
			int fd_num = select(m_maxSocket + 1, &fdRead, nullptr, nullptr, &timeout);
			if (fd_num == -1)
			{
				cout << "select任务结束" << endl;
				Close();
				return false;
			}
			else if (fd_num == 0)
			{

			}

			ReadData(fdRead);
			CheckHearTTime();
		}

		printf("CellServer %d OnRun() exit\n", m_id);
	}


	void CheckHearTTime()
	{
		//新的时间戳
		auto NowTime=CellTime::getNowInMillisecond();

		auto durationTime = NowTime - OldTime;

		OldTime = NowTime;

		for (auto iter = sock_pclient_pair.begin(); iter != sock_pclient_pair.end(); )
		{
			//心跳检测
			if (iter->second->IsDead(durationTime))
			{
				if (m_pNetEvent)
					m_pNetEvent->NEOnNetLeave(iter->second);

				//先删除new出来的ClientSocket类对象
				delete iter->second;
				//关闭客户端连接
				closesocket(iter->first);

				//再删除map中的pair元素
				//错误写法：
				//iter = sock_pclient_pair.erase(iter->first);
				//正确写法：
				iter = sock_pclient_pair.erase(iter);
				//理解：map的erase有两种重载函数：size_type erase( const Key& key ); 	iterator erase(iterator pos);

				m_ClientsChange = true;
			}
			else
			{
				iter->second->IsSend(durationTime);
				++iter;
			}
		}
	}


	//在bool OnRun()中分离业务
	void ReadData(fd_set& fdRead)
	{
		//以下写法查找待读取的套接字句柄时，时间复杂度为O(n)，可以优化
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
					//调用EasyTcpServer类对象的OnLeave()方法
					//EasyTcpServer类的vec_client元素得以减少
					if (m_pNetEvent)
						m_pNetEvent->NEOnNetLeave(vec_client[n]);

					delete* it;
					vec_client.erase(it);
				}
			}
		}
	}
	*/
#ifdef _WIN32

		for (int n = 0; n < fdRead.fd_count; ++n)
		{
			//提升查询性能
			auto iter = sock_pclient_pair.find(fdRead.fd_array[n]);
			if (iter != sock_pclient_pair.end())
			{
				if (-1 == RecvData(iter->second))
				{
					if (m_pNetEvent)
						m_pNetEvent->NEOnNetLeave(iter->second);

					//先删除new出来的ClientSocket类对象
					delete iter->second;

					//这边不再关闭客户端连接
					//closesocket(iter->first);
					//因为delete iter->second;调用了析构函数，将调用closesocket()

					//再删除map中的pair元素
					//sock_pclient_pair.erase(iter->first);
					sock_pclient_pair.erase(iter);//效率更高

					m_ClientsChange = true;
				}
			}
			else
			{
				printf("error. if (iter != sock_pclient_pair.end())\n");
			}

		}
#else
	    //Linux没有fd_count
		std::vector<ClientSocket*> temp;
		for (auto pair : sock_pclient_pair)
		{
			if (FD_ISSET(pair.second->Get_m_client_sock(), &fdRead))
			{
				if (-1 == RecvData(pair.second))
				{
					if (m_pNetEvent)
						m_pNetEvent->NEOnNetLeave(pair.second);

					m_ClientsChange = true;

					//关闭客户端连接
					//close(pair.first);

					temp.push_back(pair.second);
				}
			}
		}
		for (auto pClient : temp)
		{
			delete pClient;

			sock_pclient_pair.erase(pClient->Get_m_client_sock());
		}
#endif
	}


	//接收数据
	//处理粘包 拆分包
	int RecvData(ClientSocket* pClientSocket)
	{
		//接收客户端数据
		char* pRecv = pClientSocket->Get_m_MsgBuf() + pClientSocket->Get_m_lastPos();

		//接收客户端数据存到【服务端的】自定义接收缓冲区m_Recv
		int len = (int)recv(pClientSocket->Get_m_client_sock(),
			pRecv, (RECV_BUFFER_SIZE)-pClientSocket->Get_m_lastPos(), 0);

		m_pNetEvent->NERecv(pClientSocket);

		if (len <= 0)
		{
			//cout << "客户端<socket=" << pClientSocket->Get_m_client_sock()
			//	<< ">已退出。\n";
			return -1;
		}

		//检测到数据，视为心跳
		//pClientSocket->resetDTHeart();
		//也可以放到NEOnNetMsg()中，将特定心跳数据头视为心跳

		//将【服务端的】自定义接收缓冲区m_Recv的数据
		//拷贝到【某个客户端对应的】消息缓冲区
		//memcpy(pClientSocket->Get_m_MsgBuf() + pClientSocket->Get_m_lastPos(),m_Recv, len);
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
				OnNetMsg(pClientSocket, pHead);

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


	//响应网络消息
	void OnNetMsg(ClientSocket* pclient_sock, DataHead* pHead)
	{
		//++m_cnt;

		//改用
		m_pNetEvent->NEOnNetMsg(this, pclient_sock, pHead);
#if 0

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
			LogInResult res{};
			pclient_sock->SendData(&res);
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
		default:
		{
			cout << "收到客户端<socket=" << pclient_sock->Get_m_client_sock() << ">未定义消息"
				<< " 数据长度：" << pHead->datalength << endl;

			//发送含有CMD_ERROR的数据包
			DataHead head;
			pclient_sock->SendData(&head);
		}
		}
#endif
	}


	//正式的和在缓冲区中的客户端总数
	//***注***
	//被EasyTcpServer::addClientToCellServer()内调用
	size_t getClientCnt()
	{
		return sock_pclient_pair.size() + vec_client_buffer.size();
	}


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

	//创建具体的发送任务对象CellServerMsgToClientTask
	//并添加发送任务到CellServer类中的CellTaskServer类对象中
	void addSendTask(ClientSocket* pclient_sock, DataHead* pHead)
	{
		//生产任务
		//CellServerMsgToClientTask* pCSMTCT = new CellServerMsgToClientTask(pclient_sock, pHead);
		//***注***
		//此处存在【陷阱】
		//动态分配的具体任务的内存空间到时候需要被释放
		//动态分配的发送消息的内存空间也要被释放

		//频繁地进行内存的申请和释放很容易产生内存碎片
		//也占用了较多的CPU时间消耗

		//需要进行内存管理上的优化

		//m_CellTaskServer.addTask(pCSMTCT);

		m_CellTaskServer.addTask(
			[pclient_sock, pHead]() {
				pclient_sock->SendData(pHead);

				delete pHead;
			});
	}
};
