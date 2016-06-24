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

#ifndef incl_HPHP_UTIL_HUGETLB_H_
#define incl_HPHP_UTIL_HUGETLB_H_

namespace HPHP {

// Note: these functions for managing 1GB huge pages are not designed to be
// thread-safe.  They should execute during program initialization where only
// one thread is running.

// Specify the mount point of hugetlbfs with 1G page size.  Returns whether the
// operation succeeded, i.e., the specified path is accessible, and is on a
// hugetlbfs with 1G page size.
bool set_hugetlbfs_path(const char* path);

// Try to find a mount point for the 1G hugetlbfs automatically.  Return whether
// we find one.
bool find_hugetlbfs_path();

// Try to create a temporary directory and mount a hugetlbfs with 1G page size
// there.  Return whether the operation succeeded.
bool auto_mount_hugetlbfs();

// Get a 1GB huge page from NUMA node `node`, and return the mapped address
// suggested by `addr` or nullptr on failure.  If `addr` is nullptr, the system
// will choose a proper address.  If the address range [addr, addr+1G) already
// contains address in the process address space, nullptr is returned and the
// mapping won't be changed.  If `node` is -1, any NUMA node is OK.
void* mmap_1g(void* addr = nullptr, int node = -1);

struct Huge1GPageInfo {
  int total;                            // total number of pages reserved
  int available;                        // number of pages free
};

// Get the total/available number of 1GB huge pages on a node, -1 means all
// nodes.
Huge1GPageInfo get_huge1g_info(int node = -1);

// Get error message for hugetlb mapping.
const char* get_hugetlb_err_msg();
}

#endif
