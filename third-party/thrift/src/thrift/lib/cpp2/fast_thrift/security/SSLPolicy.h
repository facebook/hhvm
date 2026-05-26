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

namespace apache::thrift::fast_thrift::security {

// Per-server policy for how accepted sockets are treated with respect to TLS.
// Fixes the shape of the connection acceptance pipeline at server start.
enum class SSLPolicy {
  // Plaintext only. No TLS handlers installed; fizzContext is ignored.
  DISABLED,
  // TLS required on every accepted socket. FizzHandshakeHandler runs the
  // handshake; non-TLS bytes fail the handshake and the connection is dropped.
  REQUIRED,
};

} // namespace apache::thrift::fast_thrift::security
