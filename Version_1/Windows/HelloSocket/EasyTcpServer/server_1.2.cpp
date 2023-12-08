#if 1
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

struct DataPackage
{
	const char name[20];
	int age;
};

int main()
{
	//��ʼ��

	//�����汾��
	WORD ver = MAKEWORD(2, 2);
	//����Windows Sockets API����
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1 ����һ��socket
	SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//***ע***
	// ���������У�AF_INET ����ָ����ַ�壨Address Family��Ϊ IPv4��
	// �� PF_INET ������ָ��Э����Ϊ IPv4��
	// ��ʵ��ʹ���У�AF_INET �� PF_INET ���Ի���ʹ�ã�
	// �����ڴ��������£���������ȵġ�

	//��ַ��Ϣ��ʼ��
	//serv_adr�ṹ��һ��Ҫ���г�ʼ����ԭ�����TCP/IP�����̶���ʼǡ�
	sockaddr_in serv_adr = {};
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_port = htons(9190);
	//serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//127.0.0.1��IPv4��ַ�ռ��е�һ�����Ᵽ����ַ��
	//Ҳ��Ϊ�ػ���ַ�򱾵ػػ���ַ��
	//��ͨ�����������������ϵĻ��ؽӿڣ�
	//�����ڼ�����ڲ���������ͨ�źͲ������繦�ܡ�
	//��������������ӵ�127.0.0.1ʱ����ʵ�������ڳ������Լ�������ӿڽ���ͨ�š�
	//��дΪ
	serv_adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);


	// 2 bind �����ڽ��ܿͻ������ӵķ��������˿�
	if (bind(serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		cout << "bind() ERROR";
	else
		cout << "������˿ڳɹ�...\n";

	// 3 listen ��������˿�
	if (listen(serv_sock, 5) == SOCKET_ERROR)
		cout << "listen() ERROR";
	else
		cout << "��������˿ڳɹ�...\n";

	SOCKET client_sock = INVALID_SOCKET;
	sockaddr_in client_adr = {};
	int client_adr_size = sizeof(client_adr);

	// 4 accept �ȴ����ܿͻ�������
	client_sock = accept(serv_sock, (sockaddr*)&client_adr, &client_adr_size);
	if (client_sock == INVALID_SOCKET)
		cout << "���󣬽��ܵ���Ч�ͻ���SOCKET";

	//inet_ntoa()�������ֽ���������� IP ��ַת��Ϊ�ַ�����ʽ
	cout << "�¿ͻ��˼��룺IP = " << inet_ntoa(client_adr.sin_addr) << endl;

	char CmdMsg[128] = {};
	//��ѭ����ʹ��һ�����ͻ��˽���󣬺ͷ������˽��н���ʽͨ��
	while (1)
	{
		// 5 ���տͻ�������
		int len = recv(client_sock, CmdMsg, 128, 0);

		if (len <= 0)
		{
			cout << "�ͻ����˳���\n";
			break;
		}

		// 6 ��������
		if (!strcmp(CmdMsg, "getinfo"))
		{
			DataPackage dp = {"·��",24};
			send(client_sock, (const char*) & dp, sizeof(DataPackage), 0);
		}
		else
		{
			const char* msg = "???\n";
			send(client_sock, msg, strlen(msg) + 1, 0);//���ַ�ҲҪ����
		}
	}

	// 8 �ر��׽���closesocket
	closesocket(serv_sock);

	//ע��
	WSACleanup();

	cout << "�����������˳������������\n";
	cin.get();

	return 0;
}
#endif