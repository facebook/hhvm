/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/util/functional.h"
#include "hphp/runtime/base/type-conversions.h"

#if defined(__APPLE__) || defined(__USE_BSD)
/**
 * We don't actually use param.h in this file,
 * but other files which use us do, and we want
 * to enforce clearing of the isset macro from
 * that header by handling the header now
 * and wiping it out.
 */
# include <sys/param.h>
#include <algorithm>
# ifdef isset
#  undef isset
# endif
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
extern const StaticString s_self;
extern const StaticString s_parent;
extern const StaticString s_static;

///////////////////////////////////////////////////////////////////////////////
// operators

inline String concat(const String& s1, const String& s2) {
  return s1 + s2;
}

String concat3(const String& s1, const String& s2, const String& s3);
String concat4(const String& s1, const String& s2, const String& s3,
               const String& s4);

///////////////////////////////////////////////////////////////////////////////

void NEVER_INLINE throw_invalid_property_name(const String& name);
void NEVER_INLINE throw_null_object_prop();
void NEVER_INLINE throw_null_get_object_prop();
void NEVER_INLINE raise_null_object_prop();
void throw_exception(const Object& e);

///////////////////////////////////////////////////////////////////////////////
// type testing

inline bool is_null(const Variant& v)   { return v.isNull();}
inline bool is_not_null(const Variant& v) { return !v.isNull();}
inline bool is_bool(const Variant& v)   { return v.is(KindOfBoolean);}
inline bool is_int(const Variant& v)    { return v.isInteger();}
inline bool is_double(const Variant& v) { return v.is(KindOfDouble);}
inline bool is_string(const Variant& v) { return v.isString();}
inline bool is_array(const Variant& v)  { return v.is(KindOfArray);}
inline bool is_object(const Variant& var) { return var.is(KindOfObject); }
inline bool is_empty_string(const Variant& v) {
  return v.isString() && v.getStringData()->empty();
}

///////////////////////////////////////////////////////////////////////////////
// misc functions

bool array_is_valid_callback(const Array& arr);

Variant f_call_user_func_array(const Variant& function, const Array& params,
                               bool bound = false);

const HPHP::Func*
vm_decode_function(const Variant& function,
                   ActRec* ar,
                   bool forwarding,
                   ObjectData*& this_,
                   HPHP::Class*& cls,
                   StringData*& invName,
                   bool warn = true);

inline void
vm_decode_function(const Variant& function,
                   ActRec* ar,
                   bool forwarding,
                   CallCtx& ctx,
                   bool warn = true) {
  ctx.func = vm_decode_function(function, ar, forwarding, ctx.this_, ctx.cls,
                                ctx.invName, warn);
}

Variant vm_call_user_func(const Variant& function, const Variant& params,
                          bool forwarding = false);

Variant invoke_static_method(const String& s, const String& method,
                             const Variant& params, bool fatal = true);

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal = true);

void throw_instance_method_fatal(const char *name);

void throw_iterator_not_valid() ATTRIBUTE_NORETURN;
void throw_collection_modified() ATTRIBUTE_NORETURN;
void throw_collection_property_exception() ATTRIBUTE_NORETURN;
void throw_collection_compare_exception() ATTRIBUTE_NORETURN;
void throw_param_is_not_container() ATTRIBUTE_NORETURN;
void throw_cannot_modify_immutable_object(const char* className)
  ATTRIBUTE_NORETURN;
void check_collection_compare(ObjectData* obj);
void check_collection_compare(ObjectData* obj1, ObjectData* obj2);
void check_collection_cast_to_array();

Object create_object_only(const String& s);
Object create_object(const String& s, const Array &params, bool init = true);

/**
 * Argument count handling.
 *   - When level is 2, it's from constructors that turn these into fatals
 *   - When level is 1, it's from system funcs that turn both into warnings
 *   - When level is 0, it's from user funcs that turn missing arg in warnings
 */
void throw_missing_arguments_nr(const char *fn, int expected, int got,
                                int level = 0, TypedValue *rv = nullptr)
  __attribute__((cold));
void throw_missing_min_arguments_nr(const char *fn, int expected, int got,
                                    int level = 0, TypedValue *rv = nullptr)
  __attribute__((cold));
void throw_toomany_arguments_nr(const char *fn, int num, int level = 0,
                                TypedValue *rv = nullptr)
  __attribute__((cold));
void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax,
                              int level = 0, TypedValue *rv = nullptr)
  __attribute__((cold));

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
void throw_bad_type_exception(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
void throw_expected_array_exception();
void throw_expected_array_or_collection_exception();
void throw_invalid_argument(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2)
   __attribute__((cold));

/**
 * Unsetting ClassName::StaticProperty.
 */
Variant throw_fatal_unset_static_property(const char *s, const char *prop);

/**
 * Exceptions injected code throws
 */
Exception* generate_request_timeout_exception();
Exception* generate_memory_exceeded_exception();

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
                       const Array& class_whitelist = null_array);
Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type,
                       const Array& class_whitelist = null_array);

inline Variant unserialize_from_buffer(const char* str, int len,
                                       const Array& class_whitelist = null_array) {
  return unserialize_ex(str, len,
                        VariableUnserializer::Type::Serialize,
                        class_whitelist);
}

inline Variant unserialize_from_string(const String& str,
                                       const Array& class_whitelist = null_array) {
  return unserialize_from_buffer(str.data(), str.size(), class_whitelist);
}

String resolve_include(const String& file, const char* currentDir,
                       bool (*tryFile)(const String& file, void* ctx),
                       void* ctx);
Variant include_impl_invoke(const String& file, bool once = false,
                            const char *currentDir = "");
Variant require(const String& file, bool once, const char* currentDir,
                bool raiseNotice);

bool function_exists(const String& function_name);

///////////////////////////////////////////////////////////////////////////////
}

#endif
