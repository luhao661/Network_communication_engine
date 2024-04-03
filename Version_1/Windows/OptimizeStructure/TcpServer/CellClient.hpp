#pragma once

#include "Cell.hpp"

//客户端心跳检测死亡倒计时时间
#define CLIENT_HEART_DEAD_TIME 5000

//在服务端，由于要处理多个不同的客户端的数据
//因此每个客户端都应有其
//消息缓冲区还有指向消息缓冲区的数据尾部位置的变量
class ClientSocket
{
private:
	SOCKET m_client_sock;

	//自定义的接收缓冲区
	//char m_Recv[RECV_BUFFER_SIZE] = {};
	//不需要自定义的接收缓冲区，因为CellServer::RecvData()中recv()
	//每次能及时取数据放入消息缓冲区，接收缓冲区数据能得到及时清空
	//CellServer::RecvData()每次处理某个客户端的数据，
	//并会及时调用CellServer::OnNetMsg()，打印完该客户端发送的消息
	//每个客户端套接字句柄会唯一对应一个ClientSocket类指针，
	//相当于有自己的m_MsgBuf，select() 多路 I/O 作用下不会造成数据混乱

	//第二缓冲区 消息缓冲区
	char m_MsgBuf[RECV_BUFFER_SIZE] = {};
	//指向消息缓冲区的数据尾部位置
	int m_lastPos = 0;

	//第二缓冲区 发送缓冲区
	char m_SendBuf[SEND_BUFFER_SIZE] = {};
	//指向发送缓冲区的数据尾部位置
	int m_lastSendPos = 0;

	//心跳死亡计时
	time_t m_DTHeart=0;

public:
	ClientSocket(SOCKET sock = INVALID_SOCKET)
	{
		m_client_sock = sock;
		memset(m_MsgBuf, 0, sizeof(m_MsgBuf));

		memset(m_SendBuf, 0, sizeof(m_SendBuf));
	}

	SOCKET Get_m_client_sock()
	{
		return m_client_sock;
	}

	//char* Get_m_Recv();

	char* Get_m_MsgBuf()
	{
		return m_MsgBuf;
	}

	int Get_m_lastPos()
	{
		return m_lastPos;
	}

	void Set_m_lastPos(int NewPos)
	{
		m_lastPos = NewPos;
	}

	int SendData(DataHead* pHead)
	{
		int ret = SOCKET_ERROR;

		//存当前要发送的数据长度
		int NowSendLen = pHead->datalength;
		//当前要发送的数据
		const char* pSendData = reinterpret_cast<const char*>(pHead);

		while (true)
		{
			//若发送缓冲区中已有的数据量加上当前要发送数据的量大于等于SEND_BUFFER_SIZE
			if (m_lastSendPos + NowSendLen >= SEND_BUFFER_SIZE)
			{
				//计算可拷贝的数据长度
				int CopyLen = SEND_BUFFER_SIZE - m_lastSendPos;

				//拷贝数据
				memcpy(m_SendBuf + m_lastSendPos, pSendData, CopyLen);

				//当前要发送数据的剩余数据的起始位置
				pSendData += CopyLen;
				//当前要发送数据的剩余数据的长度
				NowSendLen -= CopyLen;

				ret = send(m_client_sock, (const char*)m_SendBuf,
					SEND_BUFFER_SIZE, 0);

				//发送完后，标记发送缓冲区中已有的数据量为0
				m_lastSendPos = 0;

				//如果发送错误
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else//若发送缓冲区中已有的数据量加上当前要发送数据的量小于SEND_BUFFER_SIZE
			{
				//将当前要发送数据拷贝到发送缓冲区数组的标记的尾部
				memcpy(m_SendBuf + m_lastSendPos, pSendData, NowSendLen);

				m_lastSendPos += NowSendLen;

				break;
			}
		}

		return ret;
	}

	void resetDTHeart()
	{
		m_DTHeart = 0;
	}

	//心跳检测
	bool IsDead(time_t dt)
	{
		m_DTHeart += dt;

		if (m_DTHeart >= CLIENT_HEART_DEAD_TIME)
		{
			printf("Heart dead : sock=%d, time=%d\n",m_client_sock,m_DTHeart);

			return true;
		}

		return false;
	}
};
