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

#ifndef incl_HPHP_COMPARISONS_H_
#define incl_HPHP_COMPARISONS_H_

#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/tv-comparisons.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Variant

bool same(const Variant& v1, bool v2);
bool same(const Variant& v1, int64_t v2);
inline bool same(const Variant& v1, int v2) { return same(v1, (int64_t)v2); }
bool same(const Variant& v1, double v2);
bool same(const Variant& v1, const StringData* v2);
bool same(const Variant& v1, const String& v2);
bool same(const Variant& v1, litstr v2) = delete;
bool same(const Variant& v1, const Array& v2);
bool same(const Variant& v1, const Object& v2);
bool same(const Variant& v1, const Resource& v2);
inline bool same(const Variant& v1, const Variant& v2) {
  return tvSame(*v1.asTypedValue(), *v2.asTypedValue());
}

inline bool equal(const Variant& v1, bool v2) { return cellEqual(*v1.asCell(), v2); }
inline bool equal(const Variant& v1, int v2) { return cellEqual(*v1.asCell(), v2); }
inline bool equal(const Variant& v1, int64_t v2) { return cellEqual(*v1.asCell(), v2);}
inline bool equal(const Variant& v1, double  v2) { return cellEqual(*v1.asCell(), v2);}
inline bool equal(const Variant& v1, const StringData* v2) {
  return cellEqual(*v1.asCell(), v2);
}
inline bool equal(const Variant& v1, const String& v2) {
  if (!v2.get()) return cellEqual(*v1.asCell(), false);
  return cellEqual(*v1.asCell(), v2.get());
}
inline bool equal(const Variant& v1, litstr  v2) = delete;
inline bool equal(const Variant& v1, const Array& v2) {
  if (!v2.get()) return cellEqual(*v1.asCell(), false);
  return cellEqual(*v1.asCell(), v2.get());
}
inline bool equal(const Variant& v1, const Object& v2) {
  if (!v2.get()) return cellEqual(*v1.asCell(), false);
  return cellEqual(*v1.asCell(), v2.get());
}
inline bool equal(const Variant& v1, const Resource& v2) {
  if (!v2.get()) return cellEqual(*v1.asCell(), false);
  return cellEqual(*v1.asCell(), v2.get());
}
inline bool equal(const Variant& v1, const Variant& v2) {
  return tvEqual(*v1.asTypedValue(), *v2.asTypedValue());
}

inline bool less(const Variant& v1, bool v2) {
  return cellLess(*v1.asCell(), v2);
}
inline bool less(const Variant& v1, int v2) {
  return cellLess(*v1.asCell(), v2);
}
inline bool less(const Variant& v1, int64_t v2) {
  return cellLess(*v1.asCell(), v2);
}
inline bool less(const Variant& v1, double v2) {
  return cellLess(*v1.asCell(), v2);
}
inline bool less(const Variant& v1, const StringData* v2) {
  return cellLess(*v1.asCell(), v2);
}
inline bool less(const Variant& v1, const String& v2) {
  if (!v2.get()) return cellLess(*v1.asCell(), false);
  return cellLess(*v1.asCell(), v2.get());
}
inline bool less(const Variant& v1, litstr  v2) = delete;
inline bool less(const Variant& v1, const Array& v2) {
  if (!v2.get()) return cellLess(*v1.asCell(), false);
  return cellLess(*v1.asCell(), v2.get());
}
inline bool less(const Variant& v1, const Object& v2) {
  if (!v2.get()) return cellLess(*v1.asCell(), false);
  return cellLess(*v1.asCell(), v2.get());
}
inline bool less(const Variant& v1, const Resource& v2) {
  if (!v2.get()) return cellLess(*v1.asCell(), false);
  return cellLess(*v1.asCell(), v2.get());
}
inline bool less(const Variant& v1, const Variant& v2) {
  return tvLess(*v1.asTypedValue(), *v2.asTypedValue());
}

inline bool more(const Variant& v1, bool v2) {
  return cellGreater(*v1.asCell(), v2);
}
inline bool more(const Variant& v1, int v2) {
  return cellGreater(*v1.asCell(), v2);
}
inline bool more(const Variant& v1, int64_t v2) {
  return cellGreater(*v1.asCell(), v2);
}
inline bool more(const Variant& v1, double v2) {
  return cellGreater(*v1.asCell(), v2);
}
inline bool more(const Variant& v1, const StringData* v2) {
  return cellGreater(*v1.asCell(), v2);
}
inline bool more(const Variant& v1, const String& v2) {
  if (!v2.get()) return cellGreater(*v1.asCell(), false);
  return cellGreater(*v1.asCell(), v2.get());
}
inline bool more(const Variant& v1, litstr  v2) = delete;
inline bool more(const Variant& v1, const Array& v2) {
  if (!v2.get()) return cellGreater(*v1.asCell(), false);
  return cellGreater(*v1.asCell(), v2.get());
}
inline bool more(const Variant& v1, const Object& v2) {
  if (!v2.get()) return cellGreater(*v1.asCell(), false);
  return cellGreater(*v1.asCell(), v2.get());
}
inline bool more(const Variant& v1, const Resource& v2) {
  if (!v2.get()) return cellGreater(*v1.asCell(), false);
  return cellGreater(*v1.asCell(), v2.get());
}
inline bool more(const Variant& v1, const Variant& v2) {
  return tvGreater(*v1.asTypedValue(), *v2.asTypedValue());
}

