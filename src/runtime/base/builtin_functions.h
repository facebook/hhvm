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

#include <runtime/base/execution_context.h>
#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/object_offset.h>
#include <runtime/base/frame_injection.h>
#include <runtime/base/intercept.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/runtime_option.h>
#include <util/case_insensitive.h>
#ifdef TAINTED
#include <runtime/base/propagate_tainting.h>
#endif

#ifdef __APPLE__
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

bool empty(CVarRef v, bool    offset, int64 prehash = -1);
bool empty(CVarRef v, char    offset, int64 prehash = -1);
bool empty(CVarRef v, short   offset, int64 prehash = -1);
bool empty(CVarRef v, int     offset, int64 prehash = -1);
bool empty(CVarRef v, int64   offset, int64 prehash = -1);
bool empty(CVarRef v, double  offset, int64 prehash = -1);
bool empty(CVarRef v, CArrRef offset, int64 prehash = -1);
bool empty(CVarRef v, CObjRef offset, int64 prehash = -1);
bool empty(CVarRef v, CVarRef offset, int64 prehash = -1);
bool empty(CVarRef v, litstr  offset, int64 prehash = -1,
           bool isString = false);
bool empty(CVarRef v, CStrRef offset, int64 prehash = -1,
           bool isString = false);

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
    raise_warning("Division by zero");
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

inline int64 multiply_rev(bool v2, bool v1)            { return v1 * v2;}
inline int64 multiply_rev(bool v2, int v1)             { return v1 * v2;}
inline int64 multiply_rev(bool v2, int64 v1)           { return v1 * v2;}
inline double multiply_rev(bool v2, double v1)         { return v1 * v2;}
inline Numeric multiply_rev(bool v2, CVarRef v1)       { return v1 * v2;}
inline int64 multiply_rev(int v2, bool v1)             { return v1 * v2;}
inline int64 multiply_rev(int v2, int v1)              { return v1 * v2;}
inline int64 multiply_rev(int v2, int64 v1)            { return v1 * v2;}
inline double multiply_rev(int v2, double v1)          { return v1 * v2;}
inline Numeric multiply_rev(int v2, CVarRef v1)        { return v1 * v2;}
inline int64 multiply_rev(int64 v2, bool v1)           { return v1 * v2;}
inline int64 multiply_rev(int64 v2, int v1)            { return v1 * v2;}
inline int64 multiply_rev(int64 v2, int64 v1)          { return v1 * v2;}
inline double multiply_rev(int64 v2, double v1)        { return v1 * v2;}
inline Numeric multiply_rev(int64 v2, CVarRef v1)      { return v1 * v2;}
inline double multiply_rev(double v2, bool v1)         { return v1 * v2;}
inline double multiply_rev(double v2, int v1)          { return v1 * v2;}
inline double multiply_rev(double v2, int64 v1)        { return v1 * v2;}
inline double multiply_rev(double v2, double v1)       { return v1 * v2;}
inline Numeric multiply_rev(double v2, CVarRef v1)     { return v1 * v2;}
inline Numeric multiply_rev(CVarRef v2, bool v1)       { return v1 * v2;}
inline Numeric multiply_rev(CVarRef v2, int v1)        { return v1 * v2;}
inline Numeric multiply_rev(CVarRef v2, int64 v1)      { return v1 * v2;}
inline Numeric multiply_rev(CVarRef v2, double v1)     { return v1 * v2;}
inline Numeric multiply_rev(CVarRef v2, CVarRef v1)    { return v1 * v2;}

inline int64 plus_rev(bool v2, bool v1)                { return v1 + v2;}
inline int64 plus_rev(bool v2, int v1)                 { return v1 + v2;}
inline int64 plus_rev(bool v2, int64 v1)               { return v1 + v2;}
inline double plus_rev(bool v2, double v1)             { return v1 + v2;}
inline Numeric plus_rev(bool v2, CVarRef v1)           { return v1 + v2;}
inline int64 plus_rev(int v2, bool v1)                 { return v1 + v2;}
inline int64 plus_rev(int v2, int v1)                  { return v1 + v2;}
inline int64 plus_rev(int v2, int64 v1)                { return v1 + v2;}
inline double plus_rev(int v2, double v1)              { return v1 + v2;}
inline Numeric plus_rev(int v2, CVarRef v1)            { return v1 + v2;}
inline int64 plus_rev(int64 v2, bool v1)               { return v1 + v2;}
inline int64 plus_rev(int64 v2, int v1)                { return v1 + v2;}
inline int64 plus_rev(int64 v2, int64 v1)              { return v1 + v2;}
inline double plus_rev(int64 v2, double v1)            { return v1 + v2;}
inline Numeric plus_rev(int64 v2, CVarRef v1)          { return v1 + v2;}
inline double plus_rev(double v2, bool v1)             { return v1 + v2;}
inline double plus_rev(double v2, int v1)              { return v1 + v2;}
inline double plus_rev(double v2, int64 v1)            { return v1 + v2;}
inline double plus_rev(double v2, double v1)           { return v1 + v2;}
inline Numeric plus_rev(double v2, CVarRef v1)         { return v1 + v2;}
inline Numeric plus_rev(CVarRef v2, bool v1)           { return v1 + v2;}
inline Numeric plus_rev(CVarRef v2, int v1)            { return v1 + v2;}
inline Numeric plus_rev(CVarRef v2, int64 v1)          { return v1 + v2;}
inline Numeric plus_rev(CVarRef v2, double v1)         { return v1 + v2;}
inline Numeric plus_rev(CVarRef v2, CVarRef v1)        { return v1 + v2;}

