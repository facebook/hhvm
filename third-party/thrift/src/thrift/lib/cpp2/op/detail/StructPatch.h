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
#include <thrift/lib/cpp2/op/detail/PatchTraits.h>
#include <thrift/lib/cpp2/type/Field.h>
#include <thrift/lib/cpp2/type/Id.h>

namespace apache::thrift::op::detail {

template <typename T>
struct SafePatchType;

template <typename SafePatch, typename Patch>
SafePatch toSafePatchImpl(const Patch& patch) {
  SafePatch safePatch;

  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  op::encode<type::infer_tag<Patch>>(writer, patch);

  safePatch.data() = queue.move();
  safePatch.version() = calculateMinSafePatchVersion(patch);

  return safePatch;
}

template <typename Patch, typename SafePatch>
Patch fromSafePatchImpl(const SafePatch& safePatch) {
  if (safePatch.version() == 0) {
    throw std::runtime_error("Invalid Safe Patch");
  }
  if (safePatch.version() > detail::kThriftStaticPatchVersion) {
    throw std::runtime_error(
        fmt::format("Unsupported patch version: {}", *safePatch.version()));
  }
  Patch patch;
  CompactProtocolReader reader;
  reader.setInput(safePatch.data()->get());
  op::decode<type::infer_tag<Patch>>(reader, patch);
  return patch;
}

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

template <typename Tag>
struct FieldPatchAssigner {
  template <typename Patch, typename Val>
  void operator()(Patch& patch, Val&& val) {
    patch.assign(std::forward<Val>(val));
  }
};

// Special cases FieldPatch.assign(...) for fields with adapters
template <typename Adapter, typename Tag>
struct FieldPatchAssigner<type::adapted<Adapter, Tag>> {
  template <typename Patch, typename Val>
  void operator()(Patch& patch, Val&& val) {
    using value_type = typename Patch::value_type;
    if constexpr (std::is_same_v<value_type, folly::remove_cvref_t<Val>>) {
      // Maintain compatbility with legacy AnyPatch
      patch.assign(std::forward<Val>(val));
    } else {
      patch.assign(Adapter::toThrift(std::forward<Val>(val)));
    }
  }
};

template <typename Tag>
struct PatchIfSetApplier {
  template <typename Patch, typename FieldRef>
  void operator()(const Patch& patch, FieldRef&& ref) {
    patch.apply(std::forward<FieldRef>(ref));
  }
};

// Special cases FieldPatch.apply(...) for fields with adapters
template <typename Adapter, typename Tag>
struct PatchIfSetApplier<type::adapted<Adapter, Tag>> {
  template <typename Patch, typename FieldRef>
  void operator()(const Patch& patch, FieldRef&& ref) {
    using value_type = typename Patch::value_type;
    if constexpr (std::is_same_v<
                      value_type,
                      type::native_type<type::adapted<Adapter, Tag>>>) {
      // Usually patch itself should only understand the standard type. It
      // should have zero knowledge of the adapted type. (e.g., the `assign`
      // field inside patch struct should be standard type, not adapted type).
      // We should always invoke the adapter when applying the patch.
      //
      // However, for some weird custom patch (Cough cough...
      // https://fburl.com/code/pmubnhge) that can be applied
      // to the adapted type directly, we just apply it directly without
      // adapter.
      patch.apply(std::forward<FieldRef>(ref));
    } else {
      if (auto* value = get_value_or_null(ref)) {
        type::native_type<Tag> temp = Adapter::toThrift(std::move(*value));
        patch.apply(temp);
        ref = Adapter::fromThrift(std::move(temp));
      }
    }
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
    // Don't visit the field if it is empty.
    for_each_field_id<Patch>([&](auto id) {
      if (auto& p = *get(id); !p.empty()) {
        v.template patchIfSet<decltype(id)>(p);
      }
    });
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
      PatchIfSetApplier<get_type_tag<T, Id>>{}(patch, get<Id>(v));
    }

    template <class>
    void ensure() {
      // For non-optional field, ensure is no-op
    }

    template <class Id, class Field>
    void ensure(const Field& def) {
      if (isAbsent(op::get<Id>(v))) {
        op::ensure<Id>(v) = def;
      }
    }
    template <class Id>
    void ensure(const std::unique_ptr<folly::IOBuf>& def) {
      if (!isAbsent(op::get<Id>(v))) {
        return;
      }
      if (def) {
        op::ensure<Id>(v) = std::make_unique<folly::IOBuf>(*def);
      } else {
        op::ensure<Id>(v) = std::make_unique<folly::IOBuf>();
      }
    }

    template <class Id>
    void remove() {
      using field_tag = op::get_field_tag<T, Id>;
      op::clear_field<field_tag>(op::get<Id>(v), v);
    }
  };

