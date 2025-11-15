#include "Models/MyClass/MyClass.hpp"
#include <iostream>

MyClass::MyClass(const std::string& msg) : message(msg) {}
void MyClass::print() const { std::cout << message << std::endl; }
std::string MyClass::get_message() const { return message; }
