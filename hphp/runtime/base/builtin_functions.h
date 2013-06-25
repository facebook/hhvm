/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/execution_context.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/binary_operations.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/runtime_error.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/variable_unserializer.h"
#include "hphp/runtime/base/util/request_local.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/util/case_insensitive.h"
#include "hphp/runtime/base/type_conversions.h"

#if defined(__APPLE__) || defined(__USE_BSD)
/**
 * We don't actually use param.h in this file,
 * but other files which use us do, and we want
 * to enforce clearing of the isset macro from
 * that header by handling the header now
 * and wiping it out.
 */
# include <sys/param.h>
# ifdef isset
#  undef isset
# endif
#endif

/**
 * This file contains a list of functions that HPHP generates to wrap around
 * different expressions to maintain semantics. If we read through all types of
 * expressions in compiler/expression/expression.h, we can find most of them can be
 * directly transformed into C/C++ counterpart without too much syntactical
 * changes. The functions in this file happen to be the ones that are somewhat
 * special.
 *
 * Another way to think about this file is that this file has a list of C-style
 * functions, and the rest of run-time has object/classes for other tasks,
 * although we do have some global functions defined in other files as well,
 * when they are closer to the classes/objects in the same files.
 */

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
extern const StaticString s_self;
extern const StaticString s_parent;
extern const StaticString s_static;

// empty
inline bool empty(CVarRef v) { return !v.toBoolean();}

bool empty(bool) = delete;
bool empty(char) = delete;
bool empty(short) = delete;
bool empty(int) = delete;
bool empty(int64_t) = delete;
bool empty(double) = delete;
bool empty(litstr) = delete;
bool empty(const StringData*) = delete;
bool empty(CStrRef) = delete;
bool empty(CArrRef) = delete;
bool empty(CObjRef) = delete;

bool empty(CVarRef v, bool) = delete;
bool empty(CVarRef v, int64_t) = delete;
bool empty(CVarRef v, int) = delete;
bool empty(CVarRef v, double) = delete;
bool empty(CVarRef v, CArrRef) = delete;
bool empty(CVarRef v, CObjRef) = delete;
bool empty(CVarRef v, CVarRef) = delete;
bool empty(CVarRef v, CStrRef, bool = false) = delete;

///////////////////////////////////////////////////////////////////////////////
// operators

/**
 * These functions are rarely performance bottlenecks, so we are not fully
 * type-specialized, although we could.
 */

inline bool logical_xor(bool v1, bool v2) { return (v1 ? 1:0) ^ (v2 ? 1:0);}
inline Variant bitwise_or (CVarRef v1, CVarRef v2) { return v1 | v2;}
inline Variant bitwise_and(CVarRef v1, CVarRef v2) { return v1 & v2;}
inline Variant bitwise_xor(CVarRef v1, CVarRef v2) { return v1 ^ v2;}
inline Numeric multiply(CVarRef v1, CVarRef v2)    { return v1 * v2;}
inline Variant plus(CVarRef v1, CVarRef v2)        { return v1 + v2;}
inline Numeric minus(CVarRef v1, CVarRef v2)       { return v1 - v2;}
inline Numeric divide(CVarRef v1, CVarRef v2)      { return v1 / v2; }
inline Numeric modulo(int64_t v1, int64_t v2) {
  if (UNLIKELY(v2 == 0)) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    return false;
  }
  if (UNLIKELY(uint64_t(v2+1) <= 2u)) {
    return 0;
  }
  return v1 % v2;
}
inline int64_t shift_left(int64_t v1, int64_t v2)        { return v1 << v2; }
inline int64_t shift_right(int64_t v1, int64_t v2)       { return v1 >> v2; }

inline char    negate(char v)    { return -v; }
inline short   negate(short v)   { return -v; }
inline int     negate(int v)     { return -v; }
inline int64_t   negate(int64_t v)   { return -v; }
inline double  negate(double v)  { return -v; }
inline Variant negate(CVarRef v) { return -(Variant)v; }

