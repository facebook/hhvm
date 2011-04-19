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

#ifndef __INSIDE_HPHP_COMPLEX_TYPES_H__
#error Directly including 'hphp_value.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#ifndef __HPHP_HPHPVALUE_H__
#define __HPHP_HPHPVALUE_H__

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;

struct Value {
  mutable union {
    int64        num;
    double       dbl;
    litstr       str;
    StringData  *pstr;
    ArrayData   *parr;
    ObjectData  *pobj;
    Variant     *pvar;
    TypedValue  *ptv;
  } m_data;
};

struct TypedValue : public Value {
  /**
   * The order of the data members is significant. The _count field must
   * be exactly FAST_REFCOUNT_OFFSET bytes from the beginning of the object.
   */
  mutable int _count;
  mutable DataType m_type;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HPHPVALUE_H__
