#include "./DiskManager.hpp"
#include <iostream>
#include <vector>

DiskManager::DiskManager(std::string &path) : path(path), blockCount(0) {
  this->db.open(this->path, std::ios::in | std::ios::out | std::ios::binary);

  if (!this->db.is_open()) {
    this->db.clear();
    this->db.open(this->path, std::ios::out | std::ios::binary);
    this->db.close();
    this->db.open(this->path, std::ios::in | std::ios::out | std::ios::binary);
  }

  this->db.seekg(0, std::ios::end);
  this->blockCount = this->db.tellg() / BLOCK_SIZE;
}

DiskManager::~DiskManager() { this->db.close(); }
void DiskManager::SyncFile() { this->db.flush(); }

BlockId DiskManager::AllocateBlock() {
  BlockId newBlockId = this->blockCount;
  long long offset = this->GetBlockOffset(newBlockId);
  this->db.seekp(offset, std::ios::beg);
  if (this->db.fail()) {
    this->db.clear();
    this->ThrowIOError("Failed to seek new block at offset " +
                       std::to_string(offset));
  }

  std::vector<char> emptyBlock(BLOCK_SIZE, 0);
  this->db.write(emptyBlock.data(), BLOCK_SIZE);

  if (this->db.fail()) {
    this->db.clear();
    this->ThrowIOError("Failed to write new block at offset " +
                       std::to_string(offset));
  }

  this->SyncFile();
  this->blockCount++;

  return newBlockId;
}

void DiskManager::ReadBlock(BlockId id, const char *buff) {
  if (!this->db.is_open()) {
    this->ThrowIOError("Trying to read from closed DB");
  }
  if (buff == nullptr) {
    this->ThrowIOError("Trying to read into null buffer");
  }
  if (id >= this->blockCount) {
    this->ThrowIOError("Trying to read non allocated block " +
                       std::to_string(id));
  }

  long long offset = this->GetBlockOffset(id);
  this->db.seekg(offset, std::ios::beg);
  if (this->db.fail()) {
    this->db.clear();
    this->ThrowIOError("Failed to seek read block at offset " +
                       std::to_string(offset));
  }

  this->db.read(const_cast<char *>(buff), BLOCK_SIZE);
  if (this->db.fail()) {
    this->db.clear();
    if (this->db.eof()) {
      this->ThrowIOError("Reached EOF while reading block at offset " +
                         std::to_string(offset));
    } else {
      this->ThrowIOError("Failed to read block at offset " +
                         std::to_string(offset));
    }
  }
}

void DiskManager::WriteBlock(BlockId id, const char *buff) {
  if (!this->db.is_open()) {
    this->ThrowIOError("Trying to write to closed DB");
  }
  if (buff == nullptr) {
    this->ThrowIOError("Trying to write from null buffer");
  }
  if (id >= this->blockCount) {
    this->ThrowIOError("Trying to write non allocated block " +
                       std::to_string(id));
  }

  long long offset = this->GetBlockOffset(id);
  this->db.seekp(offset, std::ios::beg);
  if (this->db.fail()) {
    this->db.clear();
    this->ThrowIOError("Failed to seek write block at offset " +
                       std::to_string(offset));
  }

  this->db.write(buff, BLOCK_SIZE);
  if (this->db.fail()) {
    this->db.clear();
    this->ThrowIOError("Failed to write block at offset " +
                       std::to_string(offset));
  }
}
