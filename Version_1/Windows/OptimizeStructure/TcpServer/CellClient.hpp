#pragma once

#include "Cell.hpp"

//客户端心跳检测死亡倒计时时间 (设置为60秒)
#define CLIENT_HEART_DEAD_TIME 60000

//指定时间后将发送缓冲区缓存的数据发送给客户端（设置为200毫秒）
#define SEND_BUFFER_REFRESH_TIME 200


//在服务端，由于要处理多个不同的客户端的数据
//因此每个客户端都应有其
//消息缓冲区还有指向消息缓冲区的数据尾部位置的变量
class ClientSocket
{
public:
	int m_id = -1;
	int m_CellServer_id = -1;

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

	//上次发送消息数据的时间
	time_t m_DTSend = 0;

	//发送缓冲区遇到写满的情况计数
	int m_SendBufFullCnt = 0;

public:
	ClientSocket(SOCKET sock = INVALID_SOCKET)
	{
		static int n = 1;
		m_id = n++;

		m_client_sock = sock;
		memset(m_MsgBuf, 0, sizeof(m_MsgBuf));
		memset(m_SendBuf, 0, sizeof(m_SendBuf));

		resetDTHeart();
		resetDTSend();
	}

	~ClientSocket()
	{
		printf("CellServer %d CellClient %d  ~ClientSocket()\n",m_CellServer_id,m_id);

		if (INVALID_SOCKET != m_client_sock)
		{
#ifdef _WIN32
			closesocket(m_client_sock);
#else
			close(m_client_sock);
#endif
			m_client_sock = INVALID_SOCKET;
		}
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

	//缓冲区的控制根据业务需求的差异而调整
	//定量发送（非阻塞模式下不使用）
	int SendData(DataHead* pHead)
	{
		int ret = SOCKET_ERROR;

		//存当前要发送的数据长度
		int SendLen = pHead->datalength;
		//当前要发送的数据
		const char* pSendData = reinterpret_cast<const char*>(pHead);

		if (m_lastSendPos + SendLen <= SEND_BUFFER_SIZE)
		{
			//将要发送的数据拷贝到发送缓冲区尾部
			memcpy(m_SendBuf + m_lastSendPos, pSendData, SendLen);

			//更新数据尾部位置
			m_lastSendPos += SendLen;

			//如果数据总长度正好为缓冲区能存储的最大长度
			if (m_lastSendPos== SEND_BUFFER_SIZE)
			{
				++m_SendBufFullCnt;
			}

			return SendLen;
		}
		else//如果缓冲区放不下这条数据了
		{
			++m_SendBufFullCnt;
		}

		//非阻塞发送数据，不再使用定时发送
		//发送完重置定时发送的计时时间
		//resetDTSend();//其实可以注释掉

		//***注***
		//SendData()函数不再实现发送数据的业务，
		//而是将该业务转交给select机制配合WriteData()实现

		return ret;
	}

	void resetDTHeart()
	{
		m_DTHeart = 0;
	}

	void resetDTSend()
	{
		m_DTSend = 0;
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

	//定时发送数据检测（非阻塞模式下不使用）
	bool IsSend(time_t dt)
	{
		m_DTSend += dt;

		if (m_DTSend >= SEND_BUFFER_REFRESH_TIME)
		{
			//printf("timed transmission : sock=%d, time=%d\n",m_client_sock, m_DTSend);

			//立即将发送缓冲区的数据发送出去
			SendDataImmediately();
			//发送完重置定时发送的计时时间
			resetDTSend();

			return true;
		}

		return false;
	}

	//立即将发送缓冲区的数据发送给客户端
	int SendDataImmediately()
	{
		int ret = 0;

		if (m_lastSendPos > 0 &&INVALID_SOCKET!=m_client_sock)
		{
			ret = send(m_client_sock, (const char*)m_SendBuf,
				m_lastSendPos, 0);

			//发送完后，标记发送缓冲区中已有的数据量为0
			m_lastSendPos = 0;

			//重置写满情况计数
			m_SendBufFullCnt = 0;

			//发送完重置定时发送的计时时间
			resetDTSend();
		}

		return ret;
	}

	//在使用select机制实现非阻塞式地发送数据，以下方法就不适用了
	//提供给外部的立即发送数据给客户端的方法
	//void SendDataToClientImmediately(DataHead* pHead)
	//{
	//	SendData(pHead);
	//	SendDataImmediately();
	//}
};
