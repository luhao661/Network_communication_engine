#if 0
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
	CMD_LOGIN, CMD_LOGIN_RESULT,
	CMD_LOGOUT, CMD_LOGOUT_RESULT,
	CMD_ERROR
};

//***ע***
//���ٽ���ͷ�Ͱ���ֿ��ýṹ������
//����ͨ���̳У�ֻ��һ�νṹ�壨������󣩾�ͬʱ�õ���ͷ�����

#if 0
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
#endif

struct DataHead
{
	short datalength;
	short cmd;
};

struct LogIn:public DataHead
{
	LogIn()
	{
		datalength = sizeof(LogIn);
		cmd = CMD_LOGIN;
	}

	char username[20];
	char password[32];
};

struct LogInResult :public DataHead
{
	LogInResult()
	{
		datalength = sizeof(LogInResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}

	int result;
};

struct LogOut :public DataHead
{
	LogOut()
	{
		datalength = sizeof(LogOut);
		cmd = CMD_LOGOUT;
	}

	char username[20];
};

struct LogOutResult :public DataHead
{
	LogOutResult()
	{
		datalength = sizeof(LogOutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}

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
		DataHead dh = {};

		// 5 ���տͻ�������
		int len = recv(client_sock, (char*)&dh, sizeof(DataHead), 0);

		if (len <= 0)
		{
			cout << "�ͻ����˳���\n";
			break;
		}

		// 6 ��������
		switch (dh.cmd)
		{
		case CMD_LOGIN:
		{
			LogIn login{};
			//***ע***
			//����д��1��
			//recv(client_sock, (char*)&login, sizeof(LogIn), 0);
			//ԭ�򣺷�������Ƚ���һ��DataHead���͵����ݣ���ʣ�µ����ݽ���ֻ����
			//�����ַ�������ռ�Ŀռ䳤�ȣ����˴�����һ��LogIn���͵����ݵĻ�������Ϊ
			//DataHead���͵����ݳ��ȼ��������ַ�������ռ�Ŀռ䳤�ȵ��ܺ�
			//����д��2��
			//recv(client_sock, (char*)&login, sizeof(LogIn)-sizeof(DataHead), 0);
			//ԭ��LogIn����ҲҪ���С�����ƫ�ơ������ܶ�Ӧ�ϴ���������
			recv(client_sock, (char*)&login+sizeof(DataHead), sizeof(LogIn) - sizeof(DataHead), 0);

			//�����ж��û������Ƿ���ȷ�Ĺ���

			cout << "�յ����CMD_LOGIN" << " ���ݳ��ȣ�"<<login.datalength << endl;
			cout << "�û�����" << login.username<< "����" << endl;

			//7 ���ͱ���
			LogInResult res{};
			send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
		}
		break;

		case CMD_LOGOUT:
		{
			LogOut logout{};
			recv(client_sock, (char*)&logout + sizeof(DataHead), sizeof(LogOut)-sizeof(DataHead), 0);

			cout << "�յ����CMD_LOGOUT" << " ���ݳ��ȣ�" << logout.datalength << endl;
			cout << "�û�����" << logout.username <<"�ǳ�" << endl;

			LogOutResult res{};
			send(client_sock, (const char*)&res, sizeof(LogOutResult), 0);
		}
		break;

		default:
		{
			cout << "Error!" << endl;
		}
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