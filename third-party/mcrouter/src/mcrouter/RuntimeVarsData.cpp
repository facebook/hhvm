/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RuntimeVarsData.h"

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

RuntimeVarsData::RuntimeVarsData(folly::StringPiece json)
    : configData_(parseJsonString(json)) {}

folly::dynamic RuntimeVarsData::getVariableByName(
    folly::StringPiece name) const {
  return configData_.getDefault(name, nullptr);
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
