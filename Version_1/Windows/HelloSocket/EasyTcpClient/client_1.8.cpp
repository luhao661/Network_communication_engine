#if 1
#include "EasyTcpClient_1.2.hpp"
#include <thread>
bool g_bRun = true;
void cmdThread(void);

int main()
{
	//����EasyTcpClient��ռ���ڴ�ռ�ܴ�
	//���Էŵ����нϺ�
	//EasyTcpClient* client1=new EasyTcpClient ;

	//EasyTcpClient* ClientArray = new EasyTcpClient[FD_SETSIZE-1]();

	const int Num_clients = FD_SETSIZE - 1;
	
	shared_ptr < EasyTcpClient >ClientArray  
		(new EasyTcpClient[Num_clients],default_delete<EasyTcpClient[]>());

	//client1->initSocket();
	//client2.initSocket();

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
	//ע��������򿪺���IP��ַ���ܻ�ı�

	//���������ͻ���socket������������������
	//client1->Connect(ip, 9190);
	//client2.Connect(ip,9191);

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).Connect(ip,9190);
	}

	// 3 ����������������̣߳�
	//�����߳�
	//***ע***
	//Mac�˲�֧�ִ�����
	//thread t1(cmdThread, &client1);//��ʽ��������  ����
	//�� thread �������ִ���̣߳�����ִ���Ҷ����س�����
	//һ�����߳��˳������ͷ��κη������Դ��

	//������ detach() ����������ζ�ŷ����˶��̵߳Ŀ���������
	//�˺����̺߳ͷ�����߳�֮�佫�໥�������У����̲߳��ٵȴ�������߳�ִ����ɡ�
	//���̲߳�����Ҫ���ĸ��̵߳�״̬�����ʱ����
	//�磺�ڷ���󣬲����ܹ��� t ���� t1.join()
	//t1.detach();

	//thread t2(cmdThread, &client2);
	//t2.detach();
	thread t1(cmdThread);//��ʽ��������  ����
	t1.detach();

	//����ʹ��t1.detach();
	//���̵߳�whileѭ����g_bRun����Ϊ�ж���ֹ������
	//��t1�߳���ֹʱ�����߳�Ҳ������whileѭ������ֹ��
	//���̱߳�t1�߳�����ֹ���ᵼ�³������

	//ʹ�� detach() �����߳��봴�����̷߳����
	//���߳̽���ʱ����ֱ��Ӱ�쵽�ѷ�����̡߳�
	//���̵߳Ľ������ᵼ���ѷ����̵߳���ǰ��ֹ���ѷ�����߳̽������ں�̨�������С�

	LogIn login{};

#ifdef _WIN32
	strncpy_s(login.username, 20, "Luhao", 6);
	strncpy_s(login.password, 32, "123456", 7);
#else
	strncpy(login.username, "Luhao", 6);
	strncpy(login.password, "123456", 7);
#endif


	while (g_bRun /*|| client2.isRun()*/)
	{
		for (int i = 0; i < Num_clients; ++i)
		{
			ClientArray.get()[i].OnRun();
			(ClientArray.get()[i]).SendData(&login);
		}
	}

	//client1->Close();
	//client2.Close();

	for (int i = 0; i < Num_clients; ++i)
	{
		(ClientArray.get()[i]).Close();
	}

	//delete client1;

	//��ֹ��EasyTcpClient.exe��һ������
	//getchar();
	cin.get();
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

/*
//�����߳�
void cmdThread(EasyTcpClient* client)
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
			//g_bRun = false;

			client->Close();

			break;
		}
		else if (!strcmp(cmdBuf, "login"))
		{
			//***����д��***
			//û����֮ƥ��Ĺ��캯��
			//LogIn login{ "Luhao","123456" };
			//��ΪLogIn�������������ˣ������Ǽ򵥵ĵ������࣬���������ôд

			LogIn login{};

#ifdef _WIN32
			strncpy_s(login.username, 20, "Luhao", 6);
			strncpy_s(login.password, 32, "123456", 7);
#else
			strncpy(login.username, "Luhao", 6);
			strncpy(login.password, "123456", 7);
#endif
			// ���ͱ���
			//***���˴������������һ�������***
			//�˴���client_sock��Ӧ���ǿͻ��˴������������ӷ��������׽��֡�
			//���ԣ����д������ڿͻ��˷�����Ϣ cmdBuf �������ӵķ�������
			//�����ڿͻ��ˣ�client_sock �� ָ�� ���������׽���

			//send(client.m_client_sock, (char*)&login, sizeof(LogIn), 0);
			//����ֱ��д��
			client->SendData(&login);
			//***ע***
			//�˴�������������ʽ����ת��
		}
		else if (!strcmp(cmdBuf, "logout"))
		{
			//LogOut logout{ "Luhao" };

			LogOut logout{};

#ifdef _WIN32
			strncpy_s(logout.username, 20, "Luhao", 6);
#else
			strncpy(logout.username, "Luhao", 6);
#endif

			//***���˴������������һ�������***
			//�˴���client_sock��Ӧ���ǿͻ��˴������������ӷ��������׽��֡�
			//���ԣ����д������ڿͻ��˷�����Ϣ CmdMsg �������ӵķ�������
			//�����ڿͻ��ˣ�client_sock �� ָ�� ���������׽���

			client->SendData(&logout);
		}
		else
			cout << "δʶ���������������룡\n";

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
*/
#endif
