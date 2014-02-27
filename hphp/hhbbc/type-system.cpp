/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <algorithm>

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

Type::Type(const Type& o) noexcept
  : m_bits(o.m_bits)
  , m_dataTag(o.m_dataTag)
{
  switch (m_dataTag) {
  case DataTag::None: return;
  case DataTag::Str: m_data.sval = o.m_data.sval; return;
  case DataTag::Arr: m_data.aval = o.m_data.aval; return;
  case DataTag::Int: m_data.ival = o.m_data.ival; return;
  case DataTag::Dbl: m_data.dval = o.m_data.dval; return;
  case DataTag::Obj: new (&m_data.dobj) DObj(o.m_data.dobj); return;
  case DataTag::Cls: new (&m_data.dcls) DCls(o.m_data.dcls); return;
  }
  not_reached();
}

Type::Type(Type&& o) noexcept
  : m_bits(o.m_bits)
  , m_dataTag(o.m_dataTag)
{
  o.m_dataTag = DataTag::None;
  switch (m_dataTag) {
  case DataTag::None: return;
  case DataTag::Str: m_data.sval = o.m_data.sval; return;
  case DataTag::Arr: m_data.aval = o.m_data.aval; return;
  case DataTag::Int: m_data.ival = o.m_data.ival; return;
  case DataTag::Dbl: m_data.dval = o.m_data.dval; return;
  case DataTag::Obj: new (&m_data.dobj) DObj(std::move(o.m_data.dobj)); return;
  case DataTag::Cls: new (&m_data.dcls) DCls(std::move(o.m_data.dcls)); return;
  }
  not_reached();
}

Type& Type::operator=(const Type& o) noexcept {
  this->~Type();
  new (this) Type(o);
  return *this;
}

Type& Type::operator=(Type&& o) noexcept {
  this->~Type();
  new (this) Type(std::move(o));
  return *this;
}

