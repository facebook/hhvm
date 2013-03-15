/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_BUILTIN_FUNCTIONS_H__
#define __HPHP_BUILTIN_FUNCTIONS_H__

#include <runtime/base/execution_context.h>
#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/binary_operations.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/intercept.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_observer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/util/request_local.h>
#include <util/case_insensitive.h>

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
extern StaticString s_self;
extern StaticString s_parent;
extern StaticString s_static;

// empty

inline bool empty(bool    v) { return !v;}
inline bool empty(char    v) { return !v;}
inline bool empty(short   v) { return !v;}
inline bool empty(int     v) { return !v;}
inline bool empty(int64_t   v) { return !v;}
inline bool empty(double  v) { return !v;}
inline bool empty(litstr  v) { return !v || !*v;}
inline bool empty(const StringData *v) { return v ? v->toBoolean() : false;}
inline bool empty(CStrRef v) { return !v.toBoolean();}
inline bool empty(CArrRef v) { return !v.toBoolean();}
inline bool empty(CObjRef v) { return !v.toBoolean();}
inline bool empty(CVarRef v) { return !v.toBoolean();}

bool empty(CVarRef v, bool    offset);
bool empty(CVarRef v, int64_t   offset);
inline bool empty(CVarRef v, int  offset) { return empty(v, (int64_t)offset); }
bool empty(CVarRef v, double  offset);
bool empty(CVarRef v, CArrRef offset);
bool empty(CVarRef v, CObjRef offset);
bool empty(CVarRef v, CVarRef offset);
bool empty(CVarRef v, litstr  offset, bool isString = false);
bool empty(CVarRef v, CStrRef offset, bool isString = false);

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
    raise_warning("Division by zero");
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
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  return s1 + s2;
}
inline String &concat_assign(String &s1, litstr s2)  {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  return s1 += s2;
}
inline String &concat_assign(String &s1, CStrRef s2) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  return s1 += s2;
}

#define MAX_CONCAT_ARGS 6
String concat3(CStrRef s1, CStrRef s2, CStrRef s3);
String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4);
String concat5(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5);
String concat6(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5,
               CStrRef s6);

inline Variant &concat_assign(Variant &v1, litstr s2) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

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
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

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

inline String &concat_assign(const StringOffset &s1, litstr s2) {
  return concat_assign(s1.lval(), s2);
}

inline String &concat_assign(const StringOffset &s1, CStrRef s2) {
  return concat_assign(s1.lval(), s2);
}

inline bool instanceOf(ObjectData *v, CStrRef s) {
  return v && v->o_instanceof(s);
}

inline bool instanceOf(bool    v, CStrRef s) { return false;}
inline bool instanceOf(char    v, CStrRef s) { return false;}
inline bool instanceOf(short   v, CStrRef s) { return false;}
inline bool instanceOf(int     v, CStrRef s) { return false;}
inline bool instanceOf(int64_t   v, CStrRef s) { return false;}
inline bool instanceOf(double  v, CStrRef s) { return false;}
inline bool instanceOf(litstr  v, CStrRef s) { return false;}
inline bool instanceOf(CStrRef v, CStrRef s) { return false;}
inline bool instanceOf(CArrRef v, CStrRef s) { return false;}
inline bool instanceOf(CObjRef v, CStrRef s) { return instanceOf(v.get(), s);}
inline bool instanceOf(CVarRef v, CStrRef s) {
  return v.is(KindOfObject) && instanceOf(v.getObjectData(), s);
}

template <class K, class V>
const V &String::set(K key, const V &value) {
  StringData *s = StringData::Escalate(m_px);
  SmartPtr<StringData>::operator=(s);
  m_px->setChar(toInt32(key), toString(value));
  return value;
}

///////////////////////////////////////////////////////////////////////////////
// output functions

inline int print(const char *s) {
  g_context->write(s);
  return 1;
}
inline int print(CStrRef s) {
  // print is not a real function. x_print exists, but this function gets called
  // directly. We therefore need to setup the TaintObserver.
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  g_context->write(s);
  return 1;
}
inline void echo(const char *s) {
  g_context->write(s);
}
inline void echo(CStrRef s) {
  // echo is not a real function. x_echo exists, but this function gets called
  // directly. We therefore need to setup the TaintObserver.
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  g_context->write(s);
}

