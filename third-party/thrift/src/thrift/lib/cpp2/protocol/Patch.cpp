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

#include <thrift/lib/cpp2/protocol/Patch.h>

#include <cmath>
#include <limits>
#include <stdexcept>

#include <fmt/core.h>
#include <folly/MapUtil.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Exception.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp/util/SaturatingMath.h>
#include <thrift/lib/cpp/util/VarintUtils.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/detail/Patch.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_detail_types.h>
#include <thrift/lib/thrift/gen-cpp2/patch_op_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {
namespace detail {
namespace {

using op::PatchOp;

template <typename Tag>
using value_field_id =
    type::field_id_tag<static_cast<FieldId>(type::base_type_v<Tag>)>;

template <typename Tag>
using value_native_type =
    op::get_native_type<detail::Value, value_field_id<Tag>>;

constexpr FieldId kSafePatchVersionId = FieldId{1};
constexpr FieldId kSafePatchDataId = FieldId{2};

void checkNotSafePatch(const Object& patch) {
  const Value* version = patch.if_contains(kSafePatchVersionId);
  const Value* data = patch.if_contains(kSafePatchDataId);
  if (version && version->is_i32() && data && data->is_binary()) {
    folly::throw_exception<std::runtime_error>(
        "Safe Patch provided. Use `fromSafePatch` to convert to Dynamic Patch first.");
  }
}

PatchOp toOp(FieldId id) {
  auto op = static_cast<PatchOp>(id);
  if (util::enumName<PatchOp>(op) == nullptr) {
    folly::throw_exception<std::runtime_error>(
        fmt::format("Unknown operation id found in patch object: {}", id));
  }
  return op;
}

void checkOps(
    const Object& patch,
    Value::Type valueType,
    folly::F14VectorSet<PatchOp> supportedOps) {
  checkNotSafePatch(patch);
  for (const auto& field : patch) {
    auto op = toOp(FieldId{field.first});
    if (supportedOps.find(op) == supportedOps.end()) {
      folly::throw_exception<std::runtime_error>(fmt::format(
          "Unsupported op: {}({})",
          util::enumNameSafe<PatchOp>(op),
          util::enumNameSafe<Value::Type>(valueType)));
    }
  }
}

const Value* findOp(const Object& patch, PatchOp op) {
  return patch.if_contains(static_cast<FieldId>(op));
}

Value* findOp(Object&& patch, PatchOp op) {
  return patch.if_contains(static_cast<FieldId>(op));
}

template <typename Tag, typename V>
decltype(auto) argAs(V&& value) {
  static_assert(std::is_same_v<folly::remove_cvref_t<V>, Value>);
  using Id = type::field_id_tag<static_cast<FieldId>(type::base_type_v<Tag>)>;
  constexpr auto expected = static_cast<Value::Type>(Id::value);
  if (value.getType() != Value::Type(Id::value)) {
    folly::throw_exception<std::runtime_error>(fmt::format(
        "Unexpected type in the patch. Expected {} got {}",
        util::enumNameSafe<Value::Type>(expected),
        util::enumNameSafe<Value::Type>(value.getType())));
  }
  return *op::get<Id, Value::ThriftValue>(std::forward<V>(value).toThrift());
}

template <typename Tag>
bool applyAssign(const Object& patch, value_native_type<Tag>& value) {
  if (const Value* arg = findOp(patch, PatchOp::Assign)) {
    value = argAs<Tag>(*arg);
    return true;
  }
  return false;
}

template <typename Tag>
bool applyAssign(Object&& patch, value_native_type<Tag>& value) {
  if (Value* arg = findOp(std::move(patch), PatchOp::Assign)) {
    value = argAs<Tag>(std::move(*arg));
    return true;
  }
  return false;
}

template <typename Tag, typename T>
void applyNumericPatch(const Object& patch, T& value) {
  constexpr auto valueType = static_cast<Value::Type>(type::base_type_v<Tag>);
  checkOps(patch, valueType, {PatchOp::Assign, PatchOp::Clear, PatchOp::Add});
  if (applyAssign<Tag>(patch, value)) {
    return; // Ignore all other ops.
  }
  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value = {};
    }
  }

  if (auto* arg = findOp(patch, PatchOp::Add)) {
    value = util::add_saturating<T>(value, argAs<Tag>(*arg));
  }
}

} // namespace

