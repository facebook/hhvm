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
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteMap.h>

#include <memory_resource>

namespace apache::thrift::dynamic {

namespace {
// Helper to invoke a lambda with a concrete map type from a TypeRef.
template <typename F>
decltype(auto) withConcreteType(const type_system::TypeRef::Map& type, F&& f) {
  return type.keyType().matchKind(
      [&]<type_system::TypeRef::Kind kKind>(
          type_system::TypeRef::KindConstant<kKind>) {
        using KeyType = detail::type_of_type_kind<kKind>;

        return type.valueType().matchKind(
            [&]<type_system::TypeRef::Kind vKind>(
                type_system::TypeRef::KindConstant<vKind>) {
              using ValueType = detail::type_of_type_kind<vKind>;
              using ConcreteType = detail::ConcreteMap<KeyType, ValueType>;
              return std::forward<F>(f).template operator()<ConcreteType>();
            });
      });
}
} // namespace

Map makeMap(
    type_system::TypeRef::Map mapType, std::pmr::memory_resource* allocator) {
  return Map(mapType, allocator);
}

Map fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::TypeRef::Map& mapType,
    std::pmr::memory_resource* alloc) {
  auto ret = makeMap(mapType, alloc);

  const auto& mapRecords = r.asMap();
  const type_system::TypeRef keyType = mapType.keyType();
  const type_system::TypeRef valueType = mapType.valueType();

  for (const auto& entry : mapRecords) {
    const type_system::SerializableRecord& keyRecord = entry.first;
    const type_system::SerializableRecord& valueRecord = entry.second;
    auto keyDatum = keyType.visit([&](auto&& t) {
      return detail::Datum::make(fromRecord(keyRecord, t, alloc));
    });
    auto valueDatum = valueType.visit([&](auto&& t) {
      return detail::Datum::make(fromRecord(valueRecord, t, alloc));
    });
    ret.insert(
        DynamicValue(keyType, std::move(keyDatum)),
        DynamicValue(valueType, std::move(valueDatum)));
  }

  return ret;
}

// Map method implementations
Map::Map(type_system::TypeRef::Map mapType, std::pmr::memory_resource* mr)
    : mapType_(mapType), mr_(mr), impl_(nullptr) {}

Map::Map(detail::IMap::Ptr impl)
    : mapType_(impl->type().asMapUnchecked()),
      mr_(nullptr),
      impl_(std::move(impl)) {}

Map::Map(const Map& other)
    : mapType_(other.mapType_),
      mr_(other.mr_),
      impl_(other.impl_ ? other.impl_->clone() : nullptr) {}

Map::Map(Map&&) noexcept = default;

Map::~Map() = default;

Map& Map::operator=(const Map& other) {
  if (this != &other) {
    mapType_ = other.mapType_;
    mr_ = other.mr_;
    impl_ = other.impl_ ? other.impl_->clone() : nullptr;
  }
  return *this;
}

Map& Map::operator=(Map&&) noexcept = default;

detail::IMap& Map::ensureInit() {
  if (!impl_) {
    withConcreteType(mapType_, [this]<typename ConcreteType>() {
      if (mr_) {
        impl_.reset(
            std::pmr::polymorphic_allocator<>(mr_).new_object<ConcreteType>(
                mapType_, mr_));
      } else {
        impl_.reset(new ConcreteType(mapType_, mr_));
      }
    });
  }
  return *impl_;
}

std::optional<DynamicRef> Map::get(const DynamicConstRef& key) {
  if (!impl_) {
    return std::nullopt;
  }
  return impl_->get(key);
}

std::optional<DynamicConstRef> Map::get(const DynamicConstRef& key) const {
  if (!impl_) {
    return std::nullopt;
  }
  return impl_->get(key);
}

void Map::insert(DynamicValue key, DynamicValue value) {
  ensureInit().insert(std::move(key), std::move(value));
}

