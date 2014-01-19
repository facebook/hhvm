// Copyright 2004-present Facebook. All Rights Reserved.
#include "ti/proxygen/lib/workers/WorkerThread.h"

#include <boost/thread.hpp>
#include <signal.h>

#include "hphp/third_party/stubs/glog/portability.h"
#include "folly/String.h"
#include "thrift/lib/cpp/async/TEventBaseManager.h"

using apache::thrift::async::TEventBaseManager;

namespace facebook { namespace proxygen {

__thread WorkerThread *WorkerThread::currentWorker_ = nullptr;

WorkerThread::WorkerThread(TEventBaseManager* eventBaseManager)
    : eventBaseManager_(eventBaseManager) {
}

WorkerThread::~WorkerThread() {
  CHECK(state_ == State::IDLE);
}

void WorkerThread::start() {
  CHECK(state_ == State::IDLE);
  state_ = State::STARTING;

  boost::barrier barrier(2);
  thread_ = std::thread([&] {
      this->setup();
      barrier.wait();
      this->runLoop();
      this->cleanup();
  });
  barrier.wait();
  // The server has been set up and is now in the loop implementation
}

void WorkerThread::stopWhenIdle() {
  // Call runInEventBaseThread() to perform all of the work in the actual
  // worker thread.
  //
  // This way we don't have to synchronize access to state_.
  eventBase_.runInEventBaseThread([this] {
    if (state_ == State::RUNNING) {
      state_ = State::STOP_WHEN_IDLE;
      eventBase_.terminateLoopSoon();
    } else if (state_ != State::STOP_WHEN_IDLE) {
      LOG(FATAL) << "stopWhenIdle() called in unexpected state " <<
          static_cast<int>(state_);
    }
  });
}

void WorkerThread::forceStop() {
  // Call runInEventBaseThread() to perform all of the work in the actual
  // worker thread.
  //
  // This way we don't have to synchronize access to state_.
  //
  // This also has the benefit of preserving ordering between functions already
  // scheduled with runInEventBaseThread() and the actual stop.  Functions
  // already scheduled before forceStop() was called are guaranteed to be run
  // before the thread stops.  (If we called terminateLoopSoon() from the
  // current thread, the worker thread may stop before running functions
  // already scheduled via runInEventBaseThread().)
  eventBase_.runInEventBaseThread([this] {
    if (state_ == State::RUNNING || state_ == State::STOP_WHEN_IDLE) {
      state_ = State::FORCE_STOP;
      eventBase_.terminateLoopSoon();
    } else {
      LOG(FATAL) << "forceStop() called in unexpected state " <<
          static_cast<int>(state_);
    }
  });
}

void WorkerThread::wait() {
  thread_.join();
}

void WorkerThread::setup() {
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

  // Update the currentWorker_ thread-local pointer
  CHECK_NULL(currentWorker_);
  currentWorker_ = this;

  // Update the manager with the event base this worker runs on
  if (eventBaseManager_) {
    eventBaseManager_->setEventBase(&eventBase_, false);
  }
}

void WorkerThread::cleanup() {
  currentWorker_ = nullptr;
  if (eventBaseManager_) {
    eventBaseManager_->clearEventBase();
  }
}

void WorkerThread::runLoop() {
  // Update state_
  CHECK(state_ == State::STARTING);
  state_ = State::RUNNING;

  VLOG(1) << "WorkerThread " << this << " starting";

  // Call loopForever().  This will only return after stopWhenIdle() or
  // forceStop() has been called.
  eventBase_.loopForever();

  if (state_ == State::STOP_WHEN_IDLE) {
    // We have been asked to stop when there are no more events left.
    // Call loop() to finish processing events.  This will return when there
    // are no more events to process, or after forceStop() has been called.
    VLOG(1) << "WorkerThread " << this << " finishing non-internal events";
    eventBase_.loop();
  }

  CHECK(state_ == State::STOP_WHEN_IDLE || state_ == State::FORCE_STOP);
  state_ = State::IDLE;

  VLOG(1) << "WorkerThread " << this << " terminated";
}

}} // facebook::proxygen
