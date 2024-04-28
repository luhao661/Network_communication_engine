#pragma

#include "Cell.hpp"

class CellBuffer
{
private:

	//第二缓冲区
	char* m_pBuf = nullptr;//***注***用于创建动态的缓冲区，更便于内存管理
	//指向缓冲区的数据尾部位置，已有数据的长度
	int m_lastPos = 0;
	//缓冲区总的空间大小
	int m_BufferSize=0;
	//缓冲区写满次数计数
	int m_BufFullCnt = 0;

public:

	CellBuffer(int nsize=8192)
	{
		m_BufferSize = nsize;
		m_pBuf = new char[m_BufferSize];
	}

	~CellBuffer()
	{
		delete[] m_pBuf;
		m_pBuf = nullptr;
	}

	bool AddMsgToBuf(const char* pData,int Len)
	{
		if (m_lastPos + Len <= m_BufferSize)
		{
			//将要发送的数据拷贝到发送缓冲区尾部
			memcpy(m_pBuf + m_lastPos, pData, Len);

			//更新数据尾部位置
			m_lastPos += Len;

			//如果数据总长度正好为缓冲区能存储的最大长度
			if (m_lastPos == m_BufferSize)
			{
				++m_BufFullCnt;
			}

			return true;
		}
		else//如果缓冲区放不下这条数据了
		{
			++m_BufFullCnt;

			return false;
		}

		//非阻塞发送数据，不再使用定时发送
		//发送完重置定时发送的计时时间
		//resetDTSend();//其实可以注释掉

		//***注***
		//SendData()函数不再实现发送数据的业务，
		//而是将该业务转交给select机制配合WriteData()实现
	}

	//参照CellClient::SendData()的代码，封装缓冲区的操作相关代码
	int WriteToSocket(SOCKET socket)
	{
		int ret = 0;

		if (m_lastPos > 0 && INVALID_SOCKET != socket)
		{
			ret = send(socket, (const char*)m_pBuf,
				m_lastPos, 0);

			//发送完后，标记缓冲区中已有的数据量为0
			m_lastPos = 0;

			//重置写满情况计数
			m_BufFullCnt = 0;
		}

		return ret;
	}

	//参照CellServer::RecvData()的代码，封装数据存入缓冲区的操作相关代码
	int ReadFromSocket(SOCKET socket)
	{
		if (m_lastPos < m_BufferSize)
		{
			//计算新客户端数据应该存放的位置
			char* pRecv = m_pBuf + m_lastPos;

			//接收客户端数据存到【客户端的】自定义接收缓冲区m_pBuf
			int len = (int)recv(socket,pRecv, m_BufferSize - m_lastPos, 0);

			if (len <= 0)
			{
				return len;
			}

			m_lastPos += len;
			return len;
		}

		return 0;
	}

	//参照CellServer::RecvData()的代码，封装从缓冲区解析数据包的操作相关代码
	bool hasMsg()
	{
		if (m_lastPos >= sizeof(DataHead))
		{
			//指向某个客户端对应的m_pBuf的指针解释为DataHead*类型的指针，
			//用于访问DataHead的数据成员
			DataHead* pHead = reinterpret_cast<DataHead*>(m_pBuf);

			//判断消息缓冲区的数据长度是否大于消息长度
			//解决【少包】的问题
			return (m_lastPos >= pHead->datalength);
		}
		else
			return false;
	}

	//被	DataHead* frontMsg() 调用
	char* getBuf()
	{
		return m_pBuf;
	}

	void ReduceMsgInBuf(int len)
	{
		int unprocessed = m_lastPos - len;

		//防御性判断
		if (unprocessed > 0)
		{
			memcpy(m_pBuf, m_pBuf + len, unprocessed);
		}
		//若unprocessed为0，直接标记m_lastPos为0即可
		m_lastPos = unprocessed;

		if (m_BufFullCnt > 0)
			--m_BufFullCnt;
	}
};