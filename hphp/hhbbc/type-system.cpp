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

// Legal to call with !isPredefined(bits)
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

/*
 * Note: currently we're limiting all represented types to predefined
 * bit patterns (instead of arbitrary unions), so this function is
 * around for assertions.
 *
 * This may be relaxed later if we think we can get more out of it,
 * but for now the thought is that the likelihood of getting
 * significantly better types from the inference algorithm might be
 * counter-balanced by the increased chance of hard-to-track
 * type-system bugs, so at least for now (at the time of this writing,
 * before shipping hhbbc) we're ruling out a bunch of edge cases.
 *
 * Aside from types like Obj<= or things like TSStr, a lot of cases
 * with arbitrary-unions may not lead to better code at JIT time
 * (unless the inference turns them back into a predefined type),
 * since we'll have to guard anyway to see which one it was.  We'll
 * try it later.
 */
bool isPredefined(trep bits) {
  switch (bits) {
  case BBottom:
  case BUninit:
  case BInitNull:
  case BFalse:
  case BTrue:
  case BInt:
  case BDbl:
  case BSStr:
  case BCStr:
  case BSArr:
  case BCArr:
  case BObj:
  case BRes:
  case BCls:
  case BRef:
  case BNull:
  case BBool:
  case BStr:
  case BArr:
  case BInitUnc:
  case BUnc:
  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptInt:
  case BOptDbl:
  case BOptSStr:
  case BOptCStr:
  case BOptStr:
  case BOptSArr:
  case BOptCArr:
  case BOptArr:
  case BOptObj:
  case BOptRes:
  case BInitCell:
  case BCell:
  case BInitGen:
  case BGen:
  case BTop:
    return true;
  }
  return false;
}

// Pre: isPredefined(bits)
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
  case BCStr:
  case BSArr:
  case BCArr:
  case BObj:
  case BRes:
    return true;

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
  case BOptCStr:
  case BOptStr:
  case BOptSArr:
  case BOptCArr:
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
   * From here we have an intersection that may have data, and we know
   * that neither type completely contains the other.
   *
   * For most of our types, where m_data represents an exact constant
   * value, this just means the types only overlap if there is no
   * data.
   *
   * The exception to that are option types with data and
   * objects/classes.
   */

  /*
   * If the intersection allowed data, and either type was an option
   * type, we can simplify the case to whether the unopt'd version of
   * the option type couldBe the other type.  (The case where
   * TInitNull was the overlapping part would already be handled
   * above, because !mayHaveData(TInitNull).)
   */
  if (is_opt(*this)) return unopt(*this).couldBe(o);
  if (is_opt(o))     return unopt(o).couldBe(*this);

  /*
   * For objects or classes, we have to assume any object or class
   * could be another object/class, because we currently don't look at
   * the inheritance chain.  (We already handled the trivial cases of
   * Obj=Foo <: Obj<=Foo in the subtype check above.)
   *
   * TODO(#3343798): use res::Class::couldBe
   */
  if (isPredefined(isect)) {
    if (Type(isect).subtypeOf(TObj)) return true;
    if (Type(isect).subtypeOf(TCls)) return true;
  }

  return !m_data && !o.m_data;
}

bool Type::checkInvariants() const {
  assert(isPredefined(m_bits));
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

Type unopt(Type t) {
  assert(is_opt(t));
  t.m_bits = static_cast<trep>(t.m_bits & ~BInitNull);
  assert(!is_opt(t));
  return t;
}

bool is_opt(Type t) {
  if (t.m_bits == BInitNull) return false;
  if (!t.couldBe(TInitNull)) return false;
  auto const nonNullBits = static_cast<trep>(t.m_bits & ~BInitNull);
  return isPredefined(nonNullBits) && canBeOptional(nonNullBits);
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
    if (is_opt(t))   break;
    switch (t.m_bits) {
    case BInt:   return make_tv<KindOfInt64>(t.m_data->ival);
    case BDbl:   return make_tv<KindOfDouble>(t.m_data->dval);
    case BSStr:  return make_tv<KindOfStaticString>(t.m_data->sval);
    case BSArr:  return make_tv<KindOfArray>(
                   const_cast<ArrayData*>(t.m_data->aval)
                 );
    case BObj:
      break;
    default:
      assert(0 && "invalid type with value");
    }
  }

  return folly::none;
}

Type type_of_istype(IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Null:   return TNull;
  case IsTypeOp::Bool:   return TBool;
  case IsTypeOp::Int:    return TInt;
  case IsTypeOp::Dbl:    return TDbl;
  case IsTypeOp::Str:    return TStr;
  case IsTypeOp::Arr:    return TArr;
  case IsTypeOp::Obj:    return TObj;
  case IsTypeOp::Scalar: always_assert(0);
  }
  not_reached();
}

DObj dobj_of(Type t) {
  assert(t.checkInvariants());
  assert(t.strictSubtypeOf(TObj) ||
         (t.subtypeOf(TOptObj) && unopt(t).strictSubtypeOf(TOptObj)));
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
  // TODO(#3343798): TObj/TCls unions can be smarter if they both have
  // data, and if res::Class provides enough info (least common
  // ancestor, etc).
  X(TObj)
  X(TCls)
  X(TNull)
  X(TBool)
  X(TStr)
  X(TArr)

  /*
   * Merging option types tries to preserve subtype information where
   * it's possible.  E.g. if you union InitNull and Obj<=Foo, we want
   * OptObj<=Foo to be the result.
   */
  if (a == TInitNull && canBeOptional(b.m_bits)) return opt(b);
  if (b == TInitNull && canBeOptional(a.m_bits)) return opt(a);

  // A few optional unions still need to be checked despite the above
  // (e.g. if we are merging TOptTrue and TOptFalse, we want TOptBool,
  // but neither was TInitNull).
  X(TOptBool)
  X(TOptStr)
  X(TOptArr)

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
