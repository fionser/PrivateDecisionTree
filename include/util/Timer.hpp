#ifndef UTIL_TIMER_HPP
#define UTIL_TIMER_HPP
#include <chrono>

class Timer {
public:
    using Time_t = std::chrono::nanoseconds;
    using Clock = std::chrono::high_resolution_clock;

    Timer(double *save_to) : save_to_(save_to) { stamp_ = Clock::now(); }
    
    ~Timer() { 
        auto end = Clock::now();
        if (save_to_)
            *save_to_ = as_millsecond(end - stamp_);
    }

    double as_millsecond(const Time_t &t) const { return t.count() / 1.0e6; }

private:
    Clock::time_point stamp_;
    double *save_to_;
};
#endif //SYMRLWE_TIMER_HPP
