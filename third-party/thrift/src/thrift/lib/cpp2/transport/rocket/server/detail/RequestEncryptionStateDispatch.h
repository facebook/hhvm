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

namespace apache::thrift {
class Cpp2ConnContext;
class Cpp2RequestContext;
enum class RequestEncryptionState;
} // namespace apache::thrift

namespace apache::thrift::rocket::context_utils {

/**
 * Inspects the request's connection state and sets
 * Cpp2RequestContext::setRequestEncryptionState() to one of:
 *   - RequestEncryptionState::Plaintext
 *   - RequestEncryptionState::Encrypted
 *   - RequestEncryptionState::StoptlsEncrypted
 *   - RequestEncryptionState::StoptlsSkipped
 *
 * Gated by THRIFT_FLAG(server_request_encryption_tracking_enabled). When
 * disabled, this is a no-op (the field stays at its default Plaintext).
 *
 * When enabled:
 * - StopTLSv2 connections: walks the cast chain to read the record layer's
 *   hasObservedPlaintext() sticky latch. Cost: ~modest (string compare +
 *   dynamic_cast + 2 method calls).
 * - All other connections: cheap branch on getSecurityProtocol() + a single
 *   bool read via isTransportEncrypted(). Cost: ~negligible.
 *
 * Must be called after Cpp2RequestContext is fully constructed (connection
 * context attached, security protocol available), before any user code runs.
 */
void checkRequestEncryptionState(Cpp2RequestContext& reqContext);

/**
 * Inspects the response's connection state and returns one of:
 *   - RequestEncryptionState::Plaintext
 *   - RequestEncryptionState::Encrypted
 *   - RequestEncryptionState::StoptlsEncrypted
 *   - RequestEncryptionState::StoptlsSkipped
 *
 * Symmetric to checkRequestEncryptionState() but reads the write record layer
 * instead of the read record layer. Captures the encryption state of the
 * outbound response.
 *
 * Gated by THRIFT_FLAG(server_request_encryption_tracking_enabled). When
 * disabled, this returns the default Plaintext state.
 *
 * Must be called after the response has been sent, immediately before request
 * event logging.
 */
RequestEncryptionState getWriteEncryptionState(const Cpp2ConnContext& connCtx);

/**
 * Inspects the response's connection state and sets
 * Cpp2RequestContext::setWriteEncryptionState(). The request context must
 * still be alive; response-write callbacks should use getWriteEncryptionState()
 * with the longer-lived connection context instead.
 */
void checkWriteEncryptionState(Cpp2RequestContext& reqContext);

} // namespace apache::thrift::rocket::context_utils
