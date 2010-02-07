/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/base/execution_context.h>
#include <cpp/base/types.h>
#include <cpp/base/type_string.h>
#include <cpp/base/type_array.h>
#include <cpp/base/type_variant.h>
#include <cpp/base/type_object.h>
#include <cpp/base/string_offset.h>
#include <cpp/base/object_offset.h>
#include <cpp/base/frame_injection.h>
#include <util/logger.h>

/**
 * This file contains a list of functions that HPHP generates to wrap around
 * different expressions to maintain semantics. If we read through all types of
 * expressions in lib/expression/expression.h, we can find most of them can be
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
// type conversion functions

inline bool toBoolean(bool    v) { return v;}
inline bool toBoolean(char    v) { return v;}
inline bool toBoolean(short   v) { return v;}
inline bool toBoolean(int     v) { return v;}
inline bool toBoolean(int64   v) { return v;}
inline bool toBoolean(double  v) { return v;}
inline bool toBoolean(litstr  v) { return v && *v;}
inline bool toBoolean(CStrRef v) { return v.toBoolean();}
inline bool toBoolean(CArrRef v) { return v.toBoolean();}
inline bool toBoolean(CObjRef v) { return v.toBoolean();}
inline bool toBoolean(CVarRef v) { return v.toBoolean();}

inline bool empty(bool    v) { return !toBoolean(v);}
inline bool empty(char    v) { return !toBoolean(v);}
inline bool empty(short   v) { return !toBoolean(v);}
inline bool empty(int     v) { return !toBoolean(v);}
inline bool empty(int64   v) { return !toBoolean(v);}
inline bool empty(double  v) { return !toBoolean(v);}
inline bool empty(litstr  v) { return !toBoolean(v);}
inline bool empty(CStrRef v) { return !toBoolean(v);}
inline bool empty(CArrRef v) { return !toBoolean(v);}
inline bool empty(CObjRef v) { return !toBoolean(v);}
inline bool empty(CVarRef v) { return !toBoolean(v);}

bool empty(CVarRef v, const CVarRef offset, int64 prehash = -1);

inline char toByte(bool    v) { return v ? 1 : 0;}
inline char toByte(char    v) { return v;}
inline char toByte(short   v) { return v;}
inline char toByte(int     v) { return v;}
inline char toByte(int64   v) { return v;}
inline char toByte(double  v) { return (char)v;}
inline char toByte(litstr  v) { return StringData(v).toByte();}
inline char toByte(CStrRef v) { return v.toByte();}
inline char toByte(CArrRef v) { return v.toByte();}
inline char toByte(CObjRef v) { return v.toByte();}
inline char toByte(CVarRef v) { return v.toByte();}

inline short toInt16(bool    v) { return v ? 1 : 0;}
inline short toInt16(char    v) { return v;}
inline short toInt16(short   v) { return v;}
inline short toInt16(int     v) { return v;}
inline short toInt16(int64   v) { return v;}
inline short toInt16(double  v) { return (short)v;}
inline short toInt16(litstr  v) { return StringData(v).toInt16();}
inline short toInt16(CStrRef v) { return v.toInt16();}
inline short toInt16(CArrRef v) { return v.toInt16();}
inline short toInt16(CObjRef v) { return v.toInt16();}
inline short toInt16(CVarRef v) { return v.toInt16();}

inline int toInt32(bool    v) { return v ? 1 : 0;}
inline int toInt32(char    v) { return v;}
inline int toInt32(short   v) { return v;}
inline int toInt32(int     v) { return v;}
inline int toInt32(int64   v) { return v;}
inline int toInt32(double  v) { return (int)v;}
inline int toInt32(litstr  v) { return StringData(v).toInt32();}
inline int toInt32(CStrRef v) { return v.toInt32();}
inline int toInt32(CArrRef v) { return v.toInt32();}
inline int toInt32(CObjRef v) { return v.toInt32();}
inline int toInt32(CVarRef v) { return v.toInt32();}

inline int64 toInt64(bool    v) { return v ? 1 : 0;}
inline int64 toInt64(char    v) { return v;}
inline int64 toInt64(short   v) { return v;}
inline int64 toInt64(int     v) { return v;}
inline int64 toInt64(int64   v) { return v;}
inline int64 toInt64(double  v) {
  return ((v > LONG_MAX) ? (uint64)v : (int64)v);
}
inline int64 toInt64(litstr  v) { return StringData(v).toInt64();}
inline int64 toInt64(CStrRef v) { return v.toInt64();}
inline int64 toInt64(CArrRef v) { return v.toInt64();}
inline int64 toInt64(CObjRef v) { return v.toInt64();}
inline int64 toInt64(CVarRef v) { return v.toInt64();}

inline double toDouble(bool    v) { return v ? 1 : 0;}
inline double toDouble(char    v) { return v;}
inline double toDouble(short   v) { return v;}
inline double toDouble(int     v) { return v;}
inline double toDouble(int64   v) { return v;}
inline double toDouble(double  v) { return v;}
inline double toDouble(litstr  v) { return StringData(v).toDouble();}
inline double toDouble(CStrRef v) { return v.toDouble();}
inline double toDouble(CArrRef v) { return v.toDouble();}
inline double toDouble(CObjRef v) { return v.toDouble();}
inline double toDouble(CVarRef v) { return v.toDouble();}

inline String toString(bool    v) { return v ? "1" : "";}
inline String toString(char    v) { return (int64)v;}
inline String toString(short   v) { return (int64)v;}
inline String toString(int     v) { return (int64)v;}
inline String toString(int64   v) { return v;}
inline String toString(double  v) { return v;}
inline String toString(litstr  v) { return v;}
inline String toString(CStrRef v) { return v;}
inline String toString(CArrRef v) { return "Array";}
inline String toString(CObjRef v) { return v.toString();}
inline String toString(CVarRef v) { return v.toString();}

inline Array toArray(bool    v) { return Array::Create(v);}
inline Array toArray(char    v) { return Array::Create(v);}
inline Array toArray(short   v) { return Array::Create(v);}
inline Array toArray(int     v) { return Array::Create(v);}
inline Array toArray(int64   v) { return Array::Create(v);}
inline Array toArray(double  v) { return Array::Create(v);}
inline Array toArray(litstr  v) { return Array::Create(v);}
inline Array toArray(CStrRef v) { return Array::Create(v);}
inline Array toArray(CArrRef v) { return v;}
inline Array toArray(CObjRef v) { return v.toArray();}
inline Array toArray(CVarRef v) { return v.toArray();}

inline Object toObject(bool    v) { return Variant(v).toObject();}
inline Object toObject(char    v) { return Variant(v).toObject();}
inline Object toObject(short   v) { return Variant(v).toObject();}
inline Object toObject(int     v) { return Variant(v).toObject();}
inline Object toObject(int64   v) { return Variant(v).toObject();}
inline Object toObject(double  v) { return Variant(v).toObject();}
inline Object toObject(litstr  v) { return Variant(v).toObject();}
inline Object toObject(CStrRef v) { return Variant(v).toObject();}
inline Object toObject(CArrRef v) { return v.toObject();}
inline Object toObject(CObjRef v) { return v;}
inline Object toObject(CVarRef v) { return v.toObject();}

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
inline Numeric modulo(int64 v1, int64 v2) {
  if (abs(v2) == 1) {
    return 0;
  }
  if (v2 == 0) {
    Logger::Warning("Division by zero");
    return false;
  }
  return v1 % v2;
}
inline int64 shift_left(int64 v1, int64 v2)        { return v1 << v2; }
inline int64 shift_right(int64 v1, int64 v2)       { return v1 >> v2; }

inline bool logical_xor_rev(bool v2, bool v1)
{ return (v1 ? 1:0) ^ (v2 ? 1:0);}
inline Variant bitwise_or_rev(CVarRef v2, CVarRef v1)  { return v1 | v2;}
inline Variant bitwise_and_rev(CVarRef v2, CVarRef v1) { return v1 & v2;}
inline Variant bitwise_xor_rev(CVarRef v2, CVarRef v1) { return v1 ^ v2;}
inline Variant multiply_rev(CVarRef v2, CVarRef v1)    { return v1 * v2;}
inline Variant plus_rev(CVarRef v2, CVarRef v1)        { return v1 + v2;}
inline Numeric minus_rev(CVarRef v2, CVarRef v1)       { return v1 - v2;}
inline Numeric divide_rev(CVarRef v2, CVarRef v1)      { return v1 / v2; }
inline int64 modulo_rev(int64 v2, int64 v1)            { return v1 % v2; }
inline int64 shift_left_rev(int64 v2, int64 v1)        { return v1 << v2; }
inline int64 shift_right_rev(int64 v2, int64 v1)       { return v1 >> v2; }

inline Variant bitwise_or_assign_rev(CVarRef v2, Variant& v1)
{ return v1 |= v2;}
inline Variant bitwise_and_assign_rev(CVarRef v2, Variant& v1)
{ return v1 &= v2;}
inline Variant bitwise_xor_assign_rev(CVarRef v2, Variant& v1)
{ return v1 ^= v2;}
inline Variant multiply_assign_rev(CVarRef v2, Variant& v1)
{ return v1 *= v2;}
inline Variant plus_assign_rev(CVarRef v2, Variant& v1)
{ return v1 += v2;}
inline Numeric minus_assign_rev(CVarRef v2, Variant& v1)
{ return v1 -= v2;}
inline Numeric divide_assign_rev(CVarRef v2, Variant& v1)
{ return v1 /= v2; }
inline int64 modulo_assign_rev(int64 v2, Variant& v1)
{ return v1 %= v2; }
inline int64 shift_left_assign_rev(int64 v2, Variant& v1)
{ return v1 <<= v2; }
inline int64 shift_right_assign_rev(int64 v2, Variant& v1)
{ return v1 >>= v2; }

inline char    negate(char v)    { return -v; }
inline short   negate(short v)   { return -v; }
inline int     negate(int v)     { return -v; }
inline int64   negate(int64 v)   { return -v; }
inline double  negate(double v)  { return -v; }
inline Variant negate(CVarRef v) { return -(Variant)v; }

inline String concat(CStrRef s1, CStrRef s2)         { return s1 + s2;}
inline String concat_rev(CStrRef s2, CStrRef s1)     { return s1 + s2;}
inline String &concat_assign(String &s1, litstr s2)  { return s1 += s2;}
inline String &concat_assign(String &s1, CStrRef s2) { return s1 += s2;}

#define MAX_CONCAT_ARGS 6
String concat3(CStrRef s1, CStrRef s2, CStrRef s3);
String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4);
String concat5(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5);
String concat6(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5,
               CStrRef s6);

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

Variant &concat_assign(ObjectOffset v1, CStrRef s2);

inline String &concat_assign(const StringOffset &s1, litstr s2) {
  return concat_assign(s1.lval(), s2);
}

inline String &concat_assign(const StringOffset &s1, CStrRef s2) {
  return concat_assign(s1.lval(), s2);
}

inline String &concat_assign_rev(litstr s2, String &s1)  {
  return concat_assign(s1, s2);
}
inline String &concat_assign_rev(CStrRef s2, String &s1) {
  return concat_assign(s1, s2);
}
inline Variant &concat_assign_rev(litstr s2, Variant &v1) {
  return concat_assign(v1, s2);
}
inline Variant &concat_assign_rev(CStrRef s2, Variant &v1) {
  return concat_assign(v1, s2);
}
inline Variant &concat_assign_rev(CStrRef s2, ObjectOffset v1) {
  return concat_assign(v1, s2);
}
inline String &concat_assign_rev(litstr s2, const StringOffset &s1) {
  return concat_assign(s1, s2);
}
inline String &concat_assign_rev(CStrRef s2, const StringOffset &s) {
  return concat_assign(s, s2);
}

inline bool instanceOf(bool    v, const char *s) { return false;}
inline bool instanceOf(char    v, const char *s) { return false;}
inline bool instanceOf(short   v, const char *s) { return false;}
inline bool instanceOf(int     v, const char *s) { return false;}
inline bool instanceOf(int64   v, const char *s) { return false;}
inline bool instanceOf(double  v, const char *s) { return false;}
inline bool instanceOf(litstr  v, const char *s) { return false;}
inline bool instanceOf(CStrRef v, const char *s) { return false;}
inline bool instanceOf(CArrRef v, const char *s) { return false;}
inline bool instanceOf(CObjRef v, const char *s) { return v.instanceof(s);}
inline bool instanceOf(CVarRef v, const char *s) {
  return v.is(KindOfObject) &&
    toObject(v).instanceof(s);
}
template <typename T>
bool instanceOf(const T &v, const char *s) {
  return v.instanceof(s);
}

///////////////////////////////////////////////////////////////////////////////
// output functions

inline int print(litstr  s) {
  g_context->out() << s;
  return 1;
}
inline int print(CStrRef s) {
  g_context->out().write((const char *)s, s.length());
  return 1;
}
inline void echo(litstr  s) {
  g_context->out() << s;
}
inline void echo(CStrRef s) {
  g_context->out().write((const char *)s, s.length());
}

inline void silenceInc() {
  g_context->silenceInc();
}
inline Variant silenceDec(Variant v) {
  g_context->silenceDec();
  return v;
}

String get_source_filename(litstr path);

inline void throw_exception(CObjRef v) { throw v;}

///////////////////////////////////////////////////////////////////////////////
// isset/unset

inline bool isInitialized(CVarRef v) { return v.isInitialized();}

inline bool isset(CVarRef v) { return !v.isNull();}
bool isset(CVarRef v, const CVarRef offset, int64 prehash = -1);

inline Variant unset(Variant &v)               { v.unset();   return null;}
inline Variant unset(const ObjectOffset &v)    { return unset(v.lval());}
inline Variant unset(CVarRef v)                {              return null;}
inline Variant setNull(Variant &v)             { v.setNull(); return null;}

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
inline Variant &lval(const ObjectOffset  &v) { return v.lval();}

template<class T>
Variant &unsetLval(Variant &v, const T &key, int64 prehash = -1) {
  if (v.isNull()) {
    return v;
  }
  if (v.is(KindOfArray)) {
    if (v.getArrayData()->exists(key, prehash)) {
      return v.lvalAt(key, prehash);
    }
    return Variant::lvalBlackHole();
  }
  return Variant::lvalInvalid();
}

template<class T>
Variant &unsetLval(Array &v, const T &key, int64 prehash = -1) {
  if (!v.isNull() && v.exists(key, prehash)) {
    return v.lvalAt(key, prehash);
  }
  return Variant::lvalBlackHole();
}

/**
 * ref() sets contagious flag, so that next assignment will make both sides
 * strongly bind to the same underlying variant data. For example,
 *
 *   a = ref(b); // string binding: now both a and b point to the same data
 *   a = b;      // weak binding: a will copy or copy-on-write
 */
