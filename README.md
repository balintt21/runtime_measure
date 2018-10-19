# runtime_measure
C++11 [RAII](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization) class to measure CPU time spent executing instructions of the calling process.

### :clipboard: Features
* Invokes the given callback after measuring
* Measures user-space and kernel-space CPU time
* Measuring can be done before lifetime ends

### :warning: Requirements
* **C++11**
* **POSIX**

### :computer: [Implementation](/runtime_measurement.h)

### :page_with_curl: Examples

  [Example](/README.md#example), [Example 1](/README.md#example-1), [Example 2](/README.md#example-2)

## Example

```c++
#include <stdio.h>
#include <experimental/random>
#include "runtime_measurement.h"

double userSpaceWorker(uint32_t cycle_count)
{
    int numerator = 0;
    int denominator = 0;
    double sum = 0.0;
    for(int i = 0; i < cycle_count; ++i)
    {
        numerator = std::experimental::randint(1, 1024);
        denominator = std::experimental::randint(1, 1024);
        sum += (numerator / static_cast<double>(denominator));
    }
    return sum;
}

int main(int argc, char** argv)
{
    RuntimeMeasurement measurerer([](const RuntimeMeasurement::Result& result)
    {
        printf("measurement: duration: %f s, user-space: %ld(%f s), kernel-space: %ld(%f s)\n"
              , result.durationToSec()
              , result.user_time_clktck, result.userTimeToSec()
              , result.system_time_clktck, result.systemTimeToSec());
    });
    auto sum = userSpaceWorker(25*1024*1024);
    return 0;
}
```

## Example 1

```c++
int main(int argc, char** argv)
{
    double sum = 0.0;
    {
        RuntimeMeasurement measurerer([](const RuntimeMeasurement::Result& result)
        {
            printf("measurement: user-space: %ld(%f s), kernel-space: %ld(%f s)\n"
                  , result.user_time_clktck, result.userTimeToSec(), result.system_time_clktck, result.systemTimeToSec());
        });
        sum = userSpaceWorker(25*1024*1024);
        //lifetime of measurerer ends here
    }
    printf("Sum: %f\n", sum);
    return 0;
}
```

## Example 2

```c++
int main(int argc, char** argv)
{
    double sum = 0.0;
    RuntimeMeasurement measurerer;
    sum = userSpaceWorker(25*1024*1024);
    
    auto result = measurerer.measure();
    
    printf("measurement: user-space: %ld(%f s), kernel-space: %ld(%f s)\n"
                  , result.user_time_clktck, result.userTimeToSec(), result.system_time_clktck, result.systemTimeToSec());
    printf("Sum: %f\n", sum);
    return 0;
}
```
