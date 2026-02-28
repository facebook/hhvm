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

#include <thrift/lib/cpp2/dynamic/detail/DatumHash.h>

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/Union.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>

namespace apache::thrift::dynamic::detail {

std::size_t DatumHash::operator()(const Datum& d) const {
  return d.visit(*this);
}

std::size_t DatumHash::operator()(const Struct& value) const {
  size_t hash = 0;
  for (size_t idx = 0; idx < value.type().asStructUnchecked().fields().size();
       idx++) {
    auto handle = type_system::FastFieldHandle::fromIndex(idx);
    auto content = value.getField(handle);
    size_t fieldHash = content ? operator()(*content) : 0;
    hash = folly::hash::hash_combine(hash, fieldHash);
  }
  return hash;
}

std::size_t DatumHash::operator()(const Union& value) const {
  if (value.isEmpty()) {
    return 0;
  }

  auto activeHandle = value.activeField();
  const auto& unionDef = value.type().asUnionUnchecked();
  const auto& activeFieldDef = unionDef.fields()[activeHandle.index()];
  auto fieldRef = value.getField(activeHandle);
  size_t fieldHash = operator()(fieldRef);
  return folly::hash::hash_combine(activeFieldDef.identity().id(), fieldHash);
}

std::size_t DatumHash::operator()(const List& value) const {
  // Start with a base hash
  size_t hash = 0;

  // Hash each element in order
  for (auto it = value.cbegin(); it != value.cend(); ++it) {
    size_t elemHash = operator()((*it));
    hash = folly::hash::hash_combine(hash, elemHash);
  }

  return hash;
}

std::size_t DatumHash::operator()(const Set& value) const {
  // Start with a base hash
  size_t hash = 0;

  // Hash each element order-independently
  for (auto it = value.cbegin(); it != value.cend(); ++it) {
    size_t elemHash = operator()((*it));
    hash = folly::hash::commutative_hash_combine(hash, elemHash);
  }

  return hash;
}

std::size_t DatumHash::operator()(const Map& value) const {
  // Start with a base hash
  size_t hash = 0;

  // Hash each key-value pair, then hash the result order-independently
  for (auto [keyRef, valRef] : value) {
    size_t keyHash = operator()(keyRef);
    size_t valueHash = operator()(valRef);
    size_t pairHash = folly::hash::hash_combine(keyHash, valueHash);
    hash = folly::hash::commutative_hash_combine(hash, pairHash);
  }

  return hash;
}

std::size_t DatumHash::operator()(const DynamicConstRef& value) const {
  return std::visit(
      [this](const auto* type) { return operator()(*type); }, value.ptr_);
}

} // namespace apache::thrift::dynamic::detail
