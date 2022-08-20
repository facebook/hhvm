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

#include <forward_list>
#include <type_traits>
#include <utility>

#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/op/detail/ContainerPatch.h>
#include <thrift/lib/cpp2/op/detail/StructPatch.h>
#include <thrift/lib/cpp2/op/detail/ValuePatch.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// A patch for an 'optional' value.
//
// Patch must have the following fields:
//   bool clear;
//   P patchPrior;
//   optional T ensure;
//   P patchAfter;
// Where P is the patch type for a non-optional value.
template <typename Patch>
class OptionalPatch : public BaseEnsurePatch<Patch, OptionalPatch<Patch>> {
  using Base = BaseEnsurePatch<Patch, OptionalPatch>;
  using T = typename Base::value_type;
  using P = typename Base::value_patch_type;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::apply;
  using Base::assign;
  using Base::clear;
  using Base::patch;

  // Set to the given value.
  template <typename U>
  if_not_opt_type<folly::remove_cvref_t<U>> assign(U&& val) {
    clearAnd().ensure().emplace(std::forward<U>(val));
  }
  template <typename U>
  if_opt_type<folly::remove_cvref_t<U>> assign(U&& val) {
    clear();
    if (val.has_value()) {
      ensure(*std::forward<U>(val));
    }
  }
#ifdef THRIFT_HAS_OPTIONAL
  void assign(std::nullopt_t) { clear(); }
  OptionalPatch& operator=(std::nullopt_t) { return (clear(), *this); }
#endif
  template <typename U>
  if_opt_type<folly::remove_cvref_t<U>, OptionalPatch&> operator=(U&& val) {
    assign(std::forward<U>(val));
    return *this;
  }

  // Ensure value is set, initalizing to the default if required.
  FOLLY_NODISCARD static OptionalPatch createEnsure() {
    OptionalPatch patch;
    patch.ensure();
    return patch;
  }
  template <typename U = T>
  FOLLY_NODISCARD static OptionalPatch createEnsure(U&& _default) {
    OptionalPatch patch;
    patch.ensure(std::forward<U>(_default));
    return patch;
  }
  P& ensure() { return *ensureAnd().patch(); }
  P& ensure(const T& val) { return *ensureAnd(val).patch(); }
  P& ensure(T&& val) { return *ensureAnd(std::move(val)).patch(); }
  P& operator*() { return patch(); }
  P* operator->() { return &patch(); }

  bool empty() const { return emptyEnsure(); }

  template <typename U>
  if_opt_type<folly::remove_cvref_t<U>> apply(U&& val) const {
    applyEnsure(val);
  }

  // Non-optional value overload.
  //
  // Throws a op::bad_patch_access, if the resulting value should be
  // 'unset', which is not representable in a non-optional context.
  //
  // TODO(afuller): Consider also supporting union_field_ref.
  void apply(T& val) const {
    if (*data_.clear()) {
      if (!data_.ensure().has_value()) { // Cannot represent 'unset'.
        folly::throw_exception<op::bad_patch_access>();
      }
      val = *data_.ensure();
    }
    data_.patch()->apply(val);
  }

  template <typename U>
  void merge(U&& next) {
    mergeEnsure(std::forward<U>(next));
  }

 private:
  using Base::applyEnsure;
  using Base::clearAnd;
  using Base::data_;
  using Base::emptyEnsure;
  using Base::ensureAnd;
  using Base::mergeEnsure;

  Patch& ensureAnd() { return (data_.ensure().ensure(), data_); }
};

template <template <typename> class PatchType>
struct PatchAdapter {
  template <typename Patch>
  static decltype(auto) toThrift(Patch&& value) {
    return std::forward<Patch>(value).toThrift();
  }

  template <typename Patch>
  static PatchType<Patch> fromThrift(Patch&& value) {
    return PatchType<Patch>{std::forward<Patch>(value)};
  }
};

// Adapter for all base types.
using AssignPatchAdapter = PatchAdapter<AssignPatch>;
using BoolPatchAdapter = PatchAdapter<BoolPatch>;
using NumberPatchAdapter = PatchAdapter<NumberPatch>;
using StringPatchAdapter = PatchAdapter<StringPatch>;

// Adapters for structred types.
using FieldPatchAdapter = PatchAdapter<FieldPatch>;
using StructPatchAdapter = PatchAdapter<StructPatch>;
using UnionPatchAdapter = PatchAdapter<UnionPatch>;

// Adapters for containers.
using ListPatchAdapter = PatchAdapter<ListPatch>;
using SetPatchAdapter = PatchAdapter<SetPatch>;
using MapPatchAdapter = PatchAdapter<MapPatch>;

// Adapter for all optional values.
using OptionalPatchAdapter = PatchAdapter<OptionalPatch>;

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
