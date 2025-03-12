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

#include <algorithm>
#include <folly/Overload.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/detail/DynamicPatch.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_types.h>

#include <thrift/lib/cpp2/patch/detail/PatchBadge.h>

namespace apache::thrift::protocol {

using detail::badge;
using detail::ValueList;
using detail::ValueMap;
using detail::ValueSet;

namespace {

// bools, integers or floating pointers.
template <class Tag, class T>
std::enable_if_t<std::is_arithmetic_v<T>, op::patch_type<Tag>>
createArithmeticPatch(const T& src, const T& dst) {
  if (src == dst) {
    return {};
  }

  op::patch_type<Tag> ret;
  if (op::isEmpty<Tag>(dst)) {
    ret.clear();
  } else {
    ret.assign(dst);
  }
  return ret;
}

[[noreturn]] void notImpl() {
  folly::throw_exception<std::runtime_error>("Not implemented");
}

Value* get_ptr(Object& obj, op::PatchOp op) {
  return folly::get_ptr(*obj.members(), folly::to_underlying(op));
}

DynamicPatch createPatchFromObjectAsValueType(const Value& value, Object obj) {
  switch (value.getType()) {
    case Value::Type::boolValue:
      return DynamicPatch{
          detail::createPatchFromObject<op::BoolPatch>(badge, std::move(obj))};
    case Value::Type::byteValue:
      return DynamicPatch{
          detail::createPatchFromObject<op::BytePatch>(badge, std::move(obj))};
    case Value::Type::i16Value:
      return DynamicPatch{
          detail::createPatchFromObject<op::I16Patch>(badge, std::move(obj))};
    case Value::Type::i32Value:
      return DynamicPatch{
          detail::createPatchFromObject<op::I32Patch>(badge, std::move(obj))};
    case Value::Type::i64Value:
      return DynamicPatch{
          detail::createPatchFromObject<op::I64Patch>(badge, std::move(obj))};
    case Value::Type::floatValue:
      return DynamicPatch{
          detail::createPatchFromObject<op::FloatPatch>(badge, std::move(obj))};
    case Value::Type::doubleValue:
      return DynamicPatch{detail::createPatchFromObject<op::DoublePatch>(
          badge, std::move(obj))};
    case Value::Type::stringValue:
      return DynamicPatch{detail::createPatchFromObject<op::StringPatch>(
          badge, std::move(obj))};
    case Value::Type::binaryValue:
      return DynamicPatch{detail::createPatchFromObject<op::BinaryPatch>(
          badge, std::move(obj))};
    case Value::Type::listValue:
      return DynamicPatch{detail::createPatchFromObject<DynamicListPatch>(
          badge, std::move(obj))};
    case Value::Type::setValue:
      return DynamicPatch{detail::createPatchFromObject<DynamicSetPatch>(
          badge, std::move(obj))};
    case Value::Type::mapValue:
      return DynamicPatch{detail::createPatchFromObject<DynamicMapPatch>(
          badge, std::move(obj))};
    case Value::Type::objectValue:
      // We are not sure whether it's a struct/union/any patch
      return DynamicPatch{detail::createPatchFromObject<DynamicUnknownPatch>(
          badge, std::move(obj))};
    case Value::Type::__EMPTY__:
      folly::throw_exception<std::runtime_error>("value is empty.");
    default:
      notImpl();
  }
}

Value emptyValue(const Value::Type& type) {
  Value value;
  op::invoke_by_field_id<Value>(
      static_cast<FieldId>(type),
      [&](auto id) { op::get<decltype(id)>(value).emplace(); },
      [] { folly::throw_exception<std::runtime_error>("Not Implemented."); });
  return value;
}

} // namespace

namespace detail {

constexpr std::string_view kPatchUriSuffix = "Patch";

void convertStringToBinary(ValueList& list) {
  for (Value& i : list) {
    convertStringToBinary(i);
  }
}

void convertStringToBinary(ValueSet& set) {
  ValueSet a;
  for (auto i : set) {
    convertStringToBinary(i);
    a.insert(std::move(i));
  }
  set = std::move(a);
}

void convertStringToBinary(ValueMap& map) {
  ValueMap a;
  for (const auto& kv : map) {
    auto k = kv.first;
    auto v = kv.second;
    convertStringToBinary(k);
    convertStringToBinary(v);
    a[std::move(k)] = std::move(v);
  }
  map = std::move(a);
}

void convertStringToBinary(Object& obj) {
  for (auto& [id, field] : obj) {
    convertStringToBinary(field);
  }
}

void convertStringToBinary(Value& v) {
  switch (v.getType()) {
    case Value::Type::boolValue:
    case Value::Type::byteValue:
    case Value::Type::i16Value:
    case Value::Type::i32Value:
    case Value::Type::i64Value:
    case Value::Type::floatValue:
    case Value::Type::doubleValue:
    case Value::Type::binaryValue:
      return;
    case Value::Type::stringValue: {
      auto tmp = std::move(v.as_string());
      v.emplace_binary(*folly::IOBuf::copyBuffer(tmp.data(), tmp.size()));
      return;
    }
    case Value::Type::listValue:
      return convertStringToBinary(v.as_list());
    case Value::Type::setValue:
      return convertStringToBinary(v.as_set());
    case Value::Type::mapValue:
      return convertStringToBinary(v.as_map());
    case Value::Type::objectValue:
      return convertStringToBinary(v.as_object());
    case Value::Type::__EMPTY__:
    default:
      notImpl();
  }
}

type::Type toPatchType(type::Type input) {
  for (auto t :
       {input.toThrift().name()->structType_ref(),
        input.toThrift().name()->unionType_ref()}) {
    if (!t) {
      continue;
    }
    if (auto p = t->uri_ref()) {
      *p = toPatchUri(*p);
      return input;
    }
    if (auto p = t->scopedName_ref()) {
      *p = toPatchUri(*p);
      return input;
    }
    folly::throw_exception<std::runtime_error>(fmt::format(
        "Unsupported Uri: {}",
        apache::thrift::util::enumNameSafe(t->getType())));
  }

  folly::throw_exception<std::runtime_error>(fmt::format(
      "Unsupported type: {}",
      apache::thrift::util::enumNameSafe(input.toThrift().name()->getType())));
}

void checkSameType(
    const apache::thrift::protocol::Value& v1,
    const apache::thrift::protocol::Value& v2) {
  if (v1.getType() != v2.getType()) {
    throw std::runtime_error(fmt::format(
        "Value type does not match: ({}) v.s. ({})",
        apache::thrift::util::enumNameSafe(v1.getType()),
        apache::thrift::util::enumNameSafe(v2.getType())));
  }
}
void checkCompatibleType(
    const ValueList& l, const apache::thrift::protocol::Value& v) {
  if (!l.empty()) {
    if (l.back().getType() != v.getType()) {
      throw std::runtime_error(fmt::format(
          "Type of value ({}) does not match value type of list ({}) in patch.",
          apache::thrift::util::enumNameSafe(l.back().getType()),
          apache::thrift::util::enumNameSafe(v.getType())));
    }
  }
}
void checkCompatibleType(
    const ValueSet& s, const apache::thrift::protocol::Value& v) {
  if (!s.empty()) {
    auto it = s.begin();
    if (it->getType() != v.getType()) {
      throw std::runtime_error(fmt::format(
          "Type of value ({}) does not match value type of set ({}) in patch.",
          apache::thrift::util::enumNameSafe(it->getType()),
          apache::thrift::util::enumNameSafe(v.getType())));
    }
  }
}

void checkHomogeneousContainerImpl(const auto& l) {
  if (std::adjacent_find(
          l.begin(), l.end(), [](const Value& v1, const Value& v2) {
            return v1.getType() != v2.getType();
          }) == l.end()) {
    return;
  }
  throw std::runtime_error("Thrift does not support heterogeneous conatiner.");
}
void checkHomogeneousContainer(const ValueList& l) {
  checkHomogeneousContainerImpl(l);
}
void checkHomogeneousContainer(const ValueSet& s) {
  checkHomogeneousContainerImpl(s);
}
void checkHomogeneousContainer(const ValueMap& m) {
  if (std::adjacent_find(
          m.begin(), m.end(), [](const auto& p1, const auto& p2) {
            return p1.first.getType() != p2.first.getType() ||
                p1.second.getType() != p2.second.getType();
          }) == m.end()) {
    return;
  }
  throw std::runtime_error("Thrift does not support heterogeneous conatiner.");
}

} // namespace detail

std::string toPatchUri(std::string s) {
  s += detail::kPatchUriSuffix;
  return s;
}

std::string fromPatchUri(std::string s) {
  auto newSize = s.size() - detail::kPatchUriSuffix.size();
  if (s.size() <= detail::kPatchUriSuffix.size() ||
      s.substr(newSize) != detail::kPatchUriSuffix) {
    folly::throw_exception<std::invalid_argument>(
        fmt::format("Uri {} is not a Patch.", s));
  }
  s.resize(newSize);
  return s;
}

bool DynamicPatch::empty(detail::Badge badge) const {
  return std::visit(
      [&](auto&& v) {
        if constexpr (__FBTHRIFT_IS_VALID(v, v.empty(badge))) {
          return v.empty(badge);
        } else {
          return v.empty();
        }
      },
      *patch_);
}

bool DynamicPatch::empty() const {
  return empty(badge);
}
DynamicPatch& DynamicMapPatch::patchByKey(detail::Badge, Value&& k) {
  return patchByKey(badge, std::move(k), {});
}
DynamicPatch& DynamicMapPatch::patchByKey(detail::Badge, const Value& k) {
  return patchByKey(badge, k, {});
}

template <class SubPatch>
DynamicPatch& DynamicMapPatch::patchByKeyImpl(Value k, SubPatch&& p) {
  ensurePatchable();
  auto& patch = (add_.contains(k) || put_.contains(k) || remove_.contains(k))
      ? patchAfter_
      : patchPrior_;
  if (auto subPatch = get_ptr(patch, k)) {
    subPatch->merge(std::forward<SubPatch>(p));
    return *subPatch;
  } else {
    return patch.emplace(std::move(k), std::forward<SubPatch>(p)).first->second;
  }
}
Object DynamicMapPatch::toObject() && {
  Object ret;
  if (assign_) {
    ret[FieldId(op::PatchOp::Assign)].emplace_map(std::move(*assign_));
    return ret;
  }

  if (clear_) {
    ret[FieldId(op::PatchOp::Clear)].emplace_bool(true);
  }

  for (auto& [k, p] : patchPrior_) {
    ret[FieldId(op::PatchOp::PatchPrior)].ensure_map()[k].emplace_object(
        std::move(p).toObject());
  }

  for (auto& [k, p] : patchAfter_) {
    ret[FieldId(op::PatchOp::PatchAfter)].ensure_map()[k].emplace_object(
        std::move(p).toObject());
  }

  if (!add_.empty()) {
    ret[FieldId(op::PatchOp::EnsureStruct)].emplace_map(add_);
  }

  if (!remove_.empty()) {
    ret[FieldId(op::PatchOp::Remove)].emplace_set(std::move(remove_));
  }

  if (!put_.empty()) {
    ret[FieldId(op::PatchOp::Put)].emplace_map(std::move(put_));
  }

  if (!options_.doNotConvertStringToBinary) {
    detail::convertStringToBinary(ret);
  }
  return ret;
}
Object DynamicMapPatch::toObject() const& {
  return DynamicMapPatch(*this).toObject();
}
void DynamicMapPatch::fromObject(detail::Badge, Object obj) {
  if (auto assign = get_ptr(obj, op::PatchOp::Assign)) {
    assign_ = std::move(assign->as_map());
    return;
  }

  if (auto clear = get_ptr(obj, op::PatchOp::Clear)) {
    clear_ = clear->as_bool();
  }

  if (auto patchPrior = get_ptr(obj, op::PatchOp::PatchPrior)) {
    for (auto& [k, v] : patchPrior->as_map()) {
      patchPrior_[k].fromObject(badge, std::move(v.as_object()));
    }
  }

  if (auto patchAfter = get_ptr(obj, op::PatchOp::PatchAfter)) {
    for (auto& [k, v] : patchAfter->as_map()) {
      patchAfter_[k].fromObject(badge, std::move(v.as_object()));
    }
  }

  if (auto add = get_ptr(obj, op::PatchOp::EnsureStruct)) {
    add_ = std::move(add->as_map());
  }

  if (auto remove = get_ptr(obj, op::PatchOp::Remove)) {
    remove_ = std::move(remove->as_set());
  }

  if (auto put = get_ptr(obj, op::PatchOp::Put)) {
    put_ = std::move(put->as_map());
  }
}

void DynamicMapPatch::insert_or_assign(detail::Badge, Value k, Value v) {
  undoChanges(k);
  setOrCheckMapType(k, v);
  put_.insert_or_assign(std::move(k), std::move(v));
}

void DynamicMapPatch::erase(detail::Badge, Value k) {
  undoChanges(k);
  remove_.insert(std::move(k));
}
void DynamicMapPatch::tryPutMulti(detail::Badge, detail::ValueMap map) {
  detail::checkHomogeneousContainer(map);
  ensurePatchable();
  map.eraseInto(map.begin(), map.end(), [&](auto&& k, auto&& v) {
    setOrCheckMapType(k, v);
    if (put_.contains(k)) {
      // We already `put` the value, ignore `add`
      return;
    } else if (remove_.erase(k)) {
      // We removed the `value`, we need to add it back as `put`
      patchAfter_.erase(k);
      put_.insert_or_assign(std::move(k), std::move(v));
    } else {
      add_.try_emplace(std::move(k), std::move(v));
    }
  });
}

void DynamicMapPatch::ensurePatchable() {
  if (assign_) {
    put_ = std::move(*assign_);
    clear_ = true;
    assign_.reset();
  }
}

void DynamicMapPatch::undoChanges(const Value& k) {
  ensurePatchable();
  add_.erase(k);
  patchPrior_.erase(k);
  patchAfter_.erase(k);
  remove_.erase(k);
  put_.erase(k);
}

void DynamicMapPatch::apply(detail::Badge, detail::ValueMap& v) const {
  struct Visitor {
    void assign(detail::Badge, detail::ValueMap v) {
      detail::convertStringToBinary(v);
      value = std::move(v);
    }
    void clear(detail::Badge) { value.clear(); }
    void patchByKey(detail::Badge, Value key, const DynamicPatch& p) {
      detail::convertStringToBinary(key);
      if (auto ptr = folly::get_ptr(value, key)) {
        p.apply(*ptr);
      }
    }
    void removeMulti(detail::Badge, const detail::ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.erase(i);
      }
    }
    void tryPutMulti(detail::Badge, const detail::ValueMap& map) {
      for (const auto& kv : map) {
        auto k = kv.first;
        auto v = kv.second;
        detail::convertStringToBinary(k);
        detail::convertStringToBinary(v);
        value.emplace(std::move(k), std::move(v));
      }
    }
    void putMulti(detail::Badge, const detail::ValueMap& map) {
      for (const auto& kv : map) {
        auto k = kv.first;
        auto v = kv.second;
        detail::convertStringToBinary(k);
        detail::convertStringToBinary(v);
        value.insert_or_assign(std::move(k), std::move(v));
      }
    }
    detail::ValueMap& value;
  };

