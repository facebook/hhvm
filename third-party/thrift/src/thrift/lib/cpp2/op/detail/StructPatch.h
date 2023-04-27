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

#include <utility>

#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

/// Patch for a Thrift field.
///
/// Requires Patch have fields with ids 1:1 with the fields they patch.
template <typename Patch>
class FieldPatch : public BasePatch<Patch, FieldPatch<Patch>> {
  using Base = BasePatch<Patch, FieldPatch>;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;
  using Base::get;
  using Base::toThrift;

  template <typename T>
  static FieldPatch createFrom(T&& val) {
    FieldPatch patch;
    patch.assignFrom(std::forward<T>(val));
    return patch;
  }

  /// Returns the pointer to the Thrift patch struct.
  Patch* operator->() noexcept { return &data_; }
  const Patch* operator->() const noexcept { return &data_; }
  /// Returns the reference to the Thrift patch struct.
  Patch& operator*() noexcept { return data_; }
  const Patch& operator*() const noexcept { return data_; }

  template <typename T>
  void apply(T& val) const {
    for_each_field_id<Patch>(
        [&](auto id) { get(id)->apply(op::get<>(id, val)); });
  }

  /// @copydoc AssignPatch::merge
  template <typename U>
  void merge(U&& next) {
    auto&& tval = std::forward<U>(next).toThrift();
    for_each_field_id<Patch>([&](auto id) {
      get(id)->merge(*op::get<>(id, std::forward<decltype(tval)>(tval)));
    });
  }

 private:
  using Base::data_;

  friend bool operator==(const FieldPatch& lhs, const Patch& rhs) {
    return lhs.data_ == rhs;
  }
  friend bool operator==(const Patch& lhs, const FieldPatch& rhs) {
    return lhs == rhs.data_;
  }
  friend bool operator!=(const FieldPatch& lhs, const Patch& rhs) {
    return lhs.data_ != rhs;
  }
  friend bool operator!=(const Patch& lhs, const FieldPatch& rhs) {
    return lhs != rhs.data_;
  }
};

/// Create a base patch that supports Ensure operator.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
/// * `terse bool clear`
/// * `terse P patchPrior`
/// * `terse T ensure`
/// * `terse P patch`
/// Where `P` is the field patch type for the struct type `T`.
template <typename Patch, typename Derived>
class BaseEnsurePatch : public BaseClearPatch<Patch, Derived> {
  using Base = BaseClearPatch<Patch, Derived>;
  using T = typename Base::value_type;
  template <typename Id>
  using F = type::native_type<get_field_tag<T, Id>>;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::assign;
  /// Corresponding FieldPatch of this struct patch.
  using patch_type = std::decay_t<decltype(*std::declval<Patch>().patch())>;

  BaseEnsurePatch(const BaseEnsurePatch&) = default;
  BaseEnsurePatch(BaseEnsurePatch&&) noexcept = default;
  BaseEnsurePatch& operator=(const BaseEnsurePatch&) = default;
  BaseEnsurePatch& operator=(BaseEnsurePatch&&) noexcept = default;

  /// Returns if the patch ensures the given field is set (explicitly or
  /// implicitly).
  template <typename Id>
  constexpr bool ensures() const {
    return !isAbsent(getEnsure<Id>(data_));
  }

  /// Returns if the patch modifies the given field.
  template <typename Id>
  bool modifies() const {
    return hasAssign() || data_.clear() == true || getEnsure<Id>(data_) ||
        !getRawPatch<Id>(data_.patchPrior()).empty() ||
        !getRawPatch<Id>(data_.patch()).empty();
  }

  template <typename Id, typename... Ids>
  std::enable_if_t<sizeof...(Ids) != 0, bool> modifies() const {
    // If hasAssign() == true, the whole struct (all fields) will be replaced.
    if (hasAssign() || data_.clear() == true || getEnsure<Id>(data_)) {
      return true;
    }

    return getRawPatch<Id>(data_.patchPrior()).template modifies<Ids...>() ||
        getRawPatch<Id>(data_.patch()).template modifies<Ids...>();
  }

