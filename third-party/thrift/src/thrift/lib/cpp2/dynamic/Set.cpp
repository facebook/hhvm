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
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteSet.h>

#include <memory_resource>

namespace apache::thrift::dynamic {

namespace {
// Helper to invoke a lambda with a concrete set type from a TypeRef.
template <typename F>
auto withConcreteType(const type_system::TypeRef::Set& type, F&& f) {
  return type.elementType().matchKind(
      [&]<type_system::TypeRef::Kind k>(type_system::TypeRef::KindConstant<k>) {
        using T = detail::type_of_type_kind<k>;
        using ConcreteType = detail::ConcreteSet<T>;
        return std::forward<F>(f).template operator()<ConcreteType>();
      });
}
} // namespace

Set makeSet(
    type_system::TypeRef::Set setType, std::pmr::memory_resource* allocator) {
  return Set(setType, allocator);
}

// Set method implementations
Set::Set(type_system::TypeRef::Set setType, std::pmr::memory_resource* mr)
    : setType_(setType), mr_(mr), impl_(nullptr) {}

Set::Set(detail::ISet::Ptr impl)
    : setType_(impl->type().asSetUnchecked()),
      mr_(nullptr),
      impl_(std::move(impl)) {}

Set::Set(const Set& other)
    : setType_(other.setType_),
      mr_(other.mr_),
      impl_(other.impl_ ? other.impl_->clone() : nullptr) {}

Set::Set(Set&&) noexcept = default;

Set::~Set() = default;

Set& Set::operator=(const Set& other) {
  if (this != &other) {
    setType_ = other.setType_;
    mr_ = other.mr_;
    impl_ = other.impl_ ? other.impl_->clone() : nullptr;
  }
  return *this;
}

Set& Set::operator=(Set&&) noexcept = default;

detail::ISet& Set::ensureInit() {
  if (!impl_) {
    withConcreteType(setType_, [this]<typename ConcreteType>() {
      if (mr_) {
        impl_.reset(
            std::pmr::polymorphic_allocator<>(mr_).new_object<ConcreteType>(
                setType_, mr_));
      } else {
        impl_.reset(new ConcreteType(setType_, mr_));
      }
    });
  }
  return *impl_;
}

bool Set::insert(DynamicValue value) {
  return ensureInit().insert(std::move(value));
}

bool Set::erase(const DynamicConstRef& value) {
  if (!impl_) {
    return false;
  }
  return impl_->erase(value);
}

bool Set::contains(const DynamicConstRef& value) const {
  if (!impl_) {
    return false;
  }
  return impl_->contains(value);
}

size_t Set::size() const {
  return impl_ ? impl_->size() : 0;
}

bool Set::isEmpty() const {
  return impl_ ? impl_->isEmpty() : true;
}

void Set::clear() {
  if (impl_) {
    impl_->clear();
  }
}

void Set::reserve(size_t capacity) {
  if (capacity == 0) {
    return;
  }
  ensureInit().reserve(capacity);
}

type_system::TypeRef Set::elementType() const {
  return setType_.elementType();
}

type_system::TypeRef Set::type() const {
  return type_system::TypeRef(setType_);
}

bool operator==(const Set& lhs, const Set& rhs) {
  if (!lhs.impl_ && !rhs.impl_) {
    return type_system::TypeRef(lhs.setType_)
        .isEqualIdentityTo(type_system::TypeRef(rhs.setType_));
  }
  if (!lhs.impl_ || !rhs.impl_) {
    return false; // One is empty, the other is not
  }
  return lhs.impl_->operator==(*rhs.impl_);
}

} // namespace apache::thrift::dynamic
