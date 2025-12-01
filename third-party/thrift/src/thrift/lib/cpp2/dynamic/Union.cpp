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

#include <thrift/lib/cpp2/dynamic/Union.h>

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>

#include <fmt/core.h>

namespace apache::thrift::dynamic {

Union::~Union() {
  deleteDatumPtr(activeFieldData_);
}

detail::Datum* Union::makeDatumPtr(detail::Datum&& datum) {
  if (mr_) {
    std::pmr::polymorphic_allocator<> alloc(mr_);
    return alloc.new_object<detail::Datum>(std::move(datum));
  } else {
    return new detail::Datum(std::move(datum));
  }
}

void Union::deleteDatumPtr(detail::Datum* ptr) {
  if (ptr) {
    if (mr_) {
      std::pmr::polymorphic_allocator<> alloc(mr_);
      alloc.delete_object(ptr);
    } else {
      delete ptr;
    }
  }
}

Union::Union(
    const type_system::UnionNode& unionType, std::pmr::memory_resource* mr)
    : unionType_(unionType),
      mr_(mr),
      activeFieldData_(nullptr),
      activeFieldDef_(nullptr) {}

Union::Union(const Union& other)
    : unionType_(other.unionType_),
      mr_(other.mr_),
      activeFieldData_(
          other.activeFieldData_
              ? makeDatumPtr(detail::Datum(*other.activeFieldData_))
              : nullptr),
      activeFieldDef_(other.activeFieldDef_) {}

Union::Union(Union&& other) noexcept
    : unionType_(other.unionType_),
      mr_(other.mr_),
      activeFieldData_(other.activeFieldData_),
      activeFieldDef_(other.activeFieldDef_) {
  other.activeFieldData_ = nullptr;
  other.activeFieldDef_ = nullptr;
}

Union& Union::operator=(const Union& other) {
  if (this != &other) {
    deleteDatumPtr(activeFieldData_);
    unionType_ = other.unionType_;
    activeFieldData_ = other.activeFieldData_
        ? makeDatumPtr(detail::Datum(*other.activeFieldData_))
        : nullptr;
    activeFieldDef_ = other.activeFieldDef_;
    mr_ = other.mr_;
  }
  return *this;
}

Union& Union::operator=(Union&& other) noexcept {
  if (this != &other) {
    deleteDatumPtr(activeFieldData_);
    unionType_ = other.unionType_;
    activeFieldData_ = other.activeFieldData_;
    activeFieldDef_ = other.activeFieldDef_;
    mr_ = other.mr_;
    other.activeFieldData_ = nullptr;
    other.activeFieldDef_ = nullptr;
  }
  return *this;
}

Union makeUnion(
    const type_system::UnionNode& unionType, std::pmr::memory_resource* mr) {
  return Union(unionType, mr);
}

DynamicRef Union::getField(type_system::FastFieldHandle handle) {
  const auto& unionNode = unionType_.asUnionUnchecked();
  const auto& fieldDef = unionNode.fields()[handle.index()];

  if (activeFieldDef_ != &fieldDef) {
    throw std::runtime_error(
        fmt::format(
            "Field '{}' is not the active field in union",
            fieldDef.identity().name()));
  }

  return DynamicRef(fieldDef.type(), *activeFieldData_);
}

DynamicConstRef Union::getField(type_system::FastFieldHandle handle) const {
  const auto& unionNode = unionType_.asUnionUnchecked();
  const auto& fieldDef = unionNode.fields()[handle.index()];

  if (activeFieldDef_ != &fieldDef) {
    throw std::runtime_error(
        fmt::format(
            "Field '{}' is not the active field in union",
            fieldDef.identity().name()));
  }

  return DynamicConstRef(fieldDef.type(), *activeFieldData_);
}

DynamicRef Union::getField(std::string_view name) {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(fmt::format("Field '{}' not found in union", name));
  }
  return getField(handle);
}

DynamicConstRef Union::getField(std::string_view name) const {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(fmt::format("Field '{}' not found in union", name));
  }
  return getField(handle);
}

