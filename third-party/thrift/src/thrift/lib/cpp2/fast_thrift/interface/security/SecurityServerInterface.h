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
 * A marker base class for a fast_thrift service intended for reading security
 * metadata (the fast_thrift counterpart of
 * apache::thrift::SecurityServerInterface from
 * thrift/lib/cpp2/server/SecurityServerInterface.h).
 *
 * Security-metadata introspection RPCs — "who am I to this server", which
 * interface owns a method, what the server's enforcement posture is — are
 * answered on this interface. Marker exists purely as a type-system
 * guardrail: passing a user-facing handler to setSecurityInterface is a
 * compile error.
 *
 * Unlike the monitoring / status / debug slots, no IDL ships alongside this
 * header: the canonical security-metadata service depends on Meta-internal
 * types that cannot live in this tree. The embedder supplies both the IDL
 * and the FastServiceHandler that implements it.
 *
 * Naming contract for that service: authorization gates exempt these RPCs by
 * matching the `Service.method` name against a fixed allowlist of interface
 * prefixes (facebook::services::isInternalMethod). Two consequences for an
 * embedder. The service name must be one the allowlist already recognizes,
 * and — because a fast_thrift method name on the wire is bare — a gate must
 * qualify it with the service name before matching. An unqualified name never
 * matches, which denies the request rather than exempting it.
 *
 * DO NOT inherit this type if the ThriftServerAppAdapter returned by your
 * class handles non-security methods.
 */
class SecurityServerInterface
    : public virtual thrift::ThriftServerAppAdapterFactory {};

} // namespace apache::thrift::fast_thrift
