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

#include "hphp/util/kernel-version.h"

#ifdef HAVE_NUMA
#include <numa.h>
#endif
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace HPHP {

constexpr size_t size1g = 1 << 30;

static char s_hugePath[256];
constexpr size_t maxErrorMsgLen = 256;
static char s_errorMsg[maxErrorMsgLen];

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
#ifdef HAVE_NUMA
  const int MAX_NUMA_NODE = numa_max_node();
#else
  const int MAX_NUMA_NODE = 0;
#endif
  for (int i = 0; i <= MAX_NUMA_NODE; ++i) {
    auto const info = get_huge1g_info(i);
    nr_huge += info.total;
    free_huge += info.available;
  }
#endif
  return Huge1GPageInfo{nr_huge, free_huge};
}

bool auto_mount_hugetlbfs() {
#ifdef __linux__
  auto const info = get_huge1g_info();
  if (info.total <= 0) return false;   // No 1G page reserved.

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

inline void* mmap_1g_impl(void* addr) {
#ifdef __linux__
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
               MAP_PRIVATE, fd, 0);
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
      int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_1GB;
      ret = mmap(addr, size1g, PROT_READ | PROT_WRITE, flags, 0, 0);
      if (ret == MAP_FAILED) {
        record_err_msg("mmap() with MAP_HUGE_1GB failed: ");
        return nullptr;
      }
    } else {
      return nullptr;
    }
  }

  // Didn't get the desired address.  Note: don't do MAP_FIXED.
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
#else
  return nullptr;
#endif
}

void* mmap_1g(void* addr /* = nullptr */, int node /* = -1 */) {
#ifdef __linux__
#ifdef HAVE_NUMA
  bitmask* memMask = nullptr;
  bitmask* interleaveMask = nullptr;
  if (node >= 0) {
    memMask = numa_get_membind();
    interleaveMask = numa_get_interleave_mask();
    bitmask* mask = numa_allocate_nodemask();
    numa_bitmask_setbit(mask, node);
    numa_set_membind(mask);
    numa_bitmask_free(mask);
  }
#endif
  void* ret = mmap_1g_impl(addr);
#ifdef HAVE_NUMA
  if (node >= 0) {
    numa_set_membind(memMask);
    numa_set_interleave_mask(interleaveMask);
    numa_bitmask_free(memMask);
    numa_bitmask_free(interleaveMask);
  }
#endif
  return ret;
#else
  return nullptr;
#endif
}

}
