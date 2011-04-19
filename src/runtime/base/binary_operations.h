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

#ifndef __HPHP_BINARY_OPERATIONS_H__
#define __HPHP_BINARY_OPERATIONS_H__

#include <runtime/base/complex_types.h>

/**
 * This file contains binary operations that are frequently used and they
 * are overloaded to ease the code generation so that we need fewer type
 * casts.
 */
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Variant

inline Variant operator+(bool    n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(char    n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(short   n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(int     n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(int64   n, CVarRef v) { return Variant(v) += n;}
inline Variant operator+(double  n, CVarRef v) { return Variant(v) += n;}

// String + Variant means string concatenation
inline Variant operator+(CStrRef n, CVarRef v) { return String(n) += v;}
inline Variant operator+(litstr  n, CVarRef v) { return Variant(v) += n;}

// Array + Variant is already defined in type_array.h
inline Variant operator+(CObjRef n, CVarRef v) { return Variant(v) += n;}

inline Variant operator+(CVarRef v, bool    n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, char    n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, short   n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, int     n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, int64   n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, double  n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, CStrRef n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, litstr  n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, CArrRef n) { return Variant(v) += n;}
inline Variant operator+(CVarRef v, CObjRef n) { return Variant(v) += n;}

// String + String is already defined in type_string.h
// Array + Array is already defined in type_array.h
inline Variant operator+(CObjRef v, CObjRef n) { return Variant(v) += n;}

inline Variant operator-(bool    n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(char    n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(short   n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(int     n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(int64   n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(double  n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(CStrRef n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(litstr  n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(CArrRef n, CVarRef v) { return v.negate() += n;}
inline Variant operator-(CObjRef n, CVarRef v) { return v.negate() += n;}

inline Variant operator-(CVarRef v, bool    n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, char    n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, short   n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, int     n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, int64   n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, double  n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, CStrRef n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, litstr  n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, CArrRef n) { return Variant(v) -= n;}
inline Variant operator-(CVarRef v, CObjRef n) { return Variant(v) -= n;}

inline Variant operator*(bool    n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(char    n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(short   n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(int     n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(int64   n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(double  n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(CStrRef n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(litstr  n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(CArrRef n, CVarRef v) { return Variant(v) *= n;}
inline Variant operator*(CObjRef n, CVarRef v) { return Variant(v) *= n;}

inline Variant operator*(CVarRef v, bool    n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, char    n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, short   n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, int     n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, int64   n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, double  n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, CStrRef n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, litstr  n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, CArrRef n) { return Variant(v) *= n;}
inline Variant operator*(CVarRef v, CObjRef n) { return Variant(v) *= n;}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_BINARY_OPERATIONS_H__
