#pragma once

#include <chrono>

using namespace std::chrono;

class Timestamp
{
private:
	time_point<high_resolution_clock> m_begin;

public:
	Timestamp():m_begin(high_resolution_clock::now())
	{}

	void update()
	{
		//***ע***
		//��Ҫ�߾���ʱ���������̳���ʱ��ʱ����ʹ�� high_resolution_clock ��
		//system_clock �ṩ��ʱ����ʱ����ͨ������ʵ�����ʱ�����,
		//����Ҫ������ʵ�����ʱ����ʱ����ʹ�� system_clock
		m_begin = high_resolution_clock::now();
	}

	//��ȡ΢�뼶ʱ��
	long long getElapsedTimeInMicrosecond()
	{
		return duration_cast<microseconds>
			(high_resolution_clock::now() - m_begin).count();
	}

	//��ȡ���뼶ʱ��
	double getElapsedTimeInMillisecond()
	{
		return getElapsedTimeInMicrosecond() * 0.001;
	}

	//��ȡ�뼶ʱ��
	double getElapsedTimeInSecond()
	{
		return getElapsedTimeInMicrosecond() * 0.000001;
	}
};


