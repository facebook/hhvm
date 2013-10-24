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

#include "hphp/runtime/base/shared-store-base.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/concurrent-shared-store.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/util/alloc.h"
#include "hphp/util/timer.h"
#include "hphp/util/logger.h"
#include <sys/mman.h>

#if !defined(HAVE_POSIX_FALLOCATE) && \
  (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L)
# define HAVE_POSIX_FALLOCATE 1
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void StoreValue::set(APCHandle *v, int64_t ttl) {
  var = v;
  expiry = ttl ? time(nullptr) + ttl : 0;
}
bool StoreValue::expired() const {
  return expiry && time(nullptr) >= expiry;
}

//////////////////////////////////////////////////////////////////////

SharedStores s_apc_store;

SharedStores::SharedStores() {
}

void SharedStores::create() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    switch (apcExtension::TableType) {
      case apcExtension::TableTypes::ConcurrentTable:
        m_stores[i] = new ConcurrentTableSharedStore(i);
        break;
      default:
        assert(false);
    }
  }
}

SharedStores::~SharedStores() {
  clear();
}

void SharedStores::clear() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    delete m_stores[i];
    m_stores[i] = nullptr;
  }
}

void SharedStores::reset() {
  clear();
  create();
}

void SharedStores::Create() {
  s_apc_store.create();
}

///////////////////////////////////////////////////////////////////////////////

SharedStoreFileStorage s_apc_file_storage;

void SharedStoreFileStorage::enable(const std::string& prefix,
                                    int64_t chunkSize, int64_t maxSize) {
  Lock lock(m_lock);
  m_prefix = prefix;
  m_chunkSize = chunkSize;
  m_maxSize = maxSize;
  if (m_state != StorageState::Invalid) {
    return;
  }
  if (!addFile()) {
    Logger::Error("Failed to open file for apc, fallback to in-memory mode");
    return;
  }
  m_state = StorageState::Open;
}

char *SharedStoreFileStorage::put(const char *data, int32_t len) {
  Lock lock(m_lock);
  if (m_state != StorageState::Open ||
      len + PaddingSize > m_chunkSize - PaddingSize) {
    return nullptr;
  }
  if (len + PaddingSize > m_chunkRemain && !addFile()) {
    m_state = StorageState::Full;
    return nullptr;
  }
  assert(m_current);
  assert(len + PaddingSize <= m_chunkRemain);
  strhash_t h = hash_string_inline(data, len);
  *(strhash_t*)m_current = h;
  m_current += sizeof(h);
  *(int32_t*)m_current = len;
  m_current += sizeof(len);
  // should be no overlap
  memcpy(m_current, data, len);
  char *addr = m_current;
  addr[len] = '\0';
  m_current += len + sizeof(char);
  *(strhash_t*)m_current = TombHash;
  m_chunkRemain -= len + PaddingSize;
  return addr;
}

void SharedStoreFileStorage::seal() {
  Lock lock(m_lock);
  if (m_state == StorageState::Sealed) {
    return;
  }
  assert(m_state == StorageState::Open || m_state == StorageState::Full);
  m_current = nullptr;
  m_chunkRemain = 0;
  m_state = StorageState::Sealed;

  for (int i = 0; i < (int)m_chunks.size(); i++) {
    if (mprotect(m_chunks[i], m_chunkSize, PROT_READ) < 0) {
      Logger::Error("Failed to mprotect chunk %d", i);
    }
  }
}

void SharedStoreFileStorage::adviseOut() {
  Lock lock(m_lock);
  Timer timer(Timer::WallTime, "advising out apc prime");
  for (int i = 0; i < (int)m_chunks.size(); i++) {
    if (madvise(m_chunks[i], m_chunkSize, MADV_DONTNEED) < 0) {
      Logger::Error("Failed to madvise chunk %d", i);
    }
  }
}

bool SharedStoreFileStorage::hashCheck() {
  Lock lock(m_lock);
  for (int i = 0; i < (int)m_chunks.size(); i++) {
    char *current = (char*)m_chunks[i];
    char *boundary = (char*)m_chunks[i] + m_chunkSize;
    while (1) {
      strhash_t h = *(strhash_t*)current;
      if (h == TombHash) {
        break;
      }
      current += sizeof(h);
      int32_t len = *(int32_t*)current;
      current += sizeof(len);
      if (len < 0 ||
          len + PaddingSize >= (int64_t)boundary - (int64_t)current) {
        Logger::Error("invalid len %d at chunk %d offset %" PRId64, len, i,
                      (int64_t)current - (int64_t)m_chunks[i]);
        return false;
      }
      strhash_t h_data = hash_string_inline(current, len);
      if (h_data != h) {
        Logger::Error("invalid hash at chunk %d offset %" PRId64, i,
                      (int64_t)current - (int64_t)m_chunks[i]);
        return false;
      }
      current += len;
      if (*current != '\0') {
        Logger::Error("missing \\0 at chunk %d offset %" PRId64, i,
                      (int64_t)current - (int64_t)m_chunks[i]);
        return false;
      }
      current++;
    }
  }
  return true;
}

void SharedStoreFileStorage::cleanup() {
  Lock lock(m_lock);
  for (unsigned int i = 0 ; i < m_fileNames.size(); i++) {
    unlink(m_fileNames[i].c_str());
  }
  m_chunks.clear();
}

bool SharedStoreFileStorage::addFile() {
  if ((int64_t)m_chunks.size() * m_chunkSize >= m_maxSize) {
    m_state = StorageState::Full;
    return false;
  }
  char name[PATH_MAX];
  snprintf(name, sizeof(name), "%s.XXXXXX", m_prefix.c_str());
  int fd = mkstemp(name);
  if (fd < 0) {
    Logger::Error("Failed to open temp file");
    return false;
  }
  bool couldAllocate = false;
#if defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  couldAllocate = posix_fallocate(fd, 0, m_chunkSize) == 0;
#elif defined(__APPLE__)
  fstore_t store = { F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, m_chunkSize };
  int ret = fcntl(fd, F_PREALLOCATE, &store);
  if(ret == -1) {
    store.fst_flags = F_ALLOCATEALL;
    ret = fcntl(fd, F_PREALLOCATE, &store);
    if (ret == -1) {
        couldAllocate = false;
    }
  }
  couldAllocate = ftruncate(fd, m_chunkSize) == 0;
#else
 #error "No implementation for posix_fallocate on your platform."
#endif
  if (!couldAllocate) {
    Logger::Error("Failed to posix_fallocate of size %" PRId64, m_chunkSize);
    close(fd);
    return false;
  }
  if (apcExtension::FileStorageKeepFileLinked) {
    m_fileNames.push_back(std::string(name));
  } else {
    unlink(name);
  }
  char *addr = (char *)mmap(nullptr, m_chunkSize, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (addr == (char *)-1) {
    Logger::Error("Failed to mmap %s of size %" PRId64, name, m_chunkSize);
    close(fd);
    return false;
  }
  Util::numa_interleave(addr, m_chunkSize);
  m_current = addr;
  m_chunkRemain = m_chunkSize - PaddingSize;
  m_chunks.push_back(addr);
  close(fd);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

}
