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
#ifndef HPHP_THRIFT_ASYNC_TIMEOUTMANAGER_H
#define HPHP_THRIFT_ASYNC_TIMEOUTMANAGER_H 1

#include <chrono>
#include <stdint.h>

namespace apache { namespace thrift { namespace async {

class TAsyncTimeout;

/**
 * Base interface to be implemented by all classes expecting to manage
 * timeouts. TAsyncTimeout will use implementations of this interface
 * to schedule/cancel timeouts.
 */
class TimeoutManager {
 public:
  enum class InternalEnum {
    INTERNAL,
    NORMAL
  };

  virtual ~TimeoutManager() {}

  /**
   * Attaches/detaches TimeoutManager to TAsyncTimeout
   */
  virtual void attachTimeoutManager(TAsyncTimeout* obj,
                                    InternalEnum internal) = 0;
  virtual void detachTimeoutManager(TAsyncTimeout* obj) = 0;

  /**
   * Schedules TAsyncTimeout to fire after `timeout` milliseconds
   */
  virtual bool scheduleTimeout(TAsyncTimeout* obj,
                               std::chrono::milliseconds timeout) = 0;

  /**
   * Cancels the TAsyncTimeout, if scheduled
   */
  virtual void cancelTimeout(TAsyncTimeout* obj) = 0;

  /**
   * This is used to mark the beginning of a new loop cycle by the
   * first handler fired within that cycle.
   */
  virtual bool bumpHandlingTime() = 0;

  /**
   * Helper method to know whether we are running in the timeout manager
   * thread
   */
  virtual bool isInTimeoutManagerThread() = 0;
};

}}} // apache::thrift::async

#endif // HPHP_THRIFT_ASYNC_TASYNCTIMEOUT_H