DynamicRef Union::getField(FieldId id) {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in union", static_cast<int16_t>(id)));
  }
  return getField(handle);
}

DynamicConstRef Union::getField(FieldId id) const {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in union", static_cast<int16_t>(id)));
  }
  return getField(handle);
}

void Union::setField(std::string_view name, DynamicValue value) {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(fmt::format("Field '{}' not found in union", name));
  }
  setField(handle, std::move(value));
}

void Union::setField(FieldId id, DynamicValue value) {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in union", static_cast<int16_t>(id)));
  }
  setField(handle, std::move(value));
}

void Union::setField(type_system::FastFieldHandle handle, DynamicValue value) {
  const auto& unionNode = unionType_.asUnionUnchecked();
  const auto& fieldDef = unionNode.fields()[handle.index()];
  detail::expectType(fieldDef.type(), value.type());

  deleteDatumPtr(activeFieldData_);
  activeFieldDef_ = &fieldDef;
  activeFieldData_ = makeDatumPtr(std::move(value).datum_);
}

bool Union::hasField(type_system::FastFieldHandle handle) const {
  if (!activeFieldDef_) {
    return false;
  }
  const auto& unionNode = unionType_.asUnionUnchecked();
  const auto& fieldDef = unionNode.fields()[handle.index()];
  return activeFieldDef_ == &fieldDef;
}

bool Union::hasField(std::string_view name) const {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(name);
  if (!handle.valid()) {
    throw std::out_of_range(fmt::format("Field '{}' not found in union", name));
  }
  return hasField(handle);
}

bool Union::hasField(FieldId id) const {
  const auto& unionNode = unionType_.asUnionUnchecked();
  auto handle = unionNode.fieldHandleFor(id);
  if (!handle.valid()) {
    throw std::out_of_range(
        fmt::format(
            "Field with id {} not found in union", static_cast<int16_t>(id)));
  }
  return hasField(handle);
}

type_system::FastFieldHandle Union::activeField() const {
  if (!activeFieldDef_) {
    return type_system::FastFieldHandle();
  }
  const auto& unionNode = unionType_.asUnionUnchecked();
  return unionNode.fieldHandleFor(activeFieldDef_->identity().id());
}

void Union::clear() {
  deleteDatumPtr(activeFieldData_);
  activeFieldData_ = nullptr;
  activeFieldDef_ = nullptr;
}

bool operator==(const Union& lhs, const Union& rhs) noexcept {
  if (!lhs.unionType_.isEqualIdentityTo(rhs.unionType_)) {
    return false;
  }

  if (lhs.activeFieldDef_ != rhs.activeFieldDef_) {
    return false;
  }

  if (!lhs.activeFieldDef_) {
    return true;
  }

  return *lhs.activeFieldData_ == *rhs.activeFieldData_;
}

Union fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::UnionNode& unionType,
    std::pmr::memory_resource* mr) {
  const auto& fieldSet = r.asFieldSet();
  const auto& fields = unionType.fields();
  Union ret(unionType, mr);

  if (fieldSet.empty()) {
    return ret;
  }

  if (fieldSet.size() > 1) {
    throw std::runtime_error(
        "Union cannot have more than one active field in record");
  }

  const auto& [fieldId, fieldValue] = *fieldSet.begin();

  for (size_t i = 0; i < fields.size(); ++i) {
    const auto& fieldDef = fields[i];
    if (fieldDef.identity().id() == fieldId) {
      ret.activeFieldDef_ = &fieldDef;
      ret.activeFieldData_ = fieldDef.type().visit([&](auto&& t) {
        return ret.makeDatumPtr(
            detail::Datum::make(fromRecord(fieldValue, t, mr)));
      });
      break;
    }
  }

  if (!ret.activeFieldDef_) {
    throw std::runtime_error(
        fmt::format(
            "Unknown field id {} in union record",
            static_cast<int16_t>(fieldId)));
  }

  return ret;
}

} // namespace apache::thrift::dynamic