void ApplyPatch::operator()(const Object& patch, protocol::Value& value) const {
  // TODO(afuller): Consider using visitation instead.
  switch (value.getType()) {
    case Value::Type::boolValue:
      return operator()(patch, value.as_bool());
    case Value::Type::byteValue:
      return operator()(patch, value.as_byte());
    case Value::Type::i16Value:
      return operator()(patch, value.as_i16());
    case Value::Type::i32Value:
      return operator()(patch, value.as_i32());
    case Value::Type::i64Value:
      return operator()(patch, value.as_i64());
    case Value::Type::floatValue:
      return operator()(patch, value.as_float());
    case Value::Type::doubleValue:
      return operator()(patch, value.as_double());
    case Value::Type::stringValue: {
      auto binaryValue = folly::IOBuf::wrapBufferAsValue(
          value.as_string().data(), value.as_string().size());
      operator()(patch, binaryValue);
      value.emplace_string(binaryValue.to<std::string>());
      return;
    }
    case Value::Type::binaryValue:
      return operator()(patch, value.as_binary());
    case Value::Type::listValue:
      return operator()(patch, value.as_list());
    case Value::Type::setValue:
      return operator()(patch, value.as_set());
    case Value::Type::mapValue:
      return operator()(patch, value.as_map());
    case Value::Type::objectValue:
      return operator()(patch, value.as_object());
    default:
      folly::throw_exception<std::runtime_error>(
          "Not Implemented type support.");
  }
}

void ApplyPatch::operator()(const Object& patch, bool& value) const {
  checkOps(
      patch,
      Value::Type::boolValue,
      {PatchOp::Assign, PatchOp::Put, PatchOp::Clear});
  if (applyAssign<type::bool_t>(patch, value)) {
    return; // Ignore all other ops.
  }
  if (auto* clear = findOp(patch, PatchOp::Clear);
      clear != nullptr && clear->as_bool()) {
    value = false;
  }
  if (auto* invert = findOp(patch, PatchOp::Put)) { // Put is Invert for bool.
    if (argAs<type::bool_t>(*invert)) {
      value = !value;
    }
  }
}

void ApplyPatch::operator()(const Object& patch, int8_t& value) const {
  applyNumericPatch<type::byte_t>(patch, value);
}
void ApplyPatch::operator()(const Object& patch, int16_t& value) const {
  applyNumericPatch<type::i16_t>(patch, value);
}
void ApplyPatch::operator()(const Object& patch, int32_t& value) const {
  applyNumericPatch<type::i32_t>(patch, value);
}
void ApplyPatch::operator()(const Object& patch, int64_t& value) const {
  applyNumericPatch<type::i64_t>(patch, value);
}
void ApplyPatch::operator()(const Object& patch, float& value) const {
  applyNumericPatch<type::float_t>(patch, value);
}
void ApplyPatch::operator()(const Object& patch, double& value) const {
  applyNumericPatch<type::double_t>(patch, value);
}

void ApplyPatch::operator()(const Object& patch, folly::IOBuf& value) const {
  checkOps(
      patch,
      Value::Type::binaryValue,
      {PatchOp::Assign, PatchOp::Clear, PatchOp::Add, PatchOp::Put});
  if (applyAssign<type::cpp_type<folly::IOBuf, type::binary_t>>(patch, value)) {
    return; // Ignore all other ops.
  }

  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value = folly::IOBuf{};
    }
  }

  auto* prepend = findOp(patch, PatchOp::Add);
  auto* append = findOp(patch, PatchOp::Put);
  if (append || prepend) {
    folly::IOBufQueue queue;
    if (prepend) {
      queue.append(argAs<type::binary_t>(*prepend));
    }
    queue.append(value);
    if (append) {
      queue.append(argAs<type::binary_t>(*append));
    }
    value = queue.moveAsValue();
  }
}

void ApplyPatch::operator()(
    const Object& patch, std::vector<Value>& value) const {
  DCHECK(
      !findOp(patch, PatchOp::Add)); // Make sure no one relies on List::prepend
  checkOps(
      patch,
      Value::Type::listValue,
      {PatchOp::Assign, PatchOp::Clear, PatchOp::Put});
  if (applyAssign<type::list_c>(patch, value)) {
    return; // Ignore all other ops.
  }

  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value.clear();
    }
  }

  if (auto* append = findOp(patch, PatchOp::Put)) {
    const auto* appendVector = append->if_list();
    if (!appendVector) {
      throw std::runtime_error("list put patch should contain a list");
    }
    value.insert(value.end(), appendVector->begin(), appendVector->end());
  }
}

