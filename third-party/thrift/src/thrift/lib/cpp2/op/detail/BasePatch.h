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

template <typename T, typename U>
if_opt_type<T, bool> sameType(const T& opt1, const U& opt2) {
  return opt1.has_value() == opt2.has_value();
}
template <typename T, typename U>
bool sameType(field_ref<T> unn1, const U& unn2) {
  return unn1->getType() == unn2.getType();
}
template <typename T, typename U>
bool sameType(terse_field_ref<T> unn1, const U& unn2) {
  return unn1->getType() == unn2.getType();
}

// Base class for all patch types.
// - Patch: The Thrift struct representation for the patch.
// - Derived: The leaf type deriving from this class.
template <typename Patch, typename Derived>
class BasePatch : public type::detail::EqWrap<Derived, Patch> {
  using Base = type::detail::EqWrap<Derived, Patch>;

 public:
  using Base::Base;

  // Automatically dereference non-optional fields.
  template <typename U>
  void apply(field_ref<U> field) const {
    derived().apply(*field);
  }
  template <typename U>
  void apply(terse_field_ref<U> field) const {
    derived().apply(*field);
  }
  template <typename U>
  type::if_not_id<U> assign(field_ref<U> val) {
    derived().assign(std::forward<U>(*val));
  }
  template <typename U>
  type::if_not_id<U> assign(terse_field_ref<U> val) {
    derived().assign(std::forward<U>(*val));
  }
  template <typename U>
  Derived& operator=(field_ref<U> field) {
    derived().assign(std::forward<U>(*field));
    return derived();
  }
  template <typename U>
  Derived& operator=(terse_field_ref<U> field) {
    derived().assign(std::forward<U>(*field));
    return derived();
  }

 protected:
  using Base::derived;
  using Base::resetAnd;

  ~BasePatch() = default; // abstract base class
};

// Base class for assign patch types.
//
// Patch must have the following fields:
//   [optional] T assign;
template <typename Patch, typename Derived>
class BaseAssignPatch : public BasePatch<Patch, Derived> {
  using Base = BasePatch<Patch, Derived>;

 public:
  using value_type =
      folly::remove_cvref_t<decltype(*std::declval<Patch>().assign())>;
  using Base::apply;
  using Base::assign;
  using Base::operator=;
  using Base::Base;

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
  if_opt_type<folly::remove_cvref_t<U>> apply(U&& field) const {
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

  FOLLY_NODISCARD bool hasAssign() const { return hasValue(data_.assign()); }
  FOLLY_NODISCARD value_type& assignOr(value_type& value) noexcept {
    return hasAssign() ? *data_.assign() : value;
  }

  bool applyAssign(value_type& val) const {
    if (hasAssign()) {
      val = *data_.assign();
      return true;
    }
    return false;
  }

  template <typename U>
  bool mergeAssign(U&& next) {
    if (hasValue(next.toThrift().assign())) {
      data_ = std::forward<U>(next).toThrift();
      return true;
    }
    if (hasAssign()) {
      next.apply(*data_.assign());
      return true;
    }
    return false;
  }
};

// Base class for clearable patch types.
//
// Patch must have the following fields:
//   (optional) T assign;
//   bool clear;
template <typename Patch, typename Derived>
class BaseClearPatch : public BaseAssignPatch<Patch, Derived> {
  using Base = BaseAssignPatch<Patch, Derived>;
  using T = typename Base::value_type;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::apply;

  FOLLY_NODISCARD static Derived createClear() {
    Derived patch;
    patch.clear();
    return patch;
  }

  // Clear resets optional fields.
  template <typename U>
  if_opt_type<folly::remove_cvref_t<U>> apply(U&& field) const {
    if (data_.clear() == true && !hasAssign()) {
      field.reset();
    } else if (field.has_value()) {
      derived().apply(*std::forward<U>(field));
    }
  }

 protected:
  template <typename>
  friend class StructPatch;
  using Base::applyAssign;
  using Base::data_;
  using Base::derived;
  using Base::hasAssign;
  using Base::mergeAssign;
  using Base::resetAnd;
  ~BaseClearPatch() = default;

  template <typename U>
  bool mergeAssignAndClear(U&& next) {
    // Clear is slightly stronger than assigning a 'cleared' value in some
    // cases. For example a struct with non-terse, non-optional fields with
    // custom defaults and missmatched schemas... it's also smaller, so prefer
    // it.
    if (*next.toThrift().clear() && !hasValue(next.toThrift().assign())) {
      // Next patch completely replaces this one.
      data_ = std::forward<U>(next).toThrift();
      return true;
    }
    return mergeAssign(std::forward<U>(next));
  }

  template <typename Tag>
  bool applyAssignAndClear(T& val) const {
    if (applyAssign(val)) {
      return true;
    }
    if (data_.clear() == true) {
      op::clear<Tag>(val);
    }
    return false;
  }

  void clear() { resetAnd().clear() = true; }
  FOLLY_NODISCARD T& clearAnd() { return (clear(), data_); }
};

// Base class for 'container' patch types.
//
// Patch must have the following fields:
//   (optional) T assign;
//   bool clear;
template <typename Patch, typename Derived>
class BaseContainerPatch : public BaseClearPatch<Patch, Derived> {
  using Base = BaseClearPatch<Patch, Derived>;
  using T = typename Base::value_type;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::clear;

 protected:
  using Base::applyAssign;
  using Base::data_;
  ~BaseContainerPatch() = default; // Abstract base class.

  // Returns true if assign was applied, and no more patchs should be applied.
  bool applyAssignOrClear(T& val) const {
    if (applyAssign(val)) {
      return true;
    }
    if (data_.clear() == true) {
      val.clear();
    }
    return false;
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
