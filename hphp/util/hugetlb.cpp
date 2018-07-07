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

#include "hphp/util/hugetlb.h"

// Techniques used here are Linux-specific, so don't bother to be portable.
#ifdef __linux__
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <fcntl.h>
#ifdef HAVE_NUMA
#include <numaif.h>
#endif
#include <unistd.h>

#include "hphp/util/kernel-version.h"
#include "hphp/util/numa.h"

#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace HPHP {

static char s_hugePath[256];
constexpr size_t maxErrorMsgLen = 256;
static char s_errorMsg[maxErrorMsgLen];

static unsigned s_num1GPages;
constexpr unsigned kMaxNum1GPages = 16;
static void* s_1GPages[kMaxNum1GPages];

static unsigned s_num2MPages;

// Record error message based on errno, with an optional message.
static void record_err_msg(const char* msg = nullptr) {
  size_t len = 0;
  if (msg) {
    len = strlen(msg);
    if (len > maxErrorMsgLen / 2) {
      len = maxErrorMsgLen / 2;
    }
    memcpy(s_errorMsg, msg, len);
    s_errorMsg[len] = 0;
  } else {
    len = strlen(s_errorMsg);
  }
#ifdef __linux__
#ifdef _GNU_SOURCE
  char* err = strerror_r(errno, s_errorMsg + len, maxErrorMsgLen - len);
  if (len == strlen(s_errorMsg)) {
    size_t appendLen = strlen(err);
    if (appendLen + len >= maxErrorMsgLen) {
      appendLen = maxErrorMsgLen - 1 - len;
    }
    memcpy(s_errorMsg + len, err, appendLen);
    s_errorMsg[len + appendLen] = 0;
  }
#else
  strerror_r(errno, s_errorMsg + len, maxErrorMsgLen - len);
#endif
#endif
}

const char* get_hugetlb_err_msg() {
  return s_errorMsg;
}

// Return the page size for hugetlbfs mount point, or 0 if anything goes wrong:
// e.g., mount point doesn't exist, mount point isn't hugetlbfs.
static size_t get_hugepage_size(const char* path) {
#ifdef __linux__
  struct statfs64 sb;
  if (statfs64(path, &sb) == 0) {
    // Magic number defined in Linux kernel: include/uapi/linux/magic.h
    auto constexpr HUGETLBFS_MAGIC = 0x958458f6;
    if (sb.f_type == HUGETLBFS_MAGIC) {
      return sb.f_bsize;
    } else {
      snprintf(s_errorMsg, maxErrorMsgLen,
               "path %s isn't mounted as hugetlbfs", path);
    }
  } else {
    snprintf(s_errorMsg, maxErrorMsgLen,
             "statfs64() for %s failed: ", path);
    record_err_msg();
  }
#endif
  return 0;
}

bool set_hugetlbfs_path(const char* path) {
  if (get_hugepage_size(path) != size1g) return false;
  size_t len = strlen(path);
  if (len + 8 >= sizeof(s_hugePath)) return false;
  memcpy(s_hugePath, path, len);
  *reinterpret_cast<int*>(s_hugePath + len) = 0;
  if (s_hugePath[len - 1] != '/') {
    s_hugePath[len] = '/';
  }
  return true;
}

bool find_hugetlbfs_path() {
#ifdef __linux__
  auto mounts = fopen("/proc/mounts", "r");
  if (!mounts) return false;
  // Search the file for lines like the following
  // none /dev/hugepages hugetlbfs seclabel,relatime...
  char line[4096];
  char path[4096];
  char option[4096];
  while (fgets(line, sizeof(line), mounts)) {
    if (sscanf(line, "%*s %s hugetlbfs %s", path, option) == 2) {
      // It matches hugetlbfs, check page size and save results.
      if (set_hugetlbfs_path(path)) {
        fclose(mounts);
        return true;
      }
    }
  }
  fclose(mounts);
#endif
  return false;
}