void ApplyPatch::operator()(
    const Object& patch, folly::F14VectorSet<Value>& value) const {
  checkOps(
      patch,
      Value::Type::setValue,
      {PatchOp::Assign,
       PatchOp::Clear,
       PatchOp::Add,
       PatchOp::Put,
       PatchOp::Remove});
  if (applyAssign<type::set_c>(patch, value)) {
    return; // Ignore all other ops.
  }

  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value.clear();
    }
  }

  auto validate_if_set = [](const auto patchOp, auto name) {
    const auto* opData = patchOp->if_set();
    if (!opData) {
      throw std::runtime_error(
          fmt::format("set {} patch should contain a set", name));
    }
    return opData;
  };

  if (auto* remove = findOp(patch, PatchOp::Remove)) {
    for (const auto& key : *validate_if_set(remove, "remove")) {
      value.erase(key);
    }
  }

  auto insert_set = [&](const auto& to_insert) {
    value.insert(to_insert.begin(), to_insert.end());
  };

  if (auto* add = findOp(patch, PatchOp::Add)) {
    insert_set(*validate_if_set(add, "add"));
  }

  if (auto* put = findOp(patch, PatchOp::Put)) {
    auto msg = "SetPatch::Put should be migrated to SetPatch::Add";
    XLOG(DFATAL) << msg;
    insert_set(*validate_if_set(put, "put"));
  }
}

void ApplyPatch::operator()(
    const Object& patch, folly::F14FastMap<Value, Value>& value) const {
  checkOps(
      patch,
      Value::Type::mapValue,
      {PatchOp::Assign,
       PatchOp::Clear,
       PatchOp::PatchPrior,
       PatchOp::EnsureStruct,
       PatchOp::Put,
       PatchOp::Remove,
       PatchOp::PatchAfter});
  if (applyAssign<type::map_c>(patch, value)) {
    return; // Ignore all other ops.
  }

  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value.clear();
    }
  }

  auto validated_map = [](auto patchOp, auto patchOpName) {
    auto opData = patchOp->if_map();
    if (!opData) {
      throw std::runtime_error(
          fmt::format("map {} patch should contain map", patchOpName));
    }
    return opData;
  };

  auto patchElements = [&](auto patchFields) {
    for (const auto& [keyv, valv] :
         *validated_map(patchFields, "patch/patchPrior")) {
      // Only patch values for fields that exist for now
      if (auto* field = folly::get_ptr(value, keyv)) {
        applyPatch(valv.as_object(), *field);
      }
    }
  };

  if (auto* patchFields = findOp(patch, PatchOp::PatchPrior)) {
    patchElements(patchFields);
  }

  // This is basicly inserting key/value pair into the map if key doesn't exist
  if (auto* ensure = findOp(patch, PatchOp::EnsureStruct)) {
    const auto* mapVal = validated_map(ensure, "ensureStruct");
    value.insert(mapVal->begin(), mapVal->end());
  }

  if (auto* remove = findOp(patch, PatchOp::Remove)) {
    const auto* to_remove = remove->if_set();
    if (!to_remove) {
      throw std::runtime_error("map remove patch should contain set");
    }
    for (const auto& key : *to_remove) {
      value.erase(key);
    }
  }

  if (auto* put = findOp(patch, PatchOp::Put)) {
    for (const auto& [key, val] : *validated_map(put, "put")) {
      value.insert_or_assign(key, val);
    }
  }

  if (auto* patchFields = findOp(patch, PatchOp::PatchAfter)) {
    patchElements(patchFields);
  }
}

