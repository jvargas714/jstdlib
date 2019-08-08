#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
#include <chrono>
#include <iostream>

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

void foo(const std::vector<int>& data) {
    cout << "const ref" << endl;
    for (const auto& el : data) {;}
}
//
//void foo(std::vector<int>&& data) {
//    cout << "rval ref" << endl;
//    for (auto el : data) {;}
//}

//void foo(std::vector<int> data) {
//    cout << "rval ref" << endl;
//    for (auto el : data) {;}
//}


int main () {
	 std::vector<int> data(1000000, 55);
//	MyClass data;
    t_point t1_bub, t2_bub;
//    cout << "calling const ref" << endl;
    t1_bub = hrc::now();
	std::vector<int> data2(std::move(data));
	t2_bub = hrc::now();
    auto milli_sec = duration_cast<milliseconds>( t2_bub - t1_bub ).count();
    auto micro_sec = duration_cast<microseconds>( t2_bub - t1_bub ).count();
    std::cout << "time of execution 1-->\n" << milli_sec << "msec\n" << micro_sec << "usec\n\n" << std::endl;

//    cout << "calling rval ref" << endl;
//    t1_bub = hrc::now();
//    foo(std::move(data));
//    t2_bub = hrc::now();
//    milli_sec = duration_cast<milliseconds>( t2_bub - t1_bub ).count();
//    micro_sec = duration_cast<microseconds>( t2_bub - t1_bub ).count();
//    std::cout << "time of execution 2-->\n" << milli_sec << "msec\n" << micro_sec << "usec" << std::endl;
    return 0;
}

