#if 1
#include <iostream>

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

#include <vector>

using namespace std;

vector<SOCKET> vec_client;

enum
{
	CMD_LOGIN, CMD_LOGIN_RESULT,
	CMD_LOGOUT, CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//***注***
//不再将包头和包体分开用结构体声明
//而是通过继承，只传一次结构体（或类对象）就同时得到包头与包体

struct DataHead
{
	short datalength;
	short cmd;
};

struct LogIn :public DataHead
{
	LogIn()
	{
		datalength = sizeof(LogIn);
		cmd = CMD_LOGIN;
	}

	char username[20];
	char password[32];
};

struct LogInResult :public DataHead
{
	LogInResult()
	{
		datalength = sizeof(LogInResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}

	int result;
};

struct LogOut :public DataHead
{
	LogOut()
	{
		datalength = sizeof(LogOut);
		cmd = CMD_LOGOUT;
	}

	char username[20];
};

struct LogOutResult :public DataHead
{
	LogOutResult()
	{
		datalength = sizeof(LogOutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}

	int result;
};

struct NewUserJoin :public DataHead
{
	NewUserJoin()
	{
		datalength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}

	int sock;
};

int Process(SOCKET client_sock);

int main()
{
	//初始化

	//创建版本号
	WORD ver = MAKEWORD(2, 2);
	//创建Windows Sockets API数据
	WSADATA dat;
	WSAStartup(ver, &dat);

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
	//可以用常数 INADDR_ANY 来获取服务器端的 IP 地址
	serv_adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	//  bind 绑定用于接受客户端连接的服务端网络端口
	if (bind(serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
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
		FD_SET(serv_sock,&fdRead);
		FD_SET(serv_sock,&fdWrite);
		FD_SET(serv_sock,&fdExp);

		// 4 将客户端套接字句柄添加到fdRead集合
		// 这样做的目的是为了将这些已连接的客户端套接字加入到 fdRead 集合中进行监视，
		// 以便在调用 select() 函数时，能够监视这些套接字的读取操作。
		// 这意味着如果有任何已连接的客户端发送数据，select() 函数将返回并通知程序，
		// 使得程序可以在套接字可读的情况下进行相应的处理
		for (int n = (int)vec_client.size() - 1; n >=0 ; --n)
		{
			FD_SET(vec_client[n],&fdRead);
		}

		cout << "fdRead.fd_count = " << fdRead.fd_count << endl;


		// 5 使用select函数
		// 对于第一个参数，int nfds，是指fd_set集合中所有套接字句柄的范围，
		// 而不是数量，是所有句柄的最大值+1，在windows中可以写0
		// 要写为socket值【最大的一个socket值再加一】
		int fd_num = select(serv_sock + 1, &fdRead, &fdWrite, &fdExp, &timeout);
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
			FD_CLR(serv_sock,&fdRead);

			SOCKET client_sock = INVALID_SOCKET;
			sockaddr_in client_adr = {};
			int client_adr_size = sizeof(client_adr);

			//  accept 等待接受客户端连接
			client_sock = accept(serv_sock, (sockaddr*)&client_adr, &client_adr_size);
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
		cout << "fdRead.fd_count = " << fdRead.fd_count << endl;

		//处理已连接的客户端套接字句柄
		//循环遍历 fdRead 集合中的套接字句柄，然后通过调用 Process() 函数来处理这些句柄
		for (int n = 0; n < fdRead.fd_count; ++n)
		{
			//以下写法为什么在处理第二个客户端时出现无法捕获状态变化的问题
			//if(FD_ISSET(vec_client[n], &fdRead))  {...}
			//因为新的已连接的客户端套接字句柄加入是在容器的尾部，而且这样写无法确定
			//是哪个客户端与服务端进行了数据传输

			if (Process(fdRead.fd_array[n]) == -1)
			{
				//如果某个客户端出现问题或断开连接，就将其从已连接客户端的列表中删除。
				auto it = find(vec_client.begin(), vec_client.end(), fdRead.fd_array[n]);
				if (it != vec_client.end())
					vec_client.erase(it);
			}
			
		}
	}

	//关闭全部的客户端套接字
	for (int n = vec_client.size() - 1; n >= 0; --n)
	{
		closesocket(vec_client[n]);
	}

	//  关闭套节字closesocket
	closesocket(serv_sock);

	//注销
	WSACleanup();

	cout << "服务器端已退出，任务结束。\n";
	cin.get();

	return 0;
}

int Process(SOCKET client_sock)
{
	DataHead dh = {};

	//  接收客户端数据
	int len = recv(client_sock, (char*)&dh, sizeof(DataHead), 0);

	if (len <= 0)
	{
		cout << "客户端<socket="<<client_sock<<">已退出。\n";
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

			cout << "收到客户端<socket="<<client_sock<<">命令：CMD_LOGIN" 
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
}
//***注***
//select() 函数通常与 I/O 多路复用机制相关联，但它本身【并不是多线程的范畴】
//它是一种在单个线程中实现多路 I/O 的机制。
//select() 是一种允许单个进程或线程监视多个文件描述符（通常是套接字）的状态变化的方式。
// 它能够检查多个文件描述符是否就绪（例如，是否有数据可读、是否可写等），
// 并在它们之间进行切换，从而允许程序同时监听多个 I/O 操作。
//可以将 select() 与多线程一起使用来处理多个并发的 I/O 操作。
//例如，在一个线程中使用 select() 监视多个套接字的状态变化，
//并根据事件的发生情况调用其他线程来处理具体的业务逻辑。

//在给定的代码中，使用了 `select` 函数来等待客户端的连接请求，以及处理已连接的客户端。
// 假设第一个客户端接入后，马上有第二个客户端接入，程序的运行过程可能如下：
//1. 初始化和创建套接字：程序开始时，通过 `socket()` 创建了一个服务端套接字 `serv_sock`。
//2. 绑定和监听：使用 `bind()` 将 `serv_sock` 绑定到指定端口，并使用 `listen()` 开始监听来自客户端的连接请求。
//3. 进入主循环：程序进入 `while(1)` 循环，使用 `select()` 函数等待连接请求和已连接客户端的数据读取。
//4. 第一个客户端连接：当第一个客户端发起连接请求时，`select()` 函数会通知有新的连接请求到达，
// `FD_ISSET(serv_sock, & fdRead)` 返回 true。然后调用 `accept()` 接受该客户端的连接，得到一个新的套接字 `client_sock1`。
//5. 处理第一个客户端：程序将 `client_sock1` 添加到 `vec_client` 中，并继续循环。
//6. 第二个客户端连接：在循环中，第二个客户端也发起连接请求。由于 `select()` 函数一直在监视 `serv_sock` 的读操作，
// 它会检测到有新的连接请求到达，`FD_ISSET(serv_sock, & fdRead)` 再次返回 true。
//7. 接受第二个客户端连接：因为 `serv_sock` 处于可读状态，程序再次调用 `accept()` 函数来接受第二个客户端的连接，
// 得到另一个新的套接字 `client_sock2`。
//8. 处理第二个客户端：类似地，程序将 `client_sock2` 添加到 `vec_client` 中，允许对其进行后续的处理。
//9. 循环继续：程序继续在 `while(1)` 循环中使用 `select()` 函数等待新的连接请求或已连接客户端的数据读取。
// 如果有新的连接请求到达或者有已连接客户端发送数据，`select()` 将返回并处理相应的事件。
//这个过程将继续下去，程序会不断接受新的客户端连接并处理已连接客户端的数据，直到有某种条件（比如错误、断开连接等）
// 导致程序跳出循环。
#endif