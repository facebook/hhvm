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

#include "hphp/util/cache/cache-data.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <random>
#include <string>

#include "folly/Format.h"
#include "folly/ScopeGuard.h"
#include "hphp/util/cache/cache-data.h"
#include "hphp/util/cache/cache-saver.h"
#include "hphp/util/cache/magic-numbers.h"
#include "hphp/util/cache/mmap-file.h"
#include "hphp/util/compression.h"
#include "hphp/util/logger.h"

namespace HPHP {

using folly::format;
using folly::makeGuard;
using folly::ScopeGuard;
using std::string;

static const int kGzipLevel = 9;

CacheData::CacheData()
    : should_free_(false) {}

CacheData::~CacheData() {
  if (should_free_) {
    free((void*) file_data_);
  }
}

bool CacheData::loadFromFile(const string& name, uint64_t id,
                             const string& path) {
  int fd = open(path.c_str(), O_RDONLY);

  if (fd < 0) {
    Logger::Error(format("Unable to open {}: {}",
                         path, folly::errnoStr(errno)).str());
    return false;
  }

  SCOPE_EXIT { close(fd); };

  struct stat fs;
  if (fstat(fd, &fs) != 0) {
    Logger::Error(format("Unable to fstat {}: {}",
                         path, folly::errnoStr(errno)).str());
    return false;
  }

  id_ = id;
  mtime_ = fs.st_mtime;
  flags_ = 0 | kFlag_RegularFile;
  file_data_length_ = fs.st_size;

  char* temp_data = static_cast<char*>(malloc(file_data_length_));
  always_assert(temp_data != nullptr);

  ScopeGuard guard = makeGuard([&] { free(temp_data); });

  should_free_ = true;

  ssize_t read_len = read(fd, temp_data, file_data_length_);

  if (read_len < 0) {
    Logger::Error(format("Unable to load from {}: {}",
                         path, folly::errnoStr(errno)).str());
    return false;
  }

  if (read_len != fs.st_size) {
    Logger::Error(path + " changed size during read");
    return false;
  }

  guard.dismiss();

  int new_len = read_len;   // Changed by gzencode(), sigh.

  char* compressed = gzencode(temp_data, new_len, kGzipLevel, CODING_GZIP);

  if (compressed != nullptr) {
    if (sufficientlyCompressed(read_len, new_len)) {
      free(temp_data);
      temp_data = compressed;

      file_data_length_ = new_len;
      flags_ |= kFlag_Compressed;

    } else {
      free(compressed);
    }
  }

  checksum_ = createChecksum();

  name_ = name;
  file_data_ = temp_data;

  return true;
}

void CacheData::createEmpty(const string& name, uint64_t id) {
  name_ = name;
  id_ = id;
  mtime_ = 0;
  flags_ = 0 | kFlag_EmptyEntry;
  file_data_ = nullptr;
  file_data_length_ = 0;
  should_free_ = false;
  checksum_ = createChecksum();
}

void CacheData::createDirectory(const std::string& name, uint64_t id) {
  name_ = name;
  id_ = id;
  mtime_ = 0;
  flags_ = 0 | kFlag_Directory;
  file_data_ = nullptr;
  file_data_length_ = 0;
  should_free_ = false;
  checksum_ = createChecksum();
}

bool CacheData::loadFromMmap(MmapFile* mmap_file, string* name) {
  if (!mmap_file->readUInt64(&id_) ||
      !mmap_file->readUInt64(&flags_) ||
      !mmap_file->readUInt64(&mtime_) ||
      !mmap_file->readUInt64(&checksum_)) {
    Logger::Error("Can't read initial metadata for file");
    return false;
  }

  uint64_t temp_ofs;
  if (!mmap_file->readUInt64(&temp_ofs)) {
    return false;
  }

  // Hey there, future troubleshooter!  If this ever fires, odds are
  // good the CacheSaver::rewriteDirectory stuff didn't happen properly.

  if (temp_ofs == kDataOfsPlaceholder) {
    Logger::Error("Invalid offset found in directory "
                  "(placeholder not reset to actual value)");
    return false;
  }

  if (!mmap_file->readUInt64(&file_data_length_)) {
    return false;
  }

  string temp_name;
  if (!mmap_file->readString(&temp_name)) {
    return false;
  }

  if (!mmap_file->makePointer(temp_ofs, file_data_length_, &file_data_)) {
    Logger::Error("Unable to get pointer for file data into mmapped space");
    return false;
  }

  should_free_ = false;

  *name = temp_name;
  return true;
}

bool CacheData::save(CacheSaver* cs) const {
  CacheSaver::DirEntry de = { 0 };

  de.id = id_;
  de.flags = flags_;
  de.mtime = mtime_;
  de.checksum = checksum_;
  de.data_len = file_data_length_;
  de.data_ptr = file_data_;
  de.name = name_;

  return cs->writeDirEntry(de);
}

bool CacheData::isRegularFile() const {
  return (flags_ & kFlag_RegularFile) == kFlag_RegularFile;
}

bool CacheData::isDirectory() const {
  return (flags_ & kFlag_Directory) == kFlag_Directory;
}

bool CacheData::isCompressed() const {
  return (flags_ & kFlag_Compressed) == kFlag_Compressed;
}

bool CacheData::isEmpty() const {
  return (flags_ & kFlag_EmptyEntry) == kFlag_EmptyEntry;
}

uint64_t CacheData::fileSize() const {
  return file_data_length_;
}

bool CacheData::getDataPointer(const char** data, uint64_t* data_len,
                               bool* compressed) const {
  if (!isRegularFile()) {
    Logger::Error("Unable to get data pointer: not a regular file");
    return false;
  }

  // This would be a great place to verify the checksum... once.

  *data = file_data_;
  *data_len = file_data_length_;
  *compressed = isCompressed();
  return true;
}

bool CacheData::getDecompressedData(string* data) const {
  if (!isCompressed()) {
    return false;
  }

  int new_len = file_data_length_;    // Changed by gzdecode(), sigh.
  char *temp = gzdecode(file_data_, new_len);

  if (temp == nullptr) {
    return false;
  }

  data->assign(temp, new_len);
  free(temp);
  return true;
}

// --- Private functions.

uint64_t CacheData::createChecksum() const {
  // For possible future extension.

  return 0;
}

bool CacheData::sufficientlyCompressed(uint64_t orig_size,
                                       uint64_t new_size) const {
  return new_size < (orig_size * .75);
}

}  // namespace HPHP