template <class Patch>
void impl(Patch&& patch, Object& value) {
  static_assert(std::is_same_v<folly::remove_cvref_t<Patch>, Object>);
  checkOps(
      patch,
      Value::Type::objectValue,
      {PatchOp::Assign,
       PatchOp::Clear,
       PatchOp::Remove,
       PatchOp::PatchPrior,
       PatchOp::EnsureStruct,
       PatchOp::EnsureUnion,
       PatchOp::PatchAfter,
       PatchOp::PatchIfTypeIsPrior,
       PatchOp::EnsureAny,
       PatchOp::PatchIfTypeIsAfter});
  if (applyAssign<type::struct_c>(std::forward<Patch>(patch), value)) {
    return; // Ignore all other ops.
  }

  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value.members()->clear();
    }
  }

  auto applyFieldPatch = [&](auto patchFields) {
    const auto* obj = patchFields->if_object();
    if (!obj) {
      throw std::runtime_error(
          "struct patch PatchPrior/Patch should contain an object");
    }
    for (const auto& [id, field_value] : *obj->members()) {
      // Only patch values for fields that exist for now
      if (auto* field = folly::get_ptr(*value.members(), id)) {
        applyPatch(field_value.as_object(), *field);
      }
    }
  };

  if (auto* patchFields = findOp(patch, PatchOp::PatchPrior)) {
    applyFieldPatch(patchFields);
  }

  if (const auto* sensure = findOp(patch, PatchOp::EnsureStruct)) {
    if (const auto* obj = sensure->if_object()) {
      value.members()->insert(obj->begin(), obj->end());
    } else {
      throw std::runtime_error("struct patch Ensure should contain an object");
    }
  }

  if (const auto* uensure = findOp(patch, PatchOp::EnsureUnion)) {
    const auto* ensureUnion = uensure->if_object();
    if (!ensureUnion) {
      throw std::runtime_error("union patch Ensure should contain an object");
    }

    // EnsureUnion is not optional and will contain an empty union by default.
    // We should ignore such cases.
    if (!ensureUnion->empty()) {
      if (ensureUnion->size() != 1) {
        throw std::runtime_error(
            "union patch Ensure should contain an object with only one field set");
      }

      auto& id = ensureUnion->begin()->first;
      auto itr = value.members()->find(id);
      if (itr == value.end()) {
        value = *ensureUnion;
      } else if (value.size() != 1) {
        // Clear other values, without copying the current value
        *value.members() = {{itr->first, std::move(itr->second)}};
      }
    }
  }

  if (auto* patchFields = findOp(patch, PatchOp::PatchAfter)) {
    applyFieldPatch(patchFields);
  }
  if (auto* to_remove = findOp(patch, PatchOp::Remove)) {
    auto remove = [&](const auto& ids) {
      for (const auto& field_id : ids) {
        if (!field_id.is_i16()) {
          throw std::runtime_error(fmt::format(
              "The `PatchOp::Remove` field in struct/union patch is not `set<i16>` but `set<{}>`",
              util::enumNameSafe(field_id.getType())));
        }

        value.erase(FieldId{field_id.as_i16()});
      }
    };

    if (const auto* p_set = to_remove->if_set()) {
      // TODO: Remove this after migrating to List
      auto msg = "StructPatch::Remove should be migrated from `set` to `list`";
      XLOG(DFATAL) << msg;
      remove(*p_set);
    } else if (const auto* p_list = to_remove->if_list()) {
      remove(*p_list);
    } else {
      throw std::runtime_error(fmt::format(
          "The `PatchOp::Remove` field in struct/union patch is not `set<i16>`/`list<i16>` but `{}`",
          util::enumNameSafe(to_remove->getType())));
    }
  }

  // Handling Thrift AnyPatch operations.
  const auto* typePatchPriorVal = findOp(patch, PatchOp::PatchIfTypeIsPrior);
  const auto* ensureAnyVal = findOp(patch, PatchOp::EnsureAny);
  const auto* typePatchAfterVal = findOp(patch, PatchOp::PatchIfTypeIsAfter);

  if (!typePatchPriorVal && !ensureAnyVal && !typePatchAfterVal) {
    return;
  }

  type::AnyStruct anyStruct;
  if (!ProtocolValueToThriftValue<type::struct_t<type::AnyStruct>>{}(
          value, anyStruct)) {
    throw std::runtime_error("Failed to convert current object to AnyStruct");
  }

  auto applyTypePatch = [&](const Value* typePatchListPrior,
                            const Value* typePatchListAfter) {
    std::optional<protocol::Value> val;

    if (typePatchListPrior) {
      for (const auto& typePatchVal : typePatchListPrior->as_list()) {
        op::TypeToPatchInternalDoNotUse type_to_patch;
        if (!ProtocolValueToThriftValue<
                type::struct_t<op::TypeToPatchInternalDoNotUse>>{}(
                typePatchVal, type_to_patch)) {
          throw std::runtime_error("Invalid AnyPatch PatchIfTypeIsPrior");
        }
        if (type::identicalType(
                type_to_patch.type().value(), anyStruct.type().value())) {
          val = protocol::detail::parseValueFromAny(anyStruct);
          auto dynPatch =
              protocol::detail::parseValueFromAny(type_to_patch.patch().value())
                  .as_object();
          ApplyPatch{}(dynPatch, val.value());
          break;
        }
      }
    }

    if (typePatchListAfter) {
      for (const auto& typePatchVal : typePatchListAfter->as_list()) {
        op::TypeToPatchInternalDoNotUse type_to_patch;
        if (!ProtocolValueToThriftValue<
                type::struct_t<op::TypeToPatchInternalDoNotUse>>{}(
                typePatchVal, type_to_patch)) {
          throw std::runtime_error("Invalid AnyPatch PatchIfTypeIsAfter");
        }
        if (type::identicalType(
                type_to_patch.type().value(), anyStruct.type().value())) {
          if (!val) {
            val = protocol::detail::parseValueFromAny(anyStruct);
          }
          auto dynPatch =
              protocol::detail::parseValueFromAny(type_to_patch.patch().value())
                  .as_object();
          ApplyPatch{}(dynPatch, val.value());
          break;
        }
      }
    }

    if (val) {
      anyStruct = protocol::detail::toAny(
                      val.value(),
                      anyStruct.type().value(),
                      anyStruct.protocol().value())
                      .toThrift();
    }
  };

  if (ensureAnyVal) {
    type::AnyStruct ensureAny;
    if (!ProtocolValueToThriftValue<type::struct_t<type::AnyStruct>>{}(
            *ensureAnyVal, ensureAny)) {
      throw std::runtime_error("Invalid AnyPatch ensureAny");
    }
    // If 'ensureAny' type does not match the type of stored value in Thrift
    // Any, we can ignore 'patchIfTypeIsPrior'.
    if (!type::identicalType(
            ensureAny.type().value(), anyStruct.type().value())) {
      anyStruct = std::move(ensureAny);
      applyTypePatch(nullptr, typePatchAfterVal);
      value =
          asValueStruct<type::struct_t<type::AnyStruct>>(anyStruct).as_object();
      return;
    }
  }

  applyTypePatch(typePatchPriorVal, typePatchAfterVal);
  value = asValueStruct<type::struct_t<type::AnyStruct>>(anyStruct).as_object();
}

