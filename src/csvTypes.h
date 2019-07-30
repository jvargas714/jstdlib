
#ifndef JSTDLIB_CSVTYPES_H
#define JSTDLIB_CSVTYPES_H
#include <stdexcept>
#include <string>
#include <algorithm>
/*
 * jdebug :: write out tests for csv types
 */
namespace jstd {
    namespace csv {
        constexpr char DEFAULT_CSV_OUTPUT_FILENAME[] = "output.csv";
        constexpr uint64_t DEFAULT_MAX_CACHE_SIZE = (1024*1024*100);
        namespace error {
            class HeaderIndexOutOfBounds : public std::runtime_error {
            public:
                HeaderIndexOutOfBounds(int pos)
                : std::runtime_error(
                    std::string("<CSV> :: Header position: " + std::to_string(pos) + " is out of bounds.")
                    ) {}
            };

            class CsvFileReadError : public std::runtime_error {
                public: explicit CsvFileReadError(std::string msg): std::runtime_error(msg) {}
            };

            class CsvFileWriteError : public std::runtime_error {
            public: explicit CsvFileWriteError(std::string msg): std::runtime_error(msg) {}
            };

            class CsvRowIndexOutOfBounds : public std::runtime_error {
            public: explicit CsvRowIndexOutOfBounds(std::string msg): std::runtime_error(msg) {}
            };

            class CsvColumnIndexOutOfBounds: public std::runtime_error {
            public: explicit CsvColumnIndexOutOfBounds(std::string msg): std::runtime_error(msg) {}
            };
        }  // namespace error

        // Policy is how csv row strings are handled, Policy type class implement a static execute(std::string&) function
        namespace policy {
            class RemoveDoubleQuotes {
            public:
                static void execute(std::string& str) { str.erase(std::remove(str.begin(), str.end(), '"'), str.end()); };
            };

            class RemoveSingleQuotes {
            public:
                static void execute(std::string& str) { str.erase(std::remove(str.begin(), str.end(), '\''), str.end()); }
            };

            template<char ... toBeRemoved>
            class TrimChars {
                constexpr static bool leftToTrim() { return false; }

                template<class ...OtherCharsToTrim>
                constexpr static bool leftToTrim(char ch, OtherCharsToTrim... toBeTrimmed) { return true; }

                template<class ...OtherCharsToTrim>
                static void _remove(std::string& str, char ch, OtherCharsToTrim...otherCharsToTrim) {
                    str.erase(
                        std::remove(str.begin(), str.end(), ch),
                        str.end()
                        );
                    _remove(str, toBeRemoved...);
                }

                static void _remove(std::string& str, char ch) {
                    str.erase(
                        std::remove(str.begin(), str.end(), ch),
                        str.end()
                    );
                }

            public:
                static void execute(std::string& str) {
                    _remove(str, toBeRemoved...);
                }
            };
        }  // namespace policy
    }  // namespace csv
} // namespace jstd
#endif //JSTDLIB_CSVTYPES_H
