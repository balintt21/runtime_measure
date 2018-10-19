#ifndef _RUNTIME_MEASUREMENT_H_
#define _RUNTIME_MEASUREMENT_H_

#include <stdint.h>
#include <sys/times.h>
#include <utility>
#include <unistd.h>
#include <functional>
#include <chrono>

class RuntimeMeasurement
{
public:
    struct Result
    {
        int64_t user_time_clktck;
        int64_t system_time_clktck;
        int64_t duration_ns;
        int64_t clocks_per_sec;

        double durationToSec() const { return static_cast<double>(duration_ns) / 1000000000.0; }
        double userTimeToSec() const { return (user_time_clktck / static_cast<double>(clocks_per_sec)); }
        double systemTimeToSec() const { return (system_time_clktck / static_cast<double>(clocks_per_sec)); }
    };
protected:
    int64_t                                 mStartTime;
    struct tms                              mStartTimes;
    struct tms                              mCurrentTimes;
    std::function<void (const Result&)>  mResultCallback;
public:
    static int64_t CLOCK_TICKS_PER_SEC() { return sysconf(_SC_CLK_TCK); }

    inline Result measure()
    {
        times(&mCurrentTimes);
        Result result {  std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now())
                                                                               .time_since_epoch().count() - mStartTime
                       , (mCurrentTimes.tms_utime - mStartTimes.tms_utime)
                       , (mCurrentTimes.tms_stime - mStartTimes.tms_stime)
                       , sysconf(_SC_CLK_TCK) };
        if( mResultCallback ) { mResultCallback(result); }
        return result;
    }
    
    RuntimeMeasurement(const std::function<void (const Result&)>& result_callback = nullptr) : mResultCallback(result_callback)
    {
        mStartTime = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
        times(&mStartTimes);
    }
    
    ~RuntimeMeasurement()
    {
        measure();
    }
};

#endif
