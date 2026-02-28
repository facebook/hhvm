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

#include <thrift/lib/cpp2/dynamic/Struct.h>

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>

#include <fmt/core.h>

namespace apache::thrift::dynamic {

namespace detail {

/**
 * Interface for type-erased struct implementations.
 * Concrete implementations store and manage field data.
 */
class IStruct {
 public:
  using Ptr = std::unique_ptr<IStruct, detail::FreeDeleter>;

  virtual ~IStruct() = default;

  /**
   * Free this struct implementation. This must be called instead of delete
   * because implementations may use custom allocation strategies.
   * After calling free(), the pointer is invalid and must not be used.
   */
  virtual void free() = 0;

  /**
   * Create a deep copy of this struct implementation.
   * Returns a new IStruct with the same type and field values.
   */
  virtual Ptr clone() const = 0;

  /**
   * Get field by handle (non-const version).
   * The handle must be valid for this struct type.
   * Returns empty optional if the field is an unset optional field.
   */
  virtual std::optional<DynamicRef> getField(
      type_system::FastFieldHandle handle) = 0;

  /**
   * Get field by handle (const version).
   * The handle must be valid for this struct type.
   * Returns empty optional if the field is an unset optional field.
   */
  virtual std::optional<DynamicConstRef> getField(
      type_system::FastFieldHandle handle) const = 0;

  /**
   * Set field by handle.
   * The handle must be valid for this struct type.
   */
  virtual void setField(
      type_system::FastFieldHandle handle, DynamicValue value) = 0;

  /**
   * Check if a field is set (non-null) by handle.
   * The handle must be valid for this struct type.
   */
  virtual bool hasField(type_system::FastFieldHandle handle) const = 0;

  /**
   * Clear a field by handle (set it to null).
   * The handle must be valid for this struct type.
   */
  virtual void clearField(type_system::FastFieldHandle handle) = 0;

  /**
   * Returns the struct type.
   */
  virtual type_system::TypeRef type() const = 0;

  /**
   * Check if this struct equals another struct.
   * Both structs must have the same type identity.
   */
  virtual bool equals(const IStruct& other) const = 0;
};

/**
 * Concrete implementation of IStruct that stores field data in tailroom.
 * The Datum fields are allocated immediately after the StructImpl object
 * for better cache locality and reduced allocations.
 */
class StructImpl final : public IStruct {
 public:
  // Factory function to create StructImpl with tailroom for fields
  static std::unique_ptr<StructImpl, FreeDeleter> create(
      const type_system::StructNode& structType,
      std::pmr::memory_resource* mr = nullptr);

  // Copy/move are deleted - use create() to make a deep copy
  StructImpl(const StructImpl& other) = delete;
  StructImpl(StructImpl&& other) = delete;
  StructImpl& operator=(const StructImpl& other) = delete;
  StructImpl& operator=(StructImpl&& other) = delete;

  ~StructImpl() override;

  void free() override;

  std::unique_ptr<IStruct, FreeDeleter> clone() const override;

  std::optional<DynamicRef> getField(
      type_system::FastFieldHandle handle) override;
  std::optional<DynamicConstRef> getField(
      type_system::FastFieldHandle handle) const override;

  void setField(
      type_system::FastFieldHandle handle, DynamicValue value) override;

  bool hasField(type_system::FastFieldHandle handle) const override;

  void clearField(type_system::FastFieldHandle handle) override;

  type_system::TypeRef type() const override;

  bool equals(const IStruct& other) const override;

  std::span<const Datum> fields() const { return fields_; }

  std::span<Datum> fields() { return fields_; }

  std::pmr::memory_resource* mr_;

 private:
  // Private constructor - use create() factory function
  StructImpl(
      const type_system::StructNode& structType, std::pmr::memory_resource* mr);

