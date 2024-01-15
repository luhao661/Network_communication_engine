#pragma once

#ifdef _WIN32
//���windows.h��winsock2.h�º궨���ͻ
#define WIN32_LEAN_AND_MEAN
//ʹinet_ntoa()����
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define FD_SETSIZE 1024

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
#else
//UNIX �±�׼C����ͷ�ļ�
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

//��SOCKET���ͺ궨��Ϊint����
//��ΪLinux���׽����ļ�����������int����
#define SOCKET int
//��������Linux��û�еĺ궨��
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include "MessageHeader_1.1.hpp"
#include <iostream>
#include <vector>

using namespace std;

#ifndef RECV_BUFFER_SIZE
#define RECV_BUFFER_SIZE 1024
#endif

//�ڷ���ˣ�����Ҫ��������ͬ�Ŀͻ��˵�����
//���ÿ���ͻ��˶�Ӧ����
//�Զ���Ľ��ջ���������Ϣ����������ָ����Ϣ������������
//β��λ�õı���
class ClientSocket
{
private:
	SOCKET m_client_sock;

	//�Զ���Ľ��ջ�����
	char m_Recv[RECV_BUFFER_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char m_MsgBuf[RECV_BUFFER_SIZE * 10] = {};
	//ָ����Ϣ������������β��λ��
	int m_lastPos = 0;

public:
	ClientSocket(SOCKET m_client_sock = INVALID_SOCKET);

	SOCKET Get_m_client_sock();

	char* Get_m_Recv();

	char* Get_m_MsgBuf();

	int Get_m_lastPos();

	void Set_m_lastPos(int NewPos);
};

ClientSocket::ClientSocket(SOCKET sock)
{
	m_client_sock = sock;
	memset(m_MsgBuf, 0, sizeof(m_MsgBuf));
}

SOCKET ClientSocket::Get_m_client_sock()
{
	return m_client_sock;
}

char* ClientSocket::Get_m_Recv()
{
	return m_Recv;
}

char* ClientSocket::Get_m_MsgBuf()
{
	return m_MsgBuf;
}

int ClientSocket::Get_m_lastPos()
{
	return m_lastPos;
}

void ClientSocket::Set_m_lastPos(int NewPos)
{
	m_lastPos = NewPos;
}

class EasyTcpServer
{
private:
	SOCKET m_serv_sock;
	vector<ClientSocket*> vec_client;

	//�Զ���Ľ��ջ�����
	char m_Recv[RECV_BUFFER_SIZE] = {};

public:
	EasyTcpServer();

	virtual ~EasyTcpServer();

	SOCKET initSocket();

	void Close();

	int Bind(const char* ip, unsigned short port);

	int Listen(int backlog);

	SOCKET Accept();


	//�Ƿ�������������
	bool isRun()
	{
		return m_serv_sock != INVALID_SOCKET;
	}

	//������������
	//��ѯ�Ƿ��д���ȡ������
	bool OnRun();

	//��������
	//����ճ�� ��ְ�
	int RecvData(ClientSocket* pClientSocket);

	//��Ӧ������Ϣ
	//***ע***
	//����Ϊvirtual����֮����౻�̳У�Ҫ��д��Ӧ������Ϣ����ʱ��
	//OnNetMsg()�����ɻ�ö�̬��
	void OnNetMsg(SOCKET client_sock, DataHead* pHead);

	//��������
	int SendData(SOCKET client_sock, DataHead* pHead);

