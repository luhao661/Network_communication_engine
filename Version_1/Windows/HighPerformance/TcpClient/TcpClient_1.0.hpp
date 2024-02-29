#pragma once
//.hpp �ļ���չ��ͨ������ C++ ��ͷ�ļ����� .h ͷ�ļ����������ơ�
// ���Ƕ������� C++ ��������������������塢����ԭ�͡��ࡢģ�����Ϣ��
// ������Դ�ļ����ú�ʹ�á�
//.hpp �ļ�ͨ����ʾ C++ ͷ�ļ���C++ ͷ�ļ��п��ܰ��� C++ �ض������ݣ�
// ����ģ�塢namespace ������ C++ ���ԣ��� .h �ļ����ܽ����� C ������.
//C++ ��������Ĭ�Ͻ� .hpp �ļ�ʶ��Ϊ C++ �ļ�

#ifdef   _WIN32
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

#include <iostream>
#include "MessageHeader_1.0.hpp"//100�ֽ�

using namespace std;

#ifndef RECV_BUFFER_SIZE
#define RECV_BUFFER_SIZE 10240
#endif

class EasyTcpClient
{
private:
	SOCKET m_client_sock;

	//�Զ���Ľ��ջ�����
	char m_Recv[RECV_BUFFER_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char m_MsgBuf[RECV_BUFFER_SIZE * 10] = {};
	//ָ����Ϣ������������β��λ��
	int m_lastPos = 0;

	bool m_isConnected = false;

public:
	EasyTcpClient();

	//�ڶ�̬����±���ֲ����ٶ��󣨡�Effective C++��P41��
	virtual ~EasyTcpClient();

	void initSocket();

	void Close();

	int Connect(const char* ip, unsigned short port);

	//�Ƿ�������������
	bool isRun()
	{
		return (m_client_sock != INVALID_SOCKET) && m_isConnected;
	}

	//������������
	//��ѯ�Ƿ��д���ȡ������
	bool OnRun();

	//��������
	//����ճ�� ��ְ�
	int RecvData();

	//��Ӧ������Ϣ
	void OnNetMsg(DataHead* pHead);

	//��������
	int SendData(DataHead* pHead, int Len);
};

EasyTcpClient::EasyTcpClient()
{
	m_client_sock = INVALID_SOCKET;
}

EasyTcpClient::~EasyTcpClient()
{
	Close();
}

void EasyTcpClient::initSocket()
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
	if (m_client_sock != INVALID_SOCKET)
	{
		cout << "<socket=" << m_client_sock << ">�رվ�����\n";
		Close();
	}

	// 1 ����һ��socket
	m_client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_client_sock == INVALID_SOCKET)
		cout << "����socketʧ��\n";
	else
	{
		//cout << "����socket=<" << m_client_sock << ">�ɹ�...\n";
	}
}

