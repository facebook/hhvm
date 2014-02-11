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

const StaticString s_WaitHandle("WaitHandle");

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
  case BNum:
  case BBool:
  case BStr:
  case BArr:
  case BInitPrim:
  case BPrim:
  case BInitUnc:
  case BUnc:
  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptInt:
  case BOptDbl:
  case BOptNum:
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
  case BNum:
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
  case BOptNum:
  case BOptSStr:
  case BOptCStr:
  case BOptStr:
  case BOptSArr:
  case BOptCArr:
  case BOptArr:
  case BOptObj:
  case BOptRes:
    return false;

  case BInitPrim:
  case BPrim:
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
    // For purposes of Type equivalence, NaNs are equal.
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
    if (m_data->dobj.type == o.m_data->dobj.type &&
        m_data->dobj.cls.same(o.m_data->dobj.cls)) {
      return true;
    }
    if (o.m_data->dobj.type == DObj::Sub) {
      return m_data->dobj.cls.subtypeOf(o.m_data->dobj.cls);
    }
    return false;
  case BCls:
    if (m_data->dcls.type == o.m_data->dcls.type &&
        m_data->dcls.cls.same(o.m_data->dcls.cls)) {
      return true;
    }
    if (o.m_data->dcls.type == DCls::Sub) {
      return m_data->dcls.cls.subtypeOf(o.m_data->dcls.cls);
    }
    return false;
  default:
    return equivData(o);
  }
}

