/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TYPE_CONVERSIONS_H_
#define incl_HPHP_TYPE_CONVERSIONS_H_

#include <limits>

#include "hphp/runtime/base/type-variant.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// type conversion functions

inline bool toBoolean(bool    v) { return v;}
inline bool toBoolean(char    v) { return v;}
inline bool toBoolean(short   v) { return v;}
inline bool toBoolean(int     v) { return v;}
inline bool toBoolean(int64_t v) { return v;}
inline bool toBoolean(double  v) { return v;}
inline bool toBoolean(const char* v) = delete;
inline bool toBoolean(const StringData *v) {
  return v ? v->toBoolean() : false;
}
inline bool toBoolean(const String& v) { return toBoolean(v.get());}
inline bool toBoolean(const ArrayData *v) {
  return v && !v->empty();
}
inline bool toBoolean(const Array& v) { return toBoolean(v.get());}
inline bool toBoolean(const ObjectData *v) {
  return v ? v->toBoolean() : false;
}
inline bool toBoolean(const Object& v) { return toBoolean(v.get());}
inline bool toBoolean(const Variant& v) { return v.toBoolean();}

inline int toInt32(bool    v) { return v ? 1 : 0;}
inline int toInt32(char    v) { return v;}
inline int toInt32(short   v) { return v;}
inline int toInt32(int     v) { return v;}
inline int toInt32(int64_t v) { return v;}
inline int toInt32(double  v) { return (int)v;}
inline int toInt32(const char* v) = delete;
inline int toInt32(const StringData *v) { return v ? v->toInt32() : 0;}
inline int toInt32(const String& v) { return toInt32(v.get());}
inline int toInt32(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline int toInt32(const Array& v) { return toInt32(v.get());}
inline int toInt32(const ObjectData *v) { return v ? v->toInt64() : 0;}
inline int toInt32(const Object& v) { return toInt32(v.get());}
inline int toInt32(const Variant& v) { return v.toInt32();}

inline int64_t toInt64(bool    v) { return v ? 1 : 0;}
inline int64_t toInt64(char    v) { return v;}
inline int64_t toInt64(short   v) { return v;}
inline int64_t toInt64(int     v) { return v;}
inline int64_t toInt64(int64_t v) { return v;}
inline int64_t toInt64(double  v) {
  if (v >= 0) {
    return v < std::numeric_limits<uint64_t>::max() ? (uint64_t)v : 0u;
  } else if (v < 0) {
    return (int64_t)v;
  } else {
    // If v >= 0 is false and v < 0 is false, then v is NaN. In that
    // case, the semantics differ:
    //   - In PHP7, you just get 0.
    //   - In PHP5, on Intel, you get 0x800..00, a.k.a. the minimum int64_t.
    //     We mimic that on all platforms, though this makes us sad.
    return RuntimeOption::PHP7_IntSemantics
      ? 0
      : std::numeric_limits<int64_t>::min();
  }
}
inline int64_t toInt64(const char* v) = delete;
inline int64_t toInt64(const StringData *v) { return v ? v->toInt64() : 0;}
inline int64_t toInt64(const String& v) { return toInt64(v.get());}
inline int64_t toInt64(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline int64_t toInt64(const Array& v) { return toInt64(v.get());}
inline int64_t toInt64(const ObjectData *v) { return v ? v->toInt64() : 0;}
inline int64_t toInt64(const Object& v) { return toInt64(v.get());}
inline int64_t toInt64(const Variant& v) { return v.toInt64();}

inline double toDouble(bool    v) { return v ? 1 : 0;}
inline double toDouble(char    v) { return v;}
inline double toDouble(short   v) { return v;}
inline double toDouble(int     v) { return v;}
inline double toDouble(int64_t v) { return v;}
inline double toDouble(double  v) { return v;}
inline double toDouble(const char* v) = delete;
inline double toDouble(const StringData *v) { return v? v->toDouble() : 0;}
inline double toDouble(const String& v) { return toDouble(v.get());}
inline double toDouble(const ArrayData *v) {
  return (v && !v->empty()) ? 1.0 : 0.0;
}
inline double toDouble(const Array& v) { return toDouble(v.get());}
inline double toDouble(const ObjectData *v) { return v ? v->toDouble() : 0;}
inline double toDouble(const Object& v) { return toDouble(v.get());}
inline double toDouble(const Variant& v) { return v.toDouble();}

inline String toString(bool    v) { return v ? "1" : "";}
inline String toString(char    v) { return (int64_t)v;}
inline String toString(short   v) { return (int64_t)v;}
inline String toString(int     v) { return (int64_t)v;}
inline String toString(int64_t v) { return v;}
inline String toString(double  v) { return v;}
inline String toString(const char* v) = delete;
inline String toString(StringData *v) {
  return v ? String(v) : empty_string();
}
inline String toString(const String& v) { return toString(v.get());}
inline String toString(const ArrayData *v) {
  if (v == nullptr) return empty_string();
  raise_notice("Array to string conversion");
  return array_string;
}
inline String toString(const Array& v) { return toString(v.get());}
inline String toString(ObjectData *v) {
  return v ? v->invokeToString() : empty_string();
}
inline String toString(const Object& v) { return toString(v.get());}
inline String toString(const Variant& v) { return v.toString();}

inline Array toArray(bool    v) { return Array::Create(v);}
inline Array toArray(char    v) { return Array::Create(v);}
inline Array toArray(short   v) { return Array::Create(v);}
inline Array toArray(int     v) { return Array::Create(v);}
inline Array toArray(int64_t v) { return Array::Create(v);}
inline Array toArray(double  v) { return Array::Create(v);}
inline Array toArray(const char* v) = delete;
inline Array toArray(StringData *v) {
  return v ? Array::Create(Variant{v}) : Array::Create();
}
inline Array toArray(const String& v) { return toArray(v.get());}
inline Array toArray(ArrayData *v) { return v ? Array(v) : Array::Create();}
inline Array toArray(const Array& v) { return toArray(v.get());}
inline Array toArray(const ObjectData *v) {
  return v ? v->toArray() : Array::Create();
}
inline Array toArray(const Object& v) { return toArray(v.get());}
inline Array toArray(const Variant& v) { return v.toArray();}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TYPE_CONVERSIONS_H_
