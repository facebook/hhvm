/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/DynamicUtil.h"

#include <type_traits>

#include <folly/json/dynamic.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

template <class T>
std::remove_reference_t<T>* searchDynamicInternal(
    T&& haystack,
    const folly::dynamic& needle) {
  if (needle == haystack) {
    return &haystack;
  }

  if (haystack.type() == folly::dynamic::OBJECT) {
    for (auto& val : haystack.values()) {
      if (auto res = searchDynamicInternal(val, needle)) {
        return res;
      }
    }
  } else if (haystack.type() == folly::dynamic::ARRAY) {
    for (auto& item : haystack) {
      if (auto res = searchDynamicInternal(item, needle)) {
        return res;
      }
    }
  }

  return nullptr;
}

} // anonymous namespace

folly::dynamic* searchDynamic(
    folly::dynamic& haystack,
    const folly::dynamic& needle) {
  return searchDynamicInternal(haystack, needle);
}
const folly::dynamic* searchDynamic(
    const folly::dynamic& haystack,
    const folly::dynamic& needle) {
  return searchDynamicInternal(haystack, needle);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
