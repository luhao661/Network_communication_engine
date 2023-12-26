#if 1
#include "EasyTcpServer_1.0.hpp"

int Process(SOCKET client_sock);

int main()
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
	SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//***注***
	// 在网络编程中，AF_INET 用于指定地址族（Address Family）为 IPv4，
	// 而 PF_INET 则用于指定协议族为 IPv4。
	// 在实际使用中，AF_INET 与 PF_INET 可以互换使用，
	// 而且在大多数情况下，它们是相等的。

	//地址信息初始化
	//serv_adr结构体一定要进行初始化，原因见《TCP/IP网络编程读书笔记》
	sockaddr_in serv_adr = {};
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_port = htons(9190);

#ifdef _WIN32
	//可以用常数 INADDR_ANY 来获取服务器端的 IP 地址
	serv_adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif

	//  bind 绑定用于接受客户端连接的服务端网络端口
	// Mac环境下将bind调用限定为使用全局命名空间中的函数
	if (::bind(serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		cout << "bind() ERROR";
	else
		cout << "绑定网络端口成功...\n";

	//  listen 监听网络端口
	if (listen(serv_sock, 5) == SOCKET_ERROR)
		cout << "listen() ERROR";
	else
		cout << "监听网络端口成功...\n";

	// 4 创建timeval结构布局的结构变量timeout以作为select数的第五个实参
	//为了防止陷入无限阻塞的状态，使用 timeout 传递超时信息。
	struct timeval timeout;

	while (1)
	{
		//设置5秒的超时时间
		timeout.tv_sec = 3;
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
		FD_SET(serv_sock, &fdRead);
		FD_SET(serv_sock, &fdWrite);
		FD_SET(serv_sock, &fdExp);

		//创建maxSocket来存储所有客户端加入后的套接字的最大值
		SOCKET maxSocket = serv_sock;
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
			break;
		}
		else if (fd_num == 0)
		{
			cout << "空闲时间处理其他业务" << endl;
		}

		//用FD_ISSET宏筛选在fdRead集合中发生状态变化的句柄
		//检查服务器端套接字（serv_sock）是否处于待读取数据的状态
		//如果是服务器端套接字的变化，表示有客户端连接请求到达，将受理连接请求。
		if (FD_ISSET(serv_sock, &fdRead))
		{
			//从fdRead集合中删除该句柄
			//在后续的 I/O 多路复用操作中不再监听或检查该句柄的状态变化
			//（不写也没关系）
			FD_CLR(serv_sock, &fdRead);

			SOCKET client_sock = INVALID_SOCKET;
			sockaddr_in client_adr = {};
			int client_adr_size = sizeof(client_adr);

			//  accept 等待接受客户端连接
#ifdef _WIN32
			client_sock = accept(serv_sock, (sockaddr*)&client_adr, &client_adr_size);
#else
			client_sock = accept(serv_sock, (sockaddr*)&client_adr, (socklen_t*)&client_adr_size);
#endif
			if (client_sock == INVALID_SOCKET)
				cout << "错误，接受到无效客户端SOCKET";
			else
			{
				//inet_ntoa()将网络字节序的整数型 IP 地址转换为字符串形式
				cout << "新客户端加入：IP = " << inet_ntoa(client_adr.sin_addr)
					<< "  <socket=" << client_sock << ">" << endl;

				//通知该客户端之前的所有客户端有新用户加入
				for (int n = (int)vec_client.size() - 1; n >= 0; --n)
				{
					NewUserJoin newuserjoin;
					send(vec_client[n], (const char*)&newuserjoin, sizeof(NewUserJoin), 0);
				}

				vec_client.push_back(client_sock);
			}
		}

		//***注***
		//如果 select() 返回后没有任何套接字处于待读取状态，fdRead.fd_count 将为 0
		//所以两处的fdRead.fd_count的值，前者会是1  2  或更大，而后者通常只会是0或者1
		//（通常就是指多个客户端不是同时向服务端发送数据）
		// cout << "fdRead.fd_count = " << fdRead.fd_count << endl;

		// //处理已连接的客户端套接字句柄
		// //循环遍历 fdRead 集合中的套接字句柄，然后通过调用 Process() 函数来处理这些句柄
		// for (int n = 0; n < fdRead.fd_count; ++n)
		// {
		// 	//以下写法为什么在处理第二个客户端时出现无法捕获状态变化的问题
		// 	//if(FD_ISSET(vec_client[n], &fdRead))  {...}
		// 	//因为新的已连接的客户端套接字句柄加入是在容器的尾部，而且这样写无法确定
		// 	//是哪个客户端与服务端进行了数据传输

		// 	if (Process(fdRead.fd_array[n]) == -1)
		// 	{
		// 		//如果某个客户端出现问题或断开连接，就将其从已连接客户端的列表中删除。
		// 		auto it = find(vec_client.begin(), vec_client.end(), fdRead.fd_array[n]);
		// 		if (it != vec_client.end())
		// 			vec_client.erase(it);
		// 	}

		// }

		for (int n = (int)vec_client.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(vec_client[n], &fdRead))
			{
				if (Process(vec_client[n]) == -1)
				{
					auto it = vec_client.begin() + n;
					if (it != vec_client.end())
						vec_client.erase(it);
				}
			}
		}

	}

