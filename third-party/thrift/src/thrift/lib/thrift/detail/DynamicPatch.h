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

#include <folly/lang/Badge.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Object.h>

namespace apache::thrift::op::detail {
template <typename>
class AnyPatch;
}

namespace apache::thrift::op {
class AnyPatchStruct;
using AnyPatch = detail::AnyPatch<AnyPatchStruct>;
} // namespace apache::thrift::op

namespace apache::thrift::protocol {
class DiffVisitorBase;

class DynamicListPatch;
class DynamicSetPatch;
class DynamicMapPatch;
class DynamicStructPatch;
class DynamicUnionPatch;

namespace detail {
using ValueList = std::vector<Value>;
using ValueSet = folly::F14FastSet<Value>;
using ValueMap = folly::F14FastMap<Value, Value>;

// Use Badge pattern to control access.
// See thrift/lib/cpp2/patch/detail/PatchBadge.h
struct PatchBadgeFactory;
using Badge = folly::badge<PatchBadgeFactory>;

template <typename T, typename U>
using if_same_type_after_remove_cvref = std::enable_if_t<
    std::is_same_v<folly::remove_cvref_t<T>, folly::remove_cvref_t<U>>>;

template <class PatchType>
PatchType createPatchFromObject(Badge badge, Object obj) {
  PatchType patch;
  if constexpr (__FBTHRIFT_IS_VALID(
                    patch, patch.fromObject(badge, std::move(obj)))) {
    patch.fromObject(badge, std::move(obj));
  } else {
    // TODO: schema validation
    patch = fromObjectStruct<type::infer_tag<PatchType>>(std::move(obj));
  }
  return patch;
}

void convertStringToBinary(Value& v);
void convertStringToBinary(ValueList& v);
void convertStringToBinary(ValueSet& v);
void convertStringToBinary(ValueMap& v);
void convertStringToBinary(Object& obj);

struct DynamicPatchOptions {
  // If users are confident their input doesn't contain string, we can skip
  // converting it to binary.
  bool doNotConvertStringToBinary = false;
};

// Note: this function is half-baked.
// Currently we only implemented struct/union for internal use cases.
// Ideally we should also support primitive types (e.g., i32 --> I32Patch).
// External users should use toPatchUri/fromPatchUri instead for now.
type::Type toPatchType(type::Type input);
} // namespace detail

class DynamicPatch;

class DynamicPatchBase {
 public:
  DynamicPatchBase() = default;
  DynamicPatchBase(const DynamicPatchBase&) = default;
  DynamicPatchBase(DynamicPatchBase&&) = default;
  DynamicPatchBase& operator=(const DynamicPatchBase&) = default;
  DynamicPatchBase& operator=(DynamicPatchBase&&) = default;

  void clear(detail::Badge) {
    patch_.members()->clear();
    get(op::PatchOp::Clear).emplace_bool(true);
  }

  void fromObject(detail::Badge, Object obj) { patch_ = std::move(obj); }

  [[nodiscard]] Object toObject() const& {
    return DynamicPatchBase(*this).toObject();
  }

  [[nodiscard]] Object toObject() && {
    if (!options_.doNotConvertStringToBinary) {
      detail::convertStringToBinary(patch_);
    }
    return std::move(patch_);
  }

  void doNotConvertStringToBinary(detail::Badge) {
    options_.doNotConvertStringToBinary = true;
  }

  // The number of added/removed/replaced/patched elements in Patch, it will be
  // used to decide whether to use assign patch.
  [[nodiscard]] std::size_t patchedElementCount(detail::Badge) const;

  [[nodiscard]] bool empty(detail::Badge) const { return patch_.empty(); }

 protected:
  ~DynamicPatchBase() = default;

  [[nodiscard]] Value& get(op::PatchOp patchOp) {
    return patch_[static_cast<FieldId>(patchOp)];
  }

  [[nodiscard]] Value* get_ptr(op::PatchOp patchOp) {
    return folly::get_ptr(*patch_.members(), folly::to_underlying(patchOp));
  }

  [[nodiscard]] const Value* get_ptr(op::PatchOp patchOp) const {
    return folly::get_ptr(*patch_.members(), folly::to_underlying(patchOp));
  }

  Object patch_;
  detail::DynamicPatchOptions options_;
};

// For Patches that there are multiple possible types.
// See DynamicUnknownPatch::Category for all possible scenarios.
class DynamicUnknownPatch : public DynamicPatchBase {
 public:
  void assign(detail::Badge, Object v);
  void remove(detail::Badge badge, Value v);
  void patchIfSet(detail::Badge, FieldId, const DynamicPatch&);

