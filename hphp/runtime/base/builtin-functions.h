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

#ifndef incl_HPHP_BUILTIN_FUNCTIONS_H_
#define incl_HPHP_BUILTIN_FUNCTIONS_H_

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/util/functional.h"
#include "hphp/util/portability.h"
#include "hphp/runtime/base/req-root.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
extern const StaticString s_self;
extern const StaticString s_parent;
extern const StaticString s_static;

extern const StaticString s_cmpWithCollection;
extern const StaticString s_cmpWithVec;
extern const StaticString s_cmpWithDict;
extern const StaticString s_cmpWithKeyset;
extern const StaticString s_cmpWithRecord;

///////////////////////////////////////////////////////////////////////////////
// operators

inline String concat(const String& s1, const String& s2) {
  return s1 + s2;
}

String concat3(const String& s1, const String& s2, const String& s3);
String concat4(const String& s1, const String& s2, const String& s3,
               const String& s4);

///////////////////////////////////////////////////////////////////////////////

[[noreturn]] void NEVER_INLINE throw_missing_this(const Func* f);
[[noreturn]] void NEVER_INLINE throw_has_this_need_static(const Func* f);
void NEVER_INLINE throw_invalid_property_name(const String& name);

[[noreturn]]
void throw_exception(const Object& e);

///////////////////////////////////////////////////////////////////////////////
// type testing

inline bool is_null(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsNull(c);
}

inline bool is_bool(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsBool(c);
}

inline bool is_int(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsInt(c);
}

inline bool is_double(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsDouble(c);
}

inline bool is_string(const Cell* c) {
  if (tvIsString(c)) return true;
  if (tvIsFunc(c)) {
    if (RuntimeOption::EvalIsStringNotices) {
      raise_notice("Func used in is_string");
    }
    return true;
  }
  if (tvIsClass(c)) {
    if (RuntimeOption::EvalIsStringNotices) {
      raise_notice("Class used in is_string");
    }
    return true;
  }
  return false;
}

inline bool is_array(const Cell* c) {
  assertx(cellIsPlausible(*c));
  if (tvIsClsMeth(c)) {
    if (!RuntimeOption::EvalHackArrDVArrs) {
      if (RuntimeOption::EvalIsVecNotices) {
        raise_notice(Strings::CLSMETH_COMPAT_IS_ARR);
      }
      return true;
    }
    return false;
  }
  return tvIsArrayOrShape(c);
}

inline bool is_vec(const Cell* c) {
  assertx(cellIsPlausible(*c));
  if (tvIsClsMeth(c)) {
    if (RuntimeOption::EvalHackArrDVArrs) {
      if (RuntimeOption::EvalIsVecNotices) {
        raise_notice(Strings::CLSMETH_COMPAT_IS_VEC);
      }
      return true;
    }
    return false;
  }
  return tvIsVec(c);
}

inline bool is_dict(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsDictOrShape(c);
}

inline bool is_keyset(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsKeyset(c);
}

inline bool is_varray(const Cell* c) {
  if (tvIsClsMeth(c)) {
    if (!RuntimeOption::EvalHackArrDVArrs) {
      if (RuntimeOption::EvalIsVecNotices) {
        raise_notice(Strings::CLSMETH_COMPAT_IS_VARR);
      }
      return true;
    }
    return false;
  }
  return RuntimeOption::EvalHackArrDVArrs
    ? tvIsVec(c)
    : (tvIsArray(c) && c->m_data.parr->isVArray());
}

inline bool is_darray(const Cell* c) {
  return RuntimeOption::EvalHackArrDVArrs
    ? tvIsDictOrShape(c)
    : (tvIsArray(c) && c->m_data.parr->isDArray());
}

inline bool is_object(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsObject(c) &&
    c->m_data.pobj->getVMClass() != SystemLib::s___PHP_Incomplete_ClassClass;
}

inline bool is_clsmeth(const Cell* c) {
  assertx(cellIsPlausible(*c));
  return tvIsClsMeth(c);
}

inline bool is_empty_string(const Cell* c) {
  return tvIsString(c) && c->m_data.pstr->empty();
}

///////////////////////////////////////////////////////////////////////////////
// misc functions

/*
 * Semantics of is_callable defined here:
 * http://php.net/manual/en/function.is-callable.php
 */
bool is_callable(const Variant& v, bool syntax_only, RefData* name);
/*
 * Equivalent to is_callable(v, false, nullptr)
 */
bool is_callable(const Variant& v);
bool array_is_valid_callback(const Array& arr);

enum class DecodeFlags { Warn, NoWarn, LookupOnly };
const HPHP::Func*
vm_decode_function(const_variant_ref function,
                   ActRec* ar,
                   ObjectData*& this_,
                   HPHP::Class*& cls,
                   StringData*& invName,
                   bool& dynamic,
                   ArrayData*& reifiedGenerics,
                   DecodeFlags flags = DecodeFlags::Warn);

inline void
vm_decode_function(const_variant_ref function,
                   CallCtx& ctx,
                   DecodeFlags flags = DecodeFlags::Warn) {
  ctx.func = vm_decode_function(function, nullptr, ctx.this_, ctx.cls,
                                ctx.invName, ctx.dynamic, ctx.reifiedGenerics,
                                flags);
}

std::pair<Class*, Func*> decode_for_clsmeth(
  const String& clsName,
  const String& funcName,
  ActRec* ar,
  StringData*& invName,
  DecodeFlags flags = DecodeFlags::Warn);

