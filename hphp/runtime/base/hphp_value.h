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
#ifndef incl_HPHP_HPHPVALUE_H_
#define incl_HPHP_HPHPVALUE_H_

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'hphp_value.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#include <type_traits>

#include "hphp/runtime/base/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Class;

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
  Class      *pcls; // only in vm stack, no type tag.
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

/*
 * These may be used to provide a little more self-documentation about
 * whether typed values must be cells (not KindOfRef) or ref (must be
 * KindOfRef).
 *
 * See bytecode.specification for details.  Note that in
 * bytecode.specification, refs are abbreviated as "V".
 *
 */
typedef TypedValue Cell;
typedef TypedValue Ref;

/*
 * A TypedNum is a TypedValue that is either KindOfDouble or
 * KindOfInt64.
 */
typedef TypedValue TypedNum;

//////////////////////////////////////////////////////////////////////

/*
 * make_value and make_tv are helpers for creating TypedValues and
 * Values as temporaries, without messing up the conversions.
 */

template<class T>
typename std::enable_if<
  std::is_integral<T>::value,
  Value
>::type make_value(T t) { Value v; v.num = t; return v; }

template<class T>
typename std::enable_if<
  std::is_pointer<T>::value,
  Value
>::type make_value(T t) {
  Value v;
  v.num = reinterpret_cast<int64_t>(t);
  return v;
}

inline Value make_value(double d) { Value v; v.dbl = d; return v; }

namespace detail {

  template<DataType> struct DataTypeCPPType;
#define X(dt, cpp) \
  template<> struct DataTypeCPPType<dt> { typedef cpp type; }

  X(KindOfNull,         void);
  X(KindOfBoolean,      bool);
  X(KindOfInt64,        int64_t);
  X(KindOfDouble,       double);
  X(KindOfArray,        ArrayData*);
  X(KindOfObject,       ObjectData*);
  X(KindOfRef,          RefData*);
  X(KindOfString,       StringData*);
  X(KindOfStaticString, StringData*);

#undef X

}

template<DataType DType>
typename std::enable_if<
  !std::is_same<typename detail::DataTypeCPPType<DType>::type,void>::value,
  TypedValue
>::type make_tv(typename detail::DataTypeCPPType<DType>::type val) {
  TypedValue ret;
  ret.m_data = make_value(val);
  ret.m_type = DType;
  return ret;
}

template<DataType DType>
typename std::enable_if<
  std::is_same<typename detail::DataTypeCPPType<DType>::type,void>::value,
  TypedValue
>::type make_tv() {
  TypedValue ret;
  ret.m_type = DType;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HPHPVALUE_H_
