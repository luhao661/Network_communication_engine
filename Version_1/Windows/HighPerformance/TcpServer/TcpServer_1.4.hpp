#pragma once

#ifdef _WIN32
//���windows.h��winsock2.h�º궨���ͻ
#define WIN32_LEAN_AND_MEAN
//ʹinet_ntoa()����
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define FD_SETSIZE 1024
//������⣺
//���֧�ֵĿͻ�����������ΪFD_SETSIZE-1
//***ע***
//���֧�ֵĿͻ�������Ϊ 
//CELLSERVER_THREADS_COUNT * (FD_SETSIZE)
//��ΪCellServer����OnRun()�е�FD_SET()ע���ȫ�ǿͻ���

//������߳�����
//#define CELLSERVER_THREADS_COUNT 4

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

#include "MessageHeader_1.0.hpp"//��Ϊ100�ֽڵ����ݰ�
#include "Timestamp.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>//mem_fn
#include <atomic>//ԭ�Ӳ���
#include <map>

using namespace std;

#ifndef RECV_BUFFER_SIZE

#define RECV_BUFFER_SIZE 1024*10
#define SEND_BUFFER_SIZE 1024*10

#endif

//�ڷ���ˣ�����Ҫ��������ͬ�Ŀͻ��˵�����
//���ÿ���ͻ��˶�Ӧ����
//��Ϣ����������ָ����Ϣ������������β��λ�õı���
class ClientSocket
{
private:
	SOCKET m_client_sock;

	//�Զ���Ľ��ջ�����
	//char m_Recv[RECV_BUFFER_SIZE] = {};
	//����Ҫ�Զ���Ľ��ջ���������ΪCellServer::RecvData()��recv()
	//ÿ���ܼ�ʱȡ���ݷ�����Ϣ�����������ջ����������ܵõ���ʱ���
	//CellServer::RecvData()ÿ�δ���ĳ���ͻ��˵����ݣ�
	//���ἰʱ����CellServer::OnNetMsg()����ӡ��ÿͻ��˷��͵���Ϣ
	//ÿ���ͻ����׽��־����Ψһ��Ӧһ��ClientSocket��ָ�룬
	//�൱�����Լ���m_MsgBuf��select() ��· I/O �����²���������ݻ���

	//�ڶ������� ��Ϣ������
	char m_MsgBuf[RECV_BUFFER_SIZE] = {};
	//ָ����Ϣ������������β��λ��
	int m_lastPos = 0;

	//�ڶ������� ���ͻ�����
	char m_SendBuf[SEND_BUFFER_SIZE] = {};
	//ָ���ͻ�����������β��λ��
	int m_lastSendPos = 0;

public:
	ClientSocket(SOCKET m_client_sock = INVALID_SOCKET);

	SOCKET Get_m_client_sock();

	//char* Get_m_Recv();

	char* Get_m_MsgBuf();

	int Get_m_lastPos();

	void Set_m_lastPos(int NewPos);

	int SendData(DataHead* pHead);
};


//�����¼��ӿ�
//CellServer���vec_client��Ԫ����������ʱ��֪ͨEasyTcpServer��
//NetEvent����Կ����Ǹ��������ࡿ��������EasyTcpServer�����
class NetEvent
{
private:

public:
	//�ͻ��˼����¼�
	virtual void NEOnNetJoin(ClientSocket* pClient) = 0;//���麯��

	//�ͻ����뿪�¼�
	virtual void NEOnNetLeave(ClientSocket* pClient) = 0;

	virtual void NEOnNetMsg(ClientSocket* client_sock, DataHead* pHead) = 0;

	//recv�¼�
	virtual void NERecv(ClientSocket* client_sock) = 0;
};


//���ã�
//�߳�����¿ͻ��ˡ�������ͻ����С���
//����������ͻ������ڵ��¿ͻ����롾��ʽ�ͻ����С�
//�����������Ƿ���ڴ���ȡ���ݡ�
//�������ݡ���Ӧ������Ϣ
class CellServer
{
private:
	SOCKET m_serv_sock;
	//�Զ���Ľ��ջ�����
	char m_Recv[RECV_BUFFER_SIZE] = {};
	//��ʽ�ͻ�����
	//vector<ClientSocket*> vec_client;
	//��ʽ�ͻ�����
	map<SOCKET, ClientSocket*> sock_pclient_pair;

	//����ͻ�����
	vector<ClientSocket*> vec_client_buffer;
	//���������
	mutex m_mutex;
	//�߳�ָ��
	thread* m_pThread;
	//ָ�������¼������ָ��
	//ָ���������ָ��
	//��ָ��Ķ�����EasyTcpServer::StartThread()�����д�����
	// �����Ǹ���������󣬼�ָ��ָ�����������
	NetEvent* m_pNetEvent;

	//���� fdRead �пͻ���socket
	fd_set m_fdReadBackUp;

	bool m_ClientsChange = false;

