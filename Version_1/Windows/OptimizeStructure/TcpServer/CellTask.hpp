#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>//适合快速安插和移除元素
#include <functional>//mem_fn

#include "CellThread.hpp"

//执行任务的服务类型
//该类型作用是
//不断等待任务的到来，并添加具体任务到list容器（类似于EasyTcpServer类）
//并不断执行具体任务
class CellTaskServer
{
	typedef std::function<void()>CellTask;

public:
	int m_CellServer_id = -1;

private:
	//任务数据
	std::list<CellTask> m_tasks;
	//任务数据缓冲区
	std::list<CellTask> m_tasks_buffer;
	//改变任务数据缓冲区时需要加锁
	std::mutex m_mutex;

	//创建线程管理类
	CellThread m_thread;

public:
	CellTaskServer()
	{}

	~CellTaskServer()
	{}

	//维护一个list容器，存储添加的具体任务指针
	void addTask(CellTask task)
	{
		{
			std::lock_guard<std::mutex>lg(m_mutex);
			m_tasks_buffer.push_back(task);
		}
	}

	//启动工作线程
	void Start()
	{
		//std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		//m_isRun = true;

		//t.detach();
		
		m_thread.Start(nullptr, [this](CellThread* pThread) {
			//OnRun();可以直接这样写，但当创建多个线程时，容易调用了不同线程管理类的某方法，
			// 从不易写错方面考虑，改为使用
			OnRun(pThread);
			},nullptr);

		//pThread值？
		//从调试中可以得知pThread = &m_thread
	}

	void Close()
	{
		printf("CellTaskServer %d Close() begin\n", m_CellServer_id);

		//关闭CellTaskServer开的线程
		m_thread.Close();

		printf("CellTaskServer %d Close() end\n", m_CellServer_id);
	}

protected://声明为protected使CellTaskServer对象无法访问OnRun()

	//工作函数
	void OnRun(CellThread *pThread)//被某个线程所启动的OnRun()一定是该线程所属的		
	{									                  //线程管理类的方法来进行判断，避免混淆
		while (pThread->isRun())
		{
			//从缓冲区中取出数据
			if (!m_tasks_buffer.empty())
			{
				std::lock_guard<std::mutex>lg(m_mutex);
				for (auto pTask : m_tasks_buffer)
					m_tasks.push_back(pTask);
				m_tasks_buffer.clear();
			}
			else//如果没有数据
			{
				std::chrono::milliseconds time(1);
				std::this_thread::sleep_for(time);

				continue;
			}

			//处理任务(消费任务)
			for (auto Task : m_tasks)
			{
				Task;
			}

			//清空任务元素
			m_tasks.clear();
		}
		printf("CellTaskServer %d OnRun() exit\n", m_CellServer_id);
	}

};
#endif