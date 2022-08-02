#include "Timer.h"

#include <chrono>
#include <vector>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>

Timer::Timer()
{
	m_StartTimepoint = std::chrono::high_resolution_clock::now();
}

Timer::~Timer()
{
	Stop();
}

double Timer::Stop() 
{
	auto endTimePoint = std::chrono::high_resolution_clock::now();

	auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
	auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

	auto duration = end - start;
	return (double)duration;
}



TimeProfiler::TimeProfiler()
{
}
TimeProfiler::~TimeProfiler()
{
	Stop();
}

void TimeProfiler::addTimer()
{
	this->timers.push_back(Timer());
	count++;
}
void TimeProfiler::stopTimer()
{
	times.push_back(timers.back().Stop());
}

void TimeProfiler::Stop()
{
	double time = 0;
	for (const auto& value : times)
	{
		time += value;
	}
	double avg_time = time / count;

	MGlobal::displayInfo(print_info + MString("total processing time in ms = ") + time*0.001);
	MGlobal::displayInfo(print_info + MString("average processing time in ms = ") + avg_time * 0.001);
	MGlobal::displayInfo(print_info + MString(" was traversed this many times: ") + count);

}