int EasyTcpClient::Connect(const char* ip, unsigned short port)
{
	//����׽��ֻ�û������
	if (m_client_sock == INVALID_SOCKET)
	{
		initSocket();
	}
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
	serv_adr.sin_port = htons(port);
	//serv_adr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//127.0.0.1��IPv4��ַ�ռ��е�һ�����Ᵽ����ַ��
	//Ҳ��Ϊ�ػ���ַ�򱾵ػػ���ַ��
	//��ͨ�����������������ϵĻ��ؽӿڣ�
	//�����ڼ�����ڲ���������ͨ�źͲ������繦�ܡ�
	//��������������ӵ�127.0.0.1ʱ����ʵ�������ڳ������Լ�������ӿڽ���ͨ�š�
	//��дΪ

#ifdef _WIN32
	serv_adr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	serv_adr.sin_addr.s_addr = inet_addr(ip);
#endif

	// 2 ���ӷ����� connect
	int res = connect(m_client_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (res == SOCKET_ERROR)
		cout << "<socket=" << m_client_sock << ">���ӷ�����<"
		<< ip << " : " << port << ">ʧ��...\n";
	else
	{
		//cout << "<socket=" << m_client_sock << ">���ӷ�����<"
		//<< ip << " : " << port << ">�ɹ�...\n";

		//printf("<socket=%d>���ӷ�����<%s : %hd>�ɹ�...\n",
			//m_client_sock, ip, (short)port);

		m_isConnected = true;
	}
	//***ע***
	//�ͻ����׽����ڵ���connect()ʱ�󶨣�Ҳ�з��䣩�˿ͻ��˵�ַ

	return res;
}

void EasyTcpClient::Close()
{
	//��������رտͻ����׽��֣���ô������������Ҫ�ٹر�һ������
	if (m_client_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		// 7 �ر��׽���closesocket
		closesocket(m_client_sock);

		//ע��
		WSACleanup();

#else
		close(m_client_sock);
#endif
		//�����ظ��ر�
		m_client_sock = INVALID_SOCKET;
	}

	m_isConnected = false;
}

//static long long cnt = 0;

bool EasyTcpClient::OnRun()
{
	if (!isRun())
		return false;

	fd_set fdRead;

	FD_ZERO(&fdRead);

	FD_SET(m_client_sock, &fdRead);

	struct timeval timeout = { 0,0 };

	int fd_num = select(m_client_sock + 1, &fdRead, 0, 0, &timeout);
	if (fd_num == -1)
	{
		cout << "<socket=" << m_client_sock << ">select�������_1" << endl;
		//��ֹ��⵽����ֵ-1�����ڳ�������OnRun()
		Close();
		return false;
	}
	else if (fd_num == 0)
	{
		//cout << "����ʱ�䴦������ҵ��" << endl;
	}

	//cout << "fd_num=" << fd_num << ", cnt=" << cnt++ << endl;

	if (FD_ISSET(m_client_sock, &fdRead))
	{
		FD_CLR(m_client_sock, &fdRead);

		if (-1 == RecvData())
		{
			cout << "<socket=INVALID_SOCKET" << ">select�������_2" << endl;
		}
	}

	return true;
}

int EasyTcpClient::RecvData()
{
	//����RECV_BUFFER_SIZE���ȵ����ݵ��Զ���Ľ��ջ�����
	int len = (int)recv(m_client_sock, (char*)&m_Recv, RECV_BUFFER_SIZE, 0);
	if (len <= 0)
	{
		cout << "<socket=" << m_client_sock <<
			">��������Ͽ����ӣ��������" << endl;
		return -1;
	}

	//���յ������ݿ�������Ϣ������
	memcpy(m_MsgBuf + m_lastPos, m_Recv, len);
	//��ʾ��Ϣ������������β����λ�õı���m_lastPos����
	m_lastPos += len;

	//�ж���Ϣ�����������ݳ����Ƿ������Ϣͷ�ĳ���
	//��whileѭ���������ճ����
	while (m_lastPos >= sizeof(DataHead))
	{
		//ָ��m_MsgBuf��ָ�����ΪDataHead*���͵�ָ�룬
		//���ڷ���DataHead�����ݳ�Ա
		DataHead* pHead = reinterpret_cast<DataHead*>(m_MsgBuf);
		//�ж���Ϣ�����������ݳ����Ƿ������Ϣ����
		//������ٰ���������
		if (m_lastPos >= pHead->datalength)
		{
			//����������Ϣ
			OnNetMsg(pHead);

			/*
			//��m_MsgBuf���Ѿ����������Ϣ������������δ��������ݽ���
			//���ݸ���
			memcpy(m_MsgBuf,m_MsgBuf+pHead->datalength,
				m_lastPos-pHead->datalength);

			//����m_MsgBuf������β����λ��
			//����д����
			//m_lastPos -= pHead->datalength;
			//ԭ�����ݸ��Ǻ�pHeadָ������ݲ�����֮ǰ��δ����ǰ������
			//�ᵼ��m_lastPosָ����������β��
			*/

			//�ݴ��ʾ�Զ������Ϣ��������ʣ��δ��������ݵĳ��ȵı���
			int unprocessed = m_lastPos - pHead->datalength;
			//��m_MsgBuf���Ѿ����������Ϣ������������δ��������ݽ���
			//���ݸ���
			memcpy(m_MsgBuf, m_MsgBuf + pHead->datalength, unprocessed);

			//����m_MsgBuf������β����λ��
			m_lastPos = unprocessed;
		}
		else//ʣ�����ݲ���һ����������Ϣ
			break;
	}

	return 0;
}

void EasyTcpClient::OnNetMsg(DataHead* pHead)
{
	switch (pHead->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		LogInResult* loginresult = reinterpret_cast<LogInResult*>(pHead);

		//cout << "<socket=" << m_client_sock <<
		//	">�յ��������Ϣ��CMD_LOGIN_RESULT"
		//	<< " ���ݳ��ȣ�" << loginresult->datalength << endl;
	}
	break;

	case CMD_LOGOUT_RESULT:
	{
		LogOutResult* logoutresult = reinterpret_cast<LogOutResult*>(pHead);

		cout << "<socket=" << m_client_sock <<
			">�յ��������Ϣ��CMD_LOGOUT_RESULT"
			<< " ���ݳ��ȣ�" << logoutresult->datalength << endl;
	}
	break;

	case CMD_NEW_USER_JOIN:
	{
		//NewUserJoin* newuserjoin = reinterpret_cast<NewUserJoin*>(pHead);
		//cout << "\n<socket=" << m_client_sock <<
		//	">�յ��������Ϣ��CMD_NEW_USER_JOIN"
		//	<< " ���ݳ��ȣ�" << newuserjoin->datalength << endl;
	}
	break;

	case CMD_ERROR:
	{
		cout << "\n<socket=" << m_client_sock <<
			">�յ��������Ϣ��CMD_ERROR"
			<< " ���ݳ��ȣ�" << pHead->datalength << endl;
	}
	break;

	default:
	{
		cout << "\n<socket=" << m_client_sock <<
			">�յ�δ֪�������Ϣ" << endl;
	}
	}
}

int EasyTcpClient::SendData(DataHead* pHead, int Len)
{
	SOCKET res = SOCKET_ERROR;

	if (isRun() && pHead)
	{
		//return send(m_client_sock, (const char*)pHead, pHead->datalength, 0);

		res = send(m_client_sock, (const char*)pHead, Len, 0);

		if (res == SOCKET_ERROR)
		{
			Close();
			return SOCKET_ERROR;
		}
		else
			return res;
	}

	return SOCKET_ERROR;
}