inline int64 minus_rev(bool v2, bool v1)               { return v1 - v2;}
inline int64 minus_rev(bool v2, int v1)                { return v1 - v2;}
inline int64 minus_rev(bool v2, int64 v1)              { return v1 - v2;}
inline double minus_rev(bool v2, double v1)            { return v1 - v2;}
inline Numeric minus_rev(bool v2, CVarRef v1)          { return v1 - v2;}
inline int64 minus_rev(int v2, bool v1)                { return v1 - v2;}
inline int64 minus_rev(int v2, int v1)                 { return v1 - v2;}
inline int64 minus_rev(int v2, int64 v1)               { return v1 - v2;}
inline double minus_rev(int v2, double v1)             { return v1 - v2;}
inline Numeric minus_rev(int v2, CVarRef v1)           { return v1 - v2;}
inline int64 minus_rev(int64 v2, bool v1)              { return v1 - v2;}
inline int64 minus_rev(int64 v2, int v1)               { return v1 - v2;}
inline int64 minus_rev(int64 v2, int64 v1)             { return v1 - v2;}
inline double minus_rev(int64 v2, double v1)           { return v1 - v2;}
inline Numeric minus_rev(int64 v2, CVarRef v1)         { return v1 - v2;}
inline double minus_rev(double v2, bool v1)            { return v1 - v2;}
inline double minus_rev(double v2, int v1)             { return v1 - v2;}
inline double minus_rev(double v2, int64 v1)           { return v1 - v2;}
inline double minus_rev(double v2, double v1)          { return v1 - v2;}
inline Numeric minus_rev(double v2, CVarRef v1)        { return v1 - v2;}
inline Numeric minus_rev(CVarRef v2, bool v1)          { return v1 - v2;}
inline Numeric minus_rev(CVarRef v2, int v1)           { return v1 - v2;}
inline Numeric minus_rev(CVarRef v2, int64 v1)         { return v1 - v2;}
inline Numeric minus_rev(CVarRef v2, double v1)        { return v1 - v2;}
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

inline String concat(CStrRef s1, CStrRef s2)         {
  #ifndef TAINTED
  return s1 + s2;
  #else
  String res = s1 + s2;
  propagate_tainting2(s1, s2, res);
  return res;
  #endif
}
inline String concat_rev(CStrRef s2, CStrRef s1)     {
  #ifndef TAINTED
  return s1 + s2;
  #else
  String res = s1 + s2;
  propagate_tainting2(s1, s2, res);
  return res;
  #endif
}
inline String &concat_assign(String &s1, litstr s2)  {
  return s1 += s2;
  // nothing to be done for tainting
}
inline String &concat_assign(String &s1, CStrRef s2) {
  #ifndef TAINTED
  return s1 += s2;
  #else
  propagate_tainting2(s1, s2, s1 += s2);
  return s1;
  #endif
}

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
      // nothing to be done for tainting
      return v1;
    }
  }
  String s1 = v1.toString();
  s1 += s2;
  v1 = s1;
  // nothing to be done for tainting
  return v1;
}

inline Variant &concat_assign(Variant &v1, CStrRef s2) {
  if (v1.getType() == KindOfString) {
    StringData *data = v1.getStringData();
    if (data->getCount() == 1) {
      data->append(s2.data(), s2.size());
      #ifndef TAINTED
      return v1;
      #else
      String s1 = v1.toString();
      propagate_tainting2(s1, s2, s1);
      return v1;
      #endif
    }
  }
  String s1 = v1.toString();
  s1 += s2;
  #ifdef TAINTED
  propagate_tainting2(s1, s2, s1);
  #endif
  v1 = s1;
  return v1;
}

