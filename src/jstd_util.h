#ifndef JSTDLIB_JSTD_UTIL_H
#define JSTDLIB_JSTD_UTIL_H
#include <chrono>
#include <thread>
#include <bitset>

namespace jstd {
	namespace util {
		namespace chrono {  // time based helper methods
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

		namespace io {  // io utility functions files, sockets, etc etc
			bool goto_line(std::ifstream &str, size_t lineNo);
		}

		namespace bit {  // bitwise helper methods
			template<size_t nbits>
			std::bitset<nbits> intToBitSet(int64_t);
		}
	}
}
#endif //JSTDLIB_JSTD_UTIL_H