void ApplyPatch::operator()(const Object& patch, Object& value) const {
  impl(patch, value);
}
void ApplyPatch::operator()(Object&& patch, Object& value) const {
  impl(std::move(patch), value);
}

// Insert allMask to getIncludesRef(mask)[id] if id does not exist.
template <typename Id, typename F>
void tryInsertAllMask(Mask& mask, Id id, const F& getIncludesRef) {
  getIncludesRef(mask).ensure().emplace(std::move(id), allMask());
}

// Constructs the field mask from the patch object for the field.
void insertNextFieldsToMask(
    ExtractedMasksFromPatch& masks, const Object& patch) {
  auto getIncludesObjRef = [&](Mask& mask) { return mask.includes(); };
  for (const auto& [id, value] : patch) {
    // Object patch can get here only StructPatch::Patch(Prior|After)
    // operations, which require reading existing value to know if/how given
    // operations can/should be applied. Generate allMask() read mask for them
    // if the recursively extracted masks from patch does not include the
    // field.
    auto nextMasks = extractMaskFromPatch(value.as_object());
    insertNextMask(masks, nextMasks, id, id, getIncludesObjRef);
    insertFieldsToMask(masks.read, FieldId{id});
  }
}

// Constructs the map mask from the patch object for the map. It uses the
// appropriate integer map mask and string map mask after parsing from Value.
void insertNextMapItemsToMask(
    ExtractedMasksFromPatch& masks,
    const folly::F14FastMap<protocol::Value, protocol::Value>& patch) {
  auto getIncludesMapRef = [&](Mask& mask) { return mask.includes_map(); };
  auto getIncludesStringMapRef = [&](Mask& mask) {
    return mask.includes_string_map();
  };
  // Map patch can get here only MapPatch::Patch(Prior|After) operations,
  // which require reading existing value to know if/how given operations
  // can/should be applied. Generate allMask() read map mask if the
  // recursively extracted masks from patch do not include the key.
  for (const auto& [key, value] : patch) {
    auto nextMasks = extractMaskFromPatch(value.as_object());
    if (getArrayKeyFromValue(key) == ArrayKey::Integer) {
      auto id = static_cast<int64_t>(getMapIdFromValue(key));
      insertNextMask(masks, nextMasks, id, id, getIncludesMapRef);
      insertKeysToMask(masks.read, id);
    } else {
      auto id = getStringFromValue(key);
      insertNextMask(masks, nextMasks, id, id, getIncludesStringMapRef);
      insertKeysToMask(masks.read, id);
    }
  }
}

void insertNextTypesToMask(
    ExtractedMasksFromPatch& masks, const Value& patchTypes) {
  auto getIncludesTypeRef = [&](Mask& mask) { return mask.includes_type(); };
  for (const auto& typePatchVal : patchTypes.as_list()) {
    op::TypeToPatchInternalDoNotUse type_to_patch;
    if (!ProtocolValueToThriftValue<
            type::struct_t<op::TypeToPatchInternalDoNotUse>>{}(
            typePatchVal, type_to_patch)) {
      throw std::runtime_error(
          "Invalid AnyPatch PatchIfTypeIsPrior/PatchIfTypeIsAfter");
    }
    if (!type_to_patch.type()->isValid()) {
      throw std::runtime_error("Invalid Type");
    }
    auto dynPatch =
        protocol::detail::parseValueFromAny(type_to_patch.patch().value());
    auto nextMasks = extractMaskFromPatch(dynPatch.as_object());
    insertNextMask(
        masks,
        nextMasks,
        type_to_patch.type().value(),
        type_to_patch.type().value(),
        getIncludesTypeRef);
    insertTypeToMaskIfNotAllMask(masks.read, type_to_patch.type().value());
  }
}

