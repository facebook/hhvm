/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <folly/Format.h>
#include <folly/Traits.h>

#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/BigValueRouteIf.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace folly {
class IOBuf;
} // namespace folly

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * For get-like request:
 * 1. Perform get-like request on child route handle.
 * 2. If the received reply is a reply for big get request, generate chunk
 * getlike requests and forward to child route handle.
 * Merge all the replies and return it.
 * 3. Else return the reply.
 *
 * Note that the above describes how lease-get logically works, but the
 * actual implementation of BigValueRoute for lease-gets is more complicated.
 *
 * For update-like request:
 * 1. If value size is below or equal to threshold option,
 * route request to child route handle and return reply
 * 2. If value size is greater than threshold option,
 * generate chunk requests from original request and
 * send them to child route handle. If all of the chunk
 * update is successful, route request with original key and modified value
 * to child route handle and return reply. Else, return worse of the
 * replies for chunk updates
 *
 * Default behavior for other type of operations
 */
template <class RouterInfo>
class BigValueRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  static std::string routeName() {
    return "big-value";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const;

  BigValueRoute(RouteHandlePtr ch, BigValueRouteOptions options);

  template <class Request>
  typename std::enable_if<
      folly::IsOneOf<
          Request,
          McGetRequest,
          McGetsRequest,
          McGatRequest,
          McGatsRequest>::value,
      ReplyT<Request>>::type
  route(const Request& req) const;

  McMetagetReply route(const McMetagetRequest& req) const;
  McLeaseGetReply route(const McLeaseGetRequest& req) const;

  template <class Request>
  typename std::enable_if_t<MemcacheUpdateLike<Request>::value, ReplyT<Request>>
  route(const Request& req) const;

  template <class Request>
  typename std::enable_if_t<
      OtherThanMemcacheGetUpdateLike<Request>::value,
      ReplyT<Request>>
  route(const Request& req) const;

 private:
  const RouteHandlePtr ch_;
  const BigValueRouteOptions options_;

  class ChunksInfo {
   public:
    explicit ChunksInfo(folly::StringPiece replyValue);
    explicit ChunksInfo(uint32_t numChunks, uint64_t suffix__);

    folly::IOBuf toStringType() const;
    uint32_t numChunks() const;
    uint64_t suffix() const;
    bool valid() const;

   private:
    const uint32_t infoVersion_;
    uint32_t numChunks_;
    uint64_t suffix_;
    bool valid_;
  };

  McLeaseGetReply doLeaseGetRoute(
      const McLeaseGetRequest& req,
      size_t retriesLeft) const;

  template <class FuncIt>
  std::vector<typename std::result_of<
      typename std::iterator_traits<FuncIt>::value_type()>::type>
  collectAllByBatches(FuncIt beginF, FuncIt endF) const;

  template <class Request>
  typename std::enable_if_t<
      MemcacheUpdateLike<Request>::value,
      std::pair<std::vector<McSetRequest>, ChunksInfo>>
  chunkUpdateRequests(
      folly::StringPiece baseKey,
      const folly::IOBuf& value,
      int32_t exptime) const;

  template <class FromRequest>
  std::vector<McGetRequest> chunkGetRequests(
      const FromRequest& req,
      const ChunksInfo& info) const;

  template <typename InputIterator, class Reply>
  Reply mergeChunkGetReplies(
      InputIterator begin,
      InputIterator end,
      Reply&& initReply) const;

  folly::IOBuf
  createChunkKey(folly::StringPiece key, uint32_t index, uint64_t suffix) const;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "BigValueRoute-inl.h"
