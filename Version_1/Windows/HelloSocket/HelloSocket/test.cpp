#include <iostream>

#define WIN32_LEAN_AND_MEAN
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
	WORD ver = MAKEWORD(2,2);
	//创建Windows Sockets API数据
	WSADATA dat;
	WSAStartup(ver,&dat);




//注销
	WSACleanup();

	return 0;
}