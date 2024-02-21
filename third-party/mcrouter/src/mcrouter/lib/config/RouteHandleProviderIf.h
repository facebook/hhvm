/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <folly/Range.h>
#include <folly/json/dynamic.h>

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

/**
 * Interface for RouteHandleProvider to easily mock it. Implementation
 * should create corresponding RouteHandle by given type and JSON.
 */
template <class RouteHandleIf>
class RouteHandleProviderIf {
 public:
  /**
   * Creates list of RouteHandles by given type and JSON representation. Should
   * also validate passed object and throw exception in case JSON is incorrect.
   *
   * @param factory   RouteHandleFactory to create children routes.
   * @param type      Which RouteHandle is represented by json.
   * @param json      JSON object with RouteHandle representation.
   */
  virtual std::vector<std::shared_ptr<RouteHandleIf>> create(
      RouteHandleFactory<RouteHandleIf>& factory,
      folly::StringPiece type,
      const folly::dynamic& json) = 0;

  /**
   * Loads a pool from ConfigApi, expand `inherit`, etc.
   *
   * @param json  Json with the pool information.
   *
   * @return      The folly::dynamic object with pool name and final json blob.
   */
  virtual const folly::dynamic& parsePool(const folly::dynamic& json) = 0;

  virtual ~RouteHandleProviderIf() {}
};

} // namespace memcache
} // namespace facebook
