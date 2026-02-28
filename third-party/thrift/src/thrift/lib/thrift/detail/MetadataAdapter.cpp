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

#include "thrift/lib/thrift/detail/MetadataAdapter.h"
#include "thrift/lib/thrift/gen-cpp2/metadata_types.h"

namespace apache::thrift::metadata {

const ThriftConstStruct* findStructuredAnnotation(
    const detail::LimitedVector<ThriftConstStruct>& annotations,
    std::string_view name) {
  for (const auto& i : annotations) {
    if (i.type()->name() == name) {
      return &i;
    }
  }
  return nullptr;
}
const ThriftConstStruct& findStructuredAnnotationOrThrow(
    const detail::LimitedVector<ThriftConstStruct>& annotations,
    std::string_view name) {
  if (const auto* p = findStructuredAnnotation(annotations, name)) {
    return *p;
  }

  throw std::out_of_range("Can not find annotation: " + std::string(name));
}

} // namespace apache::thrift::metadata
