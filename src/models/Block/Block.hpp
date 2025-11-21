#pragma once
#include "../../types/Constants.hpp"
#include <cstring>

class Block {
public:
  char data[BLOCK_SIZE];
  BlockId block_id;
  int referenceCount;
  bool isDirty;

  Block() : block_id(0), referenceCount(0), isDirty(false) {
    std::memset(data, 0, BLOCK_SIZE);
  }

  Block(const Block &) = delete;
  Block &operator=(const Block &) = delete;
  Block(Block &&) = delete;
  Block &operator=(Block &&) = delete;
};