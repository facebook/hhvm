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

#include <thrift/lib/cpp2/server/ServiceInterceptorBase.h>

namespace apache::thrift {

namespace server {
class ServerConfigs;
} // namespace server

class Cpp2RequestContext;

/**
 * Invokes onRequestDropped on all registered ServiceInterceptors.
 * Called from drop paths with access to Cpp2RequestContext.
 */
void processServiceInterceptorsOnDrop(
    const server::ServerConfigs& serverConfigs,
    const Cpp2RequestContext& reqCtx,
    ServiceInterceptorBase::DropReason reason);

/**
 * Convenience overload that extracts ServerConfigs from the request context's
 * connection context. No-ops if any pointer in the chain is null.
 *
 * Only safe to call from threads where the connection is guaranteed alive
 * (IO thread paths). CPU-thread callers should use the three-arg overload
 * with ServerConfigs& directly to avoid a TOCTOU race with connection
 * teardown on the IO thread.
 */
void processServiceInterceptorsOnDrop(
    const Cpp2RequestContext& reqCtx,
    ServiceInterceptorBase::DropReason reason);

} // namespace apache::thrift
