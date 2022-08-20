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

// Helpers for detecting compatible optional types.
template <typename T>
struct is_optional_type : std::false_type {};
template <typename T>
struct is_optional_type<optional_field_ref<T>> : std::true_type {};
template <typename T>
struct is_optional_type<optional_boxed_field_ref<T>> : std::true_type {};
#ifdef THRIFT_HAS_OPTIONAL
template <typename T>
struct is_optional_type<std::optional<T>> : std::true_type {};
#endif

template <typename T, typename R = void>
using if_opt_type = std::enable_if_t<is_optional_type<T>::value, R>;
template <typename T, typename R = void>
using if_not_opt_type = std::enable_if_t<!is_optional_type<T>::value, R>;

template <typename T>
if_opt_type<T, bool> hasValue(const T& opt) {
  return opt.has_value();
}
template <typename T>
bool hasValue(field_ref<T> val) {
  return !thrift::empty(*val);
}
template <typename T>
bool hasValue(terse_field_ref<T> val) {
  return !thrift::empty(*val);
}
// TODO: use op::clear and op::ensure to avoid duplication
template <typename T>
if_opt_type<T> clearValue(T& opt) {
  opt.reset();
}
template <typename T>
if_not_opt_type<T> clearValue(T& unn) {
  thrift::clear(unn);
}
template <typename T>
if_opt_type<T> ensureValue(T& opt) {
  if (!opt.has_value()) {
    opt.emplace();
  }
}
template <typename T>
void ensureValue(field_ref<T> val) {
  val.ensure();
}
template <typename T>
void ensureValue(terse_field_ref<T>) {
  // A terse field doesn't have a set or unset state, so ensure is a noop.
}
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
class BasePatch : public type::detail::Wrap<Patch> {
  using Base = type::detail::Wrap<Patch>;

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
  void assign(field_ref<U> val) {
    derived().assign(std::forward<U>(*val));
  }
  template <typename U>
  void assign(terse_field_ref<U> val) {
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
  using Base::resetAnd;
  ~BasePatch() = default; // abstract base class

  Derived& derived() { return static_cast<Derived&>(*this); }
  const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

// Base class for value patch types.
//
// Patch must have the following fields:
//   optional T assign;
template <typename Patch, typename Derived>
class BaseValuePatch : public BasePatch<Patch, Derived> {
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

  ~BaseValuePatch() = default; // abstract base class

  FOLLY_NODISCARD value_type& assignOr(value_type& value) noexcept {
    return hasValue(data_.assign()) ? *data_.assign() : value;
  }

  bool applyAssign(value_type& val) const {
    if (hasValue(data_.assign())) {
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
    if (hasValue(data_.assign())) {
      next.apply(*data_.assign());
      return true;
    }
    return false;
  }
};

// Base class for clearable value patch types.
//
// Patch must have the following fields:
//   optional T assign;
//   bool clear;
template <typename Patch, typename Derived>
class BaseClearValuePatch : public BaseValuePatch<Patch, Derived> {
  using Base = BaseValuePatch<Patch, Derived>;
  using T = typename Base::value_type;

 public:
  using Base::Base;
  using Base::operator=;

  FOLLY_NODISCARD static Derived createClear() {
    Derived patch;
    patch.clear();
    return patch;
  }

  void clear() { resetAnd().clear() = true; }

 protected:
  using Base::applyAssign;
  using Base::data_;
  using Base::mergeAssign;
  using Base::resetAnd;

  ~BaseClearValuePatch() = default;

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
};

// Patch must have the following fields:
//   bool clear;
//   P patchPrior;
//   (optional) T ensure;
//   P patch;
template <typename Patch, typename Derived>
class BaseEnsurePatch : public BasePatch<Patch, Derived> {
  using Base = BasePatch<Patch, Derived>;

 public:
  using value_type =
      folly::remove_cvref_t<decltype(*std::declval<Patch>().ensure())>;
  using value_patch_type =
      folly::remove_cvref_t<decltype(*std::declval<Patch>().patch())>;
  using Base::assign;
  using Base::operator=;
  using Base::Base;

  // Ensure the value is set to the given value.
  template <typename U = value_type>
  FOLLY_NODISCARD static Derived createAssign(U&& val) {
    Derived patch;
    patch.assign(std::forward<U>(val));
    return patch;
  }
  void assign(const value_type& val) { clearAnd().ensure().emplace(val); }
  void assign(value_type&& val) { clearAnd().ensure().emplace(std::move(val)); }
  Derived& operator=(const value_type& val) { return (assign(val), derived()); }
  Derived& operator=(value_type&& val) {
    assign(std::move(val));
    return derived();
  }

  // Unset any value.
  FOLLY_NODISCARD static Derived createClear() {
    Derived patch;
    patch.clear();
    return patch;
  }
  void clear() { resetAnd().clear() = true; }

  // Patch any set value.
  FOLLY_NODISCARD value_patch_type& patch() {
    if (hasValue(data_.ensure())) {
      return *data_.patch();
    } else if (*data_.clear()) {
      folly::throw_exception<bad_patch_access>();
    }
    return *data_.patchPrior();
  }

 protected:
  using Base::data_;
  using Base::derived;
  using Base::resetAnd;

  ~BaseEnsurePatch() = default;

  Patch& clearAnd() { return (clear(), data_); }
  template <typename U = value_type>
  Patch& ensureAnd(U&& _default) {
    if (!hasValue(data_.ensure())) {
      data_.ensure().emplace(std::forward<U>(_default));
    }
    return data_;
  }

  bool emptyEnsure() const {
    return !*data_.clear() && data_.patchPrior()->empty() &&
        !hasValue(data_.ensure()) && data_.patch()->empty();
  }

  template <typename U>
  bool mergeEnsure(U&& next) {
    if (*next.toThrift().clear()) {
      if (hasValue(next.toThrift().ensure())) {
        data_.clear() = true;
        data_.patchPrior()->reset(); // We can ignore next.patch.
        data_.ensure() = *std::forward<U>(next).toThrift().ensure();
        data_.patch() = *std::forward<U>(next).toThrift().patch();
      } else {
        clear(); // We can ignore everything else.
      }
      return true; // It's a complete replacement.
    }

    if (hasValue(data_.ensure())) {
      // All values will be set before next, so ignore next.ensure and
      // merge next.patch and next.patch into this.patch.
      auto temp = *std::forward<U>(next).toThrift().patch();
      data_.patch()->merge(*std::forward<U>(next).toThrift().patchPrior());
      data_.patch()->merge(std::move(temp));
    } else { // Both this.ensure and next.clear are known to be empty.
      // Merge anything (oddly) in patch into patch.
      data_.patchPrior()->merge(std::move(*data_.patch()));
      // Merge in next.patch into patch.
      data_.patchPrior()->merge(*std::forward<U>(next).toThrift().patchPrior());
      // Consume next.ensure, if any.
      if (hasValue(next.toThrift().ensure())) {
        data_.ensure() = *std::forward<U>(next).toThrift().ensure();
      }
      // Consume next.patch.
      data_.patch() = *std::forward<U>(next).toThrift().patch();
    }
    return false;
  }

  template <typename U>
  void applyEnsure(U& val) const {
    // Clear or patch.
    if (*data_.clear()) {
      clearValue(val);
    } else {
      data_.patchPrior()->apply(val);
    }
    // Ensure if needed.
    if (hasValue(data_.ensure()) && !sameType(data_.ensure(), val)) {
      val = *data_.ensure();
    }
    // Apply the patch after ensure.
    data_.patch()->apply(val);
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
