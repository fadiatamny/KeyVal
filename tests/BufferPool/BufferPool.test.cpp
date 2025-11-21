#include "../../src/models/BufferPool/BufferPool.hpp"
#include "../../src/models/DiskManager/DiskManager.hpp"

#include <cassert>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <system_error>

using namespace std::string_literals;
namespace fs = std::filesystem;

static std::string make_temp_db_path() {
  auto tmp = fs::temp_directory_path();
  auto now =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::random_device rd;
  std::mt19937_64 eng(rd());
  std::uniform_int_distribution<uint64_t> dist;
  uint64_t r = dist(eng);
  std::string filename = "keyval_test_bufferpool_" + std::to_string(now) + "_" +
                         std::to_string(r) + ".db";
  return (tmp / filename).string();
}

static void safe_remove(const std::string &path) {
  std::error_code ec;
  fs::remove(path, ec);
  (void)ec;
}

static void test_fetch_block_not_in_pool() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);

    BlockId id = dm->AllocateBlock();
    char write_data[BLOCK_SIZE];
    std::memset(write_data, 'A', BLOCK_SIZE);
    dm->WriteBlock(id, write_data);

    BufferPool pool(3, std::move(dm));

    Block *block = pool.FetchBlock(id);
    assert(block != nullptr && "FetchBlock should return non-null block");
    assert(block->block_id == id && "Block should have correct block_id");
    assert(block->referenceCount == 1 &&
           "Block should have referenceCount of 1");
    assert(!block->isDirty && "Freshly loaded block should not be dirty");
    assert(block->data[0] == 'A' && "Block data should be loaded from disk");

    pool.ReleaseBlock(id, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_fetch_block_already_in_pool() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BlockId id = dm->AllocateBlock();

    BufferPool pool(3, std::move(dm));

    Block *block1 = pool.FetchBlock(id);
    assert(block1->referenceCount == 1);

    Block *block2 = pool.FetchBlock(id);
    assert(block2 == block1 && "Should return same block pointer");
    assert(block2->referenceCount == 2 && "Reference count should increment");

    pool.ReleaseBlock(id, false);
    assert(block1->referenceCount == 1);

    pool.ReleaseBlock(id, false);
    assert(block1->referenceCount == 0);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_new_block() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(3, std::move(dm));

    Block *block = pool.NewBlock();
    assert(block != nullptr && "NewBlock should return non-null block");
    assert(block->referenceCount == 1 &&
           "New block should have referenceCount of 1");
    assert(!block->isDirty && "New block should not be dirty");

    bool allZero = true;
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
      if (block->data[i] != 0) {
        allZero = false;
        break;
      }
    }
    assert(allZero && "New block data should be zeroed");

    pool.ReleaseBlock(block->block_id, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_release_block_marks_dirty() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(3, std::move(dm));

    Block *block = pool.NewBlock();
    assert(!block->isDirty && "New block should not be dirty initially");

    std::memset(block->data, 'X', BLOCK_SIZE);
    pool.ReleaseBlock(block->block_id, true);

    assert(block->isDirty &&
           "Block should be marked dirty after release with isDirty=true");
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_eviction_when_pool_full() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(2, std::move(dm));

    Block *block0 = pool.NewBlock();
    BlockId id0 = block0->block_id;
    pool.ReleaseBlock(id0, false);

    Block *block1 = pool.NewBlock();
    BlockId id1 = block1->block_id;
    pool.ReleaseBlock(id1, false);

    Block *block2 = pool.NewBlock();
    BlockId id2 = block2->block_id;

    assert(block2 != nullptr &&
           "Should be able to allocate block even when pool is full");
    assert(block2->referenceCount == 1);

    pool.ReleaseBlock(id2, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_cannot_evict_block_in_use() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(2, std::move(dm));

    Block *block0 = pool.NewBlock();
    Block *block1 = pool.NewBlock();

    bool threw = false;
    try {
      Block *block2 = pool.NewBlock();
      (void)block2;
    } catch (const BufferPoolException &e) {
      threw = true;
    }

    assert(threw && "Should throw when all blocks are in use and pool is full");

    pool.ReleaseBlock(block0->block_id, false);
    pool.ReleaseBlock(block1->block_id, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_flush_dirty_block_on_eviction() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(2, std::move(dm));

    Block *block0 = pool.NewBlock();
    BlockId id0 = block0->block_id;
    std::memset(block0->data, 'Z', BLOCK_SIZE);
    pool.ReleaseBlock(id0, true);
    assert(block0->isDirty && "Block should be dirty");

    Block *block1 = pool.NewBlock();
    BlockId id1 = block1->block_id;
    pool.ReleaseBlock(id1, false);

    Block *block2 = pool.NewBlock();

    Block *reloaded = pool.FetchBlock(id0);
    assert(reloaded->data[0] == 'Z' &&
           "Dirty block should have been flushed on eviction");

    pool.ReleaseBlock(id0, false);
    pool.ReleaseBlock(block2->block_id, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_flush_block_writes_to_disk() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(3, std::move(dm));

    Block *block = pool.NewBlock();
    BlockId id = block->block_id;
    std::memset(block->data, 'F', BLOCK_SIZE);
    block->isDirty = true;

    pool.FlushBlock(id);
    assert(!block->isDirty && "Block should not be dirty after flush");

    pool.ReleaseBlock(id, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_flush_all_blocks() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(3, std::move(dm));

    Block *block0 = pool.NewBlock();
    block0->isDirty = true;

    Block *block1 = pool.NewBlock();
    block1->isDirty = true;

    pool.FlushAllBlocks();

    assert(!block0->isDirty && "Block 0 should be clean after flush all");
    assert(!block1->isDirty && "Block 1 should be clean after flush all");

    pool.ReleaseBlock(block0->block_id, false);
    pool.ReleaseBlock(block1->block_id, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_release_non_existent_block_throws() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(3, std::move(dm));

    bool threw = false;
    try {
      pool.ReleaseBlock(999, false);
    } catch (const BufferPoolException &) {
      threw = true;
    }

    assert(threw && "Releasing non-existent block should throw");
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_multiple_blocks_fifo_eviction() {
  std::string path = make_temp_db_path();
  try {
    auto dm = std::make_unique<DiskManager>(path);
    BufferPool pool(3, std::move(dm));

    Block *block0 = pool.NewBlock();
    BlockId id0 = block0->block_id;
    pool.ReleaseBlock(id0, false);

    Block *block1 = pool.NewBlock();
    BlockId id1 = block1->block_id;
    pool.ReleaseBlock(id1, false);

    Block *block2 = pool.NewBlock();
    BlockId id2 = block2->block_id;
    pool.ReleaseBlock(id2, false);

    Block *block3 = pool.NewBlock();
    BlockId id3 = block3->block_id;

    assert(block3 != nullptr && "Should allocate new block with FIFO eviction");

    pool.ReleaseBlock(id3, false);
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

int main() {
  std::cout << "Running BufferPool unit tests...\n";

  test_fetch_block_not_in_pool();
  std::cout << " - fetch block not in pool test passed\n";

  test_fetch_block_already_in_pool();
  std::cout << " - fetch block already in pool test passed\n";

  test_new_block();
  std::cout << " - new block test passed\n";

  test_release_block_marks_dirty();
  std::cout << " - release block marks dirty test passed\n";

  test_eviction_when_pool_full();
  std::cout << " - eviction when pool full test passed\n";

  test_cannot_evict_block_in_use();
  std::cout << " - cannot evict block in use test passed\n";

  test_flush_dirty_block_on_eviction();
  std::cout << " - flush dirty block on eviction test passed\n";

  test_flush_block_writes_to_disk();
  std::cout << " - flush block writes to disk test passed\n";

  test_flush_all_blocks();
  std::cout << " - flush all blocks test passed\n";

  test_release_non_existent_block_throws();
  std::cout << " - release non-existent block throws test passed\n";

  test_multiple_blocks_fifo_eviction();
  std::cout << " - multiple blocks FIFO eviction test passed\n";

  std::cout << "All BufferPool tests passed.\n";
  return 0;
}