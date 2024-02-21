/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <folly/json/json.h>
#include <thrift/lib/cpp2/FieldRef.h>

#include "mcrouter/lib/carbon/CommonSerializationTraits.h"
#include "mcrouter/lib/carbon/Fields.h"
#include "mcrouter/lib/carbon/Keys.h"
#include "mcrouter/lib/carbon/Result.h"

namespace carbon {

namespace detail {

inline std::string getMixinName(folly::StringPiece name) {
  return folly::to<std::string>("__", name);
}

class ToDynamicVisitor {
 public:
  explicit ToDynamicVisitor(FollyDynamicConversionOptions opts)
      : value_(folly::dynamic::object()), opts_(opts) {}

  template <class T>
  bool enterMixin(uint16_t /* id */, folly::StringPiece name, const T& value) {
    if (!opts_.inlineMixins) {
      auto mixin = convertToFollyDynamic(value, opts_);
      if (mixin != nullptr && shouldSerialize(mixin)) {
        value_.insert(getMixinName(name), std::move(mixin));
      }
      return false;
    } else {
      return true;
    }
  }

  bool leaveMixin() {
    return true;
  }

  template <class T>
  bool visitField(uint16_t /* id */, folly::StringPiece name, const T& value) {
    auto val = toDynamic(value);
    if (val != nullptr && shouldSerialize(val)) {
      value_.insert(name, std::move(val));
    }
    return true;
  }

  /**
   * Obtain serialized output.
   */
  folly::dynamic moveOutput() {
    return std::move(value_);
  }

 private:
  folly::dynamic value_;
  FollyDynamicConversionOptions opts_;

  bool shouldSerialize(const folly::dynamic& val) {
    if (opts_.serializeFieldsWithDefaultValue) {
      return true;
    }

    if (val.isBool() && !val.getBool()) {
      return false;
    } else if (val.isInt() && val.getInt() == 0) {
      return false;
    } else if (val.isDouble() && val.getDouble() == 0.0) {
      return false;
    } else if (
        (val.isString() || val.isArray() || val.isObject()) && val.empty()) {
      return false;
    }
    return true;
  }

  folly::dynamic toDynamic(carbon::Result res) const {
    return folly::dynamic(carbon::resultToString(res));
  }

  folly::dynamic toDynamic(char c) const {
    return folly::dynamic(std::string(1, c));
  }

  folly::dynamic toDynamic(const folly::IOBuf& value) const {
    if (value.isChained()) {
      auto copy = value;
      return folly::StringPiece(copy.coalesce());
    } else {
      return folly::StringPiece(folly::ByteRange(value.data(), value.length()));
    }
  }

  template <class T>
  folly::dynamic toDynamic(const folly::Optional<T>& value) const {
    if (value.hasValue()) {
      return toDynamic(*value);
    }
    return nullptr;
  }

  template <class T>
  folly::dynamic toDynamic(
      const apache::thrift::optional_field_ref<T&> value) const& {
    if (value.has_value()) {
      return toDynamic(*value);
    }
    return nullptr;
  }

  template <class T>
  folly::dynamic toDynamic(const Keys<T>& value) const {
    return value.fullKey();
  }

  template <class T>
  typename std::enable_if<std::is_arithmetic<T>::value, folly::dynamic>::type
  toDynamic(const T& value) const {
    return value;
  }

  template <class T>
  typename std::enable_if<!std::is_arithmetic<T>::value, folly::dynamic>::type
  toDynamic(const T& value) const {
    return toDynamic2(value);
  }

  template <class T>
  typename std::enable_if<IsThriftWrapperStruct<T>::value, folly::dynamic>::type
  toDynamic2(const T& /* value */) const {
    return folly::dynamic("(thrift struct)");
  }

  template <class T>
  typename std::enable_if<!IsThriftWrapperStruct<T>::value, folly::dynamic>::
      type
      toDynamic2(const T& value) const {
    return toDynamic3(value);
  }

  template <class T>
  typename std::enable_if<IsCarbonStruct<T>::value, folly::dynamic>::type
  toDynamic3(const T& value) const {
    return convertToFollyDynamic(value, opts_);
  }

