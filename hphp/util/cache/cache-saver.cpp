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

#include "hphp/util/cache/cache-saver.h"

#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

#include "folly/FileUtil.h"
#include "folly/Format.h"
#include "hphp/util/cache/magic-numbers.h"
#include "hphp/util/logger.h"

namespace HPHP {

using folly::format;
using folly::writeFull;
using std::string;
using std::vector;

static const mode_t kDumpMode = S_IRUSR | S_IWUSR |
                                S_IRGRP | S_IWGRP |
                                S_IROTH;

CacheSaver::CacheSaver(const string& path)
    : path_(path),
      initialized_(false) {}

CacheSaver::~CacheSaver() {
  if (initialized_) {
    close(fd_);
  }
}

bool CacheSaver::init(uint64_t file_magic, uint64_t num_dirents) {
  fd_ = open(path_.c_str(), O_CREAT | O_EXCL | O_RDWR, kDumpMode);

  if (fd_ < 0) {
    Logger::Error(format("Unable to open {}: {}",
                         path_, folly::errnoStr(errno)).str());
    return false;
  }

  if (!writeUInt64(file_magic)) {
    Logger::Error("Failed to write magic number at beginning of file");
    return false;
  }

  directory_ofs_ = getOfs();

  if (!writeUInt64(num_dirents)) {
    Logger::Error("Failed to write directory size");
    return false;
  }

  initialized_ = true;
  return true;
}

bool CacheSaver::writeDirEntry(const DirEntry& direntry) {
  CHECK(initialized_) << ": call CacheSaver::init";

  if (!writeUInt64(direntry.id) ||
      !writeUInt64(direntry.flags) ||
      !writeUInt64(direntry.mtime) ||
      !writeUInt64(direntry.checksum)) {
    Logger::Error("Failed to write initial directory entry metadata");
    return false;
  }

  FilePointer fp;
  fp.id = direntry.id;

  // Remember where the "data offset" will go.
  fp.dirent_update_ofs = getOfs();

  if (!writeUInt64(kDataOfsPlaceholder)) {
    Logger::Error("Failed while writing initial placeholder for data offset");
    return false;
  }

  if (!writeUInt64(direntry.data_len)) {
    Logger::Error("Failed while writing data length");
    return false;
  }

  fp.data_ptr = direntry.data_ptr;
  fp.data_len = direntry.data_len;

  // This won't be set until we write the actual file data later on.
  fp.data_ofs = 0;

  file_pointers_.push_back(fp);

  if (!writeString(direntry.name)) {
      Logger::Error("Failed while writing name in directory entry");
    return false;
  }

  return true;
}

bool CacheSaver::endDirectory(uint64_t directory_end_magic) {
  return writeUInt64(directory_end_magic);
}

bool CacheSaver::writeFiles() {
  CHECK(initialized_) << ": call CacheSaver::init";

  for (auto& it : file_pointers_) {
    FilePointer& fp = it;

    // Remember where this file data is going.
    fp.data_ofs = getOfs();

    ssize_t ret = writeFull(fd_, (void*) fp.data_ptr, fp.data_len);

    if (ret != fp.data_len) {
      Logger::Error("Failed during write of file contents");
      return false;
    }
  }

  return true;
}

bool CacheSaver::rewriteDirectory(uint64_t expected_directory_len) {
  CHECK(initialized_) << ": call CacheSaver::init";

  if (!setOfs(directory_ofs_)) {
    return false;
  }

  // Paranoia.
  uint64_t check_len;
  ssize_t ret = read(fd_, (void*) &check_len, sizeof(check_len));

  if (ret != sizeof(check_len)) {
    Logger::Error(format("Unable to check directory length on disk: {}",
                         folly::errnoStr(errno)).str());
  }

  if (check_len != expected_directory_len) {
    Logger::Error("Sanity check failed: didn't find expected length counter "
                  "after seeking back to directory offset");
    return false;
  }

  for (const auto& it : file_pointers_)  {
    const FilePointer& fp = it;

    if (!setOfs(fp.dirent_update_ofs)) {
      return false;
    }

    ssize_t ret = writeFull(fd_, (void*) &fp.data_ofs, sizeof(fp.data_ofs));

    if (ret != sizeof(fp.data_ofs)) {
      Logger::Error(format("Unable to update directory entry: {}",
                           folly::errnoStr(errno)).str());
    }
  }

  return true;
}

bool CacheSaver::finish() {
  CHECK(initialized_) << ": call CacheSaver::init";
  return close(fd_) == 0;
}

// --- Private functions.

off_t CacheSaver::getOfs() const {
  return lseek(fd_, 0, SEEK_CUR);
}

bool CacheSaver::setOfs(off_t ofs) const {
  return lseek(fd_, ofs, SEEK_SET) == ofs;
}

bool CacheSaver::writeUInt64(uint64_t value) const {
  ssize_t ret = writeFull(fd_, (const void*) &value, sizeof(value));
  return ret == sizeof(value);
}

bool CacheSaver::writeString(const string& str) const {
  if (!writeUInt64(str.length())) {
    return false;
  }

  ssize_t write_len = str.length();
  always_assert(write_len >= 0);

  ssize_t ret = writeFull(fd_, (const void*) str.data(), write_len);

  if (ret < 0) {
    Logger::Error(format("Unable to write string: {}",
                         folly::errnoStr(errno)).str());
    return false;
  }

  return ret == write_len;
}

}  // namespace HPHP
