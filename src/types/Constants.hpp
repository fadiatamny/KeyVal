#pragma once

#include <cstdint>
using BlockId = uint32_t;
constexpr int BLOCK_SIZE = 4096; // 4KB
constexpr BlockId INVALID_BLOCK_ID = static_cast<BlockId>(-1);
