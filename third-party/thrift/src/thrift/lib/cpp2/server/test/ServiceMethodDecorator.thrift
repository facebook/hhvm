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

struct ServiceMethodDecoratorTestRequest {
  1: string id;
}

struct ServiceMethodDecoratorTestResponse {
  1: string text;
}

@cpp.GenerateServiceMethodDecorator
service ServiceMethodDecoratorTest {
  void noop();

  string echo(1: string text);

  i64 increment(1: i64 num);

  i64 sum(1: list<i64> nums);

  ServiceMethodDecoratorTestResponse withStruct(
    1: ServiceMethodDecoratorTestRequest request,
  );
}
