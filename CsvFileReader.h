#ifndef JSTDLIB_CSVFILEREADER_H
#define JSTDLIB_CSVFILEREADER_H
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "CsvRow.h"
#include "csvTypes.h"
#include "csvUtil.h"
/*
 * Description:
 *  A simple CSV File Reader Class to read input file in chunks specified by a default MAX_BUFFER_SIZE (1MB)
 *  passed as a template parameter.
 *  If max buff size is reached then last line number read is saved, but file reading stops. Cached CSV data can be moved out
 *  and the cached cleared when appropriate to do so. Calling readChunk will resume file read/cache. If includeHeader is set
 *  to true then it will be assumed that the first Row read in will be the row names. By default it is assumed the
 *  first row is data.
 *
 *  Policies:
 *  As data from the csv file is being cached various policies are applied to each row, by default each row will have
 *  quotes removed, as well as tabs and spaces.
 *  Policy Types must implement void execute(std::string&) method
 */
namespace jstd {
    namespace csv {
        template<
                class RowType,
                class DoubleQuotePolicy=csv::policy::RemoveDoubleQuotes,
                class SingleQuotePolicy=csv::policy::RemoveSingleQuotes,
                uint64_t MAX_BUFFER_SIZE = DEFAULT_MAX_CACHE_SIZE,
                bool includeHeader = false>
        class CsvFileReader {
            static constexpr int NO_LINES_READ = -1;
            std::string m_filename;
            std::vector<RowType> m_rows;
            RowType m_header;
            bool m_fileRead;
            int64_t m_lastLineRead;
            size_t m_bytesRead;
            bool m_maxNumBytesCached;

        public:
            CsvFileReader() : m_fileRead(false),
                              m_maxNumBytesCached(false),
                              m_lastLineRead(-1),
                              m_bytesRead(0),
                              m_filename() {}

            explicit CsvFileReader(std::string filename) :
                    m_filename(std::move(filename)),
                    m_fileRead(false),
                    m_bytesRead(0),
                    m_lastLineRead(-1),
                    m_maxNumBytesCached(false) {}

            ~CsvFileReader() = default;

            template<typename DataType>
            DataType getCell(int rowIndex, int colIndex) {
                if (rowIndex >= m_rows.size())
                    throw error::CsvRowIndexOutOfBounds("row index is out of bounds");
                const RowType &tmp = m_rows[rowIndex];
                return tmp.template getVal<DataType>(colIndex);
            }

            template<typename DataType>
            DataType getVal(int rowIndex, const std::string &colName) const {
                if (rowIndex >= m_rows.size())
                    throw error::CsvRowIndexOutOfBounds("row index is out of bounds");
                const RowType &row = m_rows[rowIndex];
                return row.template getVal<DataType>(colName);
            }

            inline bool isFullyRead() const noexcept { return m_fileRead; }

            // read chunk of file upto MAX_BUFFER_SIZE bytes, return true if whole file is read in and cached
            bool readChunk() {
                std::fstream fstrm(m_filename);
                std::string line;
                if (fstrm.is_open()) {
                    csv::util::goToLine(fstrm, m_lastLineRead + 1);
                    if (includeHeader && m_lastLineRead == NO_LINES_READ) {
                        std::getline(fstrm, line);
                        employPolicies(line);
                        m_header = RowType(line);
                        m_lastLineRead++;
                    }
                    while (fstrm) {
                        std::getline(fstrm, line);
                        if (line.empty()) continue;
                        employPolicies(line);
                        m_bytesRead += line.size();
                        if (m_bytesRead >= MAX_BUFFER_SIZE) {
                            m_maxNumBytesCached = true;
                            m_fileRead = false;
                            return false;
                        }
                        m_rows.emplace_back(line);
                        m_lastLineRead++;
                    }
                } else
                    throw csv::error::CsvFileReadError(std::string(__FUNCTION__) + "CSV file not found");
                std::cout << __FUNCTION__ << "(): bytes read: " << m_bytesRead << ", file read" << std::endl;
                m_fileRead = true;
                return true;
            }

            inline std::vector<RowType> getRows() const noexcept { return m_rows; }

            inline RowType getHeader() const noexcept { return m_header; }

            void clearCache() noexcept {
                m_rows.clear();
                m_maxNumBytesCached = false;
                m_bytesRead = 0;
            }

        private:
            void employPolicies(std::string &str) {
                SingleQuotePolicy::execute(str);
                DoubleQuotePolicy::execute(str);
            }
        };
    }  // end namespace csv
}
#endif //JSTDLIB_CSVFILEREADER_H