  /// Ensures the given field is set, and return the associated patch object.
  template <typename Id>
  decltype(auto) ensure() {
    return (maybeEnsure<Id>(), patchAfter<Id>());
  }
  /// Same as `ensure()` method, except uses the provided default value.
  template <typename Id, typename U = F<Id>>
  decltype(auto) ensure(U&& defaultVal) {
    if (maybeEnsure<Id>()) {
      getEnsure<Id>(data_) = std::forward<U>(defaultVal);
    }
    return patchAfter<Id>();
  }
  /// Ensures the given field is initalized, and return the associated patch
  /// object.
  template <typename Id>
  decltype(auto) patch() {
    return ensure<Id>();
  }

 protected:
  using Base::data_;
  using Base::hasAssign;
  ~BaseEnsurePatch() = default;

  // Clears the field with the given id.
  template <typename Id>
  void clear() {
    if (hasAssign()) {
      op::clear<Id>(*data_.assign());
      return;
    }
    patchPrior<Id>().clear();
    op::clear<Id>(*data_.ensure());
    patchAfter<Id>().reset();
  }
  using Base::clear;

  template <typename Id, typename U>
  static decltype(auto) getEnsure(U&& data) {
    return op::get<Id>(*std::forward<U>(data).ensure());
  }

  template <typename Id>
  decltype(auto) patchPrior() {
    return (ensurePatchable(), getRawPatch<Id>(data_.patchPrior()));
  }

  template <typename Id>
  decltype(auto) patchAfter() {
    return (ensurePatchable(), getRawPatch<Id>(data_.patch()));
  }

  void ensurePatchable() {
    if (data_.assign().has_value()) {
      for_each_field_id<T>([&](auto id) {
        using Id = decltype(id);
        auto&& field = op::get<>(id, *data_.assign());
        auto&& prior = getRawPatch<Id>(data_.patchPrior());
        auto&& ensure = op::get<>(id, *data_.ensure());
        auto&& after = getRawPatch<Id>(data_.patch());
        if (isAbsent(field)) {
          prior.toThrift().clear() = true;
        } else {
          ensure = {};
          after.assign(std::move(*field));
        }
      });
      // Unset assign.
      data_.assign().reset();
    }
  }

  template <typename Id>
  bool maybeEnsure() {
    if (*patchAfter<Id>().toThrift().clear()) {
      // Since we cleared the field in PatchAfter, we should remove any existing
      // ensured value.
      op::clear<Id>(*data_.ensure());
    }
    if (ensures<Id>()) {
      return false;
    }
    // Merge anything (oddly) in patchAfter into patchPrior.
    if (!patchAfter<Id>().empty()) {
      patchPrior<Id>().merge(std::move(patchAfter<Id>()));
      patchAfter<Id>().reset();
    }
    getEnsure<Id>(data_).ensure();
    return true;
  }

 private:
  template <typename Id, typename U>
  decltype(auto) getRawPatch(U&& patch) const {
    // Field Ids must always be used to access patch(Prior).
    return *patch->get(get_field_id<T, Id>{});
  }
};

/// Patch for a Thrift struct.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
/// * `terse bool clear`
/// * `terse P patchPrior`
/// * `terse T ensure`
/// * `terse P patch`
/// Where `P` is the field patch type for the struct type `T`.
template <typename Patch>
class StructPatch : public BaseEnsurePatch<Patch, StructPatch<Patch>> {
  using Base = BaseEnsurePatch<Patch, StructPatch>;
  using T = typename Base::value_type;
  template <typename Id>
  using F = type::native_type<get_field_tag<T, Id>>;

 public:
  using Base::apply;
  using Base::assign;
  using Base::Base;
  using Base::operator=;
  using patch_type = std::decay_t<decltype(*std::declval<Patch>().patch())>;

  void clear() {
    Base::clear();
    // Custom defaults must also be cleared.
    op::clear<>(*data_.ensure());
  }
  template <typename Id>
  void clear() {
    Base::template clear<Id>();
  }