  type_system::TypeRef structType_;
  std::span<Datum> fields_;
};

std::unique_ptr<StructImpl, FreeDeleter> StructImpl::create(
    const type_system::StructNode& structType, std::pmr::memory_resource* mr) {
  const size_t numFields = structType.fields().size();
  const size_t totalBytes = sizeof(StructImpl) + numFields * sizeof(Datum);

  // Allocate memory for StructImpl + tailroom for Datum fields
  void* memory = mr ? mr->allocate(totalBytes, alignof(StructImpl))
                    : std::malloc(totalBytes);

  // Construct StructImpl in the allocated memory
  StructImpl* impl = new (memory) StructImpl(structType, mr);

  // Construct Datum objects in tailroom with appropriate defaults
  Datum* fields = reinterpret_cast<Datum*>(impl + 1);
  const auto& fieldDefs = structType.fields();
  for (size_t i = 0; i < numFields; ++i) {
    const auto& fieldDef = fieldDefs[i];
    if (fieldDef.presence() == type_system::PresenceQualifier::OPTIONAL_) {
      // Optional fields start as null
      new (&fields[i]) Datum();
    } else if (fieldDef.customDefault()) {
      // Use custom default
      new (&fields[i])
          Datum(fromRecord(*fieldDef.customDefault(), fieldDef.type(), mr));
    } else {
      // Use type's default value
      auto defaultValue = DynamicValue::makeDefault(fieldDef.type(), mr);
      new (&fields[i]) Datum(std::move(defaultValue).datum_);
    }
  }

  // Return wrapped in unique_ptr with custom deleter
  return std::unique_ptr<StructImpl, FreeDeleter>(impl);
}

StructImpl::StructImpl(
    const type_system::StructNode& structType, std::pmr::memory_resource* mr)
    : mr_(mr),
      structType_(structType),
      fields_{reinterpret_cast<Datum*>(this + 1), structType.fields().size()} {}

StructImpl::~StructImpl() {
  // Manually destroy each Datum in tailroom
  for (auto& field : fields_) {
    field.~Datum();
  }
}

void StructImpl::free() {
  const size_t numFields = fields_.size();
  const size_t totalBytes = sizeof(StructImpl) + numFields * sizeof(Datum);
  auto* mr = mr_;

  // Destructor will clean up fields
  this->~StructImpl();

  // Deallocate through memory resource
  mr ? mr->deallocate(this, totalBytes, alignof(StructImpl)) : std::free(this);
}

std::unique_ptr<IStruct, FreeDeleter> StructImpl::clone() const {
  const auto& structNode = structType_.asStructUnchecked();

  // Create new impl with same type
  auto newImpl = StructImpl::create(structNode, mr_);

  // Copy all fields
  auto* newStructImpl = static_cast<StructImpl*>(newImpl.get());
  auto& newFields = newStructImpl->fields_;

  for (size_t i = 0; i < fields_.size(); ++i) {
    newFields[i] = fields_[i];
  }

  return newImpl;
}

std::optional<DynamicRef> StructImpl::getField(
    type_system::FastFieldHandle handle) {
  size_t index = handle.index();
  const auto& structNode = structType_.asStructUnchecked();
  const auto& fieldDef = structNode.fields()[index];

  // Return empty optional if this is an unset optional field
  if (fields_[index].isNull()) {
    DCHECK(fieldDef.presence() == type_system::PresenceQualifier::OPTIONAL_);
    return std::nullopt;
  }

  return DynamicRef(fieldDef.type(), fields_[index]);
}

std::optional<DynamicConstRef> StructImpl::getField(
    type_system::FastFieldHandle handle) const {
  size_t index = handle.index();
  const auto& structNode = structType_.asStructUnchecked();
  const auto& fieldDef = structNode.fields()[index];

  // Return empty optional if this is an unset optional field
  if (fields_[index].isNull()) {
    DCHECK(fieldDef.presence() == type_system::PresenceQualifier::OPTIONAL_);
    return std::nullopt;
  }

  return DynamicConstRef(fieldDef.type(), fields_[index]);
}

void StructImpl::setField(
    type_system::FastFieldHandle handle, DynamicValue value) {
  const auto& structNode = structType_.asStructUnchecked();
  const auto& fieldDef = structNode.fields()[handle.index()];
  expectType(fieldDef.type(), value.type());
  fields_[handle.index()] = std::move(value).datum_;
}

bool StructImpl::hasField(type_system::FastFieldHandle handle) const {
  size_t index = handle.index();
  const auto& structNode = structType_.asStructUnchecked();
  const auto& fieldDef = structNode.fields()[index];

  // Required fields are always considered "present"
  if (fieldDef.presence() != type_system::PresenceQualifier::OPTIONAL_) {
    return true;
  }

  // Optional fields are present only if they've been set (non-null)
  return !fields_[index].isNull();
}

void StructImpl::clearField(type_system::FastFieldHandle handle) {
  fields_[handle.index()] = Datum();
}

type_system::TypeRef StructImpl::type() const {
  return structType_;
}

bool StructImpl::equals(const IStruct& other) const {
  // Types must match
  if (!structType_.isEqualIdentityTo(other.type())) {
    return false;
  }

  // Safe to cast since we know other is also a StructImpl with same type
  const auto& otherImpl = static_cast<const StructImpl&>(other);

  // Compare all fields
  return std::equal(fields_.begin(), fields_.end(), otherImpl.fields_.begin());
}

} // namespace detail

// Destructor defined here to avoid requiring complete IStruct type in header
Struct::~Struct() = default;

Struct::Struct(IStructPtr impl) : impl_(std::move(impl)) {}

Struct::Struct(
    const type_system::StructNode& structType, std::pmr::memory_resource* mr)
    : impl_(detail::StructImpl::create(structType, mr)) {}

Struct::Struct(const Struct& other) {
  if (!other.impl_) {
    impl_ = nullptr;
  } else {
    impl_ = other.impl_->clone();
  }
}

Struct::Struct(Struct&& other) noexcept = default;

Struct& Struct::operator=(const Struct& other) {
  if (this != &other) {
    if (!other.impl_) {
      impl_ = nullptr;
    } else {
      impl_ = other.impl_->clone();
    }
  }
  return *this;
}

Struct& Struct::operator=(Struct&& other) noexcept = default;

Struct makeStruct(
    const type_system::StructNode& structType, std::pmr::memory_resource* mr) {
  return Struct(structType, mr);
}

type_system::TypeRef Struct::type() const {
  return impl_->type();
}

std::optional<DynamicRef> Struct::getField(
    type_system::FastFieldHandle handle) {
  return impl_->getField(handle);
}

std::optional<DynamicConstRef> Struct::getField(
    type_system::FastFieldHandle handle) const {
  return impl_->getField(handle);
}

std::optional<DynamicRef> Struct::getField(std::string_view name) {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format("Field '{}' not found in struct", name));
  }
  return getField(handle);
}

