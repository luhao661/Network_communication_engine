#if 0
#include "EasyTcpServer_1.3.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread(void);

int main()
{
	EasyTcpServer server;

	server.initSocket();
	server.Bind(nullptr, 9190);
	server.Listen(5);
	server.Accept();	   //��OnRun()���Ѿ�����Accept()�������ܴ����ͻ��˵�ͨ������

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