inline CVarRef ref(CVarRef v) { v.setContagious(); return v;}
inline CVarRef ref(const ObjectOffset  &v) { return ref(v.lval());}

///////////////////////////////////////////////////////////////////////////////
// misc functions

Variant f_call_user_func_array(CVarRef function, CArrRef params);

/**
 * Fallback when a dynamic function call fails to find a user function
 * matching the name. This will attempt to call a system function first,
 * and attempt to resolve the function via any registered invocation
 * handlers (via register_invocation_handler). If no handlers are able to
 * invoke the function, throw an InvalidFunctionCallException.
 */
Variant invoke_failed(const char *func, CArrRef params, int64 hash,
                      bool fatal = true);

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal = true);

/**
 * A function call that has too many arguments will be invoked through this
 * wrapper, so to make sure all extra parameters are still evaluated.
 */
template <class T>
T invoke_too_many_args(const char *func, int count, T ret) {
  Logger::Verbose("Calling %s with %d more arguments", func, count);
  return ret;
}

/**
 * Invocation handlers attempt to call the named function with the passed
 * parameters. They return true if the function was found and called, or
 * false if the function could not be found. They should only throw
 * exceptions if the function was found but an error occured during its
 * execution.
 */
typedef bool(*InvocationHandler)(Variant& /*retval*/, const char* /*func*/,
                                 CArrRef /*params*/, int64 /*hash*/);