HugePageInfo read_hugepage_info(size_t pagesize, int node /* = -1 */) {
  int nr_huge = 0, free_huge = 0;
  if (pagesize != size2m && pagesize != size1g) { // only 2M and 1G supported
    return HugePageInfo{0, 0};
  }
#ifdef __linux__
  if (node >= 0) {
    auto const readNumFrom = [] (const char* path) {
      int result = 0;
      char buffer[32];
      memset(buffer, 0, sizeof(buffer));
      int fd = open(path, O_RDONLY);
      if (fd < 0) return result;
      bool done = false;
      do {
        ssize_t bytes = read(fd, buffer, 20);
        if (bytes == 0) break;          // EOF
        if (bytes < 0) {
          if (errno == EINTR) continue; // try again
          break;                        // totally failed
        }
        for (ssize_t i = 0; i < bytes; ++i) {
          char c = buffer[i];
          // only read numbers, and stop on white space, etc.
          if (c < '0' || c > '9') {
            done = true;
            break;
          }
          result = result * 10 + c - '0';
        }
      } while (!done);
      close(fd);
      return result;
    };

    char fileName[256];
    memcpy(fileName, "/sys/devices/system/node/node", 29);
    assert(strlen("/sys/devices/system/node/node") == 29);
    char* p = fileName + 29;
    // We support at most 32 NUMA node, so at most two bytes.
    if (node >= 10) *p++ = '0' + node / 10;
    *p++ = '0' + node % 10;
    if (pagesize == size2m) {
      memcpy(p, "/hugepages/hugepages-2048kB/", 28);
      assert(strlen("/hugepages/hugepages-2048kB/") == 28);
      p += 28;
    } else {
      memcpy(p, "/hugepages/hugepages-1048576kB/", 31);
      assert(strlen("/hugepages/hugepages-1048576kB/") == 31);
      p += 31;
    }

    memcpy(p, "nr_hugepages", 13);
    assert(strlen("nr_hugepages") == 12); // extra \0 byte
    nr_huge = readNumFrom(fileName);

    memcpy(p, "free_hugepages", 15);
    assert(strlen("free_hugepages") == 14); // extra \0 byte
    free_huge = readNumFrom(fileName);

    return HugePageInfo{nr_huge, free_huge};
  }
  // All nodes
#ifdef HAVE_NUMA
  const int MAX_NUMA_NODE = numa_max_node();
#else
  constexpr int MAX_NUMA_NODE = 0;
#endif
  for (int i = 0; i <= MAX_NUMA_NODE; ++i) {
    // Skip nodes we are not allowed to allocate on.
    if (!numa_node_allowed(i)) continue;
    auto const info = read_hugepage_info(pagesize, i);
    nr_huge += info.nr_hugepages;
    free_huge += info.free_hugepages;
  }
#endif
  return HugePageInfo{nr_huge, free_huge};
}

HugePageInfo get_huge1g_info(int node /* = -1 */) {
  return read_hugepage_info(size1g, node);
}

HugePageInfo get_huge2m_info(int node /* = -1 */) {
  return read_hugepage_info(size2m, node);
}

bool auto_mount_hugetlbfs() {
#ifdef __linux__
  auto const info = get_huge1g_info();
  if (info.nr_hugepages <= 0) return false; // No page reserved.

  const char* hugePath = "/tmp/huge1g";
  if (mkdir(hugePath, 0777)) {
    if (errno != EEXIST) {
      snprintf(s_errorMsg, maxErrorMsgLen, "Failed to mkdir %s: ", hugePath);
      record_err_msg();
      return false;
    }
  }
  if (mount("none", hugePath, "hugetlbfs", 0, "pagesize=1G,mode=0777")) {
    record_err_msg("Failed to mount hugetlbfs with 1G page size: ");
    return false;
  }
  return set_hugetlbfs_path(hugePath);
#else
  return false;
#endif
}

#ifdef __linux__
// Beware that MAP_FIXED overrides existing mapping silently.  If the specified
// memory was mapped in, it may no longer be after this function fails.
// mincore() can be used to check if a memory region is stilled mapped in.
inline void* mmap_2m_impl(void* addr, int prot, bool shared, bool fixed) {
  void* ret = MAP_FAILED;
  int flags = MAP_ANONYMOUS | MAP_HUGETLB
    | (shared ? MAP_SHARED : MAP_PRIVATE)
    | (fixed ? MAP_FIXED : 0);
  // MAP_HUGE_2MB can be specified after 3.8 kernel.
  static KernelVersion version;
  if (version.m_major > 3 || (version.m_major == 3 && version.m_minor >= 8)) {
#ifndef MAP_HUGE_2MB
#define MAP_HUGE_2MB (21 << 26)
#endif
    flags |= MAP_HUGE_2MB;
  }
  ret = mmap(addr, size2m, prot, flags, -1, 0);
  if (ret == MAP_FAILED) {
    record_err_msg("mmap() with MAP_HUGE_2MB failed: ");
    return nullptr;
  }
  if (addr && ret != addr) {
    assert(fixed == false);
    // Didn't get the intended address.
    munmap(ret, size2m);
    return nullptr;
  }

  // Fault the page in.  This guarantees availablility of memory, and avoids
  // subsequent errors when the huge page isn't really available.  Ideally the
  // kernel should've failed mmap() in such a case, but it doesn't seem to even
  // with MAP_LOCKED | MAP_POPULATE.
  if (mlock(ret, 1)) {
    snprintf(s_errorMsg, maxErrorMsgLen, "mlock() failed for %p: ", ret);
    record_err_msg();
    munmap(ret, size2m);
    return nullptr;
  }

  return ret;
}

