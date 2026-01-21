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

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/server/AsyncFizzServer.h>
#include <folly/io/async/AsyncSocket.h>

namespace apache::thrift {

extern const std::string kSecurityProtocolStopTLS;
/*
 * Takes an existing transport and creates a new socket with the same
 * underlying fd while trying to preserve as much information as possible.
 *
 * read() and write() I/O go to the `fd` directly, much like an AsyncSocket.
 *
 * Some of the TLS layer state (e.g. negotiated certificates, exporter secrets,
 * etc.) is retained.
 */
template <class FizzSocket>
folly::AsyncSocketTransport::UniquePtr toFDSocket(
    FizzSocket* socket, const std::string& securityProtocol);

} // namespace apache::thrift
