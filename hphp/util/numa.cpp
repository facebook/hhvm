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
#include "hphp/util/numa.h"

#ifdef HAVE_NUMA
#include "hphp/util/portability.h"
#include <folly/Bits.h>
#include <numaif.h>
#include <fstream>
#include <map>
#include <thread>
#include <vector>

extern "C" {
HHVM_ATTRIBUTE_WEAK extern void numa_init();
}
#endif

namespace HPHP {

// Treat the system as having a single NUMA node if HAVE_NUMA not defined.
// Otherwise, initNuma() will calculate the number of allowed NUMA nodes.
uint32_t numa_num_nodes = 1;
uint32_t numa_node_set = 1;

bool use_nuca = false;

#ifdef HAVE_NUMA

uint32_t numa_node_mask;
bool use_numa = false;
std::vector<bitmask*> node_to_cpu_mask;
// For NUCA, we pin threads to pairs of nodes with consideration of locality and
// balancing. When the two nodes are equal, the thread is pinned to a single node.
std::vector<std::pair<uint8_t, uint8_t>> nuca_node_seq;

void init_numa() {
  if (getenv("HHVM_DISABLE_NUMA")) return;

  // When linked dynamically numa_init() is called before JEMallocInitializer()
  // numa_init is not exported by libnuma.so so it will be NULL
  // however when linked statically numa_init() is not guaranteed to be called
  // before JEMallocInitializer(), so call it here.
  if (&numa_init) {
    numa_init();
  }
  if (numa_available() < 0) return;

  // set interleave for early code. we'll then force interleave for a few
  // regions, and switch to local for the threads
  numa_set_interleave_mask(numa_all_nodes_ptr);

  int max_node = numa_max_node();
  if (max_node < 1 || max_node >= 32) return;

  bool ret = true;
  bitmask* run_nodes = numa_get_run_node_mask();
  bitmask* mem_nodes = numa_get_mems_allowed();
  numa_num_nodes = 0;
  numa_node_set = 0;
  for (int i = 0; i <= max_node; i++) {
    if (!numa_bitmask_isbitset(run_nodes, i) ||
        !numa_bitmask_isbitset(mem_nodes, i)) {
      // Only deal with the case of a contiguous set of nodes where we can
      // run/allocate memory on each node.
      ret = false;
      break;
    }
    numa_node_set |= 1u << i;
    numa_num_nodes++;
  }
  numa_bitmask_free(run_nodes);
  numa_bitmask_free(mem_nodes);

  if (!ret || numa_num_nodes <= 1) return;

  numa_node_mask = folly::nextPowTwo(uint32_t(max_node) + 1) - 1;
}

static int getL3CacheId(int cpuId) {
  int id = 0;
  std::ifstream file("/sys/devices/system/cpu/cpu" + std::to_string(cpuId) +
                     "/cache/index3/id");
  if (!file.is_open()) return 0;
  file >> id;
  return id;
}

void enable_numa() {
  /*
   * Check if NUMA shouldn't be used, for reasons including: (1) only one NUMA
   * node allowed; (2) NUMA disabled via environmental variable; (3) unsupported
   * cases, e.g., allowed nodes non-contiguous, more than 32 nodes; (4) errors
   * calling NUMA API in init_numa().
   */
  if (numa_num_nodes <= 1 || numa_node_mask == 0) {
    // Check for NUCA
    int cpus = numa_num_configured_cpus();
    std::map<int, bitmask*> l3toCpus;
    for (unsigned i = 0; i < cpus; ++i) {
      if (numa_bitmask_isbitset(numa_all_cpus_ptr, i)) {
        auto const l3 = getL3CacheId(i);
        if (!l3toCpus.contains(l3)) {
          l3toCpus[l3] = numa_allocate_cpumask();
        }
        numa_bitmask_setbit(l3toCpus[l3], i);
      }
    }
    if (l3toCpus.size() > 1) {
      use_nuca = true;
      node_to_cpu_mask.clear();
      std::vector<double> inc;
      std::vector<double> usage;
      for (auto v : l3toCpus) {
        node_to_cpu_mask.push_back(v.second);
        // Note that different nodes can have different number of CPU cores.
        // Keep track of the increment in node utilization with one more thread,
        // to perform balancing of utilizations across nodes.
        auto const increment = 1.0 / numa_bitmask_weight(v.second);
        inc.push_back(increment);
        usage.push_back(increment);
      }
      for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        auto iter = std::min_element(usage.begin(), usage.end());
        auto const node = iter - usage.begin();
        *iter += inc[node];
        nuca_node_seq.push_back({node, node});
      }
      // Some threads are pinned to pairs of nodes to allow some flexibility in
      // scheduling.
      std::vector<double> pair_inc;
      std::vector<double> pair_usage;
      std::vector<std::pair<uint8_t, uint8_t>> pairs;
      for (uint8_t s = 1; s < l3toCpus.size(); ++s) {
        for (uint8_t i = 0; i < l3toCpus.size(); ++i) {
          auto const j = (i + s) % l3toCpus.size();
          pairs.push_back({i, j});
          auto const increment = inc[i] * inc[j];
          pair_inc.push_back(increment);
          pair_usage.push_back(increment);
        }
      }
      for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        auto iter = std::min_element(pair_usage.begin(), pair_usage.end());
        auto const nodes_index = iter - pair_usage.begin();
        *iter += pair_inc[nodes_index];
        nuca_node_seq.push_back(pairs[nodes_index]);
      }
    }
    return;
  }
  /*
   * libnuma is only partially aware of taskset. If on entry, you have
   * completely disabled a node via taskset, the node will not be available, and
   * calling numa_run_on_node will not work for that node. But if only some of
   * the cpu's on a node were disabled, then calling numa_run_on_node will
   * enable them all. To prevent this, compute the actual masks up front.
   */
  bitmask* enabled = numa_allocate_cpumask();
  if (numa_sched_getaffinity(0, enabled) < 0) {
    return;
  }

  int num_cpus = numa_num_configured_cpus();
  int max_node = numa_max_node();
  assert(max_node >= 1 && max_node < 32);
  for (int i = 0; i <= max_node; i++) {
    bitmask* cpus_for_node = numa_allocate_cpumask();
    numa_node_to_cpus(i, cpus_for_node);
    for (int j = 0; j < num_cpus; j++) {
      if (!numa_bitmask_isbitset(enabled, j)) {
        numa_bitmask_clearbit(cpus_for_node, j);
      }
    }
    assert(node_to_cpu_mask.size() == i);
    node_to_cpu_mask.push_back(cpus_for_node);
  }
  numa_bitmask_free(enabled);

  use_numa = true;
}

