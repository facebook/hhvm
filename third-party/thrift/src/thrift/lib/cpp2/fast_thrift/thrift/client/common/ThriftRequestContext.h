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
 * Per-request context allocated by the AppAdapter on the outbound path
 * and consumed by the AppAdapter on the inbound path.
 *
 * The pipeline transports this opaquely as a TypeErasedPtr (see the
 * `requestContext` field on the message variants). The AppAdapter supplies
 * the deleter when constructing the handle (via `with_custom_deleter`);
 * the rocket layer routes the handle through its stream-id slot map and
 * runs the deleter on every cleanup path without knowing the concrete
 * type.
 *
 * Today this holds just the response handler — replacing the per-request
 * map that used to live in the AppAdapter. Future handlers (transport
 * stats, retry tracking, observability) extend this struct with their own
 * fields rather than maintaining parallel maps.
 */
struct ThriftRequestContext {
  RequestResponseHandler handler;

  explicit ThriftRequestContext(RequestResponseHandler handler)
      : handler(std::move(handler)) {}
};

} // namespace apache::thrift::fast_thrift::thrift::client
