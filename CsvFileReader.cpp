#include <fstream>
#include <memory>
#include <iostream>
#include <sstream>
#include "CsvFileReader.h"
using namespace jstd;

csv::CsvFileReader::CsvFileReader():
m_delim(','),
m_fileRead(false),
m_maxNumBytesCached(false),
m_lastLineRead(0) {}

csv::CsvFileReader::CsvFileReader(std::string filename, char delim):
	m_filename(std::move(filename)),
	m_delim(delim),
	m_fileRead(false),
	m_bytesRead(0),
	m_lastLineRead(0),
	m_maxNumBytesCached(false) {
	try {
		readFile();
	} catch (const csv::error::CsvFileReadError& e) {
		std::cerr << "there was an error reading in the file: " << m_filename << std::endl;
	}
}

void csv::CsvFileReader::clearCache() noexcept {
	m_rows.clear();
	m_maxNumBytesCached = false;
}

template<typename DataType>
DataType csv::CsvFileReader::getCell(int rowIndex, int colIndex) {
	if (rowIndex >= m_rows.size())
		throw error::CsvRowIndexOutOfBounds("row index is out of bounds");
	const Row& tmp = m_rows[rowIndex];
	return tmp.getVal<DataType>(colIndex);
}

template<typename DataType>
DataType csv::CsvFileReader::getVal(int rowIndex, std::string colName) {
	if (rowIndex >= m_rows.size())
		throw error::CsvRowIndexOutOfBounds("row index is out of bounds");
	const Row& row = m_rows[rowIndex];
	return row.getVal<DataType>(colName);
}

void csv::CsvFileReader::readFile(bool includeHeader) {
	std::ifstream fstrm(m_filename);
	std::vector<std::string> buffer;
	std::string line;
	if (fstrm.is_open()) {
		goToLine(fstrm, m_lastLineRead);
		if (includeHeader) {
			std::getline(fstrm, line);
			m_header = Row(line);
			m_lastLineRead++;
			std::cout << __FUNCTION__ << "bytes read: " << m_bytesRead << std::endl;
		}
		while (fstrm) {
			std::getline(fstrm, line);
			if (line.empty()) continue;
			m_bytesRead += line.size();
			std::cout << __FUNCTION__ << "(): bytes read: " << m_bytesRead << std::endl;
			if (m_bytesRead >= MAX_BUFFER_SIZE) {
				m_maxNumBytesCached = true;
				m_fileRead = false;
				return;
			}
			m_rows.emplace_back(line);
			m_lastLineRead++;
		}
	} else
		throw csv::error::CsvFileReadError(std::string(__FUNCTION__) + "CSV file not found");
	m_fileRead = true;
}

void csv::CsvFileReader::goToLine(std::ifstream& fstrm, int64_t lineNum) {
	for (auto i = 0; i < lineNum; i++)
		fstrm.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
