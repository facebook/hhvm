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
#include <unistd.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace HPHP {

struct Huge1GMapping {
  void* addr;                           // nullptr indicates unmapped
  int node;                             // -1 indicates unknown/any node
};

constexpr size_t size1g = 1 << 30;

// Make sure these are POD, so we don't have to worry about object contruction
// order when using them in constructors.
Huge1GMapping* g_hugeMappings;
size_t g_hugeMappingsCap;
size_t g_numHugePages;
char* g_hugePath;

constexpr size_t maxErrorMsgLen = 256;
char g_errorMsg[maxErrorMsgLen];

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
      snprintf(g_errorMsg, maxErrorMsgLen,
               "path %s isn't mounted as hugetlbfs", path);
    }
  } else {
    snprintf(g_errorMsg, maxErrorMsgLen,
             "statfs64() for %s failed: %s", path, strerror(errno));
  }
#endif
  return 0;
}

bool set_hugetlbfs_path(const char* path) {
  if (get_hugepage_size(path) != size1g) return false;
  size_t len = strlen(path);
  // Allocate a few more bytes.  We will construct a file name in place there
  // later when we create a file there.
  g_hugePath = reinterpret_cast<char*>(realloc(g_hugePath, len + 8));
  memcpy(g_hugePath, path, len);
  *reinterpret_cast<int*>(g_hugePath + len) = 0;
  if (g_hugePath[len - 1] != '/') {
    g_hugePath[len] = '/';
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

static int readNumFrom(const char* fileName) {
  int result = 0;
  auto file = fopen(fileName, "r");
  if (file == nullptr) return 0;
  fscanf(file, "%d", &result);
  fclose(file);
  return result;
}

Huge1GPageInfo get_huge1g_info(int node /* = -1 */) {
  int nr_huge = 0, free_huge = 0;
#ifdef __linux__
  char fileName[256];
  if (node >= 0) {
    snprintf(fileName, sizeof(fileName),
             "/sys/devices/system/node/node%d/hugepages/"
             "hugepages-1048576kB/nr_hugepages", node);
    nr_huge = readNumFrom(fileName);
    snprintf(fileName, sizeof(fileName),
             "/sys/devices/system/node/node%d/hugepages/"
             "hugepages-1048576kB/free_hugepages", node);
    free_huge = readNumFrom(fileName);
    return Huge1GPageInfo{nr_huge, free_huge};
  }

  constexpr int MAX_NUMA_NODE = 8;
  for (int i = 0; i < MAX_NUMA_NODE; ++i) {
    auto const info = get_huge1g_info(i);
    nr_huge += info.total;
    free_huge += info.available;
  }
#endif
  return Huge1GPageInfo{nr_huge, free_huge};
}

bool auto_mount_hugetlbfs() {
#ifdef __linux__
  auto const total = get_huge1g_info();
  if (total.total <= 0) return false;   // No 1G page reserved.

  const char* hugePath = "/tmp/huge1g";
  if (mkdir(hugePath, 0777)) {
    if (errno != EEXIST) {
      snprintf(g_errorMsg, maxErrorMsgLen, "Failed to mkdir %s: %s",
               hugePath, strerror(errno));
      return false;
    }
  }
  if (mount("none", hugePath, "hugetlbfs", 0,
            "pagesize=1G,mode=0777")) {
    snprintf(g_errorMsg, maxErrorMsgLen,
             "Failed to mount hugetlbfs with 1G page size: %s",
             strerror(errno));
    return false;
  }
  return set_hugetlbfs_path(hugePath);
#else
  return false;
#endif
}

static void add_new_mapping(void* addr, int node) {
  assert(addr != nullptr);
  assert(g_numHugePages <= g_hugeMappingsCap);
  if (g_numHugePages == g_hugeMappingsCap) {
    if (g_hugeMappingsCap == 0) g_hugeMappingsCap = 4;
    else g_hugeMappingsCap *= 2;
    g_hugeMappings = static_cast<Huge1GMapping*>(
      realloc(g_hugeMappings, sizeof(Huge1GMapping) * g_hugeMappingsCap));
  }
  g_hugeMappings[g_numHugePages++] = Huge1GMapping{addr, node};
}

void* mmap_1g(void* addr /* = nullptr */, int node /* = -1 */) {
  // So far we have only trie it on Linux x64, but ARM/PPC should support
  // similar things as well.
#if defined (__linux__) && defined (__x86_64__)
  if (g_hugePath == nullptr) return nullptr;
  int fd = -1;
  size_t dirNameLen = strlen(g_hugePath);
  assert(dirNameLen > 0 && g_hugePath[dirNameLen - 1] == '/');
  for (char i = '0'; i <= '9'; ++i) {
    g_hugePath[dirNameLen] = i;
    // We don't put code on 1G huge pages, so no execute permission.
    fd = open(g_hugePath, O_CREAT | O_EXCL | O_RDWR, 0666);
    // Retry a few times if the file already exists.
    if (fd < 0) {
      if (errno == EEXIST) {
        errno = 0;
        continue;
      } else {
        snprintf(g_errorMsg, maxErrorMsgLen,
                 "Failed to create hugetlbfs-backed file %s: %s",
                 g_hugePath, strerror(errno));
        g_hugePath[dirNameLen] = 0;
        return nullptr;
      }
    } else {
      unlink(g_hugePath);
    }
    break;
  }
  g_hugePath[dirNameLen] = 0;
  if (fd < 0) {
    snprintf(g_errorMsg, maxErrorMsgLen,
             "Failed to create a hugetlbfs-backed file in %s: "
             "it seems already full of files", g_hugePath);
    return nullptr;
  }

  auto ret = mmap(addr, size1g, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE, fd, 0);
  close(fd);

  if (ret == MAP_FAILED) {
    snprintf(g_errorMsg, maxErrorMsgLen,
             "mmap() for huge page failed : %s", strerror(errno));
    return nullptr;
  }
  // Didn't get the desired address.  Note: don't do MAP_FIXED.
  if (addr != nullptr && ret != addr) {
    snprintf(g_errorMsg, maxErrorMsgLen,
             "mmap() for huge page returned %p, desired %p",
             ret, addr);
    munmap(addr, size1g);
    return nullptr;
  }

  extern void numa_bind_to(void* start, size_t size, int node);
  if (node >= 0) numa_bind_to(addr, size1g, node);

  add_new_mapping(ret, node);
  return ret;
#else
  return nullptr;
#endif
}

}
