/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string_view>

#include <folly/FBString.h>
#include <folly/Range.h>
#include <folly/lang/Exception.h>
#include <thrift/common/universal_name.h>
#include <thrift/lib/cpp2/type/UniversalHashAlgorithm.h>

namespace apache::thrift::type {

using hash_size_t = int8_t;

inline constexpr hash_size_t kDisableUniversalHash = 0;
inline constexpr hash_size_t kDefaultTypeHashBytes = 16;

// Validates that uri is a valid universal name of the form:
// {domain}/{path}. For example: facebook.com/thrift/Value.
//
// The scheme "fbthrift://"" is implied and not included in the uri.
//
// Throws std::invalid_argument on failure.
inline void validateUniversalName(std::string_view uri) {
  return validate_universal_name(uri);
}

// Validates that the given type hash meets the size requirements.
//
// Throws std::invalid_argument on failure.
void validateUniversalHash(
    UniversalHashAlgorithm alg,
    folly::StringPiece universalHash,
    hash_size_t minHashBytes);

// Validates that the given type hash bytes size meets size requirements.
//
// Throws std::invalid_argument on failure.
void validateUniversalHashBytes(
    hash_size_t hashBytes, hash_size_t minHashBytes);

// The number of bytes returned by the given type hash algorithm.
hash_size_t getUniversalHashSize(UniversalHashAlgorithm alg);

// Returns the hash for the given universal name uri.
//
// The hash includes the implied scheme, "fbthrift://".
folly::fbstring getUniversalHash(
    UniversalHashAlgorithm alg, std::string_view uri);

// Shrinks the universalHash to fit in the given number of bytes.
folly::StringPiece getUniversalHashPrefix(
    folly::StringPiece universalHash, hash_size_t hashBytes);

// Returns the type hash prefix iff smaller than the uri.
folly::fbstring maybeGetUniversalHashPrefix(
    UniversalHashAlgorithm alg, std::string_view uri, hash_size_t hashBytes);

// Returns true iff prefix was derived from universalHash.
bool matchesUniversalHash(
    folly::StringPiece universalHash, folly::StringPiece prefix);

// Returns true, if the given sorted map contains an entry that matches the
// given type hash prefix.
template <typename C, typename K>
bool containsUniversalHash(C& sortedMap, const K& universalHashPrefix) {
  auto itr = sortedMap.lower_bound(universalHashPrefix);
  return itr != sortedMap.end() &&
      matchesUniversalHash(itr->first, universalHashPrefix);
}

// Finds a matching hash within the given sorted map.
//
// Raises a std::runtime_error if the result is ambiguous.
template <typename C, typename K>
auto findByUniversalHash(C& sortedMap, const K& universalHashPrefix) {
  auto itr = sortedMap.lower_bound(universalHashPrefix);
  if (itr == sortedMap.end() ||
      !matchesUniversalHash(itr->first, universalHashPrefix)) {
    return sortedMap.end();
  }
  auto next = itr;
  if (++next != sortedMap.end() &&
      matchesUniversalHash(next->first, universalHashPrefix)) {
    folly::throw_exception<std::runtime_error>("type hash look up ambiguous");
  }
  return itr;
}

} // namespace apache::thrift::type
