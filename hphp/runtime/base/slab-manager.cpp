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

#include "hphp/runtime/base/slab-manager.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/alloc.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/logger.h"
#include "hphp/util/numa.h"

#include <folly/portability/SysMman.h>
#include <vector>

namespace HPHP {

std::vector<SlabManager*> SlabManager::s_slabManagers;

void TaggedSlabList::addRange(void* ptr, std::size_t size) {
  if (!ptr) return;
  while (size >= kSlabSize) {
    push_front(ptr, 0);
    size -= kSlabSize;
    ptr = reinterpret_cast<char*>(ptr) + kSlabSize;
  }
}

void SlabManager::init() {
  if (!s_slabManagers.empty()) return;
#ifdef HAVE_NUMA
  unsigned max_node = numa_max_node();
#else
  unsigned constexpr max_node = 0;
#endif
  const unsigned numNodes = num_numa_nodes(); // number of NUMA node allowed
  s_slabManagers.reserve(max_node + 1);

  unsigned num1GPages = RuntimeOption::EvalNum1GPagesForSlabs;
  unsigned num2MPages = RuntimeOption::EvalNum2MPagesForSlabs;
  if (numNodes > 1) {
    auto perNode1G = num1GPages / numNodes;
    if (auto const excessive = num1GPages % numNodes) {
      Logger::Warning("uneven distribution of %u 1G huge pages "
                      "(Eval.Num1GPagesForSlabs) to %u NUMA nodes, "
                      "excessive %u pages not used",
                      num1GPages, numNodes, excessive);
    }
    num1GPages = perNode1G;
    auto perNode2M = num2MPages / numNodes;
    if (auto const excessive = num2MPages % numNodes) {
      Logger::Warning("uneven distribution of %u 2M huge pages "
                      "(Eval.Num1MPagesForSlabs) to %u NUMA nodes, "
                      "excessive %u pages not used",
                      num2MPages, numNodes, excessive);
    }
    num2MPages = perNode2M;
  }

  for (unsigned node = 0; node <= max_node; ++node) {
    if (!numa_node_allowed(node)) {
      s_slabManagers.push_back(nullptr);
      continue;
    }
    constexpr auto kCacheLineSize = 64u;
    static_assert(sizeof(SlabManager) <= kCacheLineSize, "");
    auto slabManager =
      new (mallocx_on_node(kCacheLineSize, node, kCacheLineSize)) SlabManager;
    s_slabManagers.push_back(slabManager);
    unsigned actual1g = 0, actual2m = 0;
    for (int i = 0; i < num1GPages; ++i) {
      auto ptr = mmap_1g(nullptr, node, /* MAP_FIXED */ false);
      if (!ptr) {
        Logger::Warning("Insufficient 1G huge pages for slabs "
                        "on NUMA node %u, desired %u, actual %u",
                        node, num1GPages, actual1g);
        break;
      }
      ++actual1g;
      slabManager->addRange(ptr, size1g);
    }
    for (unsigned i = 0; i < num2MPages; ++i) {
      auto ptr = mmap_2m(nullptr, PROT_READ | PROT_WRITE, node,
                          /* MAP_SHARED */ false, /* MAP_FIXED */ false);
      if (!ptr) {
        Logger::Warning("Insufficient 2M huge pages for slabs "
                        "on NUMA node %u, desired %u, actual %u",
                        node, num2MPages, actual2m);
        break;
      }
#ifdef __linux__
      madvise(ptr, size2m, MADV_DONTFORK);
#endif
      ++actual2m;
      slabManager->addRange(ptr, size2m);
    }
  }
}

}
