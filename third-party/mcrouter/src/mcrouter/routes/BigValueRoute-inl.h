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
#include "mcrouter/routes/McRouteHandleBuilder.h"

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

template <class RouterInfo>
template <class FuncIt>
std::vector<typename std::result_of<
    typename std::iterator_traits<FuncIt>::value_type()>::type>
BigValueRoute<RouterInfo>::collectAllByBatches(FuncIt beginF, FuncIt endF)
    const {
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

template <class RouterInfo>
template <class Request>
bool BigValueRoute<RouterInfo>::traverse(
    const Request& req,
    const RouteHandleTraverser<RouteHandleIf>& t) const {
  return t(*ch_, req);
}

template <class RouterInfo>
template <class Request>
typename std::enable_if<
    folly::IsOneOf<
        Request,
        McGetRequest,
        McGetsRequest,
        McGatRequest,
        McGatsRequest>::value,
    ReplyT<Request>>::type
BigValueRoute<RouterInfo>::route(const Request& req) const {
  auto initialReply = ch_->route(req);
  bool isBigValue = ((*initialReply.flags_ref() & MC_MSG_FLAG_BIG_VALUE) != 0);
  if (!isBigValue) {
    return initialReply;
  }

  // unset big value flag
  if (options_.hideReplyFlags) {
    *initialReply.flags_ref() ^= MC_MSG_FLAG_BIG_VALUE;
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

template <class RouterInfo>
template <class Request>
typename std::enable_if_t<MemcacheUpdateLike<Request>::value, ReplyT<Request>>
BigValueRoute<RouterInfo>::route(const Request& req) const {
  if (req.value_ref()->computeChainDataLength() <= options_.threshold) {
    return ch_->route(req);
  }

  auto reqsInfoPair = chunkUpdateRequests<Request>(
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

template <class RouterInfo>
template <class Request>
typename std::
    enable_if_t<OtherThanMemcacheGetUpdateLike<Request>::value, ReplyT<Request>>
    BigValueRoute<RouterInfo>::route(const Request& req) const {
  return ch_->route(req);
}

template <class RouterInfo>
template <class FromRequest>
std::vector<McGetRequest> BigValueRoute<RouterInfo>::chunkGetRequests(
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

template <class RouterInfo>
template <typename InputIterator, class Reply>
Reply BigValueRoute<RouterInfo>::mergeChunkGetReplies(
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

template <class RouterInfo>
template <class Request>
typename std::enable_if_t<
    MemcacheUpdateLike<Request>::value,
    std::pair<
        std::vector<McSetRequest>,
        typename BigValueRoute<RouterInfo>::ChunksInfo>>
BigValueRoute<RouterInfo>::chunkUpdateRequests(
    folly::StringPiece baseKey,
    const folly::IOBuf& value,
    int32_t exptime) const {
  int numChunks = (value.computeChainDataLength() + options_.threshold - 1) /
      options_.threshold;
  ChunksInfo info(numChunks, detail::hashBigValue(value));

  std::vector<McSetRequest> chunkReqs;
  chunkReqs.reserve(numChunks);

  folly::IOBuf chunkValue;
  folly::io::Cursor cursor(&value);
  for (int i = 0; i < numChunks; ++i) {
    cursor.cloneAtMost(chunkValue, options_.threshold);
    chunkReqs.emplace_back(createChunkKey(baseKey, i, info.suffix()));
    chunkReqs.back().value_ref() = std::move(chunkValue);
    chunkReqs.back().exptime_ref() = exptime;
  }

  return std::make_pair(std::move(chunkReqs), info);
}

template <class RouterInfo>
McMetagetReply BigValueRoute<RouterInfo>::route(
    const McMetagetRequest& req) const {
  // TODO: Make metaget work with BigValueRoute. One way to make this work well
  // is to add 'flags' to McMetagetReply.
  return ch_->route(req);
}

template <class RouterInfo>
McLeaseGetReply BigValueRoute<RouterInfo>::route(
    const McLeaseGetRequest& req) const {
  // Limit the number of recursive calls since we're on a fiber.
  return doLeaseGetRoute(req, 1 /* retriesLeft */);
}

template <class RouterInfo>
McLeaseGetReply BigValueRoute<RouterInfo>::doLeaseGetRoute(
    const McLeaseGetRequest& req,
    size_t retriesLeft) const {
  auto initialReply = ch_->route(req);
  bool isBigValue = ((*initialReply.flags_ref() & MC_MSG_FLAG_BIG_VALUE) != 0);
  if (!isBigValue) {
    return initialReply;
  }
  // unset big value flag
  if (options_.hideReplyFlags) {
    *initialReply.flags_ref() ^= MC_MSG_FLAG_BIG_VALUE;
  }

  if (!isHitResult(*initialReply.result_ref())) {
    // if bigValue item, create a new reply with result, lease-token and return
    // so that we don't send any meta data that may be present in initialReply
    McLeaseGetReply reply(*initialReply.result_ref());
    reply.leaseToken_ref() = *initialReply.leaseToken_ref();
    return reply;
  }

  ChunksInfo chunksInfo(coalesceAndGetRange(initialReply.value_ref()));
  if (!chunksInfo.valid()) {
    // We cannot return carbon::Result::NOTFOUND without a valid lease token. We
    // err on the side of allowing clients to make progress by returning a lease
    // token of -1.
    McLeaseGetReply missReply(carbon::Result::NOTFOUND);
    missReply.leaseToken_ref() = static_cast<uint64_t>(-1);
    return missReply;
  }

  // Send a gets request for the metadata while sending ordinary get requests
  // to fetch the subpieces. We may need to use the returned CAS token to
  // invalidate the metadata piece later on.
  const auto key = req.key_ref()->fullKey();
  McGetsRequest getsMetadataReq(key);
  const auto reqs = chunkGetRequests(req, chunksInfo);
  std::vector<std::function<McGetReply()>> fs;
  fs.reserve(reqs.size());

  auto& target = *ch_;
  for (const auto& chunkReq : reqs) {
    fs.push_back([&target, &chunkReq]() { return target.route(chunkReq); });
  }

  McGetsReply getsMetadataReply;
  std::vector<McGetReply> replies;
  std::vector<std::function<void()>> tasks;
  tasks.emplace_back([&getsMetadataReq, &getsMetadataReply, &target]() {
    getsMetadataReply = target.route(getsMetadataReq);
  });
  tasks.emplace_back([this, &replies, fs = std::move(fs)]() mutable {
    replies = collectAllByBatches(fs.begin(), fs.end());
  });

  folly::fibers::collectAll(tasks.begin(), tasks.end());
  const auto reducedReply = mergeChunkGetReplies(
      replies.begin(), replies.end(), std::move(initialReply));

  // Return reducedReply on hit or error
  if (!isMissResult(*reducedReply.result_ref())) {
    return reducedReply;
  }

  if (isErrorResult(*getsMetadataReply.result_ref())) {
    if (retriesLeft > 0) {
      return doLeaseGetRoute(req, --retriesLeft);
    }
    McLeaseGetReply errorReply(*getsMetadataReply.result_ref());
    errorReply.message_ref() = std::move(*getsMetadataReply.message_ref());
    return errorReply;
  }

  // This is the tricky part with leases. There was a hit on the metadata,
  // but a miss/error on one of the subpieces. One of the clients needs to
  // invalidate the metadata. Then one (possibly the same) client will be able
  // to get a valid lease token.
  // TODO: Consider also firing off async deletes for the subpieces for better
  // cache use.
  if (isHitResult(*getsMetadataReply.result_ref())) {
    McCasRequest invalidateReq(key);
    invalidateReq.exptime_ref() = -1;
    invalidateReq.casToken_ref() = *getsMetadataReply.casToken_ref();
    auto invalidateReply = ch_->route(invalidateReq);
    if (isErrorResult(*invalidateReply.result_ref())) {
      McLeaseGetReply errorReply(*invalidateReply.result_ref());
      errorReply.message_ref() = std::move(*invalidateReply.message_ref());
      return errorReply;
    }
  }

  if (retriesLeft > 0) {
    return doLeaseGetRoute(req, --retriesLeft);
  }

  McLeaseGetReply reply(carbon::Result::REMOTE_ERROR);
  reply.message_ref() = folly::sformat(
      "BigValueRoute: exhausted retries for lease-get for key {}", key);
  return reply;
}

template <class RouterInfo>
BigValueRoute<RouterInfo>::ChunksInfo::ChunksInfo(folly::StringPiece replyValue)
    : infoVersion_(1), valid_(true) {
  // Verify that replyValue is of the form version-numChunks-suffix,
  // where version, numChunks and suffix should be numeric
  uint32_t version;
  int charsRead;
  valid_ &=
      (sscanf(
           replyValue.data(),
           "%u-%u-%lu%n",
           &version,
           &numChunks_,
           &suffix_,
           &charsRead) == 3);
  valid_ &= (static_cast<size_t>(charsRead) == replyValue.size());
  valid_ &= (version == infoVersion_);
}

template <class RouterInfo>
BigValueRoute<RouterInfo>::ChunksInfo::ChunksInfo(
    uint32_t chunks,
    uint64_t suffix__)
    : infoVersion_(1), numChunks_(chunks), suffix_(suffix__), valid_(true) {}

template <class RouterInfo>
folly::IOBuf BigValueRoute<RouterInfo>::ChunksInfo::toStringType() const {
  return folly::IOBuf(
      folly::IOBuf::COPY_BUFFER,
      folly::sformat("{}-{}-{}", infoVersion_, numChunks_, suffix_));
}

template <class RouterInfo>
uint32_t BigValueRoute<RouterInfo>::ChunksInfo::numChunks() const {
  return numChunks_;
}

template <class RouterInfo>
uint64_t BigValueRoute<RouterInfo>::ChunksInfo::suffix() const {
  return suffix_;
}

template <class RouterInfo>
bool BigValueRoute<RouterInfo>::ChunksInfo::valid() const {
  return valid_;
}

template <class RouterInfo>
BigValueRoute<RouterInfo>::BigValueRoute(
    RouteHandlePtr ch,
    BigValueRouteOptions options)
    : ch_(std::move(ch)), options_(options) {
  assert(ch_ != nullptr);
}

template <class RouterInfo>
folly::IOBuf BigValueRoute<RouterInfo>::createChunkKey(
    folly::StringPiece baseKey,
    uint32_t chunkIndex,
    uint64_t suffix) const {
  return folly::IOBuf(
      folly::IOBuf::COPY_BUFFER,
      folly::sformat("{}:{}:{}", baseKey, chunkIndex, suffix));
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeBigValueRoute(
    typename RouterInfo::RouteHandlePtr rh,
    BigValueRouteOptions options) {
  return makeRouteHandleWithInfo<RouterInfo, BigValueRoute>(
      std::move(rh), std::move(options));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
