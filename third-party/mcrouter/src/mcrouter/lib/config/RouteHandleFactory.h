/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <stack>
#include <vector>

#include <folly/Range.h>
#include <folly/container/F14Map.h>
#include <folly/dynamic.h>

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleProviderIf;

/**
 * Parses RouteHandle tree from JSON object.
 */
template <class RouteHandleIf>
class RouteHandleFactory {
 public:
  using RouteHandlePtr = std::shared_ptr<RouteHandleIf>;

  RouteHandleFactory(const RouteHandleFactory&) = delete;
  RouteHandleFactory& operator=(const RouteHandleFactory&) = delete;

  /**
   * @param provider  creates single node of RouteHandle tree
   * @param threadId  thread where route handles will run
   */
  RouteHandleFactory(
      RouteHandleProviderIf<RouteHandleIf>& provider,
      size_t threadId);

  /**
   * Adds a named route handle that may be used later.
   *
   * NOTE: The caller must ensure that the provided JSON pointer outlives
   * this factory, because we hold onto a reference to it for our lifetime.
   *
   * @param json object that contains RouteHandle with (optional) children.
   */
  void addNamed(const folly::StringPiece name, const folly::dynamic& json);

  /**
   * Creates single RouteHandle from JSON object.
   *
   * @param json object that contains RouteHandle with (optional) children.
   */
  RouteHandlePtr create(const folly::dynamic& json);

  /**
   * Creates multiple subtrees from JSON object. Should be used to create
   * children of some RouteHandle.
   *
   * @param json array, object or string that represents zero, one or multiple
   *             RouteHandles.
   */
  std::vector<RouteHandlePtr> createList(const folly::dynamic& json);

  /**
   * Loads a pool from ConfigApi, expand `inherit`, etc.
   *
   * @param json  Json with the pool information.
   *
   * @return      The folly::dynamic object with pool name and final json blob.
   */
  const folly::dynamic& parsePool(const folly::dynamic& json);

  size_t getThreadId() const noexcept {
    return threadId_;
  }

  /**
   * Pushes a list of route_handles that will be used the next time this class
   * sees $children_list$ in the config. The list of route handles are kept in
   * a stack - so the last list pushed will be the first one to be used.
   *
   * NOTE: Callers must explicitly call popChildrenList() when the children
   *       list is no longer necessary (e.g. when we are done processing the
   *       piece of config that might use the children_list).
   *
   * @param children  The list of route_handles to be used to replace
   *                  $children_list$ in the config.
   */
  void pushChildrenList(std::vector<RouteHandlePtr> children);

  /**
   * Pops the last children_list object pushed using the pushChildrenList()
   * method.
   */
  void popChildrenList();

 private:
  RouteHandleProviderIf<RouteHandleIf>& provider_;

  /// Registered named routes that are not parsed yet
  folly::F14NodeMap<std::string, const folly::dynamic*> registered_;
  /// Named routes we've already parsed
  folly::F14NodeMap<std::string, std::vector<RouteHandlePtr>> seen_;
  /// Thread where route handles created by this factory will be used
  size_t threadId_;

  // list of servers that should be used to replace "$children_list$" in config.
  std::stack<std::vector<RouteHandlePtr>> childrenLists_;

  const std::vector<RouteHandlePtr>& createNamed(
      folly::StringPiece name,
      const folly::dynamic& json);

  // Unlike createNamed, does not check for this name in the registered_ map,
  // or in the seen_ values de-duplication cache.
  const std::vector<RouteHandlePtr>& createNamedImpl(
      folly::StringPiece name,
      const folly::dynamic& json);
};

} // namespace memcache
} // namespace facebook

#include "RouteHandleFactory-inl.h"
