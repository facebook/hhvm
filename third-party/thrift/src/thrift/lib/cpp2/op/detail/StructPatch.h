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
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace apache {
namespace thrift {
namespace ident {
struct remove;
}
namespace op {
namespace detail {

struct FieldIdListToSetAdapter {
  using FieldIdSet = std::unordered_set<FieldId>;
  using FieldIdList = std::vector<std::int16_t>;
  static FieldIdSet fromThrift(const FieldIdList& v) {
    FieldIdSet ret;
    ret.reserve(v.size());
    for (auto i : v) {
      ret.emplace(static_cast<FieldId>(i));
    }
    return ret;
  }
  static FieldIdList toThrift(const FieldIdSet& v) {
    FieldIdList ret;
    ret.reserve(v.size());
    for (auto i : v) {
      ret.emplace_back(folly::to_underlying(i));
    }
    return ret;
  }
};

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

    template <class>
    void ensure() {
      // For non-optional field, ensure is no-op
    }

    template <class Id, class Field>
    void ensure(const Field& def) {
      if (isAbsent(op::get<Id>(v))) {
        op::get<Id>(v) = def;
      }
    }
  };

  template <class Id>
  static constexpr bool is_optional_or_union_field_v =
      is_optional_type<get_field_ref<T, Id>>::value || is_thrift_union_v<T>;

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
    // Ensuring non-optional field to intrinsic default is allowed since we
    // might want to ensure field in case the field doesn't exist in dynamic
    // value. (e.g., Terse field with default value. Without ensuring it first,
    // we will not be able to patch such field at all).
    maybeEnsure<Id>();
  }
  /// Same as `ensure()` method, except uses the provided default value.
  template <typename Id, typename U = FieldType<Id>>
  std::enable_if_t<is_optional_or_union_field_v<Id>> ensure(U&& defaultVal) {
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
    ensurePatchable();
    if constexpr (!is_thrift_union_v<T>) {
      if (Base::derived().template isRemoved<Id>()) {
        // If field is already cleared, Patch should be ignored.
        getRawPatch<Id>(data_.patch()).toThrift().clear() = true;
        return getRawPatch<Id>(data_.patchPrior());
      }
    }
    return ensures<Id>() ? getRawPatch<Id>(data_.patch())
                         : getRawPatch<Id>(data_.patchPrior());
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
        v.template ensure<Id>();
        if constexpr (is_optional_or_union_field_v<Id>) {
          v.template ensure<Id>(FieldType<Id>{});
        }
        v.template patchIfSet<Id>(FieldPatchType{});
      });
    }

    if (Base::template customVisitAssignAndClear(std::forward<Visitor>(v))) {
      return;
    }

    data_.patchPrior()->customVisit(std::forward<Visitor>(v));

    // TODO: Optimize ensure for UnionPatch
    for_each_field_id<T>([&](auto id) {
      using Id = decltype(id);
      if (auto p = op::get<Id>(*data_.ensure())) {
        if constexpr (is_optional_or_union_field_v<Id>) {
          std::forward<Visitor>(v).template ensure<Id>(*p);
        } else {
          std::forward<Visitor>(v).template ensure<Id>();
        }
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

  friend class BaseEnsurePatch<Patch, StructPatch<Patch>>;

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

  template <class Protocol>
  uint32_t encode(Protocol& prot) const {
    uint32_t s = 0;
    s += prot.writeStructBegin(op::get_class_name_v<Patch>.data());
    const auto remove = removedFields();
    op::for_each_field_id<Patch>([&](auto id) {
      using Id = decltype(id);
      using Tag = op::get_type_tag<Patch, Id>;
      constexpr bool isRemoveField =
          std::is_same<get_ident<Patch, Id>, ident::remove>::value;

      auto&& field = op::get<Id>(data_);

      if (!isRemoveField && !should_write<Tag>(field)) {
        return;
      }

      if (isRemoveField && remove.empty()) {
        return;
      }

      s += prot.writeFieldBegin(
          &*op::get_name_v<Patch, Id>.begin(),
          typeTagToTType<Tag>,
          folly::to_underlying(Id::value));
      s += op::encode<Tag>(
          prot, folly::if_constexpr<isRemoveField>(remove, *field));
      s += prot.writeFieldEnd();
    });
    s += prot.writeFieldStop();
    s += prot.writeStructEnd();
    return s;
  }

  ~StructPatch() {
    if (false) {
      // Implement this check in destructor to make sure it's instantiated.
      op::for_each_ordinal<T>([](auto id) {
        static_assert(
            !apache::thrift::detail::is_shared_or_unique_ptr_v<
                op::get_field_ref<T, decltype(id)>>,
            "Patching cpp.ref field is unsupported since we cannot distinguish "
            "unqualified and optional fields. Why? The type of `foo.field()` "
            "is `std::unique_ptr` regardless Whether field is optional or "
            "unqualified. In addition, Thrift Patch has different behavior "
            "between optional and unqualified fields, e.g. `PatchOp::Clear` "
            "will clear an optional field, but set an unqualified field to the "
            "intrinsic default.");
      });
    }
  }

 private:
  using Base::data_;

  // Whether the field is removed
  template <class Id>
  bool isRemoved() const {
    const auto& prior = data_.patchPrior()->toThrift();
    const auto& ensure = *data_.ensure();
    const auto& after = data_.patch()->toThrift();

    using Ref = folly::remove_cvref_t<decltype(get<Id>(std::declval<T>()))>;
    if (!is_optional_type<Ref>::value) {
      // non-optional fields can not be removed
      return false;
    }

    if (*get<Id>(after)->toThrift().clear()) {
      // Cleared field in patch after
      return true;
    }

    if (*get<Id>(prior)->toThrift().clear() && !get<Id>(ensure).has_value()) {
      // Cleared field in patch prior and not ensured
      return true;
    }

    return false;
  }

  // Combine fields from PatchOp::Clear and PatchOp::Remove operations
  std::unordered_set<FieldId> removedFields() const {
    auto removed = *data_.remove();
    op::for_each_field_id<T>([&](auto id) {
      if (isRemoved<decltype(id)>()) {
        removed.insert(id.value);
      }
    });
    return removed;
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
  template <typename Id>
  using F = type::native_type<get_field_tag<T, Id>>;

 public:
  using Base::Base;
  using Base::operator=;
  using Base::apply;
  using Base::assign;
  using Base::clear;

  /// Assigns to the given field, ensuring first if needed.
  template <typename Id, typename U = F<Id>>
  void assign(U&& val) {
    op::get<Id>(Base::resetAnd().assign().ensure()) = std::forward<U>(val);
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
