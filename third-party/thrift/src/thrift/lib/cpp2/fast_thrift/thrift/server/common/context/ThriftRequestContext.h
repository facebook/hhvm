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

#pragma once

#include <utility>

#include <boost/intrusive_ptr.hpp>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>

namespace apache::thrift::fast_thrift::thrift {

// Per-request context. Lives for the duration of one in-flight RPC.
//
// Default-constructed empty. Pipeline handlers populate individual fields as
// the request traverses the chain: RequestContextHandler creates the object,
// then ConnectionContextHandler stamps in the per-connection context, and so
// on as additional handlers come online (metadata, headers, etc.).
class ThriftRequestContext {
 public:
  ThriftRequestContext() = default;

  ThriftRequestContext(const ThriftRequestContext&) = delete;
  ThriftRequestContext& operator=(const ThriftRequestContext&) = delete;
  ThriftRequestContext(ThriftRequestContext&&) = default;
  ThriftRequestContext& operator=(ThriftRequestContext&&) = default;

  void setConnectionContext(
      boost::intrusive_ptr<ThriftConnContext> ctx) noexcept {
    connContext_ = std::move(ctx);
  }

  ThriftConnContext* getConnectionContext() const noexcept {
    return connContext_.get();
  }

 private:
  boost::intrusive_ptr<ThriftConnContext> connContext_;
};

} // namespace apache::thrift::fast_thrift::thrift
