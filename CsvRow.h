#ifndef JSTD_CSVROW_H
#define JSTD_CSVROW_H
#include <vector>
#include <string>

namespace jstd {
	namespace csv {
		class Row {
			std::vector<std::string> m_values;
			std::string m_rowStr;
			char m_delim;
		public:
			Row() = default;

			Row(std::vector<std::string> data);

			Row(const Row &rw);

			Row(Row &&rw) noexcept;

			explicit Row(std::string rowStr);

			~Row() = default;

			Row &operator=(const Row &rw);

			Row &operator=(Row &&rw) noexcept;

			inline void setDelimiter(char delim) { m_delim = delim; }

			template<typename DataType>
			DataType getVal(int colPos);

			inline size_t size() const noexcept { return m_values.size(); }

			std::string toString() const;

			inline void setDelim(char delim) { m_delim = delim; }

		private:
			std::vector<std::string> split(std::string &&);

			void removeChars(std::string &str, char toRemove);

		};

		std::ostream &operator<<(std::ostream &os, const Row &row);

		// template specializations
		template<>
		int csv::Row::getVal(int);
	} // end namespace csv
}  // end namespace jstd
#endif //JSTD_CSVROW_H
