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

include "thrift/annotation/thrift.thrift"
include "injected_field.thrift"

namespace go test.fixtures.inject_metadata_fields

struct Fields {
  100: string injected_field;
  @thrift.Box
  101: optional string injected_structured_annotation_field;
  @thrift.Box
  102: optional string injected_unstructured_annotation_field;
}

struct FieldsWithIncludedStruct {
  1: injected_field.InjectedField injected_field;
}
