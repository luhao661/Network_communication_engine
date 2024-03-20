#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>//适合快速安插和移除元素
#include <functional>//mem_fn()

//任务类型——抽象基类
class CellTask
{
private:

public:
	CellTask()
	{

	}

	virtual ~CellTask()
	{

	}

	virtual void doTask() = 0;
};


typedef std::shared_ptr<CellTask> CellTaskPtr;


//执行任务的服务类型
//该类型作用是
//不断等待任务的到来，并添加具体任务到list容器（类似于EasyTcpServer类）
//并不断执行具体任务
class CellTaskServer
{
private:
	//任务数据
	std::list<CellTaskPtr> m_tasks;
	//任务数据缓冲区
	std::list<CellTaskPtr> m_tasks_buffer;
	//改变任务数据缓冲区时需要加锁
	std::mutex m_mutex;

public:
	CellTaskServer()
	{}

	~CellTaskServer()
	{}

	//维护一个list容器，存储添加的具体任务指针
	//错误写法：
	//void addTask(CellServerMsgToClientTaskPtr task)
	//正确写法：
	void addTask(CellTaskPtr& task)
	{
		{
			std::lock_guard<std::mutex>lg(m_mutex);
			m_tasks_buffer.push_back(task);
		}
	}
	//指向基类的指针可以指向派生类，也适用于智能指针，但实参传入端要进行强制类型转换

	//启动工作线程
	void Start()
	{
		std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		t.detach();
	}

protected://声明为protected使CellTaskServer对象无法访问OnRun()

	//工作函数
	void OnRun()
	{
		while (true)
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
			for (auto pTask : m_tasks)
			{
				pTask->doTask();
				//delete pTask;
			}

			//清空任务元素
			m_tasks.clear();
		}
	}

};
#endif