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

#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>

#include <thrift/common/detail/string.h>
#include <thrift/common/tree_printer.h>
#include <thrift/lib/cpp/util/EnumUtils.h>

#include <folly/Memory.h>
#include <folly/base64.h>
#include <folly/hash/Hash.h>
#include <folly/lang/Assume.h>

#include <fmt/core.h>

#include <ostream>

namespace apache::thrift::type_system {

SerializableRecord::~SerializableRecord() noexcept = default;

SerializableRecord::SerializableRecord(FieldSet&& fieldSet) noexcept
    : datum_(folly::copy_to_unique_ptr(std::move(fieldSet))) {}
SerializableRecord::SerializableRecord(List&& list) noexcept
    : datum_(folly::copy_to_unique_ptr(std::move(list))) {}
SerializableRecord::SerializableRecord(Set&& set) noexcept
    : datum_(folly::copy_to_unique_ptr(std::move(set))) {}
SerializableRecord::SerializableRecord(Map&& map) noexcept
    : datum_(folly::copy_to_unique_ptr(std::move(map))) {}

SerializableRecord::SerializableRecord(const SerializableRecord& other)
    : datum_(folly::variant_match(
          other.datum_,
          [](const auto& datum) -> Alternative { return datum; },
          [](const FieldSetPtr& datum) -> Alternative {
            return folly::copy_through_unique_ptr(datum);
          },
          [](const ListPtr& datum) -> Alternative {
            return folly::copy_through_unique_ptr(datum);
          },
          [](const SetPtr& datum) -> Alternative {
            return folly::copy_through_unique_ptr(datum);
          },
          [](const MapPtr& datum) -> Alternative {
            return folly::copy_through_unique_ptr(datum);
          },
          [](const SerializableRecord::ByteArray& datum) -> Alternative {
            return datum->clone();
          })) {}

SerializableRecord& SerializableRecord::operator=(
    const SerializableRecord& other) {
  datum_ = folly::variant_match(
      other.datum_,
      [](const auto& datum) -> Alternative { return datum; },
      [](const FieldSetPtr& datum) -> Alternative {
        return folly::copy_through_unique_ptr(datum);
      },
      [](const ListPtr& datum) -> Alternative {
        return folly::copy_through_unique_ptr(datum);
      },
      [](const SetPtr& datum) -> Alternative {
        return folly::copy_through_unique_ptr(datum);
      },
      [](const MapPtr& datum) -> Alternative {
        return folly::copy_through_unique_ptr(datum);
      },
      [](const SerializableRecord::ByteArray& datum) -> Alternative {
        return datum->clone();
      });
  return *this;
}

/* static */ SerializableRecord SerializableRecord::fromThrift(
    SerializableRecordUnion&& value) {
  using Type = SerializableRecordUnion::Type;
  // NOTE: The constructors below perform validation on the input data. It may
  // throw exceptions.
  switch (value.getType()) {
    case Type::boolDatum:
      return SerializableRecord::Bool(*value.boolDatum());
    case Type::int8Datum:
      return SerializableRecord::Int8(*value.int8Datum());
    case Type::int16Datum:
      return SerializableRecord::Int16(*value.int16Datum());
    case Type::int32Datum:
      return SerializableRecord::Int32(*value.int32Datum());
    case Type::int64Datum:
      return SerializableRecord::Int64(*value.int64Datum());
    case Type::float32Datum:
      return SerializableRecord::Float32(*value.float32Datum());
    case Type::float64Datum:
      return SerializableRecord::Float64(*value.float64Datum());
    case Type::textDatum:
      return SerializableRecord::Text(std::move(*value.textDatum()));
    case Type::byteArrayDatum:
      // union_field_ref::value() elides the unique_ptr, which we need here.
      return SerializableRecord::ByteArray(value.move_byteArrayDatum());
    case Type::fieldSetDatum: {
      std::map<FieldId, SerializableRecordUnion>& fieldSet =
          *value.fieldSetDatum();
      SerializableRecord::FieldSet result;
      for (auto&& [id, datum] : fieldSet) {
        result.emplace(id, fromThrift(std::move(datum)));
      }
      return result;
    }
    case Type::listDatum: {
      std::vector<SerializableRecordUnion>& list = *value.listDatum();
      SerializableRecord::List result;
      result.reserve(list.size());
      for (auto& datum : list) {
        result.push_back(fromThrift(std::move(datum)));
      }
      return result;
    }
    case Type::setDatum: {
      std::vector<SerializableRecordUnion>& list = *value.setDatum();
      SerializableRecord::Set result;
      for (auto& datum : list) {
        auto [_, inserted] = result.insert(fromThrift(std::move(datum)));
        if (!inserted) {
          folly::throw_exception<std::invalid_argument>(
              "Duplicate element in setDatum");
        }
      }
      return result;
    }
    case Type::mapDatum: {
      std::vector<SerializableRecordMapEntry>& list = *value.mapDatum();
      SerializableRecord::Map result;
      for (auto& entry : list) {
        SerializableRecordUnion& k = *entry.key();
        SerializableRecordUnion& v = *entry.value();
        auto [_, inserted] =
            result.emplace(fromThrift(std::move(k)), fromThrift(std::move(v)));
        if (!inserted) {
          folly::throw_exception<std::invalid_argument>(
              fmt::format("Duplicate element in mapDatum"));
        }
      }
      return result;
    }
    case Type::__EMPTY__:
    default:
      break;
  }
  folly::throw_exception<std::invalid_argument>(
      "SerializedRecord cannot be empty");
}

/* static */ SerializableRecordUnion SerializableRecord::toThrift(
    const SerializableRecord& value) {
  using Kind = SerializableRecord::Kind;
  SerializableRecordUnion result;
  switch (value.kind()) {
    case Kind::BOOL:
      result.boolDatum() = bool(value.asBool());
      break;
    case Kind::INT8:
      result.int8Datum() = std::int8_t(value.asInt8());
      break;
    case Kind::INT16:
      result.int16Datum() = std::int16_t(value.asInt16());
      break;
    case Kind::INT32:
      result.int32Datum() = std::int32_t(value.asInt32());
      break;
    case Kind::INT64:
      result.int64Datum() = std::int64_t(value.asInt64());
      break;
    case Kind::FLOAT32:
      result.float32Datum() = float(value.asFloat32());
      break;
    case Kind::FLOAT64:
      result.float64Datum() = double(value.asFloat64());
      break;
    case Kind::TEXT:
      result.textDatum() = std::string(value.asText());
      break;
    case Kind::BYTE_ARRAY:
      result.set_byteArrayDatum(value.asByteArray()->clone());
      break;
    case Kind::FIELD_SET: {
      std::map<FieldId, SerializableRecordUnion> fieldSet;
      for (const auto& [id, datum] : value.asFieldSet()) {
        fieldSet.emplace(id, toThrift(datum));
      }
      result.fieldSetDatum() = std::move(fieldSet);
    } break;
    case Kind::LIST: {
      std::vector<SerializableRecordUnion> list;
      const SerializableRecord::List& valueAsList = value.asList();
      list.reserve(valueAsList.size());
      for (const auto& r : valueAsList) {
        list.push_back(toThrift(r));
      }
      result.listDatum() = std::move(list);
    } break;
    case Kind::SET: {
      // NOTE: the order of the output is not deterministic
      std::vector<SerializableRecordUnion> set;
      const SerializableRecord::Set& valueAsSet = value.asSet();
      set.reserve(valueAsSet.size());
      for (const auto& r : valueAsSet) {
        set.push_back(toThrift(r));
      }
      result.setDatum() = std::move(set);
    } break;
    case Kind::MAP: {
      // NOTE: the order of the output is not deterministic
      std::vector<SerializableRecordMapEntry> map;
      const SerializableRecord::Map& valueAsMap = value.asMap();
      for (const auto& [k, v] : valueAsMap) {
        SerializableRecordMapEntry entry;
        entry.key() = toThrift(k);
        entry.value() = toThrift(v);
        map.push_back(std::move(entry));
      }
      result.mapDatum() = std::move(map);
    } break;
    default:
      break;
  }
  return result;
}

[[noreturn]] void SerializableRecord::throwAccessInactiveKind() const {
  std::string actualKind = util::enumNameSafe(std::invoke([&] {
    using Type = SerializableRecordUnion::Type;
    switch (kind()) {
      case Kind::BOOL:
        return Type::boolDatum;
      case Kind::INT8:
        return Type::int8Datum;
      case Kind::INT16:
        return Type::int16Datum;
      case Kind::INT32:
        return Type::int32Datum;
      case Kind::INT64:
        return Type::int64Datum;
      case Kind::FLOAT32:
        return Type::float32Datum;
      case Kind::FLOAT64:
        return Type::float64Datum;
      case Kind::TEXT:
        return Type::textDatum;
      case Kind::BYTE_ARRAY:
        return Type::byteArrayDatum;
      case Kind::FIELD_SET:
        return Type::fieldSetDatum;
      case Kind::LIST:
        return Type::listDatum;
      case Kind::SET:
        return Type::setDatum;
      case Kind::MAP:
        return Type::mapDatum;
      default:
        break;
    }
    return Type::__EMPTY__;
  }));
  folly::throw_exception<std::runtime_error>(fmt::format(
      "tried to access SerializableRecord with inactive kind, actual kind was: {}",
      actualKind));
}

/* static */ [[noreturn]] void SerializableRecord::throwAccessEmpty() {
  folly::throw_exception<std::runtime_error>(
      "tried to access empty SerializableRecord");
}

bool operator==(const SerializableRecord& lhs, const SerializableRecord& rhs) {
  using Kind = SerializableRecord::Kind;
  switch (lhs.kind()) {
    case Kind::BOOL:
      return lhs.asBool() == rhs;
    case Kind::INT8:
      return lhs.asInt8() == rhs;
    case Kind::INT16:
      return lhs.asInt16() == rhs;
    case Kind::INT32:
      return lhs.asInt32() == rhs;
    case Kind::INT64:
      return lhs.asInt64() == rhs;
    case Kind::FLOAT32:
      return lhs.asFloat32() == rhs;
    case Kind::FLOAT64:
      return lhs.asFloat64() == rhs;
    case Kind::TEXT:
      return lhs.asText() == rhs;
    case Kind::BYTE_ARRAY:
      return lhs.asByteArray() == rhs;
    case Kind::FIELD_SET:
      return lhs.asFieldSet() == rhs;
    case Kind::LIST:
      return lhs.asList() == rhs;
    case Kind::SET:
      return lhs.asSet() == rhs;
    case Kind::MAP:
      return lhs.asMap() == rhs;
    default:
      break;
  }
  folly::assume_unreachable();
}

namespace detail {

std::size_t SerializableRecordHasher::operator()(
    const SerializableRecord& record) const noexcept {
  const std::size_t activeMemberHash = record.visit(
      [](auto&& datum) -> std::size_t {
        return std::hash<std::decay_t<decltype(datum)>>{}(datum);
      },
      [](const SerializableRecord::ByteArray& datum) -> std::size_t {
        return folly::IOBufHash{}(*datum);
      },
      [](const SerializableRecord::FieldSet& datum) -> std::size_t {
        return folly::hash::hash_range(
            datum.begin(), datum.end(), datum.size() /* seed */);
      },
      [](const SerializableRecord::List& datum) -> std::size_t {
        return folly::hash::hash_range(
            datum.begin(), datum.end(), datum.size() /* seed */);
      },
      [](const SerializableRecord::Set& datum) -> std::size_t {
        // Hash sets are unordered, so we need a commutative combination
        // function
        return folly::hash::commutative_hash_combine_range_generic(
            datum.size() /* seed */,
            datum.hash_function(),
            datum.begin(),
            datum.end());
      },
      [&](const SerializableRecord::Map& datum) -> std::size_t {
        using EntryHasher = std::hash<SerializableRecord::Map::value_type>;
        // Hash maps are unordered, so we need a commutative combination
        // function
        return folly::hash::commutative_hash_combine_range_generic(
            datum.size() /* seed */, EntryHasher{}, datum.begin(), datum.end());
      });

  using KindHash = std::hash<std::underlying_type_t<SerializableRecord::Kind>>;
  return folly::hash::hash_combine(
      KindHash{}(folly::to_underlying(record.kind())), activeMemberHash);
}

bool areByteArraysEqual(
    const type::ByteBuffer& lhs, const type::ByteBuffer& rhs) {
  return folly::IOBufEqualTo{}(lhs, rhs);
}

void ensureUTF8OrThrow(std::string_view string) {
  const auto onError = [&]() -> void {
    folly::throw_exception<std::invalid_argument>(
        fmt::format("UTF-8 validation failed for '{}'", string));
  };
  const auto charAt = [&](std::size_t i) -> unsigned char {
    return static_cast<unsigned char>(string[i]);
  };

  for (std::size_t i = 0; i < string.length();) {
    const auto c = charAt(i);
    // Determine the number of bytes in the UTF-8 sequence
    std::size_t n = 0;
    if (c <= 0x7f) {
      n = 1; // 0bbbbbbb
    } else if ((c & 0xE0) == 0xC0) {
      n = 2; // 110bbbbb
    } else if ((c & 0xF0) == 0xE0) {
      n = 3; // 1110bbbb
    } else if ((c & 0xF8) == 0xF0) {
      n = 4; // 11110bbb
    } else {
      onError();
      return;
    }
    // Check for invalid sequences
    if (n > 1 && (c & (0xFF << (8 - n))) == (0xFF << (8 - n))) {
      onError();
      return;
    }
    // Check for U+D800 to U+DFFF
    if (n == 3 && c == 0xed && i + 1 < string.length() &&
        (charAt(i + 1) & 0xa0) == 0xa0) {
      onError();
      return;
    }
    // Verify the rest of the sequence
    for (std::size_t j = 1; j < n; ++j) {
      if (i + j >= string.length() || (charAt(i + j) & 0xC0) != 0x80) {
        onError();
        return;
      }
    }
    // Move to the next sequence
    i += n;
  }
}

} // namespace detail

// This code produces a link error on Windows:
//     lld-link: error: undefined symbol: class std::basic_string<char, struct
//     std::char_traits<char>, class std::allocator<char>> __cdecl
//     apache::thrift::detail::escape(class std::basic_string_view<char, struct
//     std::char_traits<char>>)
//       >>> referenced by .\xplat\thrift\common\tree_printer.cc:104
#ifndef _WIN32
namespace {

void printTo(tree_printer::scope& scope, const SerializableRecord& record) {
  record.visit(
      [&](SerializableRecord::Bool value) {
        scope.print("Bool({})", value ? "true" : "false");
      },
      [&](SerializableRecord::Int8 value) { scope.print("Int8({})", value); },
      [&](SerializableRecord::Int16 value) { scope.print("Int16({})", value); },
      [&](SerializableRecord::Int32 value) { scope.print("Int32({})", value); },
      [&](SerializableRecord::Int64 value) { scope.print("Int64({})", value); },
      [&](SerializableRecord::Float32 value) {
        scope.print("Float32({})", value);
      },
      [&](SerializableRecord::Float64 value) {
        scope.print("Float64({})", value);
      },
      [&](const SerializableRecord::Text& value) {
        scope.print("Text(\"{}\")", apache::thrift::detail::escape(value));
      },
      [&](const SerializableRecord::ByteArray& value) {
        scope.print(
            "ByteArray(\"{}\")", folly::base64Encode(value->toString()));
      },
      [&](const SerializableRecord::FieldSet& value) {
        scope.print("FieldSet(size={})", value.size());
        for (const auto& [fieldId, field] : value) {
          printTo(scope.make_child("{} → ", fieldId), field);
        }
      },
      [&](const SerializableRecord::List& value) {
        scope.print("List(size={})", value.size());
        for (std::size_t i = 0; i < value.size(); ++i) {
          printTo(scope.make_child("[{}] → ", i), value[i]);
        }
      },
      [&](const SerializableRecord::Set& value) {
        // The order of elements is indeterminate because it's a hashset.
        scope.print("Set(size={})", value.size());
        for (const auto& element : value) {
          printTo(scope.make_child(), element);
        }
      },
      [&](const SerializableRecord::Map& value) {
        // The order of elements is indeterminate because it's a hashmap.
        scope.print("Map(size={})", value.size());
        for (const auto& [k, v] : value) {
          tree_printer::scope& keyScope = scope.make_child("key = ");
          printTo(keyScope, k);
          tree_printer::scope& valueScope = scope.make_child("value = ");
          printTo(valueScope, v);
        }
      });
}

} // namespace

std::string toDebugString(const SerializableRecord& record) {
  auto scope = tree_printer::scope::make_root();
  printTo(scope, record);
  return tree_printer::to_string(scope);
}

std::ostream& operator<<(std::ostream& out, const SerializableRecord& record) {
  auto scope = tree_printer::scope::make_root();
  printTo(scope, record);
  return out << scope;
}
#endif

} // namespace apache::thrift::type_system
