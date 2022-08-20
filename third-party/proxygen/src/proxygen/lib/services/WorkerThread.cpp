/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/services/WorkerThread.h>

#include <folly/io/async/EventBase.h>

#include <folly/Portability.h>
#include <folly/String.h>
#include <folly/experimental/io/IoUringBackend.h>
#include <folly/io/async/EventBaseManager.h>
#include <glog/logging.h>
#include <signal.h>

#if !FOLLY_MOBILE && __has_include(<liburing.h>)

DEFINE_int32(pwt_io_uring_capacity, -1, "io_uring backend capacity");
DEFINE_int32(pwt_io_uring_max_submit, 128, "io_uring backend max submit");
DEFINE_int32(pwt_io_uring_max_get, -1, "io_uring backend max get");
DEFINE_bool(pwt_io_uring_use_registered_fds,
            false,
            "io_uring backend use registered fds");

namespace {
std::unique_ptr<folly::EventBaseBackendBase> getEventBaseBackend() {
  if (FLAGS_pwt_io_uring_capacity > 0) {
    try {
      folly::PollIoBackend::Options options;
      options.setCapacity(static_cast<size_t>(FLAGS_pwt_io_uring_capacity))
          .setMaxSubmit(static_cast<size_t>(FLAGS_pwt_io_uring_max_submit))
          .setMaxGet(static_cast<size_t>(FLAGS_pwt_io_uring_max_get))
          .setUseRegisteredFds(FLAGS_pwt_io_uring_use_registered_fds);

      auto ret = std::make_unique<folly::IoUringBackend>(options);
      LOG(INFO) << "Allocating io_uring backend(" << FLAGS_pwt_io_uring_capacity
                << "," << FLAGS_pwt_io_uring_max_submit << ","
                << FLAGS_pwt_io_uring_max_get << ","
                << FLAGS_pwt_io_uring_use_registered_fds << "): " << ret.get();

      return ret;
    } catch (const std::exception& ex) {
      LOG(INFO) << "Failure creating io_uring backend: " << ex.what();
    }
  }
  return folly::EventBase::getDefaultBackend();
}
} // namespace

#else

namespace {
std::unique_ptr<folly::EventBaseBackendBase> getEventBaseBackend() {
  return folly::EventBase::getDefaultBackend();
}
} // namespace

#endif

namespace proxygen {

thread_local WorkerThread* WorkerThread::currentWorker_ = nullptr;

WorkerThread::WorkerThread(folly::EventBaseManager* eventBaseManager,
                           const std::string& evbName)
    : eventBaseManager_(eventBaseManager),
      eventBase_(std::make_unique<folly::EventBase>(
          folly::EventBase::Options().setBackendFactory(
              [] { return getEventBaseBackend(); }))) {
  // Only set the event base name if not empty.
  // While not ideal, this preserves the previous program name inheritance
  // behavior.
  if (!evbName.empty()) {
    eventBase_->setName(evbName);
  }
}

WorkerThread::~WorkerThread() {
  CHECK(state_ == State::IDLE);

  // Reset the underlying event base.  This will execute all associated
  // execution pending funcs if not already reset.
  resetEventBase();
}

void WorkerThread::start() {
  CHECK(state_ == State::IDLE);
  state_ = State::STARTING;

  {
    // because you could theoretically call wait in parallel with start,
    // why are you in such a hurry anyways?
    std::lock_guard<std::mutex> guard(joinLock_);
    thread_ = std::thread([&]() mutable {
      this->setup();
      this->runLoop();
      this->cleanup();
    });
  }
  eventBase_->waitUntilRunning();
  // The server has been set up and is now in the loop implementation
}

void WorkerThread::stopWhenIdle() {
  // Call runInEventBaseThread() to perform all of the work in the actual
  // worker thread.
  //
  // This way we don't have to synchronize access to state_.
  eventBase_->runInEventBaseThread([this] {
    if (state_ == State::RUNNING) {
      state_ = State::STOP_WHEN_IDLE;
      eventBase_->terminateLoopSoon();
      // state_ could be IDLE if we don't execute this callback until the
      // EventBase is destroyed in the WorkerThread destructor
    } else if (state_ != State::IDLE && state_ != State::STOP_WHEN_IDLE) {
      LOG(FATAL) << "stopWhenIdle() called in unexpected state "
                 << static_cast<int>(state_);
    }
  });
}

void WorkerThread::forceStop() {
  // Call runInEventBaseThread() to perform all of the work in the actual
  // worker thread.
  //
  // This way we don't have to synchronize access to state_.
  eventBase_->runInEventBaseThread([this] {
    if (state_ == State::RUNNING || state_ == State::STOP_WHEN_IDLE) {
      state_ = State::FORCE_STOP;
      eventBase_->terminateLoopSoon();
      // state_ could be IDLE if we don't execute this callback until the
      // EventBase is destroyed in the WorkerThread destructor
    } else if (state_ != State::IDLE) {
      LOG(FATAL) << "forceStop() called in unexpected state "
                 << static_cast<int>(state_);
    }
  });
}

void WorkerThread::wait() {
  std::lock_guard<std::mutex> guard(joinLock_);
  if (thread_.joinable()) {
    thread_.join();
  }
}

void WorkerThread::setup() {
#ifndef _MSC_VER
  sigset_t ss;

  // Ignore some signals
  sigemptyset(&ss);
  sigaddset(&ss, SIGHUP);
  sigaddset(&ss, SIGINT);
  sigaddset(&ss, SIGQUIT);
  sigaddset(&ss, SIGUSR1);
  sigaddset(&ss, SIGUSR2);
  sigaddset(&ss, SIGPIPE);
  sigaddset(&ss, SIGALRM);
  sigaddset(&ss, SIGTERM);
  sigaddset(&ss, SIGCHLD);
  sigaddset(&ss, SIGIO);
  PCHECK(pthread_sigmask(SIG_BLOCK, &ss, nullptr) == 0);
#endif

  // Update the currentWorker_ thread-local pointer
  CHECK(nullptr == currentWorker_);
  currentWorker_ = this;

  // Update the manager with the event base this worker runs on
  if (eventBaseManager_) {
    eventBaseManager_->setEventBase(eventBase_.get(), false);
  }
}

void WorkerThread::cleanup() {
  currentWorker_ = nullptr;
  if (eventBaseManager_) {
    eventBaseManager_->clearEventBase();
  }
}

void WorkerThread::resetEventBase() {
  eventBase_.reset();
}

void WorkerThread::runLoop() {
  // Update state_
  CHECK(state_ == State::STARTING);
  state_ = State::RUNNING;

  VLOG(1) << "WorkerThread " << this << " starting";

  // Call loopForever().  This will only return after stopWhenIdle() or
  // forceStop() has been called.
  eventBase_->loopForever();

  if (state_ == State::STOP_WHEN_IDLE) {
    // We have been asked to stop when there are no more events left.
    // Call loop() to finish processing events.  This will return when there
    // are no more events to process, or after forceStop() has been called.
    VLOG(1) << "WorkerThread " << this << " finishing non-internal events";
    eventBase_->loop();
  }

  CHECK(state_ == State::STOP_WHEN_IDLE || state_ == State::FORCE_STOP);
  state_ = State::IDLE;

  VLOG(1) << "WorkerThread " << this << " terminated";
}

} // namespace proxygen
