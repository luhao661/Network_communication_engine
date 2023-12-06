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
	SOCKET sock= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//***ע***
	// ���������У�AF_INET ����ָ����ַ�壨Address Family��Ϊ IPv4��
	// �� PF_INET ������ָ��Э����Ϊ IPv4��
	// ��ʵ��ʹ���У�AF_INET �� PF_INET ���Ի���ʹ�ã�
	// �����ڴ��������£���������ȵġ�
	
	//��ַ��Ϣ��ʼ��
	//sin�ṹ��һ��Ҫ���г�ʼ����ԭ�����TCP/IP�����̶���ʼǡ�
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
	sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);


	// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
	if (bind(sock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
		cout << "bind() ERROR";
	else
		cout << "������˿ڳɹ�...\n";

	// 3 listen ��������˿�
	if (listen(sock, 5) == SOCKET_ERROR)
		cout << "listen() ERROR";
	else
		cout << "��������˿ڳɹ�...\n";

	SOCKET ClientSocket=INVALID_SOCKET;
	sockaddr_in cin = {};
	int cin_addr_size = sizeof(cin);

	//��ѭ������֧�ֶ��ͬ��IP��ַ�Ŀͻ��˽���
	while (1)
	{
		// 4 accept �ȴ����ܿͻ�������
		ClientSocket = accept(sock, (sockaddr*)&cin, &cin_addr_size);
		if (ClientSocket == INVALID_SOCKET)
			cout << "���󣬽��ܵ���Ч�ͻ���SOCKET";

		//inet_ntoa()�������ֽ���������� IP ��ַת��Ϊ�ַ�����ʽ
		cout << "�¿ͻ��˼��룺IP = " << inet_ntoa(cin.sin_addr) << endl;

		const char msg[] = "Hello, I am server.";

		// 5 send ��ͻ��˷���һ������
		//***ע***
		// ҲҪ���Ϳ��ַ�
		send(ClientSocket, msg, strlen(msg) + 1, 0);	
	}

	// 6 �ر��׽���closesocket
	closesocket(sock);
	
	//ע��
	WSACleanup();

	return 0;
}