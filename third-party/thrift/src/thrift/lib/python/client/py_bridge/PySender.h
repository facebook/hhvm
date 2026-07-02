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

#include <cstdint>
#include <Python.h>

#include <folly/Executor.h>

#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache::thrift::python::client {

// Builds a PyBridgeRequestChannel whose sender forwards each enveloped request
// to the Python `ChannelHandler` `handler` (scheduling its `send_request`
// coroutine on the asyncio loop). Holds a counted reference to `handler` for
// the channel's lifetime. Keeping the (move-only) sender entirely in C++ means
// Cython only ever handles the resulting `RequestChannel::Ptr`.
RequestChannel::Ptr makeBridgedChannel(
    PyObject* handler, uint16_t protocolId, folly::Executor* executor);

} // namespace apache::thrift::python::client
