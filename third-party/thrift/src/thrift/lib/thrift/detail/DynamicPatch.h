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

namespace detail {
using ValueList = std::vector<Value>;
using ValueSet = folly::F14FastSet<Value>;
using ValueMap = folly::F14FastMap<Value, Value>;

// Use Badge pattern to control access.
// See thrift/lib/cpp2/patch/detail/PatchBadge.h
struct PatchBadgeFactory;
using Badge = folly::badge<PatchBadgeFactory>;

template <class PatchType>
PatchType createPatchFromObject(Badge badge, Object obj) {
  PatchType patch;
  if constexpr (requires { patch.fromObject(badge, std::move(obj)); }) {
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

class DynamicUnknownPatch : public DynamicPatchBase {
 public:
  void assign(detail::Badge, Object v);
  void remove(detail::Badge badge, Value v);
  void patchIfSet(detail::Badge, FieldId, const DynamicPatch&);

  void fromObject(detail::Badge badge, Object obj) {
    DynamicPatchBase::fromObject(badge, std::move(obj));
    validateAndGetCategory();
  }

  template <class Visitor>
  void customVisit(detail::Badge, Visitor&&) const;
  void apply(detail::Badge, Value&) const;
  void merge(detail::Badge badge, const DynamicUnknownPatch& other) {
    other.customVisit(badge, *this);
  }

 private:
  // The unknown patch can be classified into the following categories
  enum class Category {
    // The patch has no operation.
    EmptyPatch,

    // The patch only has clear operation.
    ClearPatch,

    // The patch has assign or patchPrior/patchAfter, but we don't know whether
    // it is a struct patch or an union patch
    StructuredPatch,

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

  template <class Visitor>
  void customVisit(detail::Badge badge, Visitor&& v) const {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      v.assign(badge, assign->as_list());
      return;
    }

    if (auto clear = get_ptr(op::PatchOp::Clear); clear && clear->as_bool()) {
      v.clear(badge);
    }

    if (auto put = get_ptr(op::PatchOp::Put)) {
      for (auto& i : put->as_list()) {
        v.push_back(badge, i);
      }
    }
  }

  void apply(detail::Badge, detail::ValueList& v) const;
  void merge(detail::Badge badge, const DynamicListPatch& other) {
    other.customVisit(badge, *this);
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
      assign->ensure_set().erase(std::move(v));
      return;
    }
    if (auto add = get_ptr(op::PatchOp::Add)) {
      add->ensure_set().erase(v);
    }
    get(op::PatchOp::Remove).ensure_set().insert(std::move(v));
  }

  void add(detail::Badge badge, const detail::ValueSet& add) {
    for (auto&& i : add) {
      insert(badge, i);
    }
  }

  void remove(detail::Badge badge, const detail::ValueSet& remove) {
    for (auto&& i : remove) {
      erase(badge, i);
    }
  }

  template <class Visitor>
  void customVisit(detail::Badge badge, Visitor&& v) const {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      v.assign(badge, assign->as_set());
      return;
    }

    if (auto clear = get_ptr(op::PatchOp::Clear); clear && clear->as_bool()) {
      v.clear(badge);
    }

    if (auto remove = get_ptr(op::PatchOp::Remove)) {
      v.remove(badge, remove->as_set());
    }

    if (auto add = get_ptr(op::PatchOp::Add)) {
      v.add(badge, add->as_set());
    }
  }

  void apply(detail::Badge, detail::ValueSet& v) const;
  void merge(detail::Badge badge, const DynamicSetPatch& other) {
    other.customVisit(badge, *this);
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

  void add(detail::Badge, detail::ValueMap v);
  void remove(detail::Badge badge, const detail::ValueSet& v) {
    for (auto&& k : v) {
      erase(badge, k);
    }
  }
  void put(detail::Badge badge, const detail::ValueMap& m) {
    for (auto&& [k, v] : m) {
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

  template <class Visitor>
  void customVisit(detail::Badge badge, Visitor&& v) const;
  void apply(detail::Badge, detail::ValueMap& v) const;
  void merge(detail::Badge badge, const DynamicMapPatch& other) {
    other.customVisit(badge, *this);
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

  DynamicPatch& patchIfSet(detail::Badge, FieldId id, const DynamicPatch& p) {
    return patchIfSetImpl(id, p);
  }
  DynamicPatch& patchIfSet(detail::Badge, FieldId id, DynamicPatch&& p) {
    return patchIfSetImpl(id, std::move(p));
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

  template <class Visitor>
  void customVisit(detail::Badge, Visitor&& v) const;
  void apply(detail::Badge, Object& obj) const;

 private:
  void undoChanges(FieldId);
  void ensurePatchable();

  void ensureStruct(FieldId, Value);
  void ensureUnion(FieldId, Value);

  template <class SubPatch>
  DynamicPatch& patchIfSetImpl(FieldId, SubPatch&&);

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
  void merge(detail::Badge badge, const DynamicStructPatch& p) {
    p.customVisit(badge, *this);
  }
};
class DynamicUnionPatch : public DynamicStructurePatch<true> {
 public:
  void merge(detail::Badge badge, const DynamicUnionPatch& p) {
    p.customVisit(badge, *this);
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

  [[nodiscard]] bool empty(detail::Badge) const;

  void apply(detail::Badge, Value&) const;

  void fromObject(detail::Badge, Object);

  template <class T>
  bool holds_alternative(detail::Badge) const {
    return std::get_if<T>(patch_.get()) != nullptr;
  }

  template <class PatchType>
  PatchType* get_if(detail::Badge) {
    return std::get_if<PatchType>(patch_.get());
  }

  void merge(detail::Badge, const DynamicPatch& other);

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

template <class Visitor>
void DynamicUnknownPatch::customVisit(detail::Badge badge, Visitor&& v) const {
  // We don't need to check EnsureStruct and EnsureUnion fields since they won't
  // exists -- otherwise we are able to identify the exact patch type and it
  // should not be unknown patch.
  if (auto assign = get_ptr(op::PatchOp::Assign)) {
    v.assign(badge, assign->as_object());
    return;
  }

  if (auto clear = get_ptr(op::PatchOp::Clear); clear && clear->as_bool()) {
    v.clear(badge);
  }

  for (auto op : {op::PatchOp::PatchPrior, op::PatchOp::PatchAfter}) {
    if (auto subPatch = get_ptr(op)) {
      for (const auto& [fieldId, fieldPatch] : subPatch->as_object()) {
        DynamicPatch patch;
        patch.fromObject(badge, fieldPatch.as_object());
        v.patchIfSet(badge, static_cast<FieldId>(fieldId), patch);
      }
    }
  }

  if (auto remove = get_ptr(op::PatchOp::Remove)) {
    for (const auto& i : remove->as_set()) {
      v.remove(badge, i);
    }
  }
}

template <class Visitor>
void DynamicMapPatch::customVisit(detail::Badge badge, Visitor&& v) const {
  if (assign_) {
    v.assign(badge, *assign_);
    return;
  }

  if (clear_) {
    v.clear(badge);
  }

  for (auto&& [k, p] : patchPrior_) {
    v.patchByKey(badge, k, p);
  }

  v.add(badge, add_);
  v.remove(badge, remove_);
  v.put(badge, put_);

  for (auto&& [k, p] : patchAfter_) {
    v.patchByKey(badge, k, p);
  }
}

template <bool IsUnion>
template <class Visitor>
void DynamicStructurePatch<IsUnion>::customVisit(
    detail::Badge badge, Visitor&& v) const {
  if (assign_) {
    v.assign(badge, *assign_);
    return;
  }

  if (clear_) {
    v.clear(badge);
  }

  for (const auto& [id, p] : patchPrior_) {
    v.patchIfSet(badge, id, p);
  }

  for (const auto& [id, value] : ensure_) {
    v.ensure(badge, id, value);
  }

  for (const auto& [id, p] : patchAfter_) {
    v.patchIfSet(badge, id, p);
  }

  if constexpr (!IsUnion) {
    for (const auto& id : remove_) {
      v.remove(badge, id);
    }
  }
}

class DiffVisitorBase {
 public:
  [[nodiscard]] DynamicPatch diff(const Object& src, const Object& dst);
  [[nodiscard]] DynamicPatch diff(
      detail::Badge, const Value& src, const Value& dst);

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
  [[nodiscard]] op::AnyPatch diffAny(const Object& src, const Object& dst);
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

} // namespace apache::thrift::protocol
