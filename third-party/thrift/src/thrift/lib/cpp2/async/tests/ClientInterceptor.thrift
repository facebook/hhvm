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

namespace cpp2 apache.thrift.test

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

/**
 * This struct is used to test passing of frameworkMetadata between client and
 * server interceptors. The single string field is used to verify
 * ClientInterceptor's ability to mutate the metadata
 */
struct FrameworkMetadata {
  1: string value;
  2: string postProcessing;
}

service ClientInterceptorTest {
  void noop();

  string echo(1: string str);

  SampleInteraction createInteraction();
  SampleInteraction, string createInteractionAndEcho(1: string str);

  string requestArgs(
    1: i32 primitive,
    2: string str,
    3: RequestArgsStruct request,
  );

  stream<i32> iota(1: i32 start);

  sink<i32, i32> dump();

  oneway void fireAndForget(1: i32 ignored);

  @cpp.GenerateDeprecatedHeaderClientMethods
  string headerClientMethod(1: string str);
}
