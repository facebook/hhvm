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
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteTypes.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/dynamic/detail/DatumHash.h>

#include <folly/container/F14Map.h>

#include <memory_resource>
#include <optional>

namespace apache::thrift::dynamic::detail {

// Forward declarations
struct FreeDeleter;

/**
 * Type-erased map interface.
 */
class IMap {
 public:
  using Ptr = std::unique_ptr<IMap, FreeDeleter>;

  explicit IMap(type_system::TypeRef::Map mapType) : mapType_(mapType) {}

  virtual ~IMap() = default;

  /**
   * Free this map implementation. This must be called instead of delete
   * because implementations may use custom allocation strategies.
   */
  virtual void free() = 0;

  virtual std::optional<DynamicRef> get(const DynamicConstRef& key) = 0;
  virtual std::optional<DynamicConstRef> get(
      const DynamicConstRef& key) const = 0;
  virtual void insert(DynamicValue key, DynamicValue value) = 0;
  virtual bool erase(const DynamicConstRef& key) = 0;
  virtual bool contains(const DynamicConstRef& key) const = 0;
  virtual size_t size() const = 0;
  virtual bool isEmpty() const = 0;
  virtual void clear() = 0;
  virtual void reserve(size_t capacity) = 0;
  virtual Ptr clone() const = 0;

  type_system::TypeRef keyType() const {
    return mapType_.asMapUnchecked().keyType();
  }

  type_system::TypeRef valueType() const {
    return mapType_.asMapUnchecked().valueType();
  }

  type_system::TypeRef type() const { return mapType_; }

  virtual bool operator==(const IMap& other) const = 0;

 protected:
  IMap(const IMap&) = default;
  IMap& operator=(const IMap&) = default;
  IMap(IMap&&) = default;
  IMap& operator=(IMap&&) = default;

  type_system::TypeRef mapType_;
};

/**
 * Concrete map implementation using F14FastMap.
 */
template <typename K, typename V>
class ConcreteMap final : public IMap {
 public:
  using Storage = folly::F14FastMap<
      K,
      V,
      DatumHash,
      DatumEqual,
      std::pmr::polymorphic_allocator<std::pair<const K, V>>>;

  explicit ConcreteMap(
      type_system::TypeRef::Map mapType,
      std::pmr::memory_resource* mr = nullptr)
      : IMap(std::move(mapType)),
        mr_(mr),
        elements_(mr ? mr : std::pmr::get_default_resource()) {}

  ConcreteMap(const ConcreteMap&) = default;
  ConcreteMap(ConcreteMap&&) = default;
  ConcreteMap& operator=(const ConcreteMap&) = delete;
  ConcreteMap& operator=(ConcreteMap&&) = default;
  ~ConcreteMap() override = default;

  /**
   * Direct access to the map.
   */
  Storage& elements() { return elements_; }
  const Storage& elements() const { return elements_; }

  // IMap interface implementation
  void free() override {
    if (mr_) {
      std::pmr::polymorphic_allocator<>(mr_).delete_object(this);
    } else {
      delete this;
    }
  }

  std::optional<DynamicRef> get(const DynamicConstRef& key) override {
    auto it = elements_.find(key.deref<K>());
    if (it == elements_.end()) {
      return std::nullopt;
    }
    return DynamicRef(
        this->mapType_.asMapUnchecked().valueType(),
        const_cast<V&>(it->second));
  }

  std::optional<DynamicConstRef> get(
      const DynamicConstRef& key) const override {
    auto it = elements_.find(key.deref<K>());
    if (it == elements_.end()) {
      return std::nullopt;
    }
    return DynamicConstRef(
        this->mapType_.asMapUnchecked().valueType(), it->second);
  }

  void insert(DynamicValue key, DynamicValue value) override {
    expectType(this->mapType_.asMapUnchecked().keyType(), key.type());
    expectType(this->mapType_.asMapUnchecked().valueType(), value.type());
    elements_.insert_or_assign(
        std::move(key).datum().as<K>(), std::move(value).datum().as<V>());
  }

  bool erase(const DynamicConstRef& key) override {
    return elements_.erase(key.deref<K>()) > 0;
  }

  bool contains(const DynamicConstRef& key) const override {
    return elements_.contains(key.deref<K>());
  }

  size_t size() const override { return elements_.size(); }

  bool isEmpty() const override { return elements_.empty(); }

  void clear() override { elements_.clear(); }

  void reserve(size_t capacity) override { elements_.reserve(capacity); }

  bool operator==(const IMap& other) const override {
    if (!mapType_.isEqualIdentityTo(other.type())) {
      return false;
    }
    const auto& otherMap = static_cast<const ConcreteMap&>(other);
    return elements_ == otherMap.elements_;
  }

  Ptr clone() const override {
    if (mr_) {
      return Ptr(
          std::pmr::polymorphic_allocator<>(mr_)
              .template new_object<ConcreteMap>(*this));

    } else {
      return Ptr(new ConcreteMap(*this));
    }
  }

 private:
  std::pmr::memory_resource* mr_;
  Storage elements_;
};

// Extern template declarations for all instantiated types

#define FBTHRIFT_EXTERN_TEMPLATE_CONCRETE_MAP_FOR_VALUE(KeyType, ValueType) \
  extern template class ConcreteMap<KeyType, ValueType>;

FBTHRIFT_DATUM_CONCRETE_MAP_TYPES(
    FBTHRIFT_EXTERN_TEMPLATE_CONCRETE_MAP_FOR_VALUE)

#undef FBTHRIFT_EXTERN_TEMPLATE_CONCRETE_MAP_FOR_VALUE

} // namespace apache::thrift::dynamic::detail
