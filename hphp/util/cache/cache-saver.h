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

// Responsible for creating the on-disk representation of the cache.
//
// Used by CacheManager and the CacheData instances to commit their data.

// --- On-disk format:
//
// All numerics are uint64_t, little-endian (Intel style).
//
// All strings are a numeric length counter and then that many chars.
//
// Note: strings are NOT \0 terminated, so it is not safe to treat them
//       as "C strings".  You must always use the accompanying length data.
//
// [header] [dirent*] [end-of-directory] [file-data*]
//
// header: [magic] [directory size]
//  - magic: numeric, used to identify this format.
//  - directory size: numeric, number of dirents to follow.
//
// dirent: [id] [flags] [mtime] [checksum] [data ofs] [data len] [name]
//  - id       : numeric, unique identifier for an entry in the cache.
//  - flags    : numeric, see kFlag_* in cache-data.h.
//  - checksum : numeric, currently unused.
//  - data ofs : numeric, file offset of contents for this entry.
//  - data len : numeric, length of contents for this entry.
//  - name     : string, the name of this entry.
//
// end-of-directory: numeric, magic number used to verify directory size.
//
// file-data: [char*]
//            note: no terminating \0 - do not use C strxxx() functions!
//            files occur in same order as dirents.
//

// --- Writing technique:
//
// Writing has one full pass and then a bunch of surgical updates.
//
// The first pass lays down the header and directory, but the directory
// entries have a bogus canary value for the "file offset".  This is just a
// number which says how far into the file the contents are - since we
// haven't gotten there yet, we can't know it at this point.  We save
// the location of these canary values in FilePointers.
//
// After writing the directory and directory terminator, the entry contents
// are then written back to back.  The locations of these contents are added
// to the FilePointers.
//
// All of this up to this point has been a linear write from 0.
//
// Finally, for each file we wrote, we seek back to the directory entry
// and overwrite the canary value with the location data captured earlier.
//
// This was thought to be simpler than attempting to pre-compute the offsets
// because formats tend to change over time.  This approach is resilient to
// changes to the format since it'll still pick up the right content offsets.

#ifndef incl_HPHP_CACHE_SAVER_H_
#define incl_HPHP_CACHE_SAVER_H_

#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

#include <boost/utility.hpp>

namespace HPHP {

class CacheSaver : private boost::noncopyable {
 public:
  struct DirEntry {
    uint64_t id;
    uint64_t flags;
    uint64_t checksum;
    uint64_t mtime;
    uint64_t data_len;
    const char* data_ptr;
    std::string name;
  };

  explicit CacheSaver(const std::string& path);
  ~CacheSaver();

  // Open file for writing and write initial magic number and directory size.
  //
  // Class behavior is undefined until this returns true.
  bool init(uint64_t file_magic, uint64_t num_dirents);

  // Write a single directory entry.  Used by CacheData.
  bool writeDirEntry(const DirEntry& direntry);

  // Write the end of directory marker.
  bool endDirectory(uint64_t directory_end_magic);

  // Write the file data, remembering their offsets into the output file.
  bool writeFiles();

  // Go back and set the data offset values.
  bool rewriteDirectory(uint64_t expected_directory_len);

  // Close the file.
  bool finish();

 private:
  struct FilePointer {
    uint64_t id;

    const char* data_ptr;
    uint64_t data_len;

    // The offset of the directory entry which will hold the data offset.
    // Captured while writing the initial directory pass.
    uint64_t dirent_update_ofs;

    // The offset of the data within the bigger save file.
    // Captured while writing the data to the save file.
    uint64_t data_ofs;
  };

  // Nicer wrappers for lseek().
  off_t getOfs() const;
  bool setOfs(off_t ofs) const;

  bool writeUInt64(uint64_t value) const;

  // Strings have a length byte in front, via an internal call to writeUInt64.
  bool writeString(const std::string& str) const;

  std::string path_;
  bool initialized_;

  int fd_;

  // Remember where the directory started, for use in rewriteDirectory().
  off_t directory_ofs_;

  // Details given to us by CacheData instances.
  std::vector<FilePointer> file_pointers_;
};

}  // namespace HPHP

#endif  // incl_HPHP_CACHE_SAVER_H_