  return customVisit(badge, Visitor{v});
}

void DynamicMapPatch::setOrCheckMapType(
    const protocol::Value& k, const protocol::Value& v) {
  if (keyType_) {
    if (*keyType_ != k.getType()) {
      throw std::runtime_error(fmt::format(
          "Type of value ({}) does not match key type of map ({}) in patch.",
          apache::thrift::util::enumNameSafe(k.getType()),
          apache::thrift::util::enumNameSafe(*keyType_)));
    }
  } else {
    keyType_ = k.getType();
  }
  if (valueType_) {
    if (*valueType_ != v.getType()) {
      throw std::runtime_error(fmt::format(
          "Type of value ({}) does not match value type of map ({}) in patch.",
          apache::thrift::util::enumNameSafe(v.getType()),
          apache::thrift::util::enumNameSafe(*valueType_)));
    }
  } else {
    valueType_ = v.getType();
  }
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::ensurePatchable() {
  if (!assign_) {
    return;
  }

  clear_ = true;
  for (auto&& [id, field] : *assign_) {
    auto fieldId = static_cast<FieldId>(id);
    ensure_[fieldId] = emptyValue(field.getType());
    Object patch;
    patch[static_cast<FieldId>(op::PatchOp::Assign)] = std::move(field);
    patchAfter_[fieldId].fromObject(badge, patch);
  }
  assign_.reset();
}
template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::undoChanges(FieldId id) {
  ensurePatchable();
  patchPrior_.erase(id);
  patchAfter_.erase(id);
  ensure_.erase(id);
  remove_.erase(id);
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::ensureUnion(FieldId id, Value v) {
  ensurePatchable();

  if (ensure_.contains(id)) {
    return;
  }

  if (clear_ || !ensure_.empty()) {
    Object obj;
    obj[id] = std::move(v);
    assign(std::move(obj));
    return;
  }

  DynamicPatch tmp = std::move(patchPrior_[id]);
  tmp.merge(patchAfter_[id]);

  // After ensuring field `id`, all other fields will be cleared, thus we don't
  // need to worry about them.
  *this = {};

  patchPrior_[id] = std::move(tmp);
  ensure_[id] = std::move(v);
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::ensureStruct(FieldId id, Value v) {
  ensurePatchable();

  if (remove_.contains(id)) {
    // We removed the `value`, we need to assign it back
    undoChanges(id);
    ensure_[id] = emptyValue(v.getType());
    Object patch;
    patch[static_cast<FieldId>(op::PatchOp::Assign)] = std::move(v);
    patchAfter_[id].fromObject(badge, patch);
    return;
  }

  if (ensure_.contains(id)) {
    return;
  }

  if (patchAfter_.contains(id)) {
    patchPrior_[id].merge(patchAfter_[id]);
    patchAfter_.erase(id);
  }

  ensure_[id] = std::move(v);
}

template <bool IsUnion>
Object DynamicStructurePatch<IsUnion>::toObject() && {
  Object ret;
  if (assign_) {
    ret[FieldId(op::PatchOp::Assign)].emplace_object(std::move(*assign_));
    return ret;
  }

  if (clear_) {
    ret[FieldId(op::PatchOp::Clear)].emplace_bool(true);
  }

  for (auto id : remove_) {
    ret[FieldId(op::PatchOp::Remove)].ensure_list().push_back(
        asValueStruct<type::i16_t>(folly::to_underlying(id)));
  }

  const auto ensureOp =
      IsUnion ? op::PatchOp::EnsureUnion : op::PatchOp::EnsureStruct;

  for (const auto& [id, value] : ensure_) {
    ret[FieldId(ensureOp)].ensure_object()[id] = value;
  }

  for (auto& [id, p] : patchPrior_) {
    ret[FieldId(op::PatchOp::PatchPrior)].ensure_object()[id].emplace_object(
        std::move(p).toObject());
  }
  for (auto& [id, p] : patchAfter_) {
    ret[FieldId(op::PatchOp::PatchAfter)].ensure_object()[id].emplace_object(
        std::move(p).toObject());
  }
  if (!options_.doNotConvertStringToBinary) {
    detail::convertStringToBinary(ret);
  }
  return ret;
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::fromObject(detail::Badge, Object obj) {
  if (auto assign = get_ptr(obj, op::PatchOp::Assign)) {
    assign_ = std::move(assign->as_object());
    return;
  }

  if (auto clear = get_ptr(obj, op::PatchOp::Clear)) {
    clear_ = clear->as_bool();
  }

  if (auto remove = get_ptr(obj, op::PatchOp::Remove)) {
    for (auto& id : remove->as_list()) {
      remove_.insert(static_cast<FieldId>(id.as_i16()));
    }
  }

  const auto ensureOp =
      IsUnion ? op::PatchOp::EnsureUnion : op::PatchOp::EnsureStruct;

  if (auto ensure = get_ptr(obj, ensureOp)) {
    for (auto& [id, field] : ensure->as_object()) {
      ensure_[static_cast<FieldId>(id)] = std::move(field);
    }
  }

  if (auto patchPrior = get_ptr(obj, op::PatchOp::PatchPrior)) {
    for (auto& [k, v] : patchPrior->as_object()) {
      patchPrior_[static_cast<FieldId>(k)].fromObject(
          badge, std::move(v.as_object()));
    }
  }

  if (auto patchAfter = get_ptr(obj, op::PatchOp::PatchAfter)) {
    for (auto& [k, v] : patchAfter->as_object()) {
      patchAfter_[static_cast<FieldId>(k)].fromObject(
          badge, std::move(v.as_object()));
    }
  }
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::apply(detail::Badge, Object& obj) const {
  struct Visitor {
    void assign(Object v) {
      detail::convertStringToBinary(v);
      obj = std::move(v);
    }
    void clear() { obj.members()->clear(); }
    void patchIfSet(FieldId id, const DynamicPatch& p) {
      if (obj.contains(id)) {
        p.apply(obj[id]);
      }
    }
    void remove(FieldId id) { obj.erase(id); }
    void ensure(FieldId id, const Value& v) {
      if (obj.contains(id)) {
        return;
      }

      if (IsUnion) {
        obj.members()->clear();
      }

      auto copied = folly::copy(v);
      detail::convertStringToBinary(copied);
      obj[id] = std::move(copied);
    }
    Object& obj;
  };

  return customVisit(Visitor{obj});
}

template class DynamicStructurePatch<true>;
template class DynamicStructurePatch<false>;

DynamicPatch DiffVisitorBase::diff(const Object& src, const Object& dst) {
  return diffStructured(src, dst);
}

DynamicPatch DiffVisitorBase::diff(const Value& src, const Value& dst) {
  return diff(badge, src, dst);
}

DynamicPatch DiffVisitorBase::diff(
    detail::Badge, const Value& src, const Value& dst) {
  if (src.getType() != dst.getType()) {
    folly::throw_exception<std::runtime_error>(fmt::format(
        "Value types mismatch: {} and {}.",
        util::enumNameSafe<Value::Type>(src.getType()),
        util::enumNameSafe<Value::Type>(dst.getType())));
  }

  if (src == dst) {
    return createPatchFromObjectAsValueType(src, {});
  }

  switch (src.getType()) {
    case Value::Type::boolValue:
      return DynamicPatch{diffBool(src.as_bool(), dst.as_bool())};
    case Value::Type::byteValue:
      return DynamicPatch{diffByte(src.as_byte(), dst.as_byte())};
    case Value::Type::i16Value:
      return DynamicPatch{diffI16(src.as_i16(), dst.as_i16())};
    case Value::Type::i32Value:
      return DynamicPatch{diffI32(src.as_i32(), dst.as_i32())};
    case Value::Type::i64Value:
      return DynamicPatch{diffI64(src.as_i64(), dst.as_i64())};
    case Value::Type::floatValue:
      return DynamicPatch{diffFloat(src.as_float(), dst.as_float())};
    case Value::Type::doubleValue:
      return DynamicPatch{diffDouble(src.as_double(), dst.as_double())};
    case Value::Type::binaryValue:
      return DynamicPatch{diffBinary(src.as_binary(), dst.as_binary())};
    case Value::Type::listValue:
      return DynamicPatch{diffList(src.as_list(), dst.as_list())};
    case Value::Type::setValue:
      return DynamicPatch{diffSet(src.as_set(), dst.as_set())};
    case Value::Type::mapValue:
      return DynamicPatch{diffMap(src.as_map(), dst.as_map())};
    case Value::Type::objectValue:
      return DynamicPatch{diffStructured(src.as_object(), dst.as_object())};
    case Value::Type::__EMPTY__:
      folly::throw_exception<std::runtime_error>("value is empty.");
    case Value::Type::stringValue:
    default:
      notImpl();
  }
}

std::size_t DynamicPatchBase::patchedElementCount(detail::Badge) const {
  size_t count = 0;
  for (const auto& [_, value] : patch_) {
    if (auto p = value.if_list()) {
      count += p->size();
    } else if (auto p = value.if_set()) {
      count += p->size();
    } else if (auto p = value.if_map()) {
      count += p->size();
    } else {
      folly::throw_exception<std::runtime_error>(fmt::format(
          "Value is not a container but `{}`",
          apache::thrift::util::enumNameSafe(value.getType())));
    }
  }
  return count;
}

op::BoolPatch DiffVisitorBase::diffBool(bool src, bool dst) {
  return createArithmeticPatch<type::bool_t>(src, dst);
}

op::BytePatch DiffVisitorBase::diffByte(std::int8_t src, std::int8_t dst) {
  return createArithmeticPatch<type::byte_t>(src, dst);
}

op::I16Patch DiffVisitorBase::diffI16(std::int16_t src, std::int16_t dst) {
  return createArithmeticPatch<type::i16_t>(src, dst);
}

op::I32Patch DiffVisitorBase::diffI32(std::int32_t src, std::int32_t dst) {
  return createArithmeticPatch<type::i32_t>(src, dst);
}

op::I64Patch DiffVisitorBase::diffI64(std::int64_t src, std::int64_t dst) {
  return createArithmeticPatch<type::i64_t>(src, dst);
}

op::FloatPatch DiffVisitorBase::diffFloat(float src, float dst) {
  return createArithmeticPatch<type::float_t>(src, dst);
}
op::DoublePatch DiffVisitorBase::diffDouble(double src, double dst) {
  return createArithmeticPatch<type::double_t>(src, dst);
}

op::BinaryPatch DiffVisitorBase::diffBinary(
    const folly::IOBuf& srcBuf, const folly::IOBuf& dstBuf) {
  auto src = srcBuf.to<std::string>();
  auto dst = dstBuf.to<std::string>();

  op::BinaryPatch patch;

  if (dst.empty()) {
    patch.clear();
    return patch;
  }

  auto pos = dst.find(src);

  if (pos == std::string::npos) {
    patch = dst;
    return patch;
  }

  if (pos != 0) {
    patch.prepend(dst.substr(0, pos));
  }

  if (pos + src.length() != dst.length()) {
    patch += dst.substr(pos + src.length(), dst.length());
  }

  return patch;
}

DynamicListPatch DiffVisitorBase::diffList(
    const ValueList& src, const ValueList& dst) {
  DynamicListPatch patch;
  patch.doNotConvertStringToBinary(badge);

  if (dst.empty()) {
    patch.clear(badge);
    return patch;
  }

  auto it = std::search(dst.begin(), dst.end(), src.begin(), src.end());
  if (it != dst.begin()) {
    patch.assign(badge, dst);
    return patch;
  }

  for (size_t i = src.size(); i < dst.size(); ++i) {
    patch.push_back(badge, dst[i]);
  }
  return patch;
}

DynamicSetPatch DiffVisitorBase::diffSet(
    const ValueSet& src, const ValueSet& dst) {
  DynamicSetPatch patch;
  patch.doNotConvertStringToBinary(badge);

  if (dst.empty()) {
    patch.clear(badge);
    return patch;
  }

  // remove keys
  for (const auto& i : src) {
    if (!dst.contains(i)) {
      patch.erase(badge, i);
    }
  }
  if (dst.size() < patch.patchedElementCount(badge)) {
    patch.assign(badge, dst);
    return patch;
  }

  // add keys
  for (const auto& i : dst) {
    if (!src.contains(i)) {
      patch.insert(badge, i);
    }
  }
  if (dst.size() < patch.patchedElementCount(badge)) {
    patch.assign(badge, dst);
    return patch;
  }

  return patch;
}

DynamicMapPatch DiffVisitorBase::diffMap(
    const ValueMap& src, const ValueMap& dst) {
  DynamicMapPatch patch;
  patch.doNotConvertStringToBinary(badge);

  if (dst.empty()) {
    patch.clear(badge);
    return patch;
  }

  folly::F14FastSet<
      std::reference_wrapper<const Value>,
      std::hash<Value>,
      std::equal_to<Value>>
      keys;

  keys.reserve(src.size() + dst.size());

  for (const auto& [k, _] : src) {
    keys.insert(k);
  }

  for (const auto& [k, _] : dst) {
    keys.insert(k);
  }

  // Remove keys with the same value
  erase_if(keys, [&](const auto& key) {
    auto p = get_ptr(src, key), q = get_ptr(dst, key);
    return p && q && *p == *q;
  });

  if (dst.size() < keys.size()) {
    patch.assign(badge, dst);
    return patch;
  }

  for (const auto& k : keys) {
    diffElement(src, dst, k, patch);
  }
  return patch;
}

// Check whether value should not be serialized due to deprecated_terse_writes,
// but serialized in languages other than C++.
static bool maybeEmptyDeprecatedTerseField(const Value& value) {
  switch (value.getType()) {
    case Value::Type::boolValue:
    case Value::Type::byteValue:
    case Value::Type::i16Value:
    case Value::Type::i32Value:
    case Value::Type::i64Value:
    case Value::Type::floatValue:
    case Value::Type::doubleValue:
      // Numeric fields maybe empty terse field regardless the value, since it
      // honors custom default
      return true;
    case Value::Type::stringValue:
    case Value::Type::binaryValue:
    case Value::Type::listValue:
    case Value::Type::setValue:
    case Value::Type::mapValue:
      // string/binary and containers fields don't honor custom default.
      // It can only be empty if it is intrinsic default
      return protocol::isIntrinsicDefault(value);
    case Value::Type::objectValue:
      // struct/union/exception can never be empty terse field
      return false;
    case Value::Type::__EMPTY__:
      folly::throw_exception<std::runtime_error>("value is empty.");
    default:
      notImpl();
  }
}

void DiffVisitorBase::diffElement(
    const ValueMap& src,
    const ValueMap& dst,
    const Value& key,
    DynamicMapPatch& patch) {
  const bool inSrc = src.contains(key);
  const bool inDst = dst.contains(key);

  if (!inSrc && !inDst) {
    return;
  }

  if (inSrc && !inDst) {
    patch.erase(badge, key);
    return;
  }

  if (!inSrc && inDst) {
    patch.insert_or_assign(badge, key, dst.at(key));
    return;
  }

  Object elementPatch;
  pushKey(key);
  auto guard = folly::makeGuard([&] { pop(); });
  auto subPatch = diff(badge, src.at(key), dst.at(key));
  patch.patchByKey(badge, key, DynamicPatch{std::move(subPatch)});
}

DynamicPatch DiffVisitorBase::diffStructured(
    const Object& src, const Object& dst) {
  if (src.empty() && dst.empty()) {
    return {};
  }

  if (src.size() <= 1 && dst.size() <= 1) {
    // If the src and dst looks like an union (no more than one field), we can
    // not tell whether it's a struct or an union. In most cases we should use
    // AssignPatch, unless both struct have the same field, in which case we can
    // use PatchPrior which works for both struct and union and it's smaller
    // than AssignPatch.

    // If both struct doesn't have the same field, the normal diffing logic uses
    // EnsureStruct and Remove. Both are not supported by union patch, thus we
    // need to use AssignPatch.
    bool shouldUseAssignPatch =
        src.empty() || dst.empty() || src.begin()->first != dst.begin()->first;

    // If field is src looks like deprecated terse field, we need to use
    // EnsureStruct, which is not supported by UnionPatch, thus we need to use
    // AssignPatch.
    shouldUseAssignPatch = shouldUseAssignPatch ||
        (src.begin()->second != dst.begin()->second &&
         maybeEmptyDeprecatedTerseField(src.begin()->second));

    if (shouldUseAssignPatch) {
      DynamicUnknownPatch patch;
      patch.doNotConvertStringToBinary(badge);
      patch.assign(badge, dst);
      return DynamicPatch{std::move(patch)};
    }
  }

  DynamicStructPatch patch;
  patch.doNotConvertStringToBinary(badge);

  for (const auto& [id, _] : src) {
    FieldId fieldId{id};
    diffField(src, dst, fieldId, patch);
  }

  for (const auto& [id, _] : dst) {
    FieldId fieldId{id};
    if (!src.contains(fieldId)) {
      diffField(src, dst, fieldId, patch);
    }
  }

  if (src.size() <= 1 && dst.size() <= 1) {
    // If the input contains less or equal to one field, we can't tell whether
    // it's a struct or an union. We need to use unknown patch here.
    DynamicUnknownPatch ret;
    ret.fromObject(badge, patch.toObject());
    return DynamicPatch{ret};
  }

  return DynamicPatch{patch};
}

DynamicUnionPatch DiffVisitorBase::diffUnion(
    const Object& src, const Object& dst) {
  DCHECK(src.size() <= 1);
  DCHECK(dst.size() <= 1);
  DynamicUnionPatch patch;

  if (dst.empty()) {
    if (!src.empty()) {
      patch.clear();
    }
    return patch;
  }

  if (src.empty() || src.begin()->first != dst.begin()->first) {
    patch.assign(dst);
    return patch;
  }

  auto id = static_cast<FieldId>(src.begin()->first);
  pushField(id);
  auto guard = folly::makeGuard([&] { pop(); });
  auto subPatch = diff(badge, src.at(id), dst.at(id));
  if (!subPatch.empty(badge)) {
    patch.patchIfSet(id).merge(DynamicPatch{std::move(subPatch)});
  }

  return patch;
}

op::AnyPatch DiffVisitorBase::diffAny(
    const Object& srcObj, const Object& dstObj) {
  op::AnyPatch patch;
  using Tag = type::struct_t<type::AnyStruct>;

  type::AnyStruct src, dst;
  if (!detail::ProtocolValueToThriftValue<type::struct_t<type::AnyStruct>>{}(
          srcObj, src)) {
    throw std::runtime_error("Failed to convert source object to AnyStruct");
  }
  if (!detail::ProtocolValueToThriftValue<type::struct_t<type::AnyStruct>>{}(
          dstObj, dst)) {
    throw std::runtime_error(
        "Failed to convert destination object to AnyStruct");
  }

  if (op::isEmpty<Tag>(src) || !type::identicalType(*src.type(), *dst.type())) {
    patch.assign(std::move(dst));
    return patch;
  }

  auto srcValue = protocol::detail::parseValueFromAny(src);
  auto dstValue = protocol::detail::parseValueFromAny(dst);

  pushType(*src.type());
  auto guard = folly::makeGuard([&] { pop(); });
  auto subPatch = diff(badge, srcValue, dstValue);
  if (!subPatch.empty(badge)) {
    type::AnyStruct anySubPatch;
    anySubPatch.protocol() = type::StandardProtocol::Compact;
    anySubPatch.data() =
        *serializeObject<CompactProtocolWriter>(subPatch.toObject());
    anySubPatch.type() = detail::toPatchType(*src.type());
    patch.patchIfTypeIs(*src.type(), std::move(anySubPatch));
  }

  return patch;
}

void DiffVisitorBase::diffField(
    const Object& src,
    const Object& dst,
    FieldId id,
    DynamicStructPatch& patch) {
  const bool inSrc = src.contains(id);
  const bool inDst = dst.contains(id);

  if (!inSrc && !inDst) {
    return;
  }

  if (inSrc && !inDst) {
    patch.remove(id);
    return;
  }

  if (!inSrc && inDst) {
    pushField(id);
    auto guard = folly::makeGuard([&] { pop(); });
    auto& field = dst.at(id);

    // patch after
    // We can't ensure the field because it will be ignored for non-optional
    // field when applied statically.
    auto empty = emptyValue(field.getType());
    auto subPatch = diff(badge, empty, field);
    patch.ensure(id, std::move(empty));
    patch.patchIfSet(id).merge(DynamicPatch{std::move(subPatch)});
    return;
  }

  pushField(id);
  auto guard = folly::makeGuard([&] { pop(); });
  auto subPatch = diff(badge, src.at(id), dst.at(id));
  if (!subPatch.empty(badge)) {
    if (maybeEmptyDeprecatedTerseField(src.at(id))) {
      patch.ensure(id, emptyValue(src.at(id).getType()));
    }
    patch.patchIfSet(id).merge(DynamicPatch{std::move(subPatch)});
  }
}

void DiffVisitorBase::pushField(FieldId id) {
  auto last = maskInPath_.top();
  auto next = &last->includes_ref().ensure()[folly::to_underlying(id)];
  *next = allMask();
  maskInPath_.push(next);
}

void DiffVisitorBase::pushType(type::Type type) {
  auto last = maskInPath_.top();
  auto next = &last->includes_type_ref().ensure()[std::move(type)];
  *next = allMask();
  maskInPath_.push(next);
}

static std::optional<std::int64_t> getIntegral(const Value& v) {
  switch (v.getType()) {
    case Value::Type::boolValue:
      return v.as_bool();
    case Value::Type::byteValue:
      return v.as_byte();
    case Value::Type::i16Value:
      return v.as_i16();
    case Value::Type::i32Value:
      return v.as_i32();
    case Value::Type::i64Value:
      return v.as_i64();
    default:
      return {};
  }
}

DynamicPatch DiffVisitorBase::createDynamicUnknownPatchWithAssign(
    const Object& obj) {
  DynamicUnknownPatch patch;
  patch.doNotConvertStringToBinary(badge);
  patch.assign(badge, obj);
  return DynamicPatch{std::move(patch)};
}

void DiffVisitorBase::pushKey(const Value& k) {
  auto last = maskInPath_.top();
  Mask* next = nullptr;
  if (auto i = getIntegral(k)) {
    next = &last->includes_map_ref().ensure()[*i];
  } else if (k.is_binary()) {
    next = &last->includes_string_map_ref().ensure()[k.as_binary().toString()];
  } else {
    // The map key is not integer, nor string
    notImpl();
  }
  *next = allMask();
  maskInPath_.push(next);
}

void DiffVisitorBase::pop() {
  maskInPath_.pop();
  *maskInPath_.top() = allMask();
}

void DynamicListPatch::apply(detail::Badge, detail::ValueList& v) const {
  struct Visitor {
    void assign(detail::Badge, detail::ValueList v) {
      detail::convertStringToBinary(v);
      value = std::move(v);
    }
    void clear(detail::Badge) { value.clear(); }
    void push_back(detail::Badge, detail::Value v) {
      detail::convertStringToBinary(v);
      value.push_back(std::move(v));
    }
    detail::ValueList& value;
  };

  return customVisit(badge, Visitor{v});
}

void DynamicSetPatch::apply(detail::Badge, detail::ValueSet& v) const {
  struct Visitor {
    void assign(detail::Badge, detail::ValueSet v) {
      detail::convertStringToBinary(v);
      value = std::move(v);
    }
    void clear(detail::Badge) { value.clear(); }
    void addMulti(detail::Badge, const detail::ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.insert(std::move(i));
      }
    }
    void removeMulti(detail::Badge, const detail::ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.erase(i);
      }
    }
    detail::ValueSet& value;
  };

  return customVisit(badge, Visitor{v});
}

template <class Other>
detail::if_same_type_after_remove_cvref<Other, DynamicPatch>
DynamicPatch::merge(Other&& other) {
  // If only one of the patch is Unknown patch, convert the Unknown patch type
  // to the known patch type.
  if (std::holds_alternative<DynamicUnknownPatch>(*patch_) &&
      !std::holds_alternative<DynamicUnknownPatch>(*other.patch_)) {
    std::visit(
        [&](auto&& other) {
          *patch_ = detail::createPatchFromObject<
              folly::remove_cvref_t<decltype(other)>>(
              badge, std::move(*this).toObject());
        },
        *other.patch_);
  }
  if (!std::holds_alternative<DynamicUnknownPatch>(*patch_) &&
      std::holds_alternative<DynamicUnknownPatch>(*other.patch_)) {
    return std::visit(
        [&](auto&& patch) {
          auto tmp = DynamicPatch{detail::createPatchFromObject<
              folly::remove_cvref_t<decltype(patch)>>(
              badge, std::forward<Other>(other).toObject())};
          return merge(std::move(tmp));
        },
        *patch_);
  }
  if (std::holds_alternative<op::StringPatch>(*patch_) &&
      std::holds_alternative<op::BinaryPatch>(*other.patch_)) {
    *patch_ = detail::createPatchFromObject<op::BinaryPatch>(
        badge, std::move(*this).toObject());
    return merge(std::forward<Other>(other));
  }
  if (std::holds_alternative<op::BinaryPatch>(*patch_) &&
      std::holds_alternative<op::StringPatch>(*other.patch_)) {
    *patch_ = detail::createPatchFromObject<op::StringPatch>(
        badge, std::move(*this).toObject());
    return merge(std::forward<Other>(other));
  }

  if (other.empty(badge)) {
    return;
  }

  if (empty(badge)) {
    *this = other;
    return;
  }

  std::visit(
      [](auto&& l, auto&& r) {
        using L = folly::remove_cvref_t<decltype(l)>;
        using R = folly::remove_cvref_t<decltype(r)>;
        if constexpr (std::is_same_v<L, R>) {
          if constexpr (__FBTHRIFT_IS_VALID(
                            l, l.merge(badge, folly::forward_like<Other>(r)))) {
            l.merge(badge, folly::forward_like<Other>(r));
          } else {
            l.merge(folly::forward_like<Other>(r));
          }
        } else {
          folly::throw_exception<std::runtime_error>(fmt::format(
              "{}::merge({}) is not implemented",
              folly::pretty_name<L>(),
              folly::pretty_name<R>()));
        }
      },
      *patch_,
      *other.patch_);
}

template void DynamicPatch::merge(DynamicPatch&);
template void DynamicPatch::merge(DynamicPatch&&);
template void DynamicPatch::merge(const DynamicPatch&);
template void DynamicPatch::merge(const DynamicPatch&&);

void DynamicPatch::fromObject(detail::Badge, Object obj) {
  if (auto p = get_ptr(obj, op::PatchOp::EnsureStruct)) {
    if (p->getType() == Value::Type::objectValue) {
      DynamicStructPatch patch;
      patch.fromObject(badge, std::move(obj));
      *patch_ = std::move(patch);
      return;
    }
    if (p->getType() != Value::Type::mapValue) {
      folly::throw_exception<std::runtime_error>(
          "Unknown EnsureStruct type: " + util::enumNameSafe(p->getType()));
    }
    DynamicMapPatch patch;
    patch.fromObject(badge, std::move(obj));
    *patch_ = std::move(patch);
    return;
  }

  if (get_ptr(obj, op::PatchOp::EnsureUnion)) {
    DynamicUnionPatch patch;
    patch.fromObject(badge, std::move(obj));
    *patch_ = std::move(patch);
    return;
  }

  // If the `remove` operation is a list, it must be struct patch.
  if (auto remove = get_ptr(obj, op::PatchOp::Remove)) {
    if (remove->is_list()) {
      DynamicStructPatch patch;
      patch.fromObject(badge, std::move(obj));
      *patch_ = std::move(patch);
      return;
    }
  }

  if (get_ptr(obj, op::PatchOp::PatchIfTypeIsPrior) ||
      get_ptr(obj, op::PatchOp::PatchIfTypeIsAfter) ||
      get_ptr(obj, op::PatchOp::EnsureAny)) {
    *patch_ =
        detail::createPatchFromObject<op::AnyPatch>(badge, std::move(obj));
    return;
  }

  // If patch contains the following operations, the patch type must be the same
  // as operation's value type.
  // E.g., for map patch, the EnsureStruct field is also a map.
  // Reference:
  // https://www.internalfb.com/intern/staticdocs/thrift/docs/contributions/patch/
  for (auto op : {
           op::PatchOp::Assign,
           op::PatchOp::PatchPrior,
           op::PatchOp::PatchAfter,
           op::PatchOp::Add,
           op::PatchOp::Put,
       }) {
    if (auto ptr = get_ptr(obj, op)) {
      *this = createPatchFromObjectAsValueType(*ptr, std::move(obj));
      return;
    }
  }

  DynamicUnknownPatch patch;
  patch.fromObject(badge, std::move(obj));
  *patch_ = std::move(patch);
}

auto DynamicUnknownPatch::validateAndGetCategory() const -> Category {
  auto throwExceptionIf = [&](bool b) {
    if (b) {
      std::runtime_error(
          "Unclassified unknown patch. "
          "Either there is a bug in the Unknown Patch implementation "
          "or the input is incorrect. Patch content: " +
          debugStringViaEncode(patch_));
    }
  };

  auto onlyHasOps = [&](folly::F14FastSet<op::PatchOp> ops) {
    for (const auto& [k, _] : patch_) {
      if (!ops.contains(static_cast<op::PatchOp>(k))) {
        return false;
      }
    }
    return true;
  };

  if (patch_.empty()) {
    return Category::EmptyPatch;
  }

  if (auto assign = get_ptr(op::PatchOp::Assign)) {
    throwExceptionIf(!assign->is_object());
    return Category::StructuredOrAnyPatch;
  }

  if (onlyHasOps({op::PatchOp::Clear})) {
    return Category::ClearPatch;
  }

  if (onlyHasOps({op::PatchOp::Clear, op::PatchOp::Remove})) {
    throwExceptionIf(!get_ptr(op::PatchOp::Remove)->is_set());
    return Category::AssociativeContainerPatch;
  }

  if (onlyHasOps(
          {op::PatchOp::Clear,
           op::PatchOp::PatchPrior,
           op::PatchOp::PatchAfter})) {
    return Category::StructuredOrAnyPatch;
  }

  throwExceptionIf(true);
  return {};
}
void DynamicUnknownPatch::throwIncompatibleCategory(
    std::string_view method) const {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "Modifying thrift patch in an incompatible way. operation: {}, patch content: {}",
      method,
      debugStringViaEncode(patch_)));
}
void DynamicUnknownPatch::removeMulti(
    detail::Badge, const detail::ValueSet& v) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::AssociativeContainerPatch})) {
    throwIncompatibleCategory("remove");
  }

  auto& s = get(op::PatchOp::Remove).ensure_set();
  for (const auto& k : v) {
    s.insert(k);
  }
}
void DynamicUnknownPatch::assign(detail::Badge, Object v) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::StructuredOrAnyPatch})) {
    throwIncompatibleCategory("assign");
  }

  patch_.members()->clear();
  get(op::PatchOp::Assign).emplace_object(std::move(v));
}
void DynamicUnknownPatch::patchIfSet(
    detail::Badge, FieldId id, const DynamicPatch& p) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::StructuredOrAnyPatch})) {
    throwIncompatibleCategory("patchIfSet");
  }
  if (auto assign = get_ptr(op::PatchOp::Assign)) {
    if (assign->as_object().contains(id)) {
      p.apply(assign->as_object().at(id));
    }
    return;
  }

  // TODO: optimize this
  DynamicPatch prior, after;
  prior.fromObject(
      badge, get(op::PatchOp::PatchPrior).ensure_object()[id].ensure_object());
  after.fromObject(
      badge, get(op::PatchOp::PatchAfter).ensure_object()[id].ensure_object());
  prior.merge(after);
  prior.merge(p);
  get(op::PatchOp::PatchAfter).ensure_object().erase(id);
  get(op::PatchOp::PatchPrior).ensure_object()[id].as_object() =
      prior.toObject();
}
void DynamicUnknownPatch::apply(detail::Badge, Value& v) const {
  struct Visitor {
    void assign(detail::Badge, detail::Object obj) {
      detail::convertStringToBinary(obj);
      value.as_object() = std::move(obj);
    }
    void clear(detail::Badge) { value = emptyValue(value.getType()); }
    void removeMulti(detail::Badge, const detail::ValueSet& v) {
      if (auto p = value.if_set()) {
        for (auto k : v) {
          detail::convertStringToBinary(k);
          p->erase(k);
        }
      } else {
        for (auto k : v) {
          detail::convertStringToBinary(k);
          value.as_map().erase(k);
        }
      }
    }
    void patchIfSet(detail::Badge, FieldId id, const DynamicPatch& p) {
      if (auto field = value.as_object().if_contains(id)) {
        p.apply(*field);
      }
    }
    Value& value;
  };

  return customVisit(badge, Visitor{v});
}
void DynamicPatch::apply(Value& value) const {
  auto applier = folly::overload(
      [&](const op::BoolPatch& patch) { patch.apply(value.as_bool()); },
      [&](const op::BytePatch& patch) { patch.apply(value.as_byte()); },
      [&](const op::I16Patch& patch) { patch.apply(value.as_i16()); },
      [&](const op::I32Patch& patch) { patch.apply(value.as_i32()); },
      [&](const op::I64Patch& patch) { patch.apply(value.as_i64()); },
      [&](const op::FloatPatch& patch) { patch.apply(value.as_float()); },
      [&](const op::DoublePatch& patch) { patch.apply(value.as_double()); },
      [&](const op::BinaryPatch& patch) {
        if (value.is_binary()) {
          patch.apply(value.as_binary());
        } else {
          patch.apply(value.as_string());
        }
      },
      [&](const op::StringPatch& patch) {
        if (value.is_binary()) {
          auto s = std::move(value.as_binary()).to<std::string>();
          patch.apply(s);
          value.as_binary() = *folly::IOBuf::fromString(std::move(s));
        } else {
          patch.apply(value.as_string());
        }
      },
      [&](const DynamicListPatch& patch) {
        patch.apply(badge, value.as_list());
      },
      [&](const DynamicSetPatch& patch) { patch.apply(badge, value.as_set()); },
      [&](const DynamicMapPatch& patch) { patch.apply(badge, value.as_map()); },
      [&](const DynamicStructPatch& patch) {
        patch.apply(badge, value.as_object());
      },
      [&](const DynamicUnionPatch& patch) {
        patch.apply(badge, value.as_object());
      },
      [&](const DynamicUnknownPatch& patch) { patch.apply(badge, value); },
      [&](const op::AnyPatch& patch) {
        using Tag = type::struct_t<type::AnyStruct>;
        auto any = fromValueStruct<Tag>(std::move(value));
        patch.apply(any);
        value = asValueStruct<Tag>(std::move(any));
      });
  visitPatch(badge, applier);
}

