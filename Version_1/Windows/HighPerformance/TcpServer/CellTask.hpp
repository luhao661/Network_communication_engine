#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>//�ʺϿ��ٰ�����Ƴ�Ԫ��
#include <functional>//mem_fn()

//�������͡����������
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

//ִ������ķ�������
//������������
//���ϵȴ�����ĵ���������Ӿ�������list������������EasyTcpServer�ࣩ
//������ִ�о�������
class CellTaskServer
{
private:
	//��������
	std::list<CellTask*> m_tasks;
	//�������ݻ�����
	std::list<CellTask*> m_tasks_buffer;
	//�ı��������ݻ�����ʱ��Ҫ����
	std::mutex m_mutex;

public:
	CellTaskServer()
	{}

	~CellTaskServer()
	{}

	//ά��һ��list�������洢��ӵľ�������ָ��
	void addTask(CellTask* task)
	{
		{
			std::lock_guard<std::mutex>lg(m_mutex);
			m_tasks_buffer.push_back(task);
		}
	}

	//���������߳�
	void Start()
	{
		std::thread t(std::mem_fn(&CellTaskServer::OnRun),this);
		t.detach();
	}

protected://����ΪprotectedʹCellTaskServer�����޷�����OnRun()

	//��������
	void OnRun()
	{
		while (true)
		{
			//�ӻ�������ȡ������
			if (!m_tasks_buffer.empty())
			{
				std::lock_guard<std::mutex>lg(m_mutex);
				for (auto pTask : m_tasks_buffer)
					m_tasks.push_back(pTask);
				m_tasks_buffer.clear();
			}
			else//���û������
			{
				std::chrono::milliseconds time(1);
				std::this_thread::sleep_for(time);

				continue;
			}

			//��������
			for (auto pTask : m_tasks)
			{
				pTask->doTask();
				delete pTask;
			}

			//�������Ԫ��
			m_tasks.clear();
		}
	}

};
#endif