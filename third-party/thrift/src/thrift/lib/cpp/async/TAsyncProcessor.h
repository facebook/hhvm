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

#ifndef THRIFT_TASYNCPROCESSOR_H
#define THRIFT_TASYNCPROCESSOR_H 1

#include <functional>
#include <memory>

#include <thrift/lib/cpp/TProcessor.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>
#include <thrift/lib/cpp/server/TConnectionContext.h>

namespace apache {
namespace thrift {
namespace async {

/**
 * Async version of a TProcessor.  It is not expected to complete by the time
 * the call to process returns.  Instead, it calls a cob to signal completion.
 *
 * @author David Reiss <dreiss@facebook.com>
 */

class TAsyncProcessor : public TProcessorBase {
 public:
  virtual ~TAsyncProcessor() {}

  virtual void process(
      std::function<void(bool success)> _return,
      std::shared_ptr<protocol::TProtocol> in,
      std::shared_ptr<protocol::TProtocol> out,
      server::TConnectionContext* context = nullptr) = 0;

  void process(
      std::function<void(bool success)> _return,
      std::shared_ptr<apache::thrift::protocol::TProtocol> io) {
    return process(_return, io, io);
  }

 protected:
  TAsyncProcessor() {}
};

class TAsyncProcessorFactory {
 public:
  virtual ~TAsyncProcessorFactory() {}

  /**
   * Get the TAsyncProcessor to use for a particular connection.
   */
  virtual std::shared_ptr<TAsyncProcessor> getProcessor(
      server::TConnectionContext* ctx) = 0;
};

class TAsyncSingletonProcessorFactory : public TAsyncProcessorFactory {
 public:
  explicit TAsyncSingletonProcessorFactory(
      const std::shared_ptr<TAsyncProcessor>& processor)
      : processor_(processor) {}

  std::shared_ptr<TAsyncProcessor> getProcessor(
      server::TConnectionContext*) override {
    return processor_;
  }

 private:
  std::shared_ptr<TAsyncProcessor> processor_;
};

} // namespace async
} // namespace thrift
} // namespace apache

// XXX I'm lazy for now
namespace apache {
namespace thrift {
using apache::thrift::async::TAsyncProcessor;
}
} // namespace apache

#endif // #ifndef THRIFT_TASYNCPROCESSOR_H