  void fromObject(detail::Badge badge, Object obj) {
    DynamicPatchBase::fromObject(badge, std::move(obj));
    validateAndGetCategory();
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&&, detail::Badge, Visitor&&);

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

  void apply(detail::Badge, Value&) const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicUnknownPatch> merge(
      detail::Badge badge, Next&& other) {
    std::forward<Next>(other).customVisit(badge, *this);
  }

 private:
  // The unknown patch can be classified into the following categories
  enum class Category {
    // The patch has no operation. It can be any kinds of patch.
    EmptyPatch,

    // The patch only has clear operation. It can be any kinds of patch.
    ClearPatch,

    // The patch has assign or patchPrior/patchAfter, but we don't know whether
    // it is a struct patch or an union patch.
    // If the patch only has assign, it can be an any patch, which is also under
    // this category.
    StructuredOrAnyPatch,

    // Patch only has the `remove` operation, we don't know whether it is a set
    // or a map patch.
    AssociativeContainerPatch
  };

  Category validateAndGetCategory() const;

  [[noreturn]] void throwIncompatibleCategory(std::string_view method) const;

  bool isOneOfCategory(std::initializer_list<Category> c) const {
    return std::find(c.begin(), c.end(), validateAndGetCategory()) != c.end();
  }
};
class DynamicListPatch : public DynamicPatchBase {
 public:
  using DynamicPatchBase::DynamicPatchBase;
  void assign(detail::Badge, detail::ValueList v) {
    patch_.members()->clear();
    get(op::PatchOp::Assign).emplace_list(std::move(v));
  }

  void push_back(detail::Badge, Value v) {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      assign->as_list().push_back(std::move(v));
      return;
    }
    get(op::PatchOp::Put).ensure_list().push_back(std::move(v));
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, detail::Badge badge, Visitor&& v) {
    if (auto assign = self.get_ptr(op::PatchOp::Assign)) {
      std::forward<Visitor>(v).assign(
          badge, folly::forward_like<Self>(assign->as_list()));
      return;
    }

    if (auto clear = self.get_ptr(op::PatchOp::Clear);
        clear && clear->as_bool()) {
      std::forward<Visitor>(v).clear(badge);
    }

    if (auto put = self.get_ptr(op::PatchOp::Put)) {
      for (auto& i : put->as_list()) {
        std::forward<Visitor>(v).push_back(badge, folly::forward_like<Self>(i));
      }
    }
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

  void apply(detail::Badge, detail::ValueList& v) const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicListPatch> merge(
      detail::Badge badge, Next&& other) {
    std::forward<Next>(other).customVisit(badge, *this);
  }
};

class DynamicSetPatch : public DynamicPatchBase {
 public:
  using DynamicPatchBase::DynamicPatchBase;
  void assign(detail::Badge, detail::ValueSet v) {
    patch_.members()->clear();
    get(op::PatchOp::Assign).emplace_set(std::move(v));
  }

  void insert(detail::Badge, Value v) {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      assign->ensure_set().insert(std::move(v));
      return;
    }
    if (auto remove = get_ptr(op::PatchOp::Remove)) {
      remove->ensure_set().erase(v);
    }
    get(op::PatchOp::Add).ensure_set().insert(std::move(v));
  }

  void erase(detail::Badge, Value v) {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      assign->ensure_set().erase(v);
      return;
    }
    if (auto add = get_ptr(op::PatchOp::Add)) {
      add->ensure_set().erase(v);
    }
    get(op::PatchOp::Remove).ensure_set().insert(std::move(v));
  }

  void addMulti(detail::Badge badge, const detail::ValueSet& add) {
    for (const auto& i : add) {
      insert(badge, i);
    }
  }

  void removeMulti(detail::Badge badge, const detail::ValueSet& remove) {
    for (const auto& i : remove) {
      erase(badge, i);
    }
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, detail::Badge badge, Visitor&& v) {
    if (auto assign = self.get_ptr(op::PatchOp::Assign)) {
      std::forward<Visitor>(v).assign(
          badge, folly::forward_like<Self>(assign->as_set()));
      return;
    }

    if (auto clear = self.get_ptr(op::PatchOp::Clear);
        clear && clear->as_bool()) {
      std::forward<Visitor>(v).clear(badge);
    }

    if (auto remove = self.get_ptr(op::PatchOp::Remove)) {
      std::forward<Visitor>(v).removeMulti(
          badge, folly::forward_like<Self>(remove->as_set()));
    }

    if (auto add = self.get_ptr(op::PatchOp::Add)) {
      std::forward<Visitor>(v).addMulti(
          badge, folly::forward_like<Self>(add->as_set()));
    }
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

  void apply(detail::Badge, detail::ValueSet& v) const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicSetPatch> merge(
      detail::Badge badge, Next&& other) {
    std::forward<Next>(other).customVisit(badge, *this);
  }
};

