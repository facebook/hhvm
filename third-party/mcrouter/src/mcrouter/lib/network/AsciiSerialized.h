/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/Range.h>

#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRoutingGroups.h"

namespace facebook {
namespace memcache {

/**
 * Class for serializing requests in ascii protocol.
 */
class AsciiSerializedRequest {
 public:
  AsciiSerializedRequest() = default;

  AsciiSerializedRequest(const AsciiSerializedRequest&) = delete;
  AsciiSerializedRequest& operator=(const AsciiSerializedRequest&) = delete;
  AsciiSerializedRequest(AsciiSerializedRequest&&) = delete;
  AsciiSerializedRequest& operator=(AsciiSerializedRequest&&) = delete;

  /**
   * Prepare buffers for given Request.
   *
   * @param request
   * @param op
   * @param iovOut  will be set to the beginning of array of ivecs that
   *                reference serialized data.
   * @param niovOut  number of valid iovecs referenced by iovOut.
   * @return true iff message was successfully prepared.
   */
  template <class Request>
  bool
  prepare(const Request& request, const struct iovec*& iovOut, size_t& niovOut);

  /**
   * Returns the size of the request.
   */
  size_t getSize() const;

 private:
  // We need at most 5 iovecs (lease-set):
  //   command + key + printBuffer + value + "\r\n"
  static constexpr size_t kMaxIovs = 8;
  // The longest print buffer we need is for lease-set/cas operations.
  // It requires 2 uint64, 2 uint32 + 4 spaces + "\r\n" + '\0' = 67 chars.
  static constexpr size_t kMaxBufferLength = 80;

  struct iovec iovs_[kMaxIovs];
  size_t iovsCount_{0};
  size_t iovsTotalLen_{0};
  char printBuffer_[kMaxBufferLength];

  void addString(folly::ByteRange range);
  void addString(folly::StringPiece str);

  template <class Arg1, class Arg2>
  void addStrings(Arg1&& arg1, Arg2&& arg2);
  template <class Arg, class... Args>
  void addStrings(Arg&& arg, Args&&... args);

  template <class Request>
  void keyValueRequestCommon(folly::StringPiece prefix, const Request& request);

  // Get-like ops.
  void prepareImpl(const McGetRequest& request);
  void prepareImpl(const McGetsRequest& request);
  void prepareImpl(const McMetagetRequest& request);
  void prepareImpl(const McLeaseGetRequest& request);
  void prepareImpl(const McGatRequest& request);
  void prepareImpl(const McGatsRequest& request);
  // Update-like ops.
  void prepareImpl(const McSetRequest& request);
  void prepareImpl(const McAddRequest& request);
  void prepareImpl(const McReplaceRequest& request);
  void prepareImpl(const McAppendRequest& request);
  void prepareImpl(const McPrependRequest& request);
  void prepareImpl(const McCasRequest& request);
  void prepareImpl(const McLeaseSetRequest& request);
  // Arithmetic ops.
  void prepareImpl(const McIncrRequest& request);
  void prepareImpl(const McDecrRequest& request);
  // Delete op.
  void prepareImpl(const McDeleteRequest& request);
  // Touch op.
  void prepareImpl(const McTouchRequest& request);
  // Version op.
  void prepareImpl(const McVersionRequest& request);
  // FlushAll op.
  void prepareImpl(const McFlushAllRequest& request);

  // Everything else is false.
  template <class Request>
  std::false_type prepareImpl(const Request& request);

  struct PrepareImplWrapper;
};

// TODO(jmswen) Merge AsciiSerializedReply and AsciiSerializedRequest into one
// class.
class AsciiSerializedReply {
 public:
  AsciiSerializedReply() = default;

  AsciiSerializedReply(const AsciiSerializedReply&) = delete;
  AsciiSerializedReply& operator=(const AsciiSerializedReply&) = delete;
  AsciiSerializedReply(AsciiSerializedReply&&) noexcept = delete;
  AsciiSerializedReply& operator=(AsciiSerializedReply&&) = delete;

  ~AsciiSerializedReply() = default;

  void clear();

  template <class Reply>
  bool prepare(
      Reply&& reply,
      folly::Optional<folly::IOBuf>& key,
      const struct iovec*& iovOut,
      size_t& niovOut,
      carbon::GetLikeT<RequestFromReplyType<Reply, RequestReplyPairs>> =
          nullptr) {
    if (key.hasValue()) {
      key->coalesce();
    }
    prepareImpl(
        std::move(reply),
        key.hasValue()
            ? folly::StringPiece(
                  reinterpret_cast<const char*>(key->data()), key->length())
            : folly::StringPiece());
    iovOut = iovs_;
    niovOut = iovsCount_;
    return true;
  }

  template <class Reply>
  bool prepare(
      Reply&& reply,
      const folly::Optional<folly::IOBuf>& /* key */,
      const struct iovec*& iovOut,
      size_t& niovOut,
      carbon::OtherThanT<Reply, carbon::GetLike<>> = nullptr) {
    prepareImpl(std::move(reply));
    iovOut = iovs_;
    niovOut = iovsCount_;
    return true;
  }

 private:
  // See comment in prepareImpl for McMetagetReply for explanation
  static constexpr size_t kMaxBufferLength = 100;

  static const size_t kMaxIovs = 16;
  struct iovec iovs_[kMaxIovs];
  size_t iovsCount_{0};
  char printBuffer_[kMaxBufferLength];
  // Used to keep alive the reply's IOBuf field (value, stats, etc.). For now,
  // replies have at most one IOBuf, so we only need one here. Note that one of
  // the iovs_ will point into the data managed by this IOBuf. A serialized
  // reply should not set iobuf_ more than once.
  // We also keep an auxiliary string for a similar purpose.
  folly::Optional<folly::IOBuf> iobuf_;
  folly::Optional<std::string> auxString_;

  void addString(folly::ByteRange range);
  void addString(folly::StringPiece str);

  template <class Arg1, class Arg2>
  void addStrings(Arg1&& arg1, Arg2&& arg2);
  template <class Arg, class... Args>
  void addStrings(Arg&& arg, Args&&... args);

  // Get-like ops
  void prepareImpl(McGetReply&& reply, folly::StringPiece key);
  void prepareImpl(McGetsReply&& reply, folly::StringPiece key);
  void prepareImpl(McMetagetReply&& reply, folly::StringPiece key);
  void prepareImpl(McLeaseGetReply&& reply, folly::StringPiece key);
  void prepareImpl(McGatReply&& reply, folly::StringPiece key);
  void prepareImpl(McGatsReply&& reply, folly::StringPiece key);
  // Update-like ops
  void prepareUpdateLike(
      carbon::Result result,
      uint16_t errorCode,
      std::string&& message,
      const char* requestName);
  void prepareImpl(McSetReply&& reply);
  void prepareImpl(McAddReply&& reply);
  void prepareImpl(McReplaceReply&& reply);
  void prepareImpl(McAppendReply&& reply);
  void prepareImpl(McPrependReply&& reply);
  void prepareImpl(McCasReply&& reply);
  void prepareImpl(McLeaseSetReply&& reply);
  // Arithmetic-like ops
  void prepareArithmeticLike(
      carbon::Result result,
      const uint64_t delta,
      uint16_t errorCode,
      std::string&& message,
      const char* requestName);
  void prepareImpl(McIncrReply&& reply);
  void prepareImpl(McDecrReply&& reply);
  // Delete
  void prepareImpl(McDeleteReply&& reply);
  // Touch
  void prepareImpl(McTouchReply&& reply);
  // Version
  void prepareImpl(const McVersionReply& reply);
  void prepareImpl(McVersionReply&& reply);
  // Miscellaneous
  void prepareImpl(McStatsReply&&);
  void prepareImpl(McShutdownReply&&);
  void prepareImpl(McQuitReply&&) {} // always noreply
  void prepareImpl(McExecReply&&);
  void prepareImpl(McFlushReReply&&);
  void prepareImpl(McFlushAllReply&&);
  // Server and client error helper
  void
  handleError(carbon::Result result, uint16_t errorCode, std::string&& message);
  void handleUnexpected(carbon::Result result, const char* requestName);
};
} // namespace memcache
} // namespace facebook

#include "AsciiSerialized-inl.h"