// We cannot distinguish SetPatch::remove and MapPatch::remove without
// additional schema information. However, we can disambiguate if the same
// patch contains other map operations which is tracked by `isMap`.
ExtractedMasksFromPatch extractMaskFromPatch(
    const protocol::Object& patch, bool isMap = false) {
  ExtractedMasksFromPatch masks = {noneMask(), noneMask()};

  // If Assign, it is a write operation
  if (findOp(patch, PatchOp::Assign)) {
    return {noneMask(), allMask()};
  }
  // If Clear, it is a write operation if not intristic default.
  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (!isIntrinsicDefault(*clear)) {
      masks = {noneMask(), allMask()};
    }
  }
  // If Add, it is a read-write operation if not intristic default.
  if (auto* add = findOp(patch, PatchOp::Add)) {
    if (!isIntrinsicDefault(*add)) {
      return {allMask(), allMask()};
    }
  }

  // If Put, it is a write operation for map and read-write operation for
  // others.
  if (auto* value = findOp(patch, PatchOp::Put)) {
    if (value->is_map()) {
      isMap = true;
      insertKeysToMaskIfNotAllMask(masks.write, value->as_map());
    } else if (!isIntrinsicDefault(*value)) {
      return {allMask(), allMask()};
    }
  }

  // If PatchPrior or PatchAfter, recursively constructs a field mask or map
  // mask.
  for (auto op : {PatchOp::PatchPrior, PatchOp::PatchAfter}) {
    if (auto* subpatch = findOp(patch, op)) {
      if (subpatch->is_object()) {
        insertNextFieldsToMask(masks, subpatch->as_object());
      } else if (subpatch->is_map()) {
        isMap = true;
        insertNextMapItemsToMask(masks, subpatch->as_map());
      }
    }
  }

  // If EnsureStruct, add fields/keys to mask.
  if (auto* ensureStruct = findOp(patch, PatchOp::EnsureStruct)) {
    if (ensureStruct->is_object()) {
      // Ensure requires reading existing value to know whether the field is set
      // or not. Insert allMask() if the field was never included in read mask
      // before.
      insertFieldsToMask(masks.read, ensureStruct->as_object());
      insertFieldsToMaskIfNotAllMask(masks.write, ensureStruct->as_object());
    } else if (ensureStruct->is_map()) {
      isMap = true;
      insertKeysToMask(masks.read, ensureStruct->as_map());
      insertKeysToMaskIfNotAllMask(masks.write, ensureStruct->as_map());
    }
  }

  // If EnsureUnion, add fields to mask for read mask and all mask for write
  // mask.
  if (auto* ensureUnion = findOp(patch, PatchOp::EnsureUnion)) {
    // Ensure requires reading existing value to know whether the field is set
    // or not. Insert allMask() if the field was never included in read mask
    // before.
    insertFieldsToMask(masks.read, ensureUnion->as_object());
    masks.write = allMask();
  }

  // We can only distinguish struct. For struct, add removed fields to write
  // mask. Both set and map use a set for Remove, so they are indistinguishable.
  // If any of map operations is used we can correctly distinguish between map
  // and set, then we add removed keys to write map mask. For set or any of map
  // operations is not used, it is a read-write operation if not intristic
  // default.
  if (auto* value = findOp(patch, PatchOp::Remove)) {
    if (value->is_list()) {
      // struct patch
      insertFieldsToMaskIfNotAllMask(masks.write, value->as_list());
    } else if (!isIntrinsicDefault(*value)) {
      if (!isMap) {
        // cannot distinguish between set/map patch
        return {allMask(), allMask()};
      }
      insertKeysToMaskIfNotAllMask(masks.write, value->as_set());
    }
  }

  // If PatchIfTypeIsPrior or PatchIfTypeIsAfter, recursively constructs the
  // mask each type patch.
  for (auto op : {PatchOp::PatchIfTypeIsPrior, PatchOp::PatchIfTypeIsAfter}) {
    if (auto* patchTypes = findOp(patch, op)) {
      insertNextTypesToMask(masks, *patchTypes);
    }
  }

  // If EnsureAny, add type to mask for read mask and all mask for write mask.
  if (auto* ensureAny = findOp(patch, PatchOp::EnsureAny)) {
    type::AnyStruct anyStruct;
    if (!ProtocolValueToThriftValue<type::struct_t<type::AnyStruct>>{}(
            *ensureAny, anyStruct)) {
      throw std::runtime_error("Failed to convert current object to AnyStruct");
    }
    if (!type::AnyData::isValid(anyStruct)) {
      throw std::runtime_error("Invalid AnyStruct");
    }
    insertTypeToMaskIfNotAllMask(masks.read, anyStruct.type().value());
    masks.write = allMask();
  }

  ensureRWMaskInvariant(masks);

  return masks;
}

