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
 * future toggles (StopTLS V1/V2, Thrift params negotiation, PSP upgrade)
 * land here without churning the cert/handshake surface.
 *
 * Empty today — populated as fast_thrift adds support for each extension.
 */
struct ThriftTlsConfig {};

} // namespace apache::thrift::fast_thrift::security
