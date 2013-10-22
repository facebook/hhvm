// Copyright (c) 2006- Facebook
// Distributed under the Thrift Software License
//
// See accompanying file LICENSE or visit the Thrift site at:
// http://developers.facebook.com/thrift/

#ifndef HPHP_THRIFT_ASYNC_TEVENTJOBQUEUE_H
#define HPHP_THRIFT_ASYNC_TEVENTJOBQUEUE_H 1

#include "thrift/lib/cpp/Thrift.h"
#include "thrift/lib/cpp/concurrency/Thread.h"
#include "thrift/lib/cpp/concurrency/PosixThreadFactory.h"
#include "thrift/lib/cpp/async/TNotificationQueue.h"

namespace apache { namespace thrift { namespace async {

/**
 * Similar to concurrency::Runnable, but run has access to a TEventBase to do
 * asynchronous work.
 *
 * There is one queue per worker thread, so work is distributed not
 * when a worker is free, but when a task is enqueued.  This is slightly
 * less fair than concurrency::Runnable, but increases the performance
 * for short jobs by about 3x due to the lack of locking.
 */
class TEventRunnable {
 public:
  TEventRunnable() : eventBase_(nullptr) {}
  virtual ~TEventRunnable() {}

  /**
   * Set the event base on this runnable
   */
  void setEventBase(TEventBase *eventBase) {
    eventBase_ = eventBase;
  }

  /**
   * Returns the event base this runnable is currently associated with
   *
   * The TEventJobQueue will set this correctly for run and jobComplete
   */
  TEventBase *getEventBase() const {
    return eventBase_;
  }

  /**
   * derived classes must implement run.
   */
  virtual void run() = 0;


 protected:

  TEventBase *eventBase_;

};


/**
 * A job queue for working in a TEventBase driven application.
 *
 * N threads are spawned and begin consuming a notification queue of
 * TEventRunnable's.
 */
class TEventJobQueue {
 private:

  /**
   * A thread that listens for work on a notification queue, and
   * executes a job
   */
  class JobThread:
      public apache::thrift::concurrency::Runnable,
      public TNotificationQueue<TEventRunnable*>::Consumer {

  public:
    explicit JobThread(TEventJobQueue *parent) :
        parent_(parent) {
    }
    virtual ~JobThread() {}

    TEventBase *getEventBase() { return &eventBase_; }

    void join() {
      thread_->join();
    }

    TNotificationQueue<TEventRunnable*>* getQueue() {
      return &jobQueue_;
    }

    /**
     * Thread main loop
     */
    void run() {
      // Hold a reference so we can join this thread later
      thread_ = thread();
      // Listen for new work
      startConsuming(&eventBase_, &jobQueue_);
      try {
        eventBase_.loop();
      } catch (const std::exception &ex) {
        LOG(ERROR) << "Unhandled exception in TEventJobQueue: " <<
          ex.what();
      }
      TNotificationQueue<TEventRunnable*>::Consumer::stopConsuming();
    }

   private:
    TEventJobQueue *parent_;
    TEventBase eventBase_;
    std::shared_ptr<apache::thrift::concurrency::Thread> thread_;
    TNotificationQueue<TEventRunnable*> jobQueue_;

    /**
     * A new runnable arrived - run it!
     */
    void messageAvailable(TEventRunnable*&& runnable) {
      runnable->setEventBase(&eventBase_);
      runnable->run();
    }
  };

 public:
  explicit TEventJobQueue(uint32_t numThreads) :
      numThreads_(numThreads),
      curThread_(0),
      shouldJoin_(true) {
  }
  // Constructor where setNumThreads must be called later
  explicit TEventJobQueue() :
      numThreads_(0),
      curThread_(0),
      shouldJoin_(true){}

  // Must be called before init() is called
  void setNumThreads(uint32_t numThreads) {
    numThreads_ = numThreads;
  }

  ~TEventJobQueue() {
    shutdown(true, shouldJoin_); // force
  }


  /**
   * Start the worker threads
   */
  bool init(apache::thrift::concurrency::ThreadFactory* argFactory = nullptr) {
    CHECK(numThreads_ > 0);
    apache::thrift::concurrency::PosixThreadFactory defaultFactory;
    defaultFactory.setDetached(false);
    apache::thrift::concurrency::ThreadFactory* factory = &defaultFactory;
    if (argFactory != nullptr) {
      factory = argFactory;
      shouldJoin_ = false;
    }

    threads_.resize(numThreads_);
    for (uint32_t idx = 0; idx < numThreads_; idx++) {
      threads_[idx] = std::shared_ptr<JobThread>(new JobThread(this));
      std::shared_ptr<apache::thrift::concurrency::Thread> thread =
        factory->newThread(threads_[idx]);
      if (!thread) {
        LOG(CRITICAL) << "Cannot create thread with number: " << idx;
        shutdown(false, shouldJoin_);
        return false;
      }
      thread->start();
    }
    return true;
  }

  /**
   * Instruct each thread to stop listening for new work.
   *
   * When force is true, terminate the consumer thread's event loop
   * even if there is work pending.
   *
   * When join is true, wait for all threads to terminate before
   * leaving shutdown.
   */
  void shutdown(bool force = false, bool join = true) {
    for (auto &thread: threads_) {
      thread->getEventBase()->runInEventBaseThread([thread, force]{
          thread->stopConsuming();
          if (force) {
            thread->getEventBase()->terminateLoopSoon();
          }
        });
    }
    if (join) {
      for (auto &thread: threads_) {
        thread->join();
      }
      threads_.clear();
    }
  }

  /**
   * Add some work to the queue
   *
   * You may call enqueueJob from any thread with no performance penalty
   */
  void enqueueJob(TEventRunnable* runnable) {
    // Note: The logic here must be thread safe with no locking.
    // It isn't perfectly fair: on wrap around, curThread_ may skip
    // some queues, but a mod and set could not be performed atomically
    threads_[(curThread_++) % numThreads_]->getQueue()->putMessage(runnable);
  }

  /**
   * Run the given function in all consumer threads.  The function is passed
   * the event base for the consumer thread.
   */
  void runInAllThreads(const std::function<void(TEventBase *)>& fn) {
    for (auto &thread: threads_) {
      thread->getEventBase()->runInEventBaseThread(
        std::bind(fn, thread->getEventBase()));
    }
  }

private:
  uint32_t numThreads_;
  std::atomic<uint64_t> curThread_;
  std::vector<std::shared_ptr<JobThread>> threads_;
  bool shouldJoin_;
};

}}}

#endif