String get_source_filename(litstr path,bool dir_component = false);

void NEVER_INLINE throw_invalid_property_name(CStrRef name) ATTRIBUTE_NORETURN;
void NEVER_INLINE throw_null_object_prop();
void NEVER_INLINE throw_null_get_object_prop();
void NEVER_INLINE raise_null_object_prop();
void throw_exception_unchecked(CObjRef e);
void throw_exception(CObjRef e);
bool set_line(int line0, int char0 = 0, int line1 = 0, int char1 = 0);

///////////////////////////////////////////////////////////////////////////////
// isset/unset

inline bool isInitialized(CVarRef v) { return v.isInitialized();}
Variant getDynamicConstant(CVarRef v, CStrRef name);
String getUndefinedConstant(CStrRef name);

inline bool isset(bool v)    { return true; }
inline bool isset(char v)    { return true; }
inline bool isset(short v)   { return true; }
inline bool isset(int v)     { return true; }
inline bool isset(int64_t v)   { return true; }
inline bool isset(double v)  { return true; }
inline bool isset(CVarRef v) { return !v.isNull();}
inline bool isset(CObjRef v) { return !v.isNull();}
inline bool isset(CStrRef v) { return !v.isNull();}
inline bool isset(CArrRef v) { return !v.isNull();}

bool isset(CVarRef v, bool    offset);
bool isset(CVarRef v, int64_t   offset);
inline bool isset(CVarRef v, int  offset) { return isset(v, (int64_t)offset); }
bool isset(CVarRef v, double  offset);
bool isset(CVarRef v, CArrRef offset);
bool isset(CVarRef v, CObjRef offset);
bool isset(CVarRef v, CVarRef offset);
bool isset(CVarRef v, litstr  offset, bool isString = false);
bool isset(CVarRef v, CStrRef offset, bool isString = false);

bool isset(CArrRef v, int64_t   offset);
inline bool isset(CArrRef v, bool   offset) { return isset(v, (int64_t)offset); }
inline bool isset(CArrRef v, int    offset) { return isset(v, (int64_t)offset); }
inline bool isset(CArrRef v, double offset) { return isset(v, (int64_t)offset); }
bool isset(CArrRef v, CArrRef offset);
bool isset(CArrRef v, CObjRef offset);
bool isset(CArrRef v, CVarRef offset);
bool isset(CArrRef v, litstr  offset, bool isString = false);
bool isset(CArrRef v, CStrRef offset, bool isString = false);

inline Variant unset(Variant &v)               { v.unset();   return uninit_null();}
inline Variant unset(CVarRef v)                {              return uninit_null();}
inline Variant setNull(Variant &v)             { v.setNull(); return uninit_null();}
inline Object setNull(Object &v)               { v.reset();   return Object();}
inline Array setNull(Array &v)                 { v.reset();   return Array();}
inline String setNull(String &v)               { v.reset();   return String();}
inline Variant unset(Object &v)                { v.reset();   return uninit_null();}
inline Variant unset(Array &v)                 { v.reset();   return uninit_null();}
///////////////////////////////////////////////////////////////////////////////
// special variable contexts

/**
 * lval() is mainly to make this work,
 *
 *   $arr['a']['b'] = $value;
 *
 * where $arr['a'] is in an l-value context. Note that lval() cannot replace
 * those offset classes, because calling these lval() functions will actually
 * insert a null value into an array/object, whereas an offset class can be
 * more powerful by not inserting a dummy value beforehand. For example,
 *
 *   isset($arr['a']); // we have to use offset's exists() function
 *   $obj['a'] = $value; // ArrayAccess's offset is completely customized
 *
 */
template<class T>
T &lval(T &v) { return v; }
inline Variant &lval(Variant &v) { return v;}
inline Array   &lval(Array   &v) { return v;}
inline Variant &lval(CVarRef  v) { // in case generating lval(1)
  throw FatalErrorException("taking reference from an r-value");
}
inline String  &lval(const StringOffset  &v) { return v.lval();}