 protected:
  template <typename U, typename R = void>
  using if_union_patch = std::enable_if_t<is_thrift_union_v<U>, R>;
  template <typename U, typename R = void>
  using if_not_union_patch = std::enable_if_t<!is_thrift_union_v<U>, R>;

 private:
  template <typename Id>
  void removeImpl() {
    ensurePatchable();
    Base::toThrift().remove()->insert(op::get_field_id_v<T, Id>);
  }

  template <typename Id>
  std::enable_if_t<!type::is_optional_or_union_field_v<T, Id>> remove() {
    // Usually for non-optional field, it should not be removable.
    // This can only happen if the field was optional when creating the Patch,
    // but later we changed the field to non-optional.
    // In this case we should set it to intrinsic default when removing it.
    removeImpl<Id>();
  }

 public:
  using Base::Base;
  using Base::operator=;
  using Base::assign;
  /// Corresponding FieldPatch of this struct patch.
  using patch_type = get_native_type<Patch, ident::patch>;

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

  /// Returns if the patch is no-op.
  bool empty() const {
    bool b = true;
    op::for_each_ordinal<T>([&](auto id) {
      using Id = decltype(id);
      if constexpr (!apache::thrift::detail::is_shared_ptr_v<
                        op::get_field_ref<T, Id>>) {
        b = b && !this->modifies<Id>();
      }
    });
    return b;
  }

  /// Remove the given field. This is only valid for optional fields.
  template <typename Id>
  std::enable_if_t<
      type::is_optional_or_union_field_v<T, Id> && !is_thrift_union_v<T>>
  remove() {
    removeImpl<Id>();
  }

  /// Returns if the patch modifies the given field.
  template <typename Id>
  bool modifies() const {
    return hasAssign() || data_.clear() == true ||
        (getEnsure<Id>(data_) && type::is_optional_or_union_field_v<T, Id>) ||
        !getRawPatch<Id>(data_.patchPrior()).empty() ||
        !getRawPatch<Id>(data_.patch()).empty() ||
        (!data_.remove()->empty() &&
         data_.remove()->contains(op::get_field_id_v<T, Id>));
  }

  template <typename Id, typename... Ids>
  std::enable_if_t<sizeof...(Ids) != 0, bool> modifies() const {
    // If hasAssign() == true, the whole struct (all fields) will be replaced.
    if (hasAssign() || data_.clear() == true ||
        (getEnsure<Id>(data_) && type::is_optional_or_union_field_v<T, Id>) ||
        (!data_.remove()->empty() &&
         data_.remove()->contains(op::get_field_id_v<T, Id>))) {
      return true;
    }

    return getRawPatch<Id>(data_.patchPrior()).template modifies<Ids...>() ||
        getRawPatch<Id>(data_.patch()).template modifies<Ids...>();
  }

  /// Ensures the given field is set.
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
  std::enable_if_t<type::is_optional_or_union_field_v<T, Id>> ensure(
      U&& defaultVal) {
    if (maybeEnsure<Id>()) {
      if (patchPrior<Id>().toThrift().clear().value() &&
          !is_thrift_union_v<T>) {
        // If the field is cleared, we need to assign the value in PatchAfter.
        // Why? In dynamic patch, the PatchOp::Clear in PatchPrior will set the
        // field to intrinsic default rather than removing it (unlike static
        // patch). If we assign the value in Patch after, then the behavior
        // would be the same between static/dynamic patch.
        patchPrior<Id>().reset();
        FieldPatchAssigner<get_type_tag<T, Id>>{}(
            patchAfter<Id>(), std::forward<U>(defaultVal));
      } else {
        getEnsure<Id>(data_) = std::forward<U>(defaultVal);
      }
    }
  }
  template <typename Id>
  std::enable_if_t<type::is_optional_or_union_field_v<T, Id>> ensure(
      const std::unique_ptr<folly::IOBuf>& defaultVal) {
    if (defaultVal) {
      ensure<Id>(std::make_unique<folly::IOBuf>(*defaultVal));
    } else {
      ensure<Id>(std::make_unique<folly::IOBuf>());
    }
  }

  /// Ensures the given field is initalized, and return the associated patch
  /// object.
  template <class Id>
  decltype(auto) patch() {
    static_assert(!std::is_same_v<Id, op::get_ordinal<T, Id>>);
    return *op::get<Id>(patchImpl<op::get_field_id<T, Id>>().toThrift());
  }

