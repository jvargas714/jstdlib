#include <fstream>
#include "jstd_util.h"


bool jstd::util::io::goto_line(std::ifstream& strm, size_t lineNo) {
	if (!strm.is_open()) {
		return false;
	}
	strm.seekg(0);
	strm.seekg(lineNo);
	return true;
}
