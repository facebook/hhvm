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

#include <thrift/lib/cpp2/transport/rocket/server/detail/RequestEncryptionStateDispatch.h>

#include <fizz/server/AsyncFizzServer.h>
#include <folly/io/async/AsyncTransport.h>

#include <thrift/lib/cpp2/Flags.h> // NOLINT(facebook-unused-include-check) used by THRIFT_FLAG_DEFINE_bool + THRIFT_FLAG
#include <thrift/lib/cpp2/security/SSLUtil.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/RequestEncryptionState.h>
#include <thrift/lib/cpp2/server/StopTLSEncryptionStateProvider.h>

THRIFT_FLAG_DEFINE_bool(server_request_encryption_tracking_enabled, false);

namespace apache::thrift::rocket::context_utils {

void checkRequestEncryptionState(Cpp2RequestContext& reqContext) {
  if (!THRIFT_FLAG(server_request_encryption_tracking_enabled)) {
    return;
  }
  auto* connCtx = reqContext.getConnectionContext();
  if (connCtx == nullptr) {
    return;
  }

  // StopTLSv2 case: encryption is per-record, check the record layer's sticky
  // latch (added in the parent stack). If we can reach the record layer, set
  // StoptlsEncrypted vs StoptlsSkipped based on hasObservedPlaintext(). If
  // the record layer is unreachable for any reason, fail safe to StoptlsSkipped
  // (we can't prove encryption — don't lie).
  if (connCtx->getSecurityProtocol() == kSecurityProtocolStopTLSV2) {
    // getUnderlyingTransport<T>() const overload returns const T*
    const auto* transport = connCtx->getTransport();
    if (transport != nullptr) {
      const auto* fizz =
          transport->getUnderlyingTransport<fizz::server::AsyncFizzServer>();
      if (fizz != nullptr) {
        // If the record layer implements the StopTLSEncryptionStateProvider
        // interface, then set the encryption state.
        const auto* layer = fizz->getState().readRecordLayer();
        const auto* provider =
            dynamic_cast<const StopTLSEncryptionStateProvider*>(layer);
        if (provider != nullptr && provider->isStopTLSNegotiated()) {
          reqContext.setRequestEncryptionState(
              provider->hasObservedPlaintext()
                  ? RequestEncryptionState::StoptlsSkipped
                  : RequestEncryptionState::StoptlsEncrypted);
          return;
        }
      }
    }
    // record layer unreachable / no implementer — fail safe to StoptlsSkipped
    reqContext.setRequestEncryptionState(
        RequestEncryptionState::StoptlsSkipped);
    return;
  }

  // All other connection types: encryption is per-connection. Use the
  // isTransportEncrypted() helper from D101058862.
  reqContext.setRequestEncryptionState(
      connCtx->isTransportEncrypted() ? RequestEncryptionState::Encrypted
                                      : RequestEncryptionState::Plaintext);
}

void checkWriteEncryptionState(Cpp2RequestContext& reqContext) {
  if (!THRIFT_FLAG(server_request_encryption_tracking_enabled)) {
    return;
  }
  auto* connCtx = reqContext.getConnectionContext();
  if (connCtx == nullptr) {
    return;
  }

  reqContext.setWriteEncryptionState(getWriteEncryptionState(*connCtx));
}

RequestEncryptionState getWriteEncryptionState(const Cpp2ConnContext& connCtx) {
  if (!THRIFT_FLAG(server_request_encryption_tracking_enabled)) {
    return RequestEncryptionState::Plaintext;
  }

  // StopTLSv2 case: encryption is per-record, check the write record layer's
  // sticky latch. If we can reach the record layer, set StoptlsEncrypted vs
  // StoptlsSkipped based on hasObservedPlaintext(). If the record layer is
  // unreachable for any reason, fail safe to StoptlsSkipped.
  if (connCtx.getSecurityProtocol() == kSecurityProtocolStopTLSV2) {
    // getUnderlyingTransport<T>() const overload returns const T*
    const auto* transport = connCtx.getTransport();
    if (transport != nullptr) {
      const auto* fizz =
          transport->getUnderlyingTransport<fizz::server::AsyncFizzServer>();
      if (fizz != nullptr) {
        // If the record layer implements the StopTLSEncryptionStateProvider
        // interface, then set the encryption state.
        const auto* layer = fizz->getState().writeRecordLayer();
        const auto* provider =
            dynamic_cast<const StopTLSEncryptionStateProvider*>(layer);
        if (provider != nullptr && provider->isStopTLSNegotiated()) {
          return provider->hasObservedPlaintext()
              ? RequestEncryptionState::StoptlsSkipped
              : RequestEncryptionState::StoptlsEncrypted;
        }
      }
    }
    // record layer unreachable / no implementer — fail safe to StoptlsSkipped
    return RequestEncryptionState::StoptlsSkipped;
  }

  // All other connection types: encryption is per-connection. Use the
  // isTransportEncrypted() helper from D101058862.
  return connCtx.isTransportEncrypted() ? RequestEncryptionState::Encrypted
                                        : RequestEncryptionState::Plaintext;
}

} // namespace apache::thrift::rocket::context_utils
