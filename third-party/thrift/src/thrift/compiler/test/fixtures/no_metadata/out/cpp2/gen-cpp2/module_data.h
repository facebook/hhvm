/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/no_metadata/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */
#pragma once

#include <thrift/lib/cpp2/gen/module_data_h.h>

#include "thrift/compiler/test/fixtures/no_metadata/gen-cpp2/module_types.h"

namespace apache { namespace thrift {

template <> struct TEnumDataStorage<::cpp2::MyEnum> {
  using type = ::cpp2::MyEnum;
  static constexpr const std::size_t size = 2;
  static constexpr std::array<type, size> values = { {
      type::MyValue1,
      type::MyValue2,
  }};
  static constexpr std::array<std::string_view, size> names = { {
      "MyValue1"sv,
      "MyValue2"sv,
  }};
};

template <> struct TEnumDataStorage<::cpp2::MyUnion::Type> {
  using type = ::cpp2::MyUnion::Type;
  static constexpr const std::size_t size = 3;
  static constexpr std::array<type, size> values = { {
      type::myEnum,
      type::myStruct,
      type::myDataItem,
  }};
  static constexpr std::array<std::string_view, size> names = { {
      "myEnum"sv,
      "myStruct"sv,
      "myDataItem"sv,
  }};
};

template <> struct TStructDataStorage<::cpp2::MyStruct> {
  static constexpr const std::size_t fields_size = 4;
  static const std::string_view name;
  static const std::array<std::string_view, fields_size> fields_names;
  static const std::array<int16_t, fields_size> fields_ids;
  static const std::array<protocol::TType, fields_size> fields_types;

 private:
  // The following fields describe internal storage metadata, and are private to
  // prevent user logic from accessing them, but they can be inspected by
  // debuggers.
  // -1 if the field has no isset.
  static const std::array<int, fields_size> isset_indexes;
};

template <> struct TStructDataStorage<::cpp2::MyDataItem> {
  static constexpr const std::size_t fields_size = 0;
  static const std::string_view name;
  static const std::array<std::string_view, fields_size> fields_names;
  static const std::array<int16_t, fields_size> fields_ids;
  static const std::array<protocol::TType, fields_size> fields_types;

 private:
  // The following fields describe internal storage metadata, and are private to
  // prevent user logic from accessing them, but they can be inspected by
  // debuggers.
  // -1 if the field has no isset.
  static const std::array<int, fields_size> isset_indexes;
};

template <> struct TStructDataStorage<::cpp2::MyUnion> {
  static constexpr const std::size_t fields_size = 3;
  static const std::string_view name;
  static const std::array<std::string_view, fields_size> fields_names;
  static const std::array<int16_t, fields_size> fields_ids;
  static const std::array<protocol::TType, fields_size> fields_types;

 private:
  // The following fields describe internal storage metadata, and are private to
  // prevent user logic from accessing them, but they can be inspected by
  // debuggers.
  // -1 if the field has no isset.
  static const std::array<int, fields_size> isset_indexes;
};

}} // apache::thrift
