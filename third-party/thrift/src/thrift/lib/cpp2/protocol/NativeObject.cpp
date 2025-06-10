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

#include <thrift/lib/cpp2/protocol/NativeObject.h>

namespace apache::thrift::protocol::experimental {

using Bool = PrimitiveTypes::Bool;
using I8 = PrimitiveTypes::I8;
using I16 = PrimitiveTypes::I16;
using I32 = PrimitiveTypes::I32;
using I64 = PrimitiveTypes::I64;
using Float = PrimitiveTypes::Float;
using Double = PrimitiveTypes::Double;

// ---- ValueHolder ---- //

ValueHolder::ValueHolder(const NativeValue& val) : data_{} {
  static_cast<NativeValue&>(*this) = val;
}

ValueHolder::ValueHolder(const ValueHolder& val) : data_{} {
  static_cast<NativeValue&>(*this) = static_cast<const NativeValue&>(val);
}

ValueHolder::ValueHolder(NativeValue&& val) noexcept : data_{} {
  static_cast<NativeValue&>(*this) = std::move(val);
}

ValueHolder::ValueHolder(ValueHolder&& val) noexcept
    : data_{std::move(val.data_)} {
  val.data_ = {};
}

NativeValue& ValueHolder::as_value() noexcept {
  return reinterpret_cast<NativeValue&>(data_);
}

const NativeValue& ValueHolder::as_value() const noexcept {
  return reinterpret_cast<const NativeValue&>(data_);
}

ValueHolder& ValueHolder::operator=(const ValueHolder& val) {
  static_cast<NativeValue&>(*this) = static_cast<const NativeValue&>(val);
  return *this;
}

ValueHolder& ValueHolder::operator=(ValueHolder&& val) noexcept {
  data_ = std::move(val.data_);
  val.data_ = {};
  return *this;
}

ValueHolder::~ValueHolder() {
  // TODO(sadroeck) - Invalidate data_ by putting sentinal value as last byte ?
  // Note: this should always be the std::variant index
  if (data_ != decltype(data_){}) {
    static_cast<NativeValue&>(*this).~NativeValue();
  }
}

bool ValueHolder::operator==(const ValueHolder& other) const {
  return static_cast<const NativeValue&>(*this) ==
      static_cast<const NativeValue&>(other);
}

bool ValueHolder::operator!=(const ValueHolder& other) const {
  return !(*this == other);
}

// ---- Value ---- //

NativeValue::NativeValue(const NativeValue& other) : kind_(other.kind_) {}
NativeValue::NativeValue(const NativeValue::Kind& kind) : kind_(kind) {}

NativeValue::NativeValue() noexcept : kind_(std::monostate{}) {}
NativeValue::NativeValue(NativeValue::Kind&& kind) noexcept
    : kind_(std::move(kind)) {}
NativeValue::NativeValue(Bool&& b) noexcept : kind_(std::move(b)) {}
NativeValue::NativeValue(I8&& i) noexcept : kind_(std::move(i)) {}
NativeValue::NativeValue(I16&& i) noexcept : kind_(std::move(i)) {}
NativeValue::NativeValue(I32&& i) noexcept : kind_(std::move(i)) {}
NativeValue::NativeValue(I64&& i) noexcept : kind_(std::move(i)) {}
NativeValue::NativeValue(Float&& f) noexcept : kind_(std::move(f)) {}
NativeValue::NativeValue(Double&& d) noexcept : kind_(std::move(d)) {}
NativeValue::NativeValue(Bytes&& s) noexcept : kind_(std::move(s)) {}
NativeValue::NativeValue(NativeList&& list) noexcept : kind_(std::move(list)) {}
NativeValue::NativeValue(NativeSet&& set) noexcept : kind_(std::move(set)) {}
NativeValue::NativeValue(NativeMap&& map) noexcept : kind_(std::move(map)) {}
NativeValue::NativeValue(NativeObject&& strct) noexcept
    : kind_(std::move(strct)) {}

NativeValue::NativeValue(const Bool& b) : kind_(b) {}
NativeValue::NativeValue(const I8& i8) : kind_(i8) {}
NativeValue::NativeValue(const I16& i16) : kind_(i16) {}
NativeValue::NativeValue(const I32& i32) : kind_(i32) {}
NativeValue::NativeValue(const I64& i64) : kind_(i64) {}
NativeValue::NativeValue(const Float& f) : kind_(f) {}
NativeValue::NativeValue(const Double& d) : kind_(d) {}
NativeValue::NativeValue(const Bytes& b) : kind_(b) {}
NativeValue::NativeValue(const NativeList& list) : kind_(list) {}
NativeValue::NativeValue(const NativeSet& set) : kind_(set) {}
NativeValue::NativeValue(const NativeMap& map) : kind_(map) {}
NativeValue::NativeValue(const NativeObject& strct) : kind_(strct) {}

const NativeValue::Kind& NativeValue::inner() const noexcept {
  return kind_;
}
NativeValue::Kind& NativeValue::inner() noexcept {
  return kind_;
}

bool NativeValue::is_empty() const noexcept {
  return std::holds_alternative<std::monostate>(kind_);
}

// ---- NativeList ---- //

const NativeList::Kind& NativeList::inner() const noexcept {
  return kind_;
}
std::size_t NativeList::size() const noexcept {
  return folly::variant_match(
      kind_,
      [](const std::monostate&) -> std::size_t { return 0; },
      [](const auto& list) { return list.size(); });
}
bool NativeList::empty() const noexcept {
  return folly::variant_match(
      kind_,
      [](const std::monostate&) -> bool { return true; },
      [](const auto& list) { return list.empty(); });
}
bool NativeList::operator==(const NativeList& other) const {
  return kind_ == other.kind_;
}
bool NativeList::operator!=(const NativeList& other) const {
  return !(*this == other);
}

NativeList::NativeList(const NativeList& other) : kind_(other.kind_) {}

// ---- NativeSet ---- //

NativeSet::NativeSet(const NativeSet& other) : kind_(other.kind_) {}

NativeSet::NativeSet(Kind&& kind) : kind_(std::move(kind)) {}
const NativeSet::Kind& NativeSet::inner() const {
  return kind_;
}
std::size_t NativeSet::size() const noexcept {
  return folly::variant_match(
      kind_,
      [](const std::monostate&) -> std::size_t { return 0; },
      [](const auto& set) { return set.size(); });
}
bool NativeSet::empty() const noexcept {
  return folly::variant_match(
      kind_,
      [](const std::monostate&) -> bool { return true; },
      [](const auto& set) { return set.empty(); });
}
bool NativeSet::contains(const NativeValue& value) const noexcept {
  return folly::variant_match(
      kind_,
      [](const std::monostate&) -> bool { return false; },
      [&](const SetOf<ValueHolder>& set) { return set.contains(value); },
      [&](const SetOf<NativeObject>& set) {
        if (const NativeObject* val_ptr = value.if_type<NativeObject>()) {
          return set.contains(*val_ptr);
        } else {
          return false;
        }
      },
      [&](const auto& set) {
        using SetElemTy = typename std::remove_cvref_t<decltype(set)>::key_type;
        if (auto* val_ptr = value.if_type<SetElemTy>()) {
          return set.contains(*val_ptr);
        } else {
          return false;
        }
      });
}
bool NativeSet::operator==(const NativeSet& other) const {
  return kind_ == other.kind_;
}
bool NativeSet::operator!=(const NativeSet& other) const {
  return !(*this == other);
}

// ---- NativeMap ---- //

NativeMap::NativeMap(const NativeMap& other) : kind_(other.kind_) {}

bool NativeMap::operator==(const NativeMap& other) const {
  return kind_ == other.kind_;
}
bool NativeMap::operator!=(const NativeMap& other) const {
  return !(kind_ == other.kind_);
}
const NativeMap::Kind& NativeMap::inner() const {
  return kind_;
}
std::size_t NativeMap::size() const noexcept {
  return folly::variant_match(
      kind_,
      [](const std::monostate&) -> std::size_t { return 0; },
      [](const auto& map) { return map.size(); });
}
bool NativeMap::empty() const noexcept {
  return folly::variant_match(
      kind_,
      [](const std::monostate&) -> bool { return true; },
      [](const auto& map) { return map.empty(); });
}
NativeMap::NativeMap(Kind&& kind) noexcept : kind_(std::move(kind)) {}

NativeMap::NativeMap(const Kind& kind) : kind_(kind) {}

bool NativeMap::contains(const NativeValue& key) const noexcept {
  return visit(
      [](const std::monostate&) { return false; },
      [&](const NativeMap::Generic& map) { return map.contains(key); },
      [&]<typename K>(const MapOf<K, ValueHolder>& map) {
        if (auto* key_ptr = key.if_type<K>()) {
          return map.contains(*key_ptr);
        } else {
          return false;
        }
      });
}

bool detail::native_map_emplace(
    NativeMap& map, NativeValue&& key, NativeValue&& val) {
  if (auto* generic_map = map.template if_type<NativeMap::Generic>()) {
    return generic_map->emplace(std::move(key), std::move(val)).second;
  }

  return key.visit(
      [](std::monostate) -> bool {
        throw std::runtime_error("Cannot insert an empty key into a map");
      },
      [&](auto&& k) {
        return val.visit(
            [](std::monostate) -> bool {
              throw std::runtime_error(
                  "Cannot insert an empty value into a map");
            },
            [&](auto&& v) {
              return detail::native_map_emplace(
                  map, std::move(k), std::move(v));
            });
      });
}

bool detail::native_map_insert_or_assign(
    NativeMap& map, NativeValue&& key, NativeValue&& val) {
  if (auto* generic_map = map.template if_type<NativeMap::Generic>()) {
    return generic_map->insert_or_assign(std::move(key), std::move(val)).second;
  }

  return key.visit(
      [](std::monostate) -> bool {
        throw std::runtime_error("Cannot insert an empty key into a map");
      },
      [&](auto&& k) {
        return val.visit(
            [](std::monostate) -> bool {
              throw std::runtime_error(
                  "Cannot insert an empty value into a map");
            },
            [&](auto&& v) {
              return detail::native_map_insert_or_assign(
                  map, std::move(k), std::move(v));
            });
      });
}

// ------- parsing functions ------- //

template <typename Protocol, typename T>
T read_primitive_as(Protocol& prot) {
  if constexpr (std::is_same_v<T, Bool>) {
    Bool b;
    prot.readBool(b);
    return b;
  } else if constexpr (std::is_same_v<T, I8>) {
    I8 i;
    prot.readByte(i);
    return i;
  } else if constexpr (std::is_same_v<T, I16>) {
    I16 i;
    prot.readI16(i);
    return i;
  } else if constexpr (std::is_same_v<T, I32>) {
    I32 i;
    prot.readI32(i);
    return i;
  } else if constexpr (std::is_same_v<T, I64>) {
    I64 i;
    prot.readI64(i);
    return i;
  } else if constexpr (std::is_same_v<T, Float>) {
    Float f;
    prot.readFloat(f);
    return f;
  } else if constexpr (std::is_same_v<T, Double>) {
    Double d;
    prot.readDouble(d);
    return d;
  } else if constexpr (std::is_same_v<T, Bytes>) {
    Bytes b;
    prot.readBinary(b.buf_);
    return b;
  } else {
    static_assert(false, "Unhandled primitive type");
  }
}

template <typename Protocol>
NativeList read_list(Protocol& prot);

template <typename Protocol, typename T>
ListOf<T> read_list_as(Protocol& prot, std::uint32_t size);

template <typename Protocol, typename T>
ListOf<ValueHolder> read_value_list_as(Protocol& prot, std::uint32_t size);

template <typename Protocol>
NativeSet read_set(Protocol& prot);

template <typename Protocol, typename T>
SetOf<T> read_set_as(Protocol& prot, std::uint32_t size);

template <typename Protocol, typename T>
SetOf<ValueHolder> read_value_set_as(Protocol& prot, std::uint32_t size);

template <typename Protocol>
NativeMap read_map(Protocol& prot);

template <typename Protocol>
NativeMap read_kv_map(
    Protocol& prot, TType keyType, TType valType, std::uint32_t size);

template <typename Protocol, typename Key>
NativeMap read_v_map_as(Protocol& prot, TType valType, std::uint32_t size);

template <typename Protocol, typename Key, typename Value>
detail::map_t<Key, Value> read_map_as(Protocol& prot, std::uint32_t size);

template <typename Protocol>
NativeObject read_struct(Protocol& prot);

constexpr bool is_primitive(TType type) {
  switch (type) {
    case protocol::T_BOOL:
    case protocol::T_BYTE:
    case protocol::T_I16:
    case protocol::T_I32:
    case protocol::T_I64:
    case protocol::T_DOUBLE:
    case protocol::T_FLOAT:
    case protocol::T_STRING:
      return true;
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_STRUCT:
    case T_MAP:
    case T_SET:
    case T_LIST:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      return false;
  }
}

template <typename Protocol, typename T>
ListOf<T> read_list_as(Protocol& prot, std::uint32_t size) {
  ListOf<T> list{};
  list.reserve(size);
  for (std::uint32_t i = 0; i < size; ++i) {
    if constexpr (std::is_same_v<T, NativeObject>) {
      list.emplace_back(read_struct<Protocol>(prot));
    } else if constexpr (detail::is_primitive_v<T>) {
      list.emplace_back(read_primitive_as<Protocol, T>(prot));
    } else {
      static_assert(false, "Invalid list specialization");
    }
  }
  return list;
}

template <typename Protocol, typename T>
ListOf<ValueHolder> read_value_list_as(Protocol& prot, std::uint32_t size) {
  ListOf<ValueHolder> list;
  list.reserve(size);
  for (std::uint32_t i = 0; i < size; ++i) {
    if constexpr (std::is_same_v<T, NativeList>) {
      list.emplace_back(NativeValue(read_list<Protocol>(prot)));
    } else if constexpr (std::is_same_v<T, NativeSet>) {
      list.emplace_back(NativeValue(read_set<Protocol>(prot)));
    } else if constexpr (std::is_same_v<T, NativeMap>) {
      list.emplace_back(NativeValue{read_map<Protocol>(prot)});
    } else {
      static_assert(false, "Missing value list specialization");
    }
  }
  return list;
}

template <typename Protocol>
NativeList read_list(Protocol& prot) {
  TType elemType{};
  std::uint32_t size{};
  prot.readListBegin(elemType, size);
  if (!canReadNElements(prot, size, {elemType})) {
    TProtocolException::throwTruncatedData();
  }
  SCOPE_SUCCESS {
    prot.readListEnd();
  };

  switch (elemType) {
    case TType::T_BOOL: {
      return read_list_as<Protocol, Bool>(prot, size);
    }
    case TType::T_BYTE: {
      return read_list_as<Protocol, I8>(prot, size);
    }
    case TType::T_I16: {
      return read_list_as<Protocol, I16>(prot, size);
    }
    case TType::T_I32: {
      return read_list_as<Protocol, I32>(prot, size);
    }
    case TType::T_I64: {
      return read_list_as<Protocol, I64>(prot, size);
    }
    case TType::T_DOUBLE: {
      return read_list_as<Protocol, Double>(prot, size);
    }
    case TType::T_FLOAT: {
      return read_list_as<Protocol, Float>(prot, size);
    }
    case TType::T_STRING: {
      return read_list_as<Protocol, Bytes>(prot, size);
    }
    case TType::T_STRUCT: {
      return read_list_as<Protocol, NativeObject>(prot, size);
    }
    case TType::T_LIST: {
      return read_value_list_as<Protocol, NativeList>(prot, size);
    }
    case TType::T_SET: {
      return read_value_list_as<Protocol, NativeSet>(prot, size);
      break;
    }
    case TType::T_MAP: {
      return read_value_list_as<Protocol, NativeMap>(prot, size);
    }
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      TProtocolException::throwInvalidSkipType(elemType);
      break;
  }
}

template <typename Protocol, typename T>
SetOf<T> read_set_as(Protocol& prot, std::uint32_t size) {
  SetOf<T> set;
  set.reserve(size);
  for (std::uint32_t i = 0; i < size; ++i) {
    if constexpr (std::is_same_v<T, NativeObject>) {
      set.insert(read_struct<Protocol>(prot));
    } else {
      static_assert(detail::is_primitive_v<T>);
      set.insert(read_primitive_as<Protocol, T>(prot));
    }
  }
  return set;
}

template <typename Protocol, typename T>
SetOf<ValueHolder> read_value_set_as(Protocol& prot, std::uint32_t size) {
  SetOf<ValueHolder> set;
  set.reserve(size);
  for (std::uint32_t i = 0; i < size; ++i) {
    if constexpr (std::is_same_v<T, NativeList>) {
      set.insert(NativeValue{read_list<Protocol>(prot)});
    } else if constexpr (std::is_same_v<T, NativeSet>) {
      set.insert(NativeValue{read_set<Protocol>(prot)});
    } else if constexpr (std::is_same_v<T, NativeMap>) {
      set.insert(NativeValue{read_map<Protocol>(prot)});
    } else {
      static_assert(false, "Missing value set specialization");
    }
  }
  return set;
}

template <typename Protocol>
NativeSet read_set(Protocol& prot) {
  TType elemType{};
  uint32_t size{};
  prot.readSetBegin(elemType, size);
  if (!canReadNElements(prot, size, {elemType})) {
    TProtocolException::throwTruncatedData();
  }
  SCOPE_SUCCESS {
    prot.readSetEnd();
  };

  switch (elemType) {
    case TType::T_BOOL: {
      return read_set_as<Protocol, Bool>(prot, size);
    }
    case TType::T_BYTE: {
      return read_set_as<Protocol, I8>(prot, size);
    }
    case TType::T_I16: {
      return read_set_as<Protocol, I16>(prot, size);
    }
    case TType::T_I32: {
      return read_set_as<Protocol, I32>(prot, size);
    }
    case TType::T_I64: {
      return read_set_as<Protocol, I64>(prot, size);
    }
    case TType::T_DOUBLE: {
      return read_set_as<Protocol, Double>(prot, size);
    }
    case TType::T_FLOAT: {
      return read_set_as<Protocol, Float>(prot, size);
    }
    case TType::T_STRING: {
      return read_set_as<Protocol, Bytes>(prot, size);
    }
    case TType::T_STRUCT: {
      return read_set_as<Protocol, NativeObject>(prot, size);
    }
    case TType::T_LIST: {
      return read_value_set_as<Protocol, NativeList>(prot, size);
    }
    case TType::T_SET: {
      return read_value_set_as<Protocol, NativeSet>(prot, size);
    }
    case TType::T_MAP: {
      return read_value_set_as<Protocol, NativeMap>(prot, size);
    }
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      TProtocolException::throwInvalidSkipType(elemType);
      break;
  }
}

template <typename Protocol, typename K, typename V>
detail::map_t<K, V> read_map_as(Protocol& prot, std::uint32_t size) {
  detail::map_t<K, V> map{};
  map.reserve(size);

  using MapKeyTy = typename detail::map_t<K, V>::key_type;
  using MapValueTy = typename detail::map_t<K, V>::mapped_type;

  auto readKey = [&]() -> MapKeyTy {
    if constexpr (std::is_same_v<MapKeyTy, detail::native_value_type_t<K>>) {
      static_assert(detail::is_primitive_v<K>);
      return read_primitive_as<Protocol, detail::native_value_type_t<K>>(prot);
    } else {
      static_assert(std::is_same_v<MapKeyTy, ValueHolder>);
      if constexpr (detail::is_primitive_v<K>) {
        return NativeValue{read_primitive_as<Protocol, K>(prot)};
      } else {
        return NativeValue{read_struct<Protocol>(prot)};
      }
    }
  };

  auto readValue = [&]() -> MapValueTy {
    if constexpr (std::is_same_v<MapValueTy, NativeObject>) {
      return read_struct<Protocol>(prot);
    } else {
      static_assert(std::is_same_v<MapValueTy, ValueHolder>);
      if constexpr (detail::is_primitive_v<V>) {
        return NativeValue{read_primitive_as<Protocol, V>(prot)};
      } else {
        return NativeValue{read_struct<Protocol>(prot)};
      }
    }
  };

  for (uint32_t i = 0; i < size; ++i) {
    map.emplace(readKey(), readValue());
  }
  return map;
}

template <typename Protocol, typename Key>
NativeMap read_v_map_as(Protocol& prot, TType valType, std::uint32_t size) {
  switch (valType) {
    case TType::T_BOOL: {
      return read_map_as<Protocol, Key, Bool>(prot, size);
    }
    case TType::T_BYTE: {
      return read_map_as<Protocol, Key, I8>(prot, size);
    }
    case TType::T_I16: {
      return read_map_as<Protocol, Key, I16>(prot, size);
    }
    case TType::T_I32: {
      return read_map_as<Protocol, Key, I32>(prot, size);
    }
    case TType::T_I64: {
      return read_map_as<Protocol, Key, I64>(prot, size);
    }
    case TType::T_FLOAT: {
      return read_map_as<Protocol, Key, Float>(prot, size);
    }
    case TType::T_DOUBLE: {
      return read_map_as<Protocol, Key, Double>(prot, size);
    }
    case TType::T_STRING: {
      return read_map_as<Protocol, Key, Bytes>(prot, size);
    }
    case TType::T_STRUCT: {
      return read_map_as<Protocol, Key, NativeObject>(prot, size);
    }
    case TType::T_LIST: {
      return read_map_as<Protocol, Key, NativeList>(prot, size);
    }
    case TType::T_SET: {
      return read_map_as<Protocol, Key, NativeSet>(prot, size);
    }
    case TType::T_MAP: {
      return read_map_as<Protocol, Key, NativeMap>(prot, size);
    }
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      TProtocolException::throwInvalidSkipType(valType);
      break;
  }
}

template <typename Protocol>
NativeMap read_kv_map(
    Protocol& prot, TType keyType, TType valType, std::uint32_t size) {
  switch (keyType) {
    case TType::T_BOOL: {
      return read_v_map_as<Protocol, Bool>(prot, valType, size);
    }
    case TType::T_BYTE: {
      return read_v_map_as<Protocol, I8>(prot, valType, size);
    }
    case TType::T_I16: {
      return read_v_map_as<Protocol, I16>(prot, valType, size);
    }
    case TType::T_I32: {
      return read_v_map_as<Protocol, I32>(prot, valType, size);
    }
    case TType::T_I64: {
      return read_v_map_as<Protocol, I64>(prot, valType, size);
    }
    case TType::T_FLOAT: {
      return read_v_map_as<Protocol, Float>(prot, valType, size);
    }
    case TType::T_DOUBLE: {
      return read_v_map_as<Protocol, Float>(prot, valType, size);
    }
    case TType::T_STRING: {
      return read_v_map_as<Protocol, Bytes>(prot, valType, size);
    }
    case TType::T_STRUCT: {
      return read_v_map_as<Protocol, NativeObject>(prot, valType, size);
    }
    case TType::T_LIST: {
      return read_v_map_as<Protocol, NativeList>(prot, valType, size);
    }
    case TType::T_SET: {
      return read_v_map_as<Protocol, NativeSet>(prot, valType, size);
    }
    case TType::T_MAP: {
      return read_v_map_as<Protocol, NativeMap>(prot, valType, size);
    }
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      TProtocolException::throwInvalidSkipType(keyType);
      break;
  }
}

template <typename Protocol>
NativeMap read_map(Protocol& prot) {
  TType keyType{};
  TType valType{};
  uint32_t size{};

  prot.readMapBegin(keyType, valType, size);
  if (!canReadNElements(prot, size, {keyType, valType})) {
    TProtocolException::throwTruncatedData();
  }
  SCOPE_SUCCESS {
    prot.readMapEnd();
  };
  return read_kv_map<Protocol>(prot, keyType, valType, size);
}
template <typename... Args>
auto make_value(Args&&... args) {
  return ValueHolder{NativeValue(std::forward<Args>(args)...)};
}

template <typename Protocol>
NativeValue read_value_as(Protocol& prot, TType ttype) {
  switch (ttype) {
    case T_BOOL: {
      return make_value(read_primitive_as<Protocol, Bool>(prot));
    }
    case T_BYTE: {
      return make_value(read_primitive_as<Protocol, I8>(prot));
    }
    case T_I16: {
      return make_value(read_primitive_as<Protocol, I16>(prot));
    }
    case T_I32: {
      return make_value(read_primitive_as<Protocol, I32>(prot));
    }
    case T_I64: {
      return make_value(read_primitive_as<Protocol, I64>(prot));
    }
    case T_FLOAT: {
      return make_value(read_primitive_as<Protocol, Float>(prot));
    }
    case T_DOUBLE: {
      return make_value(read_primitive_as<Protocol, Double>(prot));
    }
    case T_STRING: {
      return make_value(read_primitive_as<Protocol, Bytes>(prot));
    }
    case protocol::T_STRUCT: {
      return make_value(read_struct<Protocol>(prot));
    }
    case protocol::T_LIST: {
      return make_value(read_list<Protocol>(prot));
    }
    case protocol::T_SET: {
      return make_value(read_set<Protocol>(prot));
    }
    case protocol::T_MAP: {
      return make_value(read_map<Protocol>(prot));
    }
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      TProtocolException::throwInvalidSkipType(ttype);
      break;
  }
}

template <typename Protocol>
void read_struct_field(
    Protocol& prot,
    TType field_type,
    NativeObject& strct,
    NativeObject::FieldId id) {
  strct.emplace(id, read_value_as(prot, field_type));
}

template <class Protocol>
NativeObject read_struct(Protocol& prot) {
  std::string name;
  int16_t fid{};
  TType ftype{};
  NativeObject strct{};
  prot.readStructBegin(name);
  while (true) {
    prot.readFieldBegin(name, ftype, fid);
    if (ftype == protocol::T_STOP) {
      break;
    }
    read_struct_field<Protocol>(prot, ftype, strct, fid);
    prot.readFieldEnd();
  }
  prot.readStructEnd();
  return strct;
}

NativeObject detail::parseObjectVia(
    ::apache::thrift::BinaryProtocolReader& prot, bool string_to_binary) {
  if (string_to_binary) {
    return read_struct<::apache::thrift::BinaryProtocolReader>(prot);
  } else {
    return read_struct<::apache::thrift::BinaryProtocolReader>(prot);
  }
}

NativeObject detail::parseObjectVia(
    ::apache::thrift::CompactProtocolReader& prot, bool string_to_binary) {
  if (string_to_binary) {
    return read_struct<::apache::thrift::CompactProtocolReader>(prot);
  } else {
    return read_struct<::apache::thrift::CompactProtocolReader>(prot);
  }
}

NativeValue detail::parseValueVia(
    ::apache::thrift::BinaryProtocolReader& prot, protocol::TType ttype) {
  return read_value_as(prot, ttype);
}

NativeValue detail::parseValueVia(
    ::apache::thrift::CompactProtocolReader& prot, protocol::TType ttype) {
  return read_value_as(prot, ttype);
}

// ---- Serialization ---- //

#define PROTOTYPE_WRITE(CLASS) \
  template <typename Protocol> \
  std::uint32_t write(Protocol& prot, const CLASS& val);

PROTOTYPE_WRITE(Bool)
PROTOTYPE_WRITE(I8)
PROTOTYPE_WRITE(I16)
PROTOTYPE_WRITE(I32)
PROTOTYPE_WRITE(I64)
PROTOTYPE_WRITE(Float)
PROTOTYPE_WRITE(Double)
PROTOTYPE_WRITE(Bytes)
PROTOTYPE_WRITE(NativeObject)
PROTOTYPE_WRITE(NativeList)
PROTOTYPE_WRITE(NativeSet)
PROTOTYPE_WRITE(NativeMap)

template <typename Protocol, typename... Args>
std::uint32_t write(Protocol& prot, const ListOf<Args...>& list);
template <typename Protocol, typename... Args>
std::uint32_t write(Protocol& prot, const SetOf<Args...>& set);
template <typename Protocol, typename... Args>
std::uint32_t write(Protocol& prot, const MapOf<Args...>& map);
template <typename Protocol>
std::uint32_t write(Protocol&, const std::monostate&) {
  return 0;
}

#define WRITE_IMPL_VIA(CLASS, METHOD)                     \
  template <typename Protocol>                            \
  std::uint32_t write(Protocol& prot, const CLASS& val) { \
    return prot.METHOD(val);                              \
  }

WRITE_IMPL_VIA(Bool, writeBool)
WRITE_IMPL_VIA(I8, writeByte)
WRITE_IMPL_VIA(I16, writeI16)
WRITE_IMPL_VIA(I32, writeI32)
WRITE_IMPL_VIA(I64, writeI64)
WRITE_IMPL_VIA(Float, writeFloat)
WRITE_IMPL_VIA(Double, writeDouble)

template <typename Protocol>
std::uint32_t write(Protocol& prot, const Bytes& val) {
  return prot.writeBinary(
      folly::ByteRange{val.data(), val.data() + val.size()});
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeObject& obj) {
  uint32_t serializedSize = 0;
  serializedSize += prot.writeStructBegin("");
  for (const auto& [field_id, field_val] : obj) {
    auto fieldType = value_type_into_ttype(field_val.get_type());
    serializedSize += prot.writeFieldBegin("", fieldType, field_id);
    serializedSize += write(prot, field_val);
    serializedSize += prot.writeFieldEnd();
  }
  serializedSize += prot.writeFieldStop();
  serializedSize += prot.writeStructEnd();
  return serializedSize;
}

inline void ensure_same_type(const NativeValue& a, ValueType b) {
  if (a.get_type() != b) {
    TProtocolException::throwInvalidFieldData();
  }
}

// Default ttype for empty containers := I64
constexpr ValueType DEFAULT_CONTAINER_VALUE_TYPE = ValueType::I64;

template <typename Protocol>
std::uint32_t write(Protocol& prot, const ListOf<ValueHolder>& list) {
  uint32_t serializedSize = 0;
  const ValueType elementType =
      list.size() > 0 ? list[0].get_type() : DEFAULT_CONTAINER_VALUE_TYPE;
  serializedSize +=
      prot.writeListBegin(value_type_into_ttype(elementType), list.size());
  for (const auto& elem : list) {
    ensure_same_type(elem, elementType);
    serializedSize += write(prot, elem);
  }
  serializedSize += prot.writeListEnd();
  return serializedSize;
}

template <typename Protocol, typename... Args>
std::uint32_t write(Protocol& prot, const ListOf<Args...>& list) {
  using ListElemTy = typename std::remove_cvref_t<decltype(list)>::value_type;
  constexpr TType value_ttag =
      op::typeTagToTType<typename detail::native_value_type<ListElemTy>::tag>;
  uint32_t serializedSize = 0;
  serializedSize += prot.writeListBegin(value_ttag, list.size());
  for (const auto& elem : list) {
    serializedSize += write(prot, elem);
  }
  serializedSize += prot.writeListEnd();
  return serializedSize;
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeList& list) {
  return folly::variant_match(
      list.inner(),
      [&](const std::monostate&) {
        // Note: Write an empty list with a default tag of I64
        return prot.writeListBegin(
                   value_type_into_ttype(DEFAULT_CONTAINER_VALUE_TYPE), 0) +
            prot.writeListEnd();
      },
      [&](auto& l) { return write(prot, l); });
}

template <typename Protocol, typename... Args>
std::uint32_t write(Protocol& prot, const SetOf<ValueHolder, Args...>& set) {
  uint32_t serializedSize = 0;
  const ValueType elementType =
      set.size() > 0 ? set.begin()->get_type() : DEFAULT_CONTAINER_VALUE_TYPE;
  serializedSize +=
      prot.writeSetBegin(value_type_into_ttype(elementType), set.size());
  for (const auto& elem : set) {
    ensure_same_type(elem, elementType);
    serializedSize += write(prot, elem);
  }
  serializedSize += prot.writeSetEnd();
  return serializedSize;
}

template <typename Protocol, typename T, typename... Args>
std::uint32_t write(Protocol& prot, const SetOf<T, Args...>& set) {
  uint32_t serializedSize = 0;
  using Tag = detail::native_value_type<T>::tag;
  serializedSize += prot.writeSetBegin(op::typeTagToTType<Tag>, set.size());
  for (const auto& elem : set) {
    serializedSize += write(prot, elem);
  }
  serializedSize += prot.writeSetEnd();
  return serializedSize;
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeSet& set) {
  return folly::variant_match(
      set.inner(),
      [&](const std::monostate&) {
        // Note: Write an empty set with a default ttype
        return prot.writeSetBegin(
                   value_type_into_ttype(DEFAULT_CONTAINER_VALUE_TYPE), 0) +
            prot.writeSetEnd();
      },
      [&](const auto& s) { return write(prot, s); });
}

template <typename Protocol, typename K, typename V, typename... Args>
std::uint32_t write(Protocol& prot, const MapOf<K, V, Args...>& map) {
  uint32_t serializedSize = 0;
  if constexpr (
      std::is_same_v<K, ValueHolder> && std::is_same_v<V, ValueHolder>) {
    const ValueType keyType = map.size() > 0 ? map.begin()->first.get_type()
                                             : DEFAULT_CONTAINER_VALUE_TYPE;
    const ValueType valType = map.size() > 0 ? map.begin()->second.get_type()
                                             : DEFAULT_CONTAINER_VALUE_TYPE;
    serializedSize += prot.writeMapBegin(
        value_type_into_ttype(keyType),
        value_type_into_ttype(valType),
        map.size());
    for (const auto& [key, val] : map) {
      ensure_same_type(key, keyType);
      ensure_same_type(val, valType);
      serializedSize += write(prot, key);
      serializedSize += write(prot, val);
    }
  } else if constexpr (std::is_same_v<V, ValueHolder>) {
    using KeyTag = detail::native_value_type<K>::tag;
    const ValueType valType = map.size() > 0 ? map.begin()->second.get_type()
                                             : DEFAULT_CONTAINER_VALUE_TYPE;
    serializedSize += prot.writeMapBegin(
        op::typeTagToTType<KeyTag>, value_type_into_ttype(valType), map.size());
    for (const auto& [key, val] : map) {
      serializedSize += write(prot, key);
      ensure_same_type(val, valType);
      serializedSize += write(prot, val);
    }
  } else {
    using KeyTag = detail::native_value_type<K>::tag;
    using ValueTag = detail::native_value_type<V>::tag;
    static_assert(
        !std::is_same_v<K, ValueHolder> && !std::is_same_v<V, ValueHolder>);
    serializedSize += prot.writeMapBegin(
        op::typeTagToTType<KeyTag>, op::typeTagToTType<ValueTag>, map.size());
    for (const auto& [key, val] : map) {
      serializedSize += write(prot, key);
      serializedSize += write(prot, val);
    }
  }
  return serializedSize;
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeMap& map) {
  return folly::variant_match(
      map.inner(), [&](const auto& m) { return write(prot, m); });
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeValue& value) {
  return folly::variant_match(
      value.inner(), [&](const auto& v) { return write(prot, v); });
}

std::uint32_t detail::serializeObjectVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const NativeObject& obj) {
  return write(prot, obj);
}

