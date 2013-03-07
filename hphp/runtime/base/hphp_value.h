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

namespace VM {
  class Class;
}

struct TypedValue;

/*
 * This is the payload of a PHP value.  This union may only be used in
 * contexts that have a discriminator, e.g. in TypedValue (below), or
 * when the type is known beforehand.
 */
union Value {
  int64_t     num;  // KindOfInt64, KindOfBool
  double      dbl;  // KindOfDouble
  StringData *pstr; // KindOfString, KindOfStaticString
  ArrayData  *parr; // KindOfArray
  ObjectData *pobj; // KindOfObject
  VM::Class  *pcls; // only in vm stack, no type tag.
  RefData    *pref; // KindOfRef
  TypedValue *pind; // only for KindOfIndirect
};

/*
 * This is an auxilary field used to store auxilary data inside TypedValues,
 * in certian contexts.  With this union we can keep track of uses; all of
 * them will be phased out at some point.
 * TODO: t1100154 phase them out so we can get more compact TypedValues.
 */
enum VarNRFlag { NR_FLAG = 1<<29 };
union TypedValueAux {
  int32_t u_hash;        // key type and hash for HphpArray and [Stable]Map
  VarNRFlag u_varNrFlag; // magic number for asserts in VarNR
  bool u_deepInit;       // used by Class::initPropsImpl for deep init
  int32_t u_cacheHandle; // used by unit.cpp to squirrel away cache handles
};

/*
 * A TypedValue is a descriminated PHP Value.  m_tag describes the contents
 * of m_data.  m_aux is described above, and must only be read or written
 * in specialized contexts.
 */
struct TypedValue {
  /**
   * The order of the data members is significant. The m_aux field must
   * be exactly FAST_REFCOUNT_OFFSET bytes from the beginning of the object.
   */
  Value m_data;
  TypedValueAux m_aux;
  DataType m_type;

  std::string pretty() const; // debug formatting. see trace.h
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HPHPVALUE_H__
