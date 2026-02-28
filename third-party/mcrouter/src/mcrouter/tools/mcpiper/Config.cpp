/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Config.h"

#include <memory>

#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/lib/network/gen/MemcacheServer.h"
#include "mcrouter/tools/mcpiper/MessagePrinter.h"
#include "mcrouter/tools/mcpiper/SnifferParser.h"

namespace facebook {
namespace memcache {

namespace detail {

using RequestReplyRegistry = carbon::GetRequestReplyPairs<MemcacheRequestList>;

template <class Reply>
constexpr const char* MatchingRequest<Reply>::name() {
  using Request = RequestFromReplyType<Reply, RequestReplyRegistry>;
  return Request::name;
}

} // namespace detail

std::string getDefaultFifoRoot() {
  return "/var/mcrouter/fifos";
}

std::unique_ptr<ValueFormatter> createValueFormatter() {
  return std::make_unique<ValueFormatter>();
}

std::string getVersion() {
  return "mcpiper 1.0";
}

bool initCompression() {
  return false;
}

const CompressionCodecMap* getCompressionCodecMap(folly::EventBase& /* evb */) {
  return nullptr;
}

std::unordered_map<
    uint64_t,
    std::unique_ptr<SnifferParserBase<MessagePrinter>>>::iterator
addCarbonSnifferParser(
    std::string /* routerName */,
    std::unordered_map<
        uint64_t,
        std::unique_ptr<SnifferParserBase<MessagePrinter>>>& parserMap,
    uint64_t connectionId,
    MessagePrinter& printer) {
  return parserMap
      .emplace(
          connectionId,
          std::make_unique<SnifferParser<
              MessagePrinter,
              memcache::detail::MemcacheRequestList>>(printer))
      .first;
}

} // namespace memcache
} // namespace facebook
