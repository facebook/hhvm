/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

#include <folly/Range.h>

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

/**
 * Converts `json` to an integer and checks it's in [min,max] range.
 * @return value stored in `json`
 *
 * @throws std::logic_error if `json` is not an integer or out of range.
 */
int64_t parseInt(
    const folly::dynamic& json,
    folly::StringPiece name,
    int64_t min,
    int64_t max);

/**
 * Converts `json` to boolean value.
 * @return value stored in `json`
 *
 * @throws std::logic_error if `json` is not a boolean value.
 */
bool parseBool(const folly::dynamic& json, folly::StringPiece name);

/**
 * Converts `json` to string.
 * @return value stored in `json`
 *
 * @throws std::logic_error if `json` is not a string.
 */
folly::StringPiece parseString(
    const folly::dynamic& json,
    folly::StringPiece name);

/**
 * Parses `json` to an integer representing timeout.
 * @return value stored in `json`
 *
 * @throws std::logic_error if `json` is not an integer or out of range.
 */
std::chrono::milliseconds parseTimeout(
    const folly::dynamic& json,
    folly::StringPiece name);
} // namespace memcache
} // namespace facebook
