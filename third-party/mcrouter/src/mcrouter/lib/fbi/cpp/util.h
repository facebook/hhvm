/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/time.h>

#include <chrono>
#include <string>

#include <folly/Expected.h>
#include <folly/Format.h>
#include <folly/Likely.h>
#include <folly/Range.h>
#include <folly/Unit.h>
#include <folly/json/json.h>

using timeval_t = struct timeval;

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

/**
 * If `condition` is false, throws std::logic_error.
 * `fmt` and `args` are passed to folly::format().
 */
template <typename... Args>
void checkLogic(bool condition, folly::StringPiece fmt, Args&&... args) {
  if (FOLLY_UNLIKELY(!condition)) {
    throw std::logic_error(folly::sformat(fmt, std::forward<Args>(args)...));
  }
}

/**
 * Throws std::logic_error with formatted string.
 * `fmt` and `args` are passed to folly::format().
 */
template <typename... Args>
[[noreturn]] void throwLogic(folly::StringPiece fmt, Args&&... args) {
  throw std::logic_error(folly::sformat(fmt, std::forward<Args>(args)...));
}

/**
 * If `condition` is false, throws std::runtime_error.
 * `fmt` and `args` are passed to folly::format().
 */
template <typename... Args>
void checkRuntime(bool condition, folly::StringPiece fmt, Args&&... args) {
  if (FOLLY_UNLIKELY(!condition)) {
    throw std::runtime_error(folly::sformat(fmt, std::forward<Args>(args)...));
  }
}

/**
 * Throws std::runtime_error with formatted string.
 * `fmt` and `args` are passed to folly::format().
 */
template <typename... Args>
[[noreturn]] void throwRuntime(folly::StringPiece fmt, Args&&... args) {
  throw std::runtime_error(folly::sformat(fmt, std::forward<Args>(args)...));
}

/** folly::to style conversion routines */

template <typename T, typename F>
T to(const F& x);

/** milliseconds to timeval_t */
template <>
inline timeval_t to<timeval_t>(const unsigned int& ms) {
  timeval_t r;
  r.tv_sec = ms / 1000;
  r.tv_usec = ms % 1000 * 1000;
  return r;
}

/** milliseconds to timeval_t */
template <>
inline timeval_t to<timeval_t>(const std::chrono::milliseconds& ms) {
  timeval_t r;
  r.tv_sec = ms.count() / 1000;
  r.tv_usec = ms.count() % 1000 * 1000;
  return r;
}

/** timeval_t to milliseconds */
template <>
inline std::chrono::milliseconds to<std::chrono::milliseconds>(
    const timeval_t& t) {
  using namespace std::chrono;
  return duration_cast<milliseconds>(
      seconds(t.tv_sec) + microseconds(t.tv_usec));
}

/**
 * True iff a and b point to the same region in memory
 */
inline bool sameMemoryRegion(folly::StringPiece a, folly::StringPiece b) {
  return (a.empty() && b.empty()) ||
      (a.size() == b.size() && a.begin() == b.begin());
}

/**
 * Returns value from map or default, if there is no key in map.
 */
template <class Map>
inline typename Map::mapped_type tryGet(
    const Map& map,
    const typename Map::key_type& key,
    const typename Map::mapped_type def = typename Map::mapped_type()) {
  auto it = map.find(key);
  return it == map.end() ? def : it->second;
}

/**
 * Returns string with length in [minLen, maxLen] and random characters
 * from range.
 */