void DynamicPatch::applyToDataFieldInsideAny(type::AnyStruct& any) const {
  // TODO(dokwon): Consider checking type on any directly for optimization.
  auto value = protocol::detail::parseValueFromAny(any);
  apply(value);
  any = detail::toAny(
            value,
            std::move(any.type().value()),
            std::move(any.protocol().value()))
            .toThrift();
}

Object DynamicPatch::toObject() && {
  return std::visit([&](auto&& v) { return std::move(v).toObject(); }, *patch_);
}
Object DynamicPatch::toObject() const& {
  return std::visit([&](auto&& v) { return v.toObject(); }, *patch_);
}
DynamicPatch::DynamicPatch() : patch_(std::make_unique<Patch>()) {}
DynamicPatch::DynamicPatch(const DynamicPatch& other) : DynamicPatch() {
  *patch_ = *other.patch_;
}
DynamicPatch& DynamicPatch::operator=(const DynamicPatch& other) {
  *patch_ = *other.patch_;
  return *this;
}
DynamicPatch::DynamicPatch(DynamicPatch&&) noexcept = default;
DynamicPatch& DynamicPatch::operator=(DynamicPatch&& other) noexcept = default;
DynamicPatch::~DynamicPatch() = default;

type::AnyStruct DynamicPatch::toAny(detail::Badge, type::Type type) const {
  type::AnyStruct any;
  any.protocol() = type::StandardProtocol::Compact;
  any.data() = *protocol::serializeObject<CompactProtocolWriter>(toObject());
  any.type() = protocol::detail::toPatchType(type);
  return any;
}

