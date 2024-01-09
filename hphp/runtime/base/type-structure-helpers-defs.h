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
const StaticString s_param_types("param_types");
const StaticString s_return_type("return_type");
const StaticString s_variadic_type("variadic_type");
const StaticString s_fields("fields");
const StaticString s_kind("kind");
const StaticString s_value("value");
const StaticString s_nullable("nullable");
const StaticString s_soft("soft");
const StaticString s_opaque("opaque");
const StaticString s_optional_shape_field("optional_shape_field");
const StaticString s_classname("classname");
const StaticString s_wildcard("_");
const StaticString s_name("name");
const StaticString s_generic_types("generic_types");
const StaticString s_is_cls_cns("is_cls_cns");
const StaticString s_access_list("access_list");
const StaticString s_root_name("root_name");
const StaticString s_alias("alias");
const StaticString s_case_type("case_type");
const StaticString s_callable("callable");
const StaticString s_exact("exact");
const StaticString s_typevars("typevars");
const StaticString s_typevar_types("typevar_types");
const StaticString s_union_types("union_types");
const StaticString s_hh_this(annotTypeName(AnnotType::This));
const StaticString s_type_structure_non_existant_class(
  "HH\\__internal\\type_structure_non_existant_class");

// Fixed error messages
const StaticString s_reified_type_must_be_ts(
  "Reified type must be a type structure");
const StaticString s_new_instance_of_not_string(
  "You cannot create a new instance of this type as it is not a string");


///////////////////////////////////////////////////////////////////////////////

namespace detail {

ALWAYS_INLINE bool is_ts_bool(const ArrayData* ts, const String& s) {
  auto const field = ts->get(s.get());
  assertx(!field.is_init() || (isBoolType(field.type()) && field.val().num));
  return field.is_init();
}

ALWAYS_INLINE const ArrayData* get_ts_varray(const ArrayData* ts,
                                             const String& s) {
  auto const field = ts->get(s.get());
  assertx(tvIsVec(field));
  return field.val().parr;
}

ALWAYS_INLINE const ArrayData* get_ts_varray_opt(const ArrayData* ts,
                                                 const String& s) {
  auto const field = ts->get(s.get());
  if (!field.is_init()) return nullptr;
  assertx(tvIsVec(field));
  return field.val().parr;
}

ALWAYS_INLINE const ArrayData* get_ts_darray(const ArrayData* ts,
                                             const String& s) {
  auto const field = ts->get(s.get());
  assertx(tvIsDict(field));
  return field.val().parr;
}

ALWAYS_INLINE const ArrayData* get_ts_darray_opt(const ArrayData* ts,
                                                 const String& s) {
  auto const field = ts->get(s.get());
  if (!field.is_init()) return nullptr;
  assertx(tvIsDict(field));
  return field.val().parr;
}

ALWAYS_INLINE const StringData* get_ts_string(const ArrayData* ts,
                                              const String& s) {
  auto const field = ts->get(s.get());
  assertx(isStringType(field.type()));
  return field.val().pstr;
}

ALWAYS_INLINE const StringData* get_ts_string_opt(const ArrayData* ts,
                                                  const String& s) {
  auto const field = ts->get(s.get());
  if (!field.is_init()) return nullptr;
  assertx(isStringType(field.type()));
  return field.val().pstr;
}

} // detail

ALWAYS_INLINE bool is_ts_nullable(const ArrayData* ts) {
  return detail::is_ts_bool(ts, s_nullable);
}

ALWAYS_INLINE bool is_ts_soft(const ArrayData* ts) {
  return detail::is_ts_bool(ts, s_soft);
}

ALWAYS_INLINE const bool is_optional_ts_shape_field(const ArrayData* ts) {
  return detail::is_ts_bool(ts, s_optional_shape_field);
}

ALWAYS_INLINE const bool does_ts_shape_allow_unknown_fields(
  const ArrayData* ts
) {
  return detail::is_ts_bool(ts, s_allows_unknown_fields);
}

ALWAYS_INLINE const ArrayData* get_ts_elem_types(const ArrayData* ts) {
  return detail::get_ts_varray(ts, s_elem_types);
}

