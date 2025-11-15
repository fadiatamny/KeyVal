#pragma once

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

class DiskManagerException : public std::runtime_error {
public:
  explicit DiskManagerException(const std::string &message)
      : std::runtime_error(message) {}
};

constexpr int BLOCK_SIZE = 4096; // 4KB
using BlockId = uint32_t;

class DiskManager {
public:
  explicit DiskManager(std::string &path);
  ~DiskManager();

  void SyncFile();
  void ReadBlock(BlockId id, const char *buff);
  void WriteBlock(BlockId id, const char *buff);
  BlockId AllocateBlock();

private:
  std::string path;
  std::fstream db;
  BlockId blockCount;

  long long GetBlockOffset(BlockId id) {
    return static_cast<long long>(id) * BLOCK_SIZE;
  }

  void ThrowIOError(const std::string &message) {
    throw DiskManagerException(message + "\n in File" + this->path +
                               "\t Error#" + std::to_string(errno));
  }
};
