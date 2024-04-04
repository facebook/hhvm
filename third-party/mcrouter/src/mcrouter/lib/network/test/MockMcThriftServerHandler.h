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
    explicit ThriftContext(
        std::shared_ptr<apache::thrift::HandlerCallback<Reply>> callback)
        : callback_(std::move(callback)) {}

    static void reply(ThriftContext&& ctx, Reply&& reply) {
      ctx.callback_->result(std::move(reply));
      ctx.callback_.reset();
    }

   private:
    typename apache::thrift::HandlerCallback<Reply>::Ptr callback_;
  };

 public:
  void async_eb_mcGet(
      apache::thrift::HandlerCallback<facebook::memcache::McGetReply>::Ptr
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
      apache::thrift::HandlerCallback<facebook::memcache::McSetReply>::Ptr
          callback,
      const facebook::memcache::McSetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcDelete(
      apache::thrift::HandlerCallback<facebook::memcache::McDeleteReply>::Ptr
          callback,
      const facebook::memcache::McDeleteRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcLeaseGet(
      apache::thrift::HandlerCallback<facebook::memcache::McLeaseGetReply>::Ptr
          callback,
      const facebook::memcache::McLeaseGetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcLeaseSet(
      apache::thrift::HandlerCallback<facebook::memcache::McLeaseSetReply>::Ptr
          callback,
      const facebook::memcache::McLeaseSetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcAdd(
      apache::thrift::HandlerCallback<facebook::memcache::McAddReply>::Ptr
          callback,
      const facebook::memcache::McAddRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcReplace(
      apache::thrift::HandlerCallback<facebook::memcache::McReplaceReply>::Ptr
          callback,
      const facebook::memcache::McReplaceRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGets(
      apache::thrift::HandlerCallback<facebook::memcache::McGetsReply>::Ptr
          callback,
      const facebook::memcache::McGetsRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcCas(
      apache::thrift::HandlerCallback<facebook::memcache::McCasReply>::Ptr
          callback,
      const facebook::memcache::McCasRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcIncr(
      apache::thrift::HandlerCallback<facebook::memcache::McIncrReply>::Ptr
          callback,
      const facebook::memcache::McIncrRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcDecr(
      apache::thrift::HandlerCallback<facebook::memcache::McDecrReply>::Ptr
          callback,
      const facebook::memcache::McDecrRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcMetaget(
      apache::thrift::HandlerCallback<facebook::memcache::McMetagetReply>::Ptr
          callback,
      const facebook::memcache::McMetagetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcAppend(
      apache::thrift::HandlerCallback<facebook::memcache::McAppendReply>::Ptr
          callback,
      const facebook::memcache::McAppendRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcPrepend(
      apache::thrift::HandlerCallback<facebook::memcache::McPrependReply>::Ptr
          callback,
      const facebook::memcache::McPrependRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcTouch(
      apache::thrift::HandlerCallback<facebook::memcache::McTouchReply>::Ptr
          callback,
      const facebook::memcache::McTouchRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcFlushRe(
      apache::thrift::HandlerCallback<facebook::memcache::McFlushReReply>::Ptr
          callback,
      const facebook::memcache::McFlushReRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcFlushAll(
      apache::thrift::HandlerCallback<facebook::memcache::McFlushAllReply>::Ptr
          callback,
      const facebook::memcache::McFlushAllRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGat(
      apache::thrift::HandlerCallback<facebook::memcache::McGatReply>::Ptr
          callback,
      const facebook::memcache::McGatRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGats(
      apache::thrift::HandlerCallback<facebook::memcache::McGatsReply>::Ptr
          callback,
      const facebook::memcache::McGatsRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcVersion(
      apache::thrift::HandlerCallback<facebook::memcache::McVersionReply>::Ptr
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
