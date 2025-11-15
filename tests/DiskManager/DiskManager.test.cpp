#include "../../src/models/DiskManager/DiskManager.hpp"

#include <cassert>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <system_error>

using namespace std::string_literals;
namespace fs = std::filesystem;

static std::string make_temp_db_path() {


  auto tmp = fs::temp_directory_path();
  auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::random_device rd;
  std::mt19937_64 eng(rd());
  std::uniform_int_distribution<uint64_t> dist;
  uint64_t r = dist(eng);
  std::string filename = "keyval_test_diskmanager_" + std::to_string(now) + "_" +
                         std::to_string(r) + ".db";
  return (tmp / filename).string();
}

static void safe_remove(const std::string &path) {
  std::error_code ec;
  fs::remove(path, ec);

  (void)ec;
}

static void test_allocate_and_increment_block_ids() {
  std::string path = make_temp_db_path();
  try {
    std::string modpath = path;
    DiskManager dm(modpath);

    BlockId b0 = dm.AllocateBlock();
    assert(b0 == 0u && "First allocated block id should be 0");

    BlockId b1 = dm.AllocateBlock();
    assert(b1 == 1u && "Second allocated block id should be 1");

    BlockId b2 = dm.AllocateBlock();
    assert(b2 == 2u && "Third allocated block id should be 2");

    dm.SyncFile();
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_write_and_read_block_contents() {
  std::string path = make_temp_db_path();
  try {
    std::string modpath = path;
    DiskManager dm(modpath);

    BlockId id = dm.AllocateBlock();
    assert(id == 0u);


    std::vector<char> write_buf(BLOCK_SIZE, 0);
    for (size_t i = 0; i < write_buf.size(); ++i) {
      write_buf[i] = static_cast<char>((i & 0xFF));
    }

    dm.WriteBlock(id, write_buf.data());


    dm.SyncFile();


    std::vector<char> read_buf(BLOCK_SIZE, 0xFF);
    dm.ReadBlock(id, read_buf.data());


    int cmp = std::memcmp(write_buf.data(), read_buf.data(), BLOCK_SIZE);
    assert(cmp == 0 && "Read buffer should match written buffer");
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_read_unallocated_block_throws() {
  std::string path = make_temp_db_path();
  try {
    std::string modpath = path;
    DiskManager dm(modpath);


    std::vector<char> buf(BLOCK_SIZE, 0);
    bool threw = false;
    try {
      dm.ReadBlock(0u, buf.data());
    } catch (const DiskManagerException &e) {
      threw = true;
    } catch (...) {

      threw = true;
    }
    assert(threw && "Reading an unallocated block should throw an exception");
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

static void test_null_buffer_throws() {
  std::string path = make_temp_db_path();
  try {
    std::string modpath = path;
    DiskManager dm(modpath);

    BlockId id = dm.AllocateBlock();
    (void)id;

    bool threw_read = false;
    try {
      dm.ReadBlock(0u, nullptr);
    } catch (const DiskManagerException &) {
      threw_read = true;
    } catch (...) {
      threw_read = true;
    }
    assert(threw_read && "ReadBlock with nullptr should throw");

    bool threw_write = false;
    try {
      dm.WriteBlock(0u, nullptr);
    } catch (const DiskManagerException &) {
      threw_write = true;
    } catch (...) {
      threw_write = true;
    }
    assert(threw_write && "WriteBlock with nullptr should throw");
  } catch (...) {
    safe_remove(path);
    throw;
  }
  safe_remove(path);
}

int main() {
  std::cout << "Running DiskManager unit tests...\n";

  test_allocate_and_increment_block_ids();
  std::cout << " - allocate and increment test passed\n";

  test_write_and_read_block_contents();
  std::cout << " - write/read contents test passed\n";

  test_read_unallocated_block_throws();
  std::cout << " - read unallocated block throws test passed\n";

  test_null_buffer_throws();
  std::cout << " - null buffer throws test passed\n";

  std::cout << "All DiskManager tests passed.\n";
  return 0;
}