bool Type::couldBeData(Type o) const {
  assert(m_data.hasValue());
  assert(o.m_data.hasValue());
  assert(m_bits == o.m_bits);

  switch (m_bits) {
  case BOptObj: case BObj:
    if (m_data->dobj.type == o.m_data->dobj.type &&
        m_data->dobj.cls.same(o.m_data->dobj.cls)) {
      return true;
    }
    if (m_data->dobj.type == DObj::Sub || o.m_data->dobj.type == DObj::Sub) {
      return m_data->dobj.cls.couldBe(o.m_data->dobj.cls);
    }
    return false;
  case BCls:
    if (m_data->dcls.type == o.m_data->dcls.type &&
        m_data->dcls.cls.same(o.m_data->dcls.cls)) {
      return true;
    }
    if (m_data->dcls.type == DCls::Sub || o.m_data->dcls.type == DCls::Sub) {
      return m_data->dcls.cls.couldBe(o.m_data->dcls.cls);
    }
    return false;
  default:
    return true;
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

  if (m_whType) {
    if (o.m_whType) {
      return
        wait_handle_inner(*this).subtypeOf(wait_handle_inner(o)) &&
        Type(m_bits, *m_data).subtypeOf(Type(o.m_bits, *o.m_data));
    }
    return Type(m_bits, *m_data).subtypeOf(o);
  }
  if (o.m_whType) {
    return subtypeOf(Type(o.m_bits, *o.m_data));
  }

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

  if (m_whType) {
    if (o.m_whType) {
      return wait_handle_inner(*this).couldBe(wait_handle_inner(o));
    }
    return o.couldBe(Type(m_bits, *m_data));
  }
  if (o.m_whType) {
    return couldBe(Type(o.m_bits, *o.m_data));
  }

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
  if (is_opt(*this)) return is_opt(o) ? true : unopt(*this).couldBe(o);
  if (is_opt(o))     return unopt(o).couldBe(*this);

  if (m_data && o.m_data) return couldBeData(Type(isect, *o.m_data));
  return true;
}

bool Type::checkInvariants() const {
  assert(isPredefined(m_bits));
  assert(!m_data || mayHaveData(m_bits));
  if (m_data) {
    if (m_bits == BSStr) assert(m_data->sval->isStatic());
    if (m_bits == BSArr) assert(m_data->aval->isStatic());
  }
  if (auto t = m_whType.get()) {
    t->checkInvariants();
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

Type wait_handle(const Index& index, Type inner) {
  auto const rwh = index.builtin_class(s_WaitHandle.get());
  auto t = subObj(rwh);
  t.m_whType = make_copy_ptr(std::move(inner));
  return t;
}

bool is_specialized_wait_handle(const Type& t) {
  return !!t.m_whType.get();
}

Type wait_handle_inner(const Type& t) {
  assert(t.m_whType.get());
  return *t.m_whType.get();
}

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
  d.dobj = DObj { val.couldBeOverriden() ?
                            DObj::Sub : DObj::Exact, val };
  return Type(BObj, d);
}

Type objExact(res::Class val) {
  Type::Data d;
  d.dobj = DObj { DObj::Exact, val };
  return Type(BObj, d);
}

Type subCls(res::Class val) {
  Type::Data d;
  d.dcls = DCls { val.couldBeOverriden() ?
                             DCls::Sub : DCls::Exact, val };
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

bool is_specialized_obj(Type t) {
  return t.strictSubtypeOf(TObj) ||
             (is_opt(t) && unopt(t).strictSubtypeOf(TObj));
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

DObj dobj_of(const Type& t) {
  assert(t.checkInvariants());
  assert(is_specialized_obj(t));
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

Type from_DataType(DataType dt) {
  switch (dt) {
  case KindOfUninit:   return TUninit;
  case KindOfNull:     return TInitNull;
  case KindOfBoolean:  return TBool;
  case KindOfInt64:    return TInt;
  case KindOfDouble:   return TDbl;
  case KindOfStaticString:
  case KindOfString:   return TStr;
  case KindOfArray:    return TArr;
  case KindOfRef:      return TRef;
  case KindOfObject:   return TObj;
  case KindOfResource: return TRes;
  default:
    break;
  }
  always_assert(0 && "dt in from_DataType didn't satisfy preconditions");
}

Type from_hni_constraint(SString s) {
  if (!s) return TGen;

  auto p   = s->data();
  auto ret = TBottom;

  if (*p == '?') {
    ret = union_of(ret, TInitNull);
    ++p;
  }

  if (!strcmp(p, "resource")) return union_of(ret, TRes);
  if (!strcmp(p, "bool"))     return union_of(ret, TBool);
  if (!strcmp(p, "int"))      return union_of(ret, TInt);
  if (!strcmp(p, "float"))    return union_of(ret, TDbl);
  if (!strcmp(p, "num"))      return union_of(ret, TNum);
  if (!strcmp(p, "string"))   return union_of(ret, TStr);
  if (!strcmp(p, "array"))    return union_of(ret, TArr);
  if (!strcmp(p, "mixed"))    return TInitGen;

  // It might be an object, or we might want to support type aliases
  // in HNI at some point.  For now just be conservative.
  return TGen;
}

Type union_of(Type a, Type b) {
  if (a.subtypeOf(b)) return b;
  if (b.subtypeOf(a)) return a;

  // We need to check the optional cases as part of this, because
  // otherwise we'll go down the is_specialized_obj paths and lose the
  // wait handle information.
  if (is_specialized_wait_handle(a)) {
    if (is_specialized_wait_handle(b)) {
      *a.m_whType = union_of(*a.m_whType, *b.m_whType);
      return a;
    }
    if (b == TInitNull) return opt(a);
  }
  if (is_specialized_wait_handle(b)) {
    if (a == TInitNull) return opt(b);
  }

  // When both types are strict subtypes of TObj or TOptObj or both
  // are strict subtypes of TCls we look for a common ancestor if one
  // exists.
  if (is_specialized_obj(a) && is_specialized_obj(b)) {
    auto keepOpt = is_opt(a) || is_opt(b);
    auto t = dobj_of(a).cls.commonAncestor(dobj_of(b).cls);
    // We need not to distinguish between Obj<=T and Obj=T, and always
    // return an Obj<=Ancestor, because that is the single type that
    // includes both children.
    if (t) return keepOpt ? opt(subObj(*t)) : subObj(*t);
    return keepOpt ? TOptObj : TObj;
  }
  if (a.strictSubtypeOf(TCls) && b.strictSubtypeOf(TCls)) {
    auto t = dcls_of(a).cls.commonAncestor(dcls_of(b).cls);
    // Similar to above, this must alway return an Obj<=Ancestor.
    return t ? subCls(*t) : TCls;
  }

#define X(y) if (a.subtypeOf(y) && b.subtypeOf(y)) return y;
  X(TInt)
  X(TDbl)
  X(TSStr)
  X(TCStr)
  X(TSArr)
  X(TCArr)
  X(TObj)
  X(TCls)
  X(TNull)
  X(TBool)
  X(TNum)
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
  X(TOptNum)
  X(TOptStr)
  X(TOptArr)

  X(TInitPrim)
  X(TPrim)
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

Type loosen_statics(Type a) {
  if (a.couldBe(TSStr)) a = union_of(a, TStr);
  if (a.couldBe(TSArr)) a = union_of(a, TArr);
  return a;
}

Type loosen_values(Type a) {
  return a.strictSubtypeOf(TInt) ? TInt :
         a.strictSubtypeOf(TDbl) ? TDbl :
         a.strictSubtypeOf(TBool) ? TBool :
         a.strictSubtypeOf(TSStr) ? TSStr :
         a.strictSubtypeOf(TSArr) ? TSArr :
         a == TOptFalse || a == TOptTrue ? TOptBool :
         a;
}

Type remove_uninit(Type t) {
  assert(t.subtypeOf(TCell));
  if (!t.couldBe(TUninit))  return t;
  if (t.subtypeOf(TUninit)) return TBottom;
  if (t.subtypeOf(TNull))   return TInitNull;
  if (t.subtypeOf(TPrim))   return TInitPrim;
  if (t.subtypeOf(TUnc))    return TInitUnc;
  return TInitCell;
}

//////////////////////////////////////////////////////////////////////

}}
