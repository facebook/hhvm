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

// Per-connection TLS gating policy. Mirrors apache::thrift::SSLPolicy from
// the classic ThriftServer. Fixes the shape of the connection acceptance
// pipeline at server start.
enum class SSLPolicy {
  // Server never performs TLS, even if cert config is supplied.
  DISABLED,
  // Server peeks the first 9 bytes of each accepted connection; routes
  // TLS-looking ones through the fizz handshake and treats the rest as
  // plaintext on the same listening socket.
  PERMITTED,
  // Every accepted connection is forced through the fizz handshake;
  // non-TLS clients fail.
  REQUIRED,
};

} // namespace apache::thrift::fast_thrift::security
