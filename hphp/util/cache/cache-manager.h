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

// Static object cache.
//
// Use it to read resources in from disk and keep them in memory.  Then
// you can write the whole cache out to disk and distribute a single
// (large!) cache object instead of shipping individual resources around.
//
// This cache is "write once, read many".  There are no provisions for
// changing the contents once saved on disk.
//
// This is the only class which should be accessed by callers outside
// of hphp/util/cache.

#ifndef incl_HPHP_CACHE_MANAGER_H_
#define incl_HPHP_CACHE_MANAGER_H_

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>

#include <boost/utility.hpp>

namespace HPHP {

class CacheData;
class MmapFile;

class CacheManager : private boost::noncopyable {
 public:
  CacheManager();
  ~CacheManager();

  // Look up a named file to retrieve its contents.
  //
  // Returns true on success and populates data, data_len and compressed.
  //
  // Caller must not free "data" - it's owned by the cache.
  //
  // Caller is responsible for decompressing data if desired.
  bool getFileContents(const std::string& name, const char** data,
                       uint64_t* data_len, bool* compressed) const;

  // Like getFileContents, but this code does the decompression for you.
  //
  // Only works for files which are compressed (returns false otherwise).
  //
  // Returns true on success and populates data.
  bool getDecompressed(const std::string& name, std::string* data) const;

  // Find out if a named file is actually compressed without fetching it.
  bool isCompressed(const std::string& name) const;

  // Create a named entry with contents of regular file at <path>.
  // name must be unique within this CacheManager instance.
  //
  // Attempts to compress data, and keeps it in compressed form if
  // the compressor's output is deemed to be a sufficient improvement.
  // (see CacheData::sufficientlyCompressed for adjustments)
  //
  // Returns true on success.
  bool addFileContents(const std::string& name, const std::string& path);

  // Create a named entry with no contents.
  bool addEmptyEntry(const std::string& name);

  // Find out if any entry exists as <name> regardless of type.
  bool entryExists(const std::string& name) const;

  // Like entryExists, but only for regular files (those originally
  // added via addFileContents).
  bool fileExists(const std::string& name) const;

  // Like entryExists, but only for directories.
  bool dirExists(const std::string& name) const;

  // Yep, you guessed it.
  bool emptyEntryExists(const std::string& name) const;

  // Get the size of a named entry after decompression.
  // Decompresses the data on the fly if necessary - use with care!
  // Returns true on success and populates size.
  bool getUncompressedFileSize(const std::string& name, uint64_t* size) const;

  // Load a serialized instance of the cache from disk.
  bool loadCache(const std::string& path);

  // Write a serialized copy of the cache to disk.
  bool saveCache(const std::string& path) const;

  void getEntryNames(std::set<std::string>* names) const;

 private:
  void addDirectories(const std::string& name);

  std::unique_ptr<MmapFile> mmap_file_;
  uint64_t entry_counter_;

  typedef struct std::map<std::string, std::unique_ptr<CacheData>> CacheMap;
  CacheMap cache_map_;
};

}  // namespace HPHP

#endif  // incl_HPHP_CACHE_MANAGER_H_
