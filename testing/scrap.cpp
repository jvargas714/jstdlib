#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
#include <chrono>
#include <iostream>
#include <bitset>

using namespace std::chrono;
using namespace std;
typedef high_resolution_clock hrc;
typedef hrc::time_point t_point;

class MyClass {
public:
    MyClass() { cout << "reg ctor" << endl; }
    MyClass(const MyClass& cl) noexcept { cout << "cpy ctor" << endl; }
    MyClass(MyClass&& cl) noexcept { cout << "mv ctor" << endl; }
    MyClass& operator = (const MyClass& cl) noexcept { cout << "cpy op" << endl; return *this; }
    MyClass& operator = (MyClass&& cl) noexcept { cout << "mv op" << endl; return *this;}
    void funct() const { ; }
};

int foo(const std::vector<int>& data) {
    cout << "const ref" << endl;
    int val = 0;
    std::vector<int> tmp = data;
    for (auto el : tmp) { val += el; }
    return val;
}


int foo(std::vector<int>&& data) {
    cout << "rval ref" << endl;
    int val = 0;
    std::vector<int> tmp = std::move(data);
    for (auto el : tmp) {
        val += el;
    }
    return val;
}

//void foo(std::vector<int> data) {
//    cout << "rval ref" << endl;
//    for (auto el : data) {;}
//}

void testMove() {
    std::vector<int> data(1000000, 55);
//	MyClass data;
    t_point t1_bub, t2_bub;
//    cout << "calling const ref" << endl;
    t1_bub = hrc::now();
//    std::vector<int> data2(data);
//	std::vector<int> data2( std::move(data) );
    cout << foo(std::move(data)) << endl;
//    cout << foo(data) << endl;
    t2_bub = hrc::now();
    auto milli_sec = duration_cast<milliseconds>( t2_bub - t1_bub ).count();
    auto micro_sec = duration_cast<microseconds>( t2_bub - t1_bub ).count();
    std::cout << "time of execution-->\n" << milli_sec << "msec\n" << micro_sec << "usec\n\n" << std::endl;

//    cout << "calling rval ref" << endl;
//    t1_bub = hrc::now();
//    foo(std::move(data));
//    t2_bub = hrc::now();
//    milli_sec = duration_cast<milliseconds>( t2_bub - t1_bub ).count();
//    micro_sec = duration_cast<microseconds>( t2_bub - t1_bub ).count();
//    std::cout << "time of execution 2-->\n" << milli_sec << "msec\n" << micro_sec << "usec" << std::endl;
}

void bitsetTesting() {
    int64_t val = 16;
    std::bitset<8> bs(val);
    std::bitset<8> bs2(1);
    cout << val << ": " << bs << endl;
    cout << "\n" << endl;
    for (int i = 0; i < 1*8; i++) {  // index 0 is rightmost bit or least significant bit
        cout << bs[i];
    }
    cout << endl;
    bs = bs | bs2;
    cout << bs << endl;
}

int main () {
    bitsetTesting();
    return 0;
}

