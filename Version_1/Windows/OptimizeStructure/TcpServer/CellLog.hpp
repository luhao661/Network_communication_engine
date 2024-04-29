#pragma once

#include "Cell.hpp"

class CellLog
{
private:
	CellTaskServer m_CellTaskServer;

	FILE* m_logFile = nullptr;

	CellLog()
	{
		m_CellTaskServer.Start();
	}

	~CellLog()
	{
		m_CellTaskServer.Close();

		if (m_logFile)
		{
			fclose(m_logFile);
			m_logFile = nullptr;
		}
	}

public:

	//创建对象单例
	static CellLog& Instance()
	{
		static CellLog sLog;

		return sLog;
	}

	static void Info(const char* pStr)
	{
		printf(pStr);
	}

	//可变参模板
	template<typename ...Args>
	static void Info(const char* format, Args ...args)
	{
		printf(format, args...);
	}

	//在此处直接做写入日志到文件的操作，是不妥的，
	//对于一个高并发服务器，这样会很占用CPU时间，
	//最好的解决方法是将此业务交给CellTask类，即
	//用另一个线程来实现业务

	void setLogPath(const char* logPath,const char* mode)
	{
		if (m_logFile)
		{
			Info("CellLog::setLogPath m_logFile!=nullptr  fclose <%s>", logPath);

			fclose(m_logFile);
			m_logFile = nullptr;
		}

		m_logFile = fopen(logPath,mode);
		if (m_logFile)
		{
			Info("CellLog::setLogPath success, <%s,%s>",logPath,mode);
		}
		else
		{
			Info("CellLog::setLogPath failed, <%s,%s>", logPath, mode);
		}

	}
};