/**
 * Register an invocation hander for use with invoke_system.
 */
void register_invocation_handler(InvocationHandler fn);

/**
 * When fatal coding errors are transformed to this function call.
 */
inline Variant throw_fatal(const char *msg, void *dummy = NULL) {
  throw FatalErrorException(msg);
}
inline Variant throw_missing_class(const char *cls) {
  throw ClassNotFoundException((std::string("unknown class ") + cls).c_str());
}

inline Variant throw_missing_file(const char *cls) {
  throw PhpFileDoesNotExistException(cls);
}

/**
 * When fatal coding errors are transformed to this function call.
 */
inline Object throw_fatal_object(const char *msg, void *dummy = NULL) {
  throw FatalErrorException(msg);
}

void throw_unexpected_argument_type(int argNum, const char *fnName,
                                    const char *expected, CVarRef val);

/**
 * Handler for exceptions thrown from object destructors. Implemented in
 * program_functions.cpp.
 */
void handle_destructor_exception();

/**
 * If RuntimeOption::AlwaysThrowBadTypeExceptions is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::AlwaysThrowBadTypeExceptions is off, we will log a
 * warning and swallow the error.
 */
void throw_bad_type_exception(const char *fmt, ...);

/**
 * Exceptions injected code throws
 */
void throw_infinite_loop_exception();
void throw_infinite_recursion_exception();
void throw_request_timeout_exception(ThreadInfo *info);

