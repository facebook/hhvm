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
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp/util/SaturatingMath.h>
#include <thrift/lib/cpp/util/VarintUtils.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache {
namespace thrift {
namespace protocol {
namespace detail {
namespace {

using op::PatchOp;

template <typename Tag>
using value_field_id =
    type::field_id_tag<static_cast<FieldId>(type::base_type_v<Tag>)>;

template <typename Tag>
using value_native_type = op::get_native_type<value_field_id<Tag>, Value>;

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
    std::unordered_set<PatchOp> supportedOps) {
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

template <typename Tag>
decltype(auto) argAs(const Value& value) {
  using Id = type::field_id_tag<static_cast<FieldId>(type::base_type_v<Tag>)>;
  constexpr auto expected = static_cast<Value::Type>(Id::value);
  if (value.getType() != Value::Type(Id::value)) {
    folly::throw_exception<std::runtime_error>(fmt::format(
        "Unexpected type in the patch. Expected {} got {}",
        util::enumNameSafe<Value::Type>(expected),
        util::enumNameSafe<Value::Type>(value.getType())));
  }
  return *op::get<Id, Value>(value);
}

template <typename Tag>
bool applyAssign(const Object& patch, value_native_type<Tag>& value) {
  if (const Value* arg = findOp(patch, PatchOp::Assign)) {
    value = argAs<Tag>(*arg);
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
      return operator()(patch, *value.boolValue_ref());
    case Value::Type::byteValue:
      return operator()(patch, *value.byteValue_ref());
    case Value::Type::i16Value:
      return operator()(patch, *value.i16Value_ref());
    case Value::Type::i32Value:
      return operator()(patch, *value.i32Value_ref());
    case Value::Type::i64Value:
      return operator()(patch, *value.i64Value_ref());
    case Value::Type::floatValue:
      return operator()(patch, *value.floatValue_ref());
    case Value::Type::doubleValue:
      return operator()(patch, *value.doubleValue_ref());
    case Value::Type::stringValue: {
      auto binaryValue = folly::IOBuf::wrapBufferAsValue(
          value.stringValue_ref()->data(), value.stringValue_ref()->size());
      operator()(patch, binaryValue);
      value.stringValue_ref() = binaryValue.to<std::string>();
      return;
    }
    case Value::Type::binaryValue:
      return operator()(patch, *value.binaryValue_ref());
    case Value::Type::listValue:
      return operator()(patch, *value.listValue_ref());
    case Value::Type::setValue:
      return operator()(patch, *value.setValue_ref());
    case Value::Type::mapValue:
      return operator()(patch, *value.mapValue_ref());
    case Value::Type::objectValue:
      return operator()(patch, *value.objectValue_ref());
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
      clear != nullptr && *clear->boolValue_ref()) {
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
  checkOps(
      patch,
      Value::Type::listValue,
      {PatchOp::Assign,
       PatchOp::Clear,
       PatchOp::PatchPrior,
       PatchOp::Add,
       PatchOp::Put,
       PatchOp::Remove});
  if (applyAssign<type::list_c>(patch, value)) {
    return; // Ignore all other ops.
  }

  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value.clear();
    }
  }

  if (auto* elementPatches = findOp(patch, PatchOp::PatchPrior)) {
    const auto* indexPatches = elementPatches->if_map();
    if (!indexPatches) {
      throw std::runtime_error("list patch should contain a map");
    }

    for (const auto& [idx, elPatch] : *indexPatches) {
      const auto* indexPtr = idx.if_i32();
      if (!indexPtr) {
        throw std::runtime_error("expected index as i32");
      }

      auto index = type::toPosition(
          type::Ordinal(apache::thrift::util::zigzagToI32(*indexPtr)));
      if (index >= 0 && static_cast<size_t>(index) < value.size()) {
        applyPatch(*elPatch.objectValue_ref(), value[index]);
      }
    }
  }

  if (auto* remove = findOp(patch, PatchOp::Remove)) {
    if (!remove->is_set() && !remove->is_list()) {
      throw std::runtime_error(
          "list remove patch should contain a set or a list");
    }

    auto make_element_filter = [&]() -> std::function<bool(const Value&)> {
      if (const auto* to_remove = remove->if_set()) {
        return [to_remove](const auto& element) {
          return to_remove->find(element) != to_remove->end();
        };
      }
      const auto* to_remove = remove->if_list();
      return [to_remove](const auto& element) {
        return std::find(to_remove->begin(), to_remove->end(), element) !=
            to_remove->end();
      };
    };
    value.erase(
        std::remove_if(value.begin(), value.end(), make_element_filter()),
        value.end());
  }

  if (auto* add = findOp(patch, PatchOp::Add)) {
    if (const auto* to_add = add->if_set()) {
      for (const auto& element : *to_add) {
        if (std::find(value.begin(), value.end(), element) == value.end()) {
          value.insert(value.begin(), element);
        }
      }
    } else {
      const auto* prependVector = add->if_list();
      if (!prependVector) {
        throw std::runtime_error(
            "list add patch should contain a set or a list");
      }
      value.insert(value.begin(), prependVector->begin(), prependVector->end());
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

void ApplyPatch::operator()(const Object& patch, std::set<Value>& value) const {
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
          fmt::format("set {} patch should caontain a set", name));
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
    insert_set(*validate_if_set(put, "put"));
  }
}

void ApplyPatch::operator()(
    const Object& patch, std::map<Value, Value>& value) const {
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
        applyPatch(*valv.objectValue_ref(), *field);
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

void ApplyPatch::operator()(const Object& patch, Object& value) const {
  checkOps(
      patch,
      Value::Type::objectValue,
      {PatchOp::Assign,
       PatchOp::Clear,
       PatchOp::PatchPrior,
       PatchOp::EnsureStruct,
       PatchOp::EnsureUnion,
       PatchOp::PatchAfter,
       PatchOp::Add});
  if (applyAssign<type::struct_c>(patch, value)) {
    return; // Ignore all other ops.
  }

  if (auto* clear = findOp(patch, PatchOp::Clear)) {
    if (argAs<type::bool_t>(*clear)) {
      value.members().ensure().clear();
    }
  }

  auto applyFieldPatch = [&](auto patchFields) {
    value.members().ensure();
    const auto* obj = patchFields->if_object();
    if (!obj) {
      throw std::runtime_error(
          "struct patch PatchPrior/Patch should contain an object");
    }
    for (const auto& [id, field_value] : *obj->members()) {
      // Only patch values for fields that exist for now
      if (auto* field = folly::get_ptr(*value.members(), id)) {
        applyPatch(*field_value.objectValue_ref(), *field);
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
        value.members() = {{itr->first, std::move(itr->second)}};
      }
    }
  }

  if (auto* patchFields = findOp(patch, PatchOp::PatchAfter)) {
    applyFieldPatch(patchFields);
  }
  if (auto* addFields = findOp(patch, PatchOp::Add)) {
    // TODO(afuller): Implement field-wise add.
  }
}
// Inserts the next mask to getIncludesRef(mask)[id].
// Skips if mask is allMask (already includes all fields), or next is noneMask.
template <typename Id, typename F>
void insertMask(Mask& mask, Id id, const Mask& next, const F& getIncludesRef) {
  if (mask != allMask() && next != noneMask()) {
    getIncludesRef(mask).ensure()[id] = next;
  }
}

template <typename Id, typename F>
void insertNextMask(
    ExtractedMasks& masks,
    const Value& nextPatch,
    Id readId,
    Id writeId,
    bool recursive,
    const F& getIncludesRef) {
  if (recursive) {
    auto nextMasks = extractMaskFromPatch(nextPatch.as_object());
    insertMask(masks.read, readId, nextMasks.read, getIncludesRef);
    insertMask(masks.write, writeId, nextMasks.write, getIncludesRef);
  } else {
    insertMask(masks.read, readId, allMask(), getIncludesRef);
    insertMask(masks.write, writeId, allMask(), getIncludesRef);
  }
}

// if recursive, it constructs the mask from the patch object for the field.
void insertFieldsToMask(
    ExtractedMasks& masks, const Value& patchFields, bool recursive) {
  auto getIncludesMapRef = [&](Mask& mask) { return mask.includes_map_ref(); };
  auto removeHandler = [&](const auto* container) {
    for (const auto& key : *container) {
      auto readId = static_cast<int64_t>(findMapIdByValue(masks.read, key));
      auto writeId = static_cast<int64_t>(findMapIdByValue(masks.write, key));
      insertMask(masks.read, readId, allMask(), getIncludesMapRef);
      insertMask(masks.write, writeId, allMask(), getIncludesMapRef);
    }
  };

  if (const auto* obj = patchFields.if_object()) {
    for (const auto& [id, value] : *obj) {
      insertNextMask(masks, value, id, id, recursive, [&](Mask& mask) {
        return mask.includes_ref();
      });
    }
  } else if (const auto* map = patchFields.if_map()) {
    for (const auto& [key, value] : *map) {
      auto readId = static_cast<int64_t>(findMapIdByValue(masks.read, key));
      auto writeId = static_cast<int64_t>(findMapIdByValue(masks.write, key));
      insertNextMask(
          masks, value, readId, writeId, recursive, getIncludesMapRef);
    }
  } else if (const auto* set = patchFields.if_set()) {
    // set of map keys (Remove)
    removeHandler(set);
  } else if (const auto* list = patchFields.if_list()) {
    // list of map keys (Remove)
    removeHandler(list);
  }
}

// TODO: Handle EnsureUnion
ExtractedMasks extractMaskFromPatch(const protocol::Object& patch) {
  ExtractedMasks masks = {noneMask(), noneMask()};
  // If Assign, it is a write operation
  if (findOp(patch, PatchOp::Assign)) {
    masks = {noneMask(), allMask()};
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

  // Put is a read-write operation if not a map patch. Otherwise add keys to
  // mask.
  if (auto* value = findOp(patch, PatchOp::Put)) {
    if (value->mapValue_ref()) {
      insertFieldsToMask(masks, *value, false);
    } else if (!isIntrinsicDefault(*value)) {
      return {allMask(), allMask()};
    }
  }
  // Remove always adds keys to map mask. All types (list, set, and map) use
  // a set for Remove, so they are indistinguishable.
  if (auto* value = findOp(patch, PatchOp::Remove)) {
    insertFieldsToMask(masks, *value, false);
  }

  // If EnsureStruct, add the fields/ keys to mask
  if (auto* ensureStruct = findOp(patch, PatchOp::EnsureStruct)) {
    insertFieldsToMask(masks, *ensureStruct, false);
  }

  // If PatchPrior or PatchAfter, recursively constructs the mask for the
  // fields. Note that list also supports Patch, but since it is
  // indistinguishable from map (both uses a map), we just treat it as a map
  // patch.
  for (auto op : {PatchOp::PatchPrior, PatchOp::PatchAfter}) {
    if (auto* patchFields = findOp(patch, op)) {
      insertFieldsToMask(masks, *patchFields, true);
    }
  }

  return masks;
}

} // namespace detail

ExtractedMasks extractMaskFromPatch(const protocol::Object& patch) {
  return detail::extractMaskFromPatch(patch);
}

template <type::StandardProtocol Protocol>
std::unique_ptr<folly::IOBuf> applyPatchToSerializedData(
    const protocol::Object& patch, const folly::IOBuf& buf) {
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
} // namespace protocol
} // namespace thrift
} // namespace apache
