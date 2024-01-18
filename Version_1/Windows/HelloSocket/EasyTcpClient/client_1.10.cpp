#if 0
#include "EasyTcpClient_1.3.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread(void);

int main()
{
	//����EasyTcpClient��ռ���ڴ�ռ�ܴ�
	//���Էŵ����нϺ�
	//EasyTcpClient* client1=new EasyTcpClient ;

	//EasyTcpClient* ClientArray = new EasyTcpClient[FD_SETSIZE-1]();

	const int Num_clients = 1000;

	shared_ptr < EasyTcpClient >ClientArray
	(new EasyTcpClient[Num_clients], default_delete<EasyTcpClient[]>());

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).initSocket();
	}


#if 1
	const char ip[] = "127.0.0.1";
#elif 0
	//ͨ��cmd�鵽Linux�������IP��ַΪ192.168.175.132
	const char ip[] = "192.168.175.132";
#else
	//ͨ��cmd�鵽Mac�������IP��ַΪ192.168.175.133
	const char ip[] = "192.168.175.133";
#endif

	thread t1(cmdThread);//��ʽ��������  ����
	t1.detach();

	for (int i = 0; i < Num_clients; ++i)
	{
		if (!g_bRun)
			return 0;

		(ClientArray.get()[i]).Connect(ip, 9190);
	}

	LogIn login{};

#ifdef _WIN32
	strncpy_s(login.username, 32, "Luhao", 6);
	strncpy_s(login.password, 32, "123456", 7);
#else
	strncpy(login.username, "Luhao", 6);
	strncpy(login.password, "123456", 7);
#endif


	while (g_bRun /*|| client2.isRun()*/)
	{
		for (int i = 0; i < Num_clients; ++i)
		{
			//ClientArray.get()[i].OnRun();
			(ClientArray.get()[i]).SendData(&login);
		}
	}

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).Close();
	}

	cout << "�ͻ������˳������������\n";

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
