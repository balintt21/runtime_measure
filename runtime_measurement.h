#ifndef _RUNTIME_MEASUREMENT_H_
#define _RUNTIME_MEASUREMENT_H_

#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <utility>
#include <unistd.h>
#include <functional>
#include <chrono>

class RuntimeMeasurement
{
public:
    struct Result
    {
        int64_t user_time_us;
        int64_t system_time_us;
        int64_t duration_ns;
    };
protected:
    int64_t                                 mStartTime;
    struct rusage                           mStartTimes;
    struct rusage                           mCurrentTimes;
    std::function<void (const Result&)>  mResultCallback;
public:
    int64_t timevalToUSec(struct timeval& t) { return static_cast<int64_t>(t.tv_sec * 1000000ll) + static_cast<int64_t>(t.tv_usec); }
    
    inline Result measure()
    {
        getrusage(RUSAGE_THREAD, &mCurrentTimes);
        Result result {  (timevalToUSec(mCurrentTimes.ru_utime) - timevalToUSec(mStartTimes.ru_utime))
                       , (timevalToUSec(mCurrentTimes.ru_stime) - timevalToUSec(mStartTimes.ru_stime))
                       , std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now())
                                                                               .time_since_epoch().count() - mStartTime
                       };
        if( mResultCallback ) { mResultCallback(result); }
        return result;
    }
    
    RuntimeMeasurement(const std::function<void (const Result&)>& result_callback = nullptr) : mResultCallback(result_callback)
    {
        mStartTime = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
        getrusage(RUSAGE_THREAD, &mStartTimes);
    }
    
    ~RuntimeMeasurement()
    {
        measure();
    }
};

#endif
