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
  int64_t     num;  // KindOfInt64, KindOfBool (must be zero-extended)
  double      dbl;  // KindOfDouble
  StringData *pstr; // KindOfString, KindOfStaticString
  ArrayData  *parr; // KindOfArray
  ObjectData *pobj; // KindOfObject
  VM::Class  *pcls; // only in vm stack, no type tag.
  RefData    *pref; // KindOfRef
  TypedValue *pind; // only for KindOfIndirect
};

enum VarNrFlag { NR_FLAG = 1<<29 };

union AuxUnion {
  int32_t u_hash;        // key type and hash for HphpArray and [Stable]Map
  VarNrFlag u_varNrFlag; // magic number for asserts in VarNR
  bool u_deepInit;       // used by Class::initPropsImpl for deep init
  int32_t u_cacheHandle; // used by unit.cpp to squirrel away cache handles
};

/*
 * 7pack format:
 * experimental "Packed" format for TypedValues.  By grouping 7 tags
 * and 7 values separately, we can fit 7 TypedValues in 63 bytes (64 with
 * a throw-away alignment byte (t0):
 *
 *   0   1   2     7   8       16        56
 *   [t0][t1][t2]..[t7][value1][value2]..[value7]
 *
 * With this layout, a single TypedValue requires 16 bytes, and still has
 * room for a 32-bit padding field, which we still use in a few places:
 *
 *   0   1       2   3   4      8
 *   [t0][m_type][t2][t3][m_pad][m_data]
 */

/*
 * A TypedValue is a descriminated PHP Value.  m_tag describes the contents
 * of m_data.  m_aux is described above, and must only be read or written
 * in specialized contexts.
 */
#ifdef PACKED_TV
// This TypedValue layout is a subset of the full 7pack format.  Client
// code should not mess with the _t0 or _tags padding fields.
struct TypedValue {
  union {
    uint8_t _tags[8];
    struct {
      uint8_t _t0;
      DataType m_type;
      AuxUnion m_aux;
    };
  };
  Value m_data;

  std::string pretty() const;
};
#else
struct TypedValue {
  Value m_data;
  AuxUnion m_aux;
  DataType m_type;

  std::string pretty() const; // debug formatting. see trace.h
};
#endif

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

private:
  static void assertions() {
    static_assert(sizeof(TypedValueAux) <= 16,
                  "don't add big things to AuxUnion");
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HPHPVALUE_H__
