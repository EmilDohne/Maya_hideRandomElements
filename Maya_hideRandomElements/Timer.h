#pragma once

#include <chrono>
#include <vector>
#include <maya/MString.h>

class Timer
{
public:
	Timer();

	~Timer();

	double Stop();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
};


class TimeProfiler
{
public:
	std::vector<Timer> timers;
	std::vector<double> times;
	int count = 0;
	MString print_info;

	TimeProfiler();
	~TimeProfiler();

	void addTimer();
	void stopTimer();

	void Stop();
};