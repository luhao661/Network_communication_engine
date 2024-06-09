@echo off

::服务端IP地址
set cmd="strIP=any"

::服务端端口
set cmd=%cmd% nPort=4567

::工作线程数量
set cmd=%cmd% nThread=4

::客户端连接上限
set cmd=%cmd% nMaxClient=10240

::发送缓冲区大小(字节)
set cmd=%cmd% nSendBuffSize=81920

::接收缓冲区大小(字节)
set cmd=%cmd% nRecvBuffSize=81920

::收到消息后将返回应答消息
set cmd=%cmd% -sendback

::提示发送缓冲区已满
set cmd=%cmd% -sendfull

::检查接收到的服务端消息ID是否连续
set cmd=%cmd% -checkMsgID
 
::使用IPv6
set cmd=%cmd% -ipv6

Server %cmd%

::防止 cmd 窗口关闭
pause
