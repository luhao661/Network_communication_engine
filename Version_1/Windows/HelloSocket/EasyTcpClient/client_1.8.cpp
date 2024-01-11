#if 1
#include "EasyTcpClient_1.2.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread(void);

int main()
{
	//由于EasyTcpClient类占用内存空间很大
	//所以放到堆中较好
	//EasyTcpClient* client1=new EasyTcpClient ;

	//EasyTcpClient* ClientArray = new EasyTcpClient[FD_SETSIZE-1]();

	const int Num_clients = FD_SETSIZE - 1;
	
	shared_ptr < EasyTcpClient >ClientArray  
		(new EasyTcpClient[Num_clients],default_delete<EasyTcpClient[]>());

	//client1->initSocket();
	//client2.initSocket();

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).initSocket();
	}


#if 1
	const char ip[] = "127.0.0.1";
#elif 0
	//通过cmd查到Linux虚拟机的IP地址为192.168.175.132
	const char ip[] = "192.168.175.132";
#else
	//通过cmd查到Mac虚拟机的IP地址为192.168.175.133
	const char ip[] = "192.168.175.133";
#endif
	//注意虚拟机打开后，其IP地址可能会改变

	//创建两个客户端socket，来连接两个服务器
	//client1->Connect(ip, 9190);
	//client2.Connect(ip,9191);

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).Connect(ip,9190);
	}

	// 3 输入请求命令（利用线程）
	//启动线程
	//***注***
	//Mac端不支持传引用
	//thread t1(cmdThread, &client1);//格式：函数名  参数
	//从 thread 对象分离执行线程，允许执行且独立地持续。
	//一旦该线程退出，则释放任何分配的资源。

	//调用了 detach() 方法，就意味着放弃了对线程的控制能力。
	//此后，主线程和分离的线程之间将相互独立运行，主线程不再等待分离的线程执行完成。
	//主线程不再需要关心该线程的状态或结束时机。
	//如：在分离后，不再能够对 t 进行 t1.join()
	//t1.detach();

	//thread t2(cmdThread, &client2);
	//t2.detach();
	thread t1(cmdThread);//格式：函数名  参数
	t1.detach();

	//若不使用t1.detach();
	//主线程的while循环用g_bRun来作为判断终止的条件
	//在t1线程终止时，主线程也会跳出while循环而终止，
	//主线程比t1线程先终止，会导致程序崩溃

	//使用 detach() 将主线程与创建的线程分离后，
	//主线程结束时不会直接影响到已分离的线程。
	//主线程的结束不会导致已分离线程的提前终止，已分离的线程将继续在后台独立运行。

	LogIn login{};

#ifdef _WIN32
	strncpy_s(login.username, 20, "Luhao", 6);
	strncpy_s(login.password, 32, "123456", 7);
#else
	strncpy(login.username, "Luhao", 6);
	strncpy(login.password, "123456", 7);
#endif


	while (g_bRun /*|| client2.isRun()*/)
	{
		for (int i = 0; i < Num_clients; ++i)
		{
			ClientArray.get()[i].OnRun();
			(ClientArray.get()[i]).SendData(&login);
		}
	}

	//client1->Close();
	//client2.Close();

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).Close();
	}

	//delete client1;

	//防止打开EasyTcpClient.exe后一闪而过
	//getchar();
	cin.get();
	cout << "客户端已退出，任务结束。\n";

	return 0;
}

void cmdThread(void)
{
	char cmdBuf[256]{};

	//cin 和 scanf 都是阻塞式函数，
	// 而main()中的select()非阻塞，这样会造成运行逻辑冲突，所以要使用多线程

	while (1)
	{
		cin.getline(cmdBuf, 256);

		// 处理请求
		if (!strcmp(cmdBuf, "exit"))
		{
			cout << "收到退出命令，退出cmdThread线程\n";
			g_bRun = false;
			break;
		}
		else
			cout << "未识别的命令，请重新输入！\n";
	}
}

/*
//命令线程
void cmdThread(EasyTcpClient* client)
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
			//g_bRun = false;

			client->Close();

			break;
		}
		else if (!strcmp(cmdBuf, "login"))
		{
			//***错误写法***
			//没有与之匹配的构造函数
			//LogIn login{ "Luhao","123456" };
			//因为LogIn现在是派生类了，不再是简单的单独的类，否则可以这么写

			LogIn login{};

#ifdef _WIN32
			strncpy_s(login.username, 20, "Luhao", 6);
			strncpy_s(login.password, 32, "123456", 7);
#else
			strncpy(login.username, "Luhao", 6);
			strncpy(login.password, "123456", 7);
#endif
			// 发送报文
			//***理解此处的与服务器端一样的语句***
			//此处的client_sock，应该是客户端创建的用于连接服务器的套接字。
			//所以，这行代码是在客户端发送消息 cmdBuf 到已连接的服务器。
			//即：在客户端，client_sock 是 指向 服务器的套接字

			//send(client.m_client_sock, (char*)&login, sizeof(LogIn), 0);
			//可以直接写：
			client->SendData(&login);
			//***注***
			//此处利用了向上隐式类型转换
		}
		else if (!strcmp(cmdBuf, "logout"))
		{
			//LogOut logout{ "Luhao" };

			LogOut logout{};

#ifdef _WIN32
			strncpy_s(logout.username, 20, "Luhao", 6);
#else
			strncpy(logout.username, "Luhao", 6);
#endif

			//***理解此处的与服务器端一样的语句***
			//此处的client_sock，应该是客户端创建的用于连接服务器的套接字。
			//所以，这行代码是在客户端发送消息 CmdMsg 到已连接的服务器。
			//即：在客户端，client_sock 是 指向 服务器的套接字

			client->SendData(&logout);
		}
		else
			cout << "未识别的命令，请重新输入！\n";

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
*/
#endif
