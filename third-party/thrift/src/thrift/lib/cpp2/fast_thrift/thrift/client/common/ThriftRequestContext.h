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

#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ClientAppAdapter.h>

namespace apache::thrift::fast_thrift::thrift::client {

/**
 * Per-request context. Travels through the pipeline opaquely as a
 * TypeErasedPtr; only the AppAdapter knows the concrete type. Extend
 * with new fields (transport stats, retry tracking, etc.) rather than
 * adding parallel per-request maps.
 */
struct ThriftRequestContext {
  RequestResponseHandler handler;

  explicit ThriftRequestContext(RequestResponseHandler handler)
      : handler(std::move(handler)) {}
};

} // namespace apache::thrift::fast_thrift::thrift::client
