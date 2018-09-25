#ifndef JSTD_CSVDATAPROCESSOR_H
#define JSTD_CSVDATAPROCESSOR_H
#include <vector>
/*
 basic interface for a Data Processor
 */
namespace jstd {
	template<typename InputDataType, typename ProcessedDataType>
	class DataProcessor {
		std::vector<InputDataType> m_csvData;
		std::shared_ptr<ProcessedDataType> m_processedData;
	public:
		DataProcessor() = default;

		DataProcessor(std::vector<InputDataType> &csvData);

		DataProcessor(std::vector<InputDataType> &&csvData);

		virtual ~DataProcessor() = default;

		virtual void processData() = 0;
	};

	template<typename CsvRowData, typename ProcessedDataType>
	DataProcessor<CsvRowData, ProcessedDataType>::DataProcessor(std::vector<CsvRowData> &&csvData):
		m_csvData(std::move(csvData)) {}

	template<typename CsvRowData, typename ProcessedDataType>
	DataProcessor<CsvRowData, ProcessedDataType>::DataProcessor(std::vector<CsvRowData> &csvData) {}
}  // end namespace jstd
#endif //JSTD_CSVDATAPROCESSOR_H
