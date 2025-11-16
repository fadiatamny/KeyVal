#pragma once
#include "../../types/Constants.hpp"

struct Block {
  char data[BLOCK_SIZE];
  BlockId block_id;
  int referenceCount;
  bool isDirty;

  Block(const Block &) = delete;
  Block &operator=(const Block &) = delete;
  Block(Block &&) = delete;
  Block &operator=(Block &&) = delete;
};
