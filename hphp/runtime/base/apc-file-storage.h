/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_APC_FILE_STORAGE_H_
#define incl_HPHP_APC_FILE_STORAGE_H_

#include <atomic>
#include <vector>
#include <mutex>

#include "hphp/util/hash.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * To save memory, hhvm puts portions of primed APC data in a file-backed mmap
 * that we can madvise away after an initial warmup period.
 */
struct APCFileStorage {
  enum class StorageState {
    Invalid,
    Open,
    Sealed,
    Full
  };

  APCFileStorage() = default;
  APCFileStorage(const APCFileStorage&) = delete;
  APCFileStorage& operator=(const APCFileStorage&) = delete;

  void enable(const std::string& prefix, size_t chunkSize);
  char *put(const char *data, uint32_t len);
  void seal();
  void adviseOut();
  bool hashCheck();
  void cleanup();
  StorageState getState() { return m_state; }

private:
  bool addFile();

private:
  // [32-bit chunk index]:[32-bit offset]
  std::atomic_uint_fast64_t m_current{0};
  size_t m_chunkSize{0};
  StorageState m_state{StorageState::Invalid};
  std::vector<char*> m_chunks;
  std::vector<std::string> m_fileNames;
  std::string m_prefix;

  // This lock is needed when manipulating chunks.
  std::mutex m_lock;
  static constexpr strhash_t TombHash = 0xdeadbeef;
  static constexpr uint32_t PaddingSize = sizeof(strhash_t) + // hash
                                          sizeof(int32_t) + // len
                                          sizeof(char); // '\0'
};

extern APCFileStorage s_apc_file_storage;

//////////////////////////////////////////////////////////////////////

}

#endif
