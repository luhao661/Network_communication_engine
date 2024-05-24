#pragma once//防止头文件内容被多次包含

#ifdef _WIN32
//解决windows.h和winsock2.h下宏定义冲突
#define WIN32_LEAN_AND_MEAN
//使inet_ntoa()可用
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define FD_SETSIZE 10006
//理解：
//最大支持的客户端连接数量为CellServer线程数*(FD_SETSIZE-1)

//包含windows下的API
#include <windows.h>
//包含windows下的socket的API
#include <winsock2.h>

//无法解析的外部符号 imp WSAStartup，函数 main 中引用了该符号
//解决：要添加静态链接库文件
#pragma comment(lib,"ws2_32.lib") 
//#pragma comment(lib, "library_name")
//#pragma 是一个编译器指令，用于向编译器传达特定的指令或控制信息
//在编译时指示链接器引入特定的库文件。
//方法二：在项目的属性->链接器->输入->附加依赖项->添加ws2_32.lib
#else
//UNIX 下标准C语言头文件
#include <unistd.h>
#include <arpa/inet.h> 
#include <string.h>

//将SOCKET类型宏定义为int类型
//因为Linux下套接字文件描述符就是int类型
#define SOCKET int
//继续定义Linux下没有的宏定义
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

//自定义
#include "MessageHeader.hpp"//改为100字节的数据包
#include "Timestamp.hpp"
#include"CellTask.hpp"
#include "CellLog.hpp"

#include <iostream> 
#include <stdio.h>

#ifndef RECV_BUFFER_SIZE

#define RECV_BUFFER_SIZE 1024*10
#define SEND_BUFFER_SIZE 1024*100

#endif

