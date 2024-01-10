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

#include <stdexcept>
#include <type_traits>

#include <folly/Portability.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/Field.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>

namespace apache {
namespace thrift {
namespace op {

class bad_patch_access : public std::runtime_error {
 public:
  bad_patch_access() noexcept
      : std::runtime_error("Patch guarantees value is unset.") {}
};

namespace detail {

/// Base class for all patch types.
/// - Patch: The Thrift struct representation for the patch.
/// - Derived: The leaf type deriving from this class.
template <typename Patch, typename Derived>
class BasePatch : public type::detail::EqWrap<Derived, Patch> {
  using Base = type::detail::EqWrap<Derived, Patch>;

 public:
  using Base::Base;

  BasePatch(const BasePatch&) = default;
  BasePatch(BasePatch&&) noexcept = default;
  BasePatch& operator=(const BasePatch&) = default;
  BasePatch& operator=(BasePatch&&) noexcept = default;

  /// Applies patches to a Thrift value.
  // Automatically dereference non-optional fields.
  template <typename U>
  void apply(field_ref<U> field) const {
    derived().apply(*field);
  }
  /// Applies patches to a Thrift value.
  template <typename U>
  void apply(terse_field_ref<U> field) const {
    derived().apply(*field);
  }
  /// Applies patches to a Thrift value.
  template <typename U>
  void apply(required_field_ref<U> field) const {
    derived().apply(*field);
  }
  /// Replaces the existing value.
  template <typename U>
  type::if_not_id<U> assign(field_ref<U> val) {
    derived().assign(std::forward<U>(*val));
  }
  /// Replaces the existing value.
  template <typename U>
  type::if_not_id<U> assign(terse_field_ref<U> val) {
    derived().assign(std::forward<U>(*val));
  }
  /// Same as `assign(...)` method.
  template <typename U>
  Derived& operator=(field_ref<U> field) {
    derived().assign(std::forward<U>(*field));
    return derived();
  }
  /// Same as `assign(...)` method.
  template <typename U>
  Derived& operator=(terse_field_ref<U> field) {
    derived().assign(std::forward<U>(*field));
    return derived();
  }

  /// Merges another patch into the current patch. After the merge
  /// (`patch.merge(next)`), `patch.apply(value)` is equivalent to
  /// `next.apply(patch.apply(value))`.
  template <typename U>
  void merge(U&& next) {
    if (this == std::addressof(next)) {
      auto copy = next;
      std::forward<U>(copy).customVisit(derived());
    } else {
      std::forward<U>(next).customVisit(derived());
    }
  }
  [[deprecated("Use customVisit(...) method to read the patch.")]] auto&&
  toThrift() & {
    return Base::toThrift();
  }
  [[deprecated("Use customVisit(...) method to read the patch.")]] auto&&
  toThrift() && {
    return Base::toThrift();
  }
  [[deprecated("Use customVisit(...) method to read the patch.")]] auto&&
  toThrift() const& {
    return Base::toThrift();
  }
  [[deprecated("Use customVisit(...) method to read the patch.")]] auto&&
  toThrift() const&& {
    return Base::toThrift();
  }

  // Convert Static Patch to Dynamic Patch.
  protocol::Object toObject() const {
    // Pass the adapted type rather than the underlying thrift structure
    // directly to use `adapter::encode(...)` hook
    return protocol::asValueStruct<type::struct_c>(derived()).as_object();
  }

 protected:
  using Base::derived;
  using Base::resetAnd;

  ~BasePatch() = default; // abstract base class
};

/// Base class for assign patch types.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
template <typename Patch, typename Derived>
class BaseAssignPatch : public BasePatch<Patch, Derived> {
  using Base = BasePatch<Patch, Derived>;

 public:
  /// The type of patched value.
  using value_type =
      folly::remove_cvref_t<decltype(*std::declval<Patch>().assign())>;
  using Base::apply;
  using Base::assign;
  using Base::operator=;
  using Base::Base;

