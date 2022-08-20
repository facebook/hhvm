/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <folly/GroupVarint.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/fbi/cpp/TypeList.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/CaretProtocol.h"
#include "mcrouter/lib/network/McServerRequestContext.h"
#include "mcrouter/lib/network/TypedMsg.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheServer.h"
#include "mcrouter/lib/network/test/ClientSocket.h"
#include "mcrouter/lib/network/test/ListenSocket.h"
#include "mcrouter/lib/network/test/MockMc.h"

using namespace facebook::memcache;

namespace {
struct TypedMockMcOnRequest {
  MockMc& mc_;

  explicit TypedMockMcOnRequest(MockMc& mc) : mc_(mc) {}

  void onRequest(McServerRequestContext&& ctx, McGetRequest&& req) {
    auto item = mc_.get(req.key_ref()->fullKey());
    McGetReply reply;
    if (!item) {
      reply.result_ref() = carbon::Result::NOTFOUND;
    } else {
      reply.result_ref() = carbon::Result::FOUND;
      reply.value_ref() = *item->value;
      reply.flags_ref() = item->flags;
    }
    McServerRequestContext::reply(std::move(ctx), std::move(reply));
  }

  void onRequest(McServerRequestContext&& ctx, McSetRequest&& req) {
    mc_.set(
        req.key_ref()->fullKey(),
        MockMc::Item(*req.value_ref(), *req.exptime_ref(), *req.flags_ref()));
    McSetReply reply(carbon::Result::STORED);
    McServerRequestContext::reply(std::move(ctx), std::move(reply));
  }

  void onRequest(McServerRequestContext&& ctx, McDeleteRequest&& req) {
    McDeleteReply reply;
    if (mc_.del(req.key_ref()->fullKey())) {
      reply.result_ref() = carbon::Result::DELETED;
    } else {
      reply.result_ref() = carbon::Result::NOTFOUND;
    }

    McServerRequestContext::reply(std::move(ctx), std::move(reply));
  }

  template <class Request>
  void onRequest(McServerRequestContext&& ctx, Request&&) {
    /* non-typed requests not supported */
    McServerRequestContext::reply(
        std::move(ctx), ReplyT<Request>(carbon::Result::CLIENT_ERROR));
  }
};
} // namespace

TEST(CarbonMockMc, basic) {
  ListenSocket listenSock;

  AsyncMcServer::Options opts;
  opts.existingSocketFds = {listenSock.releaseSocketFd()};
  opts.numThreads = 1;

  MockMc mc;

  mc.set("key", MockMc::Item(folly::IOBuf::wrapBuffer("value", 5)));

  AsyncMcServer server(opts);
  server.spawn(
      [&mc](size_t, folly::EventBase& evb, AsyncMcServerWorker& worker) {
        worker.setOnRequest(MemcacheRequestHandler<TypedMockMcOnRequest>(mc));
        evb.loop();
      });

  ClientSocket clientSock(listenSock.getPort());

  McGetRequest getReq("key");

  carbon::CarbonQueueAppenderStorage storage;
  carbon::CarbonProtocolWriter writer(storage);
  getReq.serialize(writer);

  CaretMessageInfo requestInfo;
  requestInfo.bodySize = storage.computeBodySize();
  requestInfo.typeId = 1;
  requestInfo.reqId = 100;
  requestInfo.traceId = {0, 0};
  requestInfo.headerSize = caretPrepareHeader(
      requestInfo, reinterpret_cast<char*>(storage.getHeaderBuf()));
  storage.reportHeaderSize(requestInfo.headerSize);

  const size_t totalSize = requestInfo.headerSize + requestInfo.bodySize;
  folly::IOBuf iobuf(folly::IOBuf::CREATE, totalSize);

  const auto iovs = storage.getIovecs();
  for (size_t i = 0; i < iovs.second; ++i) {
    const auto* iov = iovs.first + i;
    std::memcpy(iobuf.writableTail(), iov->iov_base, iov->iov_len);
    iobuf.append(iov->iov_len);
  }
  auto dataSp = getRange(iobuf);
  auto reply = clientSock.sendRequest(dataSp, 16);
  EXPECT_EQ('^', reply[0]);

  CaretMessageInfo replyInfo;
  caretParseHeader((uint8_t*)reply.data(), reply.size(), replyInfo);
  EXPECT_EQ(100, replyInfo.reqId);
  EXPECT_EQ(2, replyInfo.typeId);
  auto readBuf = folly::IOBuf::wrapBuffer(
      reply.data() + replyInfo.headerSize, replyInfo.bodySize);
  carbon::CarbonProtocolReader reader(carbon::CarbonCursor(readBuf.get()));
  McGetReply getReply;
  getReply.deserialize(reader);

  EXPECT_EQ(carbon::Result::FOUND, *getReply.result_ref());

  const auto resultVal = carbon::valueRangeSlow(getReply);
  EXPECT_EQ("value", resultVal.str());

  server.shutdown();
  server.join();
}
