/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
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
  return 2;
}

enum class DistributionOperation : uint8_t { Delete, Write };
enum class DistributionType : uint8_t { Async, Distribution };

constexpr std::string_view kSerialized("serialized");
constexpr std::string_view kRegion("region");
constexpr std::string_view kVersion("version");
constexpr std::string_view kPool("pool");
constexpr std::string_view kMessage("message");
constexpr std::string_view kType("type");
constexpr std::string_view kOperation("op");
constexpr std::string_view kSourceRegion("src_region");

/**
 * Functions to generate/validate key-value pairs for a DL record.
 */
class McInvalidationKvPairs {
 public:
  using KeyValuePairs = folly::F14FastMap<std::string, std::string>;

  /**
   * Api for invalidations writer.
   *
   * Create a set of key-value pairs from the input data,
   * in order to write them to DL.
   *
   * @param serialized Serialized request (string)
   * @param region Destination region
   * @param pool Destination pool (e.g. main|tfh)
   * @param message Free-format message string
   * @param type Type of the distribution (async/distribution)
   * @param operation Operation of the serialized entity (delete/write)
   * @param srcRegion Source (current) region
   *
   */
  static KeyValuePairs createAxonKvPairs(
      std::string serialized,
      std::optional<std::string> region = std::nullopt,
      std::optional<std::string> pool = std::nullopt,
      std::optional<std::string> message = std::nullopt,
      DistributionType type = DistributionType::Async,
      DistributionOperation operation = DistributionOperation::Delete,
      std::optional<std::string> srcRegion = std::nullopt);

  /**
   * Api for invalidations reader.
   *
   * Validate key-value pairs set coming from the DL.
   *
   * Key-values at minimum must contain:
   * @param serialized Serialized delete request
   * @param version Invalidation format version
   *
   * @return True if the input is valid, False otherwise.
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
