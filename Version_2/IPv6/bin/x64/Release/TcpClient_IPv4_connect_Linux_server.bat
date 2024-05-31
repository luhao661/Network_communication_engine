@echo off

::服务端IP地址
set cmd="strIP=192.168.175.132"

::服务端端口
set cmd=%cmd% nPort=4567

::工作线程数量
set cmd=%cmd% nThread=4

::创建多少个客户端
set cmd=%cmd% nClient=1000

::等待socket可写时才实际发送
::每个客户端在nSendsleep(亳秒)时间内
::最大可写入nMsg条Login消息
::每条消息100字节(Login)
set cmd=%cmd% nMsg=100
set cmd=%cmd% nSendSleep=1000

::客户端发送缓冲区大小(字节)
set cmd=%cmd% nSendBuffSize=10240

::客户端接收缓冲区大小(字节)
set cmd=%cmd% nRecvBuffSize=10240

::检查接收到的服务端消息ID是否连续
set cmd=%cmd% -checkMsgID

::检测-发送的消息已被服务器回应
::收到服务器回应后才发送下一条消息
set cmd=%cmd% -chekSendBack
set cmd=%cmd% -ipv4

Client %cmd%

pause