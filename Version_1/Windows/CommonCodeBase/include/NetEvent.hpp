#pragma once

#include "CellClient.hpp"

class CellServer;

//网络事件接口
//CellServer类的vec_client的元素数量减少时，通知EasyTcpServer类
//NetEvent类可以看成是个【代理类】，代理了EasyTcpServer类对象
class NetEvent
{
private:

public:
	//客户端加入事件
	virtual void NEOnNetJoin(ClientSocket* pClient) = 0;//纯虚函数

	//客户端离开事件
	virtual void NEOnNetLeave(ClientSocket* pClient) = 0;
 
	virtual void NEOnNetMsg(CellServer* pCellServer, ClientSocket* client_sock, DataHead* pHead) = 0;

	//recv事件
	virtual void NERecv(ClientSocket* client_sock) = 0;
};
