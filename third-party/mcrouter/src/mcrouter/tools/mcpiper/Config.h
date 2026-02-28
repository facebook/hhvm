/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <memory>
#include <string>

#include "mcrouter/tools/mcpiper/ValueFormatter.h"

namespace facebook {
namespace memcache {

class CompressionCodecMap;
class MessagePrinter;
template <class T>
class SnifferParserBase;

namespace detail {

template <class Reply>
struct MatchingRequest {
  static constexpr const char* name();
};

} // namespace detail

/**
 * Returns the default fifo root.
 */
std::string getDefaultFifoRoot();

/**
 * Creates value formatter.
 */
std::unique_ptr<ValueFormatter> createValueFormatter();

/**
 * Return current version.
 */
std::string getVersion();

/**
 * Initializes compression support.
 */
bool initCompression();

/**
 * Gets compression codec map.
 * If compression is not initialized, return nullptr.
 */
const CompressionCodecMap* getCompressionCodecMap(folly::EventBase& evb);

/**
 * Adds SnifferParser based on protocol to the parser map
 */
std::unordered_map<
    uint64_t,
    std::unique_ptr<SnifferParserBase<MessagePrinter>>>::iterator
addCarbonSnifferParser(
    std::string name,
    std::unordered_map<
        uint64_t,
        std::unique_ptr<SnifferParserBase<MessagePrinter>>>& parserMap,
    uint64_t connectionId,
    MessagePrinter& printer);
} // namespace memcache
} // namespace facebook
