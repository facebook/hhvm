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

#pragma once

#include <folly/container/F14Map.h>
#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>

namespace apache::thrift::type_system::detail {

using RawAnnotations = folly::F14FastMap<
    apache::thrift::type_system::Uri,
    apache::thrift::type_system::SerializableRecordUnion>;

using AnnotationsMap = folly::
    F14FastMap<Uri, SerializableRecord, UriHeterogeneousHash, std::equal_to<>>;

inline RawAnnotations toRawAnnotations(const AnnotationsMap& annotations) {
  RawAnnotations raw;
  for (const auto& [uri, record] : annotations) {
    // Standard annotations are not currently bundled due to circular dependency
    // concerns. Skip it for now.
    if (uri.starts_with("facebook.com/thrift/annotation/")) {
      continue;
    }
    raw.emplace(uri, SerializableRecord::toThrift(record));
  }
  return raw;
}
} // namespace apache::thrift::type_system::detail
