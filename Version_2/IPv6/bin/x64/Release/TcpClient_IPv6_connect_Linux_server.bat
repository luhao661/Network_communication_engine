@echo off

::�����IP��ַ
set cmd="strIP=fe80::7e8b:f9e:ecf3:5d86"

::���÷�Χ��ʶ
::set cmd=%cmd% "scope_id_name=ens33"

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
set cmd=%cmd% nMsg=1
set cmd=%cmd% nSendSleep=10

::�ͻ��˷��ͻ�������С(�ֽ�)
set cmd=%cmd% nSendBuffSize=10240

::�ͻ��˽��ջ�������С(�ֽ�)
set cmd=%cmd% nRecvBuffSize=10240

::�����յ��ķ������ϢID�Ƿ�����
set cmd=%cmd% -checkMsgID

::���-���͵���Ϣ�ѱ���������Ӧ
::�յ���������Ӧ��ŷ�����һ����Ϣ
set cmd=%cmd% -chekSendBack

set cmd=%cmd% -ipv6

Client %cmd%

pause