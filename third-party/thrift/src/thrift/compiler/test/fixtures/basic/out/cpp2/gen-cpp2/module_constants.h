/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/basic/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */
#pragma once

#include <thrift/lib/cpp2/gen/module_constants_h.h>

#include "thrift/compiler/test/fixtures/basic/gen-cpp2/module_types.h"

namespace test::fixtures::basic {
/** Glean {"file": "thrift/compiler/test/fixtures/basic/src/module.thrift"} */
namespace module_constants {

  /** Glean {"constant": "FLAG"} */
  constexpr bool const FLAG_ = true;
  /** Glean {"constant": "FLAG"} */
  constexpr bool FLAG() {
    return FLAG_;
  }

  /** Glean {"constant": "OFFSET"} */
  constexpr ::std::int8_t const OFFSET_ = static_cast<::std::int8_t>(-10);
  /** Glean {"constant": "OFFSET"} */
  constexpr ::std::int8_t OFFSET() {
    return OFFSET_;
  }

  /** Glean {"constant": "COUNT"} */
  constexpr ::std::int16_t const COUNT_ = static_cast<::std::int16_t>(200);
  /** Glean {"constant": "COUNT"} */
  constexpr ::std::int16_t COUNT() {
    return COUNT_;
  }

  /** Glean {"constant": "MASK"} */
  constexpr ::std::int32_t const MASK_ = static_cast<::std::int32_t>(16388846);
  /** Glean {"constant": "MASK"} */
  constexpr ::std::int32_t MASK() {
    return MASK_;
  }

  /** Glean {"constant": "E"} */
  constexpr double const E_ = static_cast<double>(2.718281828459);
  /** Glean {"constant": "E"} */
  constexpr double E() {
    return E_;
  }

  /** Glean {"constant": "DATE"} */
  constexpr char const * const DATE_ = "June 28, 2017";
  /** Glean {"constant": "DATE"} */
  constexpr char const * DATE() {
    return DATE_;
  }

  /** Glean {"constant": "AList"} */
  ::std::vector<::std::int32_t> const& AList();

  /** Glean {"constant": "ASet"} */
  ::std::set<::std::string> const& ASet();

  /** Glean {"constant": "AMap"} */
  ::std::map<::std::string, ::std::vector<::std::int32_t>> const& AMap();

  FOLLY_EXPORT ::std::string_view _fbthrift_schema_402a672704b1a4e6();
  FOLLY_EXPORT ::folly::Range<const ::std::string_view*> _fbthrift_schema_402a672704b1a4e6_includes();
  FOLLY_EXPORT ::folly::Range<const ::std::string_view*> _fbthrift_schema_402a672704b1a4e6_uris();

} // namespace module_constants
} // namespace test::fixtures::basic
