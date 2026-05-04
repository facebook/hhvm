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

#include <memory>

#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Polymorphic base for every generated ServiceFastHandler<Service>. Provides
 * a single virtual hook that FastThriftServer calls per accepted connection
 * to obtain a fresh per-connection app adapter. The override knows the
 * concrete <Service>AppAdapter type and constructs it; the server sees only
 * the type-erased base ThriftServerAppAdapter and is responsible for placing
 * it in a pipeline.
 *
 * User code never names the ThriftServerAppAdapterFactory or the adapter —
 * they pass shared_ptr<MyHandler> to FastThriftServer::setInterface and the
 * implicit upcast to ThriftServerAppAdapterFactory happens transparently.
 */
class ThriftServerAppAdapterFactory {
 public:
  virtual ~ThriftServerAppAdapterFactory() = default;

  // Construct a per-connection adapter. The override receives `self` (the
  // server's shared handler ptr) so it can hand a typed shared_ptr to the
  // generated <Service>AppAdapter ctor without enable_shared_from_this.
  virtual std::
      unique_ptr<ThriftServerAppAdapter, folly::DelayedDestruction::Destructor>
      getAppAdapter(std::shared_ptr<ThriftServerAppAdapterFactory> self) = 0;
};

} // namespace apache::thrift::fast_thrift::thrift
