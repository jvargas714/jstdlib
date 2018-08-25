#include "logger.h"
#include "net_types.h"

using namespace std;

int main () {
    jstd::NetConnection conn1;
    jstd::NetConnection conn2;
    conn1 = conn2;
    return 0;
}