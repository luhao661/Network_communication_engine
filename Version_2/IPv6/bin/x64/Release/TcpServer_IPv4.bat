@echo off

::�����IP��ַ
set cmd="strIP=any"

::����˶˿�
set cmd=%cmd% nPort=4567

::�����߳�����
set cmd=%cmd% nThread=4

::�ͻ�����������
set cmd=%cmd% nMaxClient=10240

::���ͻ�������С(�ֽ�)
set cmd=%cmd% nSendBuffSize=81920

::���ջ�������С(�ֽ�)
set cmd=%cmd% nRecvBuffSize=81920

::�յ���Ϣ�󽫷���Ӧ����Ϣ
set cmd=%cmd% -sendback

::��ʾ���ͻ���������
set cmd=%cmd% -sendfull

::�����յ��ķ������ϢID�Ƿ�����
set cmd=%cmd% -checkMsgID

Server %cmd%

::��ֹ cmd ���ڹر�
pause