std::optional<DynamicConstRef> Struct::getField(std::string_view name) const {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format("Field '{}' not found in struct", name));
  }
  return getField(handle);
}

std::optional<DynamicRef> Struct::getField(FieldId id) {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in struct", static_cast<int16_t>(id)));
  }
  return getField(handle);
}

std::optional<DynamicConstRef> Struct::getField(FieldId id) const {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in struct", static_cast<int16_t>(id)));
  }
  return getField(handle);
}

void Struct::setField(std::string_view name, DynamicValue value) {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format("Field '{}' not found in struct", name));
  }
  setField(handle, std::move(value));
}

void Struct::setField(FieldId id, DynamicValue value) {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in struct", static_cast<int16_t>(id)));
  }
  setField(handle, std::move(value));
}

void Struct::setField(type_system::FastFieldHandle handle, DynamicValue value) {
  impl_->setField(handle, std::move(value));
}

bool Struct::hasField(type_system::FastFieldHandle handle) const {
  return impl_->hasField(handle);
}

bool Struct::hasField(std::string_view name) const {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format("Field '{}' not found in struct", name));
  }
  return hasField(handle);
}

bool Struct::hasField(FieldId id) const {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in struct", static_cast<int16_t>(id)));
  }
  return hasField(handle);
}

void Struct::clearOptionalField(type_system::FastFieldHandle handle) {
  const auto& structNode = type().asStructUnchecked();
  const auto& fieldDef = structNode.fields()[handle.index()];
  if (fieldDef.presence() != type_system::PresenceQualifier::OPTIONAL_) {
    throw std::runtime_error(
        fmt::format(
            "Field '{}' is not optional and cannot be cleared",
            fieldDef.identity().name()));
  }
  impl_->clearField(handle);
}

void Struct::clearOptionalField(std::string_view name) {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format("Field '{}' not found in struct", name));
  }
  clearOptionalField(handle);
}

void Struct::clearOptionalField(FieldId id) {
  const auto& structNode = type().asStructUnchecked();
  auto handle = structNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in struct", static_cast<int16_t>(id)));
  }
  clearOptionalField(handle);
}

bool operator==(const Struct& lhs, const Struct& rhs) noexcept {
  if (!lhs.impl_ || !rhs.impl_) {
    return lhs.impl_ == rhs.impl_; // Both null = true, one null = false
  }

  return lhs.impl_->equals(*rhs.impl_);
}

Struct fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::StructNode& structType,
    std::pmr::memory_resource* mr) {
  const auto& fieldSet = r.asFieldSet();
  const auto& fields = structType.fields();
  auto impl = detail::StructImpl::create(structType, mr);

  // Override defaults with values from the record
  for (size_t i = 0; i < fields.size(); ++i) {
    const auto& fieldDef = fields[i];
    auto it = fieldSet.find(fieldDef.identity().id());
    if (it != fieldSet.end()) {
      // Field is present in the record - override the default
      auto handle = type_system::FastFieldHandle{static_cast<uint16_t>(i + 1)};
      // Create DynamicValue from the record value
      DynamicValue value = fieldDef.type().visit([&](auto&& t) -> DynamicValue {
        return DynamicValue(
            fieldDef.type(),
            detail::Datum::make(fromRecord(it->second, t, mr)));
      });
      impl->setField(handle, std::move(value));
    }
  }

  return Struct(std::move(impl));
}

} // namespace apache::thrift::dynamic
