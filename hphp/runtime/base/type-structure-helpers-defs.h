/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-structure.h"

namespace HPHP {

const StaticString s_allows_unknown_fields("allows_unknown_fields");
const StaticString s_elem_types("elem_types");
const StaticString s_fields("fields");
const StaticString s_kind("kind");
const StaticString s_value("value");
const StaticString s_nullable("nullable");
const StaticString s_optional_shape_field("optional_shape_field");
const StaticString s_classname("classname");

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE bool is_ts_nullable(const ArrayData* ts) {
  auto const nullable_field = ts->rval(s_nullable.get());
  assertx(nullable_field == nullptr ||
    (isBoolType(nullable_field.type()) && nullable_field.val().num));
  return nullable_field != nullptr;
}

ALWAYS_INLINE const TypeStructure::Kind get_ts_kind(const ArrayData* ts) {
  auto const kind_field = ts->rval(s_kind.get());
  assertx(kind_field != nullptr && isIntType(kind_field.type()));
  return static_cast<TypeStructure::Kind>(kind_field.val().num);
}

ALWAYS_INLINE const ArrayData* get_ts_elem_types(const ArrayData* ts) {
  auto const elem_types_field = ts->rval(s_elem_types.get());
  assertx(elem_types_field != nullptr && isArrayType(elem_types_field.type()));
  return elem_types_field.val().parr;
}

ALWAYS_INLINE const ArrayData* get_ts_fields(const ArrayData* ts) {
  auto const fields_field = ts->rval(s_fields.get());
  assertx(fields_field != nullptr && isArrayType(fields_field.type()));
  return fields_field.val().parr;
}

ALWAYS_INLINE const bool does_ts_shape_allow_unknown_fields(
  const ArrayData* ts
) {
  auto const allows_unknown_fields = ts->rval(s_allows_unknown_fields.get());
  assertx(allows_unknown_fields == nullptr ||
    (isBoolType(allows_unknown_fields.type()) &&
      allows_unknown_fields.val().num));
  return allows_unknown_fields != nullptr;
}

ALWAYS_INLINE const bool is_optional_ts_shape_field(const ArrayData* ts) {
  auto const optional_shape_field = ts->rval(s_optional_shape_field.get());
  assertx(optional_shape_field == nullptr ||
    (isBoolType(optional_shape_field.type()) &&
      optional_shape_field.val().num));
  return optional_shape_field != nullptr;
}

ALWAYS_INLINE const ArrayData* get_ts_value_field(const ArrayData* ts) {
  auto const value_field = ts->rval(s_value.get());
  assertx(value_field != nullptr && isArrayType(value_field.type()));
  return value_field.val().parr;
}

ALWAYS_INLINE const StringData* get_ts_classname(const ArrayData* ts) {
  auto const classname_field = ts->rval(s_classname.get());
  assertx(classname_field != nullptr && isStringType(classname_field.type()));
  return classname_field.val().pstr;
}

///////////////////////////////////////////////////////////////////////////////

}
