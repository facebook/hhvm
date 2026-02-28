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

package "test.dev/fixtures/basic"

include "module.thrift"

namespace py3 meta.example.thrift

struct EchoRequest {
  1: string text;
}

struct EchoResponse {
  1: string text;
}

exception WhisperException {
  1: string message;
}

service EchoService {
  EchoResponse echo(1: EchoRequest request) throws (1: WhisperException ex);
}

service ExtendedEchoService extends EchoService {
  EchoResponse echo_2(1: EchoRequest request) throws (1: WhisperException ex);
}

service ExtendedMyService extends module.MyService {
  void putDataById_2(1: i64 id, 2: string data);
}
