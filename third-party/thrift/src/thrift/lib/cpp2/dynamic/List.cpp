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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteList.h>

#include <folly/Overload.h>

#include <memory_resource>

namespace apache::thrift::dynamic {

namespace {
// Helper to invoke a lambda with a concrete list type from a TypeRef.
template <typename F>
auto withConcreteType(const type_system::TypeRef::List& type, F&& f) {
  return type.elementType().matchKind(
      [&]<type_system::TypeRef::Kind k>(type_system::TypeRef::KindConstant<k>) {
        using T = detail::type_of_type_kind<k>;
        using ConcreteType = detail::ConcreteList<T>;
        return std::forward<F>(f).template operator()<ConcreteType>();
      });
}
} // namespace

List makeList(
    type_system::TypeRef::List listType, std::pmr::memory_resource* allocator) {
  return List(listType, allocator);
}

List fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::TypeRef::List& listType,
    std::pmr::memory_resource* alloc) {
  auto ret = makeList(listType, alloc);

  const auto& listRecords = r.asList();
  const type_system::TypeRef& elementType = listType.elementType();

  for (const auto& elementRecord : listRecords) {
    // Convert each element record to a DynamicValue and append
    auto datum = elementType.visit([&](auto&& t) {
      return detail::Datum::make(fromRecord(elementRecord, t, alloc));
    });
    ret.push_back(DynamicValue(elementType, std::move(datum)));
  }

  return ret;
}

// List method implementations
List::List(detail::IList::Ptr impl)
    : listType_(impl->type().asListUnchecked()),
      mr_(nullptr),
      impl_(std::move(impl)) {}

List::List(const List& other)
    : listType_(other.listType_),
      mr_(other.mr_),
      impl_(other.impl_ ? other.impl_->clone() : nullptr) {}

List::~List() = default;

List& List::operator=(const List& other) {
  if (this != &other) {
    listType_ = other.listType_;
    // Note: mr_ is just a pointer, we can copy it
    mr_ = other.mr_;
    impl_ = other.impl_ ? other.impl_->clone() : nullptr;
  }
  return *this;
}

detail::IList& List::ensureInit() {
  if (!impl_) {
    withConcreteType(listType_, [this]<typename ConcreteType>() {
      if (mr_) {
        impl_.reset(
            std::pmr::polymorphic_allocator<>(mr_).new_object<ConcreteType>(
                listType_, mr_));
      } else {
        impl_.reset(new ConcreteType(listType_, mr_));
      }
    });
  }
  return *impl_;
}

DynamicRef List::at(size_t index) {
  if (!impl_) {
    throw std::out_of_range("List index out of range");
  }
  return (*impl_)[index];
}

DynamicConstRef List::at(size_t index) const {
  if (!impl_) {
    throw std::out_of_range("List index out of range");
  }
  return (*impl_)[index];
}

DynamicRef List::operator[](size_t index) {
  return at(index);
}

DynamicConstRef List::operator[](size_t index) const {
  return at(index);
}

void List::set(size_t index, DynamicValue value) {
  if (!impl_) {
    throw std::out_of_range("List index out of range");
  }
  impl_->set(index, std::move(value));
}

size_t List::size() const {
  return impl_ ? impl_->size() : 0;
}

bool List::isEmpty() const {
  return impl_ ? impl_->isEmpty() : true;
}

void List::push_back(DynamicValue value) {
  ensureInit().push_back(std::move(value));
}

void List::push_front(DynamicValue value) {
  ensureInit().push_front(std::move(value));
}

void List::insertAtIndex(size_t index, DynamicValue value) {
  ensureInit().insertAtIndex(index, std::move(value));
}

void List::fill(size_t count, DynamicValue value) {
  if (count == 0) {
    clear();
    return;
  }
  ensureInit().fill(count, std::move(value));
}

void List::extend(const List& other) {
  if (!other.impl_ || other.isEmpty()) {
    return;
  }
  ensureInit().extend(*other.impl_);
}

List List::slice(size_t startIdx, size_t endIdxExclusive) const {
  if (!impl_) {
    if (startIdx != 0 || endIdxExclusive != 0) {
      throw std::out_of_range("List slice indices out of range");
    }
    return List(listType_, mr_);
  }
  return List(impl_->slice(startIdx, endIdxExclusive));
}

void List::clear() {
  if (impl_) {
    impl_->clear();
  }
}

void List::reserve(size_t capacity) {
  if (capacity == 0) {
    return;
  }
  ensureInit().reserve(capacity);
}

type_system::TypeRef List::elementType() const {
  return listType_.elementType();
}

type_system::TypeRef List::type() const {
  return type_system::TypeRef(listType_);
}

List::Iterator List::begin() {
  return Iterator(this, 0);
}

List::Iterator List::end() {
  return Iterator(this, size());
}

List::ConstIterator List::begin() const {
  return ConstIterator(this, 0);
}

List::ConstIterator List::end() const {
  return ConstIterator(this, size());
}

List::ConstIterator List::cbegin() const {
  return ConstIterator(this, 0);
}

List::ConstIterator List::cend() const {
  return ConstIterator(this, size());
}

bool operator==(const List& lhs, const List& rhs) {
  if (!lhs.impl_ && !rhs.impl_) {
    return type_system::TypeRef(lhs.listType_)
        .isEqualIdentityTo(type_system::TypeRef(rhs.listType_));
  }
  if (!lhs.impl_ || !rhs.impl_) {
    return false; // One is empty, the other is not
  }
  return lhs.impl_->operator==(*rhs.impl_);
}

} // namespace apache::thrift::dynamic