  template <class T>
  typename std::enable_if<!IsCarbonStruct<T>::value, folly::dynamic>::type
  toDynamic3(const T& value) const {
    return toDynamic4(value);
  }

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, folly::dynamic>::type
  toDynamic4(const T& value) const {
    return toDynamic(
        static_cast<typename std::underlying_type<T>::type>(value));
  }

  template <class T>
  typename std::enable_if<!std::is_enum<T>::value, folly::dynamic>::type
  toDynamic4(const T& value) const {
    return toDynamic5(value);
  }

  template <class T>
  typename std::enable_if<
      std::is_convertible<T, folly::StringPiece>::value,
      folly::dynamic>::type
  toDynamic5(const T& value) const {
    return folly::StringPiece(value);
  }

  template <class T>
  typename std::enable_if<
      !std::is_convertible<T, folly::StringPiece>::value,
      folly::dynamic>::type
  toDynamic5(const T& value) const {
    return toDynamic6(value);
  }

  template <class T>
  typename std::enable_if<IsLinearContainer<T>::value, folly::dynamic>::type
  toDynamic6(const T& value) const {
    folly::dynamic array = folly::dynamic::array();
    for (auto it = SerializationTraits<T>::begin(value);
         it != SerializationTraits<T>::end(value);
         ++it) {
      array.push_back(toDynamic(*it));
    }
    return array;
  }

  template <class T>
  typename std::enable_if<!IsLinearContainer<T>::value, folly::dynamic>::type
  toDynamic6(const T& value) const {
    return toDynamic7(value);
  }

  template <class T>
  typename std::enable_if<IsKVContainer<T>::value, folly::dynamic>::type
  toDynamic7(const T& m) const {
    folly::dynamic map = folly::dynamic::object();
    for (auto it = SerializationTraits<T>::begin(m);
         it != SerializationTraits<T>::end(m);
         ++it) {
      auto dynamicKey = toDynamic(it->first);
      auto dynamicVal = toDynamic(it->second);
      if (dynamicKey.isString()) {
        map[std::move(dynamicKey)] = std::move(dynamicVal);
      } else {
        map[folly::toJson(dynamicKey)] = std::move(dynamicVal);
      }
    }
    return map;
  }

  template <class T>
  typename std::enable_if<!IsKVContainer<T>::value, folly::dynamic>::type
  toDynamic7(const T& value) const {
    return toDynamic8(value);
  }

  template <class T>
  typename std::enable_if<
      std::is_convertible<T, folly::dynamic>::value,
      folly::dynamic>::type
  toDynamic8(const T& value) const {
    return value;
  }

  template <class T>
  typename std::enable_if<
      !std::is_convertible<T, folly::dynamic>::value,
      folly::dynamic>::type
  toDynamic8(const T& value) const {
    return toDynamic9(value);
  }

  template <class T>
  typename std::enable_if<IsUserReadWriteDefined<T>::value, folly::dynamic>::
      type
      toDynamic9(const T& /* value */) const {
    return "(user type)";
  }

  template <class T>
  typename std::enable_if<!IsUserReadWriteDefined<T>::value, folly::dynamic>::
      type
      toDynamic9(const T& /* value */) const {
    if (!opts_.ignoreUnserializableTypes) {
      return "(not serializable)";
    }
    return nullptr;
  }
};

class FromDynamicVisitor {
 public:
  explicit FromDynamicVisitor(
      const folly::dynamic& json,
      std::function<void(folly::StringPiece fieldName, folly::StringPiece msg)>
          onError)
      : json_(json), onError_(std::move(onError)) {}

  template <class T>
  bool enterMixin(uint16_t /* id */, folly::StringPiece name, T& value) {
    auto mixinName = getMixinName(name);
    if (auto jsonPtr = json_.get_ptr(mixinName)) {
      std::function<void(folly::StringPiece, folly::StringPiece)> onChildError;
      if (onError_) {
        onChildError = [this, &name](
                           folly::StringPiece field, folly::StringPiece msg) {
          onError(folly::to<std::string>(name, ".", field), msg);
        };
      }
      convertFromFollyDynamic(*jsonPtr, value, std::move(onChildError));
      return false;
    }
    return true;
  }

  bool leaveMixin() {
    return true;
  }

