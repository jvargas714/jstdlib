#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

int main () {
	std::string msg = "hello world how art thou";
	std::vector<uint8_t> buff(msg.begin(), msg.end());
	for (auto ch : buff) {
		cout << ch << " " << endl;
	}
    return 0;
}

