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

namespace cpp2 apache.thrift.test

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

interaction SampleInteraction {
  string echo(1: string str);
}

interaction SampleInteraction2 {
  string echo(1: string str);
}

struct RequestArgsStruct {
  1: i32 foo;
  2: string bar;
}

struct ResponseArgsStruct {
  1: i32 foo;
  2: string bar;
}

service ServiceInterceptorTestBase {
  void noop();
}

service ServiceInterceptorTest extends ServiceInterceptorTestBase {
  string echo(1: string str);

  SampleInteraction createInteraction();
  performs SampleInteraction2;

  stream<i32> iota(1: i32 start);

  string requestArgs(
    1: i32 primitive,
    2: string str,
    3: RequestArgsStruct request,
  );

  ResponseArgsStruct echoStruct(1: RequestArgsStruct request);

  @cpp.ProcessInEbThreadUnsafe
  string echo_eb(1: string str);

  oneway void fireAndForget(1: i32 ignored);
}
