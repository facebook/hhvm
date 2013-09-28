/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/cache/mmap-file.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdint>
#include <string>

#include "folly/Format.h"
#include "hphp/util/logger.h"

namespace HPHP {

using folly::format;
using std::string;

MmapFile::MmapFile(const std::string& path)
    : path_(path),
      initialized_(false) {}

MmapFile::~MmapFile() {
  if (initialized_) {
    munmap(backing_mem_, backing_mem_size_);
    close(backing_fd_);
  }
}

bool MmapFile::init() {
  int fd = open(path_.c_str(), O_RDONLY);

  if (fd < 0) {
    Logger::Error(format("Unable to open {}: {}",
                         path_, folly::errnoStr(errno)).str());
    return false;
  }

  struct stat fs;
  if (fstat(fd, &fs) != 0) {
    Logger::Error(format("Unable to fstat {}: {}",
                  path_, folly::errnoStr(errno)).str());
    close(fd);
    return false;
  }

  backing_fd_ = fd;
  backing_mem_size_ = fs.st_size;
  backing_mem_ = mmap(nullptr, backing_mem_size_, PROT_READ, MAP_PRIVATE,
                      backing_fd_, 0);

  if (backing_mem_ == (void*) -1) {
    Logger::Error(format("Unable to mmap {}: {}",
                         path_, folly::errnoStr(errno)).str());
    close(backing_fd_);
    return false;
  }

  backing_mem_end_ = (char*) backing_mem_ + backing_mem_size_;

  read_ptr_ = static_cast<char*>(backing_mem_);
  initialized_ = true;
  return true;
}

bool MmapFile::readUInt64(uint64_t* value) {
  CHECK(initialized_) << ": call MmapFile::init";

  size_t len = sizeof(*value);

  if (wouldExceedMem(len)) {
    Logger::Error("Unable to read uint64_t: would extend beyond EOF");
    return false;
  }

  memcpy((void *) value, read_ptr_, len);
  read_ptr_ += len;

  return true;
}

bool MmapFile::readString(string* str) {
  CHECK(initialized_) << ": call MmapFile::init";

  uint64_t len;
  if (!readUInt64(&len)) {
    return false;
  }

  if (wouldExceedMem(len)) {
    Logger::Error("Unable to read string: would extend beyond EOF");
    return false;
  }

  str->assign(read_ptr_, len);
  read_ptr_ += len;

  return true;
}

bool MmapFile::makePointer(uint64_t offset, uint64_t len,
                           const char** ptr) const {
  CHECK(initialized_) << ": call MmapFile::init";

  const char* temp = (const char*) backing_mem_;

  if (temp + offset > backing_mem_end_) {
    Logger::Error("Unable to make pointer: beginning beyond EOF");
    return false;
  }

  if (temp + offset + len > backing_mem_end_) {
    Logger::Error("Unable to make pointer: would end beyond EOF");
    return false;
  }

  *ptr = temp + offset;
  return true;
}

// --- Private functions.

bool MmapFile::wouldExceedMem(uint64_t len) const {
  return (read_ptr_ + len) > backing_mem_end_;
}

}  // namespace HPHP