  template <class T>
  bool visitField(uint16_t /* id */, folly::StringPiece name, T& value) {
    if (auto jsonPtr = json_.get_ptr(name)) {
      fromDynamic(name, *jsonPtr, value);
    }
    return true;
  }

  template <class T>
  bool visitField(
      uint16_t /* id */,
      folly::StringPiece name,
      apache::thrift::optional_field_ref<T&> value) {
    if (auto jsonPtr = json_.get_ptr(name)) {
      fromDynamic(name, *jsonPtr, value);
    }
    return true;
  }

  template <class T, class F>
  bool visitUnionMember(folly::StringPiece fieldName, F&& emplaceFn) {
    if (auto jsonPtr = json_.get_ptr(fieldName)) {
      fromDynamic(fieldName, *jsonPtr, emplaceFn());
    }
    return true;
  }

 private:
  const folly::dynamic& json_;
  std::function<void(folly::StringPiece fieldName, folly::StringPiece msg)>
      onError_;

  void onError(folly::StringPiece fieldName, folly::StringPiece msg) {
    if (onError_) {
      onError_(fieldName, msg);
    }
  }

  bool fromDynamic(
      folly::StringPiece name,
      const folly::dynamic& json,
      bool& valRef) {
    if (!json.isBool()) {
      onError(name, "Invalid type. Bool expected.");
      return false;
    }
    valRef = json.asBool();
    return true;
  }

  bool fromDynamic(
      folly::StringPiece name,
      const folly::dynamic& json,
      char& valRef) {
    if (!json.isString()) {
      onError(name, "Invalid type. Char expected.");
      return false;
    }
    auto str = json.asString();
    if (str.size() != 1) {
      onError(name, "Invalid length. Expected string of length 1.");
      return false;
    }
    valRef = str[0];
    return true;
  }

  bool fromDynamic(
      folly::StringPiece name,
      const folly::dynamic& json,
      folly::IOBuf& valRef) {
    if (!json.isString()) {
      onError(name, "Invalid type. String expected.");
      return false;
    }
    valRef = folly::IOBuf(folly::IOBuf::COPY_BUFFER, json.asString());
    return true;
  }

  template <class T>
  bool fromDynamic(
      folly::StringPiece name,
      const folly::dynamic& json,
      folly::Optional<T>& valRef) {
    T val;
    if (fromDynamic(name, json, val)) {
      valRef.assign(std::move(val));
      return true;
    }
    return false;
  }

  template <class T>
  bool fromDynamic(
      folly::StringPiece name,
      const folly::dynamic& json,
      apache::thrift::optional_field_ref<T&> valRef) & {
    T val;
    if (fromDynamic(name, json, val)) {
      valRef = std::move(val);
      return true;
    }
    return false;
  }

  template <class T>
  bool fromDynamic(
      folly::StringPiece name,
      const folly::dynamic& json,
      Keys<T>& valRef) {
    if (!json.isString()) {
      onError(name, "Invalid type. String expected.");
      return false;
    }
    valRef = json.asString();
    return true;
  }

  template <class T>
  typename std::enable_if<std::is_integral<T>::value, bool>::type
  fromDynamic(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    if (!json.isInt()) {
      onError(name, "Invalid type. Int expected.");
      return false;
    }
    valRef = json.asInt();
    return true;
  }

