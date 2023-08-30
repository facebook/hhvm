/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <functional>
#include <memory>
#include <mutex>
#include <utility>

#include <folly/ScopeGuard.h>
#include <folly/fibers/FiberManager.h>
#include <folly/synchronization/Baton.h>

#include "mcrouter/config.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/carbon/ExternalCarbonConnectionStats.h"
#include "mcrouter/lib/network/ConnectionOptions.h"
#include "mcrouter/lib/network/Transport.h"

namespace carbon {
namespace detail {

DECLARE_int32(cacheclient_external_connection_threads);
using ExternalConnectionLogger =
    facebook::memcache::mcrouter::AdditionalExternalConnectionLogger;

class ClientBase {
 public:
  ClientBase(
      facebook::memcache::ConnectionOptions connOpts,
      ExternalCarbonConnectionImplOptions opts);

  virtual ~ClientBase() {}

  void closeNow();

  size_t limitRequests(size_t requestsCount);

  virtual facebook::memcache::Transport& getClient() = 0;

 protected:
  const facebook::memcache::ConnectionOptions connectionOptions;
  const ExternalCarbonConnectionImplOptions options;
  counting_sem_t outstandingReqsSem;
};

template <class Transport>
class Client : public ClientBase {
 public:
  Client(
      facebook::memcache::ConnectionOptions connOpts,
      ExternalCarbonConnectionImplOptions opts,
      std::shared_ptr<ExternalConnectionLogger> logger)
      : ClientBase(std::move(connOpts), std::move(opts)),
        client_(
            folly::EventBaseManager::get()
                ->getEventBase()
                ->getVirtualEventBase(),
            connectionOptions),
        logger_(logger) {}

  virtual ~Client() override {
    closeNow();
  }

  template <class Request>
  facebook::memcache::ReplyT<Request> sendRequest(const Request& request) {
    SCOPE_EXIT {
      // New requests are scheduled as remote tasks, so we won't create
      // fibers for them, until this fiber is complete anyway.
      counting_sem_post(&outstandingReqsSem, 1);
    };
    auto reply = client_.sendSync(request, connectionOptions.writeTimeout);
    if (logger_ && logger_->shouldLog()) {
      folly::fibers::runInMainContext([&]() {
        auto stats = statsFrom(request, reply);
        logger_->log(stats);
      });
    }
    return reply;
  }

  facebook::memcache::Transport& getClient() override {
    return client_;
  }

  template <class Request>
  ExternalCarbonConnectionStats statsFrom(
      Request& req,
      facebook::memcache::ReplyT<Request>& reply) {
    ExternalCarbonConnectionStats item;
    item.key = carbon::getFullKey(req);
    item.reqType = Request::name;
    item.reqSizeBytes = carbon::valuePtrUnsafe(req)
        ? carbon::valuePtrUnsafe(req)->computeChainDataLength()
        : 0;
    auto accessPoint = connectionOptions.accessPoint;
    item.destHost = accessPoint->toHostPortString();
    item.protocol = mc_protocol_to_string(accessPoint->getProtocol());
    item.securityMechanism =
        securityMechToString(accessPoint->getSecurityMech());
    item.port = accessPoint->getPort();
    item.respSizeBytes = carbon::valuePtrUnsafe(reply)
        ? carbon::valuePtrUnsafe(reply)->computeChainDataLength()
        : 0;
    item.resultCode = carbon::resultToString(*reply.result_ref());
    return item;
  }

 private:
  Transport client_;
  std::shared_ptr<ExternalConnectionLogger> logger_;
};

class ThreadInfo {
 public:
  ThreadInfo();

  template <class Transport>
  std::weak_ptr<Client<Transport>> createClient(
      facebook::memcache::ConnectionOptions connectionOptions,
      ExternalCarbonConnectionImplOptions options,
      std::shared_ptr<ExternalConnectionLogger> logger) {
    return folly::fibers::await(
        [&](folly::fibers::Promise<std::weak_ptr<Client<Transport>>> p) {
          fiberManager_.addTaskRemote(
              [this,
               promise = std::move(p),
               connectionOptions = std::move(connectionOptions),
               options,
               logger]() mutable {
                auto client = std::make_shared<Client<Transport>>(
                    connectionOptions, options, logger);
                clients_.insert(client);
                promise.setValue(client);
              });
        });
  }

  void releaseClient(std::weak_ptr<ClientBase> clientWeak) {
    fiberManager_.addTaskRemote([this, clientWeak] {
      if (auto client = clientWeak.lock()) {
        clients_.erase(client);
        client->closeNow();
      }
    });
  }

  template <typename F>
  void addTaskRemote(F&& f) {
    fiberManager_.addTaskRemote(std::forward<F>(f));
  }

  ~ThreadInfo();

 private:
  std::thread thread_;
  folly::fibers::FiberManager fiberManager_;
  std::unordered_set<std::shared_ptr<ClientBase>> clients_;
};

// Pool of threads, each of them has its own EventBase and FiberManager.
class ThreadPool : public std::enable_shared_from_this<ThreadPool> {
 public:
  ThreadPool() {
    threads_.reserve(FLAGS_cacheclient_external_connection_threads);
  }