///////////////////////////////////////////////////////////////////////////////
// bool

inline bool same(bool v1, bool    v2) = delete;
inline bool same(bool v1, int     v2) = delete;
inline bool same(bool v1, int64_t   v2) = delete;
inline bool same(bool v1, double  v2) = delete;
inline bool same(bool v1, const StringData *v2)  { return false;}
inline bool same(bool v1, const String& v2)  { return false;}
inline bool same(bool v1, litstr  v2) = delete;
inline bool same(bool v1, const Array& v2)  { return false;}
inline bool same(bool v1, const Object& v2)  { return false;}
inline bool same(bool v1, const Resource& v2)  { return false;}
inline bool same(bool v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(bool v1, bool    v2) { return v1 == v2;}
inline bool equal(bool v1, int     v2) { return v1 == (v2 != 0);}
inline bool equal(bool v1, int64_t   v2) { return v1 == (v2 != 0);}
inline bool equal(bool v1, double  v2) { return v1 == (v2 != 0.0);}
inline bool equal(bool v1, const StringData *v2) { return v1 == toBoolean(v2);}
inline bool equal(bool v1, const String& v2) { return v1 == v2.toBoolean();}
inline bool equal(bool v1, litstr  v2) = delete;
inline bool equal(bool v1, const Array& v2) { return v1 == v2.toBoolean();}
inline bool equal(bool v1, const Object& v2) { return v1 == v2.toBoolean();}
inline bool equal(bool v1, const Resource& v2) { return v1 == v2.toBoolean();}
inline bool equal(bool v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(bool v1, bool    v2)  { return (v1?1:0) < (v2?1:0);}
inline bool less(bool v1, int     v2)  { return less(v1,(v2 != 0));}
inline bool less(bool v1, int64_t   v2)  { return less(v1,(v2 != 0));}
inline bool less(bool v1, double  v2)  { return less(v1,(v2 != 0.0));}
inline bool less(bool v1, const StringData *v2)  {
  return less(v1,toBoolean(v2));
}
inline bool less(bool v1, const String& v2)  { return less(v1,v2.toBoolean());}
inline bool less(bool v1, litstr  v2) = delete;
inline bool less(bool v1, const Array& v2)  { return less(v1,v2.toBoolean());}
inline bool less(bool v1, const Object& v2)  { return less(v1,v2.toBoolean());}
inline bool less(bool v1, const Resource& v2)  { return less(v1,v2.toBoolean());}
inline bool less(bool v1, const Variant& v2)  { return more(v2,v1);}

inline bool more(bool v1, bool    v2)  { return (v1?1:0) > (v2?1:0);}
inline bool more(bool v1, int     v2)  { return more(v1,(v2 != 0));}
inline bool more(bool v1, int64_t   v2)  { return more(v1,(v2 != 0));}
inline bool more(bool v1, double  v2)  { return more(v1,(v2 != 0.0));}
inline bool more(bool v1, const StringData *v2)  {
  return more(v1,toBoolean(v2));
}
inline bool more(bool v1, const String& v2)  { return more(v1,v2.toBoolean());}
inline bool more(bool v1, litstr  v2)  = delete;
inline bool more(bool v1, const Array& v2)  { return more(v1,v2.toBoolean());}
inline bool more(bool v1, const Object& v2)  { return more(v1,v2.toBoolean());}
inline bool more(bool v1, const Resource& v2)  { return more(v1,v2.toBoolean());}
inline bool more(bool v1, const Variant& v2)  { return less(v2,v1);}

///////////////////////////////////////////////////////////////////////////////
// int

inline bool same(int v1, bool    v2) = delete;
inline bool same(int v1, int     v2) = delete;
inline bool same(int v1, int64_t   v2) = delete;
inline bool same(int v1, double  v2) = delete;
inline bool same(int v1, const StringData *v2)  { return false;}
inline bool same(int v1, const String& v2)  { return false;}
inline bool same(int v1, litstr  v2)  = delete;
inline bool same(int v1, const Array& v2)  { return false;}
inline bool same(int v1, const Object& v2)  { return false;}
inline bool same(int v1, const Resource& v2)  { return false;}
inline bool same(int v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(int v1, bool    v2) { return equal(v2, v1);}
inline bool equal(int v1, int     v2) { return v1 == v2;}
inline bool equal(int v1, int64_t   v2) { return v1 == v2;}
inline bool equal(int v1, double  v2) { return (double)v1 == v2;}
bool equal(int v1, const StringData *v2);
inline bool equal(int v1, const String& v2) { return equal(v1, v2.get());}
inline bool equal(int v1, litstr  v2) = delete;
inline bool equal(int v1, const Array& v2) { return false;}
inline bool equal(int v1, const Object& v2) {
  return v2->isCollection() ? false : equal(v1, v2.toInt64());
}
inline bool equal(int v1, const Resource& v2) {
  return equal(v1, v2.toInt64());
}
inline bool equal(int v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(int v1, bool    v2)  { return more(v2, v1);}
inline bool less(int v1, int     v2)  { return v1 < v2;}
inline bool less(int v1, int64_t   v2)  { return v1 < v2;}
inline bool less(int v1, double  v2)  { return v1 < v2;}
bool less(int v1, const StringData *v2);
inline bool less(int v1, const String& v2) { return less(v1, v2.get());}
inline bool less(int v1, litstr  v2)  = delete;
inline bool less(int v1, const Array& v2)  { return true;}
inline bool less(int v1, const Object& v2)  {
  return less(v1, v2.toInt64ForCompare());
}
inline bool less(int v1, const Resource& v2)  { return less(v1, v2.toInt64());}
inline bool less(int v1, const Variant& v2)  { return more(v2, v1);}

inline bool more(int v1, bool    v2)  { return less(v2, v1);}
inline bool more(int v1, int     v2)  { return v1 > v2;}
inline bool more(int v1, int64_t   v2)  { return v1 > v2;}
inline bool more(int v1, double  v2)  { return v1 > v2;}
bool more(int v1, const StringData *v2);
inline bool more(int v1, const String& v2) { return more(v1, v2.get());}
inline bool more(int v1, litstr  v2)  = delete;
inline bool more(int v1, const Array& v2)  { return false;}
inline bool more(int v1, const Object& v2)  {
  return more(v1, v2.toInt64ForCompare());
}
inline bool more(int v1, const Resource& v2)  { return more(v1, v2.toInt64());}
inline bool more(int v1, const Variant& v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// int64

inline bool same(int64_t v1, bool    v2) = delete;
inline bool same(int64_t v1, int     v2) = delete;
inline bool same(int64_t v1, int64_t   v2) = delete;
inline bool same(int64_t v1, double  v2) = delete;
inline bool same(int64_t v1, const StringData *v2)  { return false;}
inline bool same(int64_t v1, const String& v2)  { return false;}
inline bool same(int64_t v1, litstr  v2)  = delete;
inline bool same(int64_t v1, const Array& v2)  { return false;}
inline bool same(int64_t v1, const Object& v2)  { return false;}
inline bool same(int64_t v1, const Resource& v2)  { return false;}
inline bool same(int64_t v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(int64_t v1, bool    v2) { return equal(v2, v1);}
inline bool equal(int64_t v1, int     v2) { return equal(v2, v1);}
inline bool equal(int64_t v1, int64_t   v2) { return v1 == v2;}
inline bool equal(int64_t v1, double  v2) { return (double)v1 == v2;}
bool equal(int64_t v1, const StringData *v2);
inline bool equal(int64_t v1, const String& v2) { return equal(v1, v2.get());}
inline bool equal(int64_t v1, litstr  v2) = delete;
inline bool equal(int64_t v1, const Array& v2) { return false;}
inline bool equal(int64_t v1, const Object& v2) {
  return v2->isCollection() ? false : equal(v1, v2.toInt64());
}
inline bool equal(int64_t v1, const Resource& v2) { return equal(v1, v2.toInt64());}
inline bool equal(int64_t v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(int64_t v1, bool    v2)  { return more(v2, v1);}
inline bool less(int64_t v1, int     v2)  { return more(v2, v1);}
inline bool less(int64_t v1, int64_t   v2)  { return v1 < v2;}
inline bool less(int64_t v1, double  v2)  { return v1 < v2;}
bool less(int64_t v1, const StringData *v2);
inline bool less(int64_t v1, const String& v2) { return less(v1, v2.get());}
inline bool less(int64_t v1, litstr  v2)  = delete;
inline bool less(int64_t v1, const Array& v2)  { return true;}
inline bool less(int64_t v1, const Object& v2)  {
  return less(v1, v2.toInt64ForCompare());
}
inline bool less(int64_t v1, const Resource& v2)  { return less(v1, v2.toInt64());}
inline bool less(int64_t v1, const Variant& v2)  { return more(v2, v1);}

inline bool more(int64_t v1, bool    v2)  { return less(v2, v1);}
inline bool more(int64_t v1, int     v2)  { return less(v2, v1);}
inline bool more(int64_t v1, int64_t   v2)  { return v1 > v2;}
inline bool more(int64_t v1, double  v2)  { return v1 > v2;}
bool more(int64_t v1, const StringData *v2);
inline bool more(int64_t v1, const String& v2) { return more(v1, v2.get());}
inline bool more(int64_t v1, litstr  v2)  = delete;
inline bool more(int64_t v1, const Array& v2)  { return false;}
inline bool more(int64_t v1, const Object& v2)  {
  return more(v1, v2.toInt64ForCompare());
}
inline bool more(int64_t v1, const Resource& v2)  { return more(v1, v2.toInt64());}
inline bool more(int64_t v1, const Variant& v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// double

inline bool same(double v1, bool    v2) = delete;
inline bool same(double v1, int     v2) = delete;
inline bool same(double v1, int64_t   v2) = delete;
inline bool same(double v1, double  v2) = delete;
inline bool same(double v1, const StringData *v2)  { return false;}
inline bool same(double v1, const String& v2)  { return false;}
inline bool same(double v1, litstr  v2)  = delete;
inline bool same(double v1, const Array& v2)  { return false;}
inline bool same(double v1, const Object& v2)  { return false;}
inline bool same(double v1, const Resource& v2)  { return false;}
inline bool same(double v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(double v1, bool    v2) { return equal(v2, v1);}
inline bool equal(double v1, int     v2) { return equal(v2, v1);}
inline bool equal(double v1, int64_t   v2) { return equal(v2, v1);}
inline bool equal(double v1, double  v2) { return v1 == v2;}
inline bool equal(double v1, const StringData *v2) {
  return v1 == toDouble(v2);
}
inline bool equal(double v1, const String& v2) { return v1 == v2.toDouble();}
inline bool equal(double v1, litstr  v2) = delete;
inline bool equal(double v1, const Array& v2) { return false;}
inline bool equal(double v1, const Object& v2) {
  if (v2->isCollection()) return false;
  return equal(v1, v2.toDouble());
}
inline bool equal(double v1, const Resource& v2) { return equal(v1, v2.toDouble());}
inline bool equal(double v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(double v1, bool    v2)  { return more(v2, v1);}
inline bool less(double v1, int     v2)  { return more(v2, v1);}
inline bool less(double v1, int64_t   v2)  { return more(v2, v1);}
inline bool less(double v1, double  v2)  { return v1 < v2;}
inline bool less(double v1, const StringData *v2)  {
  return less(v1,toDouble(v2));
}
inline bool less(double v1, const String& v2)  { return less(v1,v2.toDouble());}
inline bool less(double v1, litstr  v2)  = delete;
inline bool less(double v1, const Array& v2)  { return true;}
inline bool less(double v1, const Object& v2)  {
  return less(v1, v2.toDoubleForCompare());
}
inline bool less(double v1, const Resource& v2)  { return less(v1, v2.toDouble());}
inline bool less(double v1, const Variant& v2)  { return more(v2, v1);}

inline bool more(double v1, bool    v2)  { return less(v2, v1);}
inline bool more(double v1, int     v2)  { return less(v2, v1);}
inline bool more(double v1, int64_t   v2)  { return less(v2, v1);}
inline bool more(double v1, double  v2)  { return v1 > v2;}
inline bool more(double v1, const StringData *v2)  {
  return more(v1,toDouble(v2));
}
inline bool more(double v1, const String& v2)  { return more(v1,v2.toDouble());}
inline bool more(double v1, litstr  v2)  = delete;
inline bool more(double v1, const Array& v2)  { return false;}
inline bool more(double v1, const Object& v2)  {
  return more(v1, v2.toDoubleForCompare());
}
inline bool more(double v1, const Resource& v2)  { return more(v1, v2.toDouble());}
inline bool more(double v1, const Variant& v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// StringData *

inline bool same(const StringData *v1, bool    v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, int     v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, int64_t   v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, double  v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, const StringData *v2) {
  if (v1 == v2) return true;
  if (v1 && v2) return v1->same(v2);
  return false;
}
inline bool same(const StringData *v1, const String& v2)  {
  return same(v1, v2.get());
}
inline bool same(const StringData *v1, litstr  v2) = delete;
inline bool same(const StringData *v1, const Array& v2)  { return false; }
inline bool same(const StringData *v1, const Object& v2)  { return false; }
inline bool same(const StringData *v1, const Resource& v2)  { return false; }
inline bool same(const StringData *v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(const StringData *v1, bool    v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, int     v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, int64_t   v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, double  v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, const StringData *v2) {
  if (v1 == v2) return true;
  if (v1 == nullptr) return v2->empty();
  if (v2 == nullptr) return v1->empty();
  return v1->equal(v2);
}
inline bool equal(const StringData *v1, const String& v2) {
  return equal(v1, v2.get());
}
inline bool equal(const StringData *v1, litstr  v2) = delete;
inline bool equal(const StringData *v1, const Array& v2) {
  if (v1 == nullptr || v2.get() == nullptr) {
    return equal(toBoolean(v1), v2.toBoolean());
  }
  return false;
}
inline bool equal(const StringData *v1, const Object& v2) {
  if (v1 == nullptr || v2.get() == nullptr) {
    return equal(toBoolean(v1), v2.toBoolean());
  }
  if (v2->isCollection()) return false;
  if (!v2->hasToString()) return false;
  return equal(v1, v2.toString());
}
inline bool equal(const StringData *v1, const Resource& v2) {
  if (v1 == nullptr || v2.get() == nullptr) {
    return equal(toBoolean(v1), v2.toBoolean());
  }
  return false;
}
inline bool equal(const StringData *v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(const StringData *v1, bool    v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, int     v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, int64_t   v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, double  v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, const StringData *v2) {
  if (v1 == v2 || v2 == nullptr) return false;
  if (v1 == nullptr) return !v2->empty();
  return v1->compare(v2) < 0;
}
inline bool less(const StringData *v1, const String& v2)  {
  return less(v1, v2.get());
}
inline bool less(const StringData *v1, litstr  v2) = delete;
inline bool less(const StringData *v1, const Array& v2)  {
  if (v1 == nullptr || v2.get() == nullptr) {
    return less(toBoolean(v1), v2.toBoolean());
  }
  return true;
}
inline bool less(const StringData *v1, const Object& v2)  {
  if (v1 == nullptr || v2.get() == nullptr) {
    return less(toBoolean(v1), v2.toBoolean());
  }
  check_collection_compare(v2.get());
  if (!v2->hasToString()) return true;
  return less(v1, v2.toString());
}
inline bool less(const StringData *v1, const Resource& v2)  {
  if (v1 == nullptr || v2.get() == nullptr) {
    return less(toBoolean(v1), v2.toBoolean());
  }
  return true;
}
inline bool less(const StringData *v1, const Variant& v2)  { return more(v2, v1);}

inline bool more(const StringData *v1, bool    v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, int     v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, int64_t   v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, double  v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, const StringData *v2)  {
  if (v1 == nullptr) return false;
  if (v2 == nullptr) return !v1->empty();
  return v1->compare(v2) > 0;
}
inline bool more(const StringData *v1, const String& v2)  {
  return more(v1, v2.get());
}
inline bool more(const StringData *v1, litstr  v2) = delete;
inline bool more(const StringData *v1, const Array& v2)  {
  if (v1 == nullptr || v2.get() == nullptr) {
    return more(toBoolean(v1), v2.toBoolean());
  }
  return false;
}
inline bool more(const StringData *v1, const Object& v2)  {
  if (v1 == nullptr || v2.get() == nullptr) {
    return more(toBoolean(v1), v2.toBoolean());
  }
  check_collection_compare(v2.get());
  if (!v2->hasToString()) return false;
  return more(v1, v2.toString());
}
inline bool more(const StringData *v1, const Resource& v2)  {
  if (v1 == nullptr || v2.get() == nullptr) {
    return more(toBoolean(v1), v2.toBoolean());
  }
  return false;
}
inline bool more(const StringData *v1, const Variant& v2)  { return less(v2, v1);}


///////////////////////////////////////////////////////////////////////////////
// String

inline bool same(const String& v1, bool    v2)  { return same(v2, v1);}
inline bool same(const String& v1, int     v2)  { return same(v2, v1);}
inline bool same(const String& v1, int64_t   v2)  { return same(v2, v1);}
inline bool same(const String& v1, double  v2)  { return same(v2, v1);}
inline bool same(const String& v1, const StringData *v2)  {
  return same(v2, v1.get());
}
inline bool same(const String& v1, const String& v2)  { return v1.same(v2);}
inline bool same(const String& v1, litstr  v2)  = delete;
inline bool same(const String& v1, const Array& v2)  { return v1.same(v2);}
inline bool same(const String& v1, const Object& v2)  { return v1.same(v2);}
inline bool same(const String& v1, const Resource& v2)  { return v1.same(v2);}
inline bool same(const String& v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(const String& v1, bool    v2) { return equal(v2, v1);}
inline bool equal(const String& v1, int     v2) { return equal(v2, v1);}
inline bool equal(const String& v1, int64_t   v2) { return equal(v2, v1);}
inline bool equal(const String& v1, double  v2) { return equal(v2, v1);}
inline bool equal(const String& v1, const StringData *v2) {
  return equal(v2, v1.get());
}
inline bool equal(const String& v1, const String& v2) { return v1.equal(v2);}
inline bool equal(const String& v1, litstr  v2) = delete;
inline bool equal(const String& v1, const Array& v2) { return v1.equal(v2);}
inline bool equal(const String& v1, const Object& v2) { return v1.equal(v2);}
inline bool equal(const String& v1, const Resource& v2) { return v1.equal(v2);}
inline bool equal(const String& v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(const String& v1, bool    v2)  { return more(v2, v1);}
inline bool less(const String& v1, int     v2)  { return more(v2, v1);}
inline bool less(const String& v1, int64_t   v2)  { return more(v2, v1);}
inline bool less(const String& v1, double  v2)  { return more(v2, v1);}
inline bool less(const String& v1, const StringData *v2)  {
  return more(v2, v1.get());
}
inline bool less(const String& v1, const String& v2)  { return v1.less(v2);}
inline bool less(const String& v1, litstr  v2)  = delete;
inline bool less(const String& v1, const Array& v2)  { return v1.less(v2);}
inline bool less(const String& v1, const Object& v2)  { return v1.less(v2);}
inline bool less(const String& v1, const Resource& v2)  { return v1.less(v2);}
inline bool less(const String& v1, const Variant& v2)  { return more(v2, v1);}

inline bool more(const String& v1, bool    v2)  { return less(v2, v1);}
inline bool more(const String& v1, int     v2)  { return less(v2, v1);}
inline bool more(const String& v1, int64_t   v2)  { return less(v2, v1);}
inline bool more(const String& v1, double  v2)  { return less(v2, v1);}
inline bool more(const String& v1, const StringData *v2)  {
  return less(v2, v1.get());
}
inline bool more(const String& v1, const String& v2)  { return v1.more(v2);}
inline bool more(const String& v1, litstr  v2)  = delete;
inline bool more(const String& v1, const Array& v2)  { return v1.more(v2);}
inline bool more(const String& v1, const Object& v2)  { return v1.more(v2);}
inline bool more(const String& v1, const Resource& v2)  { return v1.more(v2);}
inline bool more(const String& v1, const Variant& v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// litstr

inline bool same(litstr v1, bool    v2) = delete;
inline bool same(litstr v1, int     v2) = delete;
inline bool same(litstr v1, int64_t   v2) = delete;
inline bool same(litstr v1, double  v2) = delete;
inline bool same(litstr v1, const StringData *v2)  = delete;
inline bool same(litstr v1, const String& v2) = delete;
inline bool same(litstr v1, litstr  v2) = delete;
inline bool same(litstr v1, const Array& v2) = delete;
inline bool same(litstr v1, const Object& v2) = delete;
inline bool same(litstr v1, const Resource& v2) = delete;
inline bool same(litstr v1, const Variant& v2) = delete;

inline bool equal(litstr v1, bool    v2)= delete;
inline bool equal(litstr v1, int     v2)= delete;
inline bool equal(litstr v1, int64_t   v2)= delete;
inline bool equal(litstr v1, double  v2)= delete;
inline bool equal(litstr v1, const StringData *v2)= delete;
inline bool equal(litstr v1, const String& v2)= delete;
inline bool equal(litstr v1, litstr  v2)= delete;
inline bool equal(litstr v1, const Array& v2)= delete;
inline bool equal(litstr v1, const Object& v2)= delete;
inline bool equal(litstr v1, const Resource& v2)= delete;
inline bool equal(litstr v1, const Variant& v2)= delete;

inline bool less(litstr v1, bool    v2) = delete;
inline bool less(litstr v1, int     v2) = delete;
inline bool less(litstr v1, int64_t   v2) = delete;
inline bool less(litstr v1, double  v2) = delete;
inline bool less(litstr v1, const StringData *v2) = delete;
inline bool less(litstr v1, const String& v2) = delete;
inline bool less(litstr v1, litstr  v2) = delete;
inline bool less(litstr v1, const Array& v2) = delete;
inline bool less(litstr v1, const Object& v2) = delete;
inline bool less(litstr v1, const Resource& v2) = delete;
inline bool less(litstr v1, const Variant& v2) = delete;

inline bool more(litstr v1, bool    v2) = delete;
inline bool more(litstr v1, int     v2) = delete;
inline bool more(litstr v1, int64_t   v2) = delete;
inline bool more(litstr v1, double  v2) = delete;
inline bool more(litstr v1, const StringData *v2) = delete;
inline bool more(litstr v1, const String& v2) = delete;
inline bool more(litstr v1, litstr  v2) = delete;
inline bool more(litstr v1, const Array& v2) = delete;
inline bool more(litstr v1, const Object& v2) = delete;
inline bool more(litstr v1, const Resource& v2) = delete;
inline bool more(litstr v1, const Variant& v2) = delete;

///////////////////////////////////////////////////////////////////////////////
// Array

inline bool same(const Array& v1, bool    v2)  { return same(v2, v1);}
inline bool same(const Array& v1, int     v2)  { return same(v2, v1);}
inline bool same(const Array& v1, int64_t   v2)  { return same(v2, v1);}
inline bool same(const Array& v1, double  v2)  { return same(v2, v1);}
inline bool same(const Array& v1, const StringData *v2)  { return same(v2, v1);}
inline bool same(const Array& v1, const String& v2)  { return same(v2, v1);}
inline bool same(const Array& v1, litstr  v2)  = delete;
inline bool same(const Array& v1, const Array& v2)  { return v1.same(v2);}
inline bool same(const Array& v1, const Object& v2)  { return v1.same(v2);}
inline bool same(const Array& v1, const Resource& v2)  { return false;}
inline bool same(const Array& v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(const Array& v1, bool    v2) { return equal(v2, v1);}
inline bool equal(const Array& v1, int     v2) { return equal(v2, v1);}
inline bool equal(const Array& v1, int64_t   v2) { return equal(v2, v1);}
inline bool equal(const Array& v1, double  v2) { return equal(v2, v1);}
inline bool equal(const Array& v1, const StringData *v2) { return equal(v2, v1);}
inline bool equal(const Array& v1, const String& v2) { return equal(v2, v1);}
inline bool equal(const Array& v1, litstr  v2) = delete;
inline bool equal(const Array& v1, const Array& v2) { return v1.equal(v2);}
inline bool equal(const Array& v1, const Object& v2) { return v1.equal(v2);}
inline bool equal(const Array& v1, const Resource& v2) { return false;}
inline bool equal(const Array& v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(const Array& v1, bool    v2)  { return more(v2, v1);}
inline bool less(const Array& v1, int     v2)  { return more(v2, v1);}
inline bool less(const Array& v1, int64_t   v2)  { return more(v2, v1);}
inline bool less(const Array& v1, double  v2)  { return more(v2, v1);}
inline bool less(const Array& v1, const StringData *v2)  { return more(v2, v1);}
inline bool less(const Array& v1, const String& v2)  { return more(v2, v1);}
inline bool less(const Array& v1, litstr  v2) = delete;
inline bool less(const Array& v1, const Array& v2)  { return v1.less(v2);}
inline bool less(const Array& v1, const Object& v2)  { return v1.less(v2);}
inline bool less(const Array& v1, const Resource& v2)  { return false;}
inline bool less(const Array& v1, const Variant& v2)  { return v1.less(v2);}

inline bool more(const Array& v1, bool    v2)  { return less(v2, v1);}
inline bool more(const Array& v1, int     v2)  { return less(v2, v1);}
inline bool more(const Array& v1, int64_t   v2)  { return less(v2, v1);}
inline bool more(const Array& v1, double  v2)  { return less(v2, v1);}
inline bool more(const Array& v1, const StringData *v2)  { return less(v2, v1);}
inline bool more(const Array& v1, const String& v2)  { return less(v2, v1);}
inline bool more(const Array& v1, litstr  v2)  = delete;
inline bool more(const Array& v1, const Array& v2)  { return v1.more(v2);}
inline bool more(const Array& v1, const Object& v2)  { return v1.more(v2);}
inline bool more(const Array& v1, const Resource& v2)  { return true;}
inline bool more(const Array& v1, const Variant& v2)  { return v1.more(v2);}

///////////////////////////////////////////////////////////////////////////////
// Object

inline bool same(const Object& v1, bool    v2)  { return same(v2, v1);}
inline bool same(const Object& v1, int     v2)  { return same(v2, v1);}
inline bool same(const Object& v1, int64_t   v2)  { return same(v2, v1);}
inline bool same(const Object& v1, double  v2)  { return same(v2, v1);}
inline bool same(const Object& v1, const StringData *v2)  { return same(v2, v1);}
inline bool same(const Object& v1, const String& v2)  { return same(v2, v1);}
inline bool same(const Object& v1, litstr  v2)  = delete;
inline bool same(const Object& v1, const Array& v2)  { return same(v2, v1);}
inline bool same(const Object& v1, const Object& v2)  { return v1.same(v2);}
inline bool same(const Object& v1, const Resource& v2)  { return false;}
inline bool same(const Object& v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(const Object& v1, bool    v2) { return equal(v2, v1);}
inline bool equal(const Object& v1, int     v2) { return equal(v2, v1);}
inline bool equal(const Object& v1, int64_t   v2) { return equal(v2, v1);}
inline bool equal(const Object& v1, double  v2) { return equal(v2, v1);}
inline bool equal(const Object& v1, const StringData *v2) { return equal(v2, v1);}
inline bool equal(const Object& v1, const String& v2) { return equal(v2, v1);}
inline bool equal(const Object& v1, litstr  v2) = delete;
inline bool equal(const Object& v1, const Array& v2) { return equal(v2, v1);}
inline bool equal(const Object& v1, const Object& v2) { return v1.equal(v2);}
inline bool equal(const Object& v1, const Resource& v2) { return false;}
inline bool equal(const Object& v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(const Object& v1, bool    v2)  { return more(v2, v1);}
inline bool less(const Object& v1, int     v2)  { return more(v2, v1);}
inline bool less(const Object& v1, int64_t   v2)  { return more(v2, v1);}
inline bool less(const Object& v1, double  v2)  { return more(v2, v1);}
inline bool less(const Object& v1, const StringData *v2)  { return more(v2, v1);}
inline bool less(const Object& v1, const String& v2)  { return more(v2, v1);}
inline bool less(const Object& v1, litstr  v2)  = delete;
inline bool less(const Object& v1, const Array& v2)  { return more(v2, v1);}
inline bool less(const Object& v1, const Object& v2)  { return v1.less(v2);}
inline bool less(const Object& v1, const Resource& v2)  { return false;}
inline bool less(const Object& v1, const Variant& v2)  {
  if (v2.isObject()) return v1.less(v2.toObject());
  return more(v2, v1);
}

inline bool more(const Object& v1, bool    v2)  { return less(v2, v1);}
inline bool more(const Object& v1, int     v2)  { return less(v2, v1);}
inline bool more(const Object& v1, int64_t   v2)  { return less(v2, v1);}
inline bool more(const Object& v1, double  v2)  { return less(v2, v1);}
inline bool more(const Object& v1, const StringData *v2)  { return less(v2, v1);}
inline bool more(const Object& v1, const String& v2)  { return less(v2, v1);}
inline bool more(const Object& v1, litstr  v2)  = delete;
inline bool more(const Object& v1, const Array& v2)  { return less(v2, v1);}
inline bool more(const Object& v1, const Object& v2)  { return v1.more(v2);}
inline bool more(const Object& v1, const Resource& v2)  { return false;}
inline bool more(const Object& v1, const Variant& v2)  {
  if (v2.isObject()) return v1.more(v2.toObject());
  return less(v2, v1);
}

///////////////////////////////////////////////////////////////////////////////
// Resource

inline bool same(const Resource& v1, bool    v2)  { return same(v2, v1);}
inline bool same(const Resource& v1, int     v2)  { return same(v2, v1);}
inline bool same(const Resource& v1, int64_t   v2)  { return same(v2, v1);}
inline bool same(const Resource& v1, double  v2)  { return same(v2, v1);}
inline bool same(const Resource& v1, const StringData *v2)  { return same(v2, v1);}
inline bool same(const Resource& v1, const String& v2)  { return same(v2, v1);}
inline bool same(const Resource& v1, litstr  v2)  = delete;
inline bool same(const Resource& v1, const Array& v2)  { return false;}
inline bool same(const Resource& v1, const Object& v2)  { return false;}
inline bool same(const Resource& v1, const Resource& v2)  { return v1.same(v2);}
inline bool same(const Resource& v1, const Variant& v2)  { return same(v2, v1);}

inline bool equal(const Resource& v1, bool    v2) { return equal(v2, v1);}
inline bool equal(const Resource& v1, int     v2) { return equal(v2, v1);}
inline bool equal(const Resource& v1, int64_t   v2) { return equal(v2, v1);}
inline bool equal(const Resource& v1, double  v2) { return equal(v2, v1);}
inline bool equal(const Resource& v1, const StringData *v2) { return equal(v2, v1);}
inline bool equal(const Resource& v1, const String& v2) { return equal(v2, v1);}
inline bool equal(const Resource& v1, litstr  v2) = delete;
inline bool equal(const Resource& v1, const Array& v2) { return false;}
inline bool equal(const Resource& v1, const Object& v2) { return false;}
inline bool equal(const Resource& v1, const Resource& v2) { return v1.equal(v2);}
inline bool equal(const Resource& v1, const Variant& v2) { return equal(v2, v1);}

inline bool less(const Resource& v1, bool    v2)  { return more(v2, v1);}
inline bool less(const Resource& v1, int     v2)  { return more(v2, v1);}
inline bool less(const Resource& v1, int64_t   v2)  { return more(v2, v1);}
inline bool less(const Resource& v1, double  v2)  { return more(v2, v1);}
inline bool less(const Resource& v1, const StringData *v2)  { return more(v2, v1);}
inline bool less(const Resource& v1, const String& v2)  { return more(v2, v1);}
inline bool less(const Resource& v1, litstr  v2)  = delete;
inline bool less(const Resource& v1, const Array& v2)  { return true;}
inline bool less(const Resource& v1, const Object& v2)  { return false;}
inline bool less(const Resource& v1, const Resource& v2)  { return v1.less(v2);}
inline bool less(const Resource& v1, const Variant& v2)  { return more(v2, v1);}

inline bool more(const Resource& v1, bool    v2)  { return less(v2, v1);}
inline bool more(const Resource& v1, int     v2)  { return less(v2, v1);}
inline bool more(const Resource& v1, int64_t   v2)  { return less(v2, v1);}
inline bool more(const Resource& v1, double  v2)  { return less(v2, v1);}
inline bool more(const Resource& v1, const StringData *v2)  { return less(v2, v1);}
inline bool more(const Resource& v1, const String& v2)  { return less(v2, v1);}
inline bool more(const Resource& v1, litstr  v2)  = delete;
inline bool more(const Resource& v1, const Array& v2)  { return false;}
inline bool more(const Resource& v1, const Object& v2)  { return false;}
inline bool more(const Resource& v1, const Resource& v2)  { return v1.more(v2);}
inline bool more(const Resource& v1, const Variant& v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_COMPARISONS_H_
