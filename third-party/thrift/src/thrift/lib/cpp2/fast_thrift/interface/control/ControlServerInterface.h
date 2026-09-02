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
 * A marker base class for a fast_thrift service that reads or mutates the
 * environment the server runs in — options, gflags, settings (the fast_thrift
 * counterpart of apache::thrift::ControlServerInterface from
 * thrift/lib/cpp2/server/ControlServerInterface.h).
 *
 * Marker exists purely as a type-system guardrail: passing a user-facing
 * handler to setControlInterface is a compile error.
 *
 * Nothing installs a control handler by default, on either stack. The legacy
 * createDefaultExtraInterfaces leaves this slot null and services opt in, so
 * a server that wants option or gflag introspection wires one itself.
 *
 * As with the security slot, no IDL ships alongside this header: the
 * canonical Control service depends on Meta-internal settings and config
 * types that cannot live in this tree. The embedder supplies both the IDL and
 * the FastServiceHandler that implements it.
 *
 * DO NOT inherit this type if the ThriftServerAppAdapter returned by your
 * class handles non-control methods.
 */
class ControlServerInterface
    : public virtual thrift::ThriftServerAppAdapterFactory {};

} // namespace apache::thrift::fast_thrift
