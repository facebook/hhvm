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
#include <functional>
#include <optional>

#include <folly/Overload.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>
#include <thrift/lib/cpp2/protocol/detail/Patch.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/detail/DynamicPatch.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_types.h>

#include <thrift/lib/cpp2/patch/detail/PatchBadge.h>

namespace apache::thrift::protocol {
using detail::badge;
using thrift::detail::DynamicCursorSerializationWrapper;
using thrift::detail::StructuredDynamicCursorReader;
using thrift::detail::StructuredDynamicCursorWriter;

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
  op::invoke_by_field_id<Value::ThriftValue>(
      static_cast<FieldId>(type),
      [&](auto id) { op::get<decltype(id)>(value.toThrift()).emplace(); },
      [] { folly::throw_exception<std::runtime_error>("Not Implemented."); });
  return value;
}

template <type::StandardProtocol Protocol, class Patch>
std::unique_ptr<folly::IOBuf> applyToSerializedObjectImpl(
    const Patch& patch, std::unique_ptr<folly::IOBuf> buf) {
  using Reader = ProtocolReaderFor<Protocol>;
  using Writer = ProtocolWriterFor<Protocol>;
  DynamicCursorSerializationWrapper<Reader, Writer> inWrapper(std::move(buf));
  DynamicCursorSerializationWrapper<Reader, Writer> outWrapper;
  auto reader = inWrapper.beginRead();
  auto writer = outWrapper.beginWrite();
  patch.applyAllFieldsInStream(badge, reader, writer);
  inWrapper.endRead(std::move(reader));
  outWrapper.endWrite(std::move(writer));
  return std::move(outWrapper).serializedData();
}
} // namespace