	SOCKET m_maxSocket;

public:
	//ÿ������CellServer::OnNetMsg()��m_cnt����1
	//��EasyTcpServer�������ݳ�ԱrecvCntʹ�ã�
	//������ÿ��һ����̵߳�m_cnt��ֵ����ʹ�ú���m_cntΪ0
	//atomic_int m_cnt;

	CellServer(SOCKET serv_sock = INVALID_SOCKET)
	{
		m_serv_sock = serv_sock;
		m_pThread = nullptr;
		//m_cnt = 0;
		m_pNetEvent = nullptr;
	}

	//***ע***
	//�˴���ʹ������������
	//��Ϊ���಻��Ϊ���࣬û��Ҫʹ������������
	//����һ�������麯�����ͻ�����ָ�������ڲ�ͬƽ̨�����ݽṹ�ᷢ���仯
	~CellServer()
	{
		delete m_pThread;
		Close();
		m_serv_sock = INVALID_SOCKET;
	}

	void Close(void);

	//��EasyTcpServer::StartThread()����
	void Start();

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
	void OnNetMsg(ClientSocket* pclient_sock, DataHead* pHead);

	//��ʽ�ĺ��ڻ������еĿͻ�������
	//***ע***
	//��EasyTcpServer::addClientToCellServer()�ڵ���
	size_t getClientCnt();

	//��EasyTcpServer::addClientToCellServer()�ڵ���
	void addClient(ClientSocket* pClient)
	{
		//Ҳ����д�����Զ���
		//lock_guard<mutex>lg(mutex);

		m_mutex.lock();
		vec_client_buffer.push_back(pClient);
		m_mutex.unlock();
	}

	//����m_pNetEventָ��ָ��EasyTcpServer�����
	void setEventObj(NetEvent* event)
	{
		m_pNetEvent = event;
	}
};


//���ã�
//�����socket�Ĵ����������׽��ֵ�ַ������ȴ���������״̬
//����������̲߳������߳�(serv->Start())��
//����ͳ�Ʋ���ʾ�����߳�ÿ�봦��������������ȴ��ͻ��˵���������
//ȷ���µĿͻ��˱���ӵ���ǰ�ͻ����������ٵ��̷߳�����ϣ�
//�洢Ϊ������ͻ����С�
class EasyTcpServer : public NetEvent
{
private:
	SOCKET m_serv_sock;
	//vector<ClientSocket*> vec_client;
	vector<CellServer*> CellServers;

	Timestamp m_time;

	//�Զ���Ľ��ջ�����
	char m_Recv[RECV_BUFFER_SIZE] = {};

protected:
	//�յ���Ϣ����
	atomic_int m_MsgCnt;
	//�ͻ��˸�������
	atomic_int m_clients_cnt;
	//RecvData()���д�������
	atomic_int m_RecvCnt;

public:
	EasyTcpServer();

	virtual ~EasyTcpServer();

	SOCKET initSocket();

	void Close();

	int Bind(const char* ip, unsigned short port);

	int Listen(int backlog);

	//����������߳�
	void StartThread(int CellServerThreadsCnt)
	{
		for (int n = 0; n < CellServerThreadsCnt; ++n)
		{
			auto serv = new CellServer(m_serv_sock);
			CellServers.push_back(serv);

			//ע�������¼�
			//ʹCellServer��m_pNetEventָ��EasyTcpServer�����
			serv->setEventObj(this);
			//***ע***
			//�������ڼ̳���NetEvent�࣬setEventObj()���β�NetEvent*
			//�ſɱ���ֵΪEasyTcpServer�����ĵ�ַ������ǿ��ת����

			//������Ϣ�����߳�
			serv->Start();
		}
	}

	//�Ƿ�������������
	bool isRun()
	{
		return m_serv_sock != INVALID_SOCKET;
	}

	//ͳ�������߳�ÿ�봦�������
	void time4msg();

	//������������
	//��ѯ�Ƿ����¿ͻ�����������
	bool OnRun();

	//������addClientToCellServer()
	SOCKET Accept();

	//ȷ���µĿͻ��˱���ӵ���ǰ�ͻ����������ٵ�CellServer�ϣ�
	//�Է�ɢ����������
	void addClientToCellServer(ClientSocket* pClient);

	virtual void NEOnNetJoin(ClientSocket* pClient);

	virtual void NEOnNetLeave(ClientSocket* pClient);

	virtual void NEOnNetMsg(ClientSocket* pclient_sock, DataHead* pHead);

	virtual void NERecv(ClientSocket* client_sock);

	//��������
	//����ճ�� ��ְ�
	//int RecvData(ClientSocket* pClientSocket);

	//��Ӧ������Ϣ
	//***ע***
	//����Ϊvirtual����֮����౻�̳У�Ҫ��д��Ӧ������Ϣ����ʱ��
	//OnNetMsg()�����ɻ�ö�̬��
	//void OnNetMsg(SOCKET client_sock, DataHead* pHead);

	//��������
	//int SendData(SOCKET client_sock, DataHead* pHead);

	//void SendDataToAll(DataHead* pHead);
};








