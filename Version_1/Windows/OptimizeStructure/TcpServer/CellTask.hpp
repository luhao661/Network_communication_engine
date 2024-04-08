#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>//适合快速安插和移除元素
#include <functional>//mem_fn

//执行任务的服务类型
//该类型作用是
//不断等待任务的到来，并添加具体任务到list容器（类似于EasyTcpServer类）
//并不断执行具体任务
class CellTaskServer
{
	typedef std::function<void()>CellTask;

private:
	//任务数据
	std::list<CellTask> m_tasks;
	//任务数据缓冲区
	std::list<CellTask> m_tasks_buffer;
	//改变任务数据缓冲区时需要加锁
	std::mutex m_mutex;
	
	bool m_isRun = false;

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
		std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		m_isRun = true;

		t.detach();
	}

	void Close()
	{
		m_isRun = false;
	}

protected://声明为protected使CellTaskServer对象无法访问OnRun()

	//工作函数
	void OnRun()
	{
		while (m_isRun)
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
	}

};
#endif