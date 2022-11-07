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

// Individual in-memory component of the cache.
// Usually normal files (web static resources).

#pragma once

#include <cstdint>
#include <string>
#include <atomic>
#include <folly/Range.h>
#include <folly/container/F14Map.h>

namespace HPHP {

struct CacheSaver;
struct MmapFile;

struct CacheData {
  CacheData();
  ~CacheData();

  CacheData(const CacheData&) = delete;
  CacheData& operator=(const CacheData&) = delete;

  // --- Creation functions: call at most one of these per instance.

  // Create a named entry with contents of regular file at <path>.
  bool loadFromFile(const std::string& name, uint64_t id,
                    const std::string& path);

  // Create a named entry without any contents.
  void createEmpty(const std::string& name, uint64_t id);

  // Create a named directory entry.
  void createDirectory(const std::string& name, uint64_t id);

  // Point into an existing cache file on disk.
  bool loadFromMmap(MmapFile* mmap_file);

  // --- End creation functions.

  // Push the internal data for this instance to CacheSaver for serialization.
  bool save(CacheSaver* cs) const;

  // Get the name which is stored internally.
  const std::string& getName() const {
    return m_name;
  }

  bool isRegularFile() const;
  bool isDirectory() const;
  bool isCompressed() const;
  bool isEmpty() const;

  bool existChecked() {
    // this flag only ever changes from false to true, so do a load first
    // to avoid thrashing the caches for subsequent checks.
    return
      m_exist_checked.load(std::memory_order_relaxed) ||
      m_exist_checked.exchange(true, std::memory_order_relaxed);
  }

  bool dataFetched() {
    return
      m_data_fetched.load(std::memory_order_relaxed) ||
      m_data_fetched.exchange(true, std::memory_order_relaxed);
  }

  uint64_t fileSize() const;

  // Access contents if possible.  Populates all three arguments on success.
  bool getDataPointer(const char** data, uint64_t* datalen,
                              bool* compressed) const;

  // Make a decompressed copy of the data.  Only works if actually compressed.
  // Returns false if data is not compressed, or on any other error.
  bool getDecompressedData(std::string* data) const;

  // Write out the contents of this entry cache to stdout, for logging.
  void dump() const;

  // CacheManager needs fast access to the directory listing, but it
  // stores its files in an unsorted hash table for performance. So
  // we so store a flat list of child items on the CacheData for the
  // parent directory.
  void addChildToDirectory(std::string_view childName);
  std::vector<std::string> getDirectoryChildren() const;

 private:
  static const int kFlag_Compressed  = 0x00000001;

  static const int kFlag_RegularFile = 0x00010000;
  static const int kFlag_EmptyEntry  = 0x00020000;
  static const int kFlag_Directory   = 0x00040000;

  uint64_t createChecksum() const;
  bool sufficientlyCompressed(uint64_t orig_size, uint64_t new_size) const;

  uint64_t m_id;
  uint64_t m_flags;
  uint64_t m_mtime;
  uint64_t m_checksum;

  // It might be something we malloced, or something we got via mmap...
  const char* m_file_data;
  uint64_t m_file_data_length;

  // ... hence this bool.
  bool m_should_free{false};
  std::atomic<bool> m_exist_checked{false};
  std::atomic<bool> m_data_fetched{false};

  // To facilitate rapid directory listings, the CacheManager
  // stores a list of child items on parent directory entries.
  // (The directories may be created on demand when a file is
  // added.) To address the typical usecase (load up a bunch of
  // files initially, and then repeatedly list a few directories),
  // we just append the items to a vector as they're added and
  // set a dirty flag.
  // When/if a particular directory is listed, if it's dirty it
  // gets sorted and cleaned up to preserve legacy behavior,
  // and then for subsequent queries this is extremely fast.
  mutable bool m_directory_children_need_sorting{false};
  mutable std::vector<std::string> m_directory_children;

  std::string m_name;
};

}  // namespace HPHP
