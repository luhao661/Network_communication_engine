#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#if 1
#pragma once

#ifdef _WIN32
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
#define RECV_BUFFER_SIZE 10240
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
		cout << "socket=<" << m_serv_sock << ">�¿ͻ��˼��룺IP = "
			<< inet_ntoa(client_adr.sin_addr)
			<< "  socket=<" << client_sock << ">" << endl;

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
		LogInResult res{};
		//send(client_sock, (const char*)&res, sizeof(LogInResult), 0);
		//���Ը�д��
		SendData(client_sock, &res);
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
#endif


#if 0
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include<vector>
#include"MessageHeader_1.1.hpp"
#include <iostream>
using namespace std;
//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 102400
#endif // !RECV_BUFF_SZIE

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE * 10];
	//��Ϣ������������β��λ��
	int _lastPos;
};

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//��ʼ��Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���socketʧ��...\n");
		}
		else {
			printf("����socket=<%d>�ɹ�...\n", (int)_sock);
		}
		return _sock;
	}

	//��IP�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short

#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("����,������˿�<%d>ʧ��...\n", port);
		}
		else {
			printf("������˿�<%d>�ɹ�...\n", port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n)
	{
		// 3 listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("socket=<%d>����,��������˿�ʧ��...\n", _sock);
		}
		else {
			printf("socket=<%d>��������˿ڳɹ�...\n", _sock);
		}
		return ret;
	}

	//���ܿͻ�������
	SOCKET Accept()
	{
		// 4 accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("socket=<%d>����,���ܵ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			printf("socket=<%d>�¿ͻ��˼��룺socket = %d,IP = %s \n", (int)_sock, (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			// 8 �ر��׽���closesocket
			closesocket(_sock);
			//------------
			//���Windows socket����
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			// 8 �ر��׽���closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}
	//����������Ϣ
	int _nCount = 0;
	bool OnRun()
	{
		if (isRun())
		{
			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
			fd_set fdWrite;
			fd_set fdExp;
			//������
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//����������socket�����뼯��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t); //
			//printf("select ret=%d count=%d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return false;
			}
			//�ж���������socket���Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;

	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//������
	char _szRecv[RECV_BUFF_SZIE] = {};

	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocket* pClient)
	{
		// 5 ���տͻ�������
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE, 0);
		//printf("nLen=%d\n", nLen);
		if (nLen <= 0)
		{
			printf("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�������ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		while (pClient->getLastPos() >= sizeof(DataHead))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHead* head = (DataHead*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (pClient->getLastPos() >= head->datalength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = pClient->getLastPos() - head->datalength;
				//����������Ϣ
				OnNetMsg(pClient->sockfd(), head);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + head->datalength, nSize);
				//��Ϣ������������β��λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else {
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock, DataHead* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			LogIn* login = (LogIn*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LogInResult ret;
			SendData(cSock, &ret);
		}
		break;

		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", cSock, header->datalength);
			//DataHeader ret;
			//SendData(cSock, &ret);
		}
		break;
		}
	}

	//����ָ��Socket����
	int SendData(SOCKET cSock, DataHead* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->datalength, 0);
		}
		return SOCKET_ERROR;
	}

	void SendDataToAll(DataHead* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}

};

#endif // !_EasyTcpServer_hpp_
#endif