std::uint32_t detail::serializeObjectVia(
    ::apache::thrift::CompactProtocolWriter& prot, const NativeObject& obj) {
  return write(prot, obj);
}

std::uint32_t detail::serializeValueVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const NativeValue& value) {
  return write(prot, value);
}
std::uint32_t detail::serializeValueVia(
    ::apache::thrift::CompactProtocolWriter& prot, const NativeValue& value) {
  return write(prot, value);
}

// ---- Hashing utilities ---- //

const std::size_t OBJECT_HASH_SEED = 0xFE << 24;
const std::size_t SET_HASH_SEED = 0xFD << 24;
const std::size_t MAP_HASH_SEED = 0xFC << 24;

struct ValueHasher {
  size_t operator()(const NativeValue& v) const;
  size_t operator()(const ValueHolder& v) const;
  size_t operator()(const Bool& b) const;
  size_t operator()(const I8& i) const;
  size_t operator()(const I16& i) const;
  size_t operator()(const I32& i) const;
  size_t operator()(const I64& i) const;
  size_t operator()(const Float& f) const;
  size_t operator()(const Double& d) const;
  size_t operator()(const Bytes& s) const;
  size_t operator()(const NativeObject& o) const;
  size_t operator()(const NativeList& l) const;
  size_t operator()(const NativeSet& s) const;
  size_t operator()(const NativeMap& m) const;
  size_t operator()(const std::monostate&) const;
  template <typename T, typename U>
  size_t operator()(const std::pair<T, U>& p) const;
  template <typename T>
  size_t operator()(const ListOf<T>& l) const;
  template <typename T>
  size_t operator()(const SetOf<T>& s) const;
  template <typename Key, typename Value>
  size_t operator()(const MapOf<Key, Value>& map) const;
};