int32_t calculateMinSafePatchVersion(const protocol::Object& patch) {
  int32_t version = 1;
  for (const auto& [fieldId, patchMember] : *patch.members()) {
    switch (static_cast<PatchOp>(fieldId)) {
      case PatchOp::Assign:
      case PatchOp::Clear:
      case PatchOp::EnsureUnion:
      case PatchOp::EnsureStruct:
      case PatchOp::Remove:
      case PatchOp::Add:
      case PatchOp::Put:
        version = std::max(version, 1);
        break;
      case PatchOp::PatchPrior:
      case PatchOp::PatchAfter: {
        // We need to handle both StructPatch and MapPatch here.
        if (const auto* fieldPatch = patchMember.if_object()) {
          for (const auto& [_, p] : *fieldPatch) {
            version =
                std::max(version, calculateMinSafePatchVersion(p.as_object()));
          }
        } else if (const auto* elemPatch = patchMember.if_map()) {
          for (const auto& [_, p] : *elemPatch) {
            version =
                std::max(version, calculateMinSafePatchVersion(p.as_object()));
          }
        } else {
          folly::throw_exception<std::runtime_error>(
              "Invalid PatchOp::PatchPrior/PatchAfter");
        }
        break;
      }
      case PatchOp::PatchIfTypeIsPrior:
      case PatchOp::PatchIfTypeIsAfter:
      case PatchOp::EnsureAny:
        // For now, we don't need to peek into AnyPatch to calculate Thrift
        // SafePatch version.
        version = std::max(version, 2);
        break;
      case PatchOp::Unspecified:
        folly::throw_exception<std::runtime_error>("Invalid patch operation");
    }
  }
  return version;
}

ExtractedMasksFromPatch extractMapMaskFromPatchImpl(
    const protocol::Value& patch, const Mask& mask);
ExtractedMasksFromPatch extractMapMaskFromPatchImpl(
    const protocol::Object& patch, const Mask& mask);

template <typename K>
ExtractedMasksFromPatch extractMapMaskFromPatchMapImpl(
    const protocol::Object& patch, const K& k, const Mask& v) {
  if (auto* removeSet = findOp(patch, PatchOp::Remove)) {
    const auto& set = removeSet->as_set();
    if (!set.empty()) {
      if (set.contains(getValueAs(k, *set.begin()))) {
        return {noneMask(), allMask()};
      }
    }
  }

  ExtractedMasksFromPatch rwmask = {noneMask(), noneMask()};
  std::optional<Value> key;
  if (auto* putMap = findOp(patch, PatchOp::Put)) {
    const auto& map = putMap->as_map();
    if (!map.empty()) {
      key = getValueAs(k, map.begin()->first);
      if (map.contains(*key)) {
        rwmask = {noneMask(), allMask()};
      }
    }
  }

  for (auto patchOp : {PatchOp::PatchPrior, PatchOp::PatchAfter}) {
    if (auto* itemPatch = findOp(patch, patchOp)) {
      const auto& map = itemPatch->as_map();
      if (!map.empty()) {
        if (!key.has_value()) {
          key = getValueAs(k, map.begin()->first);
        }
        if (map.contains(*key)) {
          rwmask = rwmask | extractMapMaskFromPatchImpl(map.at(*key), v);
        }
      }
    }
  }

  if (auto* ensureMap = findOp(patch, PatchOp::EnsureStruct)) {
    const auto& map = ensureMap->as_map();
    if (!map.empty()) {
      if (!key.has_value()) {
        key = getValueAs(k, map.begin()->first);
      }
      if (map.contains(*key)) {
        rwmask.read = rwmask.read & allMask();
        rwmask.write = rwmask.write & allMask();
      }
    }
  }
  return rwmask;
}

