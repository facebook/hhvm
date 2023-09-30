/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FifoManager.h"

#include <signal.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <mutex>

#include <folly/Format.h>
#include <folly/Singleton.h>
#include <folly/system/ThreadName.h>

namespace facebook {
namespace memcache {

namespace {

folly::Singleton<FifoManager> gFifoManager;

pid_t gettid() {
  return (pid_t)syscall(SYS_gettid);
}

} // anonymous namespace

FifoManager::FifoManager() {
  // Handle broken pipes on write syscalls.
  signal(SIGPIPE, SIG_IGN);

  thread_ = std::thread([this]() {
    folly::setThreadName("mcr-fifo-mngr");
    while (true) {
      fifos_.withRLock([](const auto& fifos) {
        for (auto& kv : fifos) {
          kv.second->tryConnect();
        }
      });

      {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait_for(lk, std::chrono::milliseconds(1000), [this]() {
          return !running_;
        });
        if (!running_) {
          break;
        }
      }
    }
  });
}

FifoManager::~FifoManager() {
  {
    std::unique_lock<std::mutex> lk(mutex_);
    running_ = false;
    cv_.notify_all();
  }
  thread_.join();
}

std::shared_ptr<Fifo> FifoManager::fetch(const std::string& fifoPath) {
  if (auto debugFifo = find(fifoPath)) {
    return debugFifo;
  }
  return createAndStore(fifoPath);
}

std::shared_ptr<Fifo> FifoManager::fetchThreadLocal(
    const std::string& fifoBasePath) {
  if (FOLLY_UNLIKELY(fifoBasePath.empty())) {
    LOG(ERROR) << "Cannot create a debug fifo with empty path.";
    return nullptr;
  }

  return fetch(folly::sformat("{0}.{1}", fifoBasePath, gettid()));
}

std::shared_ptr<Fifo> FifoManager::find(const std::string& fifoPath) {
  return fifos_.withRLock([&fifoPath](const auto& fifos) {
    auto it = fifos.find(fifoPath);
    return it != fifos.end() ? it->second : nullptr;
  });
}

std::shared_ptr<Fifo> FifoManager::createAndStore(const std::string& fifoPath) {
  return fifos_.withWLock([&fifoPath](auto& fifos) {
    auto it =
        fifos.emplace(fifoPath, std::shared_ptr<Fifo>(new Fifo(fifoPath)));
    return it.first->second;
  });
}

void FifoManager::clear() {
  fifos_.withWLock([](auto& fifos) { fifos.clear(); });
}

/* static  */ std::shared_ptr<FifoManager> FifoManager::getInstance() {
  return folly::Singleton<FifoManager>::try_get();
}

} // namespace memcache
} // namespace facebook
