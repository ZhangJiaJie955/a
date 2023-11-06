#ifndef _H9TIMER_H
#define _H9TIMER_H
#include <string>
#include <map>

#define WIN32

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <yaml-cpp/yaml.h>

namespace H9 {


class Timer
{
public:
    explicit Timer(std::string const &, bool = true);
    ~Timer();

    float getDuration();
    void  reset();
    
    static void summarize();

private:

#ifdef WIN32
    typedef LARGE_INTEGER TimeType;
#else
    typedef struct timeval TimeType;
#endif

    struct TimerData
    {
        TimerData() : m_count(0), m_totalTime(0.f) {}
        TimerData(float time) : m_count(1), m_totalTime(time) {}
        ~TimerData() {}

        void increase(float time)
        {
            ++m_count;
            m_totalTime += time;
        }

        int m_count;
        float m_totalTime;
    };

private:
    static std::map<std::string, TimerData> &getSummary()
    {
        static std::map<std::string, TimerData> timerData;
        return timerData;
    }

    float time_lapse(TimeType const &t) const;

private:
    std::string m_name;
    bool m_saveToSummary;

    TimeType m_start;
};

void init_timer();
} //H9

#ifdef H9_ALGO_TIMER
#define H9_PROFILER(name) H9::Timer t(name, true);
#define H9_TIMER(name) H9::Timer t2(name, false);
#else
#define H9_PROFILER(name)
#define H9_TIMER(name)

#ifdef    SCOPE_TIMER_ON
//#define H9_PROFILER(name) H9::Timer t(name, true);
#define H9_TIMER(name)  H9::Timer t(name, false);
#endif

#endif

#endif
