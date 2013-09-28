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

// Owner of the mmapped file which is the cache.
// Handles all read accesses.  No writing is permitted (WORM cache).

#ifndef incl_HPHP_MMAP_FILE_H_
#define incl_HPHP_MMAP_FILE_H_

#include <cstdint>
#include <string>

#include <boost/utility.hpp>

namespace HPHP {

class MmapFile : private boost::noncopyable {
 public:
  explicit MmapFile(const std::string& path);
  ~MmapFile();

  // Class behavior is undefined until this returns true.
  // Read pointer is initialized to the beginning of the file.
  bool init();

  // Read next value from the file if possible.  Moves the read pointer.
  bool readUInt64(uint64_t* value);
  bool readString(std::string* str);

  // Create a pointer to a given offset in the file.
  bool makePointer(uint64_t offset, uint64_t length, const char** ptr) const;

 private:
  bool wouldExceedMem(uint64_t len) const;

  std::string path_;
  bool initialized_;

  int backing_fd_;
  void* backing_mem_;
  void* backing_mem_end_;

  off_t backing_mem_size_;
  char* read_ptr_;
};

}  // namespace HPHP

#endif  // incl_HPHP_MMAP_FILE_H_
