/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <cstdint>
#include <random>
#include <string>

#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/Fcntl.h>
#include <folly/portability/Unistd.h>
#include "hphp/util/cache/cache-saver.h"
#include "hphp/util/cache/magic-numbers.h"
#include "hphp/util/cache/mmap-file.h"
#include "hphp/util/assertions.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

using folly::format;
using folly::makeGuard;
using std::string;

static const int kGzipLevel = 9;

CacheData::CacheData() {}

CacheData::~CacheData() {
  if (m_should_free) {
    free((void*) m_file_data);
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

  m_id = id;
  m_mtime = fs.st_mtime;
  m_flags = 0 | kFlag_RegularFile;
  m_file_data_length = fs.st_size;

  char* temp_data = static_cast<char*>(malloc(m_file_data_length));
  always_assert(temp_data != nullptr);

  auto guard = makeGuard([&] { free(temp_data); });

  m_should_free = true;

  ssize_t read_len = read(fd, temp_data, m_file_data_length);

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

      m_file_data_length = new_len;
      m_flags |= kFlag_Compressed;

    } else {
      free(compressed);
    }
  }

  m_checksum = createChecksum();

  m_name = name;
  m_file_data = temp_data;

  return true;
}

void CacheData::createEmpty(const string& name, uint64_t id) {
  m_name = name;
  m_id = id;
  m_mtime = 0;
  m_flags = 0 | kFlag_EmptyEntry;
  m_file_data = nullptr;
  m_file_data_length = 0;
  m_should_free = false;
  m_checksum = createChecksum();
}

void CacheData::createDirectory(const std::string& name, uint64_t id) {
  m_name = name;
  m_id = id;
  m_mtime = 0;
  m_flags = 0 | kFlag_Directory;
  m_file_data = nullptr;
  m_file_data_length = 0;
  m_should_free = false;
  m_checksum = createChecksum();
}

bool CacheData::loadFromMmap(MmapFile* mmap_file, string* name) {
  if (!mmap_file->readUInt64(&m_id) ||
      !mmap_file->readUInt64(&m_flags) ||
      !mmap_file->readUInt64(&m_mtime) ||
      !mmap_file->readUInt64(&m_checksum)) {
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

  if (!mmap_file->readUInt64(&m_file_data_length)) {
    return false;
  }

  string temp_name;
  if (!mmap_file->readString(&temp_name)) {
    return false;
  }

  if (!mmap_file->makePointer(temp_ofs, m_file_data_length, &m_file_data)) {
    Logger::Error("Unable to get pointer for file data into mmapped space");
    return false;
  }

  m_should_free = false;

  *name = temp_name;
  return true;
}

bool CacheData::save(CacheSaver* cs) const {
  CacheSaver::DirEntry de;

  de.id = m_id;
  de.flags = m_flags;
  de.mtime = m_mtime;
  de.checksum = m_checksum;
  de.data_len = m_file_data_length;
  de.data_ptr = m_file_data;
  de.name = m_name;

  return cs->writeDirEntry(de);
}

bool CacheData::isRegularFile() const {
  return (m_flags & kFlag_RegularFile) == kFlag_RegularFile;
}

bool CacheData::isDirectory() const {
  return (m_flags & kFlag_Directory) == kFlag_Directory;
}

bool CacheData::isCompressed() const {
  return (m_flags & kFlag_Compressed) == kFlag_Compressed;
}

bool CacheData::isEmpty() const {
  return (m_flags & kFlag_EmptyEntry) == kFlag_EmptyEntry;
}

uint64_t CacheData::fileSize() const {
  return m_file_data_length;
}

bool CacheData::getDataPointer(const char** data, uint64_t* data_len,
                               bool* compressed) const {
  if (!isRegularFile()) {
    Logger::Error("Unable to get data pointer: not a regular file");
    return false;
  }

  // This would be a great place to verify the checksum... once.

  *data = m_file_data;
  *data_len = m_file_data_length;
  *compressed = isCompressed();
  return true;
}

bool CacheData::getDecompressedData(string* data) const {
  if (!isCompressed()) {
    return false;
  }

  int new_len = m_file_data_length;    // Changed by gzdecode(), sigh.
  char *temp = gzdecode(m_file_data, new_len);

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

void CacheData::dump() const {
  printf(
    "  Name: %s\n"
    "  Flags: 0x%08" PRIx64 "\n"
    "  Size: %" PRIu64 "\n"
    "  ID: %" PRIu64 "\n",
    m_name.c_str(),
    m_flags,
    fileSize(),
    m_id
  );
}

}  // namespace HPHP
