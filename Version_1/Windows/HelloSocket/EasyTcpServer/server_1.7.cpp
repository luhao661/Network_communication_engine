#if 1
#include "EasyTcpServer_1.1.hpp"

int main()
{
	EasyTcpServer server;

	server.initSocket();
	server.Bind(nullptr, 9190);
	server.Listen(5);
	//server.Accept();	   //��OnRun()���Ѿ�����Accept()�������ܴ����ͻ��˵�ͨ������

	while (server.isRun())
	{
		server.OnRun();
	}

	server.Close();

	cout << "�����������˳������������\n";
	cin.get();

	return 0;
}
#endif