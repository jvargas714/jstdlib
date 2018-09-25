#ifndef JSTD_CSVFILEREADER_H
#define JSTD_CSVFILEREADER_H
#include <string>
#include <fstream>
#include <vector>
#include "CsvRow.h"
#include "csvTypes.h"

namespace jstd {
	namespace csv {
		class CsvFileReader {
			std::string m_filename;
			char m_delim;
			std::vector<Row> m_rows;
			Row m_header;
			bool m_fileRead;
			int64_t m_lastLineRead;
			size_t m_bytesRead;
			bool m_maxNumBytesCached;

		public:
			CsvFileReader();

			explicit CsvFileReader(std::string filename, char delim = ',');

			~CsvFileReader() = default;

			inline void setDelim(char delim) { m_delim = delim; }

			void clearCache() noexcept;

			template<typename DataType>
			DataType getCell(int row, int col);

			template<typename DataType>
			DataType getVal(int rowIndex, std::string colName);

			void readFile(bool includeHeader = true);

			inline bool isFileRead() { return m_fileRead; }

		private:
			void goToLine(std::ifstream &, int64_t lineNum);

			void removeChars(std::string &str, char toRemove);
		};
	}  // end namespace csv
} // end namespace jstd
#endif //JSTD_CSVFILEREADER_H
