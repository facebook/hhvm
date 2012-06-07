/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/shared/shared_store_base.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/shared/shared_store.h>
#include <runtime/base/shared/concurrent_shared_store.h>
#include <util/timer.h>
#include <sys/mman.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// SharedStore

SharedStore::SharedStore(int id) : m_id(id) {
}

SharedStore::~SharedStore() {
}

std::string SharedStore::GetSkeleton(CStrRef key) {
  std::string ret;
  const char *p = key.data();
  ret.reserve(key.size());
  bool added = false; // whether consecutive numbers are replaced by # yet
  for (int i = 0; i < key.size(); i++) {
    char ch = *p++;
    if (ch >= '0' && ch <= '9') {
      if (!added) {
        ret += '#';
        added = true;
      }
    } else {
      added = false;
      ret += ch;
    }
  }
  return ret;
}

bool SharedStore::erase(CStrRef key, bool expired /* = false */) {
  bool success = eraseImpl(key, expired);

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log(success ? "apc.erased" : "apc.erase", 1);
  }
  return success;
}

void StoreValue::set(SharedVariant *v, int64 ttl) {
  var = v;
  expiry = ttl ? time(NULL) + ttl : 0;
}
bool StoreValue::expired() const {
  return expiry && time(NULL) >= expiry;
}

///////////////////////////////////////////////////////////////////////////////
// SharedStores

SharedStores s_apc_store;

SharedStores::SharedStores() {
}

void SharedStores::create() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    switch (RuntimeOption::ApcTableType) {
      case RuntimeOption::ApcHashTable:
        switch (RuntimeOption::ApcTableLockType) {
          case RuntimeOption::ApcMutex:
            m_stores[i] = new MutexHashTableSharedStore(i);
            break;
          default:
            m_stores[i] = new RwLockHashTableSharedStore(i);
        }
        break;
      case RuntimeOption::ApcLfuTable:
        {
          time_t maturity = RuntimeOption::ApcKeyMaturityThreshold;
          size_t maxCap = RuntimeOption::ApcMaximumCapacity;
          int updatePeriod = RuntimeOption::ApcKeyFrequencyUpdatePeriod;

          if (i == SHARED_STORE_DNS_CACHE) {
            maturity = RuntimeOption::DnsCacheKeyMaturityThreshold;
            maxCap = RuntimeOption::DnsCacheMaximumCapacity;
            updatePeriod = RuntimeOption::DnsCacheKeyFrequencyUpdatePeriod;
          }
          m_stores[i] = new LfuTableSharedStore(i, maturity, maxCap,
              updatePeriod);
        }
        break;
      case RuntimeOption::ApcConcurrentTable:
        m_stores[i] = new ConcurrentTableSharedStore(i);
        break;
      default:
        ASSERT(false);
    }
  }
}

SharedStores::~SharedStores() {
  clear();
}

void SharedStores::clear() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    delete m_stores[i];
    m_stores[i] = NULL;
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
                                    int64 chunkSize, int64 maxSize) {
  Lock lock(m_lock);
  m_prefix = prefix;
  m_chunkSize = chunkSize;
  m_maxSize = maxSize;
  if (m_state != StateInvalid) {
    return;
  }
  if (!addFile()) {
    Logger::Error("Failed to open file for apc, fallback to in-memory mode");
    return;
  }
  m_state = StateOpen;
}

char *SharedStoreFileStorage::put(const char *data, int32 len) {
  Lock lock(m_lock);
  if (m_state != StateOpen ||
      len + PaddingSize > m_chunkSize - PaddingSize) {
    return NULL;
  }
  if (len + PaddingSize > m_chunkRemain && !addFile()) {
    m_state = StateFull;
    return NULL;
  }
  ASSERT(m_current);
  ASSERT(len + PaddingSize <= m_chunkRemain);
  int64 h = hash_string_inline(data, len);
  *(int64*)m_current = h;
  m_current += sizeof(h);
  *(int32*)m_current = len;
  m_current += sizeof(len);
  // should be no overlap
  memcpy(m_current, data, len);
  char *addr = m_current;
  addr[len] = '\0';
  m_current += len + sizeof(char);
  *(int64*)m_current = TombHash;
  m_chunkRemain -= len + PaddingSize;
  return addr;
}

void SharedStoreFileStorage::seal() {
  Lock lock(m_lock);
  if (m_state == StateSealed) {
    return;
  }
  ASSERT(m_state == StateOpen || m_state == StateFull);
  m_current = NULL;
  m_chunkRemain = 0;
  m_state = StateSealed;

  for (int i = 0; i < (int)m_chunks.size(); i++) {
    if (msync(m_chunks[i], m_chunkSize, MS_SYNC) < 0) {
      Logger::Error("Failed to msync chunk %d", i);
    }
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
      int64 h = *(int64*)current;
      if (h == TombHash) {
        break;
      }
      current += sizeof(h);
      int32 len = *(int32*)current;
      current += sizeof(len);
      if (len < 0 ||
          len + PaddingSize >= (int64)boundary - (int64)current) {
        Logger::Error("invalid len %d at chunk %d offset %lld", len, i,
                      (int64)current - (int64)m_chunks[i]);
        return false;
      }
      int64 h_data = hash_string_inline(current, len);
      if (h_data != h) {
        Logger::Error("invalid hash at chunk %d offset %lld", i,
                      (int64)current - (int64)m_chunks[i]);
        return false;
      }
      current += len;
      if (*current != '\0') {
        Logger::Error("missing \\0 at chunk %d offset %lld", i,
                      (int64)current - (int64)m_chunks[i]);
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
  if ((int64)m_chunks.size() * m_chunkSize >= m_maxSize) {
    m_state = StateFull;
    return false;
  }
  char name[PATH_MAX];
  snprintf(name, 256, "%s.XXXXXX", m_prefix.c_str());
  int fd = mkstemp(name);
  if (fd < 0) {
    Logger::Error("Failed to open temp file");
    return false;
  }
  if (posix_fallocate(fd, 0, m_chunkSize)) {
    Logger::Error("Failred to posix_fallocate of size %llu", m_chunkSize);
    close(fd);
    return false;
  }
  if (RuntimeOption::ApcFileStorageKeepFileLinked) {
    m_fileNames.push_back(std::string(name));
  } else {
    unlink(name);
  }
  char *addr = (char *)mmap(NULL, m_chunkSize, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (addr == (char *)-1) {
    Logger::Error("Failed to mmap of size %llu", name, m_chunkSize);
    close(fd);
    return false;
  }
  m_current = addr;
  m_chunkRemain = m_chunkSize - PaddingSize;
  m_chunks.push_back(addr);
  close(fd);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
