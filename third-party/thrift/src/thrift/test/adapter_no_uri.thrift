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

namespace cpp2 apache.thrift.test.no_uri

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

cpp_include "thrift/test/AdapterTest.h"
cpp_include "thrift/lib/cpp2/Adapt.h"

union ThriftComparisonUnion {
  @cpp.Adapter{name = "::apache::thrift::test::NonComparableWrapperAdapter"}
  1: string field1;
}

union RefUnion {
  @cpp.Adapter{name = "::apache::thrift::test::NonComparableWrapperAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: string field1;
}

union AdapterComparisonUnion {
  @cpp.Adapter{name = "::apache::thrift::test::AdapterComparisonStringAdapter"}
  1: string field1;
}

union AdaptedComparisonUnion {
  @cpp.Adapter{name = "::apache::thrift::test::AdaptedComparisonStringAdapter"}
  1: string field1;
}

struct AdapterThreeWayComparisonStruct {
  @cpp.Adapter{name = "::apache::thrift::test::Adapter3WayCompareStringAdapter"}
  1: string field1;
}
