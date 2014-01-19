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

#ifndef incl_HPHP_SHARED_STORE_BASE_H_
#define incl_HPHP_SHARED_STORE_BASE_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/concurrent-shared-store.h"
#include "hphp/util/lock.h"
#include "hphp/runtime/base/complex-types.h"

#define SHARED_STORE_APPLICATION_CACHE 0
#define SHARED_STORE_DNS_CACHE 1
#define MAX_SHARED_STORE 2

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedStores {
public:
  static void Create();

public:
  SharedStores();
  ~SharedStores();
  void create();
  void clear();
  void reset();

  ConcurrentTableSharedStore& operator[](int id) {
    assert(id >= 0 && id < MAX_SHARED_STORE);
    return *m_stores[id];
  }

private:
  ConcurrentTableSharedStore* m_stores[MAX_SHARED_STORE];
};

extern SharedStores s_apc_store;

///////////////////////////////////////////////////////////////////////////////

class SharedStoreFileStorage {
public:
  enum class StorageState {
    Invalid,
    Open,
    Sealed,
    Full
  };

  SharedStoreFileStorage()
  : m_state(StorageState::Invalid), m_current(nullptr), m_chunkRemain(0) {}
  void enable(const std::string& prefix, int64_t chunkSize, int64_t maxSize);
  char *put(const char *data, int32_t len);
  void seal();
  void adviseOut();
  bool hashCheck();
  void cleanup();
  StorageState getState() { return m_state; }

private:
  bool addFile();

private:
  std::vector<void*> m_chunks;
  std::string m_prefix;
  int64_t m_chunkSize;
  int64_t m_maxSize;
  StorageState m_state;
  char *m_current;
  int32_t m_chunkRemain;
  std::vector<std::string> m_fileNames;

  Mutex m_lock;
  static const strhash_t TombHash = 0xdeadbeef;
  static const int PaddingSize = sizeof(strhash_t) + // hash
                                 sizeof(int32_t) + // len
                                 sizeof(char); // '\0'
};

extern SharedStoreFileStorage s_apc_file_storage;

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_SHARED_STORE_BASE_H_ */
