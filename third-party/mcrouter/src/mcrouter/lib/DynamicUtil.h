/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/json/dynamic.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * "Deep" search in the given dynamic. This will search all nodes the down to
 * the leaves, doing a deep comparisson of each node. It can be potentially
 * expensive.
 * Note: When searching for strings, object keys are not considered as they
 * are not technically dynamics.
 *
 * @param haystack  The dynamic that might contain the piece we are looking for.
 * @param needle    The piece that should be searched for.
 *
 * @return  Pointer to the first piece of this dynamic that matches "needle",
 *          or nullptr if not found.
 */
folly::dynamic* searchDynamic(
    folly::dynamic& haystack,
    const folly::dynamic& needle);
const folly::dynamic* searchDynamic(
    const folly::dynamic& haystack,
    const folly::dynamic& needle);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