size_t ValueHasher::operator()(const NativeValue& v) const {
  return v.inner().index() + folly::variant_match(v.inner(), ValueHasher{});
}
size_t ValueHasher::operator()(const ValueHolder& v) const {
  return ValueHasher{}(v.as_value());
}
size_t ValueHasher::operator()(const Bool& b) const {
  return std::hash<bool>{}(b);
}
size_t ValueHasher::operator()(const I8& i) const {
  return std::hash<int8_t>{}(i);
}
size_t ValueHasher::operator()(const I16& i) const {
  return std::hash<int16_t>{}(i);
}
size_t ValueHasher::operator()(const I32& i) const {
  return std::hash<int32_t>{}(i);
}
size_t ValueHasher::operator()(const I64& i) const {
  return std::hash<int64_t>{}(i);
}
size_t ValueHasher::operator()(const Float& f) const {
  return std::hash<float>{}(f);
}
size_t ValueHasher::operator()(const Double& d) const {
  return std::hash<double>{}(d);
}
size_t ValueHasher::operator()(const Bytes& s) const {
  return folly::IOBufHash{}(s.buf_);
}

size_t ValueHasher::operator()(const NativeObject& o) const {
  return folly::hash::commutative_hash_combine_range_generic(
      OBJECT_HASH_SEED, ValueHasher{}, o.begin(), o.end());
  return 0;
}
size_t ValueHasher::operator()(const NativeList& l) const {
  return l.inner().index() + folly::variant_match(l.inner(), ValueHasher{});
}
size_t ValueHasher::operator()(const NativeSet& s) const {
  return s.inner().index() + folly::variant_match(s.inner(), ValueHasher{});
}
size_t ValueHasher::operator()(const NativeMap& m) const {
  return m.inner().index() + folly::variant_match(m.inner(), ValueHasher{});
}

