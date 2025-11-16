#pragma once
#include "../../types//Constants.hpp"
#include <cstring>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>

class DiskManagerException : public std::runtime_error {
public:
  explicit DiskManagerException(const std::string &message)
      : std::runtime_error(message) {}
};

class DiskManager {
public:
  explicit DiskManager(const std::string &path);
  ~DiskManager();

  void SyncFile();
  void ReadBlock(BlockId id, char *buff);
  void WriteBlock(BlockId id, const char *buff);
  BlockId AllocateBlock();

private:
  std::string path;
  std::fstream db;
  BlockId blockCount;
  mutable std::mutex mutex;

  long long GetBlockOffset(BlockId id) {
    return static_cast<long long>(id) * BLOCK_SIZE;
  }

  std::string GetStreamErrorInfo() {
    std::string info = " (bad: " + std::to_string(this->db.bad()) +
                       ", eof: " + std::to_string(this->db.eof()) +
                       ", fail: " + std::to_string(this->db.fail());
    if (errno != 0) {
      info +=
          ", errno: " + std::to_string(errno) + " - " + std::strerror(errno);
    }
    info += ")";
    return info;
  }

  void ThrowIOError(const std::string &message) {
    std::string errorInfo = GetStreamErrorInfo();
    this->db.clear();
    throw DiskManagerException(message + errorInfo +
                               "\n in File: " + this->path);
  }
};
