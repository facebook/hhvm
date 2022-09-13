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

// Requires Patch have fields with ids 1:1 with the fields they patch.
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

  Patch* operator->() noexcept { return &data_; }
  const Patch* operator->() const noexcept { return &data_; }
  Patch& operator*() noexcept { return data_; }
  const Patch& operator*() const noexcept { return data_; }

  template <typename T>
  void apply(T& val) const {
    for_each_field_id<Patch>(
        [&](auto id) { get(id)->apply(op::get<>(id, val)); });
  }

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

// Patch must have the following fields:
//   optional T assign;
//   bool clear;
//   P patchPrior;
//   T ensure;
//   P patch;
template <typename Patch>
class StructPatch : public BaseClearPatch<Patch, StructPatch<Patch>> {
  using Base = BaseClearPatch<Patch, StructPatch>;
  using T = typename Base::value_type;
  template <typename Id>
  using F = type::native_type<get_field_tag<Id, T>>;

 public:
  using Base::apply;
  using Base::assign;
  using Base::Base;
  using Base::hasAssign;
  using Base::operator=;
  using patch_type = std::decay_t<decltype(*std::declval<Patch>().patch())>;

  void clear() {
    Base::clear();
    // Custom defaults must also be cleared.
    op::clear<type::infer_tag<T>>(*data_.ensure());
  }

  template <typename Id>
  void clear() {
    if (hasAssign()) {
      clearValue(op::get<Id>(*data_.assign()));
      return;
    }
    patchPrior<Id>().clear();
    clearValue(getEnsure<Id>(data_));
    patchAfter<Id>().reset();
  }

  // Assigns to the given field, ensuring first if needed.
  template <typename Id, typename U = F<Id>>
  void assign(U&& val) {
    if (hasValue(data_.assign())) {
      op::get<Id>(*data_.assign()) = std::forward<U>(val);
    } else {
      ensure<Id>().assign(std::forward<U>(val));
    }
  }

  // Returns the proper patch object for the given field.
  template <typename Id>
  decltype(auto) patchIfSet() {
    return ensured<Id>() ? patchAfter<Id>() : patchPrior<Id>();
  }

  // Ensures the given field is set, and return the associated patch object.
  template <typename Id>
  decltype(auto) ensure() {
    return (maybeEnsure<Id>(), patchAfter<Id>());
  }
  // Same as above, except uses the provided default value.
  template <typename Id, typename U = F<Id>>
  decltype(auto) ensure(U&& defaultVal) {
    if (maybeEnsure<Id>()) {
      getEnsure<Id>(data_) = std::forward<U>(defaultVal);
    }
    return patchAfter<Id>();
  }

  // Returns if the given field is ensured (explicitly or implicitly).
  template <typename Id>
  constexpr decltype(auto) ensured() const {
    return !isAbsent(getEnsure<Id>(data_));
  }

