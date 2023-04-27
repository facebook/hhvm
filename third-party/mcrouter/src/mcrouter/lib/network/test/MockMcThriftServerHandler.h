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
        std::unique_ptr<apache::thrift::HandlerCallback<Reply>> callback)
        : callback_(std::move(callback)) {}

    static void reply(ThriftContext&& ctx, Reply&& reply) {
      ctx.callback_->result(std::move(reply));
      ctx.callback_.reset();
    }

   private:
    std::unique_ptr<apache::thrift::HandlerCallback<Reply>> callback_;
  };

 public:
  void async_eb_mcGet(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McGetReply>> callback,
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
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McSetReply>> callback,
      const facebook::memcache::McSetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcDelete(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McDeleteReply>> callback,
      const facebook::memcache::McDeleteRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcLeaseGet(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McLeaseGetReply>> callback,
      const facebook::memcache::McLeaseGetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcLeaseSet(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McLeaseSetReply>> callback,
      const facebook::memcache::McLeaseSetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcAdd(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McAddReply>> callback,
      const facebook::memcache::McAddRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcReplace(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McReplaceReply>> callback,
      const facebook::memcache::McReplaceRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGets(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McGetsReply>> callback,
      const facebook::memcache::McGetsRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcCas(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McCasReply>> callback,
      const facebook::memcache::McCasRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcIncr(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McIncrReply>> callback,
      const facebook::memcache::McIncrRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcDecr(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McDecrReply>> callback,
      const facebook::memcache::McDecrRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcMetaget(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McMetagetReply>> callback,
      const facebook::memcache::McMetagetRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcAppend(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McAppendReply>> callback,
      const facebook::memcache::McAppendRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcPrepend(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McPrependReply>> callback,
      const facebook::memcache::McPrependRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcTouch(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McTouchReply>> callback,
      const facebook::memcache::McTouchRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcFlushRe(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McFlushReReply>> callback,
      const facebook::memcache::McFlushReRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcFlushAll(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McFlushAllReply>> callback,
      const facebook::memcache::McFlushAllRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGat(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McGatReply>> callback,
      const facebook::memcache::McGatRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcGats(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McGatsReply>> callback,
      const facebook::memcache::McGatsRequest& request) override final {
    auto reqCopy = request;
    onRequest_.onRequest(
        ThriftContext(std::move(callback)), std::move(reqCopy));
  }

  virtual void async_eb_mcVersion(
      std::unique_ptr<apache::thrift::HandlerCallback<
          facebook::memcache::McVersionReply>> callback,
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
