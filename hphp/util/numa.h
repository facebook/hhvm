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
#ifndef incl_HPHP_UTIL_NUMA_H
#define incl_HPHP_UTIL_NUMA_H

#include <atomic>

#include <cstdlib>
#ifdef HAVE_NUMA
#include <vector>
#include <numa.h>
#endif

namespace HPHP {
// numa_num_nodes and numa_node_set are always defined. They are initialized to
// 1 (which is the value when libnuma isn't used).
extern uint32_t numa_num_nodes;
extern uint32_t numa_node_set;

#ifdef HAVE_NUMA

extern uint32_t numa_node_mask;
extern std::vector<bitmask*> node_to_cpu_mask;
extern bool use_numa;

/*
 * init_numa() is called very early during process initialization, before
 * parsing runtime options. It initializes `numa_num_nodes`, `numa_node_set`,
 * and `numa_node_mask`, when NUMA APIs are usable.
 */
void init_numa();

/*
 * enable_numa() is called after parsing runtime options. It initializes
 * `use_numa`, which is used to decide whether we actually call NUMA APIs. Note
 * that on single-node systems, we don't set use_numa.
 */
void enable_numa();

/*
 * Determine the next NUMA node according to state maintained in `curr_node`.
 */
uint32_t next_numa_node(std::atomic<uint32_t>& curr_node);
/*
 * The number of numa nodes in the system
 */
inline uint32_t num_numa_nodes() {
  return use_numa ? numa_num_nodes : 1;
}
/*
 * Enable numa interleaving for the specified address range
 */
void numa_interleave(void* start, size_t size);
/*
 * Allocate the specified address range on the given node
 */
void numa_bind_to(void* start, size_t size, uint32_t node);
/*
 * Return true if node is part of the allowed set of numa nodes
 */
inline bool numa_node_allowed(uint32_t node) {
  if (numa_node_set == 0) return true;
  return numa_node_set & (1u << node);
}

#else // HAVE_NUMA undefined

inline void init_numa() {}
inline void enable_numa() {}
inline constexpr uint32_t next_numa_node(std::atomic<uint32_t>& curr_node) {
  return 0;
}
inline constexpr uint32_t num_numa_nodes() { return 1; }
inline void numa_interleave(void* start, size_t size) {}
inline void numa_bind_to(void* start, size_t size, uint32_t node) {}
inline constexpr bool numa_node_allowed(int node) { return true; }

#endif

} // namespace HPHP
#endif
