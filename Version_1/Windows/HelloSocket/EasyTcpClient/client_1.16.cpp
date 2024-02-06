#if 1
#include "EasyTcpClient_1.6.hpp"
#include <thread>
#include <vector>
bool g_bRun = true;
void cmdThread(void);
void SendThread(int id);

//ȫ�ֱ�����������ݶε�ȫ�ֱ�������

//�����̵߳Ŀͻ�������
const int Num_clients = 8;
//�����߳�����
const int T_cnt = 4;
//�ͻ�������
EasyTcpClient* pClientArray[Num_clients];

int main()
{
	//����UI�߳�
	thread uiThread(cmdThread);//��ʽ��������  ����
	uiThread.detach();

	//�����߳�
	vector<thread> threads;
	for (int i = 0; i < T_cnt; ++i)
	{
		threads.emplace_back(SendThread, i + 1);
	}

	while (g_bRun)
	{
		Sleep(100);
	}

	cout << "�ͻ������˳������������\n";

	return 0;
}

void cmdThread(void)
{
	char cmdBuf[256]{};

	//cin �� scanf ��������ʽ������
	// ��select()����������������������߼���ͻ������Ҫʹ�ö��߳�

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

//һ�ִ��������̺߳����ڶ�̬����洢�ռ����洢�����Ŀͻ���
//�����̺߳����ڵ�ָ��������
//ȱ���������߳��޷�ȡ�ÿͻ�����Դ���޷����ʿͻ���
#if 0
void SendThread(int id)
{
	//����ÿ���߳�Ҫ�е������Ŀͻ�������
	int clients_per_thread = Num_clients / T_cnt;

	shared_ptr < EasyTcpClient >ClientArray
	(new EasyTcpClient[clients_per_thread], default_delete<EasyTcpClient[]>());

	for (int i = 0; i < clients_per_thread; ++i)
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

	for (int i = 0; i < clients_per_thread; ++i)
	{
		if (!g_bRun)
			return;

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

	while (g_bRun)
	{
		for (int i = 0; i < clients_per_thread; ++i)
		{
			//ClientArray.get()[i].OnRun();
			(ClientArray.get()[i]).SendData(&login);
		}
	}

	for (int i = 0; i < clients_per_thread; ++i)
	{
		(ClientArray.get()[i]).Close();
	}

	return;
}
#endif


//��һ�ִ����ǽ�Ҫ�����Ŀͻ�����ȫ�ִ���������������
//�����ͻ�����Դ������Ϊ������Դ���������̷߳���
#if 1
void SendThread(int id)
{
	printf("�߳�<%d>��ʼ\n", id);

	//����ÿ���߳�Ҫ�е������Ŀͻ�������
	int clients_per_thread = Num_clients / T_cnt;

	//����ÿ���̶߳�̬�����Ŀͻ��˶�Ӧ�������
	//ָ����pClientArray�����еķֲ�
	int theBegin = (id - 1) * clients_per_thread;
	int theEnd = id * clients_per_thread;

	//shared_ptr < EasyTcpClient >ClientArray
	//(new EasyTcpClient[clients_per_thread], default_delete<EasyTcpClient[]>());

	for (int i = theBegin; i < theEnd; ++i)
	{
		pClientArray[i] = new EasyTcpClient();
	}

	for (int i = theBegin; i < theEnd; ++i)
	{
		pClientArray[i]->initSocket();
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

	for (int i = theBegin; i < theEnd; ++i)
	{
		if (!g_bRun)
			return;

		pClientArray[i]->Connect(ip, 9190);

		//printf("�̺߳�<%d>, �ͻ��˱��=%d ���ӳɹ�...\n",
		//	id, i);
	}

	printf("�̺߳�<%d>, �ͻ��˱��<begin=%d,  end=%d>���ӳɹ�...\n",
		id, theBegin, theEnd);

	//ʹ�ñ�׼���ṩ������
	chrono::seconds t(8);
	this_thread::sleep_for(t);
	//���пͻ��˶�������ɺ���һ������Ϣ

	const int n = 10;
	//LogIn login[10];
	LogIn login[n];

	for (int i = 0; i < n; ++i)
	{
#ifdef _WIN32
		strncpy_s(login[i].username, 32, "Luhao", 6);
		strncpy_s(login[i].password, 32, "123456", 7);
#else
		strncpy(login.username, "Luhao", 6);
		strncpy(login.password, "123456", 7);
#endif
	}

	int nLen = sizeof(login);

	while (g_bRun)
	{
		for (int i = theBegin; i < theEnd; ++i)
		{
			pClientArray[i]->OnRun();
			pClientArray[i]->SendData(login, nLen);
		}
	}

	for (int i = theBegin; i < theEnd; ++i)
	{
		pClientArray[i]->Close();
		delete pClientArray[i];
	}

	printf("�߳�<%d>�˳�\n", id);

	return;
}
#endif

#endif
