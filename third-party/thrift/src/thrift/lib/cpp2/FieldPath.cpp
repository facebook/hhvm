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

#include <folly/Traits.h>
#include <folly/Utility.h>
#include <thrift/lib/cpp2/FieldPath.h>

namespace apache::thrift::protocol {
namespace {

template <class T>
folly::like_t<T, Value>* FOLLY_NULLABLE get_impl(T* obj, const Path& path) {
  for (auto iter = path.path()->begin(); iter != path.path()->end(); ++iter) {
    auto next = obj->members()->find(folly::to_underlying(*iter));
    if (next == obj->members()->end()) {
      return nullptr;
    }

    auto& value = next->second;

    if (iter + 1 == path.path()->end()) {
      return &value;
    }

    if (!value.objectValue_ref()) {
      return nullptr;
    }

    obj = &*value.objectValue_ref();
  }
  return nullptr;
}
} // namespace

const Value* FOLLY_NULLABLE get(const Object& object, const Path& path) {
  return get_impl(&object, path);
}

Value* FOLLY_NULLABLE get(Object& object, const Path& path) {
  return get_impl(&object, path);
}
} // namespace apache::thrift::protocol
