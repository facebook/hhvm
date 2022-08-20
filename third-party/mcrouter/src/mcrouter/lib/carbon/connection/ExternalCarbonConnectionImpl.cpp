/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ExternalCarbonConnectionImpl.h"

#include <memory>
#include <thread>

#include <gflags/gflags.h>

#include <folly/Singleton.h>
#include <folly/fibers/EventBaseLoopController.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/system/ThreadName.h>

namespace carbon {
namespace detail {

DEFINE_int32(
    cacheclient_external_connection_threads,
    4,
    "Thread count for ExternalCarbonConnectionImpl");

ClientBase::ClientBase(
    facebook::memcache::ConnectionOptions connOpts,
    ExternalCarbonConnectionImplOptions opts)
    : connectionOptions(std::move(connOpts)), options(opts) {
  if (options.maxOutstanding > 0) {
    counting_sem_init(&outstandingReqsSem, options.maxOutstanding);
  }
}

void ClientBase::closeNow() {
  getClient().closeNow();
}

size_t ClientBase::limitRequests(size_t requestsCount) {
  if (options.maxOutstanding == 0) {
    return requestsCount;
  }

  if (options.maxOutstandingError) {
    return counting_sem_lazy_nonblocking(&outstandingReqsSem, requestsCount);
  } else {
    return counting_sem_lazy_wait(&outstandingReqsSem, requestsCount);
  }
}

ThreadInfo::ThreadInfo()
    : fiberManager_(
          std::make_unique<folly::fibers::EventBaseLoopController>()) {
  folly::Baton<> baton;

  thread_ = std::thread([this, &baton] {
    folly::setThreadName("mc-eccc-pool");

    folly::EventBase* evb = folly::EventBaseManager::get()->getEventBase();
    dynamic_cast<folly::fibers::EventBaseLoopController&>(
        fiberManager_.loopController())
        .attachEventBase(evb->getVirtualEventBase());
    // At this point it is safe to use fiberManager.
    baton.post();
    evb->loopForever();

    // Close all connections.
    for (auto& client : clients_) {
      client->closeNow();
    }
    clients_.clear();
  });

  // Wait until the thread is properly initialized.
  baton.wait();
}

ThreadInfo::~ThreadInfo() {
  fiberManager_.addTaskRemote([] {
    folly::EventBaseManager::get()->getEventBase()->terminateLoopSoon();
  });
  thread_.join();
}

namespace {
struct SingletonTag {};
} // anonymous namespace
folly::Singleton<ThreadPool, SingletonTag> threadPool{};

int ThreadPool::getThreadNum() {
  return FLAGS_cacheclient_external_connection_threads;
}

/*static*/ std::shared_ptr<ThreadPool> ThreadPool::getInstance() {
  return threadPool.try_get();
}

} // namespace detail
} // namespace carbon
