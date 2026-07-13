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

include "thrift/lib/python/test/reflection/annotation_value_def.thrift"

package "thrift.com/python/test/reflection/annotation_value_struct_reflection"

namespace py3 python_test.reflection

// The annotation value's type lives in included_annotation_value_type.thrift,
// which this file does not include directly. Before the fix, the generated
// thrift_reflection.py referenced that module without importing it, raising
// NameError in inspect(). Applied to both the struct and a field to cover both
// collection sites.
@annotation_value_def.WithNestedAnnotationValue{
  value = annotation_value_def.kAnnotationValue,
}
struct AnnotationValueReflectionStruct {
  @annotation_value_def.WithNestedAnnotationValue{
    value = annotation_value_def.kAnnotationValue,
  }
  1: i32 annotated_field;
}
