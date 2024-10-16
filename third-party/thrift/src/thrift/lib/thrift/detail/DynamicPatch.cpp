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

#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Patch.h>
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
        if constexpr (detail::has_empty_with_badge_v<
                          folly::remove_cvref_t<decltype(v)>>) {
          return v.empty(badge);
        } else {
          return v.empty();
        }
      },
      *patch_);
}

template <class SubPatch>
DynamicPatch& DynamicMapPatch::patchByKeyImpl(Value k, SubPatch&& p) {
  ensurePatchable();
  auto& patch = (add_.contains(k) || put_.contains(k) || remove_.contains(k))
      ? patchAfter_
      : patchPrior_;
  if (auto subPatch = get_ptr(patch, k)) {
    subPatch->merge(badge, std::forward<SubPatch>(p));
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
  put_.insert_or_assign(std::move(k), std::move(v));
}

void DynamicMapPatch::erase(detail::Badge, Value k) {
  undoChanges(k);
  remove_.insert(std::move(k));
}
void DynamicMapPatch::add(detail::Badge, detail::ValueMap map) {
  ensurePatchable();
  for (const auto& [k, v] : map) {
    if (put_.contains(k)) {
      // We already `put` the value, ignore `add`
      continue;
    } else if (remove_.erase(k)) {
      // We removed the `value`, we need to add it back as `put`
      patchAfter_.erase(k);
      put_[k] = v;
    } else {
      add_.try_emplace(k, v);
    }
  }
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
        p.apply(badge, *ptr);
      }
    }
    void remove(detail::Badge, const detail::ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.erase(i);
      }
    }
    void add(detail::Badge, const detail::ValueMap& map) {
      for (const auto& kv : map) {
        auto k = kv.first;
        auto v = kv.second;
        detail::convertStringToBinary(k);
        detail::convertStringToBinary(v);
        value.emplace(std::move(k), std::move(v));
      }
    }
    void put(detail::Badge, const detail::ValueMap& map) {
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
template <class SubPatch>
DynamicPatch& DynamicStructurePatch<IsUnion>::patchIfSetImpl(
    FieldId id, SubPatch&& p) {
  ensurePatchable();

  auto& patch = (ensure_.contains(id) ? patchAfter_ : patchPrior_);
  if (auto subPatch = folly::get_ptr(patch, id)) {
    subPatch->merge(badge, std::forward<SubPatch>(p));
    return *subPatch;
  } else {
    return patch.emplace(id, std::forward<SubPatch>(p)).first->second;
  }
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::ensureUnion(FieldId id, Value v) {
  ensurePatchable();

  if (ensure_.contains(id)) {
    return;
  }

  if (const bool removed = clear_ || !ensure_.empty()) {
    Object obj;
    obj[id] = std::move(v);
    assign(badge, std::move(obj));
    return;
  }

  DynamicPatch tmp = std::move(patchPrior_[id]);
  tmp.merge(badge, patchAfter_[id]);

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
    patchPrior_[id].merge(badge, patchAfter_[id]);
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
    void assign(detail::Badge, Object v) {
      detail::convertStringToBinary(v);
      obj = std::move(v);
    }
    void clear(detail::Badge) { obj.members()->clear(); }
    void patchIfSet(detail::Badge, FieldId id, const DynamicPatch& p) {
      if (obj.contains(id)) {
        p.apply(badge, obj[id]);
      }
    }
    void remove(detail::Badge, FieldId id) { obj.erase(id); }
    void ensure(detail::Badge, FieldId id, const Value& v) {
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

  return customVisit(badge, Visitor{obj});
}

template class DynamicStructurePatch<true>;
template class DynamicStructurePatch<false>;

DynamicPatch DiffVisitorBase::diff(const Object& src, const Object& dst) {
  return diffStructured(src, dst);
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
    // If same field is set, use normal Object diff logic.
    if (src.empty() || dst.empty() ||
        src.begin()->first != dst.begin()->first) {
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
      patch.clear(badge);
    }
    return patch;
  }

  if (src.empty() || src.begin()->first != dst.begin()->first) {
    patch.assign(badge, dst);
    return patch;
  }

  auto id = static_cast<FieldId>(src.begin()->first);
  pushField(id);
  auto guard = folly::makeGuard([&] { pop(); });
  auto subPatch = diff(badge, src.at(id), dst.at(id));
  if (!subPatch.empty(badge)) {
    patch.patchIfSet(badge, id, DynamicPatch{std::move(subPatch)});
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
    patch.remove(badge, id);
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
    patch.ensure(badge, id, std::move(empty));
    patch.patchIfSet(badge, id, DynamicPatch{std::move(subPatch)});
    return;
  }

  pushField(id);
  auto guard = folly::makeGuard([&] { pop(); });
  auto subPatch = diff(badge, src.at(id), dst.at(id));
  if (!subPatch.empty(badge)) {
    patch.patchIfSet(badge, id, DynamicPatch{std::move(subPatch)});
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
    void add(detail::Badge, const detail::ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.insert(std::move(i));
      }
    }
    void remove(detail::Badge, const detail::ValueSet& v) {
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
DynamicPatch::merge(detail::Badge, Other&& other) {
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
          return merge(badge, std::move(tmp));
        },
        *patch_);
  }
  if (std::holds_alternative<op::StringPatch>(*patch_) &&
      std::holds_alternative<op::BinaryPatch>(*other.patch_)) {
    *patch_ = detail::createPatchFromObject<op::BinaryPatch>(
        badge, std::move(*this).toObject());
    return merge(badge, std::forward<Other>(other));
  }
  if (std::holds_alternative<op::BinaryPatch>(*patch_) &&
      std::holds_alternative<op::StringPatch>(*other.patch_)) {
    *patch_ = detail::createPatchFromObject<op::StringPatch>(
        badge, std::move(*this).toObject());
    return merge(badge, std::forward<Other>(other));
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
          if constexpr (detail::has_merge_with_badge_v<L>) {
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

template void DynamicPatch::merge(detail::Badge, DynamicPatch&);
template void DynamicPatch::merge(detail::Badge, DynamicPatch&&);
template void DynamicPatch::merge(detail::Badge, const DynamicPatch&);
template void DynamicPatch::merge(detail::Badge, const DynamicPatch&&);

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
    return Category::StructuredPatch;
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
    return Category::StructuredPatch;
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
void DynamicUnknownPatch::remove(detail::Badge, Value v) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::AssociativeContainerPatch})) {
    throwIncompatibleCategory("remove");
  }

  get(op::PatchOp::Remove).ensure_set().insert(std::move(v));
}
void DynamicUnknownPatch::assign(detail::Badge, Object v) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::StructuredPatch})) {
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
           Category::StructuredPatch})) {
    throwIncompatibleCategory("patchIfSet");
  }
  if (auto assign = get_ptr(op::PatchOp::Assign)) {
    if (assign->as_object().contains(id)) {
      p.apply(badge, assign->as_object().at(id));
    }
    return;
  }

  // TODO: optimize this
  DynamicPatch prior, after;
  prior.fromObject(
      badge, get(op::PatchOp::PatchPrior).ensure_object()[id].ensure_object());
  after.fromObject(
      badge, get(op::PatchOp::PatchAfter).ensure_object()[id].ensure_object());
  prior.merge(badge, after);
  prior.merge(badge, p);
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
    void remove(detail::Badge, detail::Value v) {
      detail::convertStringToBinary(v);
      if (auto p = value.if_set()) {
        p->erase(v);
      } else {
        value.as_map().erase(v);
      }
    }
    void patchIfSet(detail::Badge, FieldId id, const DynamicPatch& p) {
      if (auto field = value.as_object().if_contains(id)) {
        p.apply(badge, *field);
      }
    }
    Value& value;
  };

  return customVisit(badge, Visitor{v});
}

