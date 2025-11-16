#include "./BufferPool.hpp"

BufferPool::BufferPool(size_t poolSize, std::unique_ptr<DiskManager> diskManager)
    : poolSize(poolSize), diskManager(std::move(diskManager)),
      pool(poolSize), isFree(poolSize, true) {
  // Initialize all frames as free
  // blockTable and evictionList start empty
}

BufferPool::~BufferPool() {
  // Flush all dirty blocks before destroying the pool
  FlushAllBlocks();
}

Block *BufferPool::FetchBlock(BlockId blockId) {
  // TODO: Implement block fetching logic
  // Steps:
  // 1. Check if blockId exists in blockTable
  //    - If YES: block is already in pool
  //      a. Get the frameId from blockTable
  //      b. Increment block's referenceCount
  //      c. Update eviction list (move this frame to indicate recent use)
  //      d. Return pointer to the block
  //    - If NO: block needs to be loaded from disk
  //      a. Find a free frame or evict one (call FindFreeOrEvictFrame)
  //      b. Prepare the frame for reuse (call PrepareFrameForReuse)
  //      c. Load block data from disk using diskManager->ReadBlock()
  //      d. Set block's block_id and initialize referenceCount to 1
  //      e. Set block's isDirty to false (freshly loaded)
  //      f. Add blockId -> frameId mapping to blockTable
  //      g. Mark frame as in use (call MarkFrameInUse)
  //      h. Return pointer to the block

  return nullptr;  // TODO: Replace with actual implementation
}

Block *BufferPool::NewBlock() {
  // TODO: Implement new block allocation
  // Steps:
  // 1. Allocate a new block on disk using diskManager->AllocateBlock()
  //    - This returns a new BlockId
  // 2. Find a free frame or evict one (call FindFreeOrEvictFrame)
  // 3. Prepare the frame for reuse (call PrepareFrameForReuse)
  // 4. Initialize the block:
  //    - Set block_id to the new BlockId
  //    - Set referenceCount to 1
  //    - Set isDirty to false
  //    - Zero out the data array (new blocks are empty)
  // 5. Add blockId -> frameId mapping to blockTable
  // 6. Mark frame as in use (call MarkFrameInUse)
  // 7. Return pointer to the block

  return nullptr;  // TODO: Replace with actual implementation
}

void BufferPool::ReleaseBlock(BlockId blockId, bool isDirty) {
  // Check if block exists in the pool
  if (blockTable.find(blockId) == blockTable.end()) {
    throw BufferPoolException("Attempting to release block not in pool: " +
                             std::to_string(blockId));
  }

  // Get the frame
  size_t frameId = blockTable[blockId];
  Block *block = &pool[frameId];

  // Decrement reference count
  block->referenceCount--;

  if (block->referenceCount < 0) {
    throw BufferPoolException("Reference count went negative for block: " +
                             std::to_string(blockId));
  }

  // Update dirty flag if the caller marked it dirty
  if (isDirty) {
    block->isDirty = true;
  }

  // TODO: If referenceCount is now 0, add frame back to eviction list
  // (Only blocks with referenceCount == 0 can be evicted)
}

void BufferPool::FlushBlock(BlockId blockId) {
  // Check if block exists in the pool
  if (blockTable.find(blockId) == blockTable.end()) {
    // Block not in pool, nothing to flush
    return;
  }

  // Get the frame
  size_t frameId = blockTable[blockId];
  Block *block = &pool[frameId];

  // Only flush if dirty
  if (!block->isDirty) {
    return;
  }

  // Write block data to disk
  diskManager->WriteBlock(blockId, block->data);

  // Mark as clean
  block->isDirty = false;
}

void BufferPool::FlushAllBlocks() {
  // Iterate through all blocks in the pool and flush dirty ones
  for (const auto &entry : blockTable) {
    BlockId blockId = entry.first;
    FlushBlock(blockId);
  }

  // Sync the disk to ensure all writes are persisted
  diskManager->SyncFile();
}

size_t BufferPool::FindFreeOrEvictFrame() {
  // TODO: Implement frame finding/eviction logic
  // Steps:
  // 1. First, try to find a free frame (fast path):
  //    - Iterate through isFree vector
  //    - If isFree[i] is true, return i
  // 2. If no free frames, must evict (slow path):
  //    - Loop through evictionList to find a frame with referenceCount == 0
  //      (You can only evict blocks that are not currently in use)
  //    - When you find one:
  //      a. If the block is dirty, flush it first (call FlushBlock)
  //      b. Remove it from evictionList
  //      c. Return the frameId
  // 3. If no frames can be evicted (all have referenceCount > 0):
  //    - Throw BufferPoolException - pool is full and all blocks are in use

  throw BufferPoolException("FindFreeOrEvictFrame not implemented");
}

void BufferPool::PrepareFrameForReuse(size_t frameId) {
  // Get the block at this frame
  Block *block = &pool[frameId];

  // If this frame was previously used, remove it from blockTable
  if (blockTable.find(block->block_id) != blockTable.end()) {
    blockTable.erase(block->block_id);
  }

  // Clear the block data
  std::memset(block->data, 0, BLOCK_SIZE);
  block->block_id = 0;
  block->referenceCount = 0;
  block->isDirty = false;

  // Mark frame as free (will be marked in-use by caller after setup)
  isFree[frameId] = true;
}

void BufferPool::MarkFrameInUse(size_t frameId) {
  // Mark the frame as occupied
  isFree[frameId] = false;

  // TODO: Add frameId to eviction list
  // (Decide where to add it based on your LRU policy - front or back)
}

void BufferPool::RemoveFromEvictionList(size_t frameId) {
  // TODO: Remove frameId from evictionList
  // You'll need to find the frameId in the list and erase it
  // Hint: Use std::find and list.erase()
}
