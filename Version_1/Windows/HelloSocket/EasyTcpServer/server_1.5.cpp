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

#include <vector>

using namespace std;

vector<SOCKET> vec_client;

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

int Process(SOCKET client_sock);

int main()
{
	//��ʼ��

	//�����汾��
	WORD ver = MAKEWORD(2, 2);
	//����Windows Sockets API����
	WSADATA dat;
	WSAStartup(ver, &dat);

	//  ����һ��socket
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
	//�����ó��� INADDR_ANY ����ȡ�������˵� IP ��ַ
	serv_adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	//  bind �����ڽ��ܿͻ������ӵķ��������˿�
	if (bind(serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
		cout << "bind() ERROR";
	else
		cout << "������˿ڳɹ�...\n";

	//  listen ��������˿�
	if (listen(serv_sock, 5) == SOCKET_ERROR)
		cout << "listen() ERROR";
	else
		cout << "��������˿ڳɹ�...\n";

	// 4 ����timeval�ṹ���ֵĽṹ����timeout����Ϊselect���ĵ����ʵ��
	//Ϊ�˷�ֹ��������������״̬��ʹ�� timeout ���ݳ�ʱ��Ϣ��
	struct timeval timeout;

	while (1)
	{
		//����5��ĳ�ʱʱ��
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		//��
		//struct timeval timeout = { 5,0 };

		//������socket	 BSD socket

		// 1 ����fd_set�ṹ�彫Ҫ���ӵ��׽��־�����е�һ���Լ�����Щ�׽��־��
		fd_set fdRead;//��ע �Ƿ���ڴ���ȡ����
		fd_set fdWrite;
		fd_set fdExp;

		//***ע***
		// Windows��fd_set�ɳ�Ա fd_count��fd_array���ɣ� fd_count�����׽��־������
		// fd_array�Ǹ����鼯�ϣ����ڱ����׽��־��
		//����Windows���׽��־���������ܴ��㿪ʼ��
		//���������ɵľ��������ֵ֮��Ҳ�Ҳ����������
		//��Ҫһ�������������׽��ֵľ���Լ�һ����¼������ı���
		
		// 2 ʹ�ú�����ɶԽṹ��������λ������Ϊ0�Ĳ���
		//���fdRead������ϣ�fdWrite������ϣ�fdExp�������
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// 3 ʹ�ú�����ṹ����ע���׽��־��serv_sock����Ϣ
		// ���׽��־��serv_sock��Ӧ��λ����Ϊ1����
		// ���׽��־����ע�ᣩ��ӵ�fdRead���ϡ�fdWrite���ϡ�fdExp������
		// ��Ҫ����serv_sock�Ƿ��ж���д���쳣
		FD_SET(serv_sock,&fdRead);
		FD_SET(serv_sock,&fdWrite);
		FD_SET(serv_sock,&fdExp);

		// 4 ���ͻ����׽��־����ӵ�fdRead����
		// ��������Ŀ����Ϊ�˽���Щ�����ӵĿͻ����׽��ּ��뵽 fdRead �����н��м��ӣ�
		// �Ա��ڵ��� select() ����ʱ���ܹ�������Щ�׽��ֵĶ�ȡ������
		// ����ζ��������κ������ӵĿͻ��˷������ݣ�select() ���������ز�֪ͨ����
		// ʹ�ó���������׽��ֿɶ�������½�����Ӧ�Ĵ���
		for (int n = (int)vec_client.size() - 1; n >=0 ; --n)
		{
			FD_SET(vec_client[n],&fdRead);
		}

		cout << "fdRead.fd_count = " << fdRead.fd_count << endl;


		// 5 ʹ��select����
		// ���ڵ�һ��������int nfds����ָfd_set�����������׽��־���ķ�Χ��
		// �����������������о�������ֵ+1����windows�п���д0
		// ҪдΪsocketֵ������һ��socketֵ�ټ�һ��
		int fd_num = select(serv_sock + 1, &fdRead, &fdWrite, &fdExp, &timeout);
		if (fd_num == -1)
		{
			cout << "select�������" << endl;
			break;
		}
		else if (fd_num == 0)
		{
			cout << "����ʱ�䴦������ҵ��" << endl;
		}
		
		//��FD_ISSET��ɸѡ��fdRead�����з���״̬�仯�ľ��
		//�����������׽��֣�serv_sock���Ƿ��ڴ���ȡ���ݵ�״̬
		//����Ƿ��������׽��ֵı仯����ʾ�пͻ����������󵽴��������������
		if (FD_ISSET(serv_sock, &fdRead))
		{
			//��fdRead������ɾ���þ��
			//�ں����� I/O ��·���ò����в��ټ�������þ����״̬�仯
			//����дҲû��ϵ��
			FD_CLR(serv_sock,&fdRead);

			SOCKET client_sock = INVALID_SOCKET;
			sockaddr_in client_adr = {};
			int client_adr_size = sizeof(client_adr);

			//  accept �ȴ����ܿͻ�������
			client_sock = accept(serv_sock, (sockaddr*)&client_adr, &client_adr_size);
			if (client_sock == INVALID_SOCKET)
				cout << "���󣬽��ܵ���Ч�ͻ���SOCKET";
			else
			{
				//inet_ntoa()�������ֽ���������� IP ��ַת��Ϊ�ַ�����ʽ
				cout << "�¿ͻ��˼��룺IP = " << inet_ntoa(client_adr.sin_addr)
					<< "  <socket=" << client_sock << ">" << endl;

				//֪ͨ�ÿͻ���֮ǰ�����пͻ��������û�����
				for (int n = (int)vec_client.size() - 1; n >= 0; --n)
				{
					NewUserJoin newuserjoin;
					send(vec_client[n], (const char*)&newuserjoin, sizeof(NewUserJoin), 0);
				}

				vec_client.push_back(client_sock);
			}
		}

		//***ע***
		//��� select() ���غ�û���κ��׽��ִ��ڴ���ȡ״̬��fdRead.fd_count ��Ϊ 0
		//����������fdRead.fd_count��ֵ��ǰ�߻���1  2  ����󣬶�����ͨ��ֻ����0����1
		//��ͨ������ָ����ͻ��˲���ͬʱ�����˷������ݣ�
		cout << "fdRead.fd_count = " << fdRead.fd_count << endl;

		//���������ӵĿͻ����׽��־��
		//ѭ������ fdRead �����е��׽��־����Ȼ��ͨ������ Process() ������������Щ���
		for (int n = 0; n < fdRead.fd_count; ++n)
		{
			//����д��Ϊʲô�ڴ���ڶ����ͻ���ʱ�����޷�����״̬�仯������
			//if(FD_ISSET(vec_client[n], &fdRead))  {...}
			//��Ϊ�µ������ӵĿͻ����׽��־����������������β������������д�޷�ȷ��
			//���ĸ��ͻ��������˽��������ݴ���

			if (Process(fdRead.fd_array[n]) == -1)
			{
				//���ĳ���ͻ��˳��������Ͽ����ӣ��ͽ���������ӿͻ��˵��б���ɾ����
				auto it = find(vec_client.begin(), vec_client.end(), fdRead.fd_array[n]);
				if (it != vec_client.end())
					vec_client.erase(it);
			}
			
		}
	}

	//�ر�ȫ���Ŀͻ����׽���
	for (int n = vec_client.size() - 1; n >= 0; --n)
	{
		closesocket(vec_client[n]);
	}

	//  �ر��׽���closesocket
	closesocket(serv_sock);

	//ע��
	WSACleanup();

	cout << "�����������˳������������\n";
	cin.get();

	return 0;
}

int Process(SOCKET client_sock)
{
	DataHead dh = {};

	//  ���տͻ�������
	int len = recv(client_sock, (char*)&dh, sizeof(DataHead), 0);

	if (len <= 0)
	{
		cout << "�ͻ���<socket="<<client_sock<<">���˳���\n";
		return -1;
	}

	//  ��������
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
			recv(client_sock, (char*)&login + sizeof(DataHead), sizeof(LogIn) - sizeof(DataHead), 0);

			//�����ж��û������Ƿ���ȷ�Ĺ���

			cout << "�յ��ͻ���<socket="<<client_sock<<">���CMD_LOGIN" 
				<< " ���ݳ��ȣ�" << login.datalength << endl;
			cout << "�û�����" << login.username << "����" << endl;

			// ���ͱ���
			LogInResult res{};
			send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
		}
		break;

		case CMD_LOGOUT:
		{
			LogOut logout{};
			recv(client_sock, (char*)&logout + sizeof(DataHead), sizeof(LogOut) - sizeof(DataHead), 0);

			cout << "�յ��ͻ���<socket=" << client_sock << ">���CMD_LOGOUT"
				<< " ���ݳ��ȣ�" << logout.datalength << endl;
			cout << "�û�����" << logout.username << "�ǳ�" << endl;

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
//***ע***
//select() ����ͨ���� I/O ��·���û�����������������������Ƕ��̵߳ķ��롿
//����һ���ڵ����߳���ʵ�ֶ�· I/O �Ļ��ơ�
//select() ��һ�����������̻��̼߳��Ӷ���ļ���������ͨ�����׽��֣���״̬�仯�ķ�ʽ��
// ���ܹ�������ļ��������Ƿ���������磬�Ƿ������ݿɶ����Ƿ��д�ȣ���
// ��������֮������л����Ӷ��������ͬʱ������� I/O ������
//���Խ� select() ����߳�һ��ʹ���������������� I/O ������
//���磬��һ���߳���ʹ�� select() ���Ӷ���׽��ֵ�״̬�仯��
//�������¼��ķ���������������߳�����������ҵ���߼���

//�ڸ����Ĵ����У�ʹ���� `select` �������ȴ��ͻ��˵����������Լ����������ӵĿͻ��ˡ�
// �����һ���ͻ��˽���������еڶ����ͻ��˽��룬��������й��̿������£�
//1. ��ʼ���ʹ����׽��֣�����ʼʱ��ͨ�� `socket()` ������һ��������׽��� `serv_sock`��
//2. �󶨺ͼ�����ʹ�� `bind()` �� `serv_sock` �󶨵�ָ���˿ڣ���ʹ�� `listen()` ��ʼ�������Կͻ��˵���������
//3. ������ѭ����������� `while(1)` ѭ����ʹ�� `select()` �����ȴ���������������ӿͻ��˵����ݶ�ȡ��
//4. ��һ���ͻ������ӣ�����һ���ͻ��˷�����������ʱ��`select()` ������֪ͨ���µ��������󵽴
// `FD_ISSET(serv_sock, & fdRead)` ���� true��Ȼ����� `accept()` ���ܸÿͻ��˵����ӣ��õ�һ���µ��׽��� `client_sock1`��
//5. �����һ���ͻ��ˣ����� `client_sock1` ��ӵ� `vec_client` �У�������ѭ����
//6. �ڶ����ͻ������ӣ���ѭ���У��ڶ����ͻ���Ҳ���������������� `select()` ����һֱ�ڼ��� `serv_sock` �Ķ�������
// �����⵽���µ��������󵽴`FD_ISSET(serv_sock, & fdRead)` �ٴη��� true��
//7. ���ܵڶ����ͻ������ӣ���Ϊ `serv_sock` ���ڿɶ�״̬�������ٴε��� `accept()` ���������ܵڶ����ͻ��˵����ӣ�
// �õ���һ���µ��׽��� `client_sock2`��
//8. ����ڶ����ͻ��ˣ����Ƶأ����� `client_sock2` ��ӵ� `vec_client` �У����������к����Ĵ���
//9. ѭ����������������� `while(1)` ѭ����ʹ�� `select()` �����ȴ��µ���������������ӿͻ��˵����ݶ�ȡ��
// ������µ��������󵽴�����������ӿͻ��˷������ݣ�`select()` �����ز�������Ӧ���¼���
//������̽�������ȥ������᲻�Ͻ����µĿͻ������Ӳ����������ӿͻ��˵����ݣ�ֱ����ĳ��������������󡢶Ͽ����ӵȣ�
// ���³�������ѭ����
#endif