  /// Returns the proper patch object for the given field.
  template <typename Id>
  decltype(auto) patchIfSet() {
    if (!type::is_optional_or_union_field_v<T, Id>) {
      return patch<Id>();
    }
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

  /// @brief This API uses the Visitor pattern to describe how Patch is applied.
  /// For each operation that will be performed by the patch, the corresponding
  /// method (that matches the write API) will be invoked.
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(const MyClass&);
  ///       void clear();
  ///       template<class Id> void patchIfSet(const FieldPatch&);
  ///       // For optional fields in structs and fields in unions
  ///       template<class Id> void ensure(const op::get_native_type<Id>&);
  ///       // For non-optional fields in structs
  ///       template<class Id> void ensure();
  ///       // For optional fields in structs and fields in unions
  ///       template<class Id> void remove();
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
        if constexpr (!apache::thrift::detail::is_shared_ptr_v<
                          op::get_field_ref<T, Id>>) {
          using FieldPatchType =
              folly::remove_cvref_t<decltype(BaseEnsurePatch{}.patch<Id>())>;
          v.template patchIfSet<Id>(FieldPatchType{});
          v.template ensure<Id>();
          if constexpr (type::is_optional_or_union_field_v<T, Id>) {
            v.template ensure<Id>(FieldType<Id>{});
          }
        }
      });
    }

    if (Base::customVisitAssignAndClear(std::forward<Visitor>(v))) {
      return;
    }

    data_.patchPrior()->customVisit(std::forward<Visitor>(v));

    if constexpr (!is_thrift_union_v<T>) {
      for_each_field_id<T>([&](auto id) {
        using Id = decltype(id);
        if constexpr (!apache::thrift::detail::is_shared_ptr_v<
                          op::get_field_ref<T, Id>>) {
          if (auto p = op::get<Id>(*data_.ensure())) {
            if constexpr (type::is_optional_or_union_field_v<T, Id>) {
              std::forward<Visitor>(v).template ensure<Id>(*p);
            } else {
              std::forward<Visitor>(v).template ensure<Id>();
            }
          }
        }
      });
    } else {
      // For Thrift Union, we only need to visit the active field.
      op::invoke_by_field_id<T>(
          static_cast<FieldId>(data_.ensure().value().getType()),
          [&](auto id) {
            using Id = decltype(id);
            std::forward<Visitor>(v).template ensure<Id>(
                *op::get<Id>(*data_.ensure()));
          },
          []() {
            // Ignore if the union is empty.
          });
    }

    data_.patch()->customVisit(std::forward<Visitor>(v));

    if constexpr (!is_thrift_union_v<T>) {
      for (auto fieldId : *data_.remove()) {
        op::invoke_by_field_id<T>(
            fieldId,
            [&](auto id) {
              using Id = decltype(id);
              if constexpr (!apache::thrift::detail::is_shared_ptr_v<
                                op::get_field_ref<T, Id>>) {
                std::forward<Visitor>(v).template remove<Id>();
              }
            },
            [] {
              // Ignore if the specified field is not part of the struct
              // considering schema evolution.
            });
      }
    }
  }

  void apply(T& val) const;

  /**
   * Returns a Thrift Patch instance corresponding to the (decoded) `SafePatch`.
   *
   * @throws std::runtime_error if the given `SafePatch` cannot be successfully
   * decoded or safely applied in this process (eg. if the version of the Thrift
   * Patch library in this process is not compatible with the minimum version
   * required by `SafePatch`).
   */
  static auto fromSafePatch(
      const typename SafePatchType<type::infer_tag<T>>::type& safePatch) {
    return fromSafePatchImpl<Derived>(safePatch);
  }

  /**
   * Returns a `SafePatch` instance corresponding to the encoded Thrift Patch.
   */
  auto toSafePatch() const {
    return toSafePatchImpl<typename SafePatchType<type::infer_tag<T>>::type>(
        Base::derived());
  }

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

  // If Assign operation exists, we need to replace it with Ensure + Patch
  // operation so that we can have sub-patches.
  template <typename U = T>
  if_union_patch<U> ensurePatchable() {
    if (data_.assign().has_value()) {
      data_.clear() = true;
      *data_.ensure() = std::move(*data_.assign());
      // Unset assign.
      data_.assign().reset();
    }
  }

  // For Thrift Struct, we always ensure the patch is patchable.
  template <typename U = T>
  if_not_union_patch<U> ensurePatchable() {
    if (data_.assign().has_value()) {
      for_each_field_id<T>([&](auto id) {
        using FId = decltype(id);
        if constexpr (!apache::thrift::detail::is_shared_ptr_v<
                          op::get_field_ref<T, FId>>) {
          auto&& field = op::get<>(id, *data_.assign());
          auto&& prior = getRawPatch<FId>(data_.patchPrior());
          auto&& ensure = op::get<>(id, *data_.ensure());
          auto&& after = getRawPatch<FId>(data_.patch());
          if (isAbsent(field)) {
            prior.toThrift().clear() = true;
          } else {
            ensure = {};
            FieldPatchAssigner<op::get_type_tag<T, FId>>{}(
                after, std::move(*field));
          }
        }
      });
      // Unset assign.
      data_.assign().reset();
    }
  }

  template <typename Id>
  bool maybeEnsure() {
    // If Thrift Union, we move assign operation to clear and ensure operations
    // so that we can ensure.
    if constexpr (is_thrift_union_v<T>) {
      if (data_.assign().has_value()) {
        data_.clear() = true;
        data_.ensure() = std::move(*data_.assign());
        data_.assign().reset();
      }
    }

    if constexpr (!is_thrift_union_v<T>) {
      if (*patchAfter<Id>().toThrift().clear()) {
        // Since we cleared the field in PatchAfter, we should remove any
        // existing ensured value.
        op::clear<Id>(*data_.ensure());
      }

      auto removeRef = Base::toThrift().remove();
      auto iter = removeRef->find(op::get_field_id_v<T, Id>);
      if (iter != removeRef->end()) {
        // If the current patch removed the field and now we want to ensure it,
        // we should clear it in patch prior and then ensure it.
        removeRef->erase(iter);
        patchPrior<Id>().reset();
        patchAfter<Id>().reset();
        patchPrior<Id>().toThrift().clear() = true;
        getEnsure<Id>(data_).emplace();
        return true;
      }
    }

    if (ensures<Id>()) {
      return false;
    }

    // If the union patch already ensured to a different field, we need to set
    // clear so that `ensure` becomes logically `assign`.
    // Why is this necessary? Without this change, consider the following patch
    //
    //   UnionPatch{ensure = {field1: 10}}
    //
    // Now let's do `unionPatch.ensure<field2>(20)`, if we just change to
    //
    //   UnionPatch{ensure = {field2: 20}}
    //
    // This is incorrect when patching union like `{field2: 10}` since the
    // result should be `{field2: 20}` instead of `{field2: 10}`.

    if (is_thrift_union_v<T> && !apache::thrift::empty(*data_.ensure())) {
      clear();
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
  template <class FieldId>
  patch_type& patchImpl();

  template <typename Id, typename U>
  static decltype(auto) getRawPatch(U&& patch) {
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
/// * `terse list<i16> remove`
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
  using patch_type = get_native_type<Patch, ident::patch>;

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
    if (data_.assign().has_value()) {
      op::get<Id>(*data_.assign()) = std::forward<U>(val);
    } else {
      Base::template patch<Id>().assign(std::forward<U>(val));
    }
  }

  /// @cond
  template <class Protocol>
  uint32_t encode(Protocol& prot) const {
    // PatchOp::Remove
    using PatchOpRemoveId = field_id<7>;
    uint32_t s = 0;
    s += prot.writeStructBegin(op::get_class_name_v<Patch>.data());
    const auto remove = removedFields();
    op::for_each_field_id<Patch>([&](auto id) {
      using Id = decltype(id);
      using Tag = op::get_type_tag<Patch, Id>;
      constexpr bool isRemoveField = std::is_same<Id, PatchOpRemoveId>::value;

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
      if constexpr (isRemoveField) {
        s += op::encode<Tag>(prot, remove);
      } else {
        s += op::encode<Tag>(prot, *field);
      }
      s += prot.writeFieldEnd();
    });
    s += prot.writeFieldStop();
    s += prot.writeStructEnd();
    return s;
  }
  /// @endcond

  void merge(const StructPatch& patch);
  void merge(StructPatch&& patch);

 private:
  using Base::data_;

  // Whether the field is removed
  template <class Id>
  bool isRemoved() const {
    const auto& prior = data_.patchPrior()->toThrift();
    const auto& ensure = *data_.ensure();
    const auto& after = data_.patch()->toThrift();
    const auto& remove = *data_.remove();

    if constexpr (!type::is_optional_or_union_field_v<T, Id>) {
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

    return remove.count(op::get_field_id_v<T, Id>);
  }

  // Combine fields from PatchOp::Clear and PatchOp::Remove operations
  std::unordered_set<FieldId> removedFields() const {
    auto removed = *data_.remove();
    op::for_each_field_id<T>([&](auto id) {
      using Id = decltype(id);
      if constexpr (!apache::thrift::detail::is_shared_ptr_v<
                        op::get_field_ref<T, Id>>) {
        if (this->isRemoved<Id>()) {
          removed.insert(id.value);
        }
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

  void merge(const UnionPatch& patch);
  void merge(UnionPatch&& patch);
};

} // namespace apache::thrift::op::detail
