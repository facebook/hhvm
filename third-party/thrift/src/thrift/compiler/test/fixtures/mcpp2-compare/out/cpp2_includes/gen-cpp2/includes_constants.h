/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/mcpp2-compare/src/includes.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */
#pragma once

#include <thrift/lib/cpp2/gen/module_constants_h.h>

#include "thrift/compiler/test/fixtures/mcpp2-compare/gen-cpp2/includes_types.h"

namespace a::different::ns {
/** Glean {"file": "thrift/compiler/test/fixtures/mcpp2-compare/src/includes.thrift"} */
namespace includes_constants {

  /** Glean {"constant": "IncludedConstant"} */
  constexpr ::std::int64_t const IncludedConstant_ = static_cast<::std::int64_t>(42);
  /** Glean {"constant": "IncludedConstant"} */
  constexpr ::std::int64_t IncludedConstant() {
    return IncludedConstant_;
  }

  FOLLY_EXPORT ::std::string_view _fbthrift_schema_75bef702b0f2c20f();
  FOLLY_EXPORT ::folly::Range<const ::std::string_view*> _fbthrift_schema_75bef702b0f2c20f_includes();
  FOLLY_EXPORT ::folly::Range<const ::std::string_view*> _fbthrift_schema_75bef702b0f2c20f_uris();

} // namespace includes_constants
} // namespace a::different::ns
