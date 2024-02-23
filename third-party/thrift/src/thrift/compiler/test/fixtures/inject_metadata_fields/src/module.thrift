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

include "thrift/annotation/internal.thrift"
include "thrift/annotation/thrift.thrift"
include "foo.thrift"

namespace java.swift test.fixtures.injectMetadataFields

struct Fields {
  100: string injected_field;
}

@internal.InjectMetadataFields{type = "Fields"}
struct FieldsInjectedToEmptyStruct {}

@internal.InjectMetadataFields{type = "Fields"}
struct FieldsInjectedToStruct {
  1: string string_field;
}

@internal.InjectMetadataFields{type = "foo.Fields"}
struct FieldsInjectedWithIncludedStruct {
  1: string string_field;
}
