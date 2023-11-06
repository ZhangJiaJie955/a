#include "h9Timer.h"
#include "h9Log.h"

using namespace std;

namespace {
#ifdef WIN32
LARGE_INTEGER frequency;

inline void get_current_time(LARGE_INTEGER &t)
{
    QueryPerformanceCounter(&t);
}


#else

inline void get_current_time(timeval &t)
{
    gettimeofday(&t, NULL);
}

#endif
}

namespace H9 {
void init_timer()
{
#ifdef WIN32
    QueryPerformanceFrequency(&frequency);
#endif
}

Timer::Timer(string const &name, bool saveToSummary):m_saveToSummary(saveToSummary)
//    : m_name(name), m_saveToSummary(saveToSummary)
{
    printf("begin  Timer!\n");
    //m_name = name;
    get_current_time(m_start);
    printf("finish  Timer!\n");
}


void Timer::reset()
{
    get_current_time(m_start);
}

float Timer::getDuration()
{
    TimeType t;
    get_current_time(t);
    float duration = time_lapse(t);
    return duration;
}

Timer::~Timer()
{
    static TimeType t;
    get_current_time(t);
    float duration = time_lapse(t);
    if (!m_saveToSummary) {
        H9_INFO("Runtime %s: %f", m_name.c_str(), duration);
     } else {
        map<string, TimerData> &summary = getSummary();
        map<string, TimerData>::iterator sit = summary.find(m_name);
        if (summary.end() == sit)
            summary[m_name] = TimerData(duration);
        else
            sit->second.increase(duration);
    }
}

void Timer::summarize()
{
    map<string, TimerData> &summary = getSummary();
    H9_INFO("\n=========================================");
    H9_INFO("Function \t count \t totaltime(ms)");
    for (map<string, TimerData>::const_iterator sit = summary.begin();
         sit != summary.end(); ++sit)
        H9_INFO("%s \t %d \t %f", sit->first.c_str() ,sit->second.m_count, sit->second.m_totalTime);
}

float Timer::time_lapse(TimeType const &t) const
{
#ifdef WIN32
        return 1000.0 * (t.QuadPart - m_start.QuadPart) / frequency.QuadPart;
#else
        return 1.0 * (t.tv_sec - m_start.tv_sec) * 1000
            + 1.0 * (t.tv_usec - m_start.tv_usec) / 1000;
#endif
}

} //H9
