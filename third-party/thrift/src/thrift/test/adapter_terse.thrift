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

namespace cpp2 apache.thrift.test.terse

include "thrift/annotation/cpp.thrift"
cpp_include "thrift/test/AdapterTest.h"

@cpp.Adapter{name = "::apache::thrift::test::AdaptTestMsAdapter"}
typedef i64 DurationMs

@cpp.Adapter{name = "::apache::thrift::test::CustomProtocolAdapter"}
typedef binary (cpp.type = "::folly::IOBuf") CustomProtocolType

struct AdaptTestStruct {
  1: DurationMs delay;
  2: CustomProtocolType custom;
}

union AdaptTestUnion {
  1: DurationMs delay;
  2: CustomProtocolType custom;
}
