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

enum VarNrFlag { NR_FLAG = 1<<29 };

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
private:
  friend struct TypedValueAux;
  union {
    int32_t u_hash;        // key type and hash for HphpArray and [Stable]Map
    VarNrFlag u_varNrFlag; // magic number for asserts in VarNR
    bool u_deepInit;       // used by Class::initPropsImpl for deep init
    int32_t u_cacheHandle; // used by unit.cpp to squirrel away cache handles
  } m_aux;
public:
  DataType m_type;

  std::string pretty() const; // debug formatting. see trace.h
};

/*
 * This TypedValue subclass exposes a 32-bit "aux" field somewhere inside it.
 * For now, access the m_aux field declared in TypedValue, but once we
 * rearrange TypedValue, the aux field can move down to this struct.
 * TODO: t1100154 phase this out completely.
 */
struct TypedValueAux : TypedValue {
  static const size_t auxOffset = offsetof(TypedValue, m_aux);
  static const size_t auxSize = sizeof(m_aux);
  int32_t& hash() { return m_aux.u_hash; }
  const int32_t& hash() const { return m_aux.u_hash; }
  int32_t& cacheHandle() { return m_aux.u_cacheHandle; }
  const int32_t& cacheHandle() const { return m_aux.u_cacheHandle; }
  bool& deepInit() { return m_aux.u_deepInit; }
  const bool& deepInit() const { return m_aux.u_deepInit; }
  VarNrFlag& varNrFlag() { return m_aux.u_varNrFlag; }
  const VarNrFlag& varNrFlag() const { return m_aux.u_varNrFlag; }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HPHPVALUE_H__
