#ifndef _RUNTIME_MEASUREMENT_H_
#define _RUNTIME_MEASUREMENT_H_

#include <stdint.h>
#include <sys/times.h>
#include <utility>
#include <unistd.h>
#include <functional>

class RuntimeMeasurement
{
public:
    struct Result
    {
        int64_t user_time_clktck;
        int64_t system_time_clktck;
        int64_t clocks_per_sec;

        double userTimeToSec() const { return (user_time_clktck / static_cast<double>(clocks_per_sec)); }
        double systemTimeToSec() const { return (system_time_clktck / static_cast<double>(clocks_per_sec)); }
    };
protected:
    struct tms                              mStartTimes;
    struct tms                              mCurrentTimes;
    std::function<void (const Result&)>  mResultCallback;
public:
    static int64_t CLOCK_TICKS_PER_SEC() { return sysconf(_SC_CLK_TCK); }

    inline Result measure()
    {
        times(&mCurrentTimes);
        Result result {  (mCurrentTimes.tms_utime - mStartTimes.tms_utime)
                       , (mCurrentTimes.tms_stime - mStartTimes.tms_stime)
                       , sysconf(_SC_CLK_TCK) };
        if( mResultCallback ) { mResultCallback(result); }
        return result;
    }
    
    RuntimeMeasurement(const std::function<void (const Result&)>& result_callback = nullptr) : mResultCallback(result_callback)
    {
        times(&mStartTimes);
    }
    
    ~RuntimeMeasurement()
    {
        measure();
    }
};

#endif
