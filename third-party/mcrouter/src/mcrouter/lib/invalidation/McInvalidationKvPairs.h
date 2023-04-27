/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include "folly/container/F14Map.h"
#include "folly/io/IOBuf.h"

namespace facebook::memcache::invalidation {

constexpr uint32_t getMcInvalidationVersion() {
  // the version gets updated whenever we change the
  // invalidation format, i.e. add/modify/remove key-values.
  // When the version is bumped it is essential to make sure that
  // the write/read logic supports both previous and current versions.
  return 1;
}

constexpr std::string_view kSerialized("serialized");
constexpr std::string_view kRegion("region");
constexpr std::string_view kVersion("version");
constexpr std::string_view kPool("pool");
constexpr std::string_view kMessage("message");

/**
 * Functions to generate/validate key-value pairs for a DL record.
 */
class McInvalidationKvPairs {
 public:
  using KeyValuePairs = folly::F14FastMap<std::string, std::string>;

  /**
   * Api for invalidations writer.
   *
   * Create a set of key-value pairs
   * in order to write them to DL.
   *
   * Provided data must contain:
   * 1. Serialized delete request (string)
   * 2. Optional destination region
   * 3. Optional destination Memcache pool
   * 4. Optional free-format message string
   */
  static KeyValuePairs createAxonKvPairs(
      const std::string& serialized,
      std::optional<std::string> regionOpt = std::nullopt,
      std::optional<std::string> poolOpt = std::nullopt,
      std::optional<std::string> messageOpt = std::nullopt);

  /**
   * Api for invalidations reader.
   *
   * Validate key-value pairs set coming from the DL.
   *
   * Key-values must contain:
   * 1. "serialized" -> serialized delete request
   * 2. "version" -> Invalidation format version
   * 3. Optional free-format message string
   *
   * Returns false if the input is insufficient/incorrect
   */
  static bool validateAxonKvPairs(const KeyValuePairs& keyValues);

  template <class T>
  static folly::IOBuf serialize(const T& data) {
    apache::thrift::CompactProtocolWriter writer;
    folly::IOBufQueue buffer;
    writer.setOutput(&buffer);
    data.write(&writer);
    return buffer.moveAsValue();
  }
};

} // namespace facebook::memcache::invalidation