  /// Assigns to the given field, ensuring first if needed.
  template <typename Id, typename U = F<Id>>
  void assign(U&& val) {
    if (hasValue(data_.assign())) {
      op::get<Id>(*data_.assign()) = std::forward<U>(val);
    } else {
      Base::template ensure<Id>().assign(std::forward<U>(val));
    }
  }

  /// Returns the proper patch object for the given field.
  template <typename Id>
  decltype(auto) patchIfSet() {
    return Base::template ensures<Id>() ? patchAfter<Id>() : patchPrior<Id>();
  }

  void apply(T& val) const {
    if (applyAssign(val)) {
      return;
    }

    // Apply clear, patchPrior, and ensure.
    if (*data_.clear()) {
      op::clear<type::infer_tag<T>>(val);
      for_each_field_id<T>([&](auto id) { // ensure
        auto&& defaultVal = op::get<>(id, *data_.ensure());
        if (defaultVal) {
          op::get<>(id, val) = *defaultVal;
        }
      });
    } else {
      data_.patchPrior()->apply(val); // patchPrior
      for_each_field_id<T>([&](auto id) { // ensure
        auto&& field = op::get<>(id, val);
        auto&& defaultVal = op::get<>(id, *data_.ensure());
        if (isAbsent(field) && !isAbsent(defaultVal)) {
          field = *defaultVal;
        }
      });
    }

    // Apply patchAfter.
    data_.patch()->apply(val);
  }

  /// @copydoc AssignPatch::merge
  template <typename U>
  void merge(U&& next) {
    if (mergeAssignAndClear(std::forward<U>(next))) {
      return; // Complete replacement.
    }

    // Field-wise merge for patchPrior, ensure, and patchAfter
    // next.assign and next.clear known to be empty.
    for_each_field_id<T>([&](auto id) {
      using Id = decltype(id);
      if (next.toThrift().patchPrior()->get(id)->toThrift().clear() == true) {
        // Complete replacement
        patchPrior<Id>() =
            *std::forward<U>(next).toThrift().patchPrior()->get(id);
        resetValue(getEnsure<Id>(data_));
      } else if (ensures<Id>()) {
        // All values will be set before next, so ignore next.ensure and
        // merge next.patchPrior and next.patch into this.patch.
        auto temp = *std::forward<U>(next).toThrift().patch()->get(id);
        patchAfter<Id>().merge(
            *std::forward<U>(next).toThrift().patchPrior()->get(id));
        patchAfter<Id>().merge(std::move(temp));
        return;
      } else {
        // Merge anything in patchAfter into patchPrior.
        patchPrior<Id>().merge(std::move(patchAfter<Id>()));
        // Merge in next.patchPrior into patchPrior.
        patchPrior<Id>().merge(
            *std::forward<U>(next).toThrift().patchPrior()->get(id));
      }

      // Consume next.ensure, if any.
      if (next.template ensures<decltype(id)>()) {
        getEnsure<Id>(data_) =
            *op::get<Id>(*std::forward<U>(next).toThrift().ensure());
      }

      // Consume next.patchAfter.
      patchAfter<Id>() = *std::forward<U>(next).toThrift().patch()->get(id);
    });
  }

 private:
  using Base::applyAssign;
  using Base::data_;
  using Base::get;
  using Base::mergeAssignAndClear;

  // We can not use `using Base::patchPrior` since using-declaration can't be
  // used to introduce the name of a dependent member template as a
  // template-name.
  // https://en.cppreference.com/w/cpp/language/using_declaration#Notes
  template <typename Id>
  decltype(auto) patchPrior() {
    return Base::template patchPrior<Id>();
  }

  template <typename Id>
  decltype(auto) ensures() const {
    return Base::template ensures<Id>();
  }

  template <typename Id, typename U>
  static decltype(auto) getEnsure(U&& data) {
    return Base::template getEnsure<Id, U>(std::forward<U>(data));
  }

  template <typename Id>
  decltype(auto) patchAfter() {
    return Base::template patchAfter<Id>();
  }
};

