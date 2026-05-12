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

/**
 * Thrift-extension knobs negotiated during the fizz handshake. Mirrors the
 * shape of apache::thrift::ThriftTlsConfig from the legacy ThriftServer so
 * future toggles (StopTLS V2, Thrift params negotiation, PSP upgrade) land
 * here without churning the cert/handshake surface.
 */
struct ThriftTlsConfig {
  // StopTLS V1: server advertises StopTLS in the Thrift TLS extension. When
  // the client also requests it, the server performs a TLS shutdown
  // immediately after the handshake and the rest of the connection runs
  // plaintext over the original FD (peer/self cert info preserved).
  // Suitable only for hops where the network is already trusted.
  bool enableStopTLS{false};
};

} // namespace apache::thrift::fast_thrift::security