	void SendDataToAll(DataHead* pHead);
};

EasyTcpServer::EasyTcpServer()
{
	m_serv_sock = INVALID_SOCKET;
}

EasyTcpServer:: ~EasyTcpServer()
{
	Close();
}

SOCKET EasyTcpServer::initSocket()
{
#ifdef _WIN32
	//��ʼ��

	//�����汾��
	WORD ver = MAKEWORD(2, 2);
	//����Windows Sockets API����
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	//�����ǰ������׽����Ѿ������ˣ��������ظ������׽���
	//�Ǿ͹ر��ˣ������´���һ��
	if (m_serv_sock != INVALID_SOCKET)
	{
		cout << "<socket=" << m_serv_sock << ">�رվ�����\n";
		Close();
	}

	//  ����һ��socket
	m_serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//***ע***
	// ���������У�AF_INET ����ָ����ַ�壨Address Family��Ϊ IPv4��
	// �� PF_INET ������ָ��Э����Ϊ IPv4��
	// ��ʵ��ʹ���У�AF_INET �� PF_INET ���Ի���ʹ�ã�
	// �����ڴ��������£���������ȵġ�

	if (m_serv_sock == INVALID_SOCKET)
		cout << "����socketʧ��\n";
	else
		cout << "���������socket=<" << m_serv_sock << ">�ɹ�...\n";


	return m_serv_sock;
}

void EasyTcpServer::Close()
{
	if (m_serv_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		//�ر�ȫ���Ŀͻ����׽���
		for (int n = vec_client.size() - 1; n >= 0; --n)
		{
			closesocket(vec_client[n]->Get_m_client_sock());
			delete vec_client[n];
		}

		//  �ر��׽���closesocket
		closesocket(m_serv_sock);

		//ע��
		WSACleanup();
#else

		//�ر�ȫ���Ŀͻ����׽���
		for (int n = (int)vec_client.size() - 1; n >= 0; --n)
		{
			close(vec_client[n]->Get_m_client_sock());
			delete vec_client[n];
		}

		//  �ر��׽���closesocket
		close(m_serv_sock);
		m_serv_sock = INVALID_SOCKET;

#endif

		vec_client.clear();
	}
}

int EasyTcpServer::Bind(const char* ip, unsigned short port)
{
	if (INVALID_SOCKET == m_serv_sock)
		initSocket();

	//��ַ��Ϣ��ʼ��
	//serv_adr�ṹ��һ��Ҫ���г�ʼ����ԭ�����TCP/IP�����̶���ʼǡ�
	sockaddr_in serv_adr = {};
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_port = htons(port);

	if (ip == nullptr)
	{
		//�����ó��� INADDR_ANY ����ȡ�������˵� IP ��ַ
#ifdef _WIN32
		serv_adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
		serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
	}
	else
	{
#ifdef _WIN32
		serv_adr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		serv_adr.sin_addr.s_addr = inet_addr(ip);
#endif
	}

	//  bind �����ڽ��ܿͻ������ӵķ��������˿�
	//Mac�����½�bind�����޶�Ϊʹ��ȫ�������ռ��еĺ���
	int res = ::bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (res == SOCKET_ERROR)
		cout << "bind() ERROR��������˿�<" << port << ">ʧ��...\n";
	else
		cout << "������˿�<" << port << ">�ɹ�...\n";

	return res;
}

int EasyTcpServer::Listen(int backlog)
{
	//  listen ��������˿�
	int res = listen(m_serv_sock, backlog);

	if (res == SOCKET_ERROR)
		cout << "listen() ERROR��socket=<" << m_serv_sock << ">��������˿�ʧ��\n";
	else
		cout << "socket=<" << m_serv_sock << ">��������˿ڳɹ�...\n";

	return res;
}

SOCKET EasyTcpServer::Accept()
{
	SOCKET client_sock = INVALID_SOCKET;
	sockaddr_in client_adr = {};
	int client_adr_size = sizeof(client_adr);

	//  accept �ȴ����ܿͻ�������
#ifdef _WIN32
	client_sock = accept(m_serv_sock, (sockaddr*)&client_adr, &client_adr_size);
#else
	client_sock = accept(m_serv_sock, (sockaddr*)&client_adr, (socklen_t*)&client_adr_size);
#endif

	if (client_sock == INVALID_SOCKET)
		cout << "socket=<" << m_serv_sock << ">���󣬽��ܵ���Ч�ͻ���SOCKET";
	else
	{
		//inet_ntoa()�������ֽ���������� IP ��ַת��Ϊ�ַ�����ʽ
		/*
		cout << "socket=<" << m_serv_sock << ">�¿ͻ��˼��룺IP = "
			<< inet_ntoa(client_adr.sin_addr)
			<< "  socket=<" << client_sock << ">" << endl;
		*/

		//printf()����
		printf("socket=<%d>�¿ͻ��˼��룺IP = %s   socket=<%d> ,��%d���ͻ��˼���\n", 
			m_serv_sock, inet_ntoa(client_adr.sin_addr), client_sock,vec_client.size()+1);

		//֪ͨ�ÿͻ���֮ǰ�����пͻ��������û�����
		NewUserJoin newuserjoin;
		SendDataToAll(&newuserjoin);

		vec_client.push_back(new ClientSocket(client_sock));
	}

	return client_sock;
}

static long long cnt = 0;

//������������
//��ѯ�Ƿ��д���ȡ������
bool EasyTcpServer::OnRun()
{
	if (!isRun())
		return false;

	// 4 ����timeval�ṹ���ֵĽṹ����timeout����Ϊselect���ĵ����ʵ��
	//Ϊ�˷�ֹ��������������״̬��ʹ�� timeout ���ݳ�ʱ��Ϣ��
	struct timeval timeout;

	//����5��ĳ�ʱʱ��
	timeout.tv_sec = 1;
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
	FD_SET(m_serv_sock, &fdRead);
	FD_SET(m_serv_sock, &fdWrite);
	FD_SET(m_serv_sock, &fdExp);

	//����maxSocket���洢���пͻ��˼������׽��ֵ����ֵ
	SOCKET maxSocket = m_serv_sock;
	//***ע***
	//Windows�²���Ҫ����Ϊ
	//�� Windows ��ʹ�� select()��ʵ��������������������������ʵ�ֶ��ļ��������ļ��ӣ�
	//��ڶ������� fdset����ָ��һ�� fd_set �ṹ�壬���а���Ҫ���ӵ��ļ����������ϣ�
	//���������� nfds ��������һ��������ȷ�����ӵ��ļ�����������

	// 4 ���ͻ����׽��־����ӵ�fdRead����
	// ��������Ŀ����Ϊ�˽���Щ�����ӵĿͻ����׽��ּ��뵽 fdRead �����н��м��ӣ�
	// �Ա��ڵ��� select() ����ʱ���ܹ�������Щ�׽��ֵĶ�ȡ������
	// ����ζ��������κ������ӵĿͻ��˷������ݣ�select() ���������ز�֪ͨ����
	// ʹ�ó���������׽��ֿɶ�������½�����Ӧ�Ĵ���
	for (int n = (int)vec_client.size() - 1; n >= 0; --n)
	{
		FD_SET(vec_client[n]->Get_m_client_sock(), &fdRead);
		maxSocket = maxSocket < (vec_client[n]->Get_m_client_sock()) ?
			(vec_client[n]->Get_m_client_sock()) : maxSocket;
	}

	// cout << "fdRead.fd_count = " << fdRead.fd_count << endl;

	// 5 ʹ��select����
	// ���ڵ�һ��������int nfds����ָfd_set�����������׽��־���ķ�Χ��
	// �����������������о�������ֵ+1����windows�п���д0
	// ҪдΪsocketֵ������һ��socketֵ�ټ�һ��
	int fd_num = select(maxSocket + 1, &fdRead, &fdWrite, &fdExp, &timeout);
	if (fd_num == -1)
	{
		cout << "select�������" << endl;
		Close();
		return false;
	}
	else if (fd_num == 0)
	{

	}
	//cout << "fd_num=" << fd_num << ", cnt=" << cnt++ << endl;

	//��FD_ISSET��ɸѡ��fdRead�����з���״̬�仯�ľ��
	//�����������׽��֣�serv_sock���Ƿ��ڴ���ȡ���ݵ�״̬
	//����Ƿ��������׽��ֵı仯����ʾ�пͻ����������󵽴��������������
	if (FD_ISSET(m_serv_sock, &fdRead))
	{
		//��fdRead������ɾ���þ��
		//�ں����� I/O ��·���ò����в��ټ�������þ����״̬�仯
		//����дҲû��ϵ��
		FD_CLR(m_serv_sock, &fdRead);

		Accept();
	}

	for (int n = (int)vec_client.size() - 1; n >= 0; n--)
	{
		if (FD_ISSET(vec_client[n]->Get_m_client_sock(), &fdRead))
		{
			if (RecvData(vec_client[n]) == -1)
			{
				auto it = vec_client.begin() + n;
				if (it != vec_client.end())
				{
					delete* it;
					vec_client.erase(it);
				}
			}
		}
	}

	return true;
}

//��������
//����ճ�� ��ְ�
#if 1
int EasyTcpServer::RecvData(ClientSocket* pClientSocket)
{
	//  ���տͻ������ݴ浽��ĳ�ͻ��˶�Ӧ�ĵġ��Զ�����ջ�����m_Recv
	int len = (int)recv(pClientSocket->Get_m_client_sock(),
		pClientSocket->Get_m_Recv(), RECV_BUFFER_SIZE, 0);

	if (len <= 0)
	{
		cout << "�ͻ���<socket=" << pClientSocket->Get_m_client_sock()
			<< ">���˳���\n";
		return -1;
	}

	//����ĳ�ͻ��˶�Ӧ�ġ��Զ�����ջ�����m_Recv������
	//��������ĳ���ͻ��˶�Ӧ�ġ���Ϣ������
	memcpy(pClientSocket->Get_m_MsgBuf() + pClientSocket->Get_m_lastPos(),
		pClientSocket->Get_m_Recv(), len);
	//��ʾ��Ϣ������������β����λ�õı���m_lastPos����
	pClientSocket->Set_m_lastPos(pClientSocket->Get_m_lastPos() + len);

	//�ж���Ϣ�����������ݳ����Ƿ������Ϣͷ�ĳ���
	//��whileѭ���������ճ����
	while (pClientSocket->Get_m_lastPos() >= sizeof(DataHead))
	{
		//ָ��ĳ���ͻ��˶�Ӧ��m_MsgBuf��ָ�����ΪDataHead*���͵�ָ�룬
		//���ڷ���DataHead�����ݳ�Ա
		DataHead* pHead = reinterpret_cast<DataHead*>(pClientSocket->Get_m_MsgBuf());

		//�ж���Ϣ�����������ݳ����Ƿ������Ϣ����
		//������ٰ���������
		if (pClientSocket->Get_m_lastPos() >= pHead->datalength)
		{
			//����������Ϣ
			OnNetMsg(pClientSocket->Get_m_client_sock(), pHead);

			//�ݴ��ʾ�Զ������Ϣ��������ʣ��δ��������ݵĳ��ȵı���
			int unprocessed = pClientSocket->Get_m_lastPos() - pHead->datalength;

			//��m_MsgBuf���Ѿ����������Ϣ������������δ��������ݽ���
			//���ݸ���
			memcpy(pClientSocket->Get_m_MsgBuf(),
				pClientSocket->Get_m_MsgBuf() + pHead->datalength, unprocessed);

			//����m_MsgBuf������β����λ��
			pClientSocket->Set_m_lastPos(unprocessed);
		}
		else
			break;
	}

	return 0;
}
#endif

#if 0
int EasyTcpServer::RecvData(ClientSocket* pClientSocket)
{
	//  ���տͻ������ݴ浽������˵ġ��Զ�����ջ�����m_Recv
	int len = (int)recv(pClientSocket->Get_m_client_sock(),
		m_Recv, RECV_BUFFER_SIZE, 0);

	//cout << "len = " << len << endl;

	if (len <= 0)
	{
		cout << "�ͻ���<socket=" << pClientSocket->Get_m_client_sock()
			<< ">���˳���\n";
		return -1;
	}

	//��������˵ġ��Զ�����ջ�����m_Recv������
	//��������ĳ���ͻ��˶�Ӧ�ġ���Ϣ������
	memcpy(pClientSocket->Get_m_MsgBuf() + pClientSocket->Get_m_lastPos(),
		m_Recv, len);
	//��ʾ��Ϣ������������β����λ�õı���m_lastPos����
	pClientSocket->Set_m_lastPos(pClientSocket->Get_m_lastPos() + len);

	//�ж���Ϣ�����������ݳ����Ƿ������Ϣͷ�ĳ���
	//��whileѭ���������ճ����
	while (pClientSocket->Get_m_lastPos() >= sizeof(DataHead))
	{
		//ָ��ĳ���ͻ��˶�Ӧ��m_MsgBuf��ָ�����ΪDataHead*���͵�ָ�룬
		//���ڷ���DataHead�����ݳ�Ա
		DataHead* pHead = reinterpret_cast<DataHead*>(pClientSocket->Get_m_MsgBuf());

		//�ж���Ϣ�����������ݳ����Ƿ������Ϣ����
		//������ٰ���������
		if (pClientSocket->Get_m_lastPos() >= pHead->datalength)
		{
			//����������Ϣ
			OnNetMsg(pClientSocket->Get_m_client_sock(), pHead);

			//�ݴ��ʾ�Զ������Ϣ��������ʣ��δ��������ݵĳ��ȵı���
			int unprocessed = pClientSocket->Get_m_lastPos() - pHead->datalength;

			//��m_MsgBuf���Ѿ����������Ϣ������������δ��������ݽ���
			//���ݸ���
			memcpy(pClientSocket->Get_m_MsgBuf(),
				pClientSocket->Get_m_MsgBuf() + pHead->datalength, unprocessed);

			//����m_MsgBuf������β����λ��
			pClientSocket->Set_m_lastPos(unprocessed);
		}
		else
			break;
	}

	return 0;
}
#endif

//��Ӧ������Ϣ
void EasyTcpServer::OnNetMsg(SOCKET client_sock, DataHead* pHead)
{
	//  ��������
	switch (pHead->cmd)
	{
	case CMD_LOGIN:
	{
		LogIn* login = reinterpret_cast<LogIn*>(pHead);

		//�����ж��û������Ƿ���ȷ�Ĺ���

		//***ע***
		//д���´��룬�ᵼ�·���˺Ϳͻ����ڽ���ͨ�ź󲻾þͲ�����
		//cout << "�յ��ͻ���<socket=" << client_sock << ">���CMD_LOGIN"
		//	<< " ���ݳ��ȣ�" << login->datalength << endl;
		//cout << "�û�����" << login->username << "����" << endl;

		// ���ͱ���
		//LogInResult res{};
		//send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
		//���Ը�д��
		//SendData(client_sock, &res);
	}
	break;

	case CMD_LOGOUT:
	{
		LogOut* logout = reinterpret_cast<LogOut*>(pHead);

		cout << "�յ��ͻ���<socket=" << client_sock << ">���CMD_LOGOUT"
			<< " ���ݳ��ȣ�" << logout->datalength << endl;
		cout << "�û�����" << logout->username << "�ǳ�" << endl;

		LogOutResult res{};
		send(client_sock, (const char*)&res, sizeof(LogOutResult), 0);
	}
	break;

	default:
	{
		DataHead head = { sizeof(DataHead),CMD_ERROR };
		send(client_sock, (const char*)&head, sizeof(DataHead), 0);

		cout << "Error!" << endl;
	}
	}
}

//��ָ���ͻ��˷�������
int EasyTcpServer::SendData(SOCKET client_sock, DataHead* pHead)
{
	if (isRun() && pHead)
	{
		return send(client_sock, (const char*)pHead, pHead->datalength, 0);
	}

	return SOCKET_ERROR;
}

//�����пͻ��˽��й㲥
void EasyTcpServer::SendDataToAll(DataHead* pHead)
{
	for (int n = (int)vec_client.size() - 1; n >= 0; --n)
	{
		SendData(vec_client[n]->Get_m_client_sock(), pHead);
	}
}
