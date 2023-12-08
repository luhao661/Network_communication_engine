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

using namespace std;

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
	serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// 2 连接服务器 connect
	if (connect(client_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		cout << "connect() ERROR\n";
	else
		cout << "连接成功...\n";
	//***注***
	//客户端套接字在调用connect()时绑定（也叫分配）了客户端地址

	// 3 输入请求命令
	char CmdMsg[128] = {};
	while (1)
	{
		cout << "请输入命令：";
		cin.getline(CmdMsg, 128);

		// 4 处理请求
		if (!strcmp(CmdMsg, "exit"))
		{
			cout << "收到退出命令\n";
			break;
		}
		else
		{
			//***理解此处的与服务器端一样的语句***
			//此处的client_sock，应该是客户端创建的用于连接服务器的套接字。
			//所以，这行代码是在客户端发送消息 CmdMsg 到已连接的服务器。
			//即：在客户端，client_sock 是 指向 服务器的套接字
			send(client_sock, CmdMsg, strlen(CmdMsg) + 1, 0);
		}

		// 6 接收服务器信息
		char recv_msg[256] = { '\0' };
		int len = recv(client_sock, recv_msg, 255, 0);
		if (len > 0)
			cout << recv_msg;
		else
			cout << "无数据！\n";
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
#endif