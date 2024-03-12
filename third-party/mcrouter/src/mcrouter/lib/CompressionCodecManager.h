/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>

#include <folly/io/async/EventBaseLocal.h>

#include "mcrouter/lib/Compression.h"

namespace facebook {
namespace memcache {

class CompressionCodecMap;

/**
 * Represents a configuration of a compression codec.
 */
struct CodecConfig {
  const uint32_t id;
  const CompressionCodecType codecType;
  const std::string dictionary;
  const FilteringOptions filteringOptions;
  const uint32_t compressionLevel;

  CodecConfig(
      uint32_t codecId,
      CompressionCodecType type,
      std::string dic,
      FilteringOptions codecFilteringOptions = FilteringOptions(),
      uint32_t codecCompressionLevel = 1)
      : id(codecId),
        codecType(type),
        dictionary(std::move(dic)),
        filteringOptions(codecFilteringOptions),
        compressionLevel(codecCompressionLevel) {}
};
using CodecConfigPtr = std::unique_ptr<CodecConfig>;

/**
 * Manager of compression codecs.
 */
class CompressionCodecManager {
 public:
  explicit CompressionCodecManager(
      std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs);

  /**
   * Return the compression codec map.
   * Note: thread-safe.
   */
  const CompressionCodecMap* getCodecMap(folly::EventBase& evb) const;

 private:
  // Storage of compression codec configs (codecId -> codecConfig).
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs_;
  // EventBaseLocal of compression codec map, as codecs are not thread-safe.
  mutable folly::EventBaseLocal<CompressionCodecMap> compressionCodecMap_;
  // Codec id range
  uint32_t smallestCodecId_{0};
  uint32_t size_{0};
};

/**
 * Represent a range of valid compression codecs ids
 */
struct CodecIdRange {
  uint64_t firstId;
  size_t size;

  bool isEmpty() const {
    return firstId == Empty.firstId && size == Empty.size;
  }

  static const CodecIdRange Empty;
};
/**
 * Map of codec compressors.
 * The ids of the codecs held by this map must be contiguous.
 */
class CompressionCodecMap {
 public:
  /**
   * Returns the compression codec of the given id.
   *
   * @param id  Id of the codec.
   * @return    The codec (or nullptr if not found).
   */
  CompressionCodec* get(uint32_t id) const noexcept;

  /**
   * Get the compression codec that best matches the filters.
   *
   * @param codecRange     Range of codecs ids.
   * @param bodySize       Size of the reply body.
   * @param typeId         TypeId of the reply.
   * @return               The codec that best matches the filters
   *                       (or nullptr if none found).
   */
  CompressionCodec* getBest(
      const CodecIdRange& codecRange,
      const size_t bodySize,
      const size_t typeId) const noexcept;

  /**
   * Returns the size of this map.
   */
  size_t size() const noexcept {
    return codecs_.size();
  }

  /**
   * Returns the range (firstId, size) of codec ids present in this map.
   */
  const CodecIdRange getIdRange() const noexcept {
    return {firstId_, size()};
  }

 private:
  std::vector<std::unique_ptr<CompressionCodec>> codecs_;
  std::vector<std::vector<uint32_t>> codecsIdByTypeId_;
  const uint32_t firstId_{0};

  /**
   * Builds an empty codec map.
   */
  CompressionCodecMap() noexcept;

  /**
   * Builds a map containing codecs which the ids are within the given range.
   * Note: All codecs in the [smallestCodecId, smallestCodecIdtId + size]
   *       range must be present and valid.
   * Note: createCompressionCodec may throw exceptions when failing to load
   *       compression codecs of specific type.
   *
   * @param codecConfigs       Map of (codecId -> codecConfig). Must contain all
   *                           codecs in the given range.
   * @param smallestCodecId    First id of the range of codecs.
   * @param size               Size of the range.
   */
  CompressionCodecMap(
      const std::unordered_map<uint32_t, CodecConfigPtr>& codecConfigs,
      uint32_t smallestCodecId,
      uint32_t size);

  /**
   * Get the compression codec that best matches the filters considering
   * codecs only with typeId equal to passed typeId of the reply
   *
   * @param codecRange     Range of codecs ids.
   * @param bodySize       Size of the reply.
   * @param typeId         TypeId of the reply.
   * @return               The codec that best matches the filters
   *                       (or nullptr if none found).
   */
  CompressionCodec* getBestByTypeId(
      const CodecIdRange& codecRange,
      const size_t bodySize,
      const size_t typeId) const noexcept;

  // Return the codecs_ vector index given the codec id.
  uint32_t index(uint32_t id) const noexcept;

  friend class CompressionCodecManager;
};
} // namespace memcache
} // namespace facebook
