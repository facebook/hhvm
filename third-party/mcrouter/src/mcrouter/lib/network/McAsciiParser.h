/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <exception>
#include <typeindex>

#include <folly/Optional.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/carbon/Variant.h"
#include "mcrouter/lib/fbi/cpp/TypeList.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {

class McAsciiParserBase {
 public:
  enum class State {
    // The parser is not initialized to parse any messages.
    UNINIT,
    // Have partial message, and need more data to complete it.
    PARTIAL,
    // There was an error on the protocol level.
    ERROR,
    // Complete message had been parsed and ready to be returned.
    COMPLETE,
  };

  McAsciiParserBase() = default;

  McAsciiParserBase(const McAsciiParserBase&) = delete;
  McAsciiParserBase& operator=(const McAsciiParserBase&) = delete;

  State getCurrentState() const noexcept {
    return state_;
  }

  /**
   * Check if McAsciiParser already has its own buffer.
   * @return  true iff we already have our own buffer that we can read into.
   */
  bool hasReadBuffer() const noexcept;

  std::pair<void*, size_t> getReadBuffer() noexcept;

  void readDataAvailable(size_t length);

  /**
   * Get a human readable description of error cause (e.g. received ERROR
   * reply, or failed to parse some data.)
   */
  folly::StringPiece getErrorDescription() const;

 protected:
  void handleError(folly::IOBuf& buffer);
  /**
   * Read value data.
   * It uses remainingIOBufLength_ to determine how much we need to read. It
   * will also update that variable and currentIOBuf_ accordingly.
   *
   * @return true iff the value was completely read.
   */
  bool readValue(folly::IOBuf& buffer, folly::IOBuf& to);
  bool readValue(folly::IOBuf& buffer, folly::Optional<folly::IOBuf>& to);

  static void appendKeyPiece(
      const folly::IOBuf& from,
      folly::IOBuf& to,
      const char* posStart,
      const char* posEnd);
  static void trimIOBufToRange(
      folly::IOBuf& buffer,
      const char* posStart,
      const char* posEnd);

  // limit the value size.
  static constexpr uint32_t maxValueBytes = 1 * 1024 * 1024 * 1024; // 1GB

  std::string currentErrorDescription_;

  uint64_t currentUInt_{0};

  folly::IOBuf* currentIOBuf_{nullptr};
  size_t remainingIOBufLength_{0};
  State state_{State::UNINIT};
  bool negative_{false};

  // Variables used by ragel.
  int savedCs_;
  int errorCs_;
  const char* p_{nullptr};
  const char* pe_{nullptr};
};

class McClientAsciiParser : public McAsciiParserBase {
 public:
  /**
   * Consume given IOBuf.
   *
   * Should be called only in case hasReadBuffer() returned false.
   *
   * @param buffer  data to consume.
   * @return  new parser state.
   */
  State consume(folly::IOBuf& buffer);

  /**
   * Prepares parser for parsing reply for given request type and operation.
   */
  template <class Request>
  void initializeReplyParser();

  /**
   * Obtain the message that was parsed.
   *
   * Should be called by user to obtain reply after consume() returns
   * State::COMPLETE.
   *
   * @tparam T  type of expected reply.
   */
  template <class T>
  T getReply();

 private:
  template <class Reply>
  void initializeCommon();

  template <class Reply>
  void initializeArithmReplyCommon();
  template <class Reply>
  void initializeStorageReplyCommon();

  template <class Reply>
  void consumeArithmReplyCommon(folly::IOBuf& buffer);
  template <class Reply>
  void consumeStorageReplyCommon(folly::IOBuf& buffer);

  template <class Request>
  void consumeMessage(folly::IOBuf& buffer);

  template <class Reply>
  void consumeErrorMessage(const folly::IOBuf& buffer);

  template <class Reply>
  void consumeVersion(const folly::IOBuf& buffer);

  template <class Reply>
  void consumeIpAddr(const folly::IOBuf& buffer);
  template <class Reply>
  void consumeIpAddrHelper(const folly::IOBuf& buffer);

  template <class Reply>
  void resetErrorMessage(Reply& message);

  static void initFirstCharIOBuf(
      const folly::IOBuf& from,
      folly::IOBuf& to,
      const char* pos);
  static void appendCurrentCharTo(
      const folly::IOBuf& from,
      folly::IOBuf& to,
      const char* pos);

  using ReplyVariant = carbon::makeVariantFromList<MapT<ReplyT, McRequestList>>;
  ReplyVariant currentMessage_;

  using ConsumerFunPtr = void (McClientAsciiParser::*)(folly::IOBuf&);
  ConsumerFunPtr consumer_{nullptr};
};

namespace detail {
template <class RequestList>
class CallbackBase;
} // namespace detail

class McServerAsciiParser : public McAsciiParserBase {
 public:
  template <class Callback>
  explicit McServerAsciiParser(Callback& cb);

  /**
   * Consume given IOBuf.
   *
   * Should be called only in case hasReadBuffer() returned false.
   *
   * @param buffer  data to consume.
   * @return  new parser state.
   */
  State consume(folly::IOBuf& buffer);

 private:
  void opTypeConsumer(folly::IOBuf& buffer);

  // Get-like.
  template <class Request>
  void initGetLike();
  template <class Request>
  void consumeGetLike(folly::IOBuf& buffer);
  template <class Request>
  void initGatLike();
  template <class Request>
  void consumeGatLike(folly::IOBuf& buffer);

  // Update-like.
  template <class Request>
  void initSetLike();
  template <class Request>
  void consumeSetLike(folly::IOBuf& buffer);
  void consumeCas(folly::IOBuf& buffer);
  void consumeLeaseSet(folly::IOBuf& buffer);

  void consumeDelete(folly::IOBuf& buffer);
  void consumeTouch(folly::IOBuf& buffer);

  void consumeShutdown(folly::IOBuf& buffer);

  // Arithmetic.
  template <class Request>
  void initArithmetic();
  template <class Request>
  void consumeArithmetic(folly::IOBuf& buffer);

  void consumeStats(folly::IOBuf& buffer);
  void consumeExec(folly::IOBuf& buffer);

  // Flush.
  void consumeFlushRe(folly::IOBuf& buffer);
  void consumeFlushAll(folly::IOBuf& buffer);

  void finishReq();

  std::unique_ptr<detail::CallbackBase<McRequestList>> callback_;

  const char* keyPieceStart_{nullptr};
  folly::IOBuf currentKey_;
  bool noreply_{false};

  using RequestVariant = carbon::makeVariantFromList<McRequestList>;
  RequestVariant currentMessage_;

  using ConsumerFunPtr = void (McServerAsciiParser::*)(folly::IOBuf&);
  ConsumerFunPtr consumer_{nullptr};
};
} // namespace memcache
} // namespace facebook

#include "McAsciiParser-inl.h"
