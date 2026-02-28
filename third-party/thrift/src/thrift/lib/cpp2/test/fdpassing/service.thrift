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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 facebook.demo

struct DemoRequest {
  1: string hello;
}

struct DemoResponse {
  1: string goodbye;
}

service DemoService {
  @cpp.GenerateDeprecatedHeaderClientMethods
  DemoResponse demo(1: DemoRequest r);
// TODO: Add coverage for these requests types, too
// stream<i32> intStream(1: DemoRequest r);
// oneway void demoNoResponse(1: DemoRequest r);
}