class DynamicMapPatch {
 public:
  void assign(detail::Badge, detail::ValueMap v) {
    *this = {};
    assign_ = std::move(v);
  }

  void clear(detail::Badge) {
    *this = {};
    clear_ = true;
  }

  void insert_or_assign(detail::Badge, Value k, Value v);
  void erase(detail::Badge, Value k);

  void tryPutMulti(detail::Badge, detail::ValueMap v);
  void removeMulti(detail::Badge badge, const detail::ValueSet& v) {
    for (const auto& k : v) {
      erase(badge, k);
    }
  }
  void putMulti(detail::Badge badge, const detail::ValueMap& m) {
    for (const auto& [k, v] : m) {
      insert_or_assign(badge, k, v);
    }
  }

  // Return the subPatch. We can use it to provide similar APIs to static patch.
  DynamicPatch& patchByKey(detail::Badge, Value k, const DynamicPatch& p) {
    return patchByKeyImpl(std::move(k), p);
  }
  DynamicPatch& patchByKey(detail::Badge, Value k, DynamicPatch&& p) {
    return patchByKeyImpl(std::move(k), std::move(p));
  }

  DynamicPatch& patchByKey(detail::Badge, Value&&);
  DynamicPatch& patchByKey(detail::Badge, const Value&);

  [[nodiscard]] Object toObject() &&;
  [[nodiscard]] Object toObject() const&;
  void fromObject(detail::Badge, Object obj);

  [[nodiscard]] std::size_t patchedElementCount(detail::Badge) const {
    return add_.size() + remove_.size() + put_.size() + patchPrior_.size() +
        patchAfter_.size();
  }

  [[nodiscard]] bool empty(detail::Badge) const {
    return !assign_ && !clear_ && add_.empty() && remove_.empty() &&
        put_.empty() && patchPrior_.empty() && patchAfter_.empty();
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, detail::Badge badge, Visitor&& v);

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);
  void apply(detail::Badge, detail::ValueMap& v) const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicMapPatch> merge(
      detail::Badge badge, Next&& other) {
    std::forward<Next>(other).customVisit(badge, *this);
  }

  void doNotConvertStringToBinary(detail::Badge) {
    options_.doNotConvertStringToBinary = true;
  }

 private:
  void undoChanges(const Value& k);
  void ensurePatchable();

  template <class SubPatch>
  DynamicPatch& patchByKeyImpl(Value k, SubPatch&& p);

  std::optional<detail::ValueMap> assign_;
  bool clear_ = false;
  folly::F14VectorMap<Value, DynamicPatch> patchPrior_;
  detail::ValueMap add_;
  folly::F14VectorMap<Value, DynamicPatch> patchAfter_;
  detail::ValueSet remove_;
  detail::ValueMap put_;
  detail::DynamicPatchOptions options_;
};

template <bool IsUnion>
class DynamicStructurePatch {
 public:
  void assign(detail::Badge, Object v) {
    *this = {};
    assign_ = std::move(v);
  }

  void clear(detail::Badge) {
    *this = {};
    clear_ = true;
  }

  void remove(detail::Badge, FieldId id) {
    undoChanges(id);
    remove_.insert(id);
  }

  void ensure(detail::Badge, FieldId id, Value v) {
    return IsUnion ? ensureUnion(id, std::move(v))
                   : ensureStruct(id, std::move(v));
  }

  // patchIfSet
  template <class Tag>
  auto& patchIfSet(detail::Badge badge, FieldId id) {
    auto& subPatch = patchIfSet(badge, id);
    return subPatch.template getStoredPatchByTag<Tag>(badge);
  }

  DynamicPatch& patchIfSet(detail::Badge, FieldId id) {
    ensurePatchable();
    return ensure_.contains(id) ? patchAfter_[id] : patchPrior_[id];
  }

  [[nodiscard]] bool empty(detail::Badge) const {
    return !assign_ && !clear_ && remove_.empty() && ensure_.empty() &&
        patchPrior_.empty() && patchAfter_.empty();
  }

