#pragma once

#include <chrono>

using namespace std::chrono;

class Timestamp
{
private:
	time_point<high_resolution_clock> m_begin;

public:
	Timestamp() :m_begin(high_resolution_clock::now())
	{}

	void update()
	{
		//***注***
		//需要高精度时钟来测量短持续时间时，请使用 high_resolution_clock ，
		//system_clock 提供的时间点和时间间隔通常与现实世界的时间相关,
		//当需要测量现实世界的时间间隔时，请使用 system_clock
		m_begin = high_resolution_clock::now();
	}

	//获取微秒级时间
	long long getElapsedTimeInMicrosecond()
	{
		return duration_cast<microseconds>
			(high_resolution_clock::now() - m_begin).count();
	}

	//获取毫秒级时间
	double getElapsedTimeInMillisecond()
	{
		return getElapsedTimeInMicrosecond() * 0.001;
	}

	//获取秒级时间
	double getElapsedTimeInSecond()
	{
		return getElapsedTimeInMicrosecond() * 0.000001;
	}
};