void DynamicPatch::fromAny(detail::Badge, const type::AnyStruct& any) {
  auto v = protocol::detail::parseValueFromAny(any);
  fromObject(badge, std::move(v.as_object()));
}

template <typename Protocol>
std::uint32_t DynamicPatch::encode(detail::Badge, Protocol& prot) const {
  return visitPatch(badge, [&](const auto& patch) {
    if constexpr (__FBTHRIFT_IS_VALID(patch, patch.encode(prot))) {
      return patch.encode(prot);
    } else {
      // TODO(dokwon): Provide direct encode from DynamicPatch.
      return protocol::detail::serializeObject(prot, toObject());
    }
  });
}

template <typename Protocol>
void DynamicPatch::decode(detail::Badge, Protocol& prot) {
  // TODO(dokwon): Provide direct decode to DynamicPatch.
  fromObject(badge, protocol::parseObject(prot));
}

template <typename Protocol>
std::unique_ptr<folly::IOBuf> DynamicPatch::encode(detail::Badge) const {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  Protocol prot;
  prot.setOutput(&queue);
  encode(badge, prot);
  return queue.move();
}

template <typename Protocol>
void DynamicPatch::decode(detail::Badge, const folly::IOBuf& buf) {
  Protocol prot;
  prot.setInput(&buf);
  decode(badge, prot);
}