template <typename T, typename U>
size_t ValueHasher::operator()(const std::pair<T, U>& p) const {
  return folly::hash::hash_combine_generic(ValueHasher{}, p.first, p.second);
}

template <typename T>
size_t ValueHasher::operator()(const ListOf<T>& l) const {
  if constexpr (std::is_same_v<T, Bool>) {
    return std::hash<ListOf<Bool>>{}(l);
  } else {
    return folly::hash::hash_range(l.begin(), l.end(), 0, ValueHasher{});
  }
}

template <typename T>
size_t ValueHasher::operator()(const SetOf<T>& s) const {
  // Note: Hash must be order-independent
  return folly::hash::commutative_hash_combine_range_generic(
      SET_HASH_SEED, ValueHasher{}, s.begin(), s.end());
}

template <typename Key, typename Value>
size_t ValueHasher::operator()(const MapOf<Key, Value>& map) const {
  // Note: Hash must be order-independent
  return folly::hash::commutative_hash_combine_range_generic(
      MAP_HASH_SEED, ValueHasher{}, map.begin(), map.end());
}

size_t ValueHasher::operator()(const std::monostate&) const {
  return 0;
}

template <typename T>
size_t hash_value(const T& t) {
  return ValueHasher{}(t);
}

size_t detail::hash_value(const NativeValue& v) {
  return ValueHasher{}(v);
}

