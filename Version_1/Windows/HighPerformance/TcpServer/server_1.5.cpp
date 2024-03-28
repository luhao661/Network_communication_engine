#if 1

#include "Allocator.h"
#include "TcpServer_1.6.hpp"
bool g_bRun = true;
void cmdThread(void);

class MyServer :public EasyTcpServer
{
public:
	virtual void NEOnNetJoin(ClientSocketPtr& pClient)
	{
		//������+������������������ʽ�����û����ͬ������
		EasyTcpServer::NEOnNetJoin(pClient);
	}

	virtual void NEOnNetLeave(ClientSocketPtr& pClient)
	{
		EasyTcpServer::NEOnNetLeave(pClient);
		cout << "Clinet<" << pClient->Get_m_client_sock() << "> �˳�\n";
	}

	virtual void NEOnNetMsg(CellServer* pCellServer, ClientSocketPtr& pclient_sock, DataHead* pHead)
	{
		EasyTcpServer::NEOnNetMsg(pCellServer, pclient_sock, pHead);

		//  ��������
		switch (pHead->cmd)
		{
		case CMD_LOGIN:
		{
			LogIn* login = reinterpret_cast<LogIn*>(pHead);

			//�����ж��û������Ƿ���ȷ�Ĺ���

			//***ע***
			//д���´��룬�ᵼ�·���˺Ϳͻ����ڽ���ͨ�ź󲻾þͲ�����
#if 0
			cout << "�յ��ͻ���<socket=" << pclient_sock->Get_m_client_sock() << ">���CMD_LOGIN"
				<< " ���ݳ��ȣ�" << login->datalength << endl;
			cout << "�û�����" << login->username << "����" << endl;
#endif
			// ���ͱ���
			//LogInResult* res = new LogInResult{};
			//pclient_sock->SendData(res);

			LogInResultPtr res = make_shared<LogInResult>();
			pCellServer->addSendTask(pclient_sock, res);
		}
		break;

		/*
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
		*/
		default:
		{
			cout << "�յ��ͻ���<socket=" << pclient_sock->Get_m_client_sock() << ">δ������Ϣ"
				<< " ���ݳ��ȣ�" << pHead->datalength << endl;

			//���ͺ���CMD_ERROR�����ݰ�
			DataHeadPtr head=make_shared<DataHead>();
			pclient_sock->SendData(head);
		}
		}

	}

	virtual void NERecv(ClientSocketPtr& client_sock)
	{
		EasyTcpServer::NERecv(client_sock);
	}
};


int main()
{
	MyServer server;

	server.initSocket();
	server.Bind(nullptr, 9190);
	server.Listen(5);
	//server.Accept();	   //��OnRun()���Ѿ�����Accept()�������ܴ����ͻ��˵�ͨ������

	//�������߳�
	server.StartThread(4);

	thread t1(cmdThread);//��ʽ��������  ����
	t1.detach();

	while (server.isRun() && g_bRun)
	{
		server.OnRun();
	}

	server.Close();

	cout << "�����������˳������������\n";
	cin.get();

	return 0;
}
void cmdThread(void)
{
	char cmdBuf[256]{};

	//cin �� scanf ��������ʽ������
	// ��main()�е�select()����������������������߼���ͻ������Ҫʹ�ö��߳�

	while (1)
	{
		cin.getline(cmdBuf, 256);

		// ��������
		if (!strcmp(cmdBuf, "exit"))
		{
			cout << "�յ��˳�����˳�cmdThread�߳�\n";
			g_bRun = false;
			break;
		}
		else
			cout << "δʶ���������������룡\n";
	}
}
#endif