inline void* mmap_1g_impl(void* addr, bool map_fixed) {
  void* ret = MAP_FAILED;
  if (s_hugePath[0] != 0) {
    int fd = -1;
    size_t dirNameLen = strlen(s_hugePath);
    assert(dirNameLen > 0 && s_hugePath[dirNameLen - 1] == '/');
    for (char i = '0'; i <= '9'; ++i) {
      s_hugePath[dirNameLen] = i;
      // We don't put code on 1G huge pages, so no execute permission.
      fd = open(s_hugePath, O_CREAT | O_EXCL | O_RDWR, 0666);
      // Retry a few times if the file already exists.
      if (fd < 0) {
        if (errno == EEXIST) {
          errno = 0;
          continue;
        } else {
          snprintf(s_errorMsg, maxErrorMsgLen,
                   "Failed to create hugetlbfs file %s: ", s_hugePath);
          record_err_msg();
          s_hugePath[dirNameLen] = 0;
          return nullptr;
        }
      } else {
        unlink(s_hugePath);
      }
      break;
    }

    s_hugePath[dirNameLen] = 0;
    if (fd < 0) {
      snprintf(s_errorMsg, maxErrorMsgLen,
               "Failed to create a hugetlbfs file in %s: "
               "it seems already full of files", s_hugePath);
      return nullptr;
    }

    ret = mmap(addr, size1g, PROT_READ | PROT_WRITE,
               MAP_SHARED | (map_fixed ? MAP_FIXED : 0),
               fd, 0);
    if (ret == MAP_FAILED) {
      snprintf(s_errorMsg, maxErrorMsgLen,
               "mmap() for hugetlbfs file failed: ");
      record_err_msg();
    }
    close(fd);
  }

  if (ret == MAP_FAILED) {
    // MAP_HUGE_1GB is available in 3.9 and later kernels
    KernelVersion version;
    if (version.m_major > 3 || (version.m_major == 3 && version.m_minor >= 9)) {
#ifndef MAP_HUGE_1GB
#define MAP_HUGE_1GB (30 << 26)
#endif
      int flags = MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_1GB |
        (map_fixed ? MAP_FIXED : 0);
      ret = mmap(addr, size1g, PROT_READ | PROT_WRITE, flags, -1, 0);
      if (ret == MAP_FAILED) {
        record_err_msg("mmap() with MAP_HUGE_1GB failed: ");
        return nullptr;
      }
    } else {
      return nullptr;
    }
  }

  // Didn't get the desired address.  This can happen is map_fixed is false.
  if (addr != nullptr && ret != addr) {
    snprintf(s_errorMsg, maxErrorMsgLen,
             "mmap() for huge page returned %p, desired %p", ret, addr);
    munmap(ret, size1g);
    return nullptr;
  }

  // Fault the page in.  This guarantees availablility of memory, and avoids
  // SIGBUS when the huge page isn't really available.  In many cases
  // RLIMIT_MEMLOCK isn't big enough for us to lock 1G.  Fortunately that
  // is unnecessary here; a byte should work equally well.
  if (mlock(ret, 1)) {
    snprintf(s_errorMsg, maxErrorMsgLen, "mlock() failed for %p: ", ret);
    record_err_msg();
    munmap(ret, size1g);
    return nullptr;
  }

  return ret;
}
#endif

#ifdef HAVE_NUMA
namespace {
// We support at most 32 NUMA nodes (numa_node_set in 32-bit), so a single
// unsigned long is more than enough for the mask.  This can be used in jemalloc
// allocation hooks, so it is wise to avoid calling malloc/free here, even
// though jemalloc might still be able to handle reentrance correctly.  Thus, we
// bypass libnuma and do the syscalls directly here.
struct SavedNumaPolicy {
  bool needRestore{false};
  int oldPolicy{0};
  unsigned long oldMask{0};

  // Save NUMA policy for the current thread.
  void save() {
    needRestore = !get_mempolicy(&oldPolicy, &oldMask, sizeof(oldMask),
                                 nullptr, 0);
  }
  ~SavedNumaPolicy() {
    if (needRestore) {
      set_mempolicy(oldPolicy, &oldMask, sizeof(oldMask));
    }
  }
};
}
#endif

void* mmap_2m(void* addr, int prot, int node /* = -1 */,
              bool map_shared /* = false */, bool map_fixed /* = false */) {
#ifdef __linux__
  if (get_huge2m_info(node).free_hugepages <= 0) return nullptr;
  if (node >= 0 && !numa_node_allowed(node)) return nullptr;
#ifdef HAVE_NUMA
  SavedNumaPolicy numaPolicy;
  if (node >= 0 && numa_num_nodes > 1) {
    numaPolicy.save();
    unsigned long singleNodeMask = 1ul << node;
    set_mempolicy(MPOL_BIND, &singleNodeMask, sizeof(singleNodeMask));
  }
#endif
  void* ret = mmap_2m_impl(addr, prot, map_shared, map_fixed);
  s_num2MPages += !!ret;
  return ret;
#else  // not linux
  return nullptr;
#endif
}

size_t remap_interleaved_2m_pages(void* addr, size_t pages, int prot,
                                  bool shared /* = false */) {
#ifdef __linux__
  assert(reinterpret_cast<uintptr_t>(addr) % size2m == 0);
  assert(addr != nullptr);

  if (pages == 0) return 0;

#ifdef HAVE_NUMA
  SavedNumaPolicy numaPolicy;
  const int maxNode = numa_max_node();
  unsigned long singleNodeMask;
  if (maxNode > 0) {
    numaPolicy.save();
  }
#else
  constexpr int maxNode = 0;
#endif
  int node = -1;
  int failed = 0;                       // consecutive failure count
  int mapped_count = 0;
  do {
#ifdef HAVE_NUMA
    if (maxNode > 0) {
      if (++node > maxNode) node = 0;
      if (!numa_node_allowed(node)) {
        // Numa policy forbids allocation on node
        if (++failed > maxNode) break;
        continue;
      }
      singleNodeMask = 1ul << node;
      set_mempolicy(MPOL_BIND, &singleNodeMask, sizeof(singleNodeMask));
    }
#endif
    // Fail early if we don't have huge pages reserved.
    if (get_huge2m_info(node).free_hugepages > 0 &&
        mmap_2m_impl(addr, prot, shared, true /* MAP_FIXED */)) {
      addr = (char*)addr + size2m;
      ++mapped_count;
      failed = 0;
      continue;
    }
    // We failed on node, give up if we have failed on all nodes
    if (++failed > maxNode) break;
  } while (mapped_count < pages);

  return mapped_count;
#else  // not linux
  return 0;
#endif
}

void* mmap_1g(void* addr, int node, bool map_fixed) {
#ifdef __linux__
  if (s_num1GPages >= kMaxNum1GPages) return nullptr;
  if (get_huge1g_info(node).free_hugepages <= 0) return nullptr;
  if (node >= 0 && !numa_node_allowed(node)) return nullptr;
#ifdef HAVE_NUMA
  SavedNumaPolicy numaPolicy;
  if (node >= 0 && numa_num_nodes > 1) {
    numaPolicy.save();
    unsigned long singleNodeMask = 1ul << node;
    set_mempolicy(MPOL_BIND, &singleNodeMask, sizeof(singleNodeMask));
  }
#endif
  void* ret = mmap_1g_impl(addr, map_fixed);
  if (ret != nullptr) {
    s_1GPages[s_num1GPages++] = ret;
  }
  return ret;
#else
  return nullptr;
#endif
}

unsigned num_1g_pages() {
  return s_num1GPages;
}

unsigned num_2m_pages() {
  return s_num2MPages;
}

int mprotect_1g_pages(int prot) {
#ifdef __linux__
  for (unsigned i = 0; i < s_num1GPages; ++i) {
    void* p = s_1GPages[i];
    assert(p != nullptr &&
           (reinterpret_cast<uintptr_t>(p) & (size1g - 1)) == 0);
    if (auto ret = mprotect(p, size1g, prot)) {
      // mprotect() failed for this page, callers should check errno if they
      // care.
      return ret;
    }
  }
#endif
  return 0;
}

}