std::string randomString(
    size_t minLen = 1,
    size_t maxLen = 20,
    folly::StringPiece range =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

/**
 * Returns hash value for a given key.
 * To use for probabilistic sampling, e.g. for stats.
 */
uint32_t getMemcacheKeyHashValue(folly::StringPiece key);

/**
 * Checks if the given hash is within a range.
 * The range is from 0 to (MAX(uint32_t)/sample_rate)
 * Used for probabilistic decisions, like stats sampling.
 */
bool determineIfSampleKeyForViolet(
    uint32_t routingKeyHash,
    uint32_t sample_period);

/**
 * @return MD5 hash of a string
 */
std::string Md5Hash(folly::StringPiece input);

/**
 * @param Cmp  comparator used to compare characters
 *
 * @return  true if two strings are equal
 */
template <class Cmp>
bool equalStr(folly::StringPiece A, folly::StringPiece B, Cmp&& cmp) {
  if (A.size() != B.size()) {
    return false;
  }
  return std::equal(A.begin(), A.end(), B.begin(), std::forward<Cmp>(cmp));
}

/**
 * Writes 'contents' to the file, overwriting any existing contents
 * (will create if file doesn't exist)
 *
 * @return true on success, false otherwise
 */
bool writeStringToFile(folly::StringPiece contents, const std::string& path);

/**
 * Append 'contents' to the file (will create if file doesn't exist)
 *
 * @return true on success, false otherwise
 */
bool appendStringToFile(folly::StringPiece contents, const std::string& path);

/**
 * Write the given 'contents' to 'absFilename' atomically. This first writes
 * the contents to a temp file to in the absFilename's parent directory
 * and then calls 'rename()', which is atomic.
 *
 * @return true on success, false otherwise
 */
bool atomicallyWriteFileToDisk(
    folly::StringPiece contents,
    const std::string& absFilename);

/**
 * Analogue of UNIX touch: changes file access and modification time, if file
 * doesn't exist creates it.
 *
 * @return true on success, false otherwise
 */
bool touchFile(const std::string& path);

/**
 * Make uint64 random number out of uint32. Especially useful for mt19937.
 */
template <class RNG>
typename std::enable_if<RNG::word_size == 32, uint64_t>::type randomInt64(
    RNG& rng) {
  return ((uint64_t)rng() << 32) | (uint64_t)rng();
}

/**
 * Specialization for random generator with uint64 result type.
 */
template <class RNG>
typename std::enable_if<RNG::word_size == 64, uint64_t>::type randomInt64(
    RNG& rng) {
  return rng();
}

/**
 * @return name of current thread
 */
std::string getThreadName();

/**
 * Parse json string
 *
 * @param s - pointer to json formatted string
 * @param metadatamap - if non-null, will be populated with parse metadata
 * @param allow_trailing_comma - enabled to allow trailing comma by default
 *
 * @return dynamic struct containing the parsed json
 */
folly::dynamic parseJsonString(
    folly::StringPiece s,
    folly::json::metadata_map* metadataMap = nullptr,
    bool allow_trailing_comma = true);

/**
 * @return returns a prefix of `s` with '...' appended if s is longer than
 *         `maxLength`.
 */
std::string shorten(folly::StringPiece s, size_t maxLength);

/**
 * @return `s` where all occurences of `from` are replaced with `to`
 */
std::string
replaceAll(std::string s, const std::string& from, const std::string& to);

/**
 * Same as folly::toPrettyJson but also sorts keys in dictionaries
 */
std::string toPrettySortedJson(const folly::dynamic& json);

/**
 * Makes sure a directory exists and is writable (e.g. create if not found, etc)
 */
bool ensureDirExistsAndWritable(const std::string& path);

/**
 * Makes sure a directory exists and is writable.
 * Return more detailed information about errors to aid debugging.
 */
folly::Expected<folly::Unit, std::runtime_error>
ensureDirExistsAndWritableOrReturnError(const std::string& path);

/**
 * Makes sure that a file or directory has the desired permissions.
 */
bool ensureHasPermission(const std::string& path, mode_t mode);

/**
 * Makes sure that a file or directory has the desired permissions.
 * Return more detailed information about errors to aid debugging.
 */
folly::Expected<folly::Unit, std::runtime_error>
ensureHasPermissionOrReturnError(const std::string& path, mode_t mode);

/**
 * Determines if any two intervals are overlapping.
 */
bool intervalOverlap(std::vector<std::vector<size_t>>& intervals);
} // namespace memcache
} // namespace facebook
