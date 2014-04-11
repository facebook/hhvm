/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef HPHP_THRIFT_ASYNC_TEVENTBASEMANAGER_H
#define HPHP_THRIFT_ASYNC_TEVENTBASEMANAGER_H 1

#include "thrift/lib/cpp/concurrency/ThreadLocal.h"
#include "thrift/lib/cpp/concurrency/Mutex.h"

#include <set>
#include <list>

namespace apache { namespace thrift { namespace async {

class TEventBase;

/**
 * Manager for per-thread TEventBase objects.
 *   This class will find or create a TEventBase for the current
 *   thread, associated with thread-specific storage for that thread.
 *   Although a typical application will generally only have one
 *   TEventBaseManager, there is no restriction on multiple instances;
 *   the TEventBases belong to one instance are isolated from those of
 *   another.
 */
class TEventBaseManager {
 public:
  // XXX Constructing a TEventBaseManager directly is DEPRECATED and not
  // encouraged. You should instead use the global singleton if possible.
  TEventBaseManager() {
  }

  ~TEventBaseManager() {
  }

  /**
   * Get the global TEventBaseManager for this program. Ideally all users
   * of TEventBaseManager go through this interface and do not construct
   * TEventBaseManager directly.
   */
  static TEventBaseManager* get();

  /**
   * Get the TEventBase for this thread, or create one if none exists yet.
   *
   * If no TEventBase exists for this thread yet, a new one will be created and
   * returned.  May throw std::bad_alloc if allocation fails.
   */
  TEventBase* getEventBase() const;

  /**
   * Get the TEventBase for this thread.
   *
   * Returns nullptr if no TEventBase has been created for this thread yet.
   */
  TEventBase* getExistingEventBase() const {
    EventBaseInfo* info = localStore_.getNoAlloc();
    if (info == nullptr) {
      return nullptr;
    }
    return info->eventBase;
  }

  /**
   * Set the TEventBase to be used by this thread.
   *
   * This may only be called if no TEventBase has been defined for this thread
   * yet.  If a TEventBase is already defined for this thread, a
   * TLibraryException is thrown.  std::bad_alloc may also be thrown if
   * allocation fails while setting the TEventBase.
   *
   * This should typically be invoked by the code that will call loop() on the
   * TEventBase, to make sure the TEventBaseManager points to the correct
   * TEventBase that is actually running in this thread.
   */
  void setEventBase(TEventBase *eventBase, bool takeOwnership);

  /**
   * Clear the TEventBase for this thread.
   *
   * This can be used if the code driving the TEventBase loop() has finished
   * the loop and new events should no longer be added to the TEventBase.
   */
  void clearEventBase();

  /**
   * Gives the caller all references to all assigned TEventBase instances at
   * this moment in time.  Locks a mutex so that these TEventBase set cannot
   * be changed, and also the caller can rely on no instances being destructed.
   */
  template<typename FunctionType>
  void withEventBaseSet(const FunctionType& runnable) {
    // grab the mutex for the caller
    apache::thrift::concurrency::Guard g(*&eventBaseSetMutex_);
    // give them only a const set to work with
    const std::set<TEventBase *>& constSet = eventBaseSet_;
    runnable(constSet);
  }


 private:
  struct EventBaseInfo {
    EventBaseInfo(TEventBase *evb, bool owned)
      : eventBase(evb),
        owned(owned) {}

    TEventBase *eventBase;
    bool owned;
  };

  class InfoManager {
   public:
    EventBaseInfo* allocate();
    void destroy(EventBaseInfo* info);

    void replace(EventBaseInfo* oldInfo, EventBaseInfo* newInfo) {
      if (oldInfo != newInfo) {
        destroy(oldInfo);
      }
    }
  };

  // Forbidden copy constructor and assignment opererator
  TEventBaseManager(TEventBaseManager const &);
  TEventBaseManager& operator=(TEventBaseManager const &);

  void trackEventBase(TEventBase *evb) {
    apache::thrift::concurrency::Guard g(*&eventBaseSetMutex_);
    eventBaseSet_.insert(evb);
  }

  void untrackEventBase(TEventBase *evb) {
    apache::thrift::concurrency::Guard g(*&eventBaseSetMutex_);
    eventBaseSet_.erase(evb);
  }

  concurrency::ThreadLocal<EventBaseInfo, InfoManager> localStore_;

  // set of "active" TEventBase instances
  // (also see the mutex "eventBaseSetMutex_" below
  // which governs access to this).
  mutable std::set<TEventBase *> eventBaseSet_;

  // a mutex to use as a guard for the above set
  apache::thrift::concurrency::Mutex eventBaseSetMutex_;
};

}}} // apache::thrift::async

#endif // HPHP_THRIFT_ASYNC_TEVENTBASEMANAGER_H
