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

template <typename T>
struct UnionFind {
  // Create a disjoint set containing only the given node. Return its index.
  size_t insert(const T& node) {
    auto const index = nodes.size();
    nodes.push_back({index, 1});
    auto const pair = indices.emplace(node, index);
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
    auto index1 = find(indices.at(node1));
    auto index2 = find(indices.at(node2));
    if (index1 == index2) return;

    if (nodes[index1].size < nodes[index2].size) {
      std::swap(index1, index2);
    }
    nodes[index2].parent = index1;
    nodes[index1].size += nodes[index2].size;
  }

  // Return the number of disjoint sets.
  size_t countGroups() const {
    auto result = size_t{0};
    for (auto i = 0; i < nodes.size(); i++) {
      if (nodes[i].parent == i) result++;
    }
    return result;
  }

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
  size_t find(size_t index) {
    while (nodes[index].parent != index) {
      auto const grandparent = nodes[nodes[index].parent].parent;
      index = nodes[index].parent = grandparent;
    }
    return index;
  }

  struct Node {
    size_t parent;
    size_t size;
  };
  std::vector<Node> nodes;
  folly::F14FastMap<T, size_t> indices;
};

}
