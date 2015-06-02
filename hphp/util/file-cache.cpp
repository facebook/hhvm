/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/file-cache.h"

#include <set>
#include <string>

#include "hphp/util/cache/cache-manager.h"
#include "hphp/util/exception.h"
#include "hphp/util/logger.h"

namespace HPHP {

using std::set;
using std::string;

string FileCache::SourceRoot;
bool FileCache::UseNewCache = true;

FileCache::FileCache()
    : cache_manager_(new CacheManager) {
  UseNewCache = true;
}

FileCache::~FileCache() {}

string FileCache::GetRelativePath(const char *path) {
  assert(path);

  string relative = path;
  unsigned int len = SourceRoot.size();
  if (len > 0 && relative.size() > len &&
      strncmp(relative.data(), SourceRoot.c_str(), len) == 0) {
    relative = relative.substr(len);
  }
  if (!relative.empty() && relative[relative.length() - 1] == '/') {
    relative = relative.substr(0, relative.length() - 1);
  }
  return relative;
}

void FileCache::write(const char *name) {
  assert(name && *name);
  assert(!exists(name));

  if (!cache_manager_->addEmptyEntry(name)) {
    throw Exception("Unable to add entry for %s", name);
  }
}

void FileCache::write(const char *name, const char *fullpath) {
  assert(name && *name);
  assert(fullpath && *fullpath);
  assert(!exists(name));

  if (!cache_manager_->addFileContents(name, fullpath)) {
    throw Exception("Unable to add entry for %s (%s)", name, fullpath);
  }
}

void FileCache::save(const char *filename) {
  assert(filename && *filename);

  if (!cache_manager_->saveCache(filename)) {
    throw Exception("Unable to save cache to %s", filename);
  }
}

void FileCache::loadMmap(const char *filename) {
  assert(filename && *filename);

  if (!cache_manager_->loadCache(filename)) {
    throw Exception("Unable to load cache from %s", filename);
  }
}

bool FileCache::fileExists(const char *name,
                           bool isRelative /* = true */) const {
  if (isRelative) {
    // Original cache behavior: an empty entry is also a "file".
    return cache_manager_->fileExists(name) ||
           cache_manager_->emptyEntryExists(name);
  }

  return fileExists(GetRelativePath(name).c_str());
}

bool FileCache::dirExists(const char *name,
                          bool isRelative /* = true */) const {
  if (isRelative) {
    return cache_manager_->dirExists(name);
  }

  return dirExists(GetRelativePath(name).c_str());
}

bool FileCache::exists(const char *name,
                       bool isRelative /* = true */) const {
  if (isRelative) {
    return cache_manager_->entryExists(name);
  }

  return exists(GetRelativePath(name).c_str());
}

char *FileCache::read(const char *name, int &len, bool &compressed) const {
  if (!name || !*name) {
    return nullptr;
  }

  const char* data;
  uint64_t data_len;
  bool data_compressed;

  if (!cache_manager_->getFileContents(name, &data, &data_len,
                                       &data_compressed)) {
    return nullptr;
  }

  compressed = data_compressed;
  len = data_len;

  // Yep, throwing away const here (for now) for API compatibility.
  return (char*) data;
}

int64_t FileCache::fileSize(const char *name, bool isRelative) const {
  if (!name || !*name) {
    return -1;
  }

  if (isRelative) {
    uint64_t size;
    if (!cache_manager_->getUncompressedFileSize(name, &size)) {
      return -1;
    }

    return size;
  }

  return fileSize(GetRelativePath(name).c_str(), true);
}

void FileCache::dump() const {
  cache_manager_->dump();
}

}  // namespace HPHP