Variant vm_call_user_func(const_variant_ref function, const Variant& params,
                          bool checkRef = false);
template<typename T>
Variant vm_call_user_func(T&& t, const Variant& params,
                          bool checkRef = false) {
  const Variant function{std::forward<T>(t)};
  return vm_call_user_func(
    const_variant_ref{function}, params, checkRef
  );
}

Variant invoke_static_method(const String& s, const String& method,
                             const Variant& params, bool fatal = true);

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal = true);

bool is_constructor_name(const char* func);
void throw_instance_method_fatal(const char *name);

[[noreturn]] void throw_invalid_collection_parameter();
[[noreturn]] void throw_invalid_operation_exception(StringData*);
[[noreturn]] void throw_arithmetic_error(StringData*);
[[noreturn]] void throw_division_by_zero_error(StringData*);
[[noreturn]] void throw_division_by_zero_exception();
[[noreturn]] void throw_iterator_not_valid();
[[noreturn]] void throw_collection_property_exception();
[[noreturn]] void throw_collection_compare_exception();
[[noreturn]] void throw_vec_compare_exception();
[[noreturn]] void throw_dict_compare_exception();
[[noreturn]] void throw_keyset_compare_exception();
[[noreturn]] void throw_record_compare_exception();
[[noreturn]] void throw_rec_non_rec_compare_exception();
[[noreturn]] void throw_param_is_not_container();
[[noreturn]] void throw_invalid_inout_base();
[[noreturn]] void throw_cannot_modify_immutable_object(const char* className);
[[noreturn]] void throw_cannot_modify_const_object(const char* className);
[[noreturn]] void throw_object_forbids_dynamic_props(const char* className);
[[noreturn]] void throw_cannot_modify_const_prop(const char* className,
                                                 const char* propName);
[[noreturn]] void throw_late_init_prop(const Class* cls,
                                       const StringData* propName,
                                       bool isSProp);
[[noreturn]] void throw_parameter_wrong_type(TypedValue tv,
                                             const Func* callee,
                                             unsigned int arg_num,
                                             DataType type);

void raise_soft_late_init_prop(const Class* cls,
                               const StringData* propName,
                               bool isSProp);

void check_collection_cast_to_array();

Object create_object_only(const String& s);
Object create_object(const String& s, const Array &params, bool init = true);
Object init_object(const String& s, const Array &params, ObjectData* o);

[[noreturn]] void throw_object(const Object& e);
#if ((__GNUC__ != 4) || (__GNUC_MINOR__ != 8))
// gcc-4.8 has a bug that causes incorrect code if we
// define this function.
[[noreturn]] void throw_object(Object&& e);
#endif

[[noreturn]] inline
void throw_object(const String& s, const Array& params, bool init = true) {
  throw_object(create_object(s, params, init));
}

/**
 * Argument count handling.
 *   - When level is 2, it's from constructors that turn these into fatals
 *   - When level is 1, it's from system funcs that turn both into warnings
 *   - When level is 0, it's from user funcs that turn missing arg in warnings
 */
void throw_wrong_argument_count_nr(const char *fn, int expected, int got,
                                   const char *expectDesc)
  __attribute__((__cold__));
void throw_missing_arguments_nr(const char *fn, int expected, int got)

  __attribute__((__cold__));
void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax)
  __attribute__((__cold__));

/**
 * Handler for exceptions thrown from user functions that we don't
 * allow exception propagation from.  E.g., object destructors or
 * certain callback hooks (user profiler). Implemented in
 * program-functions.cpp.
 */
void handle_destructor_exception(const char* situation = "Destructor");

/*
 * Deprecated wrappers for raising certain types of warnings.
 *
 * Don't use in new code.
 */
void throw_bad_type_exception(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1,2);
void throw_expected_array_exception(const char* fn = nullptr);
void throw_expected_array_or_collection_exception(const char* fn = nullptr);
void throw_invalid_argument(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1,2) __attribute__((__cold__));

/**
 * Unsetting ClassName::StaticProperty.
 */
Variant throw_fatal_unset_static_property(const char *s, const char *prop);

// unserializable default value arguments such as TimeStamp::Current()
// are serialized as "\x01"
char const kUnserializableString[] = "\x01";

/**
 * Serialize/unserialize a variant into/from a string. We need these
 * two functions in runtime/base, as there are functions in
 * runtime/base that depend on these two functions.
 */
String f_serialize(const Variant& value);
Variant unserialize_ex(const String& str,
                       VariableUnserializer::Type type,
                       const Array& options = null_array);
Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type,
                       const Array& options = null_array);

inline Variant unserialize_from_buffer(const char* str, int len,
                                       VariableUnserializer::Type type,
                                       const Array& options = null_array) {
  return unserialize_ex(str, len, type, options);
}

inline Variant unserialize_from_string(const String& str,
                                       VariableUnserializer::Type type,
                                       const Array& options = null_array) {
  return unserialize_from_buffer(str.data(), str.size(), type, options);
}

String resolve_include(const String& file, const char* currentDir,
                       bool (*tryFile)(const String& file, void* ctx),
                       void* ctx);
Variant include_impl_invoke(const String& file, bool once = false,
                            const char *currentDir = "",
                            bool callByHPHPInvoke = false);
Variant require(const String& file, bool once, const char* currentDir,
                bool raiseNotice);

bool function_exists(const String& function_name);

///////////////////////////////////////////////////////////////////////////////
}

#endif
