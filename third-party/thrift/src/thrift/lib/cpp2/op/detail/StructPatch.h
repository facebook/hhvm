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

#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/TagUtil.h>

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
    op::for_each_field_id<Patch>(
        [&](auto id) { get(id)->apply(get(id, val)); });
  }

  template <typename U>
  void merge(U&& next) {
    auto&& tval = std::forward<U>(next).toThrift();
    op::for_each_field_id<Patch>([&](auto id) {
      get(id)->merge(*get(id, std::forward<decltype(tval)>(tval)));
    });
  }

 private:
  using Base::data_;

  friend bool operator==(const FieldPatch& lhs, const FieldPatch& rhs) {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator==(const FieldPatch& lhs, const Patch& rhs) {
    return lhs.data_ == rhs;
  }
  friend bool operator==(const Patch& lhs, const FieldPatch& rhs) {
    return lhs == rhs.data_;
  }
  friend bool operator!=(const FieldPatch& lhs, const FieldPatch& rhs) {
    return lhs.data_ != rhs.data_;
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
//   P patch;
template <typename Patch>
class StructPatch : public BaseClearValuePatch<Patch, StructPatch<Patch>> {
  using Base = BaseClearValuePatch<Patch, StructPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;
  using patch_type = std::decay_t<decltype(*std::declval<Patch>().patch())>;

  // Convert to a patch, if needed, and return the
  // patch object.
  patch_type& patch() { return ensurePatch(); }
  auto* operator->() { return patch().operator->(); }

  void apply(T& val) const {
    if (applyAssign(val)) {
      return;
    }
    if (*data_.clear()) {
      thrift::clear(val);
    } else {
      data_.patchPrior()->apply(val);
    }
    data_.patch()->apply(val);
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      auto temp = *std::forward<U>(next).toThrift().patch();
      data_.patch()->merge(*std::forward<U>(next).toThrift().patchPrior());
      data_.patch()->merge(std::move(temp));
    }
  }

 private:
  using Base::applyAssign;
  using Base::data_;
  using Base::get;
  using Base::mergeAssignAndClear;

  patch_type& ensurePatch() {
    if (data_.assign().has_value()) {
      // Ensure even unknown fields are cleared.
      *data_.clear() = true;

      // Split the assignment patch into a patch of assignments.
      op::for_each_field_id<T>([&](auto id) {
        data_.patch()->get(id)->assign(get(id, std::move(*data_.assign())));
      });
      // Unset assign.
      data_.assign().reset();
    }
    return *data_.patch();
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
template <typename Patch>
class UnionPatch : public BaseEnsurePatch<Patch, UnionPatch<Patch>> {
  using Base = BaseEnsurePatch<Patch, UnionPatch>;
  using T = typename Base::value_type;
  using P = typename Base::value_patch_type;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::apply;

  template <typename U = T>
  FOLLY_NODISCARD static UnionPatch createEnsure(U&& _default) {
    UnionPatch patch;
    patch.ensure(std::forward<U>(_default));
    return patch;
  }
  T& ensure() { return *data_.ensure(); }
  P& ensure(const T& val) { return *ensureAnd(val).patch(); }
  P& ensure(T&& val) { return *ensureAnd(std::move(val)).patch(); }

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
  using Base::applyEnsure;
  using Base::data_;
  using Base::ensureAnd;
  using Base::mergeEnsure;
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