template std::uint32_t DynamicPatch::encode(
    detail::Badge, apache::thrift::BinaryProtocolWriter&) const;
template std::uint32_t DynamicPatch::encode(
    detail::Badge, apache::thrift::CompactProtocolWriter&) const;
template std::unique_ptr<folly::IOBuf> DynamicPatch::encode<
    apache::thrift::BinaryProtocolWriter>(detail::Badge) const;
template std::unique_ptr<folly::IOBuf> DynamicPatch::encode<
    apache::thrift::CompactProtocolWriter>(detail::Badge) const;
template void DynamicPatch::decode(
    detail::Badge, apache::thrift::BinaryProtocolReader&);
template void DynamicPatch::decode(
    detail::Badge, apache::thrift::CompactProtocolReader&);
template void DynamicPatch::decode<apache::thrift::BinaryProtocolReader>(
    detail::Badge, const folly::IOBuf&);
template void DynamicPatch::decode<apache::thrift::CompactProtocolReader>(
    detail::Badge, const folly::IOBuf&);

template <typename Protocol>
std::uint32_t DynamicPatch::DynamicSafePatch::encode(Protocol& prot) const {
  uint32_t s = 0;
  s += prot.writeStructBegin("");
  s += prot.writeFieldBegin("version", TType::T_I32, 1);
  s += op::encode<type::i32_t>(prot, version_);
  s += prot.writeFieldEnd();
  s += prot.writeFieldBegin("data", TType::T_STRING, 2);
  s += op::encode<type::binary_t>(prot, data_);
  s += prot.writeFieldEnd();
  s += prot.writeFieldStop();
  s += prot.writeStructEnd();
  return s;
}

