/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THRIFT_ASYNC_TASYNCEVENTCHANNEL_H_
#define THRIFT_ASYNC_TASYNCEVENTCHANNEL_H_ 1

#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/async/TAsyncChannel.h>

namespace apache {
namespace thrift {
namespace async {

/**
 * TAsyncEventChannel defines an API for TAsyncChannel objects that are driven
 * by EventBase.
 */
class TAsyncEventChannel : public TAsyncChannel,
                           public folly::DelayedDestruction {
 public:
  /**
   * Determine if this channel is idle (i.e., has no outstanding reads or
   * writes).
   */
  virtual bool isIdle() const = 0;

  /**
   * Attach the channel to a EventBase.
   *
   * This may only be called if the channel is not currently attached to a
   * EventBase (by an earlier call to detachEventBase()).
   *
   * This method must be invoked in the EventBase's thread.
   */
  virtual void attachEventBase(folly::EventBase* eventBase) = 0;

  /**
   * Detach the channel from its EventBase.
   *
   * This may only be called when the channel is idle and has no reads or
   * writes pending.  Once detached, the channel may not be used again until it
   * is re-attached to a EventBase by calling attachEventBase().
   *
   * This method must be called from the current EventBase's thread.
   */
  virtual void detachEventBase() = 0;

  /**
   * Get the receive timeout.
   *
   * @return Returns the current receive timeout, in milliseconds.  A return
   *         value of 0 indicates that no timeout is set.
   */
  virtual uint32_t getRecvTimeout() const = 0;

  /**
   * Set the timeout for receiving messages.
   *
   * When set to a non-zero value, the entire message must be received within
   * the specified number of milliseconds, or the receive will fail and the
   * channel will be closed.
   */
  virtual void setRecvTimeout(uint32_t milliseconds) = 0;

 protected:
  /**
   * Protected destructor.
   *
   * Users of TAsyncEventChannel must never delete it directly. Instead, invoke
   * destroy() instead. (See the documentation in DelayedDestruction.h for
   * more details.)
   */

  ~TAsyncEventChannel() override {}
};

} // namespace async
} // namespace thrift
} // namespace apache

#endif // THRIFT_ASYNC_TASYNCEVENTCHANNEL_H_
