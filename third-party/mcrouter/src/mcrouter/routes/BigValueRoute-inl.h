/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iterator>
#include <type_traits>

#include <folly/Range.h>
#include <folly/fibers/FiberManager.h>
#include <folly/fibers/WhenN.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/AuxiliaryCPUThreadPool.h"
#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {
template <class InputIterator>
InputIterator reduce(InputIterator begin, InputIterator end) {
  if (begin == end) {
    return end;
  }
  InputIterator worstIt = begin;
  auto worstSeverity = resultSeverity(*begin->result_ref());

  for (++begin; begin != end; ++begin) {
    if (resultSeverity(*begin->result_ref()) > worstSeverity) {
      worstIt = begin;
      worstSeverity = resultSeverity(*begin->result_ref());
    }
  }
  return worstIt;
}

// Hashes value on a separate CPU thread pool, preempts fiber until hashing is
// complete.
uint64_t hashBigValue(const folly::IOBuf& value);

} // namespace detail

template <class FuncIt>
std::vector<typename std::result_of<
    typename std::iterator_traits<FuncIt>::value_type()>::type>
BigValueRoute::collectAllByBatches(FuncIt beginF, FuncIt endF) const {
  using Reply = typename std::result_of<
      typename std::iterator_traits<FuncIt>::value_type()>::type;

  auto batchSize = options_.batchSize;
  const size_t rangeSize = std::distance(beginF, endF);
  if (batchSize == 0) {
    batchSize = rangeSize;
  }

  std::vector<Reply> allReplies;
  size_t b = 0;
  size_t e = std::min(rangeSize, batchSize);
  while (b < rangeSize) {
    auto replies = folly::fibers::collectAll(beginF + b, beginF + e);
    for (auto& r : replies) {
      allReplies.emplace_back(std::move(r));
    }
    b = e;
    e = std::min(rangeSize, e + batchSize);
  }
  return allReplies;
}

template <class Request>
bool BigValueRoute::traverse(
    const Request& req,
    const RouteHandleTraverser<MemcacheRouteHandleIf>& t) const {
  return t(*ch_, req);
}

template <class Request>
typename std::enable_if<
    folly::IsOneOf<
        Request,
        McGetRequest,
        McGetsRequest,
        McGatRequest,
        McGatsRequest>::value,
    ReplyT<Request>>::type
BigValueRoute::route(const Request& req) const {
  auto initialReply = ch_->route(req);
  bool isBigValue = ((*initialReply.flags_ref() & MC_MSG_FLAG_BIG_VALUE) != 0);
  if (!isBigValue) {
    return initialReply;
  }

  if (!isHitResult(*initialReply.result_ref())) {
    // if bigValue item, create a new reply with result and return
    // so that we don't send any meta data that may be present in initialReply
    ReplyT<Request> reply(*initialReply.result_ref());
    return reply;
  }

  ChunksInfo chunksInfo(coalesceAndGetRange(initialReply.value_ref()));
  if (!chunksInfo.valid()) {
    return createReply(DefaultReply, req);
  }

  auto reqs = chunkGetRequests(req, chunksInfo);
  std::vector<std::function<McGetReply()>> fs;
  fs.reserve(reqs.size());

  auto& target = *ch_;
  for (const auto& chunkReq : reqs) {
    fs.push_back([&target, &chunkReq]() { return target.route(chunkReq); });
  }

  auto replies = collectAllByBatches(fs.begin(), fs.end());
  return mergeChunkGetReplies(
      replies.begin(), replies.end(), std::move(initialReply));
}

template <class Request>
ReplyT<Request> BigValueRoute::route(
    const Request& req,
    carbon::UpdateLikeT<Request>) const {
  if (req.value_ref()->computeChainDataLength() <= options_.threshold) {
    return ch_->route(req);
  }

  auto reqsInfoPair = chunkUpdateRequests(
      req.key_ref()->fullKey(), *req.value_ref(), *req.exptime_ref());
  std::vector<std::function<McSetReply()>> fs;
  auto& chunkReqs = reqsInfoPair.first;
  fs.reserve(chunkReqs.size());

  auto& target = *ch_;
  for (const auto& chunkReq : chunkReqs) {
    fs.push_back([&target, &chunkReq]() { return target.route(chunkReq); });
  }

  auto replies = collectAllByBatches(fs.begin(), fs.end());

  // reply for all chunk update requests
  auto reducedReply = detail::reduce(replies.begin(), replies.end());
  if (isStoredResult(*reducedReply->result_ref())) {
    // original key with modified value stored at the back
    auto newReq = req;
    newReq.flags_ref() = *req.flags_ref() | MC_MSG_FLAG_BIG_VALUE;
    newReq.value_ref() = reqsInfoPair.second.toStringType();
    return ch_->route(newReq);
  } else {
    return ReplyT<Request>(*reducedReply->result_ref());
  }
}

template <class Request>
ReplyT<Request> BigValueRoute::route(
    const Request& req,
    carbon::OtherThanT<Request, carbon::GetLike<>, carbon::UpdateLike<>>)
    const {
  return ch_->route(req);
}

template <class FromRequest>
std::vector<McGetRequest> BigValueRoute::chunkGetRequests(
    const FromRequest& req,
    const ChunksInfo& info) const {
  std::vector<McGetRequest> bigGetReqs;
  bigGetReqs.reserve(info.numChunks());

  auto baseKey = req.key_ref()->fullKey();
  for (uint32_t i = 0; i < info.numChunks(); i++) {
    // override key with chunk keys
    bigGetReqs.emplace_back(createChunkKey(baseKey, i, info.suffix()));
  }

  return bigGetReqs;
}

template <typename InputIterator, class Reply>
Reply BigValueRoute::mergeChunkGetReplies(
    InputIterator begin,
    InputIterator end,
    Reply&& initialReply) const {
  auto reducedReplyIt = detail::reduce(begin, end);
  if (!isHitResult(*reducedReplyIt->result_ref())) {
    return Reply(*reducedReplyIt->result_ref());
  }

  initialReply.result_ref() = *reducedReplyIt->result_ref();

  std::vector<std::unique_ptr<folly::IOBuf>> dataVec;
  while (begin != end) {
    if (begin->value_ref().has_value()) {
      dataVec.push_back(begin->value_ref()->clone());
    }
    ++begin;
  }

  initialReply.value_ref() = concatAll(dataVec.begin(), dataVec.end());
  return std::move(initialReply);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