namespace detail {

constexpr std::string_view kPatchUriSuffix = "Patch";
constexpr std::string_view kSafePatchUriSuffix = "SafePatch";

Value emptyValueFromTType(TType type) {
  switch (type) {
    case TType::T_BOOL:
      return emptyValue(Value::Type::boolValue);
    case TType::T_BYTE:
      return emptyValue(Value::Type::byteValue);
    case TType::T_I16:
      return emptyValue(Value::Type::i16Value);
    case TType::T_I32:
      return emptyValue(Value::Type::i32Value);
    case TType::T_I64:
      return emptyValue(Value::Type::i64Value);
    case TType::T_FLOAT:
      return emptyValue(Value::Type::floatValue);
    case TType::T_DOUBLE:
      return emptyValue(Value::Type::doubleValue);
    case TType::T_STRING:
      return emptyValue(Value::Type::binaryValue);
    case TType::T_LIST:
      return emptyValue(Value::Type::listValue);
    case TType::T_SET:
      return emptyValue(Value::Type::setValue);
    case TType::T_MAP:
      return emptyValue(Value::Type::mapValue);
    case TType::T_STRUCT:
      return emptyValue(Value::Type::objectValue);
    default:
      folly::throw_exception<std::runtime_error>("Not Implemented.");
  }
}

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

void checkSameType(
    const apache::thrift::protocol::Value& v1,
    const apache::thrift::protocol::Value& v2) {
  if (v1.getType() != v2.getType()) {
    throw std::runtime_error(
        fmt::format(
            "Value type does not match: ({}) v.s. ({})",
            apache::thrift::util::enumNameSafe(v1.getType()),
            apache::thrift::util::enumNameSafe(v2.getType())));
  }
}
void checkCompatibleType(
    const ValueList& l, const apache::thrift::protocol::Value& v) {
  if (!l.empty()) {
    if (l.back().getType() != v.getType()) {
      throw std::runtime_error(
          fmt::format(
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
      throw std::runtime_error(
          fmt::format(
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

std::string toSafePatchUri(std::string s) {
  if (folly::StringPiece(s).endsWith(kPatchUriSuffix)) {
    folly::throw_exception<std::runtime_error>(
        fmt::format("Uri {} is already Patch.", s));
  }
  s += kSafePatchUriSuffix;
  return s;
}

std::string fromSafePatchUri(std::string s) {
  auto newSize = s.size() - kSafePatchUriSuffix.size();
  if (s.size() <= kSafePatchUriSuffix.size() ||
      s.substr(newSize) != kSafePatchUriSuffix) {
    folly::throw_exception<std::invalid_argument>(
        fmt::format("Uri {} is not a SafePatch.", s));
  }
  s.resize(newSize);
  return s;
}

} // namespace detail

/// @cond
std::string toPatchUri(std::string s) {
  if (folly::StringPiece(s).endsWith(detail::kPatchUriSuffix)) {
    folly::throw_exception<std::runtime_error>(
        fmt::format("Uri {} is already Patch.", s));
  }
  s += detail::kPatchUriSuffix;
  return s;
}
/// @endcond

/// @cond
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
/// @endcond

namespace {
// Helper function to handle URI transformation for both Patch and SafePatch
type::Type transformPatchTypeUri(
    type::Type input,
    std::function<std::string(std::string)> uriTransformer,
    bool fromPatchType,
    bool isUnion) {
  auto& name = input.toThrift().name().value();
  auto handleUri = [&](auto& type) {
    if (auto p = type.uri_ref()) {
      *p = uriTransformer(*p);
      return std::move(input);
    }
    folly::throw_exception<std::runtime_error>(fmt::format(
        "Unsupported Uri: {}",
        apache::thrift::util::enumNameSafe(type.getType())));
  };

  if (!isUnion) {
    if (auto structType = name.structType()) {
      return handleUri(*structType);
    } else if (auto unionType = name.unionType();
               unionType.has_value() && !fromPatchType) {
      // All Thrift SafePatch/Patch are struct type.
      auto temp = std::move(*unionType);
      name.structType() = std::move(temp);
      return handleUri(*name.structType());
    }
  } else {
    if (auto structType = name.structType()) {
      auto temp = std::move(*structType);
      name.unionType() = std::move(temp);
      return handleUri(*name.unionType());
    }
  }

  folly::throw_exception<std::runtime_error>(fmt::format(
      "Unsupported type: {}",
      apache::thrift::util::enumNameSafe(name.getType())));
}
} // namespace

type::Type toSafePatchType(type::Type input) {
  return transformPatchTypeUri(
      std::move(input), detail::toSafePatchUri, false, false);
}
type::Type fromSafePatchType(type::Type input, bool isUnion) {
  return transformPatchTypeUri(
      std::move(input), detail::fromSafePatchUri, true, isUnion);
}

type::Type toPatchType(type::Type input) {
  return transformPatchTypeUri(std::move(input), toPatchUri, false, false);
}
type::Type fromPatchType(type::Type input, bool isUnion) {
  return transformPatchTypeUri(std::move(input), fromPatchUri, true, isUnion);
}

DynamicListPatch& DynamicPatch::getStoredPatchByTag(type::list_c) {
  return getStoredPatch<DynamicListPatch>();
}
DynamicSetPatch& DynamicPatch::getStoredPatchByTag(type::set_c) {
  return getStoredPatch<DynamicSetPatch>();
}
DynamicMapPatch& DynamicPatch::getStoredPatchByTag(type::map_c) {
  return getStoredPatch<DynamicMapPatch>();
}
DynamicStructPatch& DynamicPatch::getStoredPatchByTag(type::struct_c) {
  return getStoredPatch<DynamicStructPatch>();
}
DynamicUnionPatch& DynamicPatch::getStoredPatchByTag(type::union_c) {
  return getStoredPatch<DynamicUnionPatch>();
}
op::I32Patch& DynamicPatch::getStoredPatchByTag(type::enum_c) {
  return getStoredPatch<op::I32Patch>();
}
op::AnyPatch& DynamicPatch::getStoredPatchByTag(
    type::struct_t<type::AnyStruct>) {
  return getStoredPatch<op::AnyPatch>();
}

bool DynamicPatch::isPatchTypeAmbiguous() const {
  return holds_alternative<DynamicUnknownPatch>(badge);
}

bool DynamicPatch::empty() const {
  return std::visit(
      [&](auto&& v) {
        if constexpr (requires { v.empty(badge); }) {
          return v.empty(badge);
        } else {
          return v.empty();
        }
      },
      *patch_);
}

DynamicPatch& DynamicMapPatch::patchByKey(Value&& k) {
  return patchByKey(std::move(k), {});
}
DynamicPatch& DynamicMapPatch::patchByKey(const Value& k) {
  return patchByKey(k, {});
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

template DynamicPatch& DynamicMapPatch::patchByKeyImpl(
    Value k, const DynamicPatch& p);

template DynamicPatch& DynamicMapPatch::patchByKeyImpl(
    Value k, DynamicPatch&& p);

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
      patchPrior_[k] = DynamicPatch::fromObject(std::move(v.as_object()));
    }
  }

  if (auto patchAfter = get_ptr(obj, op::PatchOp::PatchAfter)) {
    for (auto& [k, v] : patchAfter->as_map()) {
      patchAfter_[k] = DynamicPatch::fromObject(std::move(v.as_object()));
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

namespace {

template <typename Protocol, typename PatchMap>
std::uint32_t encodePatchMap(Protocol& prot, const PatchMap& patches) {
  TType keyType = protocol::T_STRING;
  if (!patches.empty()) {
    keyType = detail::getTType(patches.begin()->first);
  }
  std::uint32_t ret =
      prot.writeMapBegin(keyType, TType::T_STRUCT, patches.size());
  for (const auto& [key, patch] : patches) {
    ret += serializeValue(prot, key);
    ret += patch.encode(prot);
  }
  ret += prot.writeMapEnd();
  return ret;
}

template <typename Protocol, typename FieldPatchMap>
std::uint32_t encodeFieldPatchStruct(
    Protocol& prot, const char* name, const FieldPatchMap& patches) {
  std::uint32_t ret = prot.writeStructBegin(name);
  for (const auto& [id, patch] : patches) {
    ret += prot.writeFieldBegin("", TType::T_STRUCT, folly::to_underlying(id));
    ret += patch.encode(prot);
    ret += prot.writeFieldEnd();
  }
  ret += prot.writeFieldStop();
  ret += prot.writeStructEnd();
  return ret;
}

} // namespace

template <typename Protocol>
std::uint32_t DynamicMapPatch::encode(Protocol& prot) const {
  std::uint32_t ret = prot.writeStructBegin("DynamicMapPatch");

  if (assign_) {
    ret += prot.writeFieldBegin(
        "assign", TType::T_MAP, folly::to_underlying(op::PatchOp::Assign));
    ret += detail::serializeMap(prot, *assign_);
    ret += prot.writeFieldEnd();
    ret += prot.writeFieldStop();
    ret += prot.writeStructEnd();
    return ret;
  }

  if (clear_) {
    ret += prot.writeFieldBegin(
        "clear", TType::T_BOOL, folly::to_underlying(op::PatchOp::Clear));
    ret += prot.writeBool(true);
    ret += prot.writeFieldEnd();
  }

  if (!patchPrior_.empty()) {
    ret += prot.writeFieldBegin(
        "patchPrior",
        TType::T_MAP,
        folly::to_underlying(op::PatchOp::PatchPrior));
    ret += encodePatchMap(prot, patchPrior_);
    ret += prot.writeFieldEnd();
  }

  if (!patchAfter_.empty()) {
    ret += prot.writeFieldBegin(
        "patch", TType::T_MAP, folly::to_underlying(op::PatchOp::PatchAfter));
    ret += encodePatchMap(prot, patchAfter_);
    ret += prot.writeFieldEnd();
  }

  if (!add_.empty()) {
    ret += prot.writeFieldBegin(
        "ensure",
        TType::T_MAP,
        folly::to_underlying(op::PatchOp::EnsureStruct));
    ret += serializeMap(prot, add_);
    ret += prot.writeFieldEnd();
  }

  if (!remove_.empty()) {
    ret += prot.writeFieldBegin(
        "remove", TType::T_SET, folly::to_underlying(op::PatchOp::Remove));
    ret += serializeSet(prot, remove_);
    ret += prot.writeFieldEnd();
  }

  if (!put_.empty()) {
    ret += prot.writeFieldBegin(
        "put", TType::T_MAP, folly::to_underlying(op::PatchOp::Put));
    ret += serializeMap(prot, put_);
    ret += prot.writeFieldEnd();
  }

  ret += prot.writeFieldStop();
  ret += prot.writeStructEnd();
  return ret;
}

template std::uint32_t DynamicMapPatch::encode(
    apache::thrift::BinaryProtocolWriter&) const;
template std::uint32_t DynamicMapPatch::encode(
    apache::thrift::CompactProtocolWriter&) const;
template std::uint32_t DynamicMapPatch::encode(
    apache::thrift::protocol::detail::ObjectWriter&) const;

template <bool IsUnion>
template <class Protocol>
std::uint32_t DynamicStructurePatch<IsUnion>::encode(Protocol& prot) const {
  std::uint32_t ret = prot.writeStructBegin(
      IsUnion ? "DynamicUnionPatch" : "DynamicStructPatch");

  if (assign_) {
    ret += prot.writeFieldBegin(
        "assign", TType::T_STRUCT, folly::to_underlying(op::PatchOp::Assign));
    ret += detail::serializeObject(prot, *assign_);
    ret += prot.writeFieldEnd();
    ret += prot.writeFieldStop();
    ret += prot.writeStructEnd();
    return ret;
  }

  if (clear_) {
    ret += prot.writeFieldBegin(
        "clear", TType::T_BOOL, folly::to_underlying(op::PatchOp::Clear));
    ret += prot.writeBool(true);
    ret += prot.writeFieldEnd();
  }

  if (!remove_.empty()) {
    ret += prot.writeFieldBegin(
        "remove", TType::T_LIST, folly::to_underlying(op::PatchOp::Remove));
    ret += prot.writeListBegin(TType::T_I16, remove_.size());
    for (auto id : remove_) {
      ret += prot.writeI16(folly::to_underlying(id));
    }
    ret += prot.writeListEnd();
    ret += prot.writeFieldEnd();
  }

  if (!ensure_.empty()) {
    constexpr auto ensureOp =
        IsUnion ? op::PatchOp::EnsureUnion : op::PatchOp::EnsureStruct;
    ret += prot.writeFieldBegin(
        "ensure", TType::T_STRUCT, folly::to_underlying(ensureOp));
    ret += prot.writeStructBegin("ensure");
    for (const auto& [id, value] : ensure_) {
      ret += prot.writeFieldBegin(
          "", detail::getTType(value), folly::to_underlying(id));
      ret += detail::serializeValue(prot, value);
      ret += prot.writeFieldEnd();
    }
    ret += prot.writeFieldStop();
    ret += prot.writeStructEnd();
    ret += prot.writeFieldEnd();
  }

  if (!patchPrior_.empty()) {
    ret += prot.writeFieldBegin(
        "patchPrior",
        TType::T_STRUCT,
        folly::to_underlying(op::PatchOp::PatchPrior));
    ret += encodeFieldPatchStruct(prot, "patchPrior", patchPrior_);
    ret += prot.writeFieldEnd();
  }

  if (!patchAfter_.empty()) {
    ret += prot.writeFieldBegin(
        "patch",
        TType::T_STRUCT,
        folly::to_underlying(op::PatchOp::PatchAfter));
    ret += encodeFieldPatchStruct(prot, "patch", patchAfter_);
    ret += prot.writeFieldEnd();
  }

  ret += prot.writeFieldStop();
  ret += prot.writeStructEnd();
  return ret;
}

template std::uint32_t DynamicStructPatch::encode(
    apache::thrift::BinaryProtocolWriter&) const;
template std::uint32_t DynamicStructPatch::encode(
    apache::thrift::CompactProtocolWriter&) const;
template std::uint32_t DynamicStructPatch::encode(
    apache::thrift::protocol::detail::ObjectWriter&) const;
template std::uint32_t DynamicUnionPatch::encode(
    apache::thrift::BinaryProtocolWriter&) const;
template std::uint32_t DynamicUnionPatch::encode(
    apache::thrift::CompactProtocolWriter&) const;
template std::uint32_t DynamicUnionPatch::encode(
    apache::thrift::protocol::detail::ObjectWriter&) const;

void DynamicMapPatch::insert_or_assign(Value k, Value v) {
  undoChanges(k);
  setOrCheckMapType(k, v);
  put_.insert_or_assign(std::move(k), std::move(v));
}

void DynamicMapPatch::erase(Value k) {
  undoChanges(k);
  remove_.insert(std::move(k));
}
void DynamicMapPatch::tryPutMulti(ValueMap map) {
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

void DynamicMapPatch::apply(detail::Badge, ValueMap& v) const {
  struct Visitor {
    void assign(ValueMap v) {
      detail::convertStringToBinary(v);
      value = std::move(v);
    }
    void clear() { value.clear(); }
    void patchByKey(Value key, const DynamicPatch& p) {
      detail::convertStringToBinary(key);
      if (auto ptr = folly::get_ptr(value, key)) {
        p.apply(*ptr);
      }
    }
    void removeMulti(const ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.erase(i);
      }
    }
    void tryPutMulti(const ValueMap& map) {
      for (const auto& kv : map) {
        auto k = kv.first;
        auto v = kv.second;
        detail::convertStringToBinary(k);
        detail::convertStringToBinary(v);
        value.emplace(std::move(k), std::move(v));
      }
    }
    void putMulti(const ValueMap& map) {
      for (const auto& kv : map) {
        auto k = kv.first;
        auto v = kv.second;
        detail::convertStringToBinary(k);
        detail::convertStringToBinary(v);
        value.insert_or_assign(std::move(k), std::move(v));
      }
    }
    ValueMap& value;
  };

  return customVisit(Visitor{v});
}

void DynamicMapPatch::setOrCheckMapType(
    const protocol::Value& k, const protocol::Value& v) {
  if (keyType_) {
    if (*keyType_ != k.getType()) {
      throw std::runtime_error(
          fmt::format(
              "Type of value ({}) does not match key type of map ({}) in patch.",
              apache::thrift::util::enumNameSafe(k.getType()),
              apache::thrift::util::enumNameSafe(*keyType_)));
    }
  } else {
    keyType_ = k.getType();
  }
  if (valueType_) {
    if (*valueType_ != v.getType()) {
      throw std::runtime_error(
          fmt::format(
              "Type of value ({}) does not match value type of map ({}) in patch.",
              apache::thrift::util::enumNameSafe(v.getType()),
              apache::thrift::util::enumNameSafe(*valueType_)));
    }
  } else {
    valueType_ = v.getType();
  }
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::ensureAndAssignFieldsFromObject(
    Object obj) {
  for (auto&& [id, field] : obj) {
    auto fieldId = static_cast<FieldId>(id);
    ensure_[fieldId] = emptyValue(field.getType());
    Object patch;
    patch[static_cast<FieldId>(op::PatchOp::Assign)] = std::move(field);
    patchAfter_[fieldId] = DynamicPatch::fromObject(std::move(patch));
  }
}

template <bool IsUnion>
void DynamicStructurePatch<IsUnion>::ensurePatchable() {
  if (!assign_) {
    return;
  }

  clear_ = true;
  ensureAndAssignFieldsFromObject(std::move(*assign_));
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
    patchAfter_[id] = DynamicPatch::fromObject(patch);
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
      patchPrior_[static_cast<FieldId>(k)] =
          DynamicPatch::fromObject(std::move(v.as_object()));
    }
  }

  if (auto patchAfter = get_ptr(obj, op::PatchOp::PatchAfter)) {
    for (auto& [k, v] : patchAfter->as_object()) {
      patchAfter_[static_cast<FieldId>(k)] =
          DynamicPatch::fromObject(std::move(v.as_object()));
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

template <bool IsUnion>
template <type::StandardProtocol Protocol>
std::unique_ptr<folly::IOBuf>
DynamicStructurePatch<IsUnion>::applyToSerializedObject(
    std::unique_ptr<folly::IOBuf> buf) const {
  return applyToSerializedObjectImpl<Protocol>(*this, std::move(buf));
}

template std::unique_ptr<folly::IOBuf>
    DynamicStructurePatch<false>::applyToSerializedObject<
        type::StandardProtocol::Compact>(std::unique_ptr<folly::IOBuf>) const;
template std::unique_ptr<folly::IOBuf>
    DynamicStructurePatch<false>::applyToSerializedObject<
        type::StandardProtocol::Binary>(std::unique_ptr<folly::IOBuf>) const;
template std::unique_ptr<folly::IOBuf>
    DynamicStructurePatch<true>::applyToSerializedObject<
        type::StandardProtocol::Compact>(std::unique_ptr<folly::IOBuf>) const;
template std::unique_ptr<folly::IOBuf>
    DynamicStructurePatch<true>::applyToSerializedObject<
        type::StandardProtocol::Binary>(std::unique_ptr<folly::IOBuf>) const;

template <type::StandardProtocol Protocol>
std::unique_ptr<folly::IOBuf> DynamicUnknownPatch::applyToSerializedObject(
    std::unique_ptr<folly::IOBuf> buf) const {
  return applyToSerializedObjectImpl<Protocol>(*this, std::move(buf));
}

template std::unique_ptr<folly::IOBuf>
    DynamicUnknownPatch::applyToSerializedObject<
        type::StandardProtocol::Compact>(std::unique_ptr<folly::IOBuf>) const;
template std::unique_ptr<folly::IOBuf>
    DynamicUnknownPatch::applyToSerializedObject<
        type::StandardProtocol::Binary>(std::unique_ptr<folly::IOBuf>) const;

template <typename Protocol>
std::uint32_t DynamicPatchBase::encode(Protocol& prot) const {
  return protocol::detail::serializeObject(prot, patch_);
}

template std::uint32_t DynamicPatchBase::encode(
    apache::thrift::BinaryProtocolWriter&) const;
template std::uint32_t DynamicPatchBase::encode(
    apache::thrift::CompactProtocolWriter&) const;
template std::uint32_t DynamicPatchBase::encode(
    apache::thrift::protocol::detail::ObjectWriter&) const;

DynamicPatch DiffVisitorBase::diff(const Object& src, const Object& dst) {
  return diffStructured(src, dst);
}

DynamicPatch DiffVisitorBase::diff(const Value& src, const Value& dst) {
  return diff(badge, src, dst);
}

DynamicPatch DiffVisitorBase::diff(
    detail::Badge, const Value& src, const Value& dst) {
  return diffValue(src, dst);
}

DynamicPatch DiffVisitorBase::diffValue(const Value& src, const Value& dst) {
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
    patch.clear();
    return patch;
  }

  auto it = std::search(dst.begin(), dst.end(), src.begin(), src.end());
  if (it != dst.begin()) {
    patch.assign(dst);
    return patch;
  }

  for (size_t i = src.size(); i < dst.size(); ++i) {
    patch.push_back(dst[i]);
  }
  return patch;
}

DynamicSetPatch DiffVisitorBase::diffSet(
    const ValueSet& src, const ValueSet& dst) {
  DynamicSetPatch patch;
  patch.doNotConvertStringToBinary(badge);

  if (dst.empty()) {
    patch.clear();
    return patch;
  }

  // remove keys
  for (const auto& i : src) {
    if (!dst.contains(i)) {
      patch.erase(i);
    }
  }
  if (dst.size() < patch.patchedElementCount(badge)) {
    patch.assign(dst);
    return patch;
  }

  // add keys
  for (const auto& i : dst) {
    if (!src.contains(i)) {
      patch.insert(i);
    }
  }
  if (dst.size() < patch.patchedElementCount(badge)) {
    patch.assign(dst);
    return patch;
  }

  return patch;
}

DynamicMapPatch DiffVisitorBase::diffMap(
    const ValueMap& src, const ValueMap& dst) {
  DynamicMapPatch patch;
  patch.doNotConvertStringToBinary(badge);

  if (dst.empty()) {
    patch.clear();
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
    patch.assign(dst);
    return patch;
  }

  for (const auto& k : keys) {
    diffElement(src, dst, k, patch);
  }
  return patch;
}

/// @cond
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
/// @endcond

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
    patch.erase(key);
    return;
  }

  if (!inSrc && inDst) {
    patch.insert_or_assign(key, dst.at(key));
    return;
  }

  Object elementPatch;
  pushKey(key);
  auto guard = folly::makeGuard([&] { pop(); });
  auto subPatch = diff(badge, src.at(key), dst.at(key));
  patch.patchByKey(key, DynamicPatch{std::move(subPatch)});
}

DynamicPatch DiffVisitorBase::diffStructured(
    const Object& src, const Object& dst) {
  if (src.empty() && dst.empty()) {
    return {};
  }

  // If src and dst looks like a thrift.Any, only use assign operator since
  // we can't tell whether we should use AnyPatch or StructPatch.
  if (maybeAny(src) && maybeAny(dst)) {
    DynamicUnknownPatch patch;
    if (src != dst) {
      patch.doNotConvertStringToBinary(badge);
      patch.assign(dst);
    }
    return DynamicPatch{std::move(patch)};
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
      patch.assign(dst);
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
  if (!subPatch.empty()) {
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
  if (!subPatch.empty()) {
    type::AnyStruct anySubPatch;
    anySubPatch.protocol() = type::StandardProtocol::Compact;
    anySubPatch.data() =
        *serializeObject<CompactProtocolWriter>(subPatch.toObject());
    anySubPatch.type() = toPatchType(*src.type());
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
  if (!subPatch.empty()) {
    if (maybeEmptyDeprecatedTerseField(src.at(id))) {
      patch.ensure(id, emptyValue(src.at(id).getType()));
    }
    patch.patchIfSet(id).merge(DynamicPatch{std::move(subPatch)});
  }
}

void DiffVisitorBase::pushField(FieldId id) {
  auto last = maskInPath_.top();
  auto next = &last->includes().ensure()[folly::to_underlying(id)];
  *next = allMask();
  maskInPath_.push(next);
}

void DiffVisitorBase::pushType(type::Type type) {
  auto last = maskInPath_.top();
  auto next = &last->includes_type().ensure()[std::move(type)];
  *next = allMask();
  maskInPath_.push(next);
}

/// @cond
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
/// @endcond

DynamicPatch DiffVisitorBase::createDynamicUnknownPatchWithAssign(
    const Object& obj) {
  DynamicUnknownPatch patch;
  patch.doNotConvertStringToBinary(badge);
  patch.assign(obj);
  return DynamicPatch{std::move(patch)};
}

void DiffVisitorBase::pushKey(const Value& k) {
  auto last = maskInPath_.top();
  Mask* next = nullptr;
  if (auto i = getIntegral(k)) {
    next = &last->includes_map().ensure()[*i];
  } else if (k.is_binary()) {
    next = &last->includes_string_map().ensure()[k.as_binary().toString()];
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

ExtractedMasksFromPatch DynamicListPatch::extractMaskFromPatch() const {
  struct Visitor {
    void assign(const ValueList&) {
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void clear() { masks = {protocol::noneMask(), protocol::allMask()}; }
    void push_back(const Value&) {
      // TODO(dokwon): Consider optimizing by not using customVisit.
      masks = {protocol::allMask(), protocol::allMask()};
    }

    protocol::ExtractedMasksFromPatch masks{
        protocol::noneMask(), protocol::noneMask()};
  };
  Visitor v;
  customVisit(v);
  return std::move(v.masks);
}

ExtractedMasksFromPatch DynamicSetPatch::extractMaskFromPatch() const {
  struct Visitor {
    void assign(const ValueSet&) {
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void clear() { masks = {protocol::noneMask(), protocol::allMask()}; }
    void addMulti(const ValueSet& v) {
      if (v.empty()) {
        return;
      }
      masks = {protocol::allMask(), protocol::allMask()};
    }
    void removeMulti(const ValueSet& v) {
      if (v.empty()) {
        return;
      }
      masks = {protocol::allMask(), protocol::allMask()};
    }

    protocol::ExtractedMasksFromPatch masks{
        protocol::noneMask(), protocol::noneMask()};
  };
  Visitor v;
  customVisit(v);
  return std::move(v.masks);
}

ExtractedMasksFromPatch DynamicMapPatch::extractMaskFromPatch() const {
  struct Visitor {
    void assign(const ValueMap&) {
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void clear() { masks = {protocol::noneMask(), protocol::allMask()}; }
    void patchByKey(const Value& key, const DynamicPatch& patch) {
      // granular read/write operation
      // Insert next extracted mask and insert key to read mask if the next
      // extracted mask does not include the key.
      auto getIncludesMapRef = [](Mask& mask) { return mask.includes_map(); };
      auto getIncludesStringMapRef = [](Mask& mask) {
        return mask.includes_string_map();
      };
      if (detail::getArrayKeyFromValue(key) == detail::ArrayKey::Integer) {
        auto id = static_cast<int64_t>(getMapIdFromValue(key));
        auto nextMasks = patch.extractMaskFromPatch();
        detail::insertNextMask(masks, nextMasks, id, id, getIncludesMapRef);
        detail::insertKeysToMask(masks.read, id);
      } else {
        auto id = getStringFromValue(key);
        auto nextMasks = patch.extractMaskFromPatch();
        detail::insertNextMask(
            masks, nextMasks, id, id, getIncludesStringMapRef);
        detail::insertKeysToMask(masks.read, id);
      }
    }
    void removeMulti(const ValueSet& set) {
      // write operation if not empty
      if (set.empty()) {
        return;
      }
      detail::insertKeysToMaskIfNotAllMask(masks.write, set);
    }
    void tryPutMulti(const ValueMap& map) {
      // granular read/write operation if not empty
      if (map.empty()) {
        return;
      }
      ensure_ = map;
    }
    void putMulti(const ValueMap& map) {
      // write operation if not empty
      if (map.empty()) {
        return;
      }
      detail::insertKeysToMaskIfNotAllMask(masks.write, map);
    }
    void finalize() {
      if (ensure_) {
        detail::insertKeysToMask(masks.read, ensure_->get());
        detail::insertKeysToMaskIfNotAllMask(masks.write, ensure_->get());
      }
      detail::ensureRWMaskInvariant(masks);
    }

    protocol::ExtractedMasksFromPatch masks{
        protocol::noneMask(), protocol::noneMask()};
    std::optional<std::reference_wrapper<const ValueMap>> ensure_;
  };
  Visitor v;
  customVisit(v);
  v.finalize();
  return std::move(v.masks);
}

ExtractedMasksFromPatch DynamicStructPatch::extractMaskFromPatch() const {
  struct Visitor {
    void assign(const Object&) {
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void clear() { masks = {protocol::noneMask(), protocol::allMask()}; }
    void ensure(FieldId id, const Value&) {
      // granular read/write operation for the field
      ensure_ = id;
    }
    void patchIfSet(FieldId id, const DynamicPatch& patch) {
      // granular read/write operation
      // Insert next extracted mask and insert field to read mask if the next
      // extracted mask does not include the field.
      auto getIncludesObjRef = [](Mask& mask) { return mask.includes(); };
      auto nextMasks = patch.extractMaskFromPatch();
      detail::insertNextMask(
          masks,
          nextMasks,
          static_cast<int16_t>(id),
          static_cast<int16_t>(id),
          getIncludesObjRef);
      detail::insertFieldsToMask(masks.read, id);
    }
    void remove(FieldId id) {
      // write operation for the field
      detail::insertFieldsToMaskIfNotAllMask(masks.write, id);
    }
    void finalize() {
      if (ensure_) {
        detail::insertFieldsToMask(masks.read, *ensure_);
        detail::insertFieldsToMaskIfNotAllMask(masks.write, *ensure_);
      }
      detail::ensureRWMaskInvariant(masks);
    }

    protocol::ExtractedMasksFromPatch masks{
        protocol::noneMask(), protocol::noneMask()};

   private:
    std::optional<FieldId> ensure_;
  };
  Visitor v;
  customVisit(v);
  v.finalize();
  return std::move(v.masks);
}

ExtractedMasksFromPatch DynamicUnionPatch::extractMaskFromPatch() const {
  struct Visitor {
    void assign(const Object&) {
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void clear() { masks = {protocol::noneMask(), protocol::allMask()}; }
    void ensure(FieldId id, const Value&) {
      // granular read operation for the field and full write operation
      ensure_ = id;
      masks.write = protocol::allMask();
    }
    void patchIfSet(FieldId id, const DynamicPatch& patch) {
      // granular read/write operation
      // Insert next extracted mask and insert field to read mask if the next
      // extracted mask does not include the field.
      auto getIncludesObjRef = [](Mask& mask) { return mask.includes(); };
      auto nextMasks = patch.extractMaskFromPatch();
      detail::insertNextMask(
          masks,
          nextMasks,
          static_cast<int16_t>(id),
          static_cast<int16_t>(id),
          getIncludesObjRef);
      detail::insertFieldsToMask(masks.read, id);
    }
    void finalize() {
      if (ensure_) {
        detail::insertFieldsToMask(masks.read, *ensure_);
      }
      detail::ensureRWMaskInvariant(masks);
    }

    protocol::ExtractedMasksFromPatch masks{
        protocol::noneMask(), protocol::noneMask()};

   private:
    std::optional<FieldId> ensure_;
  };
  Visitor v;
  customVisit(v);
  v.finalize();
  return std::move(v.masks);
}

ExtractedMasksFromPatch DynamicUnknownPatch::extractMaskFromPatch() const {
  struct Visitor {
    void assign(const Object&) {
      // Category::StructuredOrAnyPatch
      // Cannot distinguish between struct/union/any patch
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void clear() {
      // Category::ClearPatch
      // Cannot distinguish patch
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void removeMulti(const ValueSet& v) {
      // Category::AssociativeContainerPatch
      // Cannot distinguish between set/map patch
      if (v.empty()) {
        return;
      }
      masks = {protocol::allMask(), protocol::allMask()};
    }
    void patchIfSet(FieldId id, const DynamicPatch& patch) {
      // Category::StructuredPatch
      // Cannot distinguish struct/union patch
      //
      // granular read/write operation
      // Insert next extracted mask and insert field to read mask if the next
      // extracted mask does not include the field.
      auto getIncludesObjRef = [](Mask& mask) { return mask.includes(); };
      auto nextMasks = patch.extractMaskFromPatch();
      detail::insertNextMask(
          masks,
          nextMasks,
          static_cast<int16_t>(id),
          static_cast<int16_t>(id),
          getIncludesObjRef);
      detail::insertFieldsToMask(masks.read, id);
    }
    void finalize() { detail::ensureRWMaskInvariant(masks); }

    protocol::ExtractedMasksFromPatch masks{
        protocol::noneMask(), protocol::noneMask()};
  };
  Visitor v;
  customVisit(v);
  v.finalize();
  return std::move(v.masks);
}

ExtractedMasksFromPatch DynamicPatch::extractMaskFromPatch() const {
  return visitPatch(folly::overload([&](const auto& patch) {
    return patch.extractMaskFromPatch();
  }));
}

void DynamicListPatch::apply(detail::Badge, ValueList& v) const {
  struct Visitor {
    void assign(ValueList v) {
      detail::convertStringToBinary(v);
      value = std::move(v);
    }
    void clear() { value.clear(); }
    void push_back(detail::Value v) {
      detail::convertStringToBinary(v);
      value.push_back(std::move(v));
    }
    ValueList& value;
  };

  return customVisit(Visitor{v});
}

void DynamicSetPatch::apply(detail::Badge, ValueSet& v) const {
  struct Visitor {
    void assign(ValueSet v) {
      detail::convertStringToBinary(v);
      value = std::move(v);
    }
    void clear() { value.clear(); }
    void addMulti(const ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.insert(std::move(i));
      }
    }
    void removeMulti(const ValueSet& v) {
      for (auto i : v) {
        detail::convertStringToBinary(i);
        value.erase(i);
      }
    }
    ValueSet& value;
  };

  return customVisit(Visitor{v});
}

template <folly::uncvref_same_as<DynamicPatch> Other>
void DynamicPatch::merge(Other&& other) {
  // If only one of the patch is Unknown patch, convert the Unknown patch type
  // to the known patch type.
  if (isPatchTypeAmbiguous() && !other.isPatchTypeAmbiguous()) {
    std::visit(
        [&](auto&& other) {
          using PatchType = folly::remove_cvref_t<decltype(other)>;
          get<DynamicUnknownPatch>().checkConvertible<PatchType>();
          *patch_ = detail::createPatchFromObject<PatchType>(
              badge, std::move(*this).toObject());
        },
        *other.patch_);
  }
  if (!isPatchTypeAmbiguous() && other.isPatchTypeAmbiguous()) {
    return std::visit(
        [&](auto&& patch) {
          using PatchType = folly::remove_cvref_t<decltype(patch)>;
          other.template get<DynamicUnknownPatch>()
              .template checkConvertible<PatchType>();
          auto tmp = DynamicPatch{detail::createPatchFromObject<PatchType>(
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

  if (other.empty()) {
    return;
  }

  if (empty()) {
    *this = other;
    return;
  }

  std::visit(
      [](auto&& l, auto&& r) {
        using L = folly::remove_cvref_t<decltype(l)>;
        using R = folly::remove_cvref_t<decltype(r)>;
        if constexpr (std::is_same_v<L, R>) {
          if constexpr (requires {
                          l.merge(badge, folly::forward_like<Other>(r));
                        }) {
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

DynamicPatch DynamicPatch::fromObject(Object obj) {
  if (auto p = get_ptr(obj, op::PatchOp::EnsureStruct)) {
    if (p->getType() == Value::Type::objectValue) {
      DynamicStructPatch patch;
      patch.fromObject(badge, std::move(obj));
      return DynamicPatch{std::move(patch)};
    }
    if (p->getType() != Value::Type::mapValue) {
      folly::throw_exception<std::runtime_error>(
          "Unknown EnsureStruct type: " + util::enumNameSafe(p->getType()));
    }
    DynamicMapPatch patch;
    patch.fromObject(badge, std::move(obj));
    return DynamicPatch{std::move(patch)};
  }

  if (get_ptr(obj, op::PatchOp::EnsureUnion)) {
    DynamicUnionPatch patch;
    patch.fromObject(badge, std::move(obj));
    return DynamicPatch{std::move(patch)};
  }

  // If the `remove` operation is a list, it must be struct patch.
  if (auto remove = get_ptr(obj, op::PatchOp::Remove)) {
    if (remove->is_list()) {
      DynamicStructPatch patch;
      patch.fromObject(badge, std::move(obj));
      return DynamicPatch{std::move(patch)};
    }
  }

  if (get_ptr(obj, op::PatchOp::PatchIfTypeIsPrior) ||
      get_ptr(obj, op::PatchOp::PatchIfTypeIsAfter) ||
      get_ptr(obj, op::PatchOp::EnsureAny)) {
    return DynamicPatch{
        detail::createPatchFromObject<op::AnyPatch>(badge, std::move(obj))};
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
      return createPatchFromObjectAsValueType(*ptr, std::move(obj));
    }
  }

  DynamicUnknownPatch patch;
  patch.fromObject(badge, std::move(obj));
  return DynamicPatch{std::move(patch)};
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
void DynamicUnknownPatch::throwIncompatibleConversion() const {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "Converting thrift patch in an incompatible way. patch content: {}",
      debugStringViaEncode(patch_)));
}
void DynamicUnknownPatch::removeMulti(ValueSet v) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::AssociativeContainerPatch})) {
    throwIncompatibleCategory("remove");
  }

  auto& s = get(op::PatchOp::Remove).ensure_set();
  v.eraseInto(v.begin(), v.end(), [&](auto&& k) { s.insert(std::move(k)); });
}
void DynamicUnknownPatch::assign(Object v) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::StructuredOrAnyPatch,
           Category::StructuredPatch})) {
    throwIncompatibleCategory("assign");
  }

  patch_.members()->clear();
  get(op::PatchOp::Assign).emplace_object(std::move(v));
}
void DynamicUnknownPatch::patchIfSet(FieldId id, const DynamicPatch& p) {
  if (!isOneOfCategory(
          {Category::EmptyPatch,
           Category::ClearPatch,
           Category::StructuredOrAnyPatch,
           Category::StructuredPatch})) {
    throwIncompatibleCategory("patchIfSet");
  }
  if (auto assign = get_ptr(op::PatchOp::Assign)) {
    if (assign->as_object().contains(id)) {
      p.apply(assign->as_object().at(id));
    }
    return;
  }

  // TODO: optimize this
  DynamicPatch prior{DynamicPatch::fromObject(
      get(op::PatchOp::PatchPrior).ensure_object()[id].ensure_object())};
  DynamicPatch after{DynamicPatch::fromObject(
      get(op::PatchOp::PatchAfter).ensure_object()[id].ensure_object())};
  prior.merge(after);
  prior.merge(p);
  get(op::PatchOp::PatchAfter).ensure_object().erase(id);
  get(op::PatchOp::PatchPrior).ensure_object()[id].as_object() =
      prior.toObject();
}
void DynamicUnknownPatch::apply(detail::Badge, Value& v) const {
  struct Visitor {
    void assign(detail::Object obj) {
      detail::convertStringToBinary(obj);
      value.as_object() = std::move(obj);
    }
    void clear() { value = emptyValue(value.getType()); }
    void removeMulti(const ValueSet& v) {
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
    void patchIfSet(FieldId id, const DynamicPatch& p) {
      if (auto field = value.as_object().if_contains(id)) {
        p.apply(*field);
      }
    }
    Value& value;
  };

  return customVisit(Visitor{v});
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
  visitPatch(applier);
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

void DynamicPatch::applyObjectInAny(detail::Badge, type::AnyStruct& any) const {
  if (any.protocol() == type::StandardProtocol::Binary) {
    any.data() = *applyToSerializedObjectWithoutExtractingMask<
        type::StandardProtocol::Binary>(*any.data());
  } else if (any.protocol() == type::StandardProtocol::Compact) {
    any.data() = *applyToSerializedObjectWithoutExtractingMask<
        type::StandardProtocol::Compact>(*any.data());
  } else {
    throw std::runtime_error(
        "Unsupported protocol: " + std::string(any.protocol()->name()));
  }
}

template <type::StandardProtocol Protocol>
std::unique_ptr<folly::IOBuf> DynamicPatch::applyToSerializedObject(
    const folly::IOBuf& buf) const {
  using ProtocolReader = std::conditional_t<
      Protocol == type::StandardProtocol::Binary,
      BinaryProtocolReader,
      CompactProtocolReader>;
  using ProtocolWriter = std::conditional_t<
      Protocol == type::StandardProtocol::Binary,
      BinaryProtocolWriter,
      CompactProtocolWriter>;
  auto masks = extractMaskFromPatch();
  MaskedDecodeResult result =
      parseObject<ProtocolReader>(buf, masks.read, masks.write);
  protocol::Value val;
  val.emplace_object(std::move(result.included));
  apply(val);
  return serializeObject<ProtocolWriter>(val.as_object(), result.excluded);
}

template <type::StandardProtocol Protocol>
std::unique_ptr<folly::IOBuf>
DynamicPatch::applyToSerializedObjectWithoutExtractingMask(
    const folly::IOBuf& buf) const {
  auto p = std::make_unique<folly::IOBuf>(buf);
  return visitPatch([&](const auto& patch) -> std::unique_ptr<folly::IOBuf> {
    if constexpr (requires {
                    patch.template applyToSerializedObject<Protocol>(
                        std::move(p));
                  }) {
      return patch.template applyToSerializedObject<Protocol>(std::move(p));
    } else {
      folly::throw_exception_fmt_format<std::runtime_error>(
          "NotImplemented: Applying {} to Object is not implemented",
          folly::pretty_name<folly::remove_cvref_t<decltype(patch)>>());
    }
  });
}

template std::unique_ptr<folly::IOBuf> DynamicPatch::applyToSerializedObject<
    type::StandardProtocol::Binary>(const folly::IOBuf& buf) const;
template std::unique_ptr<folly::IOBuf> DynamicPatch::applyToSerializedObject<
    type::StandardProtocol::Compact>(const folly::IOBuf& buf) const;
template std::unique_ptr<folly::IOBuf>
DynamicPatch::applyToSerializedObjectWithoutExtractingMask<
    type::StandardProtocol::Binary>(const folly::IOBuf& buf) const;
template std::unique_ptr<folly::IOBuf>
DynamicPatch::applyToSerializedObjectWithoutExtractingMask<
    type::StandardProtocol::Compact>(const folly::IOBuf& buf) const;

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

type::AnyStruct DynamicPatch::toPatch(type::Type type) const {
  type::AnyStruct any;
  any.protocol() = type::StandardProtocol::Compact;
  any.data() = *encode<CompactProtocolWriter>();
  any.type() = toPatchType(std::move(type));
  return any;
}

DynamicPatch DynamicPatch::fromPatch(const type::AnyStruct& any) {
  auto v = protocol::parseValueFromAny(any);
  return DynamicPatch::fromObject(std::move(v.as_object()));
}

template <typename Protocol>
std::uint32_t DynamicPatch::encode(Protocol& prot) const {
  return visitPatch([&](const auto& patch) { return patch.encode(prot); });
}

template <typename Protocol>
void DynamicPatch::decode(Protocol& prot) {
  // TODO(dokwon): Provide direct decode to DynamicPatch.
  *this = fromObject(protocol::parseObject(prot));
}

template <typename Protocol>
std::unique_ptr<folly::IOBuf> DynamicPatch::encode() const {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  Protocol prot;
  prot.setOutput(&queue);
  encode(prot);
  return queue.move();
}

template <typename Protocol>
void DynamicPatch::decode(const folly::IOBuf& buf) {
  Protocol prot;
  prot.setInput(&buf);
  decode(prot);
}

/// @cond
template std::uint32_t DynamicPatch::encode(
    apache::thrift::BinaryProtocolWriter&) const;
template std::uint32_t DynamicPatch::encode(
    apache::thrift::CompactProtocolWriter&) const;
template std::uint32_t DynamicPatch::encode(
    apache::thrift::protocol::detail::ObjectWriter&) const;
template std::unique_ptr<folly::IOBuf>
DynamicPatch::encode<apache::thrift::BinaryProtocolWriter>() const;
template std::unique_ptr<folly::IOBuf>
DynamicPatch::encode<apache::thrift::CompactProtocolWriter>() const;
template void DynamicPatch::decode(apache::thrift::BinaryProtocolReader&);
template void DynamicPatch::decode(apache::thrift::CompactProtocolReader&);
template void DynamicPatch::decode<apache::thrift::BinaryProtocolReader>(
    const folly::IOBuf&);
template void DynamicPatch::decode<apache::thrift::CompactProtocolReader>(
    const folly::IOBuf&);
/// @endcond

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
  void clear() {}
  void recurse(const DynamicPatch& patch) {
    // recurse visitPatch
    patch.visitPatch(
        folly::overload(
            [&](const DynamicMapPatch& p) { p.customVisit(*this); },
            [&](const DynamicStructPatch& p) { p.customVisit(*this); },
            [&](const DynamicUnionPatch& p) { p.customVisit(*this); },
            [&](const op::AnyPatch& p) {
              // recurse AnyPatch in case it only uses `assign` or `clear`
              // operations that are V1.
              p.customVisit(*this);
            },
            [&](const DynamicUnknownPatch& p) {
              // recurse DynamicUnknownPatch for `patchPrior/patchAfter` in
              // `DynamicUnknownPatch::Category::StructuredPatch`.
              p.customVisit(*this);
            },
            [&](const auto&) {
              // Short circuit all other patch types.
            }));
  }

  // Map
  void putMulti(const ValueMap&) {}
  void tryPutMulti(const ValueMap&) {}
  void removeMulti(const ValueSet&) {}
  void patchByKey(const Value&, const DynamicPatch& p) { recurse(p); }

  // Structured
  void ensure(FieldId, const Value&) {}
  void remove(FieldId) {}
  void patchIfSet(FieldId, const DynamicPatch& fieldPatch) {
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

DynamicPatch DynamicPatch::fromSafePatch(const type::AnyStruct& any) {
  DynamicPatch patch;
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
  patch.decode<apache::thrift::CompactProtocolReader>(*safePatch.data());
  return patch;
}
type::AnyStruct DynamicPatch::toSafePatch(type::Type type) const {
  auto safePatchType = toSafePatchType(std::move(type));
  MinSafePatchVersionVisitor visitor;
  visitor.recurse(*this);
  DynamicSafePatch safePatch{visitor.version, encode<CompactProtocolWriter>()};

  type::AnyStruct anyStruct;
  anyStruct.type() = std::move(safePatchType);
  anyStruct.protocol() = type::StandardProtocol::Compact;
  anyStruct.data() = *safePatch.encode<CompactProtocolWriter>();
  return anyStruct;
}

} // namespace apache::thrift::protocol
