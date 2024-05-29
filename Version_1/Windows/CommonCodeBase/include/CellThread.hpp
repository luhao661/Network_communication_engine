#pragma once

#include "CellSemaphore.hpp"

class CellThread
{
private:
	typedef std::function<void(CellThread*)>EventCall;//事件回调函数

private:
	//标记线程是否在运行中
	bool m_isRun = false;

	//控制线程的终止、退出
	CellSemaphore m_Sem;

	EventCall m_OnCreate;
	EventCall m_OnRun;
	EventCall m_OnDestory;

	//不同线程改变数据时需要加锁
	std::mutex m_mutex;

public:

	//启动函数，用于注册三个回调函数，并创建线程
	//实参至多需要三个回调函数的函数指针/引用
	void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex>lg(m_mutex);

		if(!m_isRun)
		{
			m_isRun = true;

			//注册函数
			if (onCreate)
				m_OnCreate= onCreate;
			if (onRun)
				m_OnRun= onRun;
			if (onDestory)
				m_OnDestory= onDestory;

			std::thread Thread = std::thread(std::mem_fn(&CellThread::OnWork), this);
			Thread.detach();
		}
	}

	//线程是否处于运行状态
	bool isRun()
	{
		return m_isRun;
	}

	//关闭线程
	void Close() 
	{
		std::lock_guard<std::mutex>lg(m_mutex);

		if (m_isRun)
		{
			m_isRun = false;
			m_Sem.Wait();//等待OnWork()函数退出，也就是OnRun()也退出了
		}
	}

	//从工作函数中退出
	//不需要使用信号量来阻塞等待
	void Exit()
	{
		std::lock_guard<std::mutex>lg(m_mutex);

		if (m_isRun)
		{
			m_isRun = false;
		}
	}

protected:

	//线程运行时的工作函数
	void OnWork()
	{
		/*
		在线程的不同生命周期阶段调用设置的回调函数。
		每个 if 语句检查相应的回调函数是否已被设置（即，它不是 nullptr），
		如果是，那么调用该函数并传入 this 指针作为参数。
		这使得调用的函数能够访问当前 CellThread 实例的成员。
		*/
		if (m_OnCreate)
			m_OnCreate(this);
		if (m_OnRun)
			m_OnRun(this);
		if (m_OnDestory)
			m_OnDestory(this);

		m_Sem.WakeUp();
	}
};
