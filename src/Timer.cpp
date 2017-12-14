#include "util/Timer.hpp"
double time_as_second(const Time_t &t) { return t.count() / 1.0e9; }
double time_as_millsecond(const Time_t &t) { return t.count() / 1.0e6; }


