#pragma once

#include "Cell.hpp"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#if 0
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
#endif

//如何解决虚假唤醒？(  先调用了WakeUp()然后调用了Wait()   )
#if 1
class CellSemaphore
{
private:
	//bool m_isWaitExit = false;

	//等待计数
	int m_wait = 0;

	//唤醒计数
	int m_wakeup = 0;

	//阻塞等待-条件变量
	std::condition_variable m_cv;

	std::mutex m_mutex;

public:
	//阻塞当前线程
	void Wait()
	{
		std::unique_lock<std::mutex>lock(m_mutex);

		if (--m_wait < 0)
		{
			m_cv.wait(lock, [this]()->bool {
				return m_wakeup > 0;
				});
			//***说明***
			/*
			wakeup 不一定意味着线程所需要的条件已经掌握了。更确切地说，
			在wakeup之后仍然需要代码去验证“条件实际已达成”。
			避免假醒
			*/

			--m_wakeup;
		}
	}

	//唤醒当前线程
	void WakeUp()
	{
		std::unique_lock<std::mutex>lock(m_mutex);

		if (++m_wait<=0)//考虑有多个线程调用Wait()，那就会有小于0的情况
		{
			++m_wakeup;//唤醒计数加一

			m_cv.notify_one();
		}
	}
};
/*
补充：
此处使用了多重安全机制：

利用 C++11 中的条件变量和互斥量来实现线程间的同步。
在这个实现中，有两个主要的安全机制：
1.互斥锁(std::mutex)和std::unique_lock:
互斥锁用于保护共享资源的访问，确保任何时刻只有一个线程可以修改
m_wait 和 m_wakeup 变量。
这是防止多个线程同时进入 Wait() 或 WakeUp() 函数并尝试修改共享资源，
从而可能导致数据竞争和不一致状态。
std::unique_lock 是一个范围锁（scope-based lock），它在构造时自动加锁，
并在离开作用域（如函数返回或抛出异常时）时自动释放锁。
这种自动锁定和解锁机制提供了一种异常安全的保证，
即使在发生异常时也能确保互斥量被正确释放。

2.条件变量(std::condition_variable)和条件等待:
条件变量与互斥锁配合使用，允许线程在某个条件尚未满足时阻塞等待，
直到其他线程修改了条件并通知等待的线程。这在多线程编程中是一种常见的同步机制。
在 Wait() 函数中，如果 m_wait 减一后小于 0，说明没有足够的资源可供当前线程继续执行，
因此线程通过 m_cv.wait() 进入阻塞状态，等待被唤醒。
这里的 wait 函数接受一个锁和一个谓词，谓词返回 false 时线程阻塞，返回 true 时线程被唤醒。
这个谓词检查(m_wakeup > 0)是为了防止假唤醒（spurious wakeup）。
在 WakeUp() 函数中，如果有线程正在等待（m_wait <= 0），
则通过 m_cv.notify_one() 唤醒一个等待的线程。
这里的【安全机制】在于：
即使 WakeUp 被多次调用，也只会唤醒实际正在等待的线程。
如果没有线程在等待，m_wait 的值将会大于 0，因此 WakeUp 函数将不会执行唤醒操作。
这意味着在没有线程需要被唤醒的情况下，不会发生无效的唤醒。
如果有一个或多个线程正在等待，m_wait 的值将会小于或等于 0，
WakeUp 函数将执行唤醒操作，并将 m_wakeup 的值自增，
确保有足够的“唤醒信号”可用于等待的线程。

这种机制避免了以下情况：
无谓的唤醒：如果在没有线程等待的情况下发出唤醒信号，那么唤醒信号就被浪费了，因为没有线程会接收到这个信号。
过度唤醒：如果多个 WakeUp 调用导致唤醒多个线程，但实际上只有一个线程在等待，则其他被唤醒的线程将无事可做。

这两种机制结合起来，不仅保证了对共享资源访问的互斥和同步，
还提供了一种高效的等待-通知机制，从而在资源不足时减少了线程的忙等（busy-waiting），
提高了线程的等待效率。
*/
#endif