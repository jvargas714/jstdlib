#include "CsvRow.h"
#include <sstream>
#include <utility>
#include <string>
#include "csvTypes.h"

jstd::csv::Row::Row(std::vector<std::string> values)
: m_values(std::move(values)), m_delim(',') {}

jstd::csv::Row::Row(std::string line) {
	removeChars(line, '"');
	m_values = split(std::move(line));
}

jstd::csv::Row::Row(const csv::Row &rw) {
	m_values = rw.m_values;
	m_delim = rw.m_delim;
}

jstd::csv::Row::Row(csv::Row &&rw) noexcept {
	m_values = rw.m_values;
	m_delim = rw.m_delim;
}

std::vector<std::string> jstd::csv::Row::split(std::string &&line) {
	std::string tmp;
	std::vector<std::string> result;
	for (auto ch: line) {
		if (ch == m_delim) {
			result.push_back(tmp);
			tmp.clear();
		} else {
			tmp += ch;
		}
	}
	if (!tmp.empty()) result.push_back(tmp);
	return result;
}


std::string jstd::csv::Row::toString() const {
	std::stringstream ss;
	for (int i = 0; i < static_cast<int>(m_values.size()); i++) {
		if (i == m_values.size()-1)
			ss << m_values[i];
		else
			ss << m_values[i] << m_delim;
	}
	return ss.str();
}

jstd::csv::Row &jstd::csv::Row::operator=(const csv::Row &rw) {
	m_values = rw.m_values;
	m_delim = rw.m_delim;
	return *this;
}

jstd::csv::Row &jstd::csv::Row::operator=(csv::Row &&rw) noexcept {
	m_values = rw.m_values;
	m_delim = rw.m_delim;
	return *this;
}

template<typename DataType>
DataType jstd::csv::Row::getVal(int colPos) {
	if (colPos >= m_values.size())
		throw csv::error::CsvColumnIndexOutOfBounds(
			"column is out of bounds, there are " +
			std::to_string(m_values.size()) + " columns"
			);
	DataType data;
	std::stringstream ss;
	ss << m_values[colPos];
	ss >> data;
	return data;
}

std::ostream& operator << (std::ostream& os, const jstd::csv::Row& row) {
	os << row.toString();
	return os;
}

void jstd::csv::Row::removeChars(std::string &str, char toRemove) {
	str.erase(
		std::remove(str.begin(), str.end(), '"'),
		str.end());
}

