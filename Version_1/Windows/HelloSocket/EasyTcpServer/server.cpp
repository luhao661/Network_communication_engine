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
	SOCKET sock= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//***注***
	// 在网络编程中，AF_INET 用于指定地址族（Address Family）为 IPv4，
	// 而 PF_INET 则用于指定协议族为 IPv4。
	// 在实际使用中，AF_INET 与 PF_INET 可以互换使用，
	// 而且在大多数情况下，它们是相等的。
	
	//地址信息初始化
	//sin结构体一定要进行初始化，原因见《TCP/IP网络编程读书笔记》
	sockaddr_in sin = {};
	sin.sin_family = AF_INET;
	sin.sin_port = htons(9190);
	//sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//127.0.0.1是IPv4地址空间中的一个特殊保留地址，
	//也称为回环地址或本地回环地址。
	//它通常被用作本地主机上的环回接口，
	//用于在计算机内部进行自我通信和测试网络功能。
	//当计算机尝试连接到127.0.0.1时，它实际上是在尝试与自己的网络接口进行通信。
	//或写为
	sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);


	// 2 bind 绑定用于接受客户端连接的网络端口
	if (bind(sock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
		cout << "bind() ERROR";
	else
		cout << "绑定网络端口成功...\n";

	// 3 listen 监听网络端口
	if (listen(sock, 5) == SOCKET_ERROR)
		cout << "listen() ERROR";
	else
		cout << "监听网络端口成功...\n";

	SOCKET ClientSocket=INVALID_SOCKET;
	sockaddr_in cin = {};
	int cin_addr_size = sizeof(cin);

	//用循环，来支持多个同样IP地址的客户端接入
	while (1)
	{
		// 4 accept 等待接受客户端连接
		ClientSocket = accept(sock, (sockaddr*)&cin, &cin_addr_size);
		if (ClientSocket == INVALID_SOCKET)
			cout << "错误，接受到无效客户端SOCKET";

		//inet_ntoa()将网络字节序的整数型 IP 地址转换为字符串形式
		cout << "新客户端加入：IP = " << inet_ntoa(cin.sin_addr) << endl;

		const char msg[] = "Hello, I am server.";

		// 5 send 向客户端发送一条数据
		//***注***
		// 也要发送空字符
		send(ClientSocket, msg, strlen(msg) + 1, 0);	
	}

	// 6 关闭套节字closesocket
	closesocket(sock);
	
	//注销
	WSACleanup();

	return 0;
}