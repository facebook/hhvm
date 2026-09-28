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

#pragma once

#include <cstddef>

namespace HPHP {

constexpr size_t size1g = 1ul << 30;
constexpr size_t size512m = 1ul << 29;
constexpr size_t size2m = 1ul << 21;
constexpr size_t size4k = 1ul << 12;

static_assert((size512m & (size512m - 1)) == 0);
static_assert(size1g % size512m == 0);

// Keep size1g as the unit for existing configuration settings, but use the
// native large hugetlb page size when mapping memory. ARM64 kernels with a 64K
// base page use 512M block mappings; all other supported configurations use
// 1G mappings.
size_t huge1g_page_size();

// The legacy "1g" API names below operate on one native large hugetlb page.
// Page counts returned by these APIs are physical page counts, so each 1G of
// configured budget corresponds to two pages on a 64K-page ARM64 kernel.
//
// Note: these functions for managing large huge pages (1G on x86, 512M on
// ARM64) are not designed to be thread-safe. They should execute during
// program initialization where only one thread is running.

// Get a huge page from NUMA node `node`, and return the mapped address
// specified by `addr` or nullptr on failure.  If `addr` is nullptr, the system
// will choose a proper address. If the address range
// [addr, addr + huge1g_page_size()) already contains an address in the process
// address space, nullptr is returned and the mapping won't be changed. If
// `node` is -1, any NUMA node is OK.
void* mmap_1g(void* addr, int node, bool map_fixed);

// mmap_2m() maps a 2M hugetlb page from the specified NUMA node.  It returns
// nullptr upon failure. If node is set to -1, no NUMA policy is enforced.
void* mmap_2m(int node);

// remap_2m() is similar to mmap_2m(), except that it is used to replace an
// existing memory range [addr, addr + size2m) using hugetlb pages.  All data in
// that range will be erased.
void* remap_2m(void* addr, int node);

// When you already have the memory mapped in, remap them it to use huge pages,
// and try to interleave across all enabled numa nodes.  Return the number of
// pages that are actually backed by hugetlb pages (the rest may be implemented
// as transparent huge pages).
//
// Beware this function wipes out data on existing pages.
int remap_interleaved_2m_pages(void* addr, size_t pages);

// Information from /sys/devices/system/node/node*/hugepages/*hugepages
struct HugePageInfo {
  unsigned nr_hugepages;                // total number of pages reserved
  unsigned free_hugepages;              // number of pages free
};

// Get the total/available number of huge pages on a node, -1 means all
// nodes.
HugePageInfo get_huge1g_info(int node = -1);
HugePageInfo get_huge2m_info(int node = -1);

// Get number of huge pages that has been mapped in this process.
unsigned num_1g_pages();
unsigned num_2m_pages();

// Call mprotect() on all large huge pages. Useful in limiting access in child
// processes.
int mprotect_1g_pages(int prot);

}
