/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheServer.h"
#include "mcrouter/lib/network/gen/gen-cpp2/Memcache.h"
#include "mcrouter/lib/network/test/MockMc.h"
#include "mcrouter/lib/network/test/MockMcOnRequest.h"

namespace facebook {
namespace memcache {
namespace test {

class MockMcThriftServerHandler
    : public facebook::memcache::thrift::MemcacheSvIf {
 private:
  template <class Reply>
  class ThriftContext {
   public:
    explicit ThriftContext(apache::thrift::HandlerCallbackPtr<Reply> callback)
        : callback_(std::move(callback)) {}

    static void reply(ThriftContext&& ctx, Reply&& reply) {
      ctx.callback_->result(std::move(reply));
      ctx.callback_.reset();
    }

   private:
    apache::thrift::HandlerCallbackPtr<Reply> callback_;
  };

 public:
  void async_eb_mcGet(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McGetReply>
          callback,
      const facebook::memcache::McGetRequest& request) override final {
    auto key = request.key_ref()->fullKey();
    if (key == "__mockmc__.want_load_shedding") {
      callback->appOverloadedException("load shedding");
      return;
    }
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcSet(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McSetReply>
          callback,
      const facebook::memcache::McSetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcDelete(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McDeleteReply>
          callback,
      const facebook::memcache::McDeleteRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcLeaseGet(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McLeaseGetReply>
          callback,
      const facebook::memcache::McLeaseGetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcLeaseSet(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McLeaseSetReply>
          callback,
      const facebook::memcache::McLeaseSetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcAdd(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McAddReply>
          callback,
      const facebook::memcache::McAddRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcReplace(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McReplaceReply>
          callback,
      const facebook::memcache::McReplaceRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGets(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McGetsReply>
          callback,
      const facebook::memcache::McGetsRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcCas(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McCasReply>
          callback,
      const facebook::memcache::McCasRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcIncr(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McIncrReply>
          callback,
      const facebook::memcache::McIncrRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcDecr(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McDecrReply>
          callback,
      const facebook::memcache::McDecrRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcMetaget(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McMetagetReply>
          callback,
      const facebook::memcache::McMetagetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcAppend(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McAppendReply>
          callback,
      const facebook::memcache::McAppendRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcPrepend(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McPrependReply>
          callback,
      const facebook::memcache::McPrependRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcTouch(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McTouchReply>
          callback,
      const facebook::memcache::McTouchRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcFlushRe(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McFlushReReply>
          callback,
      const facebook::memcache::McFlushReRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcFlushAll(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McFlushAllReply>
          callback,
      const facebook::memcache::McFlushAllRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGat(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McGatReply>
          callback,
      const facebook::memcache::McGatRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGats(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McGatsReply>
          callback,
      const facebook::memcache::McGatsRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcVersion(
      apache::thrift::HandlerCallbackPtr<facebook::memcache::McVersionReply>
          callback,
      const facebook::memcache::McVersionRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

 private:
  MockMcOnRequest onRequest_;
};

} // namespace test
} // namespace memcache
} // namespace facebook
