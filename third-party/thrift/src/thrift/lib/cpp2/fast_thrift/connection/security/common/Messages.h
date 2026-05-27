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

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache::thrift::fast_thrift::connection::security {

// Single message type that flows through the TLS pipeline. Each accepted
// socket enters the pipeline as one TLSPipelineMessage; each handler may
// upgrade the transport in-place (AsyncSocket → AsyncFizzServer after
// handshake → plaintext AsyncSocketTransport after StopTLS) and forward
// the same message type. The tail (ConnectionTLSHandler in its tail role)
// receives the final message and fires it back onto the outer pipeline
// as a ConnectionMessage.
//
// `extension` is null until the fizz handshake completes; downstream
// stages (StopTLSV1Handler etc.) query it to decide whether to act.
struct TLSPipelineMessage {
  folly::AsyncTransport::UniquePtr transport;
  folly::SocketAddress clientAddr;
  std::shared_ptr<apache::thrift::ThriftParametersServerExtension> extension;
};

} // namespace apache::thrift::fast_thrift::connection::security
