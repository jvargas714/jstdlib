#ifndef JSTDLIB_JSTD_UTIL_H
#define JSTDLIB_JSTD_UTIL_H
#include "jstd_util.h"
#include <chrono>
#include <thread>

namespace jstd {
    template<typename durationType>
    void sleep(int duration_units) {
        std::this_thread::sleep_for(durationType(duration_units));
    }
    void sleep_milli(int milli) {
        sleep<std::chrono::milliseconds>(milli);
    }
    void sleep_micro(int micro) {
        sleep<std::chrono::microseconds>(micro);
    }
    void sleep_nano(int nano) {
        sleep<std::chrono::nanoseconds>(nano);
    }
}
#endif //JSTDLIB_JSTD_UTIL_H
