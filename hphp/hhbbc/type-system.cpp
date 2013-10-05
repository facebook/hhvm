/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/type-system.h"

#include <type_traits>
#include <cmath>

#include "folly/Traits.h"

#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

namespace {

bool mayHaveData(trep bits) {
  switch (bits) {
  case BSStr: case BSArr: case BObj: case BInt: case BDbl: case BCls:
  case BOptSStr: case BOptSArr: case BOptObj: case BOptInt: case BOptDbl:
    return true;
  default:
    break;
  }
  return false;
}

bool canBeOptional(trep bits) {
  switch (bits) {
  case BBottom:
    return false;

  case BUninit:
  case BInitNull:
    return false;
  case BFalse:
  case BTrue:
  case BInt:
  case BDbl:
  case BSStr:
  case BSArr:
  case BObj:
  case BRes:
    return true;

  case BCStr:
  case BCArr:
    return false;

  case BNull:
  case BBool:
  case BStr:
  case BArr:
    return true;

  case BCls:
  case BRef:
    return false;

  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptInt:
  case BOptDbl:
  case BOptSStr:
  case BOptStr:
  case BOptSArr:
  case BOptArr:
  case BOptObj:
  case BOptRes:
    return false;

  case BInitUnc:
  case BUnc:
  case BInitCell:
  case BInitGen:
  case BCell:
  case BGen:
  case BTop:
    return false;
  }
  not_reached();
}

}

//////////////////////////////////////////////////////////////////////

Type::Type(trep bits, Data d)
  : m_bits(bits)
  , m_data(d)
{
  assert(checkInvariants());
}

bool Type::equivData(Type o) const {
  assert(m_data.hasValue());
  assert(o.m_data.hasValue());
  assert(m_bits == o.m_bits);

  switch (m_bits) {
  case BOptSStr:
  case BSStr:
    return m_data->sval == o.m_data->sval;
  case BOptSArr:
  case BSArr:
    return m_data->aval == o.m_data->aval;
  case BOptInt:
  case BInt:
    return m_data->ival == o.m_data->ival;
  case BOptDbl:
  case BDbl:
    // For purposes of Type equivalence, NaNs are equal...
    return m_data->dval == o.m_data->dval ||
           (std::isnan(m_data->dval) && std::isnan(o.m_data->dval));
  case BOptObj:
  case BObj:
    return m_data->dobj.type == o.m_data->dobj.type &&
           m_data->dobj.cls.same(o.m_data->dobj.cls);
  case BCls:
    return m_data->dcls.type == o.m_data->dcls.type &&
           m_data->dcls.cls.same(o.m_data->dcls.cls);
  default:
    always_assert(0 && "invalid type with value");
  }
}

bool Type::subtypeData(Type o) const {
  assert(m_data.hasValue());
  assert(o.m_data.hasValue());
  assert(m_bits == o.m_bits);

  switch (m_bits) {
  case BOptObj: case BObj:
    if (m_data->dobj.type == o.m_data->dobj.type) {
      return m_data->dobj.cls.same(o.m_data->dobj.cls);
    }
    if (m_data->dobj.type == DObj::Exact) {
      return m_data->dobj.cls.subtypeOf(o.m_data->dobj.cls);
    }
    return false;
  case BCls:
    if (m_data->dcls.type == o.m_data->dcls.type) {
      return m_data->dcls.cls.same(o.m_data->dcls.cls);
    }
    if (m_data->dcls.type == DCls::Exact) {
      return m_data->dcls.cls.subtypeOf(o.m_data->dcls.cls);
    }
    return false;
  default:
    return equivData(o);
  }
}

bool Type::operator==(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());
  if (m_bits != o.m_bits) return false;
  if (m_data.hasValue() != o.m_data.hasValue()) return false;
  if (!m_data && !o.m_data) return true;
  return equivData(o);
}

bool Type::subtypeOf(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

  auto const isect = static_cast<trep>(m_bits & o.m_bits);
  if (isect != m_bits) return false;
  if (!mayHaveData(isect)) return true;

  // No data is always more general.
  if (!m_data && !o.m_data) return true;
  if (!o.m_data) {
    assert(m_data.hasValue());
    return true;
  }

  // Both have data, and the intersection allows it, so it depends on
  // what the data says.
  return m_data.hasValue() && subtypeData(Type(isect, *o.m_data));
}

bool Type::strictSubtypeOf(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());
  return *this != o && subtypeOf(o);
}

bool Type::couldBe(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

  auto const isect = static_cast<trep>(m_bits & o.m_bits);
  if (isect == 0) return false;
  if (subtypeOf(o) || o.subtypeOf(*this)) return true;
  if (!mayHaveData(isect)) return true;

  /*
   * We have an intersection that may have data, and we already know
   * that neither type contains the other.  For most of our types,
   * where m_data represents an exact constant value, this just means
   * we only can intersect if there is no data.  For objects or
   * classes, we have to assume anything could be another
   * object/class, because we currently don't look at the inheritance
   * chain.
   *
   * TODO: use res::Class::couldBe
   */
  if (Type(isect).subtypeOf(TObj)) return true;
  if (Type(isect).subtypeOf(TCls)) return true;

  return !m_data && !o.m_data;
}

