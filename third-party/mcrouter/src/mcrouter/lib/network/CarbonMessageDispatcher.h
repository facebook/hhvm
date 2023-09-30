/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/carbon/Artillery.h"
#include "mcrouter/lib/carbon/CarbonProtocolReader.h"
#include "mcrouter/lib/carbon/CarbonProtocolWriter.h"
#include "mcrouter/lib/carbon/CarbonQueueAppender.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/network/CaretHeader.h"
#include "mcrouter/lib/network/ConnectionOptions.h"
#include "mcrouter/lib/network/TypedMsg.h"

namespace facebook {
namespace memcache {

class McServerRequestContext;

/*
 * Takes a Carbon struct and serializes it to an IOBuf
 * @param msg: The typed structure to serialize
 * @return a unique pointer to the IOBuf
 */
template <class Message>
void serializeCarbonStruct(
    const Message& msg,
    carbon::CarbonQueueAppenderStorage& storage) {
  carbon::CarbonProtocolWriter writer(storage);
  msg.serialize(writer);
}

template <class Request>
void serializeCarbonRequest(
    const Request& req,
    carbon::CarbonQueueAppenderStorage& storage) {
  if (!req.isBufferDirty()) {
    const auto& buf = *req.serializedBuffer();
    if (FOLLY_LIKELY(storage.setFullBuffer(buf))) {
      return;
    }
  }
  serializeCarbonStruct(req, storage);
}

/**
 * A dispatcher for binary protol serialized Carbon structs.
 *
 * Given a type id and an IOBuf, unserializes the corresponding Carbon struct
 * and calls Proc::onTypedMessage(headerInfo, reqBuffer, M&&, args...)
 *
 * @param MessageList  List of supported Carbon messages: List<M, ...>
 *                     All Ms in the list must be Carbon struct types.
 * @param Proc         Derived processor class, may provide
 *                       void onTypedMessage(
 *                          const CaretMessageInfo& headerInfo,
 *                          const folly::IOBuf& reqBuffer,
 *                          M&& msg,
 *                          args...).
 *                     If not provided, default implementation that forwards to
 *                       void onRequest(McServerRequestContext&& context,
 *                                      M&& req,
 *                                      const CaretMessageInfo& headerInfo,
 *                                      const folly::IOBuf& reqBuffer);
 *                     will be used.
 *                     Overloaded for every Carbon struct in MessageList.
 * @param Args         Additional arguments to pass through to onTypedMessage.
 *
 * WARNING: Using CarbonMsgDispatcher with multiple inheritance is not
 *          recommended.
 */
template <class MessageList, class Proc, class... Args>
class CarbonMessageDispatcher {
 public:
  /**
   * @return true iff headerInfo.typeId corresponds to a message in MessageList
   */
  bool dispatchTypedRequest(
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& buffer,
      Args&&... args) {
    return dispatcher_.dispatch(
        headerInfo.typeId,
        *this,
        headerInfo,
        buffer,
        std::forward<Args>(args)...);
  }

  // Default onTypedMessage() implementation
  template <class M>
  void onTypedMessage(
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& reqBuffer,
      M&& req,
      McServerRequestContext&& ctx) {
    static_cast<Proc&>(*this).onRequest(
        std::move(ctx), std::move(req), headerInfo, reqBuffer);
  }

  template <class M>
  static void processMsg(
      CarbonMessageDispatcher& me,
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& reqBuf,
      Args&&... args) {
    folly::io::Cursor cur(&reqBuf);
    cur += headerInfo.headerSize;
    carbon::CarbonProtocolReader reader(cur);
    M req;

    req.setTraceContext(
        carbon::tracing::deserializeTraceContext(headerInfo.traceId));

    req.deserialize(reader);
    static_cast<Proc&>(me).onTypedMessage(
        headerInfo, reqBuf, std::move(req), std::forward<Args>(args)...);
  }

 private:
  CallDispatcher<
      MessageList,
      CarbonMessageDispatcher,
      const CaretMessageInfo&,
      const folly::IOBuf&,
      Args...>
      dispatcher_;
};
} // namespace memcache
} // namespace facebook
