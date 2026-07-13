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

include "thrift/lib/python/test/reflection/included_annotation_value_type.thrift"

package "thrift.com/python/test/reflection/annotation_value_def"

namespace py3 python_test.reflection

// Annotation whose value field type lives in another program. Consumers apply
// it (via the constant below) without including that program, so the value type
// is only transitively reachable -- mirroring `@acl.Action`/`action.Value`.
struct WithNestedAnnotationValue {
  1: included_annotation_value_type.AnnotationValueStruct value;
}

const included_annotation_value_type.AnnotationValueStruct kAnnotationValue = included_annotation_value_type.AnnotationValueStruct{name = "reflected"};
