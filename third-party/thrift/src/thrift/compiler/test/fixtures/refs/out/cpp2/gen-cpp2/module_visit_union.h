/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/refs/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */
#pragma once

#include "thrift/compiler/test/fixtures/refs/gen-cpp2/module_metadata.h"
#include <thrift/lib/cpp2/visitation/visit_union.h>

namespace apache {
namespace thrift {
namespace detail {

template <>
struct VisitUnion<::cpp2::MyUnion> {

  template <typename F, typename T>
  decltype(auto) operator()([[maybe_unused]] F&& f, T&& t) const {
    using Union = std::remove_reference_t<T>;
    switch (t.getType()) {
    case Union::Type::anInteger:
      return f(0, *static_cast<T&&>(t).anInteger_ref());
    case Union::Type::aString:
      return f(1, *static_cast<T&&>(t).aString_ref());
    case Union::Type::__EMPTY__:
      return decltype(f(0, *static_cast<T&&>(t).anInteger_ref()))();
    default:
      throw std::runtime_error{folly::to<std::string>(
          "Enum got invalid value ",
          static_cast<std::underlying_type_t<decltype(t.getType())>>(
              t.getType()))};
    }
  }
};
template <>
struct VisitUnion<::cpp2::NonTriviallyDestructibleUnion> {

  template <typename F, typename T>
  decltype(auto) operator()([[maybe_unused]] F&& f, T&& t) const {
    using Union = std::remove_reference_t<T>;
    switch (t.getType()) {
    case Union::Type::int_field:
      return f(0, *static_cast<T&&>(t).int_field_ref());
    case Union::Type::__EMPTY__:
      return decltype(f(0, *static_cast<T&&>(t).int_field_ref()))();
    default:
      throw std::runtime_error{folly::to<std::string>(
          "Enum got invalid value ",
          static_cast<std::underlying_type_t<decltype(t.getType())>>(
              t.getType()))};
    }
  }
};
} // namespace detail
} // namespace thrift
} // namespace apache
