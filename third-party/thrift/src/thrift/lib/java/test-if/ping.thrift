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

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace java.swift com.facebook.thrift.example.ping

struct PingRequest {
  1: string request;
}

struct PingResponse {
  1: string response;
}

exception CustomException {
  1: string message;
}

service PingService {
  PingResponse ping(1: PingRequest key);
  binary pingBinary(1: PingRequest key);
  PingResponse pingException(1: PingRequest key) throws (1: CustomException ex);
  void pingVoid(1: PingRequest key);
  stream<PingResponse> streamOfPings(1: PingRequest key, 2: i32 numberOfPings);
}

service ExtendedPing extends PingService {
  void pingExtended(1: PingRequest key);
}

service PingA {
  PingResponse pingA(1: PingRequest key);
}

service PingB {
  PingResponse pingB(1: PingRequest key);
}
