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
#include "hphp/runtime/base/apc-file-storage.h"

#include <sys/mman.h>

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

#if !defined(HAVE_POSIX_FALLOCATE) && \
  (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L || defined(__CYGWIN__))
# define HAVE_POSIX_FALLOCATE 1
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

APCFileStorage s_apc_file_storage;

void APCFileStorage::enable(const std::string& prefix, size_t chunkSize) {
  std::lock_guard<std::mutex> lock(m_lock);
  if (chunkSize <= PaddingSize) return;
  m_prefix = prefix;
  m_chunkSize = chunkSize;
  if (m_state != StorageState::Invalid) {
    return;
  }
  if (!addFile()) {
    Logger::Error(
        "Failed to open initial file for file-backed APC storage, "
        "falling back to in-memory mode");
    m_state = StorageState::Full;
    return;
  }
  m_state = StorageState::Open;
}

char* APCFileStorage::put(const char* data, uint32_t len) {
  const uint32_t totalLen = len + PaddingSize;
  if (m_state != StorageState::Open || totalLen > m_chunkSize) {
    return nullptr;
  }
  assert(!m_chunks.empty());

  auto maxOffset = m_chunkSize - totalLen;
  auto current = m_current.load(std::memory_order_relaxed);
  do {
    if (UNLIKELY(static_cast<uint32_t>(current) > maxOffset)) {
      std::lock_guard<std::mutex> lock(m_lock);
      // Check again after we have the lock, other threads may have already
      // created a new chunk.
      current = m_current.load(std::memory_order::memory_order_relaxed);
      if (static_cast<uint32_t>(current) > maxOffset) {
        // It is our duty to add a chunk.  Other threads can continue to use
        // the current chunk, or block on the lock if they also find the need
        // for a new chunk.
        if (!addFile()) {
          Logger::Error(
            "Failed to open additional files for file-backed APC storage, "
             "falling back to in-memory mode");
          m_state = StorageState::Full;
          return nullptr;
        }
      }
    }
    // Try grabbing the memory
    if (m_current.compare_exchange_weak(current, current + totalLen,
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed)) {
      break;
    }
  } while(true);

  auto const chunkIndex = current >> 32;
  char* base = m_chunks[chunkIndex] + static_cast<uint32_t>(current);
  uint64_t h = hash_string_unsafe(data, len);
  *(uint64_t*)base = h | (static_cast<uint64_t>(len) << 32);
  base += sizeof(uint64_t);
  assert(base[len] == '\0');            // Should already be 0 after mmap.
  // Return the starting address of the string.
  return static_cast<char*>(memcpy(base, data, len));
}

void APCFileStorage::seal() {
  std::lock_guard<std::mutex> lock(m_lock);
  if (m_state == StorageState::Sealed || m_chunks.empty()) {
    return;
  }
  assert(m_state == StorageState::Open || m_state == StorageState::Full);
  m_state = StorageState::Sealed;

  auto const current = m_current.load(std::memory_order_acquire);
  auto const offset = static_cast<uint32_t>(current);
  if (offset < m_chunkSize - PaddingSize) {
    *reinterpret_cast<strhash_t*>(m_chunks.back() + offset) = TombHash;
  }
  m_current = m_chunkSize;

  for (int i = 0; i < (int)m_chunks.size(); i++) {
    if (mprotect(m_chunks[i], m_chunkSize, PROT_READ) < 0) {
      Logger::Error("Failed to mprotect chunk %d", i);
    }
  }
}

void APCFileStorage::adviseOut() {
  std::lock_guard<std::mutex> lock(m_lock);
  Timer timer(Timer::WallTime, "advising out apc prime");
  Logger::FInfo("Advising out {} APCFileStorage chunks\n", m_chunks.size());
  for (int i = 0; i < (int)m_chunks.size(); i++) {
    if (madvise(m_chunks[i], m_chunkSize, MADV_DONTNEED) < 0) {
      Logger::Error("Failed to madvise chunk %d", i);
    }
  }
}

bool APCFileStorage::hashCheck() {
  std::lock_guard<std::mutex> lock(m_lock);
  for (int i = 0; i < (int)m_chunks.size(); i++) {
    char* current = (char*)m_chunks[i];
    char* boundary = (char*)m_chunks[i] + m_chunkSize;
    while (1) {
      // We may not have TombHash at the end if the chunk is used up.
      if (reinterpret_cast<uintptr_t>(current) + PaddingSize >=
          reinterpret_cast<uintptr_t>(boundary)) {
        break;
      }
      strhash_t h = *(strhash_t*)current;
      if (h == TombHash) {
        break;
      }
      current += sizeof(h);
      int32_t len = *(int32_t*)current;
      current += sizeof(len);
      if (len < 0 ||
          len + sizeof(char) >= (int64_t)boundary - (int64_t)current) {
        Logger::Error("invalid len %d at chunk %d offset %" PRId64, len, i,
                      (int64_t)current - (int64_t)m_chunks[i]);
        return false;
      }
      strhash_t h_data = hash_string(current, len);
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

void APCFileStorage::cleanup() {
  std::lock_guard<std::mutex> lock(m_lock);
  for (unsigned int i = 0 ; i < m_fileNames.size(); i++) {
    unlink(m_fileNames[i].c_str());
  }
  m_chunks.clear();
}

bool APCFileStorage::addFile() {
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
  if (ret == -1) {
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
  char* addr = (char*)mmap(nullptr, m_chunkSize, PROT_READ | PROT_WRITE,
                           MAP_SHARED, fd, 0);
  if (addr == (char*)-1) {
    Logger::Error("Failed to mmap %s of size %" PRId64, name, m_chunkSize);
    close(fd);
    return false;
  }
  numa_interleave(addr, m_chunkSize);

  if (!m_chunks.empty()) {
    // We need to finish the previous chunk by writing a TombHash at the end,
    // if there is enough space.  If the usable space in the chunk is smaller
    // than PaddingSize, no TombHash is needed, because we can tell that the
    // chunk is used up.
    auto maxOffset = m_chunkSize - PaddingSize;
    auto current = m_current.load(std::memory_order_relaxed);
    while (static_cast<uint32_t>(current) < maxOffset) {
      if (m_current.compare_exchange_weak(current, current + PaddingSize,
                                          std::memory_order_acquire,
                                          std::memory_order_relaxed)) {
        // Guaranteed by the memory_order_acquire
        assert(current >> 32 == m_chunks.size() - 1);
        auto const p = m_chunks.back() + static_cast<uint32_t>(current);
        *reinterpret_cast<strhash_t*>(p) = TombHash;
        break;
      }
    }
  }

  m_chunks.push_back(addr);
  // memory_order_release guarantees that the new chunk is already present in
  // m_chunks when we reset m_current.  It is OK if other threads still try to
  // use the previous chunk before this point.
  m_current.store(static_cast<int64_t>(m_chunks.size() - 1) << 32,
                  std::memory_order_release);
  close(fd);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

}