  template <class T>
  typename std::enable_if<!std::is_integral<T>::value, bool>::type
  fromDynamic(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic2(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<std::is_floating_point<T>::value, bool>::type
  fromDynamic2(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    if (!json.isDouble()) {
      onError(name, "Invalid type. Double expected.");
      return false;
    }
    valRef = json.asDouble();
    return true;
  }

  template <class T>
  typename std::enable_if<!std::is_floating_point<T>::value, bool>::type
  fromDynamic2(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic3(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<IsThriftWrapperStruct<T>::value, bool>::type
  fromDynamic3(
      folly::StringPiece name,
      const folly::dynamic& /* json */,
      T& /* valRef */) {
    onError(name, "Could not deserialize thrift struct");
    return false;
  }

  template <class T>
  typename std::enable_if<!IsThriftWrapperStruct<T>::value, bool>::type
  fromDynamic3(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic4(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<IsCarbonUnion<T>::value, bool>::type
  fromDynamic4(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    size_t numChildErrors = 0;
    auto onChildError = [this, &name, &numChildErrors](
                            folly::StringPiece field, folly::StringPiece msg) {
      numChildErrors++;
      onError(folly::to<std::string>(name, ".", field), msg);
    };

    FromDynamicVisitor visitor(json, std::move(onChildError));
    valRef.foreachMember(visitor);
    return numChildErrors == 0;
  }

  template <class T>
  typename std::enable_if<!IsCarbonUnion<T>::value, bool>::type
  fromDynamic4(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic5(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<IsCarbonStruct<T>::value, bool>::type
  fromDynamic5(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    size_t numChildErrors = 0;
    auto onChildError = [this, &name, &numChildErrors](
                            folly::StringPiece field, folly::StringPiece msg) {
      numChildErrors++;
      onError(folly::to<std::string>(name, ".", field), msg);
    };
    convertFromFollyDynamic(json, valRef, std::move(onChildError));
    return numChildErrors == 0;
  }

  template <class T>
  typename std::enable_if<!IsCarbonStruct<T>::value, bool>::type
  fromDynamic5(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic6(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, bool>::type
  fromDynamic6(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    if (!json.isInt()) {
      onError(name, "Invalid type. Int expected.");
      return false;
    }
    valRef = static_cast<T>(json.asInt());
    return true;
  }

  template <class T>
  typename std::enable_if<!std::is_enum<T>::value, bool>::type
  fromDynamic6(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic7(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<
      std::is_convertible<T, folly::StringPiece>::value,
      bool>::type
  fromDynamic7(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    if (!json.isString()) {
      onError(name, "Invalid type. String expected.");
      return false;
    }
    valRef = json.asString();
    return true;
  }

  template <class T>
  typename std::enable_if<
      !std::is_convertible<T, folly::StringPiece>::value,
      bool>::type
  fromDynamic7(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic8(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<IsLinearContainer<T>::value, bool>::type
  fromDynamic8(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    if (!json.isArray()) {
      onError(name, "Invalid type. Array expected.");
      return false;
    }

    for (size_t i = 0; i < json.size(); ++i) {
      typename SerializationTraits<T>::inner_type val;
      if (fromDynamic(folly::sformat("{}[{}]", name, i), json[i], val)) {
        SerializationTraits<T>::emplace(valRef, std::move(val));
      }
    }
    return true;
  }

  template <class T>
  typename std::enable_if<!IsLinearContainer<T>::value, bool>::type
  fromDynamic8(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    return fromDynamic9(name, json, valRef);
  }

  template <class T>
  typename std::enable_if<IsKVContainer<T>::value, bool>::type
  fromDynamic9(folly::StringPiece name, const folly::dynamic& json, T& valRef) {
    if (!json.isObject()) {
      onError(name, "Invalid type. Map expected.");
      return false;
    }

    typename SerializationTraits<T>::key_type key;
    typename SerializationTraits<T>::mapped_type val;
    for (const auto& jsonKey : json.keys()) {
      if (!fromDynamic(folly::sformat("{}[{}]", name, jsonKey), jsonKey, key)) {
        continue;
      }
      if (!fromDynamic(
              folly::sformat("{}[{}]", name, jsonKey), json[jsonKey], val)) {
        continue;
      }
      SerializationTraits<T>::emplace(valRef, std::move(key), std::move(val));
    }
    return true;
  }

  template <class T>
  typename std::enable_if<!IsKVContainer<T>::value, bool>::type fromDynamic9(
      folly::StringPiece name,
      const folly::dynamic& /* json */,
      T& /* valRef */) {
    onError(name, "Unsupported type.");
    return false;
  }
};

} // namespace detail

template <class Message>
folly::dynamic convertToFollyDynamic(
    const Message& m,
    FollyDynamicConversionOptions opts) {
  detail::ToDynamicVisitor visitor(opts);
  m.visitFields(visitor);
  return visitor.moveOutput();
}

template <class Message>
void convertFromFollyDynamic(
    const folly::dynamic& json,
    Message& m,
    std::function<void(folly::StringPiece fieldName, folly::StringPiece msg)>
        onError) {
  detail::FromDynamicVisitor visitor(json, std::move(onError));
  m.visitFields(visitor);
}

} // namespace carbon