bool Map::erase(const DynamicConstRef& key) {
  if (!impl_) {
    return false;
  }
  return impl_->erase(key);
}

bool Map::contains(const DynamicConstRef& key) const {
  if (!impl_) {
    return false;
  }
  return impl_->contains(key);
}

size_t Map::size() const {
  return impl_ ? impl_->size() : 0;
}

bool Map::isEmpty() const {
  return impl_ ? impl_->isEmpty() : true;
}

void Map::clear() {
  if (impl_) {
    impl_->clear();
  }
}

void Map::reserve(size_t capacity) {
  if (capacity == 0) {
    return;
  }
  ensureInit().reserve(capacity);
}

type_system::TypeRef Map::keyType() const {
  return mapType_.keyType();
}

type_system::TypeRef Map::valueType() const {
  return mapType_.valueType();
}

type_system::TypeRef Map::type() const {
  return type_system::TypeRef(mapType_);
}

// Template constructor definitions - must be before begin()/end() where they're
// used
template <typename IterType>
Map::Iterator::Iterator(IterType&& it, const type_system::TypeRef::Map* mapType)
    : mapType_(mapType) {
  concreteIt_.emplace<std::decay_t<IterType>>(std::forward<IterType>(it));
}

template <typename IterType>
Map::ConstIterator::ConstIterator(
    IterType&& it, const type_system::TypeRef::Map* mapType)
    : mapType_(mapType) {
  concreteIt_.emplace<std::decay_t<IterType>>(std::forward<IterType>(it));
}

// Iterator implementations
Map::Iterator Map::begin() {
  if (!impl_) {
    // Empty map - return end iterator
    return Iterator(std::byte{}, nullptr);
  }

  return withConcreteType(mapType_, [this]<typename T>() -> Iterator {
    auto& concreteMap = static_cast<T&>(*impl_);
    return Iterator(concreteMap.elements().begin(), &this->mapType_);
  });
}

Map::Iterator Map::end() {
  if (!impl_) {
    // Empty map - return end iterator
    return Iterator(std::byte{}, nullptr);
  }

  return withConcreteType(mapType_, [this]<typename T>() -> Iterator {
    auto& concreteMap = static_cast<T&>(*impl_);
    return Iterator(concreteMap.elements().end(), &this->mapType_);
  });
}

Map::ConstIterator Map::begin() const {
  if (!impl_) {
    // Empty map - return end iterator
    return ConstIterator(std::byte{}, nullptr);
  }

  return withConcreteType(mapType_, [this]<typename T>() -> ConstIterator {
    auto& concreteMap = static_cast<T&>(*impl_);
    // Note: F14FastMap doesn't have const_iterator, so we cast away
    // const
    return ConstIterator(
        const_cast<T&>(concreteMap).elements().begin(), &this->mapType_);
  });
}

Map::ConstIterator Map::end() const {
  if (!impl_) {
    // Empty map - return end iterator
    return ConstIterator(std::byte{}, nullptr);
  }

  return withConcreteType(mapType_, [this]<typename T>() -> ConstIterator {
    auto& concreteMap = static_cast<T&>(*impl_);
    // Note: F14FastMap doesn't have const_iterator, so we cast away
    // const
    return ConstIterator(
        const_cast<T&>(concreteMap).elements().end(), &this->mapType_);
  });
}

Map::ConstIterator Map::cbegin() const {
  return begin();
}

Map::ConstIterator Map::cend() const {
  return end();
}

Map::Iterator::Iterator() : mapType_(nullptr) {}

Map::Iterator::Iterator(const Iterator& other) = default;

Map::Iterator::Iterator(Iterator&&) noexcept = default;

Map::Iterator& Map::Iterator::operator=(const Iterator& other) = default;

Map::Iterator& Map::Iterator::operator=(Iterator&&) noexcept = default;

std::pair<DynamicConstRef, DynamicRef> Map::Iterator::operator*() {
  return {key(), value()};
}

