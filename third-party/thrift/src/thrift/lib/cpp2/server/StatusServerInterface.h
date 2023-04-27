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

#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace apache {
namespace thrift {

/**
 * A marker base class for a Thrift service intended for handling server status.
 * It is intended as a way to export ThriftService::ServerStatus externally as
 * an RPC.
 *
 * The singular purpose of this type is to discourage setting a server's status
 * interface to a handler which may respond to non-status methods. It disallows
 * accidentally setting the user-facing and status interfaces to the same
 * handler by having distinct types for each. The type system handles the rest.
 *
 * DO NOT inherit this type if the `AsyncProcessor` returned by your class
 * handles non-status methods.
 */
class StatusServerInterface : public virtual AsyncProcessorFactory {};

} // namespace thrift
} // namespace apache
