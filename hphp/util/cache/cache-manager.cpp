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

#include "hphp/util/cache/cache-manager.h"

#include <fcntl.h>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "hphp/util/cache/cache-data.h"
#include "hphp/util/cache/cache-saver.h"
#include "hphp/util/cache/magic-numbers.h"
#include "hphp/util/cache/mmap-file.h"
#include "hphp/util/logger.h"
#include "hphp/util/text-util.h"

#include <folly/portability/SysMman.h>

namespace HPHP {

std::function<void(bool, const std::string&)> CacheManager::s_logger;

CacheManager::CacheManager() {}
CacheManager::~CacheManager() {}

bool CacheManager::getFileContents(const std::string& name, const char** data,
                                   uint64_t* data_len,
                                   bool* compressed) const {
  const auto it = m_cache_map.find(name);

  if (it == m_cache_map.end()) {
    return false;
  }

  if (s_logger && !it->second->dataFetched()) {
    s_logger(false, name);
  }

  const CacheData& cd = *it->second;
  return cd.getDataPointer(data, data_len, compressed);
}

bool CacheManager::getDecompressed(const std::string& name,
                                   std::string* data) const {
  const auto it = m_cache_map.find(name);

  if (it == m_cache_map.end()) {
    return false;
  }

  const CacheData& cd = *it->second;
  return cd.getDecompressedData(data);
}

bool CacheManager::isCompressed(const std::string& name) const {
  const auto it = m_cache_map.find(name);

  if (it == m_cache_map.end()) {
    return false;
  }

  const CacheData& cd = *it->second;
  return cd.isCompressed();
}

bool CacheManager::addFileContents(const std::string& name,
                                   const std::string& path) {
  if (m_cache_map.count(name)) {
    return false;
  }

  std::unique_ptr<CacheData> cd(new CacheData);

  if (!cd->loadFromFile(name, m_entry_counter++, path)) {
    return false;
  }

  addEntryToIndices(std::move(cd));

  return true;
}

bool CacheManager::addEmptyEntry(const std::string& name) {
  if (m_cache_map.count(name)) {
    return false;
  }

  std::unique_ptr<CacheData> cd(new CacheData);
  cd->createEmpty(name, m_entry_counter++);
  addEntryToIndices(std::move(cd));

  return true;
}

template<class F>
bool CacheManager::existsHelper(const std::string& name, F fn) const {
  auto it = m_cache_map.find(name);
  if (it == m_cache_map.end()) return false;

  auto cd = it->second.get();

  if (!fn(cd)) return false;

  if (s_logger && !cd->existChecked()) {
    s_logger(true, name);
  }

  return true;
}

bool CacheManager::entryExists(const std::string& name) const {
  return existsHelper(name, [] (CacheData*) { return true; });
}

bool CacheManager::fileExists(const std::string& name) const {
  return existsHelper(name, [] (CacheData* cd) {
      return cd->isRegularFile();
    });
}

bool CacheManager::dirExists(const std::string& name) const {
  return existsHelper(name, [] (CacheData* cd) {
      return cd->isDirectory();
    });
}

bool CacheManager::emptyEntryExists(const std::string& name) const {
  return existsHelper(name, [] (CacheData* cd) {
      return cd->isEmpty();
    });
}

bool CacheManager::getUncompressedFileSize(const std::string& name,
                                           uint64_t* size) const {
  const auto it = m_cache_map.find(name);

  if (it == m_cache_map.end()) {
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

  std::string data;

  if (!cd.getDecompressedData(&data)) {
    return false;
  }

  *size = data.length();
  return true;
}

bool CacheManager::loadCache(const std::string& path) {
  m_mmap_file.reset(new MmapFile(path));

  if (!m_mmap_file->init()) {
    Logger::Error("Unable to mmap cache file " + path);
    return false;
  }

  uint64_t magic;
  if (!m_mmap_file->readUInt64(&magic)) {
    Logger::Error("Unable to read magic number from " + path);
    return false;
  }

  if (magic != kCacheFileMagic) {
    Logger::Error("Bad magic number in " + path);
    return false;
  }

  uint64_t dirlen;
  if (!m_mmap_file->readUInt64(&dirlen)) {
    return false;
  }

  for (uint64_t i = 0; i < dirlen; ++i) {
    std::unique_ptr<CacheData> cd(new CacheData);

    if (!cd->loadFromMmap(m_mmap_file.get())) {
      return false;
    }

    // We auto-create parent/grandparent/etc directory entries
    // when we add entries to our internal index, and we store
    // the list of the directory's children on the directory
    // itself to support readDirectory().
    //
    // If an entry appears in the cache file before its parent
    // directory, that code path still happens. But when we
    // eventually see the parent directory in the cache file,
    // we do NOT want to replace our existing entry,
    // because that would lose the list of children stored on it.
    if (!cd->isDirectory() ||
        m_cache_map.find(cd->getName()) == m_cache_map.end()) {
      addEntryToIndices(std::move(cd));
    }
  }

  uint64_t dirterm;
  if (!m_mmap_file->readUInt64(&dirterm)) {
    return false;
  }

  if (dirterm != kDirectoryTerminatorMagic) {
    Logger::Error("Bad end-of-directory magic number in " + path);
    return false;
  }

  return true;
}

bool CacheManager::saveCache(const std::string& path) const {
  CacheSaver cs(path);

  // Write entries out in a consistent sorted order, so the F14Map
  // implementation doesn't cause the cache file to vary.
  std::vector<const CacheData*> entries = getSortedEntries();

  if (!cs.init(kCacheFileMagic, entries.size())) {
    Logger::Error("Unable to initialize CacheSaver");
    return false;
  }

  for (const CacheData* cd : entries) {
    if (!cd->save(&cs)) {
      Logger::Error("Failed to save CacheData for " + cd->getName());
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

  if (!cs.rewriteDirectory(entries.size())) {
    Logger::Error("Unable to rewrite directory");
    return false;
  }

  if (!cs.finish()) {
    Logger::Error("CacheSaver finish failed");
    return false;
  }

  return true;
}

void CacheManager::getEntryNames(std::set<std::string>* names) const {
  std::set<std::string> temp;

  for (const auto& it : m_cache_map) {
    temp.insert(it.first);
  }

  *names = temp;
}

// --- Private functions.

void CacheManager::addEntryToIndices(std::unique_ptr<CacheData> entry) {
  CacheData* cd = entry.get();
  m_cache_map.emplace(cd->getName(), std::move(entry));

  // Save the full path to this entry, and clean it up just in case.
  std::string_view path(cd->getName());
  if (auto where = path.find_last_not_of('/'); where != path.npos) {
    path.remove_suffix(path.size() - (where + 1));
  } else {
    return;
  }

  // path is something of the form "/path/to/the/file", with no
  // trailing '/'. Set parentPath to "/path/to/the/", and
  // fileName to "file".
  std::string_view parentPath = path, fileName = path;
  if (auto where = parentPath.find_last_of('/'); where != parentPath.npos) {
    parentPath.remove_suffix(parentPath.size() - (where + 1));
    fileName.remove_prefix(where + 1);
  } else {
    return;
  }

  // Strip trailing '/' from parentPath, so it's "/path/to/the",
  // which we can use to do a lookup.
  if (auto where = parentPath.find_last_not_of('/'); where != parentPath.npos) {
    parentPath.remove_suffix(parentPath.size() - (where + 1));
  } else {
    return;
  }

  // Look for an existing entry that matches the parent pathname. Move
  // this into a std::string now, because hphp_fast_string_map doesn't support
  // heterogeneous finds.
  std::string parentPathString(parentPath);
  auto existingParent = m_cache_map.find(parentPathString);
  if (existingParent != m_cache_map.end()) {
    // Found an existing item. Add fileName as a child, and we're done.
    existingParent->second->addChildToDirectory(fileName);
  } else {
    // Didn't find an existing parent directory. Have to make a new one.
    std::unique_ptr<CacheData> newParent(new CacheData);
    newParent->createDirectory(std::move(parentPathString), m_entry_counter++);

    // Initialize the parent by adding this entry as its child.
    newParent->addChildToDirectory(fileName);

    // And recurse down, to add it to its parents (creating them if needed).
    addEntryToIndices(std::move(newParent));
  }
}

VFileType CacheManager::getFileType(const std::string& name) const {
  auto file = m_cache_map.find(name);

  if (file == m_cache_map.end()) return VFileType::NotFound;
  if (file->second->isDirectory()) return VFileType::Directory;
  return VFileType::PlainFile;
}

std::vector<std::string>
CacheManager::readDirectory(const std::string& name) const {
  std::vector<std::string> result;
  auto found = m_cache_map.find(name);
  if (found != m_cache_map.end() && found->second->isDirectory()) {
    result = found->second->getDirectoryChildren();
  }
  return result;
}

void CacheManager::dump() const {
  std::vector<const CacheData*> entries = getSortedEntries();

  for (const CacheData* cd : entries) {
    printf("- %s\n", cd->getName().c_str());
    cd->dump();
  }
}

std::vector<const CacheData*> CacheManager::getSortedEntries() const {
  std::vector<const CacheData*> entries;

  entries.reserve(m_cache_map.size());
  for (const auto& it : m_cache_map) {
    entries.push_back(it.second.get());
  }

  sort(
      entries.begin(),
      entries.end(),
      [](const CacheData* a, const CacheData* b) {
        return a->getName() < b->getName();
      });

  return entries;
}

}  // namespace HPHP
