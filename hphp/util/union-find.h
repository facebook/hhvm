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

#include <vector>
#include <folly/container/F14Map.h>

#include "hphp/util/assertions.h"
#include "hphp/util/optional.h"

namespace HPHP {

template<typename SizeT>
struct UnionFindSequential {
  // Create n disjoint sets with a single node each.  Removes all previous data.
  void presize(SizeT n) {
    nodes.clear();
    nodes.reserve(n);
    for (SizeT i = 0; i < n; ++i) {
      append();
    }
  }

  // Create a new disjoint set with a single node and return its index.
  SizeT append() {
    auto const index = nodes.size();
    nodes.push_back({static_cast<SizeT>(index), 1});
    return index;
  }

  // Merge the disjoint sets for the two indexes. The indexes must have been
  // appended into the data structure already.  Returns the representative
  // index for the merged set.
  SizeT merge(const SizeT n1, const SizeT n2) {
    return merge(n1, n2, [] (const SizeT, const SizeT) {});
  }

  // This implementation of merge calls a lambda when two disjoint sets are
  // combined.  mergeLambda is called with the first parameter being the new
  // representative set the second parameter is being merged into.
  template<typename Lambda>
  SizeT merge(const SizeT n1, const SizeT n2, Lambda mergeLambda) {
    auto index1 = find(n1);
    auto index2 = find(n2);

    if (index1 == index2) return index1;

    if (nodes[index1].size < nodes[index2].size) {
      std::swap(index1, index2);
    }
    nodes[index2].parent = index1;
    nodes[index1].size += nodes[index2].size;

    mergeLambda(index1, index2);
    return index1;
  }

  // Return the number of disjoint sets.
  SizeT countGroups() const {
    auto result = SizeT{0};
    for (SizeT i = 0; i < nodes.size(); i++) {
      if (nodes[i].parent == i) result++;
    }
    return result;
  }

  // Execute `f` for each disjoint set. `f` takes std::vector<SizeT>&.
  template <typename F>
  void forEachGroup(F&& f) {
    DEBUG_ONLY auto const count = countGroups();
    folly::F14FastMap<SizeT, std::vector<SizeT>> groups;
    for (SizeT i = 0; i < nodes.size(); ++i) {
      groups[find(i)].emplace_back(i);
    }
    assertx(count == countGroups());
    assertx(count == groups.size());
    for (auto& pair : groups) {
      f(pair.second);
    }
  }

  SizeT find(SizeT index) {
    while (nodes[index].parent != index) {
      auto const grandparent = nodes[nodes[index].parent].parent;
      index = nodes[index].parent = grandparent;
    }
    return index;
  }

protected:
  struct Node {
    SizeT parent;
    SizeT size;
  };
  std::vector<Node> nodes;
};

template <typename T>
struct UnionFind : private UnionFindSequential<size_t> {
  using Base = UnionFindSequential<size_t>;

  // Create a disjoint set containing only the given node. Return its index.
  size_t insert(const T& node) {
    auto const pair = indices.emplace(node, append());
    assertx(pair.second);
    return pair.first->second;
  }

  // Return the index of the given node, assigned sequentially at insertion.
  Optional<size_t> lookup(const T& node) const {
    auto const it = indices.find(node);
    return it == indices.end() ? std::nullopt : make_optional(it->second);
  }

  // Merge the disjoint sets containing the two nodes. The nodes must have
  // been inserted into the data structure already.
  void merge(const T& node1, const T& node2) {
    assertx(indices.contains(node1));
    assertx(indices.contains(node2));
    if (node1 == node2) return;
    Base::merge(indices.at(node1), indices.at(node2));
  }

  using Base::countGroups;

  // Execute `f` for each disjoint set. `f` takes std::vector<T>&.
  template <typename F>
  void forEachGroup(F&& f) {
    DEBUG_ONLY auto const count = countGroups();
    folly::F14FastMap<size_t, std::vector<T>> groups;
    for (auto const& pair : indices) {
      groups[find(pair.second)].emplace_back(pair.first);
    }
    assertx(count == countGroups());
    assertx(count == groups.size());
    for (auto& pair : groups) {
      f(pair.second);
    }
  }

private:
  folly::F14FastMap<T, size_t> indices;
};

}
