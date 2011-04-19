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

#ifndef __HPHP_COMPARISONS_H__
#define __HPHP_COMPARISONS_H__

#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Variant

inline bool same(CVarRef v1, bool    v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, int     v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, int64   v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, double  v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, const StringData *v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, CStrRef v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, litstr  v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, CArrRef v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, CObjRef v2)  { return v1.same(v2);}
inline bool same(CVarRef v1, CVarRef v2)  { return v1.same(v2);}

inline bool equal(CVarRef v1, bool    v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, int     v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, int64   v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, double  v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, const StringData *v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, CStrRef v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, litstr  v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, CArrRef v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, CObjRef v2) { return v1.equal(v2);}
inline bool equal(CVarRef v1, CVarRef v2) { return v1.equal(v2);}

inline bool equalAsStr(CVarRef v1, bool    v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, int     v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, int64   v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, double  v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, const StringData  *v2) {
  return v1.equalAsStr(v2);
}
inline bool equalAsStr(CVarRef v1, CStrRef v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, litstr  v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, CArrRef v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, CObjRef v2) { return v1.equalAsStr(v2);}
inline bool equalAsStr(CVarRef v1, CVarRef v2) { return v1.equalAsStr(v2);}


inline bool less(CVarRef v1, bool    v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, int     v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, int64   v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, double  v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, const StringData *v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, CStrRef v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, litstr  v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, CArrRef v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, CObjRef v2)  { return v1.less(v2);}
inline bool less(CVarRef v1, CVarRef v2)  { return v1.less(v2);}

inline bool more(CVarRef v1, bool    v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, int     v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, int64   v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, double  v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, const StringData *v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, CStrRef v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, litstr  v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, CArrRef v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, CObjRef v2)  { return v1.more(v2);}
inline bool more(CVarRef v1, CVarRef v2)  { return v1.more(v2);}

///////////////////////////////////////////////////////////////////////////////
// bool