/**
 * Cloning an object.
 */
Object f_clone(Object obj);

/**
 * Serialize/unserialize a variant into/from a string. We need these two
 * functions in cpp/base, as there are functions in cpp/base that depend on
 * these two functions.
 */
String f_serialize(CVarRef value);
Variant f_unserialize(CStrRef str);


class LVariableTable;
Variant include(CStrRef file, bool once = false,
                LVariableTable* variables = NULL,
                const char *currentDir = NULL);
Variant require(CStrRef file, bool once = false,
                LVariableTable* variables = NULL,
                const char *currentDir = NULL);

/**
 * For function interception or stubout support.
 */
hphp_const_char_map<const char*> &get_renamed_functions();

inline void assignCallTemp(Variant& temp, CVarRef val) {
  temp.unset();
  temp.clearContagious();
  temp = val;
  if (temp.isReferenced()) {
    temp.setContagious();
  }
}

/**
 * For wrapping expressions that have no effect, so to make gcc happy.
 */
inline bool    id(bool    v) { return v; }
inline char    id(char    v) { return v; }
inline short   id(short   v) { return v; }
inline int     id(int     v) { return v; }
inline int64   id(int64   v) { return v; }
inline uint64  id(uint64  v) { return v; }
inline ssize_t id(ssize_t v) { return v; }
inline double  id(double  v) { return v; }
inline litstr  id(litstr  v) { return v; }
inline CStrRef id(CStrRef v) { return v; }
inline CArrRef id(CArrRef v) { return v; }
inline CObjRef id(CObjRef v) { return v; }
inline CVarRef id(CVarRef v) { return v; }

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_BUILTIN_FUNCTIONS_H__
