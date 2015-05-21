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

// Individual in-memory component of the cache.
// Usually normal files (web static resources).

#ifndef incl_HPHP_CACHE_DATA_H_
#define incl_HPHP_CACHE_DATA_H_

#include <cstdint>
#include <string>

#include <boost/utility.hpp>

namespace HPHP {

class CacheSaver;
class MmapFile;

class CacheData : private boost::noncopyable {
 public:
  CacheData();
  ~CacheData();

  // --- Creation functions: call at most one of these per instance.

  // Create a named entry with contents of regular file at <path>.
  bool loadFromFile(const std::string& name, uint64_t id,
                    const std::string& path);

  // Create a named entry without any contents.
  void createEmpty(const std::string& name, uint64_t id);

  // Create a named directory entry.
  void createDirectory(const std::string& name, uint64_t id);

  // Point into an existing cache file on disk.  Populates name on success.
  bool loadFromMmap(MmapFile* mmap_file, std::string* name);

  // --- End creation functions.

  // Push the internal data for this instance to CacheSaver for serialization.
  bool save(CacheSaver* cs) const;

  bool isRegularFile() const;
  bool isDirectory() const;
  bool isCompressed() const;
  bool isEmpty() const;

  uint64_t fileSize() const;

  // Access contents if possible.  Populates all three arguments on success.
  bool getDataPointer(const char** data, uint64_t* datalen,
                              bool* compressed) const;

  // Make a decompressed copy of the data.  Only works if actually compressed.
  // Returns false if data is not compressed, or on any other error.
  bool getDecompressedData(std::string* data) const;

  void dump() const;

 private:
  static const int kFlag_Compressed  = 0x00000001;

  static const int kFlag_RegularFile = 0x00010000;
  static const int kFlag_EmptyEntry  = 0x00020000;
  static const int kFlag_Directory   = 0x00040000;

  uint64_t createChecksum() const;
  bool sufficientlyCompressed(uint64_t orig_size, uint64_t new_size) const;

  uint64_t id_;
  uint64_t flags_;
  uint64_t mtime_;
  uint64_t checksum_;

  // It might be something we malloced, or something we got via mmap...
  const char* file_data_;
  uint64_t file_data_length_;

  // ... hence this bool.
  bool should_free_;

  std::string name_;
};

}  // namespace HPHP

#endif  // incl_HPHP_CACHE_DATA_H_