template<class T>
Variant &unsetLval(Variant &v, const T &key) {
  if (v.isNull()) {
    return v;
  }
  if (v.is(KindOfArray)) {
    if (v.toArray().exists(key)) {
      return v.lvalAt(key);
    }
    return Variant::lvalBlackHole();
  }
  return Variant::lvalInvalid();
}

template<class T>
Variant &unsetLval(Array &v, const T &key) {
  if (!v.isNull() && v.exists(key)) {
    return v.lvalAt(key);
  }
  return Variant::lvalBlackHole();
}

///////////////////////////////////////////////////////////////////////////////
// misc functions

bool array_is_valid_callback(CArrRef arr);

bool class_exists(CStrRef class_name, bool autoload = true);
String get_static_class_name(CVarRef objOrClassName);

Variant f_call_user_func_array(CVarRef function, CArrRef params,
                               bool bound = false);

const HPHP::VM::Func*
vm_decode_function(CVarRef function,
                   HPHP::VM::ActRec* ar,
                   bool forwarding,
                   ObjectData*& this_,
                   HPHP::VM::Class*& cls,
                   StringData*& invName,
                   bool warn = true);
HPHP::VM::ActRec* vm_get_previous_frame();
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
Variant throw_missing_arguments(const char *fn, int num, int level = 0);
Variant throw_toomany_arguments(const char *fn, int num, int level = 0);
Variant throw_wrong_arguments(const char *fn, int count, int cmin, int cmax,
                              int level = 0);
Variant throw_missing_typed_argument(const char *fn, const char *type, int arg);
Variant throw_assign_this();

void throw_missing_arguments_nr(const char *fn, int num, int level = 0)
  __attribute__((cold));
void throw_toomany_arguments_nr(const char *fn, int num, int level = 0)
  __attribute__((cold));
void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax,
                              int level = 0) __attribute__((cold));

/**
 * When fatal coding errors are transformed to this function call.
 */
inline Object throw_fatal_object(const char *msg, void *dummy = nullptr) {
  throw FatalErrorException(msg);
}

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
void generate_request_timeout_exception() ATTRIBUTE_COLD;
void generate_memory_exceeded_exception() ATTRIBUTE_COLD;
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
 * Serialize/unserialize a variant into/from a string. We need these two
 * functions in runtime/base, as there are functions in runtime/base that depend on
 * these two functions.
 */
String f_serialize(CVarRef value);
Variant unserialize_ex(CStrRef str, VariableUnserializer::Type type);
Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type);

inline Variant unserialize_from_buffer(const char* str, int len) {
  return unserialize_ex(str, len, VariableUnserializer::Serialize);
}

inline Variant f_unserialize(CStrRef str) {
  return unserialize_from_buffer(str.data(), str.size());
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

/**
 * For wrapping expressions that have no effect, so to make gcc happy.
 */
inline bool    id(bool    v) { return v; }
inline char    id(char    v) { return v; }
inline short   id(short   v) { return v; }
inline int     id(int     v) { return v; }
inline int64_t   id(int64_t   v) { return v; }
inline uint64_t  id(uint64_t  v) { return v; }
inline double  id(double  v) { return v; }
inline litstr  id(litstr  v) { return v; }
inline CStrRef id(CStrRef v) { return v; }
inline CArrRef id(CArrRef v) { return v; }
inline CObjRef id(CObjRef v) { return v; }
inline CVarRef id(CVarRef v) { return v; }
template <class T>
inline const SmartObject<T> &id(const SmartObject<T> &v) { return v; }

/**
 * For wrapping return values to prevent elision of copy
 * constructors. This can be a problem if the function
 * returns by value, but a "referenced" variant is returned
 * through copy-constructor elision.
 */
inline Variant wrap_variant(CVarRef x) { return x; }

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
  bool autoloadFunc(CStrRef name);
  bool autoloadConstant(CStrRef name);
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

#endif // __HPHP_BUILTIN_FUNCTIONS_H__