  [[nodiscard]] Object toObject() &&;
  [[nodiscard]] Object toObject() const& {
    return DynamicStructurePatch(*this).toObject();
  }
  void fromObject(detail::Badge, Object obj);
  void doNotConvertStringToBinary(detail::Badge) {
    options_.doNotConvertStringToBinary = true;
  }

  bool modifies(detail::Badge, FieldId id) const {
    return assign_ || clear_ || patchPrior_.contains(id) ||
        ensure_.contains(id) || patchAfter_.contains(id) ||
        remove_.contains(id);
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, detail::Badge, Visitor&& v);

  // Needed for merge(...). We can consider making this a public API.
  template <class SubPatch>
  void patchIfSet(detail::Badge badge, FieldId id, SubPatch&& patch) {
    patchIfSet(badge, id).merge(std::forward<SubPatch>(patch));
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);
  void apply(detail::Badge, Object& obj) const;

 private:
  void undoChanges(FieldId);
  void ensurePatchable();

  void ensureStruct(FieldId, Value);
  void ensureUnion(FieldId, Value);

 private:
  std::optional<Object> assign_;
  bool clear_ = false;
  folly::F14VectorMap<FieldId, DynamicPatch> patchPrior_;
  folly::F14FastMap<FieldId, Value> ensure_;
  folly::F14VectorMap<FieldId, DynamicPatch> patchAfter_;
  folly::F14FastSet<FieldId> remove_;
  detail::DynamicPatchOptions options_;
};

class DynamicStructPatch : public DynamicStructurePatch<false> {
 public:
  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicStructPatch> merge(
      detail::Badge badge, Next&& other) {
    std::forward<Next>(other).customVisit(badge, *this);
  }
};
class DynamicUnionPatch : public DynamicStructurePatch<true> {
 public:
  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicUnionPatch> merge(
      detail::Badge badge, Next&& other) {
    std::forward<Next>(other).customVisit(badge, *this);
  }

 private:
  // Hide `remove` operation since we can't remove field from union
  using DynamicStructurePatch::remove;
};

class DynamicPatch {
 public:
  DynamicPatch();
  DynamicPatch(const DynamicPatch&);
  DynamicPatch& operator=(const DynamicPatch&);

  DynamicPatch(DynamicPatch&&) noexcept;
  DynamicPatch& operator=(DynamicPatch&&) noexcept;
  ~DynamicPatch();

  template <class T>
  explicit DynamicPatch(T t) : patch_(std::make_unique<Patch>(std::move(t))) {}

  Object toObject() &&;
  Object toObject() const&;
  type::AnyStruct toAny(detail::Badge, type::Type type) const;

  [[nodiscard]] bool empty(detail::Badge) const;
  [[nodiscard]] bool empty() const;

  /// Applies the patch to the given value. Throws if the patch is not
  /// applicable to the value.
  void apply(Value&) const;

  void fromObject(detail::Badge, Object);
  void fromAny(detail::Badge, const type::AnyStruct& any);

  template <typename Protocol>
  std::uint32_t encode(Protocol& prot) const;
  template <typename Protocol>
  void decode(Protocol& prot);

  template <class T>
  bool holds_alternative(detail::Badge) const {
    return std::get_if<T>(patch_.get()) != nullptr;
  }

  template <class PatchType>
  PatchType* get_if(detail::Badge) {
    return std::get_if<PatchType>(patch_.get());
  }

  /// Merges another patch into this patch. After the merge
  /// (`patch.merge(next)`), `patch.apply(value)` is equivalent to
  /// `next.apply(patch.apply(value))`.
  template <class Other>
  detail::if_same_type_after_remove_cvref<Other, DynamicPatch> merge(Other&&);

  template <class Tag>
  auto& getStoredPatchByTag(detail::Badge badge) {
    return getStoredPatchByTag(Tag{}, badge);
  }

 private:
  template <class Tag>
  auto& getStoredPatchByTag(type::list<Tag>, detail::Badge badge) {
    return getStoredPatch<DynamicListPatch>(badge);
  }
  template <class Tag>
  auto& getStoredPatchByTag(type::set<Tag>, detail::Badge badge) {
    return getStoredPatch<DynamicSetPatch>(badge);
  }
  template <class K, class V>
  auto& getStoredPatchByTag(type::map<K, V>, detail::Badge badge) {
    return getStoredPatch<DynamicMapPatch>(badge);
  }
  template <class T>
  auto& getStoredPatchByTag(type::struct_t<T>, detail::Badge badge) {
    return getStoredPatch<DynamicStructPatch>(badge);
  }
  template <class T>
  auto& getStoredPatchByTag(type::union_t<T>, detail::Badge badge) {
    return getStoredPatch<DynamicUnionPatch>(badge);
  }
  template <class T, class Tag>
  auto& getStoredPatchByTag(type::cpp_type<T, Tag>, detail::Badge badge) {
    return getStoredPatchByTag(Tag{}, badge);
  }
  template <class T>
  auto& getStoredPatchByTag(type::enum_t<T>, detail::Badge badge) {
    return getStoredPatch<op::I32Patch>(badge);
  }
  template <class Tag>
  auto& getStoredPatchByTag(Tag, detail::Badge badge) {
    static_assert(type::is_a_v<Tag, type::primitive_c>);
    return getStoredPatch<op::patch_type<Tag>>(badge);
  }

  template <class Patch>
  Patch& getStoredPatch(detail::Badge badge) {
    // patch already has the correct type, return it directly.
    if (auto p = get_if<Patch>(badge)) {
      return *p;
    }

    // Use merge to change patch's type.
    merge(DynamicPatch{Patch{}});
    return *get_if<Patch>(badge);
  }

  template <class Self, class Visitor>
  static decltype(auto) visitPatchImpl(
      Self&& self, detail::Badge, Visitor&& visitor) {
    return std::visit(
        std::forward<Visitor>(visitor), *std::forward<Self>(self).patch_);
  }

  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, detail::Badge badge, Visitor&& v) {
    std::forward<Self>(self).visitPatch(badge, [&](auto&& patch) {
      using PatchType = folly::remove_cvref_t<decltype(patch)>;
      if constexpr (__FBTHRIFT_IS_VALID(
                        patch,
                        std::forward<PatchType>(patch).customVisit(
                            badge, std::forward<Visitor>(v)))) {
        std::forward<PatchType>(patch).customVisit(
            badge, std::forward<Visitor>(v));
      } else {
        std::forward<PatchType>(patch).customVisit(std::forward<Visitor>(v));
      }
    });
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      visitPatch, visitPatchImpl);

  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

 private:
  using Patch = std::variant<
      DynamicUnknownPatch,
      op::BoolPatch,
      op::BytePatch,
      op::I16Patch,
      op::I32Patch,
      op::I64Patch,
      op::FloatPatch,
      op::DoublePatch,
      op::StringPatch,
      op::BinaryPatch,
      op::AnyPatch,
      DynamicListPatch,
      DynamicSetPatch,
      DynamicMapPatch,
      DynamicStructPatch,
      DynamicUnionPatch>;
  std::unique_ptr<Patch> patch_;
};

template <class Self, class Visitor>
void DynamicUnknownPatch::customVisitImpl(
    Self&& self, detail::Badge badge, Visitor&& v) {
  // We don't need to check EnsureStruct and EnsureUnion fields since they won't
  // exists -- otherwise we are able to identify the exact patch type and it
  // should not be unknown patch.
  if (auto assign = self.get_ptr(op::PatchOp::Assign)) {
    std::forward<Visitor>(v).assign(
        badge, folly::forward_like<Self>(assign->as_object()));
    return;
  }

  if (auto clear = self.get_ptr(op::PatchOp::Clear);
      clear && clear->as_bool()) {
    std::forward<Visitor>(v).clear(badge);
  }

  for (auto op : {op::PatchOp::PatchPrior, op::PatchOp::PatchAfter}) {
    if (auto subPatch = self.get_ptr(op)) {
      for (auto& [fieldId, fieldPatch] : subPatch->as_object()) {
        DynamicPatch patch;
        patch.fromObject(
            badge, folly::forward_like<Self>(fieldPatch.as_object()));
        std::forward<Visitor>(v).patchIfSet(
            badge, static_cast<FieldId>(fieldId), std::move(patch));
      }
    }
  }

  if (auto remove = self.get_ptr(op::PatchOp::Remove)) {
    for (auto& i : remove->as_set()) {
      std::forward<Visitor>(v).remove(badge, i);
    }
  }
}