  BaseAssignPatch(const BaseAssignPatch&) = default;
  BaseAssignPatch(BaseAssignPatch&&) noexcept = default;
  BaseAssignPatch& operator=(const BaseAssignPatch&) = default;
  BaseAssignPatch& operator=(BaseAssignPatch&&) noexcept = default;

  /// Creates a new patch that replaces the existing value.
  template <typename U = value_type>
  FOLLY_NODISCARD static Derived createAssign(U&& val) {
    Derived patch;
    patch.assign(std::forward<U>(val));
    return patch;
  }

  void assign(const value_type& val) { resetAnd().assign().emplace(val); }
  void assign(value_type&& val) { resetAnd().assign().emplace(std::move(val)); }

  // A 'value' patch only applies to set optional values.
  template <typename U>
  type::if_optional_or_union_field_ref<folly::remove_cvref_t<U>> apply(
      U&& field) const {
    if (field.has_value()) {
      derived().apply(*std::forward<U>(field));
    }
  }
  // A 'value' patch only applies to set union field values.
  template <typename U>
  void apply(union_field_ref<U> field) const {
    if (field.has_value()) {
      derived().apply(*field);
    }
  }
  template <typename U>
  void apply(std::unique_ptr<U>& field) const {
    if (field) {
      derived().apply(*field);
    }
  }

  Derived& operator=(const value_type& val) { return (assign(val), derived()); }
  Derived& operator=(value_type&& val) {
    assign(std::move(val));
    return derived();
  }

 protected:
  using Base::data_;
  using Base::derived;
  using Base::resetAnd;
  ~BaseAssignPatch() = default; // abstract base class

  FOLLY_NODISCARD bool hasAssign() const { return data_.assign().has_value(); }
  FOLLY_NODISCARD value_type& assignOr(value_type& value) noexcept {
    return hasAssign() ? *data_.assign() : value;
  }
};

/// Base class for clearable patch types.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
/// * `[terse] bool clear`
template <typename Patch, typename Derived>
class BaseClearPatch : public BaseAssignPatch<Patch, Derived> {
  using Base = BaseAssignPatch<Patch, Derived>;
  using T = typename Base::value_type;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::apply;

  BaseClearPatch(const BaseClearPatch&) = default;
  BaseClearPatch(BaseClearPatch&&) noexcept = default;
  BaseClearPatch& operator=(const BaseClearPatch&) = default;
  BaseClearPatch& operator=(BaseClearPatch&&) noexcept = default;

  /// Creates a new patch that clears the value.
  FOLLY_NODISCARD static Derived createClear() {
    Derived patch;
    patch.clear();
    return patch;
  }

  // Clear resets optional fields.
  template <typename U>
  type::if_optional_or_union_field_ref<folly::remove_cvref_t<U>> apply(
      U&& field) const {
    if (data_.clear() == true && !hasAssign()) {
      field.reset();
    } else if (field.has_value()) {
      derived().apply(*std::forward<U>(field));
    }
  }

 protected:
  template <typename, typename>
  friend class BaseEnsurePatch;
  using Base::data_;
  using Base::derived;
  using Base::hasAssign;
  using Base::resetAnd;
  ~BaseClearPatch() = default;

  template <typename Visitor>
  bool customVisitAssignAndClear(Visitor&& v) const {
    if (hasAssign()) {
      std::forward<Visitor>(v).assign(*data_.assign());
      return true;
    }
    if (data_.clear() == true) {
      std::forward<Visitor>(v).clear();
    }
    return false;
  }

  /// Clears the value.
  void clear() { resetAnd().clear() = true; }
};

/// Base class for 'container' patch types.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// * `optional T assign`
/// * `[terse] bool clear`
template <typename Patch, typename Derived>
class BaseContainerPatch : public BaseClearPatch<Patch, Derived> {
  using Base = BaseClearPatch<Patch, Derived>;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::clear;

 protected:
  ~BaseContainerPatch() = default; // Abstract base class.
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