void DynamicPatch::apply(detail::Badge, Value& v) const {
  if (holds_alternative<DynamicUnknownPatch>(badge)) {
    return std::get<DynamicUnknownPatch>(*patch_).apply(badge, v);
  }
  switch (v.getType()) {
    case Value::Type::boolValue:
      return std::get<op::BoolPatch>(*patch_).apply(v.as_bool());
    case Value::Type::byteValue:
      return std::get<op::BytePatch>(*patch_).apply(v.as_byte());
    case Value::Type::i16Value:
      return std::get<op::I16Patch>(*patch_).apply(v.as_i16());
    case Value::Type::i32Value:
      return std::get<op::I32Patch>(*patch_).apply(v.as_i32());
    case Value::Type::i64Value:
      return std::get<op::I64Patch>(*patch_).apply(v.as_i64());
    case Value::Type::floatValue:
      return std::get<op::FloatPatch>(*patch_).apply(v.as_float());
    case Value::Type::doubleValue:
      return std::get<op::DoublePatch>(*patch_).apply(v.as_double());
    case Value::Type::stringValue: {
      if (holds_alternative<op::BinaryPatch>(badge)) {
        return detail::createPatchFromObject<op::StringPatch>(badge, toObject())
            .apply(v.as_string());
      }
      return std::get<op::StringPatch>(*patch_).apply(v.as_string());
    }
    case Value::Type::binaryValue: {
      if (holds_alternative<op::StringPatch>(badge)) {
        return detail::createPatchFromObject<op::BinaryPatch>(badge, toObject())
            .apply(v.as_binary());
      }
      return std::get<op::BinaryPatch>(*patch_).apply(v.as_binary());
    }
    case Value::Type::listValue:
      return std::get<DynamicListPatch>(*patch_).apply(badge, v.as_list());
    case Value::Type::setValue:
      return std::get<DynamicSetPatch>(*patch_).apply(badge, v.as_set());
    case Value::Type::mapValue:
      return std::get<DynamicMapPatch>(*patch_).apply(badge, v.as_map());
    case Value::Type::objectValue:
      if (holds_alternative<op::AnyPatch>(badge)) {
        using Tag = type::struct_t<type::AnyStruct>;
        auto any = fromValueStruct<Tag>(v);
        std::get<op::AnyPatch>(*patch_).apply(any);
        v = asValueStruct<Tag>(any);
        return;
      }
      if (holds_alternative<DynamicStructPatch>(badge)) {
        return std::get<DynamicStructPatch>(*patch_).apply(
            badge, v.as_object());
      }
      return std::get<DynamicUnionPatch>(*patch_).apply(badge, v.as_object());
    case Value::Type::__EMPTY__:
      folly::throw_exception<std::runtime_error>("value is empty.");
    default:
      notImpl();
  }
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
} // namespace apache::thrift::protocol