size_t detail::hash_value(const NativeObject& o) {
  return ValueHasher{}(o);
}

size_t detail::hash_value(const Bytes& s) {
  return ValueHasher{}(s);
}

size_t detail::hash_value(const ValueHolder& v) {
  return ValueHasher{}(v);
}

// ---- Object ---- //

NativeObject::Fields::iterator NativeObject::begin() {
  return fields.begin();
}

NativeObject::Fields::iterator NativeObject::end() {
  return fields.end();
}

NativeObject::Fields::const_iterator NativeObject::begin() const {
  return fields.begin();
}

NativeObject::Fields::const_iterator NativeObject::end() const {
  return fields.end();
}

NativeValue& NativeObject::operator[](FieldId i) {
  return fields[i];
}
NativeValue& NativeObject::operator[](::apache::thrift::FieldId i) {
  return fields[static_cast<FieldId>(i)];
}
NativeValue& NativeObject::at(FieldId i) {
  return fields.at(i).as_value();
}
NativeValue& NativeObject::at(::apache::thrift::FieldId i) {
  return at(static_cast<FieldId>(i));
}
const NativeValue& NativeObject::at(FieldId i) const {
  return fields.at(i).as_value();
}
const NativeValue& NativeObject::at(::apache::thrift::FieldId i) const {
  return at(static_cast<FieldId>(i));
}
bool NativeObject::contains(FieldId i) const {
  return fields.find(i) != fields.end();
}
bool NativeObject::contains(::apache::thrift::FieldId i) const {
  return contains(static_cast<FieldId>(i));
}
std::size_t NativeObject::erase(FieldId i) {
  return fields.erase(i);
}
std::size_t NativeObject::erase(::apache::thrift::FieldId i) {
  return erase(static_cast<FieldId>(i));
}
NativeValue* NativeObject::if_contains(FieldId i) {
  auto* ptr = folly::get_ptr(fields, i);
  return ptr ? &ptr->as_value() : nullptr;
}
NativeValue* NativeObject::if_contains(::apache::thrift::FieldId i) {
  return if_contains(static_cast<FieldId>(i));
}
const NativeValue* NativeObject::if_contains(FieldId i) const {
  const auto* ptr = folly::get_ptr(fields, i);
  return ptr ? &ptr->as_value() : nullptr;
}
const NativeValue* NativeObject::if_contains(
    ::apache::thrift::FieldId i) const {
  return if_contains(static_cast<FieldId>(i));
}

