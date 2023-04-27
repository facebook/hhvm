/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#ifndef THRIFT_EVENTHANDLERBASE_H_
#define THRIFT_EVENTHANDLERBASE_H_ 1

#include <memory>
#include <vector>

#include <folly/Range.h>
#include <folly/SharedMutex.h>
#include <folly/memory/not_null.h>

#include <thrift/lib/cpp/ContextStack.h>

namespace apache {
namespace thrift {

class EventHandlerBase {
 public:
  void addEventHandler(const std::shared_ptr<TProcessorEventHandler>& handler);
  void addNotNullEventHandler(
      const folly::not_null_shared_ptr<TProcessorEventHandler>& handler);

  void clearEventHandlers() { handlers_.reset(); }

  folly::Range<std::shared_ptr<TProcessorEventHandler>*> getEventHandlers()
      const;

  ContextStack::UniquePtr getContextStack(
      const char* service_name,
      const char* fn_name,
      server::TConnectionContext* connectionContext) {
    return ContextStack::create(
        handlers_, service_name, fn_name, connectionContext);
  }

 protected:
  ~EventHandlerBase() = default;

  std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>
      handlers_;
};

class TProcessorEventHandlerFactory {
 public:
  virtual ~TProcessorEventHandlerFactory() = default;
  virtual std::shared_ptr<TProcessorEventHandler> getEventHandler() = 0;
};

/**
 * Base class for all thrift processors. Used to automatically attach event
 * handlers to processors at creation time.
 */
class TProcessorBase : public EventHandlerBase {
 public:
  TProcessorBase();

  static void addProcessorEventHandler(
      std::shared_ptr<TProcessorEventHandler> handler);

  static void removeProcessorEventHandler(
      std::shared_ptr<TProcessorEventHandler> handler);

  static void addProcessorEventHandlerFactory(
      std::shared_ptr<TProcessorEventHandlerFactory> factory);

  static void removeProcessorEventHandlerFactory(
      std::shared_ptr<TProcessorEventHandlerFactory> factory);

 protected:
  ~TProcessorBase() = default;

 private:
  static folly::SharedMutex& getRWMutex();

  static std::vector<folly::not_null_shared_ptr<TProcessorEventHandler>>&
  getHandlers();
  static std::vector<std::shared_ptr<TProcessorEventHandlerFactory>>&
  getFactories();
};

/**
 * Base class for all thrift clients. Used to automatically attach event
 * handlers to clients at creation time.
 */
class TClientBase : public EventHandlerBase {
 public:
  TClientBase();
  virtual ~TClientBase() = default;

  static void addClientEventHandler(
      std::shared_ptr<TProcessorEventHandler> handler);

  static void removeClientEventHandler(
      std::shared_ptr<TProcessorEventHandler> handler);

  static void addClientEventHandlerFactory(
      std::shared_ptr<TProcessorEventHandlerFactory> factory);

  static void removeClientEventHandlerFactory(
      std::shared_ptr<TProcessorEventHandlerFactory> factory);

 private:
  static folly::SharedMutex& getRWMutex();

  static std::vector<folly::not_null_shared_ptr<TProcessorEventHandler>>&
  getHandlers();
  static std::vector<std::shared_ptr<TProcessorEventHandlerFactory>>&
  getFactories();
};

} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_EVENTHANDLERBASE_H_