  template <class Transport>
  std::pair<
      std::weak_ptr<detail::Client<Transport>>,
      std::weak_ptr<detail::ThreadInfo>>
  createClient(
      facebook::memcache::ConnectionOptions connectionOptions,
      ExternalCarbonConnectionImplOptions options) {
    auto& threadInfo = [&]() -> detail::ThreadInfo& {
      std::lock_guard<std::mutex> lck(mutex_);
      // Select a thread in round robin fashion.
      size_t threadId = (nextThreadId_++) % getThreadNum();

      // If it's not running yet, then we need to start it.
      if (threads_.size() <= threadId) {
        threads_.emplace_back(std::make_unique<detail::ThreadInfo>());
      }
      return *threads_[threadId];
    }();

    auto logger = options.enableLogging ? getLogger(options) : nullptr;

    std::weak_ptr<detail::Client<Transport>> clientPtr =
        threadInfo.template createClient<Transport>(
            connectionOptions, options, logger);

    std::shared_ptr<detail::ThreadInfo> threadInfoShared(
        shared_from_this(), &threadInfo);

    return std::make_pair(
        std::move(clientPtr),
        std::weak_ptr<detail::ThreadInfo>(std::move(threadInfoShared)));
  }

  std::shared_ptr<ExternalConnectionLogger> getLogger(
      ExternalCarbonConnectionImplOptions& opts) {
    if (!logger_) {
      createLogger(opts);
    } else if (opts.enableLogging != logger_->getEnabledStatus()) {
      logger_->setEnabledStatus(opts.enableLogging);
    }
    return logger_;
  }

  static std::shared_ptr<ThreadPool> getInstance();

  int getThreadNum();

 private:
  std::mutex mutex_;
  std::mutex loggerMtx_;
  size_t nextThreadId_{0};
  std::vector<std::unique_ptr<detail::ThreadInfo>> threads_;
  std::shared_ptr<ExternalConnectionLogger> logger_ = nullptr;

  void createLogger(ExternalCarbonConnectionImplOptions& opts) {
    std::lock_guard<std::mutex> lk(loggerMtx_);
    // Verify logger still doesn't exist
    if (!logger_) {
      ExternalCarbonConnectionLoggerOptions loggerOptions =
          ExternalCarbonConnectionLoggerOptions(
              opts.enableLogging,
              opts.logSampleRate,
              0,
              opts.hourlyLogRate,
              opts.maxLogBurstSize);
      logger_ = std::make_shared<ExternalConnectionLogger>(loggerOptions);
    }
  }
};

} // namespace detail

template <class Transport>
class Impl {
 public:
  Impl(
      facebook::memcache::ConnectionOptions connectionOptions,
      ExternalCarbonConnectionImplOptions options) {
    auto pool = detail::ThreadPool::getInstance();

    auto info = pool->createClient<Transport>(connectionOptions, options);
    client_ = info.first;
    threadInfo_ = info.second;
  }

  ~Impl() {
    if (auto threadInfo = threadInfo_.lock()) {
      threadInfo->releaseClient(client_);
    }
  }

  bool healthCheck() {
    folly::fibers::Baton baton;
    bool ret = false;

    auto clientWeak = client_;
    auto threadInfo = threadInfo_.lock();
    if (!threadInfo) {
      throw CarbonConnectionRecreateException(
          "Singleton<ThreadPool> was destroyed!");
    }

    threadInfo->addTaskRemote([clientWeak, &baton, &ret]() {
      auto client = clientWeak.lock();
      if (!client) {
        baton.post();
        return;
      }

      auto reply = client->sendRequest(facebook::memcache::McVersionRequest());
      ret = !facebook::memcache::isErrorResult(*reply.result_ref());
      baton.post();
    });

    baton.wait();
    return ret;
  }

  template <class Request, class F>
  void sendRequestOne(const Request& req, F&& f) {
    auto threadInfo = threadInfo_.lock();
    if (!threadInfo) {
      throw CarbonConnectionRecreateException(
          "Singleton<ThreadPool> was destroyed!");
    }

    auto client = client_.lock();
    assert(client);

    if (client->limitRequests(1) == 0) {
      f(req, facebook::memcache::ReplyT<Request>(carbon::Result::LOCAL_ERROR));
      return;
    }

    threadInfo->addTaskRemote(
        [clientWeak = client_, &req, f = std::forward<F>(f)]() mutable {
          auto cl = clientWeak.lock();
          if (!cl) {
            folly::fibers::runInMainContext([&req,
                                             f = std::forward<F>(f)]() mutable {
              f(req,
                facebook::memcache::ReplyT<Request>(carbon::Result::UNKNOWN));
            });
            return;
          }

          auto reply = cl->sendRequest(req);
          folly::fibers::runInMainContext(
              [&req, f = std::forward<F>(f), &reply]() mutable {
                f(req, std::move(reply));
              });
        });
  }

