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

#include "hphp/util/cache/cache-manager.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "folly/Format.h"
#include "hphp/util/cache/cache-data.h"
#include "hphp/util/cache/cache-saver.h"
#include "hphp/util/cache/magic-numbers.h"
#include "hphp/util/cache/mmap-file.h"
#include "hphp/util/logger.h"
#include "hphp/util/text-util.h"

namespace HPHP {

using folly::format;
using std::move;
using std::set;
using std::string;
using std::unique_ptr;
using std::vector;

CacheManager::CacheManager()
    : entry_counter_(0) {}

CacheManager::~CacheManager() {}

bool CacheManager::getFileContents(const string& name, const char** data,
                                   uint64_t* data_len,
                                   bool* compressed) const {
  const auto it = cache_map_.find(name);

  if (it == cache_map_.end()) {
    return false;
  }

  const CacheData& cd = *it->second;
  return cd.getDataPointer(data, data_len, compressed);
}

bool CacheManager::getDecompressed(const string& name, string* data) const {
  const auto it = cache_map_.find(name);

  if (it == cache_map_.end()) {
    return false;
  }

  const CacheData& cd = *it->second;
  return cd.getDecompressedData(data);
}

bool CacheManager::isCompressed(const string& name) const {
  const auto it = cache_map_.find(name);

  if (it == cache_map_.end()) {
    return false;
  }

  const CacheData& cd = *it->second;
  return cd.isCompressed();
}

bool CacheManager::addFileContents(const string& name, const string& path) {
  if (entryExists(name)) {
    return false;
  }

  unique_ptr<CacheData> cd(new CacheData);

  if (!cd->loadFromFile(name, entry_counter_++, path)) {
    return false;
  }

  cache_map_[name] = move(cd);

  addDirectories(name);

  return true;
}

bool CacheManager::addEmptyEntry(const string& name) {
  if (entryExists(name)) {
    return false;
  }

  unique_ptr<CacheData> cd(new CacheData);
  cd->createEmpty(name, entry_counter_++);

  cache_map_[name] = move(cd);

  addDirectories(name);

  return true;
}

bool CacheManager::entryExists(const string& name) const {
  return cache_map_.find(name) != cache_map_.end();
}

bool CacheManager::fileExists(const string& name) const {
  const auto it = cache_map_.find(name);

  if (it == cache_map_.end()) {
    return false;
  }

  return it->second->isRegularFile();
}

bool CacheManager::dirExists(const string& name) const {
  const auto it = cache_map_.find(name);

  if (it == cache_map_.end()) {
    return false;
  }

  return it->second->isDirectory();
}

bool CacheManager::emptyEntryExists(const string& name) const {
  const auto it = cache_map_.find(name);

  if (it == cache_map_.end()) {
    return false;
  }

  return it->second->isEmpty();
}


bool CacheManager::getUncompressedFileSize(const string& name,
                                           uint64_t* size) const {
  const auto it = cache_map_.find(name);

  if (it == cache_map_.end()) {
    return false;
  }

  const CacheData& cd = *it->second;

  if (!cd.isRegularFile()) {
    return false;
  }

  if (!cd.isCompressed()) {
    *size = cd.fileSize();
    return true;
  }

  string data;

  if (!cd.getDecompressedData(&data)) {
    return false;
  }

  *size = data.length();
  return true;
}

bool CacheManager::loadCache(const string& path) {
  mmap_file_.reset(new MmapFile(path));

  if (!mmap_file_->init()) {
    Logger::Error("Unable to mmap cache file " + path);
    return false;
  }

  uint64_t magic;
  if (!mmap_file_->readUInt64(&magic)) {
    Logger::Error("Unable to read magic number from " + path);
    return false;
  }

  if (magic != kCacheFileMagic) {
    Logger::Error("Bad magic number in " + path);
    return false;
  }

  uint64_t dirlen;
  if (!mmap_file_->readUInt64(&dirlen)) {
    return false;
  }

  for (uint64_t i = 0; i < dirlen; ++i) {
    unique_ptr<CacheData> cd(new CacheData);

    string name;
    if (!cd->loadFromMmap(mmap_file_.get(), &name)) {
      return false;
    }

    cache_map_[name] = move(cd);
  }

  uint64_t dirterm;
  if (!mmap_file_->readUInt64(&dirterm)) {
    return false;
  }

  if (dirterm != kDirectoryTerminatorMagic) {
    Logger::Error("Bad end-of-directory magic number in " + path);
    return false;
  }

  return true;
}

bool CacheManager::saveCache(const string& path) const {
  CacheSaver cs(path);

  if (!cs.init(kCacheFileMagic, cache_map_.size())) {
    Logger::Error("Unable to initialize CacheSaver");
    return false;
  }

  for (const auto& it : cache_map_) {
    const string& name = it.first;
    const CacheData* cd = it.second.get();

    if (!cd->save(&cs)) {
      Logger::Error("Failed to save CacheData for " + name);
      return false;
    }
  }

  if (!cs.endDirectory(kDirectoryTerminatorMagic)) {
    Logger::Error("Unable to write directory terminator");
    return false;
  }

  if (!cs.writeFiles()) {
    Logger::Error("Unable to write files");
    return false;
  }

  if (!cs.rewriteDirectory(cache_map_.size())) {
    Logger::Error("Unable to rewrite directory");
    return false;
  }

  if (!cs.finish()) {
    Logger::Error("CacheSaver finish failed");
    return false;
  }

  return true;
}

void CacheManager::getEntryNames(set<string>* names) const {
  set<string> temp;

  for (const auto& it : cache_map_) {
    temp.insert(it.first);
  }

  *names = temp;
}

// --- Private functions.

void CacheManager::addDirectories(const string& path) {
  vector<string> path_list;
  path_list = TextUtil::MakePathList(path);

  if (path_list.empty()) {
    return;
  }

  for (const auto& it : path_list) {
    unique_ptr<CacheData> cd(new CacheData);
    cd->createDirectory(it, entry_counter_++);
    cache_map_[it] = move(cd);
  }
}

}  // namespace HPHP