#ifdef _WIN32

	//关闭全部的客户端套接字
	for (int n = vec_client.size() - 1; n >= 0; --n)
	{
		closesocket(vec_client[n]);
	}

	//  关闭套节字closesocket
	closesocket(serv_sock);

	//注销
	WSACleanup();
#else

	//关闭全部的客户端套接字
	for (int n = (int)vec_client.size() - 1; n >= 0; --n)
	{
		close(vec_client[n]);
	}

	//  关闭套节字closesocket
	close(serv_sock);
#endif

	cout << "服务器端已退出，任务结束。\n";
	cin.get();

	return 0;
}

int Process(SOCKET client_sock)
{
	DataHead dh = {};

	//  接收客户端数据
	int len = (int)recv(client_sock, (char*)&dh, sizeof(DataHead), 0);

	if (len <= 0)
	{
		cout << "客户端<socket=" << client_sock << ">已退出。\n";
		return -1;
	}

	//  处理请求
	switch (dh.cmd)
	{
	case CMD_LOGIN:
	{
		LogIn login{};
		//***注***
		//错误写法1：
		//recv(client_sock, (char*)&login, sizeof(LogIn), 0);
		//原因：服务端首先接收一个DataHead类型的数据，而剩下的数据仅仅只还有
		//两个字符数组所占的空间长度，但此处接收一个LogIn类型的数据的话，长度为
		//DataHead类型的数据长度加上两个字符数组所占的空间长度的总和
		//错误写法2：
		//recv(client_sock, (char*)&login, sizeof(LogIn)-sizeof(DataHead), 0);
		//原因：LogIn对象也要进行【数据偏移】，才能对应上传来的数据
		recv(client_sock, (char*)&login + sizeof(DataHead), sizeof(LogIn) - sizeof(DataHead), 0);

		//忽略判断用户密码是否正确的过程

		cout << "收到客户端<socket=" << client_sock << ">命令：CMD_LOGIN"
			<< " 数据长度：" << login.datalength << endl;
		cout << "用户名：" << login.username << "登入" << endl;

		// 发送报文
		LogInResult res{};
		send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
	}
	break;

	case CMD_LOGOUT:
	{
		LogOut logout{};
		recv(client_sock, (char*)&logout + sizeof(DataHead), sizeof(LogOut) - sizeof(DataHead), 0);

		cout << "收到客户端<socket=" << client_sock << ">命令：CMD_LOGOUT"
			<< " 数据长度：" << logout.datalength << endl;
		cout << "用户名：" << logout.username << "登出" << endl;

		LogOutResult res{};
		send(client_sock, (const char*)&res, sizeof(LogOutResult), 0);
	}
	break;

	default:
	{
		cout << "Error!" << endl;
	}
	}

	return 0;
}
#endif