DynamicConstRef Map::Iterator::key() {
  return withConcreteType(*mapType_, [this]<typename T>() -> DynamicConstRef {
    using IterType = typename T::Storage::iterator;
    auto& it = concreteIt_.as<IterType>();
    return DynamicConstRef(mapType_->keyType(), it->first);
  });
}

DynamicRef Map::Iterator::value() {
  return withConcreteType(*mapType_, [this]<typename T>() -> DynamicRef {
    using IterType = typename T::Storage::iterator;
    auto& it = concreteIt_.as<IterType>();
    return DynamicRef(mapType_->valueType(), it->second);
  });
}

Map::Iterator& Map::Iterator::operator++() {
  withConcreteType(*mapType_, [this]<typename T>() {
    using IterType = typename T::Storage::iterator;
    ++concreteIt_.as<IterType>();
  });
  return *this;
}

Map::Iterator Map::Iterator::operator++(int) {
  Iterator tmp = *this;
  ++(*this);
  return tmp;
}

bool Map::Iterator::operator==(const Iterator& other) const {
  if (!mapType_ || !other.mapType_) {
    // Nullptr is used for end iterators for empty maps
    return mapType_ == other.mapType_;
  }

  return withConcreteType(*mapType_, [&, this]<typename T>() -> bool {
    using IterType = typename T::Storage::iterator;
    return concreteIt_.as<IterType>() == other.concreteIt_.as<IterType>();
  });
}

Map::ConstIterator::ConstIterator() : mapType_(nullptr) {}

Map::ConstIterator::ConstIterator(const ConstIterator& other) = default;

Map::ConstIterator::ConstIterator(ConstIterator&&) noexcept = default;

Map::ConstIterator& Map::ConstIterator::operator=(const ConstIterator& other) =
    default;

Map::ConstIterator& Map::ConstIterator::operator=(ConstIterator&&) noexcept =
    default;

std::pair<DynamicConstRef, DynamicConstRef> Map::ConstIterator::operator*()
    const {
  return {key(), value()};
}

DynamicConstRef Map::ConstIterator::key() const {
  return withConcreteType(*mapType_, [this]<typename T>() -> DynamicConstRef {
    using IterType = typename T::Storage::iterator;
    auto& it = concreteIt_.as<IterType>();
    return DynamicConstRef(mapType_->keyType(), it->first);
  });
}

DynamicConstRef Map::ConstIterator::value() const {
  return withConcreteType(*mapType_, [this]<typename T>() -> DynamicConstRef {
    using IterType = typename T::Storage::iterator;
    auto& it = concreteIt_.as<IterType>();
    return DynamicConstRef(mapType_->valueType(), it->second);
  });
}

Map::ConstIterator& Map::ConstIterator::operator++() {
  withConcreteType(*mapType_, [this]<typename T>() {
    using IterType = typename T::Storage::iterator;
    ++concreteIt_.as<IterType>();
  });
  return *this;
}

Map::ConstIterator Map::ConstIterator::operator++(int) {
  ConstIterator tmp = *this;
  ++(*this);
  return tmp;
}

bool Map::ConstIterator::operator==(const ConstIterator& other) const {
  if (!mapType_ || !other.mapType_) {
    // Nullptr is used for end iterators for empty maps
    return mapType_ == other.mapType_;
  }
  return withConcreteType(*mapType_, [&, this]<typename T>() -> bool {
    using IterType = typename T::Storage::iterator;
    return concreteIt_.as<IterType>() == other.concreteIt_.as<IterType>();
  });
}

bool operator==(const Map& lhs, const Map& rhs) {
  if (!lhs.impl_ && !rhs.impl_) {
    return type_system::TypeRef(lhs.mapType_)
        .isEqualIdentityTo(type_system::TypeRef(rhs.mapType_));
  }
  if (!lhs.impl_ || !rhs.impl_) {
    return false; // One is empty, the other is not
  }
  return lhs.impl_->operator==(*rhs.impl_);
}

} // namespace apache::thrift::dynamic
