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

enum
{
	CMD_LOGIN, CMD_LOGOUT, CMD_ERROR
};


//�������ݱ��ĵİ�ͷ
struct DataHead
{
	short datalength;
	short cmd;
};

//����
struct LogIn
{
	char username[20];
	char password[32];
};

struct LogOut
{
	char username[20];
};

struct LogInResult
{
	int result;
};

struct LogOutResult
{
	int result;
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
	SOCKET client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (client_sock == INVALID_SOCKET)
		cout << "����socketʧ��\n";
	else
		cout << "����socket�ɹ�...\n";
	//***ע***
	// ���������У�AF_INET ����ָ����ַ�壨Address Family��Ϊ IPv4��
	// �� PF_INET ������ָ��Э����Ϊ IPv4��
	// ��ʵ��ʹ���У�AF_INET �� PF_INET ���Ի���ʹ�ã�
	// �����ڴ��������£���������ȵġ�

	//***ע***
	//��ʼ��ֵΪ��Ŀ����������׽��֡��� IP �Ͷ˿���Ϣ��
	//һ��Ҫ���г�ʼ����ԭ�����TCP/IP�����̶���ʼǡ�
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
	serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// 2 ���ӷ����� connect
	if (connect(client_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		cout << "connect() ERROR\n";
	else
		cout << "���ӳɹ�...\n";
	//***ע***
	//�ͻ����׽����ڵ���connect()ʱ�󶨣�Ҳ�з��䣩�˿ͻ��˵�ַ

	// 3 ������������
	char CmdMsg[128] = {};
	while (1)
	{
		cout << "���������";
		cin.getline(CmdMsg, 128);

		// 4 ��������
		if (!strcmp(CmdMsg, "exit"))
		{
			cout << "�յ��˳�����\n";
			break;
		}
		else if (!strcmp(CmdMsg, "login"))
		{
			LogIn login{ "Luhao","123456" };

			//***ע***
			//����д����
			//LogIn login;
			//login.username = "Luhao";//Ӧ����strcpy(login.username,"Luhao");			
			//login.password = "123456";

			DataHead dh{ sizeof(login),CMD_LOGIN };

			//5 ���ͱ���
			//***���˴������������һ�������***
			//�˴���client_sock��Ӧ���ǿͻ��˴������������ӷ��������׽��֡�
			//���ԣ����д������ڿͻ��˷�����Ϣ CmdMsg �������ӵķ�������
			//�����ڿͻ��ˣ�client_sock �� ָ�� ���������׽���
			send(client_sock, (char*)&dh, sizeof(DataHead), 0);
			send(client_sock, (char*)&login, sizeof(LogIn), 0);

			//6 ���շ��������ص�����
			DataHead recv_Head{};
			LogInResult login_result{};

			recv(client_sock, (char*)&recv_Head, sizeof(DataHead), 0);
			recv(client_sock, (char*)&login_result, sizeof(LogInResult), 0);

			cout << "login_result��" << login_result.result << endl;
		}
		else if (!strcmp(CmdMsg, "logout"))
		{
			LogOut logout{ "Luhao" };

			DataHead dh{ sizeof(LogOut),CMD_LOGOUT };

			//***���˴������������һ�������***
			//�˴���client_sock��Ӧ���ǿͻ��˴������������ӷ��������׽��֡�
			//���ԣ����д������ڿͻ��˷�����Ϣ CmdMsg �������ӵķ�������
			//�����ڿͻ��ˣ�client_sock �� ָ�� ���������׽���
			send(client_sock, (char*)&dh, sizeof(DataHead), 0);
			send(client_sock, (char*)&logout, sizeof(LogOut), 0);

			//���շ��������ص�����
			DataHead recv_Head{};
			LogOutResult logout_result{};

			recv(client_sock, (char*)&recv_Head, sizeof(DataHead), 0);
			recv(client_sock, (char*)&logout_result, sizeof(LogOutResult), 0);

			cout << "logout_result��" <<logout_result.result << endl;
		}
		else
			cout << "δʶ���������������룡\n";
	}

	// 7 �ر��׽���closesocket
	closesocket(client_sock);

	//ע��
	WSACleanup();

	//��ֹ��EasyTcpClient.exe��һ������
	//getchar();
	cin.get();
	cout << "�ͻ������˳������������\n";

	return 0;
}
#endif