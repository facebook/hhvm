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

package "thrift.com/python/test/reflection/annotation_value_reflection"

namespace py3 python_test.reflection

// The annotation value references annotation_value_def.kAnnotationValue, whose
// type lives in included_annotation_value_type.thrift -- a program this file
// does NOT include directly. Before the reflection-import fix, the generated
// thrift_services_reflection.py referenced that module without importing it,
// raising NameError when inspect() materialized the service reflection.
@annotation_value_def.WithNestedAnnotationValue{
  value = annotation_value_def.kAnnotationValue,
}
service AnnotationValueReflectionService {
  i32 ping();
}
