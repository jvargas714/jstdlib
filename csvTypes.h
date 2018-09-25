
#ifndef JSTD_CSVTYPES_H
#define JSTD_CSVTYPES_H
#include <stdexcept>
#include <string>

namespace jstd {
	namespace csv {
		constexpr int MAX_BUFFER_SIZE = 1024 * 1024;
		namespace error {
			class HeaderIndexOutOfBounds : public std::runtime_error {
			public:
				HeaderIndexOutOfBounds(int pos)
					: std::runtime_error(
					std::string("<CSV> :: Header position: " + std::to_string(pos) + " is out of bounds.")
				) {}
			};

			class CsvFileReadError : public std::runtime_error {
			public:
				explicit CsvFileReadError(std::string msg) : std::runtime_error(msg) {}
			};

			class CsvRowIndexOutOfBounds : public std::runtime_error {
			public:
				explicit CsvRowIndexOutOfBounds(std::string msg) : std::runtime_error(msg) {}
			};

			class CsvColumnIndexOutOfBounds : public std::runtime_error {
			public:
				explicit CsvColumnIndexOutOfBounds(std::string msg) : std::runtime_error(msg) {}
			};
		}  // namespace error
	}  // namespace csv
}
#endif //JSTD_CSVTYPES_H
