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

namespace cpp2 testing
namespace py3 testing

include "thrift/lib/py3/test/mixin/b.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct Mixin3Base {
  1: string field3;
}

typedef Mixin3Base Mixin3

union Union {
  1: string field5;
  2: string field6;
}

struct Foo {
  1: string field4;
  @thrift.Mixin
  2: b.Mixin2 m2;
  @thrift.Mixin
  3: Mixin3 m3;
  @thrift.Mixin
  4: Union u;
}