inline String concat(CStrRef s1, CStrRef s2)         {
  return s1 + s2;
}
inline String &concat_assign(String &s1, litstr s2)  {
  return s1 += s2;
}
inline String &concat_assign(String &s1, CStrRef s2) {
  return s1 += s2;
}

String concat3(CStrRef s1, CStrRef s2, CStrRef s3);
String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4);

inline Variant &concat_assign(Variant &v1, litstr s2) {
  if (v1.getType() == KindOfString) {
    StringData *data = v1.getStringData();
    if (data->getCount() == 1) {
      data->append(s2, strlen(s2));
      return v1;
    }
  }
  String s1 = v1.toString();
  s1 += s2;
  v1 = s1;
  return v1;
}

inline Variant &concat_assign(Variant &v1, CStrRef s2) {
  if (v1.getType() == KindOfString) {
    StringData *data = v1.getStringData();
    if (data->getCount() == 1) {
      data->append(s2.data(), s2.size());
      return v1;
    }
  }
  String s1 = v1.toString();
  s1 += s2;

  v1 = s1;
  return v1;
}

template <class K, class V>
const V &String::set(K key, const V &value) {
  StringData *s = StringData::Escalate(m_px);
  SmartPtr<StringData>::operator=(s);
  m_px->setChar(HPHP::toInt32(key), toString(value));
  return value;
}

///////////////////////////////////////////////////////////////////////////////
// output functions

inline void echo(const char *s) {
  g_context->write(s);
}
inline void echo(CStrRef s) {
  g_context->write(s);
}

void NEVER_INLINE throw_invalid_property_name(CStrRef name) ATTRIBUTE_NORETURN;
void NEVER_INLINE throw_null_object_prop();
void NEVER_INLINE throw_null_get_object_prop();
void NEVER_INLINE raise_null_object_prop();
void throw_exception(CObjRef e);

///////////////////////////////////////////////////////////////////////////////
// isset/unset

inline bool isInitialized(CVarRef v) { return v.isInitialized();}

inline bool isset(CVarRef v) { return !v.isNull(); }

bool isset(bool v)    = delete;
bool isset(char v)    = delete;
bool isset(short v)   = delete;
bool isset(int v)     = delete;
bool isset(int64_t v) = delete;
bool isset(double v)  = delete;
bool isset(CObjRef v) = delete;
bool isset(CStrRef v) = delete;
bool isset(CArrRef v) = delete;
bool isset(CVarRef v, bool) = delete;
bool isset(CVarRef v, int64_t) = delete;
bool isset(CVarRef v, int) = delete;
bool isset(CVarRef v, double) = delete;
bool isset(CVarRef v, CArrRef) = delete;
bool isset(CVarRef v, CObjRef) = delete;
bool isset(CVarRef v, CVarRef) = delete;
bool isset(CVarRef v, CStrRef, bool = false) = delete;

bool isset(CArrRef v, int64_t) = delete;
bool isset(CArrRef v, bool)  = delete;
bool isset(CArrRef v, int) = delete;
bool isset(CArrRef v, double) = delete;
bool isset(CArrRef v, CArrRef) = delete;
bool isset(CArrRef v, CObjRef) = delete;
bool isset(CArrRef v, CVarRef) = delete;
bool isset(CArrRef v, CStrRef, bool = false) = delete;

inline Variant unset(Variant &v)   { v.unset();   return uninit_null();}
inline Variant unset(CVarRef v)    {              return uninit_null();}
inline Variant setNull(Variant &v) { v.setNull(); return uninit_null();}
inline Object setNull(Object &v)   { v.reset();   return Object();}
inline Array setNull(Array &v)     { v.reset();   return Array();}
inline String setNull(String &v)   { v.reset();   return String();}
inline Variant unset(Object &v)    { v.reset();   return uninit_null();}
inline Variant unset(Array &v)     { v.reset();   return uninit_null();}

///////////////////////////////////////////////////////////////////////////////
// type testing