/// Patch for a Thrift union.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
/// * `terse bool clear`
/// * `terse P patchPrior`
/// * `terse T ensure`
/// * `terse P patch`
/// Where `P` is the field patch type for the union type `T`.
template <typename Patch>
class UnionPatch : public BaseEnsurePatch<Patch, UnionPatch<Patch>> {
  using Base = BaseEnsurePatch<Patch, UnionPatch>;
  using T = typename Base::value_type;
  using P = typename Base::patch_type;
  template <typename Id>
  using F = type::native_type<get_field_tag<T, Id>>;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::apply;
  using Base::assign;
  using Base::clear;
  using Base::ensure;

  /// Creates a new patch that ensures the union with a given value.
  template <typename U = T>
  FOLLY_NODISCARD static UnionPatch createEnsure(U&& _default) {
    UnionPatch patch;
    patch.ensure(std::forward<U>(_default));
    return patch;
  }
  /// Returns the union that's used to ensure.
  T& ensure() { return *data_.ensure(); }
  /// Ensures the union with a given value.
  P& ensure(const T& val) { return *ensureAnd(val).patch(); }
  /// Ensures the union with a given value.
  P& ensure(T&& val) { return *ensureAnd(std::move(val)).patch(); }

  /// Assigns to the given field, ensuring first if needed.
  template <typename Id, typename U = F<Id>>
  void assign(U&& val) {
    op::get<Id>(Base::resetAnd().assign().ensure()) = std::forward<U>(val);
  }

  /// Patch any set value.
  FOLLY_NODISCARD P& patchIfSet() {
    if (hasValue(data_.ensure())) {
      return *data_.patch();
    } else if (*data_.clear()) {
      folly::throw_exception<bad_patch_access>();
    }
    return *data_.patchPrior();
  }

  void apply(T& val) const {
    if (applyAssign(val)) {
      return;
    }
    // Clear, ensure or patchPrior.
    if (*data_.clear()) {
      clearValue(val);
    } else if (hasValue(data_.ensure()) && !sameType(data_.ensure(), val)) {
      val = *data_.ensure();
    } else {
      data_.patchPrior()->apply(val);
    }
    // Apply the patch after ensure.
    data_.patch()->apply(val);
  }

  /// @copydoc AssignPatch::merge
  template <typename U>
  void merge(U&& next) {
    if (mergeAssignAndClear(std::forward<U>(next))) {
      return; // Complete replacement.
    }

    // Merge patchPrior, ensure, and patchAfter.
    // next.assign and next.clear known to be empty.

    if (hasValue(data_.ensure()) &&
        (!hasValue(next.toThrift().ensure()) ||
         (data_.ensure()->getType() == next.toThrift().ensure()->getType()))) {
      // next.ensure has the same set member (or is unset),
      // so it is safe to merge next.patchPrior and next.patch into this.patch
      auto temp = *std::forward<U>(next).toThrift().patch();
      data_.patch()->merge(*std::forward<U>(next).toThrift().patchPrior());
      data_.patch()->merge(std::move(temp));
    } else {
      auto temp = *std::forward<U>(next).toThrift().patchPrior();
      // Merge anything in patch into patchPrior.
      data_.patchPrior()->merge(std::move(*data_.patch()));
      // Merge in next.patchPrior into patchPrior.
      data_.patchPrior()->merge(std::move(temp));
      // Consume next.ensure, if any.
      if (hasValue(next.toThrift().ensure())) {
        data_.ensure() = *std::forward<U>(next).toThrift().ensure();
      }
      // Consume next.patch.
      data_.patch() = *std::forward<U>(next).toThrift().patch();
    }
  }

 private:
  using Base::applyAssign;
  using Base::data_;
  using Base::ensurePatchable;
  using Base::mergeAssignAndClear;
  using Base::resetAnd;

  template <typename U = T>
  Patch& ensureAnd(U&& _default) {
    ensurePatchable();
    assert(!op::isEmpty<>(_default));
    if (!hasValue(data_.ensure())) {
      data_.ensure().emplace(std::forward<U>(_default));
    }
    return data_;
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
