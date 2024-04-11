#pragma once

#include "Cell.hpp"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

class CellSemaphore
{
private:
	bool m_isWaitExit = false;

	//阻塞等待-条件变量
	std::condition_variable m_cv;

	std::mutex m_mutex;

public:
	void Wait()
	{
		std::unique_lock<std::mutex>lock(m_mutex);

		m_isWaitExit = true;

		//while (m_isWaitExit)
		//{
		//	std::chrono::milliseconds t(1);
		//	std::this_thread::sleep_for(t);
		//}

		m_cv.wait(lock);
	}

	void WakeUp()
	{
		std::unique_lock<std::mutex>lock(m_mutex);

		if (m_isWaitExit)
		{
			m_isWaitExit = false;

			m_cv.notify_one();
		}
		else
			printf("CellSemaphore WakeUp() error.\n");
	}

	//注意要先调用Wait()再调用WakeUp()，在多线程中容易出错
};

//如何解决虚假唤醒？(  先调用了WakeUp()然后调用了Wait()   )