Type::~Type() noexcept {
  assert(checkInvariants());

  switch (m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Arr:
  case DataTag::Int:
  case DataTag::Dbl:
    return;
  case DataTag::Obj:
    m_data.dobj.~DObj();
    return;
  case DataTag::Cls:
    m_data.dcls.~DCls();
    return;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

bool Type::hasData() const {
  return m_dataTag != DataTag::None;
}

bool Type::equivData(const Type& o) const {
  assert(m_dataTag == o.m_dataTag);
  switch (m_dataTag) {
  case DataTag::None:
    break;
  case DataTag::Str:
    return m_data.sval == o.m_data.sval;
  case DataTag::Arr:
    return m_data.aval == o.m_data.aval;
  case DataTag::Int:
    return m_data.ival == o.m_data.ival;
  case DataTag::Dbl:
    // For purposes of Type equivalence, NaNs are equal.
    return m_data.dval == o.m_data.dval ||
           (std::isnan(m_data.dval) && std::isnan(o.m_data.dval));
  case DataTag::Obj:
    return m_data.dobj.type == o.m_data.dobj.type &&
           m_data.dobj.cls.same(o.m_data.dobj.cls);
  case DataTag::Cls:
    return m_data.dcls.type == o.m_data.dcls.type &&
           m_data.dcls.cls.same(o.m_data.dcls.cls);
  }
  not_reached();
}

bool Type::subtypeData(const Type& o) const {
  assert(m_dataTag == o.m_dataTag);
  switch (m_dataTag) {
  case DataTag::Obj:
    if (m_data.dobj.type == o.m_data.dobj.type &&
        m_data.dobj.cls.same(o.m_data.dobj.cls)) {
      return true;
    }
    if (o.m_data.dobj.type == DObj::Sub) {
      return m_data.dobj.cls.subtypeOf(o.m_data.dobj.cls);
    }
    return false;
  case DataTag::Cls:
    if (m_data.dcls.type == o.m_data.dcls.type &&
        m_data.dcls.cls.same(o.m_data.dcls.cls)) {
      return true;
    }
    if (o.m_data.dcls.type == DCls::Sub) {
      return m_data.dcls.cls.subtypeOf(o.m_data.dcls.cls);
    }
    return false;
  case DataTag::Str:
  case DataTag::Arr:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::None:
    return equivData(o);
  }
  not_reached();
}

bool Type::couldBeData(const Type& o) const {
  assert(m_dataTag == o.m_dataTag);
  switch (m_bits) {
  case BOptObj: case BObj:
    if (m_data.dobj.type == o.m_data.dobj.type &&
        m_data.dobj.cls.same(o.m_data.dobj.cls)) {
      return true;
    }
    if (m_data.dobj.type == DObj::Sub || o.m_data.dobj.type == DObj::Sub) {
      return m_data.dobj.cls.couldBe(o.m_data.dobj.cls);
    }
    return false;
  case BCls:
    if (m_data.dcls.type == o.m_data.dcls.type &&
        m_data.dcls.cls.same(o.m_data.dcls.cls)) {
      return true;
    }
    if (m_data.dcls.type == DCls::Sub || o.m_data.dcls.type == DCls::Sub) {
      return m_data.dcls.cls.couldBe(o.m_data.dcls.cls);
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
  if (hasData() != o.hasData()) return false;
  if (!hasData() && !o.hasData()) return true;
  return equivData(o);
}

bool Type::subtypeOf(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

  if (is_specialized_wait_handle(*this)) {
    if (is_specialized_wait_handle(o)) {
      return
        wait_handle_inner(*this).subtypeOf(wait_handle_inner(o)) &&
        wait_handle_outer(*this).subtypeOf(wait_handle_outer(o));
    }
    return wait_handle_outer(*this).subtypeOf(o);
  }
  if (is_specialized_wait_handle(o)) {
    return subtypeOf(wait_handle_outer(o));
  }

  auto const isect = static_cast<trep>(m_bits & o.m_bits);
  if (isect != m_bits) return false;
  if (!mayHaveData(isect)) return true;

  // No data is always more general.
  if (!hasData() && !o.hasData()) return true;
  if (!o.hasData()) {
    assert(hasData());
    return true;
  }

  // Both have data, and the intersection allows it, so it depends on
  // what the data says.
  return hasData() && subtypeData(o);
}

bool Type::strictSubtypeOf(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());
  return *this != o && subtypeOf(o);
}

bool Type::couldBe(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

  if (is_specialized_wait_handle(*this)) {
    if (is_specialized_wait_handle(o)) {
      return wait_handle_inner(*this).couldBe(wait_handle_inner(o));
    }
    return o.couldBe(wait_handle_outer(*this));
  }
  if (is_specialized_wait_handle(o)) {
    return couldBe(wait_handle_outer(o));
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

  if (hasData() && o.hasData()) {
    assert(mayHaveData(isect));
    return couldBeData(o);
  }
  return true;
}

bool Type::checkInvariants() const {
  assert(isPredefined(m_bits));
  assert(!hasData() || mayHaveData(m_bits));
  switch (m_dataTag) {
  case DataTag::None: break;
  case DataTag::Str: assert(m_data.sval->isStatic()); break;
  case DataTag::Arr: assert(m_data.aval->isStatic()); break;
  case DataTag::Dbl: break;
  case DataTag::Int: break;
  case DataTag::Cls: break;
  case DataTag::Obj:
    if (auto t = m_data.dobj.whType.get()) {
      t->checkInvariants();
    }
    break;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

Type wait_handle(const Index& index, Type inner) {
  auto const rwh = index.builtin_class(s_WaitHandle.get());
  auto t = subObj(rwh);
  t.m_data.dobj.whType = make_copy_ptr(std::move(inner));
  return t;
}

bool is_specialized_wait_handle(const Type& t) {
  return
    t.m_dataTag == DataTag::Obj &&
    !!t.m_data.dobj.whType.get();
}

Type wait_handle_inner(const Type& t) {
  assert(is_specialized_wait_handle(t));
  return *t.m_data.dobj.whType;
}

Type Type::wait_handle_outer(const Type& wh) {
  auto ret      = Type{wh.m_bits};
  ret.m_dataTag = DataTag::Obj;
  new (&ret.m_data.dobj) DObj(wh.m_data.dobj.type, wh.m_data.dobj.cls);
  return ret;
}

Type sval(SString val) {
  assert(val->isStatic());
  auto r        = Type { BSStr };
  r.m_dataTag   = DataTag::Str;
  r.m_data.sval = val;
  return r;
}

Type ival(int64_t val) {
  auto r        = Type { BInt };
  r.m_dataTag   = DataTag::Int;
  r.m_data.ival = val;
  return r;
}

Type dval(double val) {
  auto r        = Type { BDbl };
  r.m_dataTag   = DataTag::Dbl;
  r.m_data.dval = val;
  return r;
}

Type aval(SArray val) {
  assert(val->isStatic());
  auto r        = Type { BSArr };
  r.m_dataTag   = DataTag::Arr;
  r.m_data.aval = val;
  return r;
}

Type subObj(res::Class val) {
  auto r        = Type { BObj };
  r.m_dataTag   = DataTag::Obj;
  new (&r.m_data.dobj) DObj(
    val.couldBeOverriden() ? DObj::Sub : DObj::Exact,
    val
  );
  return r;
}

Type objExact(res::Class val) {
  auto r        = Type { BObj };
  r.m_dataTag   = DataTag::Obj;
  new (&r.m_data.dobj) DObj(DObj::Exact, val);
  return r;
}

Type subCls(res::Class val) {
  auto r        = Type { BCls };
  r.m_dataTag   = DataTag::Cls;
  r.m_data.dcls = DCls {
    val.couldBeOverriden() ? DCls::Sub : DCls::Exact,
    val
  };
  return r;
}

Type clsExact(res::Class val) {
  auto r        = Type { BCls };
  r.m_dataTag   = DataTag::Cls;
  r.m_data.dcls = DCls { DCls::Exact, val };
  return r;
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
    if (is_opt(t)) {
      break;
    }
    switch (t.m_dataTag) {
    case DataTag::Int:  return make_tv<KindOfInt64>(t.m_data.ival);
    case DataTag::Dbl:  return make_tv<KindOfDouble>(t.m_data.dval);
    case DataTag::Str:  return make_tv<KindOfStaticString>(t.m_data.sval);
    case DataTag::Arr:  return make_tv<KindOfArray>(
                          const_cast<ArrayData*>(t.m_data.aval)
                        );
    case DataTag::Obj:  break;
    case DataTag::Cls:  break;
    case DataTag::None: break;
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
  return t.m_data.dobj;
}

DCls dcls_of(Type t) {
  assert(t.checkInvariants());
  assert(t.strictSubtypeOf(TCls));
  assert(t.m_dataTag == DataTag::Cls);
  return t.m_data.dcls;
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
      *a.m_data.dobj.whType = union_of(
        *a.m_data.dobj.whType,
        *b.m_data.dobj.whType
      );
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
    auto t = a.m_data.dobj.cls.commonAncestor(dobj_of(b).cls);
    // We need not to distinguish between Obj<=T and Obj=T, and always
    // return an Obj<=Ancestor, because that is the single type that
    // includes both children.
    if (t) return keepOpt ? opt(subObj(*t)) : subObj(*t);
    return keepOpt ? TOptObj : TObj;
  }
  if (a.strictSubtypeOf(TCls) && b.strictSubtypeOf(TCls)) {
    auto t = a.m_data.dcls.cls.commonAncestor(dcls_of(b).cls);
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
