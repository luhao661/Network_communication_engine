#if 0
#include <iostream>

//解决windows.h和winsock2.h下宏定义冲突
#define WIN32_LEAN_AND_MEAN
//使inet_ntoa()可用
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//包含windows下的API
#include <windows.h>
//包含windows下的socket的API
#include <winsock2.h>

#include <thread>
//无法解析的外部符号 imp WSAStartup，函数 main 中引用了该符号
//解决：要添加静态链接库文件
//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib, "library_name")
//#pragma 是一个编译器指令，用于向编译器传达特定的指令或控制信息
//在编译时指示链接器引入特定的库文件。
//方法二：在项目的属性->链接器->输入->附加依赖项->添加ws2_32.lib

using namespace std;

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

//创建一个全局的变量，提示线程是否正在运行
bool g_bRun = true;
void cmdThread(SOCKET client_sock);
int Process(SOCKET socket);

int main()
{
	//初始化

	//创建版本号
	WORD ver = MAKEWORD(2, 2);
	//创建Windows Sockets API数据
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1 建立一个socket
	SOCKET client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (client_sock == INVALID_SOCKET)
		cout << "建立socket失败\n";
	else
		cout << "建立socket成功...\n";
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
	serv_adr.sin_port = htons(9190);
	//serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//127.0.0.1是IPv4地址空间中的一个特殊保留地址，
	//也称为回环地址或本地回环地址。
	//它通常被用作本地主机上的环回接口，
	//用于在计算机内部进行自我通信和测试网络功能。
	//当计算机尝试连接到127.0.0.1时，它实际上是在尝试与自己的网络接口进行通信。
	//或写为
#if 1
	serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#elif 0
	//通过cmd查到Linux虚拟机的IP地址为192.168.175.1
	//因此连接Linux服务端就需要修改为
	serv_adr.sin_addr.s_addr = inet_addr("192.168.175.1");
#else
	//通过cmd查到Mac虚拟机的IP地址为192.168.175.133
	serv_adr.sin_addr.s_addr = inet_addr("192.168.175.133");
#endif

	// 2 连接服务器 connect
	if (connect(client_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		cout << "connect() ERROR\n";
	else
		cout << "连接成功...\n";
	//***注***
	//客户端套接字在调用connect()时绑定（也叫分配）了客户端地址

	// 3 输入请求命令（利用线程）
	//启动线程
	thread t1(cmdThread,client_sock);//格式：函数名  参数

	//从 thread 对象分离执行线程，允许执行且独立地持续。
	//一旦该线程退出，则释放任何分配的资源。
	
	//调用了 detach() 方法，就意味着放弃了对线程的控制能力。
	//此后，主线程和分离的线程之间将相互独立运行，主线程不再等待分离的线程执行完成。
	//主线程不再需要关心该线程的状态或结束时机。
	//如：在分离后，不再能够对 t 进行 t1.join()
	t1.detach();

	//若不使用t1.detach();
	//主线程的while循环用g_bRun来作为判断终止的条件
	//在t1线程终止时，主线程也会跳出while循环而终止，
	//主线程比t1线程先终止，会导致程序崩溃

	//使用 detach() 将主线程与创建的线程分离后，
	//主线程结束时不会直接影响到已分离的线程。
	//主线程的结束不会导致已分离线程的提前终止，已分离的线程将继续在后台独立运行。

	while (g_bRun)
	{
		fd_set fdRead;

		FD_ZERO(&fdRead);

		FD_SET(client_sock, &fdRead);

		struct timeval timeout = { 3,0 };

		int fd_num = select(client_sock + 1, &fdRead, 0, 0, &timeout);
		if (fd_num == -1)
		{
			cout << "select任务结束_1" << endl;
			break;
		}
		else if (fd_num == 0)
		{
			//cout << "空闲时间处理其他业务" << endl;
		}

		if (FD_ISSET(client_sock, &fdRead))
		{
			FD_CLR(client_sock, &fdRead);

			if (-1 == Process(client_sock))
			{
				cout << "select任务结束_2" << endl;
			}
		}
	}

	// 7 关闭套节字closesocket
	closesocket(client_sock);

	//注销
	WSACleanup();

	//防止打开EasyTcpClient.exe后一闪而过
	//getchar();
	cin.get();
	cout << "客户端已退出，任务结束。\n";

	return 0;
}

//命令线程
void cmdThread(SOCKET client_sock)
{
	char cmdBuf[256]{};

	//cin 和 scanf 都是阻塞式函数，
	// 而main()中的select()非阻塞，这样会造成运行逻辑冲突，所以要使用多线程

	while (1)
	{
		//为了让主线程打印完：“收到服务端消息： 。。。”
		this_thread::sleep_for(std::chrono::seconds(1));

		cout << "请输入命令：";
		cin.getline(cmdBuf, 256);

		// 处理请求
		if (!strcmp(cmdBuf, "exit"))
		{
			cout << "收到退出命令，退出cmdThread线程\n";
			g_bRun = false;
			break;
		}
		else if (!strcmp(cmdBuf, "login"))
		{
			//***错误写法***
			//没有与之匹配的构造函数
			//LogIn login{ "Luhao","123456" }; 
			//因为LogIn现在是派生类了，不再是简单的单独的类，否则可以这么写

			LogIn login{};
			strncpy_s(login.username, 20, "Luhao", 6);
			strncpy_s(login.password, 32, "123456", 7);

			// 发送报文
			//***理解此处的与服务器端一样的语句***
			//此处的client_sock，应该是客户端创建的用于连接服务器的套接字。
			//所以，这行代码是在客户端发送消息 cmdBuf 到已连接的服务器。
			//即：在客户端，client_sock 是 指向 服务器的套接字
			send(client_sock, (char*)&login, sizeof(LogIn), 0);

			//// 接收服务器返回的数据
			//LogInResult login_result{};

			//recv(client_sock, (char*)&login_result, sizeof(LogInResult), 0);

			//cout << "login_result：" << login_result.result << endl;
		}
		else if (!strcmp(cmdBuf, "logout"))
		{
			//LogOut logout{ "Luhao" };

			LogOut logout{};
			strncpy_s(logout.username, 20, "Luhao", 6);


			//***理解此处的与服务器端一样的语句***
			//此处的client_sock，应该是客户端创建的用于连接服务器的套接字。
			//所以，这行代码是在客户端发送消息 CmdMsg 到已连接的服务器。
			//即：在客户端，client_sock 是 指向 服务器的套接字
			send(client_sock, (char*)&logout, sizeof(LogOut), 0);

			////接收服务器返回的数据
			//LogOutResult logout_result{};

			//recv(client_sock, (char*)&logout_result, sizeof(LogOutResult), 0);

			//cout << "logout_result：" << logout_result.result << endl;
		}
		else
			cout << "未识别的命令，请重新输入！\n";

	}
}

int Process(SOCKET socket)
{
	char RecvBuff[4096] = {};

	//先接收包头
	int len = recv(socket, (char*)&RecvBuff, sizeof(DataHead), 0);
	if (len <= 0)
	{
		cout << "与服务器断开连接，任务结束" << endl;
		return -1;
	}

	//DataHead* pHead = (DataHead*)RecvBuff;
	DataHead* pHead = reinterpret_cast<DataHead*>(RecvBuff);

	switch (pHead->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(socket, (char*)RecvBuff + sizeof(DataHead), 
				pHead->datalength - sizeof(DataHead), 0);

			LogInResult* loginresult = (LogInResult*)RecvBuff;

			cout << "收到服务端消息：CMD_LOGIN_RESULT"
				<< " 数据长度：" << loginresult->datalength << endl;
		}
		break;

		case CMD_LOGOUT_RESULT:
		{
			recv(socket, (char*)RecvBuff + sizeof(DataHead),
				pHead->datalength - sizeof(DataHead), 0);

			LogOutResult* logoutresult = (LogOutResult*)RecvBuff;

			cout << "收到服务端消息：CMD_LOGOUT_RESULT"
				<< " 数据长度：" << logoutresult->datalength << endl;
		}
		break;

		case CMD_NEW_USER_JOIN:
		{
			recv(socket, (char*)RecvBuff + sizeof(DataHead),
				pHead->datalength - sizeof(DataHead), 0);

			NewUserJoin* newuserjoin = (NewUserJoin*)RecvBuff;

			cout << "\n收到服务端消息：CMD_NEW_USER_JOIN"
				<< " 数据长度：" << newuserjoin->datalength << endl;
		}
		break;
	}

}

// 为什么使用多线程？
// 多线程允许程序同时执行多个线程，每个线程都可以独立运行，
// 从而可以在某些线程因为阻塞而暂停时，让其他线程继续执行其他任务，
// 提高了程序的并发性和响应性。
// 在一个多线程的网络应用中，一个线程可能负责接收网络请求（可能会因等待网络数据而阻塞），
// 而另一个线程则负责处理这些接收到的数据。
// 这样，在一个线程等待网络数据的同时，其他线程可以继续执行而不会受到阻塞的影响，
// 从而提高了整个应用程序的响应速度
#endif