template <typename Protocol>
void DynamicPatch::DynamicSafePatch::decode(Protocol& prot) {
  std::int32_t version;
  std::unique_ptr<folly::IOBuf> data;

  std::string name;
  int16_t fid;
  TType ftype;
  prot.readStructBegin(name);
  while (true) {
    prot.readFieldBegin(name, ftype, fid);
    if (ftype == protocol::T_STOP) {
      break;
    }
    if (fid == 1 && ftype == TType::T_I32) {
      op::decode<type::i32_t>(prot, version);
    } else if (fid == 2 && ftype == TType::T_STRING) {
      op::decode<type::binary_t>(prot, data);
    } else {
      prot.skip(ftype);
    }
    prot.readFieldEnd();
  }
  prot.readStructEnd();
  *this = DynamicSafePatch{version, std::move(data)};
}

template std::uint32_t DynamicPatch::DynamicSafePatch::encode(
    apache::thrift::BinaryProtocolWriter&) const;
template std::uint32_t DynamicPatch::DynamicSafePatch::encode(
    apache::thrift::CompactProtocolWriter&) const;
template void DynamicPatch::DynamicSafePatch::decode(
    apache::thrift::BinaryProtocolReader&);
template void DynamicPatch::DynamicSafePatch::decode(
    apache::thrift::CompactProtocolReader&);

