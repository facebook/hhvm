// Copyright (c) 2008- Facebook
// Distributed under the Thrift Software License
//
// See accompanying file LICENSE or visit the Thrift site at:
// http://developers.facebook.com/thrift/

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "thrift/lib/cpp/async/TEventBase.h"

#include "thrift/lib/cpp/async/TNotificationQueue.h"
#include "thrift/lib/cpp/concurrency/Util.h"

#include <boost/static_assert.hpp>
#include <unistd.h>
#include <fcntl.h>

using apache::thrift::concurrency::Util;

namespace {

using apache::thrift::async::Cob;
using apache::thrift::async::TEventBase;

class Tr1FunctionLoopCallback : public TEventBase::LoopCallback {
 public:
  explicit Tr1FunctionLoopCallback(const Cob& function)
    : function_(function) {}

  virtual void runLoopCallback() noexcept {
    function_();
    delete this;
  }

 private:
  Cob function_;
};

}

namespace apache { namespace thrift { namespace async {

const int kNoFD = -1;

/*
 * TEventBase::FunctionRunner
 */

class TEventBase::FunctionRunner
    : public TNotificationQueue<std::pair<void (*)(void*), void*>>::Consumer {
 public:
  void messageAvailable(std::pair<void (*)(void*), void*>&& msg) {
    if (msg.first == nullptr && msg.second == nullptr) {
      // terminateLoopSoon() sends a null message just to
      // wake up the loop.  We can ignore these messages.
      return;
    }

    // If function is nullptr, just log and move on
    if (!msg.first) {
      LOG(ERROR) << "nullptr callback registered to be run in "
                 << "event base thread";
      return;
    }

    // The function should never throw an exception, because we have no
    // way of knowing what sort of error handling to perform.
    //
    // If it does throw, log a message and abort the program.
    try {
      msg.first(msg.second);
    } catch (const std::exception& ex) {
      LOG(ERROR) << "runInEventBaseThread() function threw a "
                 << typeid(ex).name() << " exception: " << ex.what();
      abort();
    } catch (...) {
      LOG(ERROR) << "runInEventBaseThread() function threw an exception";
      abort();
    }
  }
};

/*
 * TEventBase::CobTimeout methods
 */

void TEventBase::CobTimeout::timeoutExpired() noexcept {
  // For now, we just swallow any exceptions that the callback threw.
  try {
    cob_();
  } catch (const std::exception& ex) {
    LOG(ERROR) << "TEventBase::runAfterDelay() callback threw "
               << typeid(ex).name() << " exception: " << ex.what();
  } catch (...) {
    LOG(ERROR) << "TEventBase::runAfterDelay() callback threw non-exception "
               << "type";
  }

  // The CobTimeout object was allocated on the heap by runAfterDelay(),
  // so delete it now that the it has fired.
  delete this;
}

/*
 * TEventBase methods
 */

TEventBase::TEventBase()
  : runOnceCallbacks_(nullptr)
  , stop_(false)
  , running_(false)
  , evb_(static_cast<event_base*>(event_init()))
  , queue_(nullptr)
  , fnRunner_(nullptr)
  , maxLatency_(0)
  , avgLoopTime_(2000000)
  , maxLatencyLoopTime_(avgLoopTime_)
  , nextLoopCnt_(-40)       // Early wrap-around so bugs will manifest soon
  , latestLoopCnt_(nextLoopCnt_)
  , startWork_(0) {
  VLOG(5) << "TEventBase(): Created.";
  initNotificationQueue();
  RequestContext::getStaticContext();
}

// takes ownership of the event_base
TEventBase::TEventBase(event_base* evb)
  : runOnceCallbacks_(nullptr)
  , stop_(false)
  , running_(false)
  , evb_(evb)
  , queue_(nullptr)
  , fnRunner_(nullptr)
  , maxLatency_(0)
  , avgLoopTime_(2000000)
  , maxLatencyLoopTime_(avgLoopTime_)
  , nextLoopCnt_(-40)       // Early wrap-around so bugs will manifest soon
  , latestLoopCnt_(nextLoopCnt_)
  , startWork_(0) {
  initNotificationQueue();
  RequestContext::getStaticContext();
}

TEventBase::~TEventBase() {
  // Delete any unfired CobTimeout objects, so that we don't leak memory
  // (Note that we don't fire them.  The caller is responsible for cleaning up
  // its own data structures if it destroys the TEventBase with unfired events
  // remaining.)
  while (!pendingCobTimeouts_.empty()) {
    CobTimeout* timeout = &pendingCobTimeouts_.front();
    delete timeout;
  }

  (void) runLoopCallbacks(false);

  // Stop consumer before deleting TNotificationQueue
  fnRunner_->stopConsuming();
  event_base_free(evb_);
  VLOG(5) << "TEventBase(): Destroyed.";
}

int TEventBase::getNotificationQueueSize() const {
  return queue_->size();
}

// Set smoothing coefficient for loop load average; input is # of milliseconds
// for exp(-1) decay.
void TEventBase::setLoadAvgMsec(uint32_t ms) {
  uint64_t us = 1000 * ms;
  if (ms > 0) {
    maxLatencyLoopTime_.setTimeInterval(us);
    avgLoopTime_.setTimeInterval(us);
  } else {
    LOG(ERROR) << "non-positive arg to setLoadAvgMsec()";
  }
}

void TEventBase::resetLoadAvg(double value) {
  avgLoopTime_.reset(value);
  maxLatencyLoopTime_.reset(value);
}

static int64_t getTimeDelta(int64_t *prev) {
  int64_t now = Util::currentTime();
  int64_t delta = now - *prev;
  *prev = now;
  return delta;
}

// enters the event_base loop -- will only exit when forced to
bool TEventBase::loop() {
  VLOG(5) << "TEventBase(): Starting loop.";
  int res = 0;
  bool ranLoopCallbacks;
  int nonBlocking;

  loopThread_ = pthread_self();
  running_ = true;

  // TODO: Read stop_ atomically with an acquire barrier.
  // Do this once thrift can depend on google-base's atomic primitives.
  int64_t prev      = Util::currentTime();
  int64_t idleStart = Util::monotonicTimeUsec();
  while (!stop_) {
    ++nextLoopCnt_;

    // nobody can add loop callbacks from within this thread if
    // we don't have to handle anything to start with...
    nonBlocking = (loopCallbacks_.empty() ? 0 : EVLOOP_NONBLOCK);
    res = event_base_loop(evb_, EVLOOP_ONCE | nonBlocking);
    ranLoopCallbacks = runLoopCallbacks();

    int64_t busy = Util::monotonicTimeUsec() - startWork_;
    int64_t idle = startWork_ - idleStart;

    avgLoopTime_.addSample(idle, busy);
    maxLatencyLoopTime_.addSample(idle, busy);

    VLOG(11) << "TEventBase " << this         << " did not timeout "
     " loop time guess: "    << busy + idle  <<
     " idle time: "          << idle         <<
     " busy time: "          << busy         <<
     " avgLoopTime: "        << avgLoopTime_.get() <<
     " maxLatencyLoopTime: " << maxLatencyLoopTime_.get() <<
     " maxLatency_: "        << maxLatency_ <<
     " nothingHandledYet(): "<< nothingHandledYet();

    // see if our average loop time has exceeded our limit
    if ((maxLatency_ > 0) &&
        (maxLatencyLoopTime_.get() > double(maxLatency_))) {
      maxLatencyCob_();
      // back off temporarily -- don't keep spamming maxLatencyCob_
      // if we're only a bit over the limit
      maxLatencyLoopTime_.dampen(0.9);
    }

    // Our loop run did real work; reset the idle timer
    idleStart = Util::monotonicTimeUsec();

    // If the event loop indicate that there were no more events, and
    // we also didn't have any loop callbacks to run, there is nothing left to
    // do.
    if (res != 0 && !ranLoopCallbacks) {
      break;
    }

    VLOG(5) << "TEventBase " << this << " loop time: " << getTimeDelta(&prev);
  }
  running_ = false;
  // Reset stop_ so loop() can be called again
  stop_ = false;

  if (res < 0) {
    LOG(ERROR) << "TEventBase: -- error in event loop, res = " << res;
    return false;
  } else if (res == 1) {
    VLOG(5) << "TEventBase: ran out of events (exiting loop)!";
  } else if (res > 1) {
    LOG(ERROR) << "TEventBase: unknown event loop result = " << res;
    return false;
  }

  VLOG(5) << "TEventBase(): Done with loop.";
  return true;
}

void TEventBase::loopForever() {
  // Update the notification queue event to treat it as a normal (non-internal)
  // event.  The notification queue event always remains installed, and the main
  // loop won't exit with it installed.
  fnRunner_->stopConsuming();
  fnRunner_->startConsuming(this, queue_.get());

  bool ret = loop();

  // Restore the notification queue internal flag
  fnRunner_->stopConsuming();
  fnRunner_->startConsumingInternal(this, queue_.get());

  if (!ret) {
    throw TLibraryException("error in TEventBase::loopForever()");
  }
}

bool TEventBase::bumpHandlingTime() {
  VLOG(11) << "TEventBase " << this << " " << __PRETTY_FUNCTION__ <<
    " (loop) latest " << latestLoopCnt_ << " next " << nextLoopCnt_;
  if(nothingHandledYet()) {
    latestLoopCnt_ = nextLoopCnt_;
    // set the time
    startWork_ = Util::monotonicTimeUsec();
    VLOG(11) << "TEventBase " << this << " " << __PRETTY_FUNCTION__ <<
      " (loop) startWork_ " << startWork_;
    return true;
  }
  return false;
}

void TEventBase::terminateLoopSoon() {
  VLOG(5) << "TEventBase(): Received terminateLoopSoon() command.";
  // Set stop to true, so the event loop will know to exit.
  // TODO: We should really use an atomic operation here with a release
  // barrier.  For now thrift doesn't have atomic primitives, and can't depend
  // on google-base yet.
  stop_ = true;

  // Call event_base_loopbreak() so that libevent will exit the next time
  // around the loop.
  event_base_loopbreak(evb_);

  // If terminateLoopSoon() is called from another thread,
  // the TEventBase thread might be stuck waiting for events.
  // In this case, it won't wake up and notice that stop_ is set until it
  // receives another event.  Send an empty frame to the notification queue
  // so that the event loop will wake up even if there are no other events.
  //
  // We don't care about the return value of trySendFrame().  If it fails
  // this likely means the TEventBase already has lots of events waiting
  // anyway.
  try {
    queue_->putMessage(std::make_pair(nullptr, nullptr));
  } catch (...) {
    // We don't care if putMessage() fails.  This likely means
    // the TEventBase already has lots of events waiting anyway.
  }
}

void TEventBase::runInLoop(LoopCallback* callback, bool thisIteration) {
  DCHECK(isInEventBaseThread());
  callback->cancelLoopCallback();
  callback->context_ = RequestContext::saveContext();
  if (runOnceCallbacks_ != nullptr && thisIteration) {
    runOnceCallbacks_->push_back(*callback);
  } else {
    loopCallbacks_.push_back(*callback);
  }
}

void TEventBase::runInLoop(const Cob& cob, bool thisIteration) {
  DCHECK(isInEventBaseThread());
  Tr1FunctionLoopCallback* wrapper = new Tr1FunctionLoopCallback(cob);
  wrapper->context_ = RequestContext::saveContext();
  if (runOnceCallbacks_ != nullptr && thisIteration) {
    runOnceCallbacks_->push_back(*wrapper);
  } else {
    loopCallbacks_.push_back(*wrapper);
  }
}

bool TEventBase::runInEventBaseThread(void (*fn)(void*), void* arg) {
  // Send the message.
  // It will be received by the FunctionRunner in the TEventBase's thread.

  // We try not to schedule nullptr callbacks
  if (!fn) {
    LOG(ERROR) << "TEventBase " << this
               << ": Scheduling nullptr callbacks is not allowed";
    return false;
  }

  // Short-circuit if we are already in our event base
  if (inRunningEventBaseThread()) {
    runInLoop(new RunInLoopCallback(fn, arg));
    return true;

  }

  try {
    queue_->putMessage(std::make_pair(fn, arg));
  } catch (const std::exception& ex) {
    LOG(ERROR) << "TEventBase " << this << ": failed to schedule function "
               << fn << "for TEventBase thread: " << ex.what();
    return false;
  }

  return true;
}

bool TEventBase::runInEventBaseThread(const Cob& fn) {
  // Short-circuit if we are already in our event base
  if (inRunningEventBaseThread()) {
    runInLoop(fn);
    return true;
  }

  Cob* fnCopy;
  // Allocate a copy of the function so we can pass it to the other thread
  // The other thread will delete this copy once the function has been run
  try {
    fnCopy = new Cob(fn);
  } catch (const std::bad_alloc& ex) {
    LOG(ERROR) << "failed to allocate tr::function copy "
               << "for runInEventBaseThread()";
    return false;
  }

  if (!runInEventBaseThread(&TEventBase::runTr1FunctionPtr, fnCopy)) {
    delete fnCopy;
    return false;
  }

  return true;
}

bool TEventBase::runAfterDelay(const Cob& cob,
                               int milliseconds,
                               TimeoutManager::InternalEnum in) {
  CobTimeout* timeout = new CobTimeout(this, cob, in);
  if (!timeout->scheduleTimeout(milliseconds)) {
    delete timeout;
    return false;
  }

  pendingCobTimeouts_.push_back(*timeout);
  return true;
}

bool TEventBase::runLoopCallbacks(bool setContext) {
  if (!loopCallbacks_.empty()) {
    bumpHandlingTime();
    // Swap the loopCallbacks_ list with a temporary list on our stack.
    // This way we will only run callbacks scheduled at the time
    // runLoopCallbacks() was invoked.
    //
    // If any of these callbacks in turn call runInLoop() to schedule more
    // callbacks, those new callbacks won't be run until the next iteration
    // around the event loop.  This prevents runInLoop() callbacks from being
    // able to start file descriptor and timeout based events.
    LoopCallbackList currentCallbacks;
    currentCallbacks.swap(loopCallbacks_);
    runOnceCallbacks_ = &currentCallbacks;

    while (!currentCallbacks.empty()) {
      LoopCallback* callback = &currentCallbacks.front();
      currentCallbacks.pop_front();
      if (setContext) {
        RequestContext::setContext(callback->context_);
      }
      callback->runLoopCallback();
    }

    runOnceCallbacks_ = nullptr;
    return true;
  }
  return false;
}

void TEventBase::initNotificationQueue() {
  // Infinite size queue
  queue_.reset(new TNotificationQueue<std::pair<void (*)(void*), void*>>());

  // We allocate fnRunner_ separately, rather than declaring it directly
  // as a member of TEventBase solely so that we don't need to include
  // TNotificationQueue.h from TEventBase.h
  fnRunner_.reset(new FunctionRunner());

  // Mark this as an internal event, so event_base_loop() will return if
  // there are no other events besides this one installed.
  //
  // Most callers don't care about the internal notification queue used by
  // TEventBase.  The queue is always installed, so if we did count the queue as
  // an active event, loop() would never exit with no more events to process.
  // Users can use loopForever() if they do care about the notification queue.
  // (This is useful for TEventBase threads that do nothing but process
  // runInEventBaseThread() notifications.)
  fnRunner_->startConsumingInternal(this, queue_.get());
}

void TEventBase::SmoothLoopTime::setTimeInterval(uint64_t timeInterval) {
  expCoeff_ = -1.0/timeInterval;
  VLOG(11) << "expCoeff_ " << expCoeff_ << " " << __PRETTY_FUNCTION__;
}

void TEventBase::SmoothLoopTime::reset(double value) {
  value_ = value;
}

void TEventBase::SmoothLoopTime::addSample(int64_t idle, int64_t busy) {
    /*
     * Position at which the busy sample is considered to be taken.
     * (Allows to quickly skew our average without editing much code)
     */
    enum BusySamplePosition {
      RIGHT = 0,  // busy sample placed at the end of the iteration
      CENTER = 1, // busy sample placed at the middle point of the iteration
      LEFT = 2,   // busy sample placed at the beginning of the iteration
    };

  VLOG(11) << "idle " << idle << " oldBusyLeftover_ " << oldBusyLeftover_ <<
              " idle + oldBusyLeftover_ " << idle + oldBusyLeftover_ <<
              " busy " << busy << " " << __PRETTY_FUNCTION__;
  idle += oldBusyLeftover_ + busy;
  oldBusyLeftover_ = (busy * BusySamplePosition::CENTER) / 2;
  idle -= oldBusyLeftover_;

  double coeff = exp(idle * expCoeff_);
  value_ *= coeff;
  value_ += (1.0 - coeff) * busy;
}

bool TEventBase::nothingHandledYet() {
  VLOG(11) << "latest " << latestLoopCnt_ << " next " << nextLoopCnt_;
  return (nextLoopCnt_ != latestLoopCnt_);
}

/* static */
void TEventBase::runTr1FunctionPtr(Cob* fn) {
  // The function should never throw an exception, because we have no
  // way of knowing what sort of error handling to perform.
  //
  // If it does throw, log a message and abort the program.
  try {
    (*fn)();
  } catch (const std::exception &ex) {
    LOG(ERROR) << "runInEventBaseThread() tr1::function threw a "
               << typeid(ex).name() << " exception: " << ex.what();
    abort();
  } catch (...) {
    LOG(ERROR) << "runInEventBaseThread() tr1::function threw an exception";
    abort();
  }

  // The function object was allocated by runInEventBaseThread().
  // Delete it once it has been run.
  delete fn;
}

TEventBase::RunInLoopCallback::RunInLoopCallback(void (*fn)(void*), void* arg)
    : fn_(fn)
    , arg_(arg) {}

void TEventBase::RunInLoopCallback::runLoopCallback() noexcept {
  fn_(arg_);
  delete this;
}

void TEventBase::attachTimeoutManager(TAsyncTimeout* obj,
                                      InternalEnum internal) {

  struct event* ev = obj->getEvent();
  assert(ev->ev_base == nullptr);

  event_base_set(getLibeventBase(), ev);
  if (internal == TAsyncTimeout::InternalEnum::INTERNAL) {
    // Set the EVLIST_INTERNAL flag
    ev->ev_flags |= EVLIST_INTERNAL;
  }
}

void TEventBase::detachTimeoutManager(TAsyncTimeout* obj) {
  cancelTimeout(obj);
  struct event* ev = obj->getEvent();
  ev->ev_base = nullptr;
}

bool TEventBase::scheduleTimeout(TAsyncTimeout* obj,
                                 std::chrono::milliseconds timeout) {
  assert(isInEventBaseThread());
  // Set up the timeval and add the event
  struct timeval tv;
  Util::toTimeval(tv, timeout.count());

  struct event* ev = obj->getEvent();
  if (event_add(ev, &tv) < 0) {
    LOG(ERROR) << "TEventBase: failed to schedule timeout: " << strerror(errno);
    return false;
  }

  return true;
}

void TEventBase::cancelTimeout(TAsyncTimeout* obj) {
  assert(isInEventBaseThread());
  struct event* ev = obj->getEvent();
  if (TEventUtil::isEventRegistered(ev)) {
    event_del(ev);
  }
}

}}} // apache::thrift::async
