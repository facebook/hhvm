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

namespace py3 testing

include "thrift/lib/python/test/sub_dependency.thrift"
include "thrift/lib/python/test/injected_field.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/annotation/python.thrift"
include "thrift/annotation/scope.thrift"

@thrift.AllowLegacyMissingUris
package;

// An adapter-annotated struct that can be used as a transitive annotation
// from an included file. This tests that adapter type hint imports are
// generated correctly for adapters from included files.
// Using a UNIQUE adapter module that's ONLY used via this included file.
@python.Adapter{
  name = "thrift.python.test.adapters.included.IncludedAdapterImpl",
  typeHint = "thrift.python.test.adapters.included.IncludedAdapterConfig[]",
}
@scope.Transitive
struct IncludedAdapter {
  1: string signature;
}

enum Status {
  ACTIVE = 1,
  INACTIVE = 2,
  PENDING = 3,
}

typedef sub_dependency.IncludedColour ColourAlias

struct IncludedStruct {
  1: sub_dependency.Basic val;
  2: sub_dependency.IncludedColour color;
  3: list<sub_dependency.IncludedColour> color_list;
  4: list<sub_dependency.Basic> basic_list;
  5: set<sub_dependency.IncludedColour> color_set;
  6: map<sub_dependency.IncludedColour, sub_dependency.Basic> color_map;
}

struct FieldsWithIncludedStruct {
  1: injected_field.InjectedField injected_field;
}
