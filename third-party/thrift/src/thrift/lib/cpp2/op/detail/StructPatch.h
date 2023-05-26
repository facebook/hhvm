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
#include <thrift/lib/cpp2/op/Clear.h>
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

  template <typename Visitor>
  void customVisit(Visitor&& v) const {
    for_each_field_id<Patch>(
        [&](auto id) { v.template patchIfSet<decltype(id)>(*get(id)); });
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
  using FieldType = type::native_type<get_field_tag<T, Id>>;

  // Needed to access patchIfSet(...) for merge(...) method
  template <class>
  friend class FieldPatch;

  struct Applier {
    T& v;

    void assign(const T& t) { v = t; }
    void clear() { ::apache::thrift::clear(v); }

    template <class Id, class FieldPatch>
    void patchIfSet(const FieldPatch& patch) {
      patch.apply(op::get<Id>(v));
    }

    template <class Id, class Field>
    void ensure(const Field& def) {
      if (isAbsent(op::get<Id>(v))) {
        op::get<Id>(v) = def;
      }
    }
  };

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
  void ensure() {
    maybeEnsure<Id>();
  }
  /// Same as `ensure()` method, except uses the provided default value.
  template <typename Id, typename U = FieldType<Id>>
  void ensure(U&& defaultVal) {
    if (maybeEnsure<Id>()) {
      getEnsure<Id>(data_) = std::forward<U>(defaultVal);
    }
  }
  /// Ensures the given field is initalized, and return the associated patch
  /// object.
  template <typename Id>
  decltype(auto) patch() {
    return (maybeEnsure<Id>(), patchAfter<Id>());
  }

  /// Returns the proper patch object for the given field.
  template <typename Id>
  decltype(auto) patchIfSet() {
    return ensures<Id>() ? patchAfter<Id>() : patchPrior<Id>();
  }

  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(const MyClass&);
  ///       void clear();
  ///       template<class Id> void patchIfSet(const FieldPatch&);
  ///       template<class Id> void ensure(const FieldPatch&);
  ///     }
  ///
  /// For example, let's assume you have the following thrift struct:
  ///
  ///     struct MyClass {
  ///       1: string foo;
  ///       2: bool bar;
  ///     }
  ///
  /// and then you created the following patch:
  ///
  ///     MyClassPatch patch;
  ///     patch.patch<ident::bar>().invert();
  ///     patch.patch<ident::bar>().invert();
  ///     patch.patch<ident::foo>().append("_");
  ///
  /// `patch.customVisit(v)` will invoke the following methods
  ///
  ///     v.ensure<ident::foo>();
  ///     v.ensure<ident::bar>();
  ///     v.patchIfSet<ident::foo>(StringPatch::createAppend("_"));
  ///     v.patchIfSet<ident::bar>(BoolPatch{});  // no-op since inverted twice
  template <typename Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      for_each_field_id<T>([&](auto id) {
        using Id = decltype(id);
        using FieldPatchType =
            folly::remove_cvref_t<decltype(BaseEnsurePatch{}.patch<Id>())>;

        v.template patchIfSet<Id>(FieldPatchType{});
        v.template ensure<Id>(FieldType<Id>{});
        v.template patchIfSet<Id>(FieldPatchType{});
      });
    }

    if (Base::template customVisitAssignAndClear(std::forward<Visitor>(v))) {
      return;
    }

    data_.patchPrior()->customVisit(std::forward<Visitor>(v));

    // TODO: Optimize ensure for UnionPatch
    for_each_field_id<T>([&](auto id) {
      if (auto p = op::get<>(id, *data_.ensure())) {
        std::forward<Visitor>(v).template ensure<decltype(id)>(*p);
      }
    });

    data_.patch()->customVisit(std::forward<Visitor>(v));
  }

  void apply(T& val) const { return customVisit(Applier{val}); }

 protected:
  using Base::apply;
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

  // Needed for merge(...). We can consider making this a public API.
  template <typename Id, typename FieldPatch>
  void patchIfSet(const FieldPatch& p) {
    patchIfSet<Id>().merge(p);
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
      Base::template patch<Id>().assign(std::forward<U>(val));
    }
  }

 private:
  using Base::data_;
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

 private:
  using Base::data_;
  using Base::ensurePatchable;
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
