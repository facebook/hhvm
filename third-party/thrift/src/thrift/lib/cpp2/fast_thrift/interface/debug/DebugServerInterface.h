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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapterFactory.h>

namespace apache::thrift::fast_thrift {

/**
 * A marker base class for a fast_thrift service intended for handling
 * debug connections (the fast_thrift counterpart of
 * `apache::thrift::DebugServerInterface` from the legacy stack).
 *
 * Debug RPC clients (`sendRequest`, `getServerDbgInfo`, and the `info`
 * TUI) call into this interface. The marker is a type-system guardrail —
 * passing a user-facing handler to setDebugInterface is a compile error.
 *
 * Implementations should NOT keep back-references to a single
 * FastThriftServer; iterate
 * `apache::thrift::fast_thrift::thrift::instrumentation::forEachServer`
 * with `kFastThriftServerTrackerKey` instead, so a single Debug handler
 * naturally serves every FastThriftServer in the process (matching
 * legacy's `instrumentation::forEachServer` model).
 *
 * DO NOT inherit this type if the ThriftServerAppAdapter returned by your
 * class handles non-debug methods.
 */
class DebugServerInterface
    : public virtual thrift::ThriftServerAppAdapterFactory {};

} // namespace apache::thrift::fast_thrift