  template <class Request, class F>
  void sendRequestMulti(
      std::vector<std::reference_wrapper<const Request>>&& reqs,
      F&& f) {
    auto threadInfo = threadInfo_.lock();
    if (!threadInfo) {
      throw CarbonConnectionRecreateException(
          "Singleton<ThreadPool> was destroyed!");
    }

    auto cl = client_.lock();
    assert(cl);

    auto ctx =
        std::make_shared<std::vector<std::reference_wrapper<const Request>>>(
            std::move(reqs));

    for (size_t i = 0; i < ctx->size();) {
      auto num = cl->limitRequests(ctx->size() - i);

      if (num == 0) {
        // Hit outstanding limit.
        for (; i < ctx->size(); ++i) {
          f((*ctx)[i].get(),
            facebook::memcache::ReplyT<Request>(carbon::Result::LOCAL_ERROR));
        }
        break;
      }

      threadInfo->addTaskRemote([clientWeak = client_,
                                 ctx,
                                 i,
                                 num,
                                 f]() mutable {
        auto client = clientWeak.lock();
        if (!client) {
          folly::fibers::runInMainContext([&ctx, i, num, &f]() mutable {
            for (size_t cnt = 0; cnt < num; ++cnt, ++i) {
              f((*ctx)[i].get(),
                facebook::memcache::ReplyT<Request>(carbon::Result::UNKNOWN));
            }
          });
          return;
        }

        for (size_t cnt = 0; cnt + 1 < num; ++cnt, ++i) {
          const Request& req = (*ctx)[i];
          folly::fibers::addTaskFinally(
              [clientWeak, &req] {
                if (auto c = clientWeak.lock()) {
                  return c->sendRequest(req);
                }
                return facebook::memcache::ReplyT<Request>(
                    carbon::Result::UNKNOWN);
              },
              [f, &req](
                  folly::Try<facebook::memcache::ReplyT<Request>>&& r) mutable {
                f(req, std::move(r.value()));
              });
        }

        // Send last request in a batch on this fiber.
        const auto& req = (*ctx)[i].get();
        auto reply = client->sendRequest(req);
        folly::fibers::runInMainContext(
            [&req, &f, &reply]() mutable { f(req, std::move(reply)); });
      });

      i += num;
    }
  }

 private:
  std::weak_ptr<detail::ThreadInfo> threadInfo_;
  std::weak_ptr<detail::Client<Transport>> client_;
};

template <class RouterInfo>
template <class Request>
void ExternalCarbonConnectionImpl<RouterInfo>::sendRequestOne(
    const Request& req,
    RequestCb<Request> cb) {
  try {
    thriftImpl_ ? thriftImpl_->sendRequestOne(req, std::move(cb))
                : carbonImpl_->sendRequestOne(req, std::move(cb));
  } catch (const CarbonConnectionRecreateException&) {
    makeImpl();
    thriftImpl_ ? thriftImpl_->sendRequestOne(req, std::move(cb))
                : carbonImpl_->sendRequestOne(req, std::move(cb));
  }
}

template <class RouterInfo>
template <class Request>
void ExternalCarbonConnectionImpl<RouterInfo>::sendRequestMulti(
    std::vector<std::reference_wrapper<const Request>>&& reqs,
    RequestCb<Request> cb) {
  try {
    thriftImpl_ ? thriftImpl_->sendRequestMulti(std::move(reqs), std::move(cb))
                : carbonImpl_->sendRequestMulti(std::move(reqs), std::move(cb));
  } catch (const CarbonConnectionRecreateException&) {
    makeImpl();
    thriftImpl_ ? thriftImpl_->sendRequestMulti(std::move(reqs), std::move(cb))
                : carbonImpl_->sendRequestMulti(std::move(reqs), std::move(cb));
  }
}

template <class RouterInfo>
void ExternalCarbonConnectionImpl<RouterInfo>::makeImpl() {
  if (connectionOptions_.accessPoint &&
      connectionOptions_.accessPoint->getProtocol() == mc_thrift_protocol) {
    thriftImpl_ =
        std::make_unique<Impl<facebook::memcache::ThriftTransport<RouterInfo>>>(
            connectionOptions_, options_);
  } else {
    carbonImpl_ = std::make_unique<Impl<facebook::memcache::AsyncMcClient>>(
        connectionOptions_, options_);
  }
}

template <class RouterInfo>
ExternalCarbonConnectionImpl<RouterInfo>::ExternalCarbonConnectionImpl(
    facebook::memcache::ConnectionOptions connectionOptions,
    ExternalCarbonConnectionImplOptions options)
    : connectionOptions_(std::move(connectionOptions)),
      options_(std::move(options)) {
  makeImpl();
}

template <class RouterInfo>
bool ExternalCarbonConnectionImpl<RouterInfo>::healthCheck() {
  try {
    return thriftImpl_ ? thriftImpl_->healthCheck()
                       : carbonImpl_->healthCheck();
  } catch (const CarbonConnectionRecreateException&) {
    makeImpl();
    return thriftImpl_ ? thriftImpl_->healthCheck()
                       : carbonImpl_->healthCheck();
  }
}

} // namespace carbon
