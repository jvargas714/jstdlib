#ifndef JSTDLIB_CSVROW_H
#define JSTDLIB_CSVROW_H
#include <vector>
#include <string>
#include <sstream>
#include "csvTypes.h"
#include <algorithm>

namespace jstd {
    namespace csv {
        template<char delim = ','>
        class Row {
            std::vector<std::string> m_values;

        public:
            Row() = default;

            explicit Row(std::vector<std::string> data);

            Row(const Row &rw);

            Row(Row &&rw) noexcept;

            explicit Row(std::string rowStr);

            ~Row() = default;

            Row &operator=(const Row &rw);

            Row &operator=(Row &&rw) noexcept;

            template<typename DataType>
            DataType getVal(int colPos) const;

            template<typename DataType>
            void updateVal(int colPos, DataType newVal);

            inline size_t size() const noexcept { return m_values.size(); }

            std::string toString() const;

            std::vector<std::string> &values() noexcept { return m_values; }

            bool operator==(Row<delim> other) {
                return this->toString() == other.toString();
            }

        private:
            std::vector<std::string> split(std::string &&);
        };
    }

// ---------------------------------------------------implementation----------------------------------------------------
    template<char delim>
    csv::Row<delim>::Row(std::vector<std::string> values)
            : m_values(std::move(values)) {}

    template<char delim>
    csv::Row<delim>::Row(std::string rowStr) {
        m_values = split(std::move(rowStr));
    }

    template<char delim>
    csv::Row<delim>::Row(const csv::Row<delim> &rw) {
        m_values = rw.m_values;
    }

    template<char delim>
    csv::Row<delim>::Row(csv::Row<delim> &&rw) noexcept {
        m_values = rw.m_values;
    }

    template<char delim>
    std::vector<std::string> csv::Row<delim>::split(std::string &&line) {
        std::string tmp;
        std::vector<std::string> result;
        for (auto ch: line) {
            if (ch == delim) {
                result.push_back(tmp);
                tmp.clear();
            } else {
                tmp += ch;
            }
        }
        if (!tmp.empty()) result.push_back(tmp);
        return result;
    }

    template<char delim>
    std::string csv::Row<delim>::toString() const {
        std::stringstream ss;
        for (int i = 0; i < static_cast<int>(m_values.size()); i++) {
            if (i == m_values.size() - 1)
                ss << m_values[i];
            else
                ss << m_values[i] << delim;
        }
        return ss.str();
    }

    template<char delim>
    csv::Row<delim> &csv::Row<delim>::operator=(const csv::Row<delim> &rw) {
        m_values = rw.m_values;
        return *this;
    }

    template<char delim>
    csv::Row<delim> &csv::Row<delim>::operator=(csv::Row<delim> &&rw) noexcept {
        m_values = rw.m_values;
        return *this;
    }

    template<char delim>
    template<typename DataType>
    DataType csv::Row<delim>::getVal(int colPos) const {
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

    template<char delim>
    template<typename DataType>
    void csv::Row<delim>::updateVal(int colPos, DataType newVal) {
        if (colPos >= m_values.size())
            throw csv::error::CsvColumnIndexOutOfBounds(
                    "column is out of bounds, there are " +
                    std::to_string(m_values.size()) + " columns"
            );
        m_values[colPos] = std::to_string(newVal);
    }  // namespace csv
} // namespace jstd

#endif //JSTDLIB_CSVROW_H
