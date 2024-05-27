#pragma once 

#include "Cell.hpp"
#include <csignal>

class CellNetWork
{
private:
	
	CellNetWork()
	{
#ifdef _WIN32
		//初始化

		//创建版本号 启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		//创建Windows Sockets API数据
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
 
#ifndef _WIN32
		//if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		//	return (1);
		//忽略异常信号，默认情况会导致进程终止
		signal(SIGPIPE, SIG_IGN);
#endif
	}

	~CellNetWork()
	{
		//注销
#ifdef _WIN32
		WSACleanup();
#endif
	}

public:

	//不会用到该类对象中的数据内容，那就不用返回对象单例了
	//static CellNetWork& Instance()

	static void Init()
	{
		/*
		多次调用 Init 函数，CNW 对象只会在第一次调用时被创建。
		接下来的调用中，程序不会再重新创建 CNW 对象，
		而是直接使用第一次调用时创建的那个对象
		*/
		static CellNetWork CNW;
		return;
	}
};