String concat_assign(ObjectOffset v1, CStrRef s2);

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
inline String concat_assign_rev(CStrRef s2, ObjectOffset v1) {
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
bool instanceOf(const SmartObject<T> &v, const char *s) {
  return v.instanceof(s);
}
inline bool instanceOf(ObjectData *v, const char *s) {
  return v && v->o_instanceof(s);
}

///////////////////////////////////////////////////////////////////////////////
// output functions

inline int print(const char *s) {
  g_context->write(s);
  return 1;
}
inline int print(CStrRef s) {
  g_context->write(s);
  return 1;
}
inline void echo(const char *s) {
  g_context->write(s);
}
inline void echo(CStrRef s) {
  g_context->write(s);
}

String get_source_filename(litstr path);

void throw_exception(CObjRef e);
bool set_line(int line);

///////////////////////////////////////////////////////////////////////////////
// isset/unset

inline bool isInitialized(CVarRef v) { return v.isInitialized();}
Variant getDynamicConstant(CVarRef v, CStrRef name);
String getUndefinedConstant(CStrRef name);

inline bool isset(CVarRef v) { return !v.isNull();}
inline bool isset(CObjRef v) { return !v.isNull();}
bool isset(CVarRef v, bool    offset, int64 prehash = -1);
bool isset(CVarRef v, char    offset, int64 prehash = -1);
bool isset(CVarRef v, short   offset, int64 prehash = -1);
bool isset(CVarRef v, int     offset, int64 prehash = -1);
bool isset(CVarRef v, int64   offset, int64 prehash = -1);
bool isset(CVarRef v, double  offset, int64 prehash = -1);
bool isset(CVarRef v, CArrRef offset, int64 prehash = -1);
bool isset(CVarRef v, CObjRef offset, int64 prehash = -1);
bool isset(CVarRef v, CVarRef offset, int64 prehash = -1);
bool isset(CVarRef v, litstr  offset, int64 prehash = -1,
           bool isString = false);
bool isset(CVarRef v, CStrRef offset, int64 prehash = -1,
           bool isString = false);

inline Variant unset(Variant &v)               { v.unset();   return null;}
inline Variant unset(const ObjectOffset &v)    { v.unset();   return null;}
inline Variant unset(CVarRef v)                {              return null;}
inline Variant setNull(Variant &v)             { v.setNull(); return null;}
inline Variant unset(Object &v)                { v.reset();   return null;}

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
    if (v.toArray().exists(key, prehash)) {
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
 *   a = ref(b); // strong binding: now both a and b point to the same data
 *   a = b;      // weak binding: a will copy or copy-on-write
 */
inline CVarRef ref(CVarRef v) { v.setContagious(); return v;}
inline CVarRef ref(const ObjectOffset  &v) { return ref(v.lval());}

///////////////////////////////////////////////////////////////////////////////
// misc functions

bool class_exists(CStrRef class_name, bool autoload = true);
String get_static_class_name(CVarRef objOrClassName);

Variant f_call_user_func_array(CVarRef function, CArrRef params);

Variant invoke(CStrRef function, CArrRef params, int64 hash = -1,
               bool tryInterp = true, bool fatal = true);

/**
 * Fallback when a dynamic function call fails to find a user function
 * matching the name.  If no handlers are able to
 * invoke the function, throw an InvalidFunctionCallException.
 */
Variant invoke_failed(const char *s, CArrRef params,
                      int64 hash, bool fatal = true);

Variant o_invoke_failed(const char *cls, const char *meth, bool fatal = true);

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
void throw_instance_method_fatal(const char *name);

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
 * If RuntimeOption::ThrowBadTypeExceptions is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowBadTypeExceptions is off, we will log a
 * warning and swallow the error.
 */
void throw_bad_type_exception(const char *fmt, ...);

/**
 * If RuntimeOption::ThrowInvalidArguments is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowInvalidArguments is off, we will log a
 * warning and swallow the error.
 */
void throw_invalid_argument(const char *fmt, ...);

/**
 * Exceptions injected code throws
 */
void throw_infinite_loop_exception() ATTRIBUTE_COLD;
void throw_infinite_recursion_exception() ATTRIBUTE_COLD;
void throw_request_timeout_exception() ATTRIBUTE_COLD;
void throw_memory_exceeded_exception() ATTRIBUTE_COLD __attribute__((noreturn));

/**
 * Cloning an object.
 */
Object f_clone(Object obj);

/**
 * Serialize/unserialize a variant into/from a string. We need these two
 * functions in runtime/base, as there are functions in runtime/base that depend on
 * these two functions.
 */
String f_serialize(CVarRef value);
Variant f_unserialize(CStrRef str);


class LVariableTable;
Variant include(CStrRef file, bool once = false,
                LVariableTable* variables = NULL,
                const char *currentDir = "");
Variant require(CStrRef file, bool once = false,
                LVariableTable* variables = NULL,
                const char *currentDir = "");
Variant include_impl_invoke(CStrRef file, bool once = false,
                            LVariableTable* variables = NULL,
                            const char *currentDir = "");

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
 * constructors (which can incorrectly pass through
 * the contagious bit).
 */
inline Variant wrap_variant(CVarRef x) { return x; }

inline LVariableTable *lvar_ptr(const LVariableTable &vt) {
  return const_cast<LVariableTable*>(&vt);
}

bool function_exists(CStrRef function_name);

/**
 * For autoload support
 */
class Globals;
void checkClassExists(CStrRef name, Globals *g, bool nothrow = false);
bool checkClassExists(CStrRef name, const bool *declared, bool autoloadExists,
                      bool nothrow = false);
bool checkInterfaceExists(CStrRef name, const bool *declared,
                          bool autoloadExists, bool nothrow = false);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_BUILTIN_FUNCTIONS_H__