  void apply(T& val) const {
    if (applyAssign(val)) {
      return;
    }

    // Apply clear, patchPrior, and ensure.
    if (*data_.clear()) {
      val = *data_.ensure(); // clear + ensure.
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
      } else if (ensured<Id>()) {
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
      if (next.template ensured<decltype(id)>()) {
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
  template <typename Id>
  using as_id = get_field_id<Id, T>;

  template <typename Id>
  decltype(auto) patchPrior() {
    ensurePatchable();
    // Field Ids must always be used to access patchPrior.
    return *data_.patchPrior()->get(as_id<Id>{});
  }

  template <typename Id, typename U>
  static decltype(auto) getEnsure(U&& data) {
    return op::get<Id>(*data.ensure());
  }

  template <typename Id>
  bool maybeEnsure() {
    if (ensured<Id>()) {
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

  template <typename Id>
  decltype(auto) patchAfter() {
    ensurePatchable();
    // Field Ids must always be used to access patch(After).
    return *data_.patch()->get(as_id<Id>{});
  }

  void ensurePatchable() {
    if (data_.assign().has_value()) {
      // Ensure even unknown fields are cleared, and ensure is used as a
      // complete replancement.
      *data_.clear() = true;
      data_.ensure() = std::move(*data_.assign());
      // Unset assign.
      data_.assign().reset();
    }
  }
};

// A patch for an union value.
//
// Patch must have the following fields:
//   bool clear;
//   P patchPrior;
//   T ensure;
//   P patch;
// Where P is the patch type for the union type T.
// TODO(afuller): Add 'assign' support and inherit from BaseClearPatch
template <typename Patch>
class UnionPatch : public BasePatch<Patch, UnionPatch<Patch>> {
  using Base = BasePatch<Patch, UnionPatch<Patch>>;
  using T = folly::remove_cvref_t<decltype(*std::declval<Patch>().ensure())>;
  using P = folly::remove_cvref_t<decltype(*std::declval<Patch>().patch())>;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::apply;
  using value_type = T;
  using patch_type = P;

  template <typename U = T>
  FOLLY_NODISCARD static UnionPatch createEnsure(U&& _default) {
    UnionPatch patch;
    patch.ensure(std::forward<U>(_default));
    return patch;
  }
  T& ensure() { return *data_.ensure(); }
  P& ensure(const T& val) { return *ensureAnd(val).patch(); }
  P& ensure(T&& val) { return *ensureAnd(std::move(val)).patch(); }

  // Ensure the value is set to the given value.
  template <typename U = value_type>
  FOLLY_NODISCARD static UnionPatch createAssign(U&& val) {
    UnionPatch patch;
    patch.assign(std::forward<U>(val));
    return patch;
  }
  void assign(const value_type& val) { clearAnd().ensure().emplace(val); }
  void assign(value_type&& val) { clearAnd().ensure().emplace(std::move(val)); }
  UnionPatch& operator=(const value_type& val) {
    return (assign(val), derived());
  }
  UnionPatch& operator=(value_type&& val) {
    assign(std::move(val));
    return derived();
  }

  // Unset any value.
  FOLLY_NODISCARD static UnionPatch createClear() {
    UnionPatch patch;
    patch.clear();
    return patch;
  }
  void clear() { resetAnd().clear() = true; }

  // Patch any set value.
  FOLLY_NODISCARD P& patchIfSet() {
    if (hasValue(data_.ensure())) {
      return *data_.patch();
    } else if (*data_.clear()) {
      folly::throw_exception<bad_patch_access>();
    }
    return *data_.patchPrior();
  }

  void apply(T& val) const { applyEnsure(val); }

  // A 'union' patch only applies to set optional-union fields.
  template <typename U>
  if_opt_type<folly::remove_cvref_t<U>> apply(U&& field) const {
    if (field.has_value()) {
      apply(*std::forward<U>(field));
    }
  }

  // A 'union' patch only applies to set union-union fields.
  template <typename U>
  void apply(union_field_ref<U> field) const {
    if (field.has_value()) {
      apply(*field);
    }
  }

  template <typename U>
  void merge(U&& next) {
    mergeEnsure(std::forward<U>(next));
  }

 private:
  using Base::data_;
  using Base::derived;
  using Base::resetAnd;

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
        data_.patchPrior()->reset(); // We can ignore next.patchPrior.
        data_.ensure() = *std::forward<U>(next).toThrift().ensure();
        data_.patch() = *std::forward<U>(next).toThrift().patch();
      } else {
        clear(); // We can ignore everything else.
      }
      return true; // It's a complete replacement.
    }

    if (hasValue(data_.ensure())) {
      // All values will be set before next, so ignore next.ensure and
      // merge next.patchPrior and next.patch into this.patch.
      auto temp = *std::forward<U>(next).toThrift().patch();
      data_.patch()->merge(*std::forward<U>(next).toThrift().patchPrior());
      data_.patch()->merge(std::move(temp));
    } else { // Both this.ensure and next.clear are known to be empty.
      // Merge anything (oddly) in patch into patchPrior.
      data_.patchPrior()->merge(std::move(*data_.patch()));
      // Merge in next.patchPrior into patchPrior.
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