namespace {
class MinSafePatchVersionVisitor {
 public:
  // Shared
  template <typename T>
  void assign(const T&) {}
  template <typename T>
  void assign(detail::Badge, const T&) {}
  void clear() {}
  void clear(detail::Badge) {}
  void recurse(const DynamicPatch& patch) {
    // recurse visitPatch
    patch.visitPatch(
        badge,
        folly::overload(
            [&](const DynamicMapPatch& p) { p.customVisit(badge, *this); },
            [&](const DynamicStructPatch& p) { p.customVisit(*this); },
            [&](const DynamicUnionPatch& p) { p.customVisit(*this); },
            [&](const op::AnyPatch& p) {
              // recurse AnyPatch in case it only uses `assign` or `clear`
              // operations that are V1.
              p.customVisit(*this);
            },
            [&](const DynamicUnknownPatch& p) {
              // recurse DynamicUnknownPatch for `patchPrior/patchAfter` in
              // `StructuredOrAnyPatch`.
              p.customVisit(badge, *this);
            },
            [&](const auto&) {
              // Short circuit all other patch types.
            }));
  }

  // Map
  void putMulti(detail::Badge, const detail::ValueMap&) {}
  void tryPutMulti(detail::Badge, const detail::ValueMap&) {}
  void removeMulti(detail::Badge, const detail::ValueSet&) {}
  void patchByKey(detail::Badge, const Value&, const DynamicPatch& p) {
    recurse(p);
  }

