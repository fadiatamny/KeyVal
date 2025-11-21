#pragma once

#include <list>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "../../types/Constants.hpp"
#include "../Block/Block.hpp"
#include "../DiskManager/DiskManager.hpp"

class BufferPoolException : public std::runtime_error {
public:
  explicit BufferPoolException(const std::string &message)
      : std::runtime_error(message) {}
};

class BufferPool {
public:
  BufferPool(size_t poolSize, std::unique_ptr<DiskManager> diskManager);

  BufferPool(const BufferPool &) = delete;
  BufferPool &operator=(const BufferPool &) = delete;
  BufferPool(BufferPool &&) = delete;
  BufferPool &operator=(BufferPool &&) = delete;

  ~BufferPool();

  Block *FetchBlock(BlockId blockId);
  Block *NewBlock();
  void ReleaseBlock(BlockId blockId, bool isDirty);
  void FlushBlock(BlockId blockId);
  void FlushAllBlocks();

private:
  size_t poolSize;
  std::unique_ptr<DiskManager> diskManager;

  std::vector<Block> pool;
  std::unordered_map<BlockId, size_t> blockTable;

  std::list<size_t> evictionList;
  std::unordered_map<size_t, std::list<size_t>::iterator> evictionListFrameIndices;
  std::vector<bool> isFree;

  size_t FindFreeOrEvictFrame();
  void PrepareFrameForReuse(size_t frameId);
  void MarkFrameInUse(size_t frameId);
  void RemoveFromEvictionList(size_t frameId);
};