bool Type::checkInvariants() const {
  assert(!m_data || mayHaveData(m_bits));
  if (m_data) {
    if (m_bits == BSStr) assert(m_data->sval->isStatic());
    if (m_bits == BSArr) assert(m_data->aval->isStatic());
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

Type sval(SString val) {
  assert(val->isStatic());
  Type::Data d;
  d.sval = val;
  return Type(BSStr, d);
}

Type ival(int64_t val) {
  Type::Data d;
  d.ival = val;
  return Type(BInt, d);
}

Type dval(double val) {
  Type::Data d;
  d.dval = val;
  return Type(BDbl, d);
}

Type aval(SArray val) {
  Type::Data d;
  d.aval = val;
  return Type(BSArr, d);
}

Type subObj(res::Class val) {
  Type::Data d;
  d.dobj = DObj { DObj::Sub, val };
  return Type(BObj, d);
}

Type objExact(res::Class val) {
  Type::Data d;
  d.dobj = DObj { DObj::Exact, val };
  return Type(BObj, d);
}

Type subCls(res::Class val) {
  Type::Data d;
  d.dcls = DCls { DCls::Sub, val };
  return Type(BCls, d);
}

Type clsExact(res::Class val) {
  Type::Data d;
  d.dcls = DCls { DCls::Exact, val };
  return Type(BCls, d);
}

Type opt(Type t) {
  assert(canBeOptional(t.m_bits));
  auto ret = t;
  ret.m_bits = static_cast<trep>(ret.m_bits | BInitNull);
  return ret;
}

Type objcls(Type t) {
  assert(t.subtypeOf(TObj));
  if (t.strictSubtypeOf(TObj)) {
    auto const d = dobj_of(t);
    return d.type == DObj::Exact ? clsExact(d.cls) : subCls(d.cls);
  }
  return TCls;
}

//////////////////////////////////////////////////////////////////////

folly::Optional<Cell> tv(Type t) {
  assert(t.checkInvariants());

  switch (t.m_bits) {
  case BUninit:      return make_tv<KindOfUninit>();
  case BInitNull:    return make_tv<KindOfNull>();
  case BTrue:        return make_tv<KindOfBoolean>(true);
  case BFalse:       return make_tv<KindOfBoolean>(false);
  default:
    if (!t.m_data)   break;
    switch (t.m_bits) {
    case BInt:   return make_tv<KindOfInt64>(t.m_data->ival);
    case BDbl:   return make_tv<KindOfDouble>(t.m_data->dval);
    case BSStr:  return make_tv<KindOfStaticString>(t.m_data->sval);
    case BSArr:  return make_tv<KindOfArray>(
                   const_cast<ArrayData*>(t.m_data->aval)
                 );
    case BObj:
    case BOptObj:
      break;
    default:
      assert(0 && "invalid type with value");
    }
  }

  return folly::none;
}

DObj dobj_of(Type t) {
  assert(t.checkInvariants());
  assert(t.strictSubtypeOf(TObj));
  assert(t.m_data);
  return t.m_data->dobj;
}

DCls dcls_of(Type t) {
  assert(t.checkInvariants());
  assert(t.strictSubtypeOf(TCls));
  assert(t.m_data);
  return t.m_data->dcls;
}

Type from_cell(Cell cell) {
  switch (cell.m_type) {
  case KindOfUninit:   return TUninit;
  case KindOfNull:     return TInitNull;
  case KindOfBoolean:  return cell.m_data.num ? TTrue : TFalse;
  case KindOfInt64:    return ival(cell.m_data.num);
  case KindOfDouble:   return dval(cell.m_data.dbl);

  case KindOfStaticString:
  case KindOfString:
    always_assert(cell.m_data.pstr->isStatic());
    return sval(cell.m_data.pstr);

  case KindOfArray:
    always_assert(cell.m_data.parr->isStatic());
    return aval(cell.m_data.parr);

  case KindOfRef:
  case KindOfObject:
  case KindOfResource:
  default:
    break;
  }
  always_assert(0 && "reference counted type in from_cell");
}

Type union_of(Type a, Type b) {
  if (a.subtypeOf(b)) return b;
  if (b.subtypeOf(a)) return a;

#define X(y) if (a.subtypeOf(y) && b.subtypeOf(y)) return y;
  X(TInt)
  X(TDbl)
  X(TSStr)
  X(TCStr)
  X(TSArr)
  X(TCArr)
  // TODO_3: TObj/TCls unions can be smarter if they both have data,
  // and if res::Class provides enough info (least common ancestor,
  // etc).
  X(TObj)
  X(TCls)
  X(TNull)
  X(TBool)
  X(TStr)
  X(TArr)
  X(TOptTrue)
  X(TOptFalse)
  X(TOptBool)
  X(TOptInt)
  X(TOptDbl)
  X(TOptSStr)
  X(TOptStr)
  X(TOptSArr)
  X(TOptArr)
  X(TOptObj)
  X(TOptRes)
  X(TInitUnc)
  X(TUnc)
  X(TInitCell)
  X(TCell)
  X(TInitGen)
  X(TGen)
#undef X

  return TTop;
}

Type stack_flav(Type a) {
  if (a.subtypeOf(TUninit))   return TUninit;
  if (a.subtypeOf(TInitCell)) return TInitCell;
  if (a.subtypeOf(TRef))      return TRef;
  if (a.subtypeOf(TGen))      return TGen;
  if (a.subtypeOf(TCls))      return TCls;
  always_assert(0 && "stack_flav passed invalid type");
}

//////////////////////////////////////////////////////////////////////

}}
