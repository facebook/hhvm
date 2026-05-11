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

#include <fizz/server/FizzServerContext.h>

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>

namespace apache::thrift::fast_thrift::security {

/**
 * Build an immutable FizzServerContext from a cert/handshake config and the
 * thrift-extension config.
 *
 * The returned context is safe to share across all connections served by
 * a single server instance. Cert rotation is out of scope for v1 — callers
 * that need rotation should rebuild the context and replace it.
 *
 * Throws std::runtime_error on malformed PEM, missing file, or invalid
 * configuration (e.g. neither path nor buffer set).
 */
std::shared_ptr<const fizz::server::FizzServerContext> buildFizzServerContext(
    const FizzServerCertConfig& certConfig,
    const ThriftTlsConfig& thriftConfig);

} // namespace apache::thrift::fast_thrift::security
