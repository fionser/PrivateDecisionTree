#ifndef UTIL_TIMER_HPP
#define UTIL_TIMER_HPP
#include <chrono>

using std::chrono::duration_cast;
typedef std::chrono::nanoseconds Time_t;
typedef std::chrono::high_resolution_clock Clock;
double time_as_second(const Time_t &t); 
double time_as_millsecond(const Time_t &t);

#endif //SYMRLWE_TIMER_HPP
