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

#ifndef __HPHP_TYPE_CONVERSIONS_H__
#define __HPHP_TYPE_CONVERSIONS_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>

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

inline char toByte(bool    v) { return v ? 1 : 0;}
inline char toByte(char    v) { return v;}
inline char toByte(short   v) { return v;}
inline char toByte(int     v) { return v;}
inline char toByte(int64   v) { return v;}
inline char toByte(double  v) { return (char)v;}
inline char toByte(litstr  v) { return StringData(v).toByte();}
inline char toByte(const StringData *v) { return v ? v->toByte() : 0;}
inline char toByte(CStrRef v) { return toByte(v.get());}
inline char toByte(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline char toByte(CArrRef v) { return toByte(v.get());}
inline char toByte(const ObjectData *v) { return v ? v->o_toInt64() : 0;}
inline char toByte(CObjRef v) { return toByte(v.get());}
inline char toByte(CVarRef v) { return v.toByte();}

inline short toInt16(bool    v) { return v ? 1 : 0;}
inline short toInt16(char    v) { return v;}
inline short toInt16(short   v) { return v;}
inline short toInt16(int     v) { return v;}
inline short toInt16(int64   v) { return v;}
inline short toInt16(double  v) { return (short)v;}
inline short toInt16(litstr  v) { return StringData(v).toInt16();}
inline short toInt16(const StringData *v) { return v ? v->toInt16() : 0;}
inline short toInt16(CStrRef v) { return toInt16(v.get());}
inline short toInt16(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline short toInt16(CArrRef v) { return toInt16(v.get());}
inline short toInt16(const ObjectData *v) { return v ? v->o_toInt64() : 0;}
inline short toInt16(CObjRef v) { return toInt16(v.get());}
inline short toInt16(CVarRef v) { return v.toInt16();}

inline int toInt32(bool    v) { return v ? 1 : 0;}
inline int toInt32(char    v) { return v;}
inline int toInt32(short   v) { return v;}
inline int toInt32(int     v) { return v;}
inline int toInt32(int64   v) { return v;}
inline int toInt32(double  v) { return (int)v;}
inline int toInt32(litstr  v) { return StringData(v).toInt32();}
inline int toInt32(const StringData *v) { return v ? v->toInt32() : 0;}
inline int toInt32(CStrRef v) { return toInt32(v.get());}
inline int toInt32(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline int toInt32(CArrRef v) { return toInt32(v.get());}
inline int toInt32(const ObjectData *v) { return v ? v->o_toInt64() : 0;}
inline int toInt32(CObjRef v) { return toInt32(v.get());}
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
inline int64 toInt64(const StringData *v) { return v ? v->toInt64() : 0;}
inline int64 toInt64(CStrRef v) { return toInt64(v.get());}
inline int64 toInt64(const ArrayData *v) { return (v && !v->empty()) ? 1 : 0;}
inline int64 toInt64(CArrRef v) { return toInt64(v.get());}
inline int64 toInt64(const ObjectData *v) { return v ? v->o_toInt64() : 0;}
inline int64 toInt64(CObjRef v) { return toInt64(v.get());}
inline int64 toInt64(CVarRef v) { return v.toInt64();}

inline double toDouble(bool    v) { return v ? 1 : 0;}
inline double toDouble(char    v) { return v;}
inline double toDouble(short   v) { return v;}
inline double toDouble(int     v) { return v;}
inline double toDouble(int64   v) { return v;}
inline double toDouble(double  v) { return v;}
inline double toDouble(litstr  v) { return StringData(v).toDouble();}
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
inline String toString(char    v) { return (int64)v;}
inline String toString(short   v) { return (int64)v;}
inline String toString(int     v) { return (int64)v;}
inline String toString(int64   v) { return v;}
inline String toString(double  v) { return v;}
inline String toString(litstr  v) { return v;}
inline String toString(StringData *v) { return v;}
inline String toString(CStrRef v) { return v;}
inline String toString(const ArrayData *v) { return v ? "Array" : "";}
inline String toString(CArrRef v) { return toString(v.get());}
inline String toString(ObjectData *v) {
  return v ? v->t___tostring() : String();
}
inline String toString(CObjRef v) { return toString(v.get());}
inline String toString(CVarRef v) { return v.toString();}

inline Array toArray(bool    v) { return Array::Create(v);}
inline Array toArray(char    v) { return Array::Create(v);}
inline Array toArray(short   v) { return Array::Create(v);}
inline Array toArray(int     v) { return Array::Create(v);}
inline Array toArray(int64   v) { return Array::Create(v);}
inline Array toArray(double  v) { return Array::Create(v);}
inline Array toArray(litstr  v) { return Array::Create(v);}
inline Array toArray(StringData *v) { return Array::Create(v);}
inline Array toArray(CStrRef v) { return Array::Create(v);}
inline Array toArray(ArrayData *v) { return v;}
inline Array toArray(CArrRef v) { return v;}
inline Array toArray(const ObjectData *v) {
  return v ? v->o_toArray() : Array();
}
inline Array toArray(CObjRef v) { return toArray(v.get());}
inline Array toArray(CVarRef v) { return v.toArray();}

inline Object toObject(bool    v) { return Variant(v).toObject();}
inline Object toObject(char    v) { return Variant(v).toObject();}
inline Object toObject(short   v) { return Variant(v).toObject();}
inline Object toObject(int     v) { return Variant(v).toObject();}
inline Object toObject(int64   v) { return Variant(v).toObject();}
inline Object toObject(double  v) { return Variant(v).toObject();}
inline Object toObject(litstr  v) { return Variant(v).toObject();}
inline Object toObject(const StringData *v) { return Variant(v).toObject();}
inline Object toObject(CStrRef v) { return Variant(v).toObject();}
Object toObject(ArrayData *v);
inline Object toObject(CArrRef v) { return toObject(v.get());}
inline Object toObject(ObjectData *v) { return v;}
inline Object toObject(CObjRef v) { return v;}
inline Object toObject(CVarRef v) { return v.toObject();}

inline const String *const toSPOD(CStrRef v) { return &v;}
inline const Variant *toVPOD(CVarRef v) { return &v;}
inline const Object *toOPOD(CObjRef v) { return &v;}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TYPE_CONVERSIONS_H__
