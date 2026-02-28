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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteTypes.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/dynamic/detail/DatumHash.h>

#include <folly/container/F14Set.h>

#include <memory_resource>

namespace apache::thrift::dynamic::detail {

/**
 * Type-erased set interface.
 */
class ISet {
 public:
  using Ptr = std::unique_ptr<ISet, FreeDeleter>;

  explicit ISet(type_system::TypeRef::Set setType) : setType_(setType) {}

  virtual ~ISet() = default;

  /**
   * Free this set implementation. This must be called instead of delete
   * because implementations may use custom allocation strategies.
   */
  virtual void free() = 0;

  virtual bool insert(DynamicValue value) = 0;
  virtual bool erase(const DynamicConstRef& value) = 0;
  virtual bool contains(const DynamicConstRef& value) const = 0;
  virtual size_t size() const = 0;
  virtual bool isEmpty() const = 0;
  virtual void clear() = 0;
  virtual void reserve(size_t capacity) = 0;
  virtual Ptr clone() const = 0;

  type_system::TypeRef elementType() const {
    return setType_.asSetUnchecked().elementType();
  }

  type_system::TypeRef type() const { return setType_; }

  virtual bool operator==(const ISet& other) const = 0;

 protected:
  ISet(const ISet&) = default;
  ISet& operator=(const ISet&) = default;
  ISet(ISet&&) = default;
  ISet& operator=(ISet&&) = default;

  type_system::TypeRef setType_;
};

/**
 * Concrete set implementation using F14FastSet.
 */
template <typename T>
class ConcreteSet final : public ISet {
 public:
  using Storage = folly::
      F14FastSet<T, DatumHash, DatumEqual, std::pmr::polymorphic_allocator<T>>;

  explicit ConcreteSet(
      type_system::TypeRef::Set setType,
      std::pmr::memory_resource* mr = nullptr)
      : ISet(std::move(setType)),
        mr_(mr),
        elements_(mr ? mr : std::pmr::get_default_resource()) {}

  ConcreteSet(const ConcreteSet&) = default;
  ConcreteSet(ConcreteSet&&) = default;
  ConcreteSet& operator=(const ConcreteSet&) = delete;
  ConcreteSet& operator=(ConcreteSet&&) = default;
  ~ConcreteSet() override = default;

  /**
   * Direct access to the set.
   */
  Storage& elements() { return elements_; }
  const Storage& elements() const { return elements_; }

  // ISet interface implementation
  void free() override {
    if (mr_) {
      std::pmr::polymorphic_allocator<>(mr_).delete_object(this);
    } else {
      delete this;
    }
  }

  bool insert(DynamicValue value) override {
    expectType(this->setType_.asSetUnchecked().elementType(), value.type());
    return elements_.insert(std::move(value).datum().as<T>()).second;
  }

  bool erase(const DynamicConstRef& value) override {
    return elements_.erase(value.deref<T>()) > 0;
  }

  bool contains(const DynamicConstRef& value) const override {
    return elements_.contains(value.deref<T>());
  }

  size_t size() const override { return elements_.size(); }

  bool isEmpty() const override { return elements_.empty(); }

  void clear() override { elements_.clear(); }

  void reserve(size_t capacity) override { elements_.reserve(capacity); }

  bool operator==(const ISet& other) const override {
    if (!setType_.isEqualIdentityTo(other.type())) {
      return false;
    }
    const auto& otherSet = static_cast<const ConcreteSet&>(other);
    return elements_ == otherSet.elements_;
  }

  Ptr clone() const override {
    if (mr_) {
      return Ptr(
          std::pmr::polymorphic_allocator<>(mr_)
              .template new_object<ConcreteSet>(*this));

    } else {
      return Ptr(new ConcreteSet(*this));
    }
  }

 private:
  std::pmr::memory_resource* mr_;
  Storage elements_;
};

// Extern template declarations for all instantiated types

#define FBTHRIFT_EXTERN_TEMPLATE_CONCRETE_SET(T) \
  extern template class ConcreteSet<T>;

FBTHRIFT_DATUM_CONCRETE_TYPES(FBTHRIFT_EXTERN_TEMPLATE_CONCRETE_SET)

#undef FBTHRIFT_EXTERN_TEMPLATE_CONCRETE_SET

} // namespace apache::thrift::dynamic::detail
