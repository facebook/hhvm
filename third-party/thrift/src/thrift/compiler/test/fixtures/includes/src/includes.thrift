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

include "transitive.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace android one.two.three
namespace java.swift test.fixtures.includes.includes

struct Included {
  1: i64 MyIntField = 0;
  2: transitive.Foo MyTransitiveField = transitive.ExampleFoo;
}

const Included ExampleIncluded = {
  "MyIntField": 2,
  "MyTransitiveField": transitive.ExampleFoo,
};

typedef i64 IncludedInt64

const i64 IncludedConstant = 42;

// This lets us test typedefs-of-typedefs, see `service.thrift`.
typedef transitive.Foo TransitiveFoo
