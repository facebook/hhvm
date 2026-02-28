/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/json/dynamic.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

class RuntimeVarsData {
 public:
  RuntimeVarsData() = default;
  explicit RuntimeVarsData(folly::StringPiece json);

  /**
   * Returns the value of the variable with key = name.
   *
   * @param name key of the data to be retrieved
   * @return Variable value, or null if key not found
   */
  folly::dynamic getVariableByName(folly::StringPiece name) const;

 private:
  folly::dynamic configData_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
