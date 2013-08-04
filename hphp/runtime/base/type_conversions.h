/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_TYPE_CONVERSIONS_H_
#define incl_HPHP_TYPE_CONVERSIONS_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex_types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// type conversion functions

inline bool toBoolean(bool    v) { return v;}
inline bool toBoolean(char    v) { return v;}
inline bool toBoolean(short   v) { return v;}
inline bool toBoolean(int     v) { return v;}
inline bool toBoolean(int64_t   v) { return v;}
inline bool toBoolean(double  v) { return v;}
inline bool toBoolean(litstr  v) = delete;
inline bool toBoolean(const StringData *v) {
  return v ? v->toBoolean() : false;
}
inline bool toBoolean(CStrRef v) { return toBoolean(v.get());}
inline bool toBoolean(const ArrayData *v) {
  return v && !v->empty();
}
inline bool toBoolean(CArrRef v) { return toBoolean(v.get());}
inline bool toBoolean(const ObjectData *v) {
  return v ? v->o_toBoolean() : false;
}
inline bool toBoolean(CObjRef v) { return toBoolean(v.get());}
inline bool toBoolean(CVarRef v) { return v.toBoolean();}

inline int toInt32(bool    v) { return v ? 1 : 0;}
inline int toInt32(char    v) { return v;}
inline int toInt32(short   v) { return v;}
inline int toInt32(int     v) { return v;}
inline int toInt32(int64_t   v) { return v;}
inline int toInt32(double  v) { return (int)v;}
inline int toInt32(litstr  v) = delete;
inline int toInt32(const StringData *v) { return v ? v->toInt32() : 0;}
inline int toInt32(CStrRef v) { return toInt32(v.get());}
inline int toInt32(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline int toInt32(CArrRef v) { return toInt32(v.get());}
inline int toInt32(const ObjectData *v) { return v ? v->o_toInt64() : 0;}
inline int toInt32(CObjRef v) { return toInt32(v.get());}
inline int toInt32(CVarRef v) { return v.toInt32();}

inline int64_t toInt64(bool    v) { return v ? 1 : 0;}
inline int64_t toInt64(char    v) { return v;}
inline int64_t toInt64(short   v) { return v;}
inline int64_t toInt64(int     v) { return v;}
inline int64_t toInt64(int64_t   v) { return v;}
inline int64_t toInt64(double  v) {
  // If v >= 0 is false and v < 0 is false, then v is NaN. In that case, on
  // Intel, you get 0x800..00, a.k.a. the minimum int64_t. We mimic that on all
  // platforms, though this makes us sad.
  return (v >= 0
          ? (v > std::numeric_limits<uint64_t>::max() ? 0u : (uint64_t)v)
          : (v < 0 ? (int64_t)v : std::numeric_limits<int64_t>::min()));
}
inline int64_t toInt64(litstr  v) = delete;
inline int64_t toInt64(const StringData *v) { return v ? v->toInt64() : 0;}
inline int64_t toInt64(CStrRef v) { return toInt64(v.get());}
inline int64_t toInt64(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline int64_t toInt64(CArrRef v) { return toInt64(v.get());}
inline int64_t toInt64(const ObjectData *v) { return v ? v->o_toInt64() : 0;}
inline int64_t toInt64(CObjRef v) { return toInt64(v.get());}
inline int64_t toInt64(CVarRef v) { return v.toInt64();}

inline double toDouble(bool    v) { return v ? 1 : 0;}
inline double toDouble(char    v) { return v;}
inline double toDouble(short   v) { return v;}
inline double toDouble(int     v) { return v;}
inline double toDouble(int64_t   v) { return v;}
inline double toDouble(double  v) { return v;}
inline double toDouble(litstr  v) = delete;
inline double toDouble(const StringData *v) { return v? v->toDouble() : 0;}
inline double toDouble(CStrRef v) { return toDouble(v.get());}
inline double toDouble(const ArrayData *v) {
  return (v && !v->empty()) ? 1.0 : 0.0;
}
inline double toDouble(CArrRef v) { return toDouble(v.get());}
inline double toDouble(const ObjectData *v) { return v ? v->o_toDouble() : 0;}
inline double toDouble(CObjRef v) { return toDouble(v.get());}
inline double toDouble(CVarRef v) { return v.toDouble();}

inline String toString(bool    v) { return v ? "1" : "";}
inline String toString(char    v) { return (int64_t)v;}
inline String toString(short   v) { return (int64_t)v;}
inline String toString(int     v) { return (int64_t)v;}
inline String toString(int64_t   v) { return v;}
inline String toString(double  v) { return v;}
inline String toString(litstr  v) = delete;
inline String toString(StringData *v) { return v ? String(v) : String("");}
inline String toString(CStrRef v) { return toString(v.get());}
inline String toString(const ArrayData *v) { return v ? "Array" : "";}
inline String toString(CArrRef v) { return toString(v.get());}
inline String toString(ObjectData *v) {
  return v ? v->invokeToString() : String("");
}
inline String toString(CObjRef v) { return toString(v.get());}
inline String toString(CVarRef v) { return v.toString();}

inline Array toArray(bool    v) { return Array::Create(v);}
inline Array toArray(char    v) { return Array::Create(v);}
inline Array toArray(short   v) { return Array::Create(v);}
inline Array toArray(int     v) { return Array::Create(v);}
inline Array toArray(int64_t   v) { return Array::Create(v);}
inline Array toArray(double  v) { return Array::Create(v);}
inline Array toArray(litstr  v) = delete;
inline Array toArray(StringData *v) {
  return v ? Array::Create(v) : Array::Create();
}
inline Array toArray(CStrRef v) { return toArray(v.get());}
inline Array toArray(ArrayData *v) { return v ? Array(v) : Array::Create();}
inline Array toArray(CArrRef v) { return toArray(v.get());}
inline Array toArray(const ObjectData *v) {
  return v ? v->o_toArray() : Array::Create();
}
inline Array toArray(CObjRef v) { return toArray(v.get());}
inline Array toArray(CVarRef v) { return v.toArray();}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TYPE_CONVERSIONS_H_
