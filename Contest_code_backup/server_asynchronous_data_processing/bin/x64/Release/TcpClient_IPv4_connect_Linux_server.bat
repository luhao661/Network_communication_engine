@echo off

::�����IP��ַ
set cmd="strIP=192.168.175.132"

::����˶˿�
set cmd=%cmd% nPort=4567

::�����߳�����
set cmd=%cmd% nThread=4

::�������ٸ��ͻ���
set cmd=%cmd% nClient=1000

::�ȴ�socket��дʱ��ʵ�ʷ���
::ÿ���ͻ�����nSendsleep(����)ʱ����
::����д��nMsg��Login��Ϣ
::ÿ����Ϣ100�ֽ�(Login)
set cmd=%cmd% nMsg=100
set cmd=%cmd% nSendSleep=1000

::�ͻ��˷��ͻ�������С(�ֽ�)
set cmd=%cmd% nSendBuffSize=10240

::�ͻ��˽��ջ�������С(�ֽ�)
set cmd=%cmd% nRecvBuffSize=10240

::�����յ��ķ������ϢID�Ƿ�����
set cmd=%cmd% -checkMsgID

::���-���͵���Ϣ�ѱ���������Ӧ
::�յ���������Ӧ��ŷ�����һ����Ϣ
set cmd=%cmd% -chekSendBack
set cmd=%cmd% -ipv4

Client %cmd%

pause