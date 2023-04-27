/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/StatsReply.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/test/MockMc.h"

namespace facebook {
namespace memcache {

static std::atomic_uint64_t cmd_get_count{0};

class MockMcOnRequest {
 public:
  template <class Context>
  void onRequest(Context&& ctx, McMetagetRequest&& req) {
    using Reply = McMetagetReply;

    auto key = req.key_ref()->fullKey().str();

    auto item = mc_.get(key);
    if (!item) {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
      return;
    }

    Reply reply(carbon::Result::FOUND);
    reply.exptime_ref() = item->exptime;
    if (key == "unknown_age") {
      reply.age_ref() = -1;
    } else {
      reply.age_ref() = time(nullptr) - item->creationTime;
    }
    reply.ipAddress_ref() = "127.0.0.1";
    reply.ipv_ref() = 4;

    Context::reply(std::move(ctx), std::move(reply));
  }

  template <class Context>
  void onRequest(Context&& ctx, McGetRequest&& req) {
    using Reply = McGetReply;
    cmd_get_count++;

    auto key = req.key_ref()->fullKey();

    if (key == "__mockmc__.want_busy") {
      Reply reply(carbon::Result::BUSY);
      reply.appSpecificErrorCode_ref() = SERVER_ERROR_BUSY;
      reply.message_ref() = "busy";
      Context::reply(std::move(ctx), std::move(reply));
      return;
    } else if (key == "__mockmc__.want_try_again") {
      Context::reply(std::move(ctx), Reply(carbon::Result::RES_TRY_AGAIN));
      return;
    } else if (key.startsWith("__mockmc__.want_timeout")) {
      size_t timeout = 500;
      auto argStart = key.find('(');
      if (argStart != std::string::npos) {
        timeout = folly::to<size_t>(
            key.subpiece(argStart + 1, key.size() - argStart - 2));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
      Context::reply(std::move(ctx), Reply(carbon::Result::TIMEOUT));
      return;
    }

    auto item = mc_.get(key);
    if (!item) {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    } else {
      Reply reply(carbon::Result::FOUND);
      reply.value_ref() = item->value->cloneAsValue();
      reply.flags_ref() = item->flags;
      Context::reply(std::move(ctx), std::move(reply));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McGatRequest&& req) {
    using Reply = McGatReply;

    auto key = req.key_ref()->fullKey();

    if (key == "__mockmc__.want_busy") {
      Reply reply(carbon::Result::BUSY);
      reply.appSpecificErrorCode_ref() = SERVER_ERROR_BUSY;
      reply.message_ref() = "busy";
      Context::reply(std::move(ctx), std::move(reply));
      return;
    } else if (key == "__mockmc__.want_try_again") {
      Context::reply(std::move(ctx), Reply(carbon::Result::RES_TRY_AGAIN));
      return;
    } else if (key.startsWith("__mockmc__.want_timeout")) {
      size_t timeout = 500;
      auto argStart = key.find('(');
      if (argStart != std::string::npos) {
        timeout = folly::to<size_t>(
            key.subpiece(argStart + 1, key.size() - argStart - 2));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
      Context::reply(std::move(ctx), Reply(carbon::Result::TIMEOUT));
      return;
    }

    auto item = mc_.gat(*req.exptime_ref(), key);
    if (!item) {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    } else {
      Reply reply(carbon::Result::FOUND);
      reply.value_ref() = item->value->cloneAsValue();
      reply.flags_ref() = item->flags;
      Context::reply(std::move(ctx), std::move(reply));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McLeaseGetRequest&& req) {
    using Reply = McLeaseGetReply;

    auto key = req.key_ref()->fullKey().str();

    auto out = mc_.leaseGet(key);
    Reply reply(carbon::Result::FOUND);
    reply.value_ref() = out.first->value->cloneAsValue();
    reply.leaseToken_ref() = out.second;
    reply.flags_ref() = out.first->flags;
    if (out.second) {
      reply.result_ref() = carbon::Result::NOTFOUND;
    }
    Context::reply(std::move(ctx), std::move(reply));
  }

  template <class Context>
  void onRequest(Context&& ctx, McLeaseSetRequest&& req) {
    using Reply = McLeaseSetReply;

    auto key = req.key_ref()->fullKey().str();

    switch (mc_.leaseSet(key, MockMc::Item(req), *req.leaseToken_ref())) {
      case MockMc::LeaseSetResult::NOT_STORED:
        Context::reply(std::move(ctx), Reply(carbon::Result::NOTSTORED));
        return;

      case MockMc::LeaseSetResult::STORED:
        Context::reply(std::move(ctx), Reply(carbon::Result::STORED));
        return;

      case MockMc::LeaseSetResult::STALE_STORED:
        Context::reply(std::move(ctx), Reply(carbon::Result::STALESTORED));
        return;
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McSetRequest&& req) {
    McSetReply reply;
    auto key = req.key_ref()->fullKey().str();
    if (key == "__mockmc__.trigger_server_error") {
      reply.result_ref() = carbon::Result::REMOTE_ERROR;
      reply.message_ref() = "returned error msg with binary data \xdd\xab";
    } else {
      mc_.set(key, MockMc::Item(req));
      reply.result_ref() = carbon::Result::STORED;
    }

    Context::reply(std::move(ctx), std::move(reply));
  }

  template <class Context>
  void onRequest(Context&& ctx, McAddRequest&& req) {
    using Reply = McAddReply;

    auto key = req.key_ref()->fullKey().str();

    if (mc_.add(key, MockMc::Item(req))) {
      Context::reply(std::move(ctx), Reply(carbon::Result::STORED));
    } else {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTSTORED));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McReplaceRequest&& req) {
    using Reply = McReplaceReply;

    auto key = req.key_ref()->fullKey().str();

    if (mc_.replace(key, MockMc::Item(req))) {
      Context::reply(std::move(ctx), Reply(carbon::Result::STORED));
    } else {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTSTORED));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McAppendRequest&& req) {
    using Reply = McAppendReply;

    auto key = req.key_ref()->fullKey().str();

    if (mc_.append(key, MockMc::Item(req))) {
      Context::reply(std::move(ctx), Reply(carbon::Result::STORED));
    } else {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTSTORED));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McPrependRequest&& req) {
    using Reply = McPrependReply;

    auto key = req.key_ref()->fullKey().str();

    if (mc_.prepend(key, MockMc::Item(req))) {
      Context::reply(std::move(ctx), Reply(carbon::Result::STORED));
    } else {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTSTORED));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McDeleteRequest&& req) {
    using Reply = McDeleteReply;

    auto key = req.key_ref()->fullKey().str();

    if (mc_.del(key)) {
      Context::reply(std::move(ctx), Reply(carbon::Result::DELETED));
    } else {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McTouchRequest&& req) {
    using Reply = McTouchReply;

    auto key = req.key_ref()->fullKey().str();

    if (mc_.touch(key, *req.exptime_ref())) {
      Context::reply(std::move(ctx), Reply(carbon::Result::TOUCHED));
    } else {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McIncrRequest&& req) {
    using Reply = McIncrReply;

    auto key = req.key_ref()->fullKey().str();
    auto p = mc_.arith(key, *req.delta_ref());
    if (!p.first) {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    } else {
      Reply reply(carbon::Result::STORED);
      reply.delta_ref() = p.second;
      Context::reply(std::move(ctx), std::move(reply));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McDecrRequest&& req) {
    using Reply = McDecrReply;

    auto key = req.key_ref()->fullKey().str();
    auto p = mc_.arith(key, -req.delta_ref().value());
    if (!p.first) {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    } else {
      Reply reply(carbon::Result::STORED);
      reply.delta_ref() = p.second;
      Context::reply(std::move(ctx), std::move(reply));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McFlushAllRequest&& req) {
    using Reply = McFlushAllReply;

    std::this_thread::sleep_for(std::chrono::seconds(*req.delay_ref()));
    mc_.flushAll();
    Context::reply(std::move(ctx), Reply(carbon::Result::OK));
  }

  template <class Context>
  void onRequest(Context&& ctx, McGetsRequest&& req) {
    using Reply = McGetsReply;

    auto key = req.key_ref()->fullKey().str();
    auto p = mc_.gets(key);
    if (!p.first) {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    } else {
      Reply reply(carbon::Result::FOUND);
      reply.value_ref() = p.first->value->cloneAsValue();
      reply.flags_ref() = p.first->flags;
      reply.casToken_ref() = p.second;
      Context::reply(std::move(ctx), std::move(reply));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McGatsRequest&& req) {
    using Reply = McGatsReply;

    auto key = req.key_ref()->fullKey().str();
    auto p = mc_.gats(*req.exptime_ref(), key);
    if (!p.first) {
      Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
    } else {
      Reply reply(carbon::Result::FOUND);
      reply.value_ref() = p.first->value->cloneAsValue();
      reply.flags_ref() = p.first->flags;
      reply.casToken_ref() = p.second;
      Context::reply(std::move(ctx), std::move(reply));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McCasRequest&& req) {
    using Reply = McCasReply;

    auto key = req.key_ref()->fullKey().str();
    auto ret = mc_.cas(key, MockMc::Item(req), *req.casToken_ref());
    switch (ret) {
      case MockMc::CasResult::NOT_FOUND:
        Context::reply(std::move(ctx), Reply(carbon::Result::NOTFOUND));
        break;
      case MockMc::CasResult::EXISTS:
        Context::reply(std::move(ctx), Reply(carbon::Result::EXISTS));
        break;
      case MockMc::CasResult::STORED:
        Context::reply(std::move(ctx), Reply(carbon::Result::STORED));
        break;
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McStatsRequest&& req) {
    using Reply = McStatsReply;

    auto key = req.key_ref()->fullKey().str();
    if (key == "__mockmc__") {
      StatsReply stats;
      stats.addStat("cmd_get_count", cmd_get_count.load());
      Context::reply(std::move(ctx), stats.getReply());
    } else {
      Context::reply(std::move(ctx), Reply(carbon::Result::BAD_COMMAND));
    }
  }

  template <class Context>
  void onRequest(Context&& ctx, McVersionRequest&& /*req*/) {
    McVersionReply reply(carbon::Result::OK);
    reply.value_ref() =
        folly::IOBuf(folly::IOBuf::COPY_BUFFER, "TestServer-1.0");
    Context::reply(std::forward<Context>(ctx), std::move(reply));
  }

  template <class Context, class Unsupported>
  void onRequest(Context&& ctx, Unsupported&&) {
    const std::string errorMessage = folly::sformat(
        "MockMcServer does not support {}", typeid(Unsupported).name());
    LOG(ERROR) << errorMessage;
    ReplyT<Unsupported> reply(carbon::Result::REMOTE_ERROR);
    reply.message_ref() = std::move(errorMessage);
    Context::reply(std::move(ctx), std::move(reply));
  }

 private:
  MockMc mc_;
};

} // namespace memcache
} // namespace facebook