inline bool is_null(CVarRef v)   { return v.isNull();}
inline bool is_bool(CVarRef v)   { return v.is(KindOfBoolean);}
inline bool is_int(CVarRef v)    { return v.isInteger();}
inline bool is_double(CVarRef v) { return v.is(KindOfDouble);}
inline bool is_string(CVarRef v) { return v.isString();}
inline bool is_array(CVarRef v)  { return v.is(KindOfArray);}
inline bool is_object(CVarRef var) {
  // NB: just doing !var.isResource() is not right. isResource can have custom
  // implementations in extension classes, and even if that returns false,
  // is_object is still supposed to return false. Because PHP.
  return var.is(KindOfObject) &&
    var.getObjectData()->getVMClass() != SystemLib::s_resourceClass;
}
inline bool is_empty_string(CVarRef v) {
  return v.isString() && v.getStringData()->empty();
}

bool interface_supports_array(const StringData* s);
bool interface_supports_array(const std::string& n);

///////////////////////////////////////////////////////////////////////////////
// misc functions

bool array_is_valid_callback(CArrRef arr);

Variant f_call_user_func_array(CVarRef function, CArrRef params,
                               bool bound = false);

const HPHP::Func*
vm_decode_function(CVarRef function,
                   ActRec* ar,
                   bool forwarding,
                   ObjectData*& this_,
                   HPHP::Class*& cls,
                   StringData*& invName,
                   bool warn = true);

inline void
vm_decode_function(CVarRef function,
                   ActRec* ar,
                   bool forwarding,
                   CallCtx& ctx,
                   bool warn = true) {
  ctx.func = vm_decode_function(function, ar, forwarding, ctx.this_, ctx.cls,
                                ctx.invName, warn);
}

ActRec* vm_get_previous_frame();
Variant vm_call_user_func(CVarRef function, CArrRef params,
                          bool forwarding = false);

/**
 * Invoking an arbitrary static method.
 */
Variant invoke_static_method(CStrRef s, CStrRef method,
                             CArrRef params, bool fatal = true);

/**
 * Fallback when a dynamic function call fails to find a user function
 * matching the name.  If no handlers are able to
 * invoke the function, throw an InvalidFunctionCallException.
 */
Variant invoke_failed(const char *func, CArrRef params,
                      bool fatal = true);
Variant invoke_failed(CVarRef func, CArrRef params,
                      bool fatal = true);

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal = true);

/**
 * When fatal coding errors are transformed to this function call.
 */
inline Variant throw_fatal(const char *msg, void *dummy = nullptr) {
  throw FatalErrorException(msg);
}
inline Variant throw_missing_class(const char *cls) {
  throw ClassNotFoundException((std::string("unknown class ") + cls).c_str());
}

inline Variant throw_missing_file(const char *cls) {
  throw PhpFileDoesNotExistException(cls);
}
void throw_instance_method_fatal(const char *name);

void throw_iterator_not_valid() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
void throw_collection_modified() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
void throw_collection_property_exception() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
void throw_collection_compare_exception() ATTRIBUTE_COLD;
void check_collection_compare(ObjectData* obj);
void check_collection_compare(ObjectData* obj1, ObjectData* obj2);
void check_collection_cast_to_array();

Object create_object_only(CStrRef s);
Object create_object(CStrRef s, const Array &params, bool init = true);

/**
 * Argument count handling.
 *   - When level is 2, it's from constructors that turn these into fatals
 *   - When level is 1, it's from system funcs that turn both into warnings
 *   - When level is 0, it's from user funcs that turn missing arg in warnings
 */
void throw_missing_arguments_nr(const char *fn, int expected, int got,
                                int level = 0)
  __attribute__((cold));
void throw_toomany_arguments_nr(const char *fn, int num, int level = 0)
  __attribute__((cold));
void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax,
                              int level = 0) __attribute__((cold));
void throw_unexpected_argument_type(int argNum, const char *fnName,
                                    const char *expected, CVarRef val);

/**
 * Handler for exceptions thrown from user functions that we don't
 * allow exception propagation from.  E.g., object destructors or
 * certain callback hooks (user profiler). Implemented in
 * program_functions.cpp.
 */
