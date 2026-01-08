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

cpp_include "thrift/lib/cpp2/server/test/ServiceMethodDecoratorTestLib.h"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct EchoRequest {
  1: string text;
}

struct EchoResponse {
  1: string text;
}

@cpp.GenerateServiceMethodDecorator
service ServiceMethodDecoratorTest {
  void noop();

  i64 sum(1: list<i64> nums);

  EchoResponse echo(1: EchoRequest request);
}

@cpp.Adapter{
  name = "::apache::thrift::test::EchoRequestAdapter<::apache::thrift::test::detail::EchoRequestAdapted>",
}
struct EchoRequestAdapted {
  1: string text;
}

@cpp.Adapter{
  name = "::apache::thrift::test::EchoResponseAdapter<::apache::thrift::test::detail::EchoResponseAdapted>",
}
struct EchoResponseAdapted {
  1: string text;
}

@cpp.GenerateServiceMethodDecorator
service TestAdapter {
  EchoResponseAdapted echo(1: EchoRequestAdapted request);
}

interaction PerformedInteraction {
  void perform();
}

@cpp.GenerateServiceMethodDecorator
service PerformInteractionTest {
  performs PerformedInteraction;
}

interaction RunningSum {
  i64 add(1: i64 num);
}

@cpp.GenerateServiceMethodDecorator
service ReturnsInteractionTest {
  RunningSum, i64 startAdding(1: i64 num);
}

@cpp.GenerateServiceMethodDecorator
service InheritsInteractionTest extends PerformInteractionTest {
}