ALWAYS_INLINE const ArrayData* get_ts_param_types(const ArrayData* ts) {
  return detail::get_ts_varray(ts, s_param_types);
}

ALWAYS_INLINE const ArrayData* get_ts_return_type(const ArrayData* ts) {
  return detail::get_ts_darray(ts, s_return_type);
}

ALWAYS_INLINE const ArrayData* get_ts_variadic_type_opt(const ArrayData* ts) {
  return detail::get_ts_darray_opt(ts, s_variadic_type);
}

ALWAYS_INLINE const ArrayData* get_ts_fields(const ArrayData* ts) {
  return detail::get_ts_darray(ts, s_fields);
}

ALWAYS_INLINE const ArrayData* get_ts_value(const ArrayData* ts) {
  return detail::get_ts_darray(ts, s_value);
}

ALWAYS_INLINE const ArrayData* get_ts_access_list(const ArrayData* ts) {
  return detail::get_ts_varray(ts, s_access_list);
}

ALWAYS_INLINE const ArrayData* get_ts_generic_types(const ArrayData* ts) {
  return detail::get_ts_varray(ts, s_generic_types);
}

ALWAYS_INLINE const ArrayData* get_ts_generic_types_opt(const ArrayData* ts) {
  return detail::get_ts_varray_opt(ts, s_generic_types);
}

ALWAYS_INLINE const ArrayData* get_ts_union_types(const ArrayData* ts) {
  return detail::get_ts_varray(ts, s_union_types);
}

ALWAYS_INLINE const ArrayData* get_ts_union_types_opt(const ArrayData* ts) {
  return detail::get_ts_varray_opt(ts, s_union_types);
}

ALWAYS_INLINE const StringData* get_ts_classname(const ArrayData* ts) {
  return detail::get_ts_string(ts, s_classname);
}

ALWAYS_INLINE const StringData* get_ts_name(const ArrayData* ts) {
  return detail::get_ts_string(ts, s_name);
}

ALWAYS_INLINE const StringData* get_ts_root_name(const ArrayData* ts) {
  return detail::get_ts_string(ts, s_root_name);
}

ALWAYS_INLINE const StringData* get_ts_alias(const ArrayData* ts) {
  return detail::get_ts_string(ts, s_alias);
}

ALWAYS_INLINE const StringData* get_ts_case_type(const ArrayData* ts) {
  return detail::get_ts_string(ts, s_case_type);
}

ALWAYS_INLINE const StringData* get_ts_case_type_opt(const ArrayData* ts) {
  return detail::get_ts_string_opt(ts, s_case_type);
}

ALWAYS_INLINE const TypeStructure::Kind get_ts_kind(const ArrayData* ts) {
  auto const kind_field = ts->get(s_kind.get());
  assertx(isIntType(kind_field.type()));
  return static_cast<TypeStructure::Kind>(kind_field.val().num);
}

inline const ArrayData* get_ts_typevar_types_opt(const ArrayData* ts) {
  return detail::get_ts_darray_opt(ts, s_typevar_types);
}

ALWAYS_INLINE bool isValidTSType(TypedValue c, bool error) {
  if (!tvIsDict(c)) {
    if (error) raise_error("Type structure must be a darray");
    return false;
  }
  return true;
}

ALWAYS_INLINE bool isWildCard(const ArrayData* ts) {
  return get_ts_kind(ts) == TypeStructure::Kind::T_typevar &&
         ts->exists(s_name.get()) &&
         get_ts_name(ts)->equal(s_wildcard.get());
}

ALWAYS_INLINE
const StringData* get_ts_this_type_access(const ArrayData* ts) {
  if (get_ts_kind(ts) != TypeStructure::Kind::T_typeaccess ||
      !get_ts_root_name(ts)->tsame(s_hh_this.get()) ||
      is_ts_nullable(ts)) {
    return nullptr;
  }
  auto const accList = get_ts_access_list(ts);
  if (accList->size() != 1) return nullptr;
  auto const name = accList->at(int64_t(0));
  if (!tvIsString(&name)) return nullptr;
  return name.m_data.pstr;
}

///////////////////////////////////////////////////////////////////////////////

}
