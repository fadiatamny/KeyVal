#pragma once
#include <string>

class MyClass {
private:
  std::string message;

public:
  explicit MyClass(const std::string &msg);
  void print() const;
  std::string get_message() const;
};
