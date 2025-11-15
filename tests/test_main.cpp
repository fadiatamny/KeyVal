#include "Models/MyClass/MyClass.hpp"
#include <cassert>
#include <iostream>

int main() {
    MyClass TestObj("test message");
    assert(TestObj.get_message() == "test message");
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
