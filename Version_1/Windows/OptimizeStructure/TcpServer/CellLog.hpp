#pragma once
//忽略fopen导致的C4996错误
#pragma warning(disable:4996)

#include "Cell.hpp"
#include <ctime>

class CellLog
{
private:
	//直接做写入日志到文件的操作，是不妥的，
	//对于一个高并发服务器，这样会很占用CPU时间，
	//最好的解决方法是将此业务交给CellTask类，即
	//用另一个线程来实现业务
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
			Info("CellLog::~CellLog()  fclose\n");

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
		CellLog* pLog = &Instance();
#if 0
		if (pLog->m_logFile)
		{
			//获得格式化后的时间
			auto t = system_clock::now();
			auto tNow = system_clock::to_time_t(t);
			//fprintf(pLog->m_logFile, "%s", ctime(&tNow));//此格式自带换行符
			std::tm* now = std::gmtime(&tNow);
			fprintf(pLog->m_logFile,"[%d-%d-%d %d:%d:%d]",
				now->tm_year+1900,now->tm_mon+1,now->tm_mday,
				now->tm_hour+8,now->tm_min,now->tm_sec);
			//东八时区，所以tm_hour+8

			fprintf(pLog->m_logFile,"%s",pStr);
			//仅有上面的语句，server将在收到exit命令后才将各种字符串信息写入到指定文件中
			//而若程序非正常退出，是不会将数据写入的
			//用于及时写入
			fflush(pLog->m_logFile);
		}
		printf(pStr);
#endif

		//使用CellTaskServer类新开一个线程来记录日志
		pLog->m_CellTaskServer.addTask(
			[pLog, pStr]() {
				if (pLog->m_logFile)
				{
					//获得格式化后的时间
					auto t = system_clock::now();
					auto tNow = system_clock::to_time_t(t);
					//fprintf(pLog->m_logFile, "%s", ctime(&tNow));//此格式自带换行符
					std::tm* now = std::gmtime(&tNow);
					fprintf(pLog->m_logFile, "[%d-%d-%d %d:%d:%d]",
						now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
						now->tm_hour + 8, now->tm_min, now->tm_sec);
					//东八时区，所以tm_hour+8

					fprintf(pLog->m_logFile, "%s", pStr);
					//仅有上面的语句，server将在收到exit命令后才将各种字符串信息写入到指定文件中
					//而若程序非正常退出，是不会将数据写入的
					//用于及时写入
					fflush(pLog->m_logFile);
				}		

				printf("%s",pStr);
			});
	}

	//可变参模板
	template<typename ...Args>
	static void Info(const char* format, Args ...args)
	{
		CellLog* pLog = &Instance();

		pLog->m_CellTaskServer.addTask(
			[pLog, format, args...]() {
				if (pLog->m_logFile)
				{
					auto t = system_clock::now();
					auto tNow = system_clock::to_time_t(t);
					std::tm* now = std::gmtime(&tNow);
					fprintf(pLog->m_logFile, "[%d-%d-%d %d:%d:%d]",
						now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
						now->tm_hour + 8, now->tm_min, now->tm_sec);

					fprintf(pLog->m_logFile, format, args...);
					fflush(pLog->m_logFile);
				}

				printf(format, args...);			
			});
	}

	void setLogPath(const char* logPath,const char* mode)
	{
		if (m_logFile)
		{
			Info("CellLog::setLogPath  m_logFile!=nullptr  fclose <%s>\n", logPath);

			fclose(m_logFile);
			m_logFile = nullptr;
		}

		m_logFile = fopen(logPath,mode);
		if (m_logFile)
		{
			Info("CellLog::setLogPath <%s,%s> fopen()成功\n",logPath,mode);
		}
		else
		{
			Info("CellLog::setLogPath <%s,%s> fopen()失败\n", logPath, mode);
		}

	}
};