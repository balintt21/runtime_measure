#ifndef _RUNTIME_MEASUREMENT_H_
#define _RUNTIME_MEASUREMENT_H_

#include <stdint.h>
#include <sys/time.h>
#define __USE_GNU
#include <sys/resource.h>
#include <unistd.h>

#ifdef __cplusplus

#include <functional>
#include <chrono>
#include <utility>

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

#else
    #include <time.h>

    inline int64_t timeval_to_usec(struct timeval* t);
    inline int64_t timeval_to_usec(struct timeval* t) { return (int64_t)(t->tv_sec * 1000000ll) + (int64_t)(t->tv_usec); }

    inline int64_t get_monoton_time_ns(void);
    inline int64_t get_monoton_time_ns(void)
    {
        struct timespec t;
	    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
        return (t.tv_sec) * 1000000000ll + (t.tv_nsec);
    }

    struct runtime_measurement_result_t
    {
        int64_t user_time_us;
        int64_t system_time_us;
        int64_t duration_ns;
    };
    typedef struct runtime_measurement_result_t RuntimeMeasurementResult;

    struct runtime_measurement_t
    {
        RuntimeMeasurementResult result;
        void (*callback)(RuntimeMeasurementResult*);
        struct rusage start_measure;
        struct rusage current_measure;
    };

    typedef struct runtime_measurement_t RuntimeMeasurement;
    typedef void (*runtime_measurement_cb_t)(RuntimeMeasurementResult*);

    inline void runtime_measurement_init(RuntimeMeasurement* obj, runtime_measurement_cb_t cb);
    inline void runtime_measurement_init(RuntimeMeasurement* obj, runtime_measurement_cb_t cb)
    {
        obj->result.user_time_us = 0;
        obj->result.system_time_us = 0;
        obj->result.duration_ns = get_monoton_time_ns();
        obj->callback = cb;
        getrusage(RUSAGE_THREAD, &obj->start_measure);
    }

    inline void runtime_measurement_measure(RuntimeMeasurement* obj);
    inline void runtime_measurement_measure(RuntimeMeasurement* obj)
    {
        getrusage(RUSAGE_THREAD, &obj->current_measure);
        obj->result.duration_ns = get_monoton_time_ns() - obj->result.duration_ns;
        obj->result.user_time_us = (timeval_to_usec(&obj->current_measure.ru_utime) - timeval_to_usec(&obj->start_measure.ru_utime));
        obj->result.system_time_us = (timeval_to_usec(&obj->current_measure.ru_stime) - timeval_to_usec(&obj->start_measure.ru_stime));
        if(obj->callback)
        {
            obj->callback(&obj->result);
        }
    }

#endif

#endif