void handle_destructor_exception(const char* situation = "Destructor");

/**
 * If RuntimeOption::ThrowBadTypeExceptions is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowBadTypeExceptions is off, we will log a
 * warning and swallow the error.
 */
void throw_bad_type_exception(const char *fmt, ...);
void throw_bad_array_exception();

/**
 * If RuntimeOption::ThrowInvalidArguments is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowInvalidArguments is off, we will log a
 * warning and swallow the error.
 */
void throw_invalid_argument(const char *fmt, ...);

/**
 * Unsetting ClassName::StaticProperty.
 */
Variant throw_fatal_unset_static_property(const char *s, const char *prop);

/**
 * Exceptions injected code throws
 */
void throw_infinite_recursion_exception() ATTRIBUTE_COLD;
Exception* generate_request_timeout_exception() ATTRIBUTE_COLD;
Exception* generate_memory_exceeded_exception() ATTRIBUTE_COLD;
void throw_call_non_object() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
void throw_call_non_object(const char *methodName)
  ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

/**
 * Cloning an object.
 */
Object f_clone(CVarRef v);

// unserializable default value arguments such as TimeStamp::Current()
// are serialized as "\x01"
char const kUnserializableString[] = "\x01";

/**
 * Serialize/unserialize a variant into/from a string. We need these
 * two functions in runtime/base, as there are functions in
 * runtime/base that depend on these two functions.
 */
String f_serialize(CVarRef value);
Variant unserialize_ex(CStrRef str,
                       VariableUnserializer::Type type,
                       CArrRef class_whitelist = null_array);
Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type,
                       CArrRef class_whitelist = null_array);

inline Variant unserialize_from_buffer(const char* str, int len,
                                       CArrRef class_whitelist = null_array) {
  return unserialize_ex(str, len,
                        VariableUnserializer::Type::Serialize,
                        class_whitelist);
}

inline Variant unserialize_from_string(CStrRef str,
                                       CArrRef class_whitelist = null_array) {
  return unserialize_from_buffer(str.data(), str.size(), class_whitelist);
}

String resolve_include(CStrRef file, const char* currentDir,
                       bool (*tryFile)(CStrRef file, void* ctx), void* ctx);
Variant include(CStrRef file, bool once = false,
                const char *currentDir = "",
                bool raiseNotice = true);
Variant require(CStrRef file, bool once = false,
                const char *currentDir = "",
                bool raiseNotice = true);
Variant include_impl_invoke(CStrRef file, bool once = false,
                            const char *currentDir = "");
Variant invoke_file(CStrRef file, bool once = false,
                    const char *currentDir = nullptr);
bool invoke_file_impl(Variant &res, CStrRef path, bool once,
                      const char *currentDir);

bool function_exists(CStrRef function_name);

/**
 * For autoload support
 */

class AutoloadHandler : public RequestEventHandler {
  enum Result {
    Failure,
    Success,
    StopAutoloading,
    ContinueAutoloading
  };

public:
  virtual void requestInit();
  virtual void requestShutdown();

  CArrRef getHandlers() { return m_handlers; }
  bool addHandler(CVarRef handler, bool prepend);
  void removeHandler(CVarRef handler);
  void removeAllHandlers();
  bool isRunning();

  bool invokeHandler(CStrRef className, bool forceSplStack = false);
  bool autoloadFunc(StringData* name);
  bool autoloadConstant(StringData* name);
  bool autoloadType(CStrRef name);
  bool setMap(CArrRef map, CStrRef root);
  DECLARE_STATIC_REQUEST_LOCAL(AutoloadHandler, s_instance);

private:
  template <class T>
  Result loadFromMap(CStrRef name, CStrRef kind, bool toLower,
                     const T &checkExists);
  static String getSignature(CVarRef handler);

  Array m_map;
  String m_map_root;
  Array m_handlers;
  bool m_running;
};

#define CALL_USER_FUNC_FEW_ARGS_COUNT 6

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_BUILTIN_FUNCTIONS_H_
