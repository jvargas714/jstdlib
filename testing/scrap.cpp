#include "net_types.h"
#include <arpa/inet.h>
#include "jstd_util.h"
#include <iostream>

using namespace std;

int main () {
	in_addr addr;
	addr.s_addr= -1;

	cout << std::string(inet_ntoa(addr)) << endl;
    return 0;
}