template <class Self, class Visitor>
void DynamicMapPatch::customVisitImpl(
    Self&& self, detail::Badge badge, Visitor&& v) {
  if (self.assign_) {
    std::forward<Visitor>(v).assign(badge, *std::forward<Self>(self).assign_);
    return;
  }

  if (self.clear_) {
    std::forward<Visitor>(v).clear(badge);
  }

  for (auto& [k, p] : self.patchPrior_) {
    std::forward<Visitor>(v).patchByKey(badge, k, folly::forward_like<Self>(p));
  }

  std::forward<Visitor>(v).tryPutMulti(badge, std::forward<Self>(self).add_);
  std::forward<Visitor>(v).removeMulti(badge, std::forward<Self>(self).remove_);
  std::forward<Visitor>(v).putMulti(badge, std::forward<Self>(self).put_);

  for (auto& [k, p] : self.patchAfter_) {
    std::forward<Visitor>(v).patchByKey(badge, k, folly::forward_like<Self>(p));
  }
}

template <bool IsUnion>
template <class Self, class Visitor>
void DynamicStructurePatch<IsUnion>::customVisitImpl(
    Self&& self, detail::Badge badge, Visitor&& v) {
  if (self.assign_) {
    std::forward<Visitor>(v).assign(badge, *std::forward<Self>(self).assign_);
    return;
  }

  if (self.clear_) {
    std::forward<Visitor>(v).clear(badge);
  }

  for (auto& [id, p] : self.patchPrior_) {
    std::forward<Visitor>(v).patchIfSet(
        badge, id, folly::forward_like<Self>(p));
  }

  for (auto& [id, value] : self.ensure_) {
    std::forward<Visitor>(v).ensure(
        badge, id, folly::forward_like<Self>(value));
  }

  for (auto& [id, p] : self.patchAfter_) {
    std::forward<Visitor>(v).patchIfSet(
        badge, id, folly::forward_like<Self>(p));
  }

  if constexpr (!IsUnion) {
    for (const auto& id : self.remove_) {
      std::forward<Visitor>(v).remove(badge, id);
    }
  }
}

class DiffVisitorBase {
 public:
  [[nodiscard]] DynamicPatch diff(const Object& src, const Object& dst);
  [[nodiscard]] DynamicPatch diff(
      detail::Badge, const Value& src, const Value& dst);
  [[nodiscard]] DynamicPatch diff(const Value& src, const Value& dst);

  virtual ~DiffVisitorBase() = default;

 protected:
  virtual op::BoolPatch diffBool(bool, bool dst);
  [[nodiscard]] virtual op::BytePatch diffByte(
      std::int8_t src, std::int8_t dst);
  [[nodiscard]] virtual op::I16Patch diffI16(
      std::int16_t src, std::int16_t dst);
  [[nodiscard]] virtual op::I32Patch diffI32(
      std::int32_t src, std::int32_t dst);
  [[nodiscard]] virtual op::I64Patch diffI64(
      std::int64_t src, std::int64_t dst);
  [[nodiscard]] virtual op::FloatPatch diffFloat(float src, float dst);
  [[nodiscard]] virtual op::DoublePatch diffDouble(double src, double dst);
  [[nodiscard]] virtual op::BinaryPatch diffBinary(
      const folly::IOBuf& src, const folly::IOBuf& dst);
  [[nodiscard]] virtual DynamicListPatch diffList(
      const detail::ValueList& src, const detail::ValueList& dst);
  [[nodiscard]] virtual DynamicSetPatch diffSet(
      const detail::ValueSet& src, const detail::ValueSet& dst);
  [[nodiscard]] virtual DynamicMapPatch diffMap(
      const detail::ValueMap& src, const detail::ValueMap& dst);
  [[nodiscard]] virtual DynamicPatch diffStructured(
      const Object& src, const Object& dst);

 protected:
  [[nodiscard]] DynamicUnionPatch diffUnion(
      const Object& src, const Object& dst);
  [[nodiscard]] virtual op::AnyPatch diffAny(
      const Object& src, const Object& dst);
  void diffField(
      const Object& src,
      const Object& dst,
      FieldId id,
      DynamicStructPatch& patch);
  void diffElement(
      const detail::ValueMap& src,
      const detail::ValueMap& dst,
      const Value& key,
      DynamicMapPatch& patch);

  static DynamicPatch createDynamicUnknownPatchWithAssign(const Object& obj);

  void pushField(FieldId);
  void pushType(type::Type);
  void pushKey(const Value&);
  void pop();

  const Mask& getCurrentPath() const { return path_; }

 private:
  Mask path_ = allMask();

  // Keep tracking the stack of mask in path_ for optimization.
  std::stack<Mask*> maskInPath_{{&path_}};
};

// Convert a normal struct uri to patch uri
std::string toPatchUri(std::string s);
std::string fromPatchUri(std::string s);

} // namespace apache::thrift::protocol