ExtractedMasksFromPatch extractMapMaskFromPatchImpl(
    const protocol::Object& patch, const Mask& mask) {
  if (isAllMask(mask)) {
    return extractMaskFromPatch(patch, true);
  }

  // If there is an assign or clear, we can short-circut the logic.
  if (findOp(patch, PatchOp::Assign)) {
    return {noneMask(), allMask()};
  }
  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (!isIntrinsicDefault(*clear)) {
      return {noneMask(), allMask()};
    }
  }

  ExtractedMasksFromPatch rwmask = {noneMask(), noneMask()};
  if (mask.includes()) {
    const auto& [id, v] = *mask.includes()->begin();
    // If there is a StructPatch::remove, we can short-circuit the logic.
    if (auto* value = findOp(patch, PatchOp::Remove)) {
      if (std::find(
              value->as_list().begin(),
              value->as_list().end(),
              asValueStruct<type::i16_t>(id)) != value->as_list().end()) {
        return {noneMask(), allMask()};
      }
    }

    for (auto patchOp : {PatchOp::PatchPrior, PatchOp::PatchAfter}) {
      if (auto* fieldPatch = findOp(patch, patchOp)) {
        if (fieldPatch->as_object().contains(FieldId{id})) {
          rwmask = rwmask |
              extractMapMaskFromPatchImpl(
                       fieldPatch->as_object().at(FieldId{id}), v);
        }
      }
    }

    if (auto* ensureStruct = findOp(patch, PatchOp::EnsureStruct)) {
      if (ensureStruct->as_object().contains(FieldId{id})) {
        rwmask.read = rwmask.read & allMask();
        rwmask.write = rwmask.write & allMask();
      }
    }
  } else if (mask.includes_map()) {
    const auto& [k, v] = *mask.includes_map()->begin();
    return extractMapMaskFromPatchMapImpl(patch, MapId{k}, v);
  } else if (mask.includes_string_map()) {
    const auto& [k, v] = *mask.includes_string_map()->begin();
    return extractMapMaskFromPatchMapImpl(patch, k, v);
  } else if (mask.includes_type()) {
    folly::throw_exception<std::runtime_error>("not implemented");
  } else {
    folly::throw_exception<std::runtime_error>("Invalid mask");
  }
  return rwmask;
}

ExtractedMasksFromPatch extractMapMaskFromPatchImpl(
    const protocol::Value& patch, const Mask& mask) {
  return extractMapMaskFromPatchImpl(patch.as_object(), mask);
}

} // namespace detail

ExtractedMasksFromPatch extractMaskFromPatch(const protocol::Object& patch) {
  detail::checkNotSafePatch(patch);
  return detail::extractMaskFromPatch(patch, false);
}

ExtractedMasksFromPatch extractMapMaskFromPatch_DO_NOT_USE(
    const protocol::Object& patch, const Mask& mask) {
  detail::validateSinglePath(mask);
  auto masks = detail::extractMapMaskFromPatchImpl(patch, mask);
  return masks;
}

template <type::StandardProtocol Protocol>
std::unique_ptr<folly::IOBuf> applyPatchToSerializedData(
    const protocol::Object& patch, const folly::IOBuf& buf) {
  detail::checkNotSafePatch(patch);
  // TODO: create method for this operation
  static_assert(
      Protocol == type::StandardProtocol::Binary ||
      Protocol == type::StandardProtocol::Compact);
  using ProtocolReader = std::conditional_t<
      Protocol == type::StandardProtocol::Binary,
      BinaryProtocolReader,
      CompactProtocolReader>;
  using ProtocolWriter = std::conditional_t<
      Protocol == type::StandardProtocol::Binary,
      BinaryProtocolWriter,
      CompactProtocolWriter>;
  auto masks = protocol::extractMaskFromPatch(patch);
  MaskedDecodeResult result =
      parseObject<ProtocolReader>(buf, masks.read, masks.write);
  applyPatch(patch, result.included);
  return serializeObject<ProtocolWriter>(result.included, result.excluded);
}

// Uses explicit instantiations to have the function definition in .cpp file.
template std::unique_ptr<folly::IOBuf>
applyPatchToSerializedData<type::StandardProtocol::Binary>(
    const protocol::Object& patch, const folly::IOBuf& buf);
template std::unique_ptr<folly::IOBuf>
applyPatchToSerializedData<type::StandardProtocol::Compact>(
    const protocol::Object& patch, const folly::IOBuf& buf);

Object fromSafePatch(const protocol::Object& safePatch) {
  const Value* version = safePatch.if_contains(detail::kSafePatchVersionId);
  const Value* data = safePatch.if_contains(detail::kSafePatchDataId);
  if (!(version && version->is_i32() && data && data->is_binary())) {
    throw std::runtime_error("Invalid safe patch");
  }
  if (version->as_i32() == 0) {
    throw std::runtime_error("Invalid safe patch");
  }
  if (version->as_i32() > op::detail::kThriftDynamicPatchVersion) {
    throw std::runtime_error(
        fmt::format("Unsupported patch version: {}", version->as_i32()));
  }
  Object patch =
      parseObject<CompactProtocolReader>(safePatch.at(FieldId{2}).as_binary());
  return patch;
}

Object toSafePatch(const protocol::Object& patch) {
  Object safePatch;
  safePatch[detail::kSafePatchVersionId].emplace_i32(
      detail::calculateMinSafePatchVersion(patch));
  safePatch[detail::kSafePatchDataId].emplace_binary(
      *serializeObject<CompactProtocolWriter>(patch));
  return safePatch;
}

} // namespace apache::thrift::protocol
