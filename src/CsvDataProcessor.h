#ifndef JSTDLIB_CSVDATAPROCESSOR_H
#define JSTDLIB_CSVDATAPROCESSOR_H
#include <string>
#include <map>
#include <iostream>
#include "DataProcessor.h"
#include "CsvRow.h"
#include "csvTypes.h"

/*
 * General csv data processor that will add a new entry to the data aggregation map and
 * update an existing entry. Derivced class of CsvDataProcessor will specify what
 * occurs during new entry creation and entry update by implementing:
 *      OutputDataType createNewEntry(const RowDataType& inputRow)
 * and,
 *      void updateEntry(const RowDataType& inputRow)
 *
 *  template types:
 *      RowDataType :: data structure representing a csv row entry
 *          public interface:
 *              std::vector<string> values() :: returns a vector of strings representing that csv data line
 *
 *      OutputDataType<class OrderPolicy> :: data structure representing the aggegration of the processed data,
 *          a container whose ordering scheme can be specified
 *          public interface:
 *              find(const std::string&) :: search for entry by a key
 *              OutputDataType::iterator begin() :: front iterator
 *              OutputDataType::iterator end() :: returns iterator pointing to end
 *              void add(const std::string& key, ValueType&&) :: store key value
 *              void clear() :: remove all entries
 */

namespace jstd {
    namespace csv {
        typedef std::string RowKey;
        template<class RowDataType,
            class OutputDataType,
            int sortByColumnIndex=0>
        class CsvDataProcessor : public DataProcessor<RowDataType, OutputDataType> {

        protected:
            using DataProcessor<RowDataType, OutputDataType>::m_data;
            using DataProcessor<RowDataType, OutputDataType>::m_processedData;

        public:
            CsvDataProcessor() = default;
            ~CsvDataProcessor() = default;

            /*
             * processedData implements DataProcessorOutput interface
             * this method takes a row from m_data and adds it to processedData or updates it
             * implementing createNewEntry and updateEntry handles processing rows
             * of course processData can be implemented as well
             */
            virtual void processData() {
                for (auto inputEntry : m_data) {
                    RowKey key = inputEntry.values()[sortByColumnIndex];
                    auto entry = m_processedData.find(key);
                    if (entry == m_processedData.end()) {
                        m_processedData.add(key, createNewEntry(inputEntry));
                    } else {
                        updateEntry(inputEntry);
                    }
                }
            }

        virtual void clearCache() { m_data.clear(); }

        private:
            virtual typename OutputDataType::DataType createNewEntry(const RowDataType& inputRow)=0;
            virtual void updateEntry(const RowDataType& inputRow)=0;
        };
    }  // namespace csv
} // namespace jstd
#endif //JSTDLIB_CSVDATAPROCESSOR_H
