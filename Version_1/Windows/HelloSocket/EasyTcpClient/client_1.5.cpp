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

#include <thread>
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
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//***ע***
//���ٽ���ͷ�Ͱ���ֿ��ýṹ������
//����ͨ���̳У�ֻ��һ�νṹ�壨������󣩾�ͬʱ�õ���ͷ�����

struct DataHead
{
	short datalength;
	short cmd;
};

struct LogIn :public DataHead
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

struct NewUserJoin :public DataHead
{
	NewUserJoin()
	{
		datalength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}

	int sock;
};

//����һ��ȫ�ֵı�������ʾ�߳��Ƿ���������
bool g_bRun = true;
void cmdThread(SOCKET client_sock);
int Process(SOCKET socket);

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
#if 1
	serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#elif 0
	//ͨ��cmd�鵽Linux�������IP��ַΪ192.168.175.1
	//�������Linux����˾���Ҫ�޸�Ϊ
	serv_adr.sin_addr.s_addr = inet_addr("192.168.175.1");
#else
	//ͨ��cmd�鵽Mac�������IP��ַΪ192.168.175.133
	serv_adr.sin_addr.s_addr = inet_addr("192.168.175.133");
#endif

	// 2 ���ӷ����� connect
	if (connect(client_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		cout << "connect() ERROR\n";
	else
		cout << "���ӳɹ�...\n";
	//***ע***
	//�ͻ����׽����ڵ���connect()ʱ�󶨣�Ҳ�з��䣩�˿ͻ��˵�ַ

	// 3 ����������������̣߳�
	//�����߳�
	thread t1(cmdThread,client_sock);//��ʽ��������  ����

	//�� thread �������ִ���̣߳�����ִ���Ҷ����س�����
	//һ�����߳��˳������ͷ��κη������Դ��
	
	//������ detach() ����������ζ�ŷ����˶��̵߳Ŀ���������
	//�˺����̺߳ͷ�����߳�֮�佫�໥�������У����̲߳��ٵȴ�������߳�ִ����ɡ�
	//���̲߳�����Ҫ���ĸ��̵߳�״̬�����ʱ����
	//�磺�ڷ���󣬲����ܹ��� t ���� t1.join()
	t1.detach();

	//����ʹ��t1.detach();
	//���̵߳�whileѭ����g_bRun����Ϊ�ж���ֹ������
	//��t1�߳���ֹʱ�����߳�Ҳ������whileѭ������ֹ��
	//���̱߳�t1�߳�����ֹ���ᵼ�³������

	//ʹ�� detach() �����߳��봴�����̷߳����
	//���߳̽���ʱ����ֱ��Ӱ�쵽�ѷ�����̡߳�
	//���̵߳Ľ������ᵼ���ѷ����̵߳���ǰ��ֹ���ѷ�����߳̽������ں�̨�������С�

	while (g_bRun)
	{
		fd_set fdRead;

		FD_ZERO(&fdRead);

		FD_SET(client_sock, &fdRead);

		struct timeval timeout = { 3,0 };

		int fd_num = select(client_sock + 1, &fdRead, 0, 0, &timeout);
		if (fd_num == -1)
		{
			cout << "select�������_1" << endl;
			break;
		}
		else if (fd_num == 0)
		{
			//cout << "����ʱ�䴦������ҵ��" << endl;
		}

		if (FD_ISSET(client_sock, &fdRead))
		{
			FD_CLR(client_sock, &fdRead);

			if (-1 == Process(client_sock))
			{
				cout << "select�������_2" << endl;
			}
		}
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

//�����߳�
void cmdThread(SOCKET client_sock)
{
	char cmdBuf[256]{};

	//cin �� scanf ��������ʽ������
	// ��main()�е�select()����������������������߼���ͻ������Ҫʹ�ö��߳�

	while (1)
	{
		//Ϊ�������̴߳�ӡ�꣺���յ��������Ϣ�� ��������
		this_thread::sleep_for(std::chrono::seconds(1));

		cout << "���������";
		cin.getline(cmdBuf, 256);

		// ��������
		if (!strcmp(cmdBuf, "exit"))
		{
			cout << "�յ��˳�����˳�cmdThread�߳�\n";
			g_bRun = false;
			break;
		}
		else if (!strcmp(cmdBuf, "login"))
		{
			//***����д��***
			//û����֮ƥ��Ĺ��캯��
			//LogIn login{ "Luhao","123456" }; 
			//��ΪLogIn�������������ˣ������Ǽ򵥵ĵ������࣬���������ôд

			LogIn login{};
			strncpy_s(login.username, 20, "Luhao", 6);
			strncpy_s(login.password, 32, "123456", 7);

			// ���ͱ���
			//***���˴������������һ�������***
			//�˴���client_sock��Ӧ���ǿͻ��˴������������ӷ��������׽��֡�
			//���ԣ����д������ڿͻ��˷�����Ϣ cmdBuf �������ӵķ�������
			//�����ڿͻ��ˣ�client_sock �� ָ�� ���������׽���
			send(client_sock, (char*)&login, sizeof(LogIn), 0);

			//// ���շ��������ص�����
			//LogInResult login_result{};

			//recv(client_sock, (char*)&login_result, sizeof(LogInResult), 0);

			//cout << "login_result��" << login_result.result << endl;
		}
		else if (!strcmp(cmdBuf, "logout"))
		{
			//LogOut logout{ "Luhao" };

			LogOut logout{};
			strncpy_s(logout.username, 20, "Luhao", 6);


			//***���˴������������һ�������***
			//�˴���client_sock��Ӧ���ǿͻ��˴������������ӷ��������׽��֡�
			//���ԣ����д������ڿͻ��˷�����Ϣ CmdMsg �������ӵķ�������
			//�����ڿͻ��ˣ�client_sock �� ָ�� ���������׽���
			send(client_sock, (char*)&logout, sizeof(LogOut), 0);

			////���շ��������ص�����
			//LogOutResult logout_result{};

			//recv(client_sock, (char*)&logout_result, sizeof(LogOutResult), 0);

			//cout << "logout_result��" << logout_result.result << endl;
		}
		else
			cout << "δʶ���������������룡\n";

	}
}

int Process(SOCKET socket)
{
	char RecvBuff[4096] = {};

	//�Ƚ��հ�ͷ
	int len = recv(socket, (char*)&RecvBuff, sizeof(DataHead), 0);
	if (len <= 0)
	{
		cout << "��������Ͽ����ӣ��������" << endl;
		return -1;
	}

	//DataHead* pHead = (DataHead*)RecvBuff;
	DataHead* pHead = reinterpret_cast<DataHead*>(RecvBuff);

	switch (pHead->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(socket, (char*)RecvBuff + sizeof(DataHead), 
				pHead->datalength - sizeof(DataHead), 0);

			LogInResult* loginresult = (LogInResult*)RecvBuff;

			cout << "�յ��������Ϣ��CMD_LOGIN_RESULT"
				<< " ���ݳ��ȣ�" << loginresult->datalength << endl;
		}
		break;

		case CMD_LOGOUT_RESULT:
		{
			recv(socket, (char*)RecvBuff + sizeof(DataHead),
				pHead->datalength - sizeof(DataHead), 0);

			LogOutResult* logoutresult = (LogOutResult*)RecvBuff;

			cout << "�յ��������Ϣ��CMD_LOGOUT_RESULT"
				<< " ���ݳ��ȣ�" << logoutresult->datalength << endl;
		}
		break;

		case CMD_NEW_USER_JOIN:
		{
			recv(socket, (char*)RecvBuff + sizeof(DataHead),
				pHead->datalength - sizeof(DataHead), 0);

			NewUserJoin* newuserjoin = (NewUserJoin*)RecvBuff;

			cout << "\n�յ��������Ϣ��CMD_NEW_USER_JOIN"
				<< " ���ݳ��ȣ�" << newuserjoin->datalength << endl;
		}
		break;
	}

}

// Ϊʲôʹ�ö��̣߳�
// ���߳��������ͬʱִ�ж���̣߳�ÿ���̶߳����Զ������У�
// �Ӷ�������ĳЩ�߳���Ϊ��������ͣʱ���������̼߳���ִ����������
// ����˳���Ĳ����Ժ���Ӧ�ԡ�
// ��һ�����̵߳�����Ӧ���У�һ���߳̿��ܸ�������������󣨿��ܻ���ȴ��������ݶ���������
// ����һ���߳���������Щ���յ������ݡ�
// ��������һ���̵߳ȴ��������ݵ�ͬʱ�������߳̿��Լ���ִ�ж������ܵ�������Ӱ�죬
// �Ӷ����������Ӧ�ó������Ӧ�ٶ�
#endif