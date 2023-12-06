#include <iostream>

//���windows.h��winsock2.h�º궨���ͻ
#define WIN32_LEAN_AND_MEAN
//ʹinet_ntoa()����
#define _WINSOCK_DEPRECATED_NO_WARNINGS

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
	WORD ver = MAKEWORD(2, 2);
	//����Windows Sockets API����
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1 ����һ��socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock == INVALID_SOCKET)
		cout << "����socketʧ��\n";
	else
		cout << "����socket�ɹ�...\n";
	//***ע***
	// ���������У�AF_INET ����ָ����ַ�壨Address Family��Ϊ IPv4��
	// �� PF_INET ������ָ��Э����Ϊ IPv4��
	// ��ʵ��ʹ���У�AF_INET �� PF_INET ���Ի���ʹ�ã�
	// �����ڴ��������£���������ȵġ�

	//һ��Ҫ���г�ʼ����ԭ�����TCP/IP�����̶���ʼǡ�
	sockaddr_in sin = {};
	sin.sin_family = AF_INET;
	sin.sin_port = htons(9190);
	//sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//127.0.0.1��IPv4��ַ�ռ��е�һ�����Ᵽ����ַ��
	//Ҳ��Ϊ�ػ���ַ�򱾵ػػ���ַ��
	//��ͨ�����������������ϵĻ��ؽӿڣ�
	//�����ڼ�����ڲ���������ͨ�źͲ������繦�ܡ�
	//��������������ӵ�127.0.0.1ʱ����ʵ�������ڳ������Լ�������ӿڽ���ͨ�š�
	//��дΪ
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");


	// 2 ���ӷ����� connect
	if (connect(sock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
		cout << "connect() ERROR\n";
	else
		cout << "���ӳɹ�...\n";

	// 3 ���շ�������Ϣ
	char recv_msg[256] = {'\0'};
	int len=recv(sock,recv_msg,255,0);
	if (len > 0)
		cout << recv_msg<<endl;
	else
		cout << "�����ݣ�\n";

	//��ֹ��EasyTcpClient.exe��һ������
	getchar();

	// 4 �ر��׽���closesocket
	closesocket(sock);

	//ע��
	WSACleanup();

	return 0;
}