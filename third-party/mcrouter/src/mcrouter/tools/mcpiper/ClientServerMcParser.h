/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBaseManager.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/ClientMcParser.h"
#include "mcrouter/lib/network/McParser.h"
#include "mcrouter/lib/network/ServerMcParser.h"
#include "mcrouter/tools/mcpiper/Config.h"

namespace folly {
class IOBuf;
} // namespace folly

namespace facebook {
namespace memcache {

constexpr size_t kReadBufferSizeMin = 256;
constexpr size_t kReadBufferSizeMax = 4096;

namespace detail {

template <class ReplyParser, class RequestList>
class ExpectNextDispatcher {
 public:
  explicit ExpectNextDispatcher(ReplyParser* parser) : replyParser_(parser) {}

  void dispatch(size_t typeId) {
    dispatcher_.dispatch(typeId, *this);
  }

  template <class M>
  static void processMsg(ExpectNextDispatcher& me) {
    assert(me.replyParser_);
    me.replyParser_->template expectNext<M>();
  }

  void setReplyParser(ReplyParser* parser) {
    replyParser_ = parser;
  }

 private:
  ReplyParser* replyParser_;
  CallDispatcher<RequestList, ExpectNextDispatcher> dispatcher_;
};

} // namespace detail

template <class Callback, class RequestList>
class ClientServerMcParser {
 public:
  class ReplyCallback {
   public:
    explicit ReplyCallback(Callback& callback) : callback_(callback) {}

    template <class Reply>
    void
    replyReady(Reply&& reply, uint64_t msgId, RpcStatsContext rpcStatsContext) {
      callback_.template replyReady<Reply>(
          msgId, std::move(reply), rpcStatsContext);
    }

    bool nextReplyAvailable(uint64_t) {
      return true;
    }

    void parseError(carbon::Result, folly::StringPiece) {}

    void handleConnectionControlMessage(
        const CaretMessageInfo& /* headerInfo */) {}

   private:
    Callback& callback_;
  };

  struct RequestCallback
      : public CarbonMessageDispatcher<RequestList, RequestCallback> {
   public:
    template <class M>
    void onTypedMessage(
        const CaretMessageInfo& headerInfo,
        const folly::IOBuf& /* reqBuffer */,
        M&& req) {
      callback_.requestReady(headerInfo.reqId, std::move(req));
    }

    explicit RequestCallback(Callback& callback) : callback_(callback) {}

    template <class Request>
    void onRequest(Request&& req, bool /* noreply */) {
      callback_.requestReady(0, std::move(req));
    }

    void caretRequestReady(
        const CaretMessageInfo& headerInfo,
        const folly::IOBuf& buffer) {
      this->dispatchTypedRequest(headerInfo, buffer);
    }

    void multiOpEnd() {}
    void parseError(carbon::Result, folly::StringPiece) {}

   private:
    Callback& callback_;
  };

  /**
   * Creates the client/server parser.
   *
   * @param callback  Callback function that will be called when a
   *                  request/reply is successfully parsed.
   */
  explicit ClientServerMcParser(Callback& callback)
      : replyCallback_(callback),
        requestCallback_(callback),
        replyParser_(std::make_unique<ClientMcParser<ReplyCallback>>(
            replyCallback_,
            kReadBufferSizeMin,
            kReadBufferSizeMax)),
        requestParser_(std::make_unique<ServerMcParser<RequestCallback>>(
            requestCallback_,
            kReadBufferSizeMin,
            kReadBufferSizeMax)),
        expectNextDispatcher_(replyParser_.get()) {}

  /**
   * Feed data into the parser. The callback will be called as soon
   * as a message is completely parsed.
   */
  void parse(folly::ByteRange data, uint32_t typeId, bool isFirstPacket);

  void reset() {
    replyParser_ = std::make_unique<ClientMcParser<ReplyCallback>>(
        replyCallback_,
        kReadBufferSizeMin,
        kReadBufferSizeMax,
        false /* useJemallocNodumpAllocator */,
        getCompressionCodecMap(
            *folly::EventBaseManager::get()->getEventBase()));
    expectNextDispatcher_.setReplyParser(replyParser_.get());

    requestParser_ = std::make_unique<ServerMcParser<RequestCallback>>(
        requestCallback_, kReadBufferSizeMin, kReadBufferSizeMax);
  }

  mc_protocol_t getProtocol() const {
    return protocol_;
  }

 private:
  ReplyCallback replyCallback_;
  RequestCallback requestCallback_;
  mc_protocol_t protocol_{mc_unknown_protocol};

  std::unique_ptr<ClientMcParser<ReplyCallback>> replyParser_;
  std::unique_ptr<ServerMcParser<RequestCallback>> requestParser_;

  detail::ExpectNextDispatcher<ClientMcParser<ReplyCallback>, RequestList>
      expectNextDispatcher_;
};
} // namespace memcache
} // namespace facebook

#include "ClientServerMcParser-inl.h"