  // Structured
  void ensure(FieldId, const Value&) {}
  void remove(FieldId) {}
  void patchIfSet(FieldId, const DynamicPatch& fieldPatch) {
    recurse(fieldPatch);
  }

  // Unknown
  void patchIfSet(detail::Badge, FieldId, const DynamicPatch& fieldPatch) {
    recurse(fieldPatch);
  }

  // Thrift Any
  template <typename... T>
  void patchIfTypeIs(T&&...) {
    version = std::max(version, 2);
  }
  void ensureAny(const type::AnyStruct&) { version = std::max(version, 2); }

  std::int32_t version = 1;
};
} // namespace

void DynamicPatch::fromSafePatch(const type::AnyStruct& any) {
  DynamicSafePatch safePatch;
  // TODO: Add SafePatch check based on Uri
  if (any.protocol() == type::StandardProtocol::Binary) {
    safePatch.decode<apache::thrift::BinaryProtocolReader>(*any.data());
  } else if (any.protocol() == type::StandardProtocol::Compact) {
    safePatch.decode<apache::thrift::CompactProtocolReader>(*any.data());
  } else {
    folly::throw_exception<std::runtime_error>(
        "Unsupported protocol when parsing SafePatch.");
  }
  if (safePatch.version() > op::detail::kThriftDynamicPatchVersion) {
    throw std::runtime_error(
        fmt::format("Unsupported patch version: {}", safePatch.version()));
  }
  decode<apache::thrift::CompactProtocolReader>(badge, *safePatch.data());
}
type::AnyStruct DynamicPatch::toSafePatch(type::Type type) const {
  // TODO: Add SafePatch check based on Uri
  MinSafePatchVersionVisitor visitor;
  visitor.recurse(*this);
  DynamicSafePatch safePatch{
      visitor.version, encode<CompactProtocolWriter>(badge)};

  type::AnyStruct anyStruct;
  anyStruct.type() = std::move(type);
  anyStruct.protocol() = type::StandardProtocol::Compact;
  anyStruct.data() = *safePatch.encode<CompactProtocolWriter>();
  return anyStruct;
}

} // namespace apache::thrift::protocol
