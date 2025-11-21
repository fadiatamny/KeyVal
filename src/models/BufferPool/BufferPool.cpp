#include "./BufferPool.hpp"
#include <cstring>

BufferPool::BufferPool(size_t poolSize,
                       std::unique_ptr<DiskManager> diskManager)
    : poolSize(poolSize), diskManager(std::move(diskManager)), pool(poolSize),
      isFree(poolSize, true) {}

BufferPool::~BufferPool() { this->FlushAllBlocks(); }

Block *BufferPool::FetchBlock(BlockId blockId) {
  auto tableEntry = this->blockTable.find(blockId);
  if (tableEntry != this->blockTable.end()) {
    size_t frameId = tableEntry->second;
    Block *block = &this->pool[frameId];
    block->referenceCount++;
    this->RemoveFromEvictionList(frameId);
    this->evictionList.push_back(frameId);
    this->evictionListFrameIndices[frameId] = --this->evictionList.end();
    return block;
  }

  size_t frameId = this->FindFreeOrEvictFrame();
  this->PrepareFrameForReuse(frameId);
  Block *block = &this->pool[frameId];
  this->diskManager->ReadBlock(blockId, block->data);
  block->block_id = blockId;
  block->referenceCount = 1;
  block->isDirty = false;
  this->blockTable[blockId] = frameId;
  this->MarkFrameInUse(frameId);
  return block;
}

Block *BufferPool::NewBlock() {
  BlockId newBlockId = this->diskManager->AllocateBlock();
  size_t frameId = this->FindFreeOrEvictFrame();
  this->PrepareFrameForReuse(frameId);

  Block *block = &this->pool[frameId];
  block->block_id = newBlockId;
  block->referenceCount = 1;
  block->isDirty = false;
  this->blockTable[newBlockId] = frameId;
  this->MarkFrameInUse(frameId);
  return block;
}

void BufferPool::ReleaseBlock(BlockId blockId, bool isDirty) {
  if (this->blockTable.find(blockId) == this->blockTable.end()) {
    throw BufferPoolException("Attempting to release block not in pool: " +
                              std::to_string(blockId));
  }

  size_t frameId = this->blockTable[blockId];
  Block *block = &this->pool[frameId];

  block->referenceCount--;

  if (block->referenceCount < 0) {
    throw BufferPoolException("Reference count went negative for block: " +
                              std::to_string(blockId));
  }

  if (isDirty) {
    block->isDirty = true;
  }
}

void BufferPool::FlushBlock(BlockId blockId) {
  if (this->blockTable.find(blockId) == this->blockTable.end()) {
    return;
  }

  size_t frameId = this->blockTable[blockId];
  Block *block = &this->pool[frameId];

  if (!block->isDirty) {
    return;
  }

  this->diskManager->WriteBlock(blockId, block->data);

  block->isDirty = false;
}

void BufferPool::FlushAllBlocks() {
  for (const auto &entry : this->blockTable) {
    BlockId blockId = entry.first;
    this->FlushBlock(blockId);
  }

  this->diskManager->SyncFile();
}

size_t BufferPool::FindFreeOrEvictFrame() {
  for (size_t i = 0; i < this->poolSize; ++i) {
    if (this->isFree[i]) {
      return i;
    }
  }

  for (auto it = this->evictionList.begin(); it != this->evictionList.end();
       ++it) {
    auto frameId = *it;
    Block *block = &this->pool[frameId];

    if (block->referenceCount == 0) {
      if (block->isDirty) {
        this->FlushBlock(block->block_id);
      }

      this->blockTable.erase(block->block_id);
      this->evictionList.erase(it);
      this->evictionListFrameIndices.erase(frameId);
      return frameId;
    }
  }

  throw BufferPoolException(
      "Pool full - could not find free space, all blocks are in use");
}

void BufferPool::PrepareFrameForReuse(size_t frameId) {
  Block *block = &this->pool[frameId];

  std::memset(block->data, 0, BLOCK_SIZE);
  block->block_id = 0;
  block->referenceCount = 0;
  block->isDirty = false;

  this->isFree[frameId] = true;
}

void BufferPool::MarkFrameInUse(size_t frameId) {
  this->isFree[frameId] = false;
  this->evictionList.push_back(frameId);
  this->evictionListFrameIndices[frameId] = --this->evictionList.end();
}

void BufferPool::RemoveFromEvictionList(size_t frameId) {
  auto posIt = this->evictionListFrameIndices.find(frameId);
  if (posIt != this->evictionListFrameIndices.end()) {
    this->evictionList.erase(posIt->second);
    this->evictionListFrameIndices.erase(posIt);
  }
}