#define FBTHRIFT_DEF_MAIN_TYPE_ACCESS(CLASS, TYPE, NAME)     \
  bool CLASS::is_##NAME() const {                            \
    return std::holds_alternative<TYPE>(kind_);              \
  }                                                          \
  const TYPE& CLASS::as_##NAME() const {                     \
    return std::get<TYPE>(kind_);                            \
  }                                                          \
  TYPE& CLASS::as_##NAME() {                                 \
    return std::get<TYPE>(kind_);                            \
  }                                                          \
  const TYPE* CLASS::if_##NAME() const {                     \
    return std::get_if<TYPE>(&kind_);                        \
  }                                                          \
  TYPE* CLASS::if_##NAME() {                                 \
    return std::get_if<TYPE>(&kind_);                        \
  }                                                          \
  TYPE& CLASS::ensure_##NAME() {                             \
    if (!std::holds_alternative<TYPE>(kind_)) {              \
      return kind_.emplace<TYPE>();                          \
    }                                                        \
    return std::get<TYPE>(kind_);                            \
  }                                                          \
  template <typename... Args>                                \
  TYPE& CLASS::emplace_##NAME(Args&&... args) {              \
    return kind_.emplace<TYPE>(std::forward<Args>(args)...); \
  }

// ---- NativeList API ---- //

FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<Bool>, list_of_bool)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<I8>, list_of_i8)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<I16>, list_of_i16)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<I32>, list_of_i32)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<I64>, list_of_i64)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<Float>, list_of_float)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<Double>, list_of_double)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<Bytes>, list_of_bytes)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<NativeObject>, list_of_object)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeList, ListOf<ValueHolder>, list_of_value)

// ---- NativeSet API ---- //

FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<Bool>, set_of_bool)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<I8>, set_of_i8)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<I16>, set_of_i16)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<I32>, set_of_i32)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<I64>, set_of_i64)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<Float>, set_of_float)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<Double>, set_of_double)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<Bytes>, set_of_bytes)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<NativeObject>, set_of_object)
FBTHRIFT_DEF_MAIN_TYPE_ACCESS(NativeSet, SetOf<ValueHolder>, set_of_value)

// ---- Misc utilities ---- //

TType value_type_into_ttype(ValueType type) {
  switch (type) {
    case ValueType::Bool:
      return TType::T_BOOL;
    case ValueType::I8:
      return TType::T_BYTE;
    case ValueType::I16:
      return TType::T_I16;
    case ValueType::I32:
      return TType::T_I32;
    case ValueType::I64:
      return TType::T_I64;
    case ValueType::Float:
      return TType::T_FLOAT;
    case ValueType::Double:
      return TType::T_DOUBLE;
    case ValueType::String:
      return TType::T_STRING;
    case ValueType::Bytes:
      return TType::T_STRING;
    case ValueType::List:
      return TType::T_LIST;
    case ValueType::Set:
      return TType::T_SET;
    case ValueType::Map:
      return TType::T_MAP;
    case ValueType::Struct:
      return TType::T_STRUCT;
    case ValueType::Empty:
      throw std::runtime_error("An Empty value does not have a TType");
  }
}

} // namespace apache::thrift::protocol::experimental
