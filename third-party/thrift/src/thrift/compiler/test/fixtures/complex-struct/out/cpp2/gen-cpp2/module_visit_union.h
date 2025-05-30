/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/complex-struct/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */
#pragma once

#include "thrift/compiler/test/fixtures/complex-struct/gen-cpp2/module_metadata.h"
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
    case Union::Type::myEnum:
      return f(0, *static_cast<T&&>(t).myEnum_ref());
    case Union::Type::myStruct:
      return f(1, *static_cast<T&&>(t).myStruct_ref());
    case Union::Type::myDataItem:
      return f(2, *static_cast<T&&>(t).myDataItem_ref());
    case Union::Type::complexNestedStruct:
      return f(3, *static_cast<T&&>(t).complexNestedStruct_ref());
    case Union::Type::longValue:
      return f(4, *static_cast<T&&>(t).longValue_ref());
    case Union::Type::intValue:
      return f(5, *static_cast<T&&>(t).intValue_ref());
    case Union::Type::__EMPTY__:
      return decltype(f(0, *static_cast<T&&>(t).myEnum_ref()))();
    default:
      throw std::runtime_error{folly::to<std::string>(
          "Enum got invalid value ",
          static_cast<std::underlying_type_t<decltype(t.getType())>>(
              t.getType()))};
    }
  }
};
template <>
struct VisitUnion<::cpp2::MyUnionFloatFieldThrowExp> {

  template <typename F, typename T>
  decltype(auto) operator()([[maybe_unused]] F&& f, T&& t) const {
    using Union = std::remove_reference_t<T>;
    switch (t.getType()) {
    case Union::Type::myEnum:
      return f(0, *static_cast<T&&>(t).myEnum_ref());
    case Union::Type::setFloat:
      return f(1, *static_cast<T&&>(t).setFloat_ref());
    case Union::Type::myDataItem:
      return f(2, *static_cast<T&&>(t).myDataItem_ref());
    case Union::Type::complexNestedStruct:
      return f(3, *static_cast<T&&>(t).complexNestedStruct_ref());
    case Union::Type::__EMPTY__:
      return decltype(f(0, *static_cast<T&&>(t).myEnum_ref()))();
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
