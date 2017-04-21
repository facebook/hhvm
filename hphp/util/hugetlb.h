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

#ifndef incl_HPHP_UTIL_HUGETLB_H_
#define incl_HPHP_UTIL_HUGETLB_H_

#include <cstddef>

namespace HPHP {

constexpr size_t size1g = 1u << 30;
constexpr size_t size2m = 1u << 21;

// Note: these functions for managing 1GB huge pages are not designed to be
// thread-safe.  They should execute during program initialization where only
// one thread is running.

// Specify the mount point of hugetlbfs with 1G page size.  Returns whether the
// operation succeeded, i.e., the specified path is accessible, and is on a
// hugetlbfs with 1G page size.
bool set_hugetlbfs_path(const char* path);

// Try to find a mount point for the 1G hugetlbfs automatically.  Return whether
// we found one.
bool find_hugetlbfs_path();

// Try to create a temporary directory and mount a hugetlbfs with 1G page size
// there.  Return whether the operation succeeded.
bool auto_mount_hugetlbfs();

// Get a huge page from NUMA node `node`, and return the mapped address
// suggested by `addr` or nullptr on failure.  If `addr` is nullptr, the system
// will choose a proper address.  If the address range [addr, addr+1G) already
// contains address in the process address space, nullptr is returned and the
// mapping won't be changed.  If `node` is -1, any NUMA node is OK.
void* mmap_1g(void* addr = nullptr, int node = -1);

// For 2M pages, we want more control over protection and mapping flags.  Note
// that MAP_FIXED can overwrite the existing mapping without checking/failing.
void* mmap_2m(void* addr, int prot, int node = -1,
              bool map_shared = false, bool map_fixed = false);

// When you already have the memory mapped in, remap them it to use huge pages,
// and try to interleave across all enabled numa nodes (no guarantee).  Return
// the number of pages that are actually backed by huge pages.
//
// Beware this function wipes out data on existing pages, and yep, that is what
// it is designed to do.
size_t remap_interleaved_2m_pages(void* addr, size_t pages, int prot,
                                  bool map_shared = false);

// Information from /sys/devices/system/node/node*/hugepages/*hugepages
struct HugePageInfo {
  int nr_hugepages;                     // total number of pages reserved
  int free_hugepages;                   // number of pages free
};

// Get the total/available number of huge pages on a node, -1 means all
// nodes.
HugePageInfo get_huge1g_info(int node = -1);
HugePageInfo get_huge2m_info(int node = -1);

// Get error message for hugetlb mapping.
const char* get_hugetlb_err_msg();

// Get number of huge pages that has been mapped in this process.
unsigned num_1g_pages();
unsigned num_2m_pages();

// Call mprotect() on all 1G huge pages.  Useful in limiting access in child
// processes.
int mprotect_1g_pages(int prot);

}

#endif