inline bool same(bool v1, bool    v2)  { return v1 == v2;}
inline bool same(bool v1, int     v2)  { return false;}
inline bool same(bool v1, int64   v2)  { return false;}
inline bool same(bool v1, double  v2)  { return false;}
inline bool same(bool v1, const StringData *v2)  { return false;}
inline bool same(bool v1, CStrRef v2)  { return false;}
inline bool same(bool v1, litstr  v2)  { return false;}
inline bool same(bool v1, CArrRef v2)  { return false;}
inline bool same(bool v1, CObjRef v2)  { return false;}
inline bool same(bool v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(bool v1, bool    v2) { return v1 == v2;}
inline bool equal(bool v1, int     v2) { return v1 == (v2 != 0);}
inline bool equal(bool v1, int64   v2) { return v1 == (v2 != 0);}
inline bool equal(bool v1, double  v2) { return v1 == (v2 != 0.0);}
inline bool equal(bool v1, const StringData *v2) { return v1 == toBoolean(v2);}
inline bool equal(bool v1, CStrRef v2) { return v1 == v2.toBoolean();}
inline bool equal(bool v1, litstr  v2) { return equal(v1, String(v2));}
inline bool equal(bool v1, CArrRef v2) { return v1 == v2.toBoolean();}
inline bool equal(bool v1, CObjRef v2) { return v1 == v2.toBoolean();}
inline bool equal(bool v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(bool v1, bool    v2) { return v1 == v2;}
inline bool equalAsStr(bool v1, int     v2) { return v1 && v2 == 1;}
inline bool equalAsStr(bool v1, int64   v2) { return v1 && v2 == 1;}
inline bool equalAsStr(bool v1, double  v2) { return v1 && v2 == 1;}
bool equalAsStr(bool v1, const StringData *v2);
inline bool equalAsStr(bool v1, CStrRef v2) { return equalAsStr(v1, v2.get());}
inline bool equalAsStr(bool v1, litstr  v2) {
  return equalAsStr(v1, String(v2));
}
inline bool equalAsStr(bool v1, CArrRef v2) { return false;}
inline bool equalAsStr(bool v1, CObjRef v2) { return false;}
inline bool equalAsStr(bool v1, CVarRef v2) { return equalAsStr(v2, v1);}


inline bool less(bool v1, bool    v2)  { return (v1?1:0) < (v2?1:0);}
inline bool less(bool v1, int     v2)  { return less(v1,(v2 != 0));}
inline bool less(bool v1, int64   v2)  { return less(v1,(v2 != 0));}
inline bool less(bool v1, double  v2)  { return less(v1,(v2 != 0.0));}
inline bool less(bool v1, const StringData *v2)  {
  return less(v1,toBoolean(v2));
}
inline bool less(bool v1, CStrRef v2)  { return less(v1,v2.toBoolean());}
inline bool less(bool v1, litstr  v2)  { return less(v1,String(v2));}
inline bool less(bool v1, CArrRef v2)  { return less(v1,v2.toBoolean());}
inline bool less(bool v1, CObjRef v2)  { return less(v1,v2.toBoolean());}
inline bool less(bool v1, CVarRef v2)  { return more(v2,v1);}

inline bool more(bool v1, bool    v2)  { return (v1?1:0) > (v2?1:0);}
inline bool more(bool v1, int     v2)  { return more(v1,(v2 != 0));}
inline bool more(bool v1, int64   v2)  { return more(v1,(v2 != 0));}
inline bool more(bool v1, double  v2)  { return more(v1,(v2 != 0.0));}
inline bool more(bool v1, const StringData *v2)  {
  return more(v1,toBoolean(v2));
}
inline bool more(bool v1, CStrRef v2)  { return more(v1,v2.toBoolean());}
inline bool more(bool v1, litstr  v2)  { return more(v1,String(v2));}
inline bool more(bool v1, CArrRef v2)  { return more(v1,v2.toBoolean());}
inline bool more(bool v1, CObjRef v2)  { return more(v1,v2.toBoolean());}
inline bool more(bool v1, CVarRef v2)  { return less(v2,v1);}

///////////////////////////////////////////////////////////////////////////////
// int

inline bool same(int v1, bool    v2)  { return same(v2, v1);}
inline bool same(int v1, int     v2)  { return v1 == v2;}
inline bool same(int v1, int64   v2)  { return v1 == v2;}
inline bool same(int v1, double  v2)  { return (double)v1 == v2;}
inline bool same(int v1, const StringData *v2)  { return false;}
inline bool same(int v1, CStrRef v2)  { return false;}
inline bool same(int v1, litstr  v2)  { return false;}
inline bool same(int v1, CArrRef v2)  { return false;}
inline bool same(int v1, CObjRef v2)  { return false;}
inline bool same(int v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(int v1, bool    v2) { return equal(v2, v1);}
inline bool equal(int v1, int     v2) { return v1 == v2;}
inline bool equal(int v1, int64   v2) { return v1 == v2;}
inline bool equal(int v1, double  v2) { return (double)v1 == v2;}
bool equal(int v1, const StringData *v2);
inline bool equal(int v1, CStrRef v2) { return equal(v1, v2.get());}
inline bool equal(int v1, litstr  v2) { return equal(v1, String(v2));}
inline bool equal(int v1, CArrRef v2) { return false;}
inline bool equal(int v1, CObjRef v2) { return false;}
inline bool equal(int v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(int v1, bool    v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(int v1, int     v2) { return v1 == v2;}
inline bool equalAsStr(int v1, int64   v2) { return v1 == v2;}
inline bool equalAsStr(int v1, double  v2) { return v1 == v2;}
bool equalAsStr(int v1, const StringData *v2);
inline bool equalAsStr(int v1, CStrRef v2) { return equalAsStr(v1, v2.get());}
inline bool equalAsStr(int v1, litstr  v2) { return equalAsStr(v1, String(v2));}
inline bool equalAsStr(int v1, CArrRef v2) { return false;}
inline bool equalAsStr(int v1, CObjRef v2) {
  return equalAsStr(v1, v2.toString());
}
inline bool equalAsStr(int v1, CVarRef v2) { return equalAsStr(v2, v1);}

inline bool less(int v1, bool    v2)  { return more(v2, v1);}
inline bool less(int v1, int     v2)  { return v1 < v2;}
inline bool less(int v1, int64   v2)  { return v1 < v2;}
inline bool less(int v1, double  v2)  { return v1 < v2;}
bool less(int v1, const StringData *v2);
inline bool less(int v1, CStrRef v2) { return less(v1, v2.get());}
inline bool less(int v1, litstr  v2)  { return less(v1, String(v2));}
inline bool less(int v1, CArrRef v2)  { return true;}
inline bool less(int v1, CObjRef v2)  { return true;}
inline bool less(int v1, CVarRef v2)  { return more(v2, v1);}

inline bool more(int v1, bool    v2)  { return less(v2, v1);}
inline bool more(int v1, int     v2)  { return v1 > v2;}
inline bool more(int v1, int64   v2)  { return v1 > v2;}
inline bool more(int v1, double  v2)  { return v1 > v2;}
bool more(int v1, const StringData *v2);
inline bool more(int v1, CStrRef v2) { return more(v1, v2.get());}
inline bool more(int v1, litstr  v2)  { return more(v1, String(v2));}
inline bool more(int v1, CArrRef v2)  { return false;}
inline bool more(int v1, CObjRef v2)  { return false;}
inline bool more(int v1, CVarRef v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// int64

inline bool same(int64 v1, bool    v2)  { return same(v2, v1);}
inline bool same(int64 v1, int     v2)  { return same(v2, v1);}
inline bool same(int64 v1, int64   v2)  { return v1 == v2;}
inline bool same(int64 v1, double  v2)  { return (double)v1 == v2;}
inline bool same(int64 v1, const StringData *v2)  { return false;}
inline bool same(int64 v1, CStrRef v2)  { return false;}
inline bool same(int64 v1, litstr  v2)  { return false;}
inline bool same(int64 v1, CArrRef v2)  { return false;}
inline bool same(int64 v1, CObjRef v2)  { return false;}
inline bool same(int64 v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(int64 v1, bool    v2) { return equal(v2, v1);}
inline bool equal(int64 v1, int     v2) { return equal(v2, v1);}
inline bool equal(int64 v1, int64   v2) { return v1 == v2;}
inline bool equal(int64 v1, double  v2) { return (double)v1 == v2;}
bool equal(int64 v1, const StringData *v2);
inline bool equal(int64 v1, CStrRef v2) { return equal(v1, v2.get());}
inline bool equal(int64 v1, litstr  v2) { return equal(v1, String(v2));}
inline bool equal(int64 v1, CArrRef v2) { return false;}
inline bool equal(int64 v1, CObjRef v2) { return false;}
inline bool equal(int64 v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(int64 v1, bool    v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(int64 v1, int     v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(int64 v1, int64   v2) { return v1 == v2;}
inline bool equalAsStr(int64 v1, double  v2) {
  return v1 == v2 && v1 == (int64)v2;
}
bool equalAsStr(int64 v1, const StringData *v2);
inline bool equalAsStr(int64 v1, CStrRef v2) { return equalAsStr(v1, v2.get());}
bool equalAsStr(int64 v1, litstr  v2);
inline bool equalAsStr(int64 v1, CArrRef v2) { return false;}
inline bool equalAsStr(int64 v1, CObjRef v2) {
  return equalAsStr(v1, v2.toString());
}
inline bool equalAsStr(int64 v1, CVarRef v2) { return equalAsStr(v2, v1);}


inline bool less(int64 v1, bool    v2)  { return more(v2, v1);}
inline bool less(int64 v1, int     v2)  { return more(v2, v1);}
inline bool less(int64 v1, int64   v2)  { return v1 < v2;}
inline bool less(int64 v1, double  v2)  { return v1 < v2;}
bool less(int64 v1, const StringData *v2);
inline bool less(int64 v1, CStrRef v2) { return less(v1, v2.get());}
inline bool less(int64 v1, litstr  v2)  { return less(v1, String(v2));}
inline bool less(int64 v1, CArrRef v2)  { return true;}
inline bool less(int64 v1, CObjRef v2)  { return true;}
inline bool less(int64 v1, CVarRef v2)  { return more(v2, v1);}

inline bool more(int64 v1, bool    v2)  { return less(v2, v1);}
inline bool more(int64 v1, int     v2)  { return less(v2, v1);}
inline bool more(int64 v1, int64   v2)  { return v1 > v2;}
inline bool more(int64 v1, double  v2)  { return v1 > v2;}
bool more(int64 v1, const StringData *v2);
inline bool more(int64 v1, CStrRef v2) { return more(v1, v2.get());}
inline bool more(int64 v1, litstr  v2)  { return more(v1, String(v2));}
inline bool more(int64 v1, CArrRef v2)  { return false;}
inline bool more(int64 v1, CObjRef v2)  { return false;}
inline bool more(int64 v1, CVarRef v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// double

inline bool same(double v1, bool    v2)  { return same(v2, v1);}
inline bool same(double v1, int     v2)  { return same(v2, v1);}
inline bool same(double v1, int64   v2)  { return same(v2, v1);}
inline bool same(double v1, double  v2)  { return v1 == v2;}
inline bool same(double v1, const StringData *v2)  { return false;}
inline bool same(double v1, CStrRef v2)  { return false;}
inline bool same(double v1, litstr  v2)  { return false;}
inline bool same(double v1, CArrRef v2)  { return false;}
inline bool same(double v1, CObjRef v2)  { return false;}
inline bool same(double v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(double v1, bool    v2) { return equal(v2, v1);}
inline bool equal(double v1, int     v2) { return equal(v2, v1);}
inline bool equal(double v1, int64   v2) { return equal(v2, v1);}
inline bool equal(double v1, double  v2) { return v1 == v2;}
inline bool equal(double v1, const StringData *v2) {
  return v1 == toDouble(v2);
}
inline bool equal(double v1, CStrRef v2) { return v1 == v2.toDouble();}
inline bool equal(double v1, litstr  v2) { return equal(v1,String(v2));}
inline bool equal(double v1, CArrRef v2) { return false;}
inline bool equal(double v1, CObjRef v2) { return false;}
inline bool equal(double v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(double v1, bool    v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(double v1, int     v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(double v1, int64   v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(double v1, double  v2) { return v1 == v2;}
bool equalAsStr(double v1, const StringData *v2);
inline bool equalAsStr(double v1, CStrRef v2) {
  return equalAsStr(v1, v2.get());
}
bool equalAsStr(double v1, litstr  v2);
inline bool equalAsStr(double v1, CArrRef v2) { return false;}
inline bool equalAsStr(double v1, CObjRef v2) {
  return equalAsStr(v1, v2.toString());
}
inline bool equalAsStr(double v1, CVarRef v2) { return equalAsStr(v2, v1);}


inline bool less(double v1, bool    v2)  { return more(v2, v1);}
inline bool less(double v1, int     v2)  { return more(v2, v1);}
inline bool less(double v1, int64   v2)  { return more(v2, v1);}
inline bool less(double v1, double  v2)  { return v1 < v2;}
inline bool less(double v1, const StringData *v2)  {
  return less(v1,toDouble(v2));
}
inline bool less(double v1, CStrRef v2)  { return less(v1,v2.toDouble());}
inline bool less(double v1, litstr  v2)  { return less(v1,String(v2));}
inline bool less(double v1, CArrRef v2)  { return true;}
inline bool less(double v1, CObjRef v2)  { return true;}
inline bool less(double v1, CVarRef v2)  { return more(v2, v1);}

inline bool more(double v1, bool    v2)  { return less(v2, v1);}
inline bool more(double v1, int     v2)  { return less(v2, v1);}
inline bool more(double v1, int64   v2)  { return less(v2, v1);}
inline bool more(double v1, double  v2)  { return v1 > v2;}
inline bool more(double v1, const StringData *v2)  {
  return more(v1,toDouble(v2));
}
inline bool more(double v1, CStrRef v2)  { return more(v1,v2.toDouble());}
inline bool more(double v1, litstr  v2)  { return more(v1,String(v2));}
inline bool more(double v1, CArrRef v2)  { return false;}
inline bool more(double v1, CObjRef v2)  { return false;}
inline bool more(double v1, CVarRef v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// StringData *

inline bool same(const StringData *v1, bool    v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, int     v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, int64   v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, double  v2)  { return same(v2, v1);}
inline bool same(const StringData *v1, const StringData *v2) {
  if (v1 == v2) return true;
  if (v1 && v2) return v1->same(v2);
  return false;
}
inline bool same(const StringData *v1, CStrRef v2)  {
  return same(v1, v2.get());
}
inline bool same(const StringData *v1, litstr  v2)  {
  StringData sd2(v2);
  return same(v1, &sd2);
}
inline bool same(const StringData *v1, CArrRef v2)  { return false; }
inline bool same(const StringData *v1, CObjRef v2)  { return false; }
inline bool same(const StringData *v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(const StringData *v1, bool    v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, int     v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, int64   v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, double  v2) { return equal(v2, v1);}
inline bool equal(const StringData *v1, const StringData *v2) {
  if (v1 == v2) return true;
  if (v1 == NULL) return v2->empty();
  if (v2 == NULL) return v1->empty();
  return v1->compare(v2) == 0;
}
inline bool equal(const StringData *v1, CStrRef v2) {
  return equal(v1, v2.get());
}
inline bool equal(const StringData *v1, litstr  v2) {
  StringData sd2(v2);
  return equal(v1, &sd2);
}
inline bool equal(const StringData *v1, CArrRef v2) {
  if (v1 == NULL || v2.get() == NULL) {
    return equal(toBoolean(v1), v2.toBoolean());
  }
  return false;
}
inline bool equal(const StringData *v1, CObjRef v2) {
  if (v1 == NULL || v2.get() == NULL) {
    return equal(toBoolean(v1), v2.toBoolean());
  }
  if (v2.isResource()) return false;
  try {
    return equal(v1, v2.toString());
  } catch (BadTypeConversionException &e) {
    return false;
  }
  return false;
}
inline bool equal(const StringData *v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(const StringData *v1, bool    v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(const StringData *v1, int     v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(const StringData *v1, int64   v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(const StringData *v1, double  v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(const StringData *v1, const StringData *v2) {
  return same(v1, v2);
}
inline bool equalAsStr(const StringData *v1, CStrRef v2) {
  return equalAsStr(v1, v2.get());
}
inline bool equalAsStr(const StringData *v1, litstr  v2) {
  return same(v1, v2);
}
inline bool equalAsStr(const StringData *v1, CArrRef v2) {
  return equalAsStr(v1, "Array");
}
inline bool equalAsStr(const StringData *v1, CObjRef v2) {
  return equalAsStr(v1, v2.toString());
}
inline bool equalAsStr(const StringData *v1, CVarRef v2) {
  return equalAsStr(v2, v1);
}

inline bool less(const StringData *v1, bool    v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, int     v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, int64   v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, double  v2)  { return more(v2, v1);}
inline bool less(const StringData *v1, const StringData *v2) {
  if (v1 == v2 || v2 == NULL) return false;
  if (v1 == NULL) return !v2->empty();
  return v1->compare(v2) < 0;
}
inline bool less(const StringData *v1, CStrRef v2)  {
  return less(v1, v2.get());
}
inline bool less(const StringData *v1, litstr  v2)  {
  StringData sd2(v2);
  return less(v1, &sd2);
}
inline bool less(const StringData *v1, CArrRef v2)  {
  if (v1 == NULL || v2.get() == NULL) {
    return less(toBoolean(v1), v2.toBoolean());
  }
  return true;
}
inline bool less(const StringData *v1, CObjRef v2)  {
  if (v1 == NULL || v2.get() == NULL) {
    return less(toBoolean(v1), v2.toBoolean());
  }
  if (v2.isResource()) return true;
  try {
    return less(v1, v2.toString());
  } catch (BadTypeConversionException &e) {
    return true;
  }

}
inline bool less(const StringData *v1, CVarRef v2)  { return more(v2, v1);}

inline bool more(const StringData *v1, bool    v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, int     v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, int64   v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, double  v2)  { return less(v2, v1);}
inline bool more(const StringData *v1, const StringData *v2)  {
  if (v1 == NULL) return false;
  if (v2 == NULL) return !v1->empty();
  return v1->compare(v2) > 0;
}
inline bool more(const StringData *v1, CStrRef v2)  {
  return more(v1, v2.get());
}
inline bool more(const StringData *v1, litstr  v2)  {
  StringData sd2(v2);
  return more(v1, &sd2);
}
inline bool more(const StringData *v1, CArrRef v2)  {
  if (v1 == NULL || v2.get() == NULL) {
    return more(toBoolean(v1), v2.toBoolean());
  }
  return false;
}
inline bool more(const StringData *v1, CObjRef v2)  {
  if (v1 == NULL || v2.get() == NULL) {
    return more(toBoolean(v1), v2.toBoolean());
  }
  if (v2.isResource()) return false;
  try {
    return more(v1, v2.toString());
  } catch (BadTypeConversionException &e) {
    return false;
  }
}
inline bool more(const StringData *v1, CVarRef v2)  { return less(v2, v1);}


///////////////////////////////////////////////////////////////////////////////
// String

inline bool same(CStrRef v1, bool    v2)  { return same(v2, v1);}
inline bool same(CStrRef v1, int     v2)  { return same(v2, v1);}
inline bool same(CStrRef v1, int64   v2)  { return same(v2, v1);}
inline bool same(CStrRef v1, double  v2)  { return same(v2, v1);}
inline bool same(CStrRef v1, const StringData *v2)  {
  return same(v2, v1.get());
}
inline bool same(CStrRef v1, CStrRef v2)  { return v1.same(v2);}
inline bool same(CStrRef v1, litstr  v2)  { return v1.same(v2);}
inline bool same(CStrRef v1, CArrRef v2)  { return v1.same(v2);}
inline bool same(CStrRef v1, CObjRef v2)  { return v1.same(v2);}
inline bool same(CStrRef v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(CStrRef v1, bool    v2) { return equal(v2, v1);}
inline bool equal(CStrRef v1, int     v2) { return equal(v2, v1);}
inline bool equal(CStrRef v1, int64   v2) { return equal(v2, v1);}
inline bool equal(CStrRef v1, double  v2) { return equal(v2, v1);}
inline bool equal(CStrRef v1, const StringData *v2) {
  return equal(v2, v1.get());
}
inline bool equal(CStrRef v1, CStrRef v2) { return v1.equal(v2);}
inline bool equal(CStrRef v1, litstr  v2) { return v1.equal(v2);}
inline bool equal(CStrRef v1, CArrRef v2) { return v1.equal(v2);}
inline bool equal(CStrRef v1, CObjRef v2) { return v1.equal(v2);}
inline bool equal(CStrRef v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(CStrRef v1, bool    v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CStrRef v1, int     v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CStrRef v1, int64   v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CStrRef v1, double  v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CStrRef v1, const StringData *v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(CStrRef v1, CStrRef v2) { return same(v1, v2);}
inline bool equalAsStr(CStrRef v1, litstr  v2) {
  return equalAsStr(v1, String(v2));
}
inline bool equalAsStr(CStrRef v1, CArrRef v2) {
  return equalAsStr(v1.get(), v2);
}
inline bool equalAsStr(CStrRef v1, CObjRef v2) {
  return equalAsStr(v1, v2.toString());
}
inline bool equalAsStr(CStrRef v1, CVarRef v2) { return equalAsStr(v2, v1);}

inline bool less(CStrRef v1, bool    v2)  { return more(v2, v1);}
inline bool less(CStrRef v1, int     v2)  { return more(v2, v1);}
inline bool less(CStrRef v1, int64   v2)  { return more(v2, v1);}
inline bool less(CStrRef v1, double  v2)  { return more(v2, v1);}
inline bool less(CStrRef v1, const StringData *v2)  {
  return more(v2, v1.get());
}
inline bool less(CStrRef v1, CStrRef v2)  { return v1.less(v2);}
inline bool less(CStrRef v1, litstr  v2)  { return v1.less(v2);}
inline bool less(CStrRef v1, CArrRef v2)  { return v1.less(v2);}
inline bool less(CStrRef v1, CObjRef v2)  { return v1.less(v2);}
inline bool less(CStrRef v1, CVarRef v2)  { return more(v2, v1);}

inline bool more(CStrRef v1, bool    v2)  { return less(v2, v1);}
inline bool more(CStrRef v1, int     v2)  { return less(v2, v1);}
inline bool more(CStrRef v1, int64   v2)  { return less(v2, v1);}
inline bool more(CStrRef v1, double  v2)  { return less(v2, v1);}
inline bool more(CStrRef v1, const StringData *v2)  {
  return less(v2, v1.get());
}
inline bool more(CStrRef v1, CStrRef v2)  { return v1.more(v2);}
inline bool more(CStrRef v1, litstr  v2)  { return v1.more(v2);}
inline bool more(CStrRef v1, CArrRef v2)  { return v1.more(v2);}
inline bool more(CStrRef v1, CObjRef v2)  { return v1.more(v2);}
inline bool more(CStrRef v1, CVarRef v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
// litstr

inline bool same(litstr v1, bool    v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, int     v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, int64   v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, double  v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, const StringData *v2)  {
  StringData sd1(v1);
  return same(&sd1, v2);
}
inline bool same(litstr v1, CStrRef v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, litstr  v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, CArrRef v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, CObjRef v2)  { return same(String(v1), v2);}
inline bool same(litstr v1, CVarRef v2)  { return same(String(v1), v2);}

inline bool equal(litstr v1, bool    v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, int     v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, int64   v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, double  v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, const StringData *v2) {
  StringData sd1(v1);
  return equal(&sd1, v2);
}
inline bool equal(litstr v1, CStrRef v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, litstr  v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, CArrRef v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, CObjRef v2) { return equal(String(v1),v2);}
inline bool equal(litstr v1, CVarRef v2) { return equal(String(v1),v2);}

inline bool equalAsStr(litstr v1, bool    v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(litstr v1, int     v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(litstr v1, int64   v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(litstr v1, double  v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(litstr v1, const StringData *v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(litstr v1, CStrRef v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(litstr v1, litstr  v2) { return same(v1, v2);}
inline bool equalAsStr(litstr v1, CArrRef v2) {
  StringData sd1(v1);
  return equalAsStr(&sd1, v2);
}
inline bool equalAsStr(litstr v1, CObjRef v2) {
  return equalAsStr(v1, v2.toString());
}
inline bool equalAsStr(litstr v1, CVarRef v2) { return equalAsStr(v2, v1);}

inline bool less(litstr v1, bool    v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, int     v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, int64   v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, double  v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, const StringData *v2)  {
  StringData sd1(v1);
  return less(&sd1, v2);
}
inline bool less(litstr v1, CStrRef v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, litstr  v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, CArrRef v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, CObjRef v2)  { return less(String(v1), v2);}
inline bool less(litstr v1, CVarRef v2)  { return less(String(v1), v2);}

inline bool more(litstr v1, bool    v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, int     v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, int64   v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, double  v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, const StringData *v2)  {
  StringData sd1(v1);
  return more(&sd1, v2);
}
inline bool more(litstr v1, CStrRef v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, litstr  v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, CArrRef v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, CObjRef v2)  { return more(String(v1), v2);}
inline bool more(litstr v1, CVarRef v2)  { return more(String(v1), v2);}

///////////////////////////////////////////////////////////////////////////////
// Array

inline bool same(CArrRef v1, bool    v2)  { return same(v2, v1);}
inline bool same(CArrRef v1, int     v2)  { return same(v2, v1);}
inline bool same(CArrRef v1, int64   v2)  { return same(v2, v1);}
inline bool same(CArrRef v1, double  v2)  { return same(v2, v1);}
inline bool same(CArrRef v1, const StringData *v2)  { return same(v2, v1);}
inline bool same(CArrRef v1, CStrRef v2)  { return same(v2, v1);}
inline bool same(CArrRef v1, litstr  v2)  { return same(v2, v1);}
inline bool same(CArrRef v1, CArrRef v2)  { return v1.same(v2);}
inline bool same(CArrRef v1, CObjRef v2)  { return v1.same(v2);}
inline bool same(CArrRef v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(CArrRef v1, bool    v2) { return equal(v2, v1);}
inline bool equal(CArrRef v1, int     v2) { return equal(v2, v1);}
inline bool equal(CArrRef v1, int64   v2) { return equal(v2, v1);}
inline bool equal(CArrRef v1, double  v2) { return equal(v2, v1);}
inline bool equal(CArrRef v1, const StringData *v2) { return equal(v2, v1);}
inline bool equal(CArrRef v1, CStrRef v2) { return equal(v2, v1);}
inline bool equal(CArrRef v1, litstr  v2) { return equal(v2, v1);}
inline bool equal(CArrRef v1, CArrRef v2) { return v1.equal(v2);}
inline bool equal(CArrRef v1, CObjRef v2) { return v1.equal(v2);}
inline bool equal(CArrRef v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(CArrRef v1, bool    v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CArrRef v1, int     v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CArrRef v1, int64   v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CArrRef v1, double  v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CArrRef v1, const StringData *v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(CArrRef v1, CStrRef v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CArrRef v1, litstr  v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CArrRef v1, CArrRef v2) { return true;}
inline bool equalAsStr(CArrRef v1, CObjRef v2) {
  return equalAsStr("Array", v2.toString());
}
inline bool equalAsStr(CArrRef v1, CVarRef v2) { return equalAsStr(v2, v1);}

inline bool less(CArrRef v1, bool    v2)  { return more(v2, v1);}
inline bool less(CArrRef v1, int     v2)  { return more(v2, v1);}
inline bool less(CArrRef v1, int64   v2)  { return more(v2, v1);}
inline bool less(CArrRef v1, double  v2)  { return more(v2, v1);}
inline bool less(CArrRef v1, const StringData *v2)  { return more(v2, v1);}
inline bool less(CArrRef v1, CStrRef v2)  { return more(v2, v1);}
inline bool less(CArrRef v1, litstr  v2)  { return more(v2, v1);}
inline bool less(CArrRef v1, CArrRef v2)  { return v1.less(v2);}
inline bool less(CArrRef v1, CObjRef v2)  { return v1.less(v2);}
inline bool less(CArrRef v1, CVarRef v2)  { return v1.less(v2);}

inline bool more(CArrRef v1, bool    v2)  { return less(v2, v1);}
inline bool more(CArrRef v1, int     v2)  { return less(v2, v1);}
inline bool more(CArrRef v1, int64   v2)  { return less(v2, v1);}
inline bool more(CArrRef v1, double  v2)  { return less(v2, v1);}
inline bool more(CArrRef v1, const StringData *v2)  { return less(v2, v1);}
inline bool more(CArrRef v1, CStrRef v2)  { return less(v2, v1);}
inline bool more(CArrRef v1, litstr  v2)  { return less(v2, v1);}
inline bool more(CArrRef v1, CArrRef v2)  { return v1.more(v2);}
inline bool more(CArrRef v1, CObjRef v2)  { return v1.more(v2);}
inline bool more(CArrRef v1, CVarRef v2)  { return v1.more(v2);}

///////////////////////////////////////////////////////////////////////////////
// Object

inline bool same(CObjRef v1, bool    v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, int     v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, int64   v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, double  v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, const StringData *v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, CStrRef v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, litstr  v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, CArrRef v2)  { return same(v2, v1);}
inline bool same(CObjRef v1, CObjRef v2)  { return v1.same(v2);}
inline bool same(CObjRef v1, CVarRef v2)  { return same(v2, v1);}

inline bool equal(CObjRef v1, bool    v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, int     v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, int64   v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, double  v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, const StringData *v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, CStrRef v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, litstr  v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, CArrRef v2) { return equal(v2, v1);}
inline bool equal(CObjRef v1, CObjRef v2) { return v1.equal(v2);}
inline bool equal(CObjRef v1, CVarRef v2) { return equal(v2, v1);}

inline bool equalAsStr(CObjRef v1, bool    v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CObjRef v1, int     v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CObjRef v1, int64   v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CObjRef v1, double  v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CObjRef v1, const StringData *v2) {
  return equalAsStr(v2, v1);
}
inline bool equalAsStr(CObjRef v1, CStrRef v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CObjRef v1, litstr  v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CObjRef v1, CArrRef v2) { return equalAsStr(v2, v1);}
inline bool equalAsStr(CObjRef v1, CObjRef v2) {
  return equalAsStr(v1.toString(), v2.toString());
}
inline bool equalAsStr(CObjRef v1, CVarRef v2) { return equalAsStr(v2, v1);}

inline bool less(CObjRef v1, bool    v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, int     v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, int64   v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, double  v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, const StringData *v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, CStrRef v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, litstr  v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, CArrRef v2)  { return more(v2, v1);}
inline bool less(CObjRef v1, CObjRef v2)  { return v1.less(v2);}
inline bool less(CObjRef v1, CVarRef v2)  { return more(v2, v1);}

inline bool more(CObjRef v1, bool    v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, int     v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, int64   v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, double  v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, const StringData *v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, CStrRef v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, litstr  v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, CArrRef v2)  { return less(v2, v1);}
inline bool more(CObjRef v1, CObjRef v2)  { return v1.more(v2);}
inline bool more(CObjRef v1, CVarRef v2)  { return less(v2, v1);}

///////////////////////////////////////////////////////////////////////////////
/**
 * Special-casing comparisons between arrays to get the same results from
 * comparisons between uncomparable arrays.
 */
bool not_more(CVarRef v1, CVarRef v2);

bool not_less(CVarRef v1, CVarRef v2);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_COMPARISONS_H__