/*
 * Bind the current thread to a pair of nodes.
 */
inline void sched_on_nodes(unsigned n1, unsigned n2) {
  if (n1 == n2) {
    numa_sched_setaffinity(0, node_to_cpu_mask[n1]);
    return;
  }
  auto mask = numa_allocate_cpumask();
  auto const mask1 = node_to_cpu_mask[n1];
  auto const mask2 = node_to_cpu_mask[n2];
  for (unsigned i = 0; i < numa_num_configured_cpus(); ++i) {
    if (numa_bitmask_isbitset(mask1, i) || numa_bitmask_isbitset(mask2, i)) {
      numa_bitmask_setbit(mask, i);
    }
  }
  numa_sched_setaffinity(0, mask);
  numa_bitmask_free(mask);
}

void nuca_bind_thread() {
  static std::atomic_uint32_t s_total = 0;
  auto const index = s_total.fetch_add(1, std::memory_order_acq_rel);
  if (index >= nuca_node_seq.size()) return;
  auto const nodes = nuca_node_seq[index];
  sched_on_nodes(nodes.first, nodes.second);
}

uint32_t next_numa_node(std::atomic<uint32_t>& curr_node) {
  if (!use_numa) return 0;
  uint32_t node;
  do {
    node = curr_node.fetch_add(1u, std::memory_order_acq_rel);
    node &= numa_node_mask;
  } while (!((numa_node_set >> node) & 1));
  return node;
}

void numa_interleave(void* start, size_t size) {
  if (!use_numa) return;
  numa_interleave_memory(start, size, numa_all_nodes_ptr);
}

void numa_bind_to(void* start, size_t size, uint32_t node) {
  if (!use_numa) return;
  numa_tonode_memory(start, size, (int)node);
}

void SavedNumaPolicy::save() {
  needRestore = !get_mempolicy(&oldPolicy, &oldMask, sizeof(oldMask),
                               nullptr, 0);
}

SavedNumaPolicy::~SavedNumaPolicy() {
  if (needRestore) {
    set_mempolicy(oldPolicy, &oldMask, sizeof(oldMask));
  }
}

#endif
}
