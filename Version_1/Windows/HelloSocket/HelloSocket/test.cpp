#include <iostream>

#define WIN32_LEAN_AND_MEAN
//����windows�µ�API
#include <windows.h>
//����windows�µ�socket��API
#include <winsock2.h>

//�޷��������ⲿ���� imp WSAStartup������ main �������˸÷���
//�����Ҫ��Ӿ�̬���ӿ��ļ�
//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib, "library_name")
//#pragma ��һ��������ָ�����������������ض���ָ��������Ϣ
//�ڱ���ʱָʾ�����������ض��Ŀ��ļ���
//������������Ŀ������->������->����->����������->���ws2_32.lib

using namespace std;

int main()
{
//��ʼ��

	//�����汾��
	WORD ver = MAKEWORD(2,2);
	//����Windows Sockets API����
	WSADATA dat;
	WSAStartup(ver,&dat);




//ע��
	WSACleanup();

	return 0;
}