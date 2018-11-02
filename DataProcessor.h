#ifndef _JSTDLIB_DATAPROCESSOR_H
#define _JSTDLIB_DATAPROCESSOR_H
#include <vector>
#include <memory>

/*
 * jdebug :: come up with maybe a different type of data structure for m_data
 * Generic Data Processing class:
 *      - has a vector representing m_data to be processed
 *      - contains a processed data  data structure
 *      - InputDataType should be moveable
 *      Virtual methods:
 *          void processData()
 * extend this class and implement processData
 *
 * */
namespace jstd {
    template<class InputDataType, class ProcessedDataType>
    class DataProcessor {
    protected:
        std::vector<InputDataType> m_data;
        ProcessedDataType m_processedData;

    public:
        DataProcessor() = default;

        DataProcessor(std::vector<InputDataType> &data);

        DataProcessor(std::vector<InputDataType> &&data);

        virtual ~DataProcessor() = default;

        void loadData(std::vector<InputDataType> &&data) { m_data = std::move(data); }

        void loadData(std::vector<InputDataType> &data) { m_data = data; }

        ProcessedDataType getProcessedData() { return m_processedData; }

        virtual void processData() = 0;
    };
}  // namespace jstd
template<typename InputDataType, typename ProcessedDataType>
jstd::DataProcessor<InputDataType, ProcessedDataType>::DataProcessor(std::vector<InputDataType> &&data):
m_data(std::move(data)) {}

template<typename InputDataType, typename ProcessedDataType>
jstd::DataProcessor<InputDataType, ProcessedDataType>::DataProcessor(std::vector<InputDataType> &data) {}

#endif //_JSTDLIB_DATAPROCESSOR_H
