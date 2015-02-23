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

#include "hphp/runtime/vm/jit/type.h"

#include <boost/algorithm/string/trim.hpp>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/MapUtil.h>
#include <folly/gen/Base.h>

#include "hphp/util/abi-cxx.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/shape.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/struct-array-defs.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"

#include <vector>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * All non-unknown pointer kinds are disjoint, except the ref bit and Memb.
 * Unk is the top of the lattice, and all of the non-ref types are subtypes of
 * their ref version.  The Memb type includes Prop, Arr, MIS, but not the other
 * locations (it also includes some other special cases like the lvalBlackHole,
 * and currently is the only type for pointers into collection memory).
 *
 * It looks something like this:
 *
 *                            Unk
 *                             |
 *         +-------------------+----+--------+-------+
 *         |                        |        |       |
 *       RMemb                      |     ClsInit  ClsCns
 *         |                        |
 *  +------+---------+              |
 *  |      |         |              |
 *  |      |         |              |
 *  |      |         |              |
 *  |      |    +----+-----+        +--------+----- ... etc
 *  |      |    |    |     |        |        |
 *  |    Memb  RMIS RProp RArr    RFrame    RStk
 *  |      |   /  | /   | /|        |  \      | \
 *  |   +--+-+/---|/+   |/ |        |  Frame  |  Stk
 *  |   |    /    / |   /  |        |         |
 *  |   |   /|   /| |  /|  |        |         |
 *  |   |  / |  / | | / |  |        |         |
 *  |   MIS  Prop | Arr |  |        |         |
 *  |             |     |  |        |         |
 *  +-------------+--+--+--+--------+---------+
 *                   |
 *                  Ref
 *
 */

constexpr auto kPtrRefBit = static_cast<uint32_t>(Ptr::Ref);

bool has_ref(Ptr p) {
  assert(p != Ptr::Unk);
  return static_cast<uint32_t>(p) & kPtrRefBit;
}

Ptr add_ref(Ptr p) {
  if (p == Ptr::Unk || p == Ptr::ClsInit || p == Ptr::ClsCns) {
    return p;
  }
  return static_cast<Ptr>(static_cast<uint32_t>(p) | kPtrRefBit);
}

Ptr remove_ref(Ptr p) {
  // If p is unknown, or Ptr::Ref, we'll get back unknown.
  return static_cast<Ptr>(static_cast<uint32_t>(p) & ~kPtrRefBit);
}

Ptr ptr_union(Ptr a, Ptr b) {
  if (ptr_subtype(a, b)) return b;
  if (ptr_subtype(b, a)) return a;
#define X(y) if (ptr_subtype(a, y) && ptr_subtype(b, y)) return y;
  X(Ptr::RFrame);
  X(Ptr::RStk);
  X(Ptr::RGbl);
  X(Ptr::RProp);
  X(Ptr::RArr);
  X(Ptr::RSProp);
  X(Ptr::RMIS);
  X(Ptr::RMemb);
#undef X
  return Ptr::Unk;
}

folly::Optional<Ptr> ptr_isect(Ptr a, Ptr b) {
  if (ptr_subtype(a, b)) return a;
  if (ptr_subtype(b, a)) return b;
  // The types are at least partially disjoint.  The lattice is small: just
  // handle all the cases.  (If we only had more bits in Type this would be
  // nicer...)
  if (has_ref(a) && !has_ref(b)) {
    if (a == Ptr::Ref) return folly::none;
    return ptr_isect(remove_ref(a), b);
  }
  if (has_ref(b) && !has_ref(a)) {
    // Do the above.
    return ptr_isect(b, a);
  }
  if (has_ref(a) && has_ref(b)) {
    auto const nonref = ptr_isect(remove_ref(a), remove_ref(b));
    if (nonref) return ptr_union(Ptr::Ref, *nonref);
    return Ptr::Ref;
  }
  // Now we only have to intersect things that don't contain refs, and aren't
  // subtypes of each other, and we don't have any of that.  Anything here is
  // disjoint.
  return folly::none;
}

ALWAYS_INLINE Ptr operator|(Ptr a, Ptr b) {
  return ptr_union(a, b);
}
ALWAYS_INLINE folly::Optional<Ptr> operator&(Ptr a, Ptr b) {
  return ptr_isect(a, b);
}

//////////////////////////////////////////////////////////////////////

}

bool ptr_subtype(Ptr a, Ptr b) {
  if (a == b) return true;
  if (b == Ptr::Unk) return true;
  if (b == Ptr::RMemb) {
    return ptr_subtype(a, Ptr::Memb) ||
           a == Ptr::Ref ||
           a == Ptr::RMIS ||
           a == Ptr::RProp ||
           a == Ptr::RArr;
  }
  if (b == Ptr::Memb) {
    return a == Ptr::MIS ||
           a == Ptr::Prop ||
           a == Ptr::Arr;
  }
  // All the remaining cases are just the maybe-ref version of each pointer
  // type.  (Equality was handled first.)
  if (has_ref(b)) {
    return a == Ptr::Ref || remove_ref(b) == a;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

#define IRT(name, ...) const Type Type::name(Type::k##name, Ptr::Unk);
#define IRTP(name, ptr, ...) const Type Type::name(Type::k##name, Ptr::ptr);
IR_TYPES
#undef IRT
#undef IRTP

std::string Type::constValString() const {
  assert(isConst());

  if (*this <= Bool) {
    return m_boolVal ? "true" : "false";
  }
  if (*this <= Int) {
    return folly::format("{}", m_intVal).str();
  }
  if (*this <= Dbl) {
    // don't format doubles as integers.
    auto s = folly::format("{}", m_dblVal).str();
    if (!strchr(s.c_str(), '.') && !strchr(s.c_str(), 'e')) {
      return folly::format("{:.1f}", m_dblVal).str();
    }
    return s;
  }
  if (*this <= StaticStr) {
    auto str = m_strVal;
    return folly::format("\"{}\"", escapeStringForCPP(str->data(),
                                                      str->size())).str();
  }
  if (*this <= StaticArr) {
    if (m_arrVal->empty()) {
      return "array()";
    }
    return folly::format("Array({})", m_arrVal).str();
  }
  if (*this <= Func) {
    return folly::format("Func({})", m_funcVal ? m_funcVal->fullName()->data()
                                               : "nullptr").str();
  }
  if (*this <= Cls) {
    return folly::format("Cls({})", m_clsVal ? m_clsVal->name()->data()
                                             : "nullptr").str();
  }
  if (*this <= Cctx) {
    if (!m_intVal) {
      return "Cctx(Cls(nullptr))";
    }
    const Class* cls = m_cctxVal.cls();
    return folly::format("Cctx(Cls({}))", cls->name()->data()).str();
  }
  if (*this <= TCA) {
    auto name = getNativeFunctionName(m_tcaVal);
    const char* hphp = "HPHP::";

    if (!name.compare(0, strlen(hphp), hphp)) {
      name = name.substr(strlen(hphp));
    }
    auto pos = name.find_first_of('(');
    if (pos != std::string::npos) {
      name = name.substr(0, pos);
    }
    return folly::format("TCA: {}({})", m_tcaVal, boost::trim_copy(name)).str();
  }
  if (*this <= RDSHandle) {
    return folly::format("rds::Handle({:#x})", m_rdsHandleVal).str();
  }
  if (subtypeOfAny(Null, Nullptr) || *this <= PtrToGen) {
    return toString();
  }

  not_reached();
}

std::string Type::toString() const {
  // Try to find an exact match to a predefined type
# define IRT(name, ...) if (*this == name) return #name;
# define IRTP(name, ...) IRT(name)
  IR_TYPES
# undef IRT
# undef IRTP

  if (maybe(Nullptr)) {
    return folly::to<std::string>(
      "Nullptr|",
      (*this - Type::Nullptr).toString()
    );
  }

  if (*this <= BoxedCell) {
    return folly::to<std::string>("Boxed", inner().toString());
  }
  if (*this <= PtrToGen) {
    std::string ret = "PtrTo";
    switch (ptrKind()) {
    case Ptr::Unk:      break;
    case Ptr::Frame:    ret += "Frame"; break;
    case Ptr::Stk:      ret += "Stk"; break;
    case Ptr::Gbl:      ret += "Gbl"; break;
    case Ptr::Prop:     ret += "Prop"; break;
    case Ptr::Arr:      ret += "Arr"; break;
    case Ptr::SProp:    ret += "SProp"; break;
    case Ptr::MIS:      ret += "MIS"; break;
    case Ptr::Memb:     ret += "Memb"; break;
    case Ptr::ClsInit:  ret += "ClsInit"; break;
    case Ptr::ClsCns:   ret += "ClsCns"; break;
    case Ptr::RFrame:   ret += "RFrame"; break;
    case Ptr::RStk:     ret += "RStk"; break;
    case Ptr::RGbl:     ret += "RGbl"; break;
    case Ptr::RProp:    ret += "RProp"; break;
    case Ptr::RArr:     ret += "RArr"; break;
    case Ptr::RSProp:   ret += "RSProp"; break;
    case Ptr::RMIS:     ret += "RMIS"; break;
    case Ptr::RMemb:    ret += "RMemb"; break;
    case Ptr::Ref:      ret += "Ref"; break;
    }
    ret += deref().toString();
    if (isConst()) ret += folly::format("({})", m_ptrVal).str();
    return ret;
  }

  if (m_hasConstVal) {
    return folly::format("{}<{}>",
                         dropConstVal().toString(), constValString()).str();
  }

  auto t = *this;
  std::vector<std::string> parts;
  if (isSpecialized()) {
    if (auto clsSpec = this->clsSpec()) {
      auto const base = Type(m_bits & kAnyObj, Ptr::Unk).toString();
      auto const exact = clsSpec.exact() ? "=" : "<=";
      auto const name = clsSpec.cls()->name()->data();
      auto const partStr = folly::to<std::string>(base, exact, name);

      parts.push_back(partStr);
      t -= AnyObj;
    } else if (auto arrSpec = this->arrSpec()) {
      auto str = folly::to<std::string>(
        Type(m_bits & kAnyArr, Ptr::Unk).toString());
      if (auto const kind = arrSpec.kind()) {
        str += "=";
        str += ArrayData::kindToString(*kind);
      }
      if (auto const ty = arrSpec.type()) {
        str += folly::to<std::string>(':', show(*ty));
      }
      if (auto const shape = arrSpec.shape()) {
        str += folly::to<std::string>(":", show(*shape));
      }
      parts.push_back(str);
      t -= AnyArr;
    } else {
      not_reached();
    }
  }

  // Concat all of the primitive types in the custom union type
# define IRT(name, ...) if (name <= t) parts.push_back(#name);
# define IRTP(name, ...) IRT(name)
  IRT_PRIMITIVE
# undef IRT
# undef IRTP
  assert(!parts.empty());
  if (parts.size() == 1) {
    return parts.front();
  }
  return folly::format("{{{}}}", folly::join('|', parts)).str();
}

std::string Type::debugString(Type t) {
  return t.toString();
}

///////////////////////////////////////////////////////////////////////////////

bool Type::checkValid() const {
  // Note: be careful, the Type::Foo objects aren't all constructed yet in this
  // function.
  if (m_extra) {
    assert((!(m_bits & kAnyObj) || !(m_bits & kAnyArr)) &&
           "Conflicting specialization");
  }
  if ((m_bits >> kPtrShift) == 0) { // !maybe(PtrToGen)
    assert(m_ptrKind == 0);
  }
  static_assert(static_cast<uint32_t>(Ptr::Unk) == 0, "");

  return true;
}

DataType Type::toDataType() const {
  assert(!maybe(PtrToGen) || m_bits == kBottom);
  assert(isKnownDataType());

  // Order is important here: types must progress from more specific
  // to less specific to return the most specific DataType.
  if (*this <= Uninit)      return KindOfUninit;
  if (*this <= InitNull)    return KindOfNull;
  if (*this <= Bool)        return KindOfBoolean;
  if (*this <= Int)         return KindOfInt64;
  if (*this <= Dbl)         return KindOfDouble;
  if (*this <= StaticStr)   return KindOfStaticString;
  if (*this <= Str)         return KindOfString;
  if (*this <= Arr)         return KindOfArray;
  if (*this <= Obj)         return KindOfObject;
  if (*this <= Res)         return KindOfResource;
  if (*this <= BoxedCell)   return KindOfRef;
  if (*this <= Cls)         return KindOfClass;
  always_assert_flog(false,
                     "Bad Type {} in Type::toDataType()", *this);
}

Type::bits_t Type::bitsFromDataType(DataType outer, DataType inner) {
  assert(inner != KindOfRef);
  assert(inner == KindOfUninit || outer == KindOfRef);

  switch (outer) {
    case KindOfUninit        : return kUninit;
    case KindOfNull          : return kInitNull;
    case KindOfBoolean       : return kBool;
    case KindOfInt64         : return kInt;
    case KindOfDouble        : return kDbl;
    case KindOfStaticString  : return kStaticStr;
    case KindOfString        : return kStr;
    case KindOfArray         : return kArr;
    case KindOfResource      : return kRes;
    case KindOfObject        : return kObj;
    case KindOfClass         : return kCls;
    case KindOfRef:
      assert(inner != KindOfUninit);
      return bitsFromDataType(inner, KindOfUninit) << kBoxShift;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
// Combinators.

Type Type::specialize(TypeSpec spec) const {
  auto bits = m_bits;
  auto ptr = ptrKind();

  bool arr_okay = supports(SpecKind::Array);
  bool cls_okay = supports(SpecKind::Class);

  // If we support no specializations, we're done.
  if (!arr_okay && !cls_okay) return *this;

  // Remove the bits corresponding to any Bottom specializations---the
  // specializations intersected to zero, so the type component is impossible.
  if (spec.clsSpec() == ClassSpec::Bottom) {
    bits &= ~kAnyObj;
    cls_okay = false;
  }
  if (spec.arrSpec() == ArraySpec::Bottom) {
    bits &= ~kAnyArr;
    arr_okay = false;
  }

  auto generic = Type(bits, ptr);

  // If we support a nonsingular number of specializations, we're done.
  if (arr_okay == cls_okay) return *this;

  if (cls_okay && spec.clsSpec()) return Type(generic, spec.clsSpec());
  if (arr_okay && spec.arrSpec()) return Type(generic, spec.arrSpec());

  return *this;
}

Type Type::operator|(Type rhs) const {
  auto lhs = *this;

  // Representing types like {Int<12>|Arr} could get messy and isn't useful in
  // practice, so unless we're unifying a constant type with itself or Bottom,
  // drop the constant value(s).
  if (lhs == rhs || rhs == Bottom) return lhs;
  if (lhs == Bottom) return rhs;

  lhs = lhs.dropConstVal();
  rhs = rhs.dropConstVal();

  auto const ptr = [&] {
    // Handle cases where one of the types has no intersection with pointer
    // types.  We don't need to widen the resulting pointer kind at all in that
    // case.
    if (!lhs.maybe(Type::PtrToGen)) return rhs.ptrKind();
    if (!rhs.maybe(Type::PtrToGen)) return lhs.ptrKind();
    return lhs.ptrKind() | rhs.ptrKind();
  }();
  auto const bits = lhs.m_bits | rhs.m_bits;

  return Type(bits, ptr).specialize(lhs.spec() | rhs.spec());
}

Type Type::operator&(Type rhs) const {
  auto lhs = *this;

  // When intersecting a constant value with another type, the result will be
  // the constant value if the other value is a supertype of the constant, and
  // Bottom otherwise.
  if (lhs.m_hasConstVal) return lhs <= rhs ? lhs : Bottom;
  if (rhs.m_hasConstVal) return rhs <= lhs ? rhs : Bottom;

  auto const bits = lhs.m_bits & rhs.m_bits;
  auto const opt_ptr = lhs.ptrKind() & rhs.ptrKind();
  bool const is_ptr = bits & Type::PtrToGen.m_bits;

  if (!opt_ptr) return Bottom;
  auto const ptr = is_ptr ? *opt_ptr : Ptr::Unk;

  return Type(bits, ptr).specialize(lhs.spec() & rhs.spec());
}

Type Type::operator-(Type rhs) const {
  auto lhs = *this;

  if (lhs <= rhs) return Bottom;

  // If `rhs' has a constant, but `lhs' doesn't, just (conservatively) return
  // `lhs', rather than trying to represent things like "everything except
  // Int<24>".
  if (rhs.m_hasConstVal) return lhs;

  // If we have pointers to different kinds of things, be conservative unless
  // `rhs' is an unknown pointer type, in which case we can just subtract the
  // pointers but keep our kind.
  if (lhs.ptrKind() != rhs.ptrKind() &&
      rhs.ptrKind() != Ptr::Unk) {
    return lhs;
  }

  auto bits = lhs.m_bits & ~rhs.m_bits;
  auto const ptr = lhs.ptrKind();

  // Put back any bits for which `rhs' admitted a nontrivial specialization.
  // If these specializations would be subtracted out of lhs's specializations,
  // the finalization below will take care of re-eliminating it.
  if (rhs.arrSpec()) bits |= (rhs.m_bits & kAnyArr);
  if (rhs.clsSpec()) bits |= (rhs.m_bits & kAnyObj);

  auto ty = Type(bits, ptr).specialize(lhs.spec() - rhs.spec());

  if (lhs.m_hasConstVal) {
    // If `lhs' was a constant, we should not have somehow developed a
    // specialization in this process (with the exception of array constants,
    // which pretend to be ArrayKind specializations).
    assert(!ty.isSpecialized() || ty.arrSpec());
    ty.m_hasConstVal = true;
    ty.m_extra = lhs.m_extra;
  }
  return ty;
}

///////////////////////////////////////////////////////////////////////////////

Type liveTVType(const TypedValue* tv) {
  assert(tv->m_type == KindOfClass || tvIsPlausible(*tv));

  if (tv->m_type == KindOfObject) {
    Class* cls = tv->m_data.pobj->getVMClass();

    // We only allow specialization on classes that can't be
    // overridden for now. If this changes, then this will need to
    // specialize on sub object types instead.
    if (!cls || !(cls->attrs() & AttrNoOverride)) return Type::Obj;
    return Type::ExactObj(cls);
  }
  if (tv->m_type == KindOfArray) {
    ArrayData* ar = tv->m_data.parr;
    if (ar->kind() == ArrayData::kStructKind) {
      return Type::Array(StructArray::asStructArray(ar)->shape());
    }
    return Type::Array(tv->m_data.parr->kind());
  }

  auto outer = tv->m_type;
  auto inner = KindOfUninit;

  if (outer == KindOfStaticString) outer = KindOfString;
  if (outer == KindOfRef) {
    inner = tv->m_data.pref->tv()->m_type;
    if (inner == KindOfStaticString) inner = KindOfString;
  }
  return Type(outer, inner);
}

Type Type::relaxToGuardable() const {
  auto const ty = unspecialize();

  if (ty.isKnownDataType()) return ty;

  if (ty <= UncountedInit)  return Type::UncountedInit;
  if (ty <= Uncounted)      return Type::Uncounted;
  if (ty <= Cell)           return Type::Cell;
  if (ty <= BoxedCell)      return Type::BoxedCell;
  if (ty <= Gen)            return Type::Gen;
  not_reached();
}

//////////////////////////////////////////////////////////////////////

namespace {

Type setElemReturn(const IRInstruction* inst) {
  assert(inst->op() == SetElem);
  auto baseType = inst->src(minstrBaseIdx(inst->op()))->type().strip();

  // If the base is a Str, the result will always be a CountedStr (or
  // an exception). If the base might be a str, the result wil be
  // CountedStr or Nullptr. Otherwise, the result is always Nullptr.
  if (baseType <= Type::Str) {
    return Type::CountedStr;
  } else if (baseType.maybe(Type::Str)) {
    return Type::CountedStr | Type::Nullptr;
  }
  return Type::Nullptr;
}

Type builtinReturn(const IRInstruction* inst) {
  assert(inst->op() == CallBuiltin);

  Type t = inst->typeParam();
  if (t.isSimpleType() || t == Type::Cell) {
    return t;
  }
  if (t.isReferenceType() || t == Type::BoxedCell) {
    return t | Type::InitNull;
  }
  not_reached();
}

Type thisReturn(const IRInstruction* inst) {
  auto const func = inst->marker().func();

  // If the function is a cloned closure which may have a re-bound $this which
  // is not a subclass of the context return an unspecialized type.
  if (func->hasForeignThis()) return Type::Obj;

  if (auto const cls = func->cls()) {
    return Type::SubObj(cls);
  }
  return Type::Obj;
}

Type allocObjReturn(const IRInstruction* inst) {
  switch (inst->op()) {
    case ConstructInstance:
      return Type::SubObj(inst->extra<ConstructInstance>()->cls);

    case NewInstanceRaw:
      return Type::ExactObj(inst->extra<NewInstanceRaw>()->cls);

    case AllocObj:
      return inst->src(0)->isConst()
        ? Type::ExactObj(inst->src(0)->clsVal())
        : Type::Obj;

    default:
      always_assert(false && "Invalid opcode returning AllocObj");
  }
}

Type arrElemReturn(const IRInstruction* inst) {
  assert(inst->op() == LdPackedArrayElem || inst->op() == LdStructArrayElem);
  auto const tyParam = inst->hasTypeParam() ? inst->typeParam() : Type::Gen;
  assert(!inst->hasTypeParam() || inst->typeParam() <= Type::Gen);

  auto const arrTy = inst->src(0)->type().arrSpec().type();
  if (!arrTy) return tyParam;

  using T = RepoAuthType::Array::Tag;
  switch (arrTy->tag()) {
  case T::Packed:
    {
      auto const idx = inst->src(1);
      if (!idx->isConst()) return Type::Gen;
      if (idx->intVal() >= 0 && idx->intVal() < arrTy->size()) {
        return convertToType(arrTy->packedElem(idx->intVal())) & tyParam;
      }
    }
    return Type::Gen;
  case T::PackedN:
    return convertToType(arrTy->elemType()) & tyParam;
  }

  return tyParam;
}

Type unboxPtr(Type t) {
  t = t - Type::PtrToBoxedCell;
  return t.deref().ptr(add_ref(t.ptrKind()));
}

Type boxPtr(Type t) {
  auto const rawBoxed = t.deref().unbox().box();
  auto const noNull = rawBoxed - Type::BoxedUninit;
  return noNull.ptr(remove_ref(t.ptrKind()));
}

}

Type ldRefReturn(Type typeParam) {
  assert(typeParam <= Type::Cell);
  // Guarding on specialized types and uncommon unions like {Int|Bool} is
  // expensive enough that we only want to do it in situations where we've
  // manually confirmed the benefit.
  auto const type = typeParam.unspecialize();

  if (type.isKnownDataType())      return type;
  if (type <= Type::UncountedInit) return Type::UncountedInit;
  if (type <= Type::Uncounted)     return Type::Uncounted;
  always_assert(type <= Type::Cell);
  return Type::InitCell;
}

Type boxType(Type t) {
  // If t contains Uninit, replace it with InitNull.
  t = t.maybe(Type::Uninit) ? (t - Type::Uninit) | Type::InitNull : t;
  // We don't try to track when a BoxedStaticStr might be converted to
  // a BoxedStr, and we never guard on staticness for strings, so
  // boxing a string needs to forget this detail.  Same thing for
  // arrays.
  if (t <= Type::Str) {
    t = Type::Str;
  } else if (t <= Type::Arr) {
    t = Type::Arr;
  }
  // When boxing an Object, if the inner class does not have AttrNoOverride,
  // drop the class specialization.
  if (t < Type::Obj && t.clsSpec() &&
      !(t.clsSpec().cls()->attrs() & AttrNoOverride)) {
    t = t.unspecialize();
  }
  // Everything else is just a pure type-system boxing operation.
  return t.box();
}

Type convertToType(RepoAuthType ty) {
  using T = RepoAuthType::Tag;
  switch (ty.tag()) {
  case T::OptBool:        return Type::Bool      | Type::InitNull;
  case T::OptInt:         return Type::Int       | Type::InitNull;
  case T::OptSStr:        return Type::StaticStr | Type::InitNull;
  case T::OptStr:         return Type::Str       | Type::InitNull;
  case T::OptDbl:         return Type::Dbl       | Type::InitNull;
  case T::OptRes:         return Type::Res       | Type::InitNull;
  case T::OptObj:         return Type::Obj       | Type::InitNull;

  case T::Uninit:         return Type::Uninit;
  case T::InitNull:       return Type::InitNull;
  case T::Null:           return Type::Null;
  case T::Bool:           return Type::Bool;
  case T::Int:            return Type::Int;
  case T::Dbl:            return Type::Dbl;
  case T::Res:            return Type::Res;
  case T::SStr:           return Type::StaticStr;
  case T::Str:            return Type::Str;
  case T::Obj:            return Type::Obj;

  case T::Cell:           return Type::Cell;
  case T::Ref:            return Type::BoxedInitCell;
  case T::InitUnc:        return Type::UncountedInit;
  case T::Unc:            return Type::Uncounted;
  case T::InitCell:       return Type::InitCell;
  case T::InitGen:        return Type::Init;
  case T::Gen:            return Type::Gen;

  // TODO(#4205897): option specialized array types
  case T::OptArr:         return Type::Arr       | Type::InitNull;
  case T::OptSArr:        return Type::StaticArr | Type::InitNull;

  case T::SArr:
    if (auto const ar = ty.array()) return Type::StaticArray(ar);
    return Type::StaticArr;
  case T::Arr:
    if (auto const ar = ty.array()) return Type::Array(ar);
    return Type::Arr;

  case T::SubObj:
  case T::ExactObj:
  case T::OptSubObj:
  case T::OptExactObj: {
    auto base = Type::Obj;

    if (auto const cls = Unit::lookupClassOrUniqueClass(ty.clsName())) {
      if (ty.tag() == T::ExactObj || ty.tag() == T::OptExactObj) {
        base = Type::ExactObj(cls);
      } else {
        base = Type::SubObj(cls);
      }
    }
    if (ty.tag() == T::OptSubObj || ty.tag() == T::OptExactObj) {
      base |= Type::InitNull;
    }
    return base;
  }
  }
  not_reached();
}

namespace TypeNames {
#define IRT(name, ...) UNUSED const Type name = Type::name;
#define IRTP(name, ...) IRT(name)
  IR_TYPES
#undef IRT
#undef IRTP
};

Type outputType(const IRInstruction* inst, int dstId) {
  using namespace TypeNames;
  using TypeNames::TCA;
#define D(type)         return type;
#define DofS(n)         return inst->src(n)->type();
#define DRefineS(n)     return inst->src(n)->type() & inst->typeParam();
#define DParamMayRelax  return inst->typeParam();
#define DParam          return inst->typeParam();
#define DParamPtr(k)    assert(inst->typeParam() <= Type::Gen.ptr(Ptr::k)); \
                        return inst->typeParam();
#define DUnboxPtr       return unboxPtr(inst->src(0)->type());
#define DBoxPtr         return boxPtr(inst->src(0)->type());
#define DAllocObj       return allocObjReturn(inst);
#define DArrElem        return arrElemReturn(inst);
#define DArrPacked      return Type::Array(ArrayData::kPackedKind);
#define DThis           return thisReturn(inst);
#define DMulti          return Type::Bottom;
#define DSetElem        return setElemReturn(inst);
#define ND              assert(0 && "outputType requires HasDest or NaryDest");
#define DBuiltin        return builtinReturn(inst);
#define DSubtract(n, t) return inst->src(n)->type() - t;
#define DCns            return Type::Uninit | Type::InitNull | Type::Bool | \
                               Type::Int | Type::Dbl | Type::Str | Type::Res;

#define O(name, dstinfo, srcinfo, flags) case name: dstinfo not_reached();

  switch (inst->op()) {
  IR_OPCODES
  default: not_reached();
  }

#undef O

#undef D
#undef DofS
#undef DRefineS
#undef DParamMayRelax
#undef DParam
#undef DParamPtr
#undef DUnboxPtr
#undef DBoxPtr
#undef DAllocObj
#undef DArrElem
#undef DArrPacked
#undef DThis
#undef DMulti
#undef DSetElem
#undef ND
#undef DBuiltin
#undef DSubtract
#undef DCns

}

//////////////////////////////////////////////////////////////////////

namespace {

// Returns a union type containing all the types in the
// variable-length argument list
Type buildUnion() {
  return Type::Bottom;
}

template<class... Args>
Type buildUnion(Type t, Args... ts) {
  return t | buildUnion(ts...);
}

}

/*
 * Runtime typechecking for IRInstruction operands.
 *
 * This is generated using the table in ir-opcode.h.  We instantiate
 * IR_OPCODES after defining all the various source forms to do type
 * assertions according to their form (see ir-opcode.h for documentation on
 * the notation).  The checkers appear in argument order, so each one
 * increments curSrc, and at the end we can check that the argument
 * count was also correct.
 */
bool checkOperandTypes(const IRInstruction* inst, const IRUnit* unit) {
  int curSrc = 0;

  auto bail = [&] (const std::string& msg) {
    FTRACE(1, "{}", msg);
    fprintf(stderr, "%s\n", msg.c_str());

    always_assert_log(false, [&] { return msg; });
  };

  if (opHasExtraData(inst->op()) != (bool)inst->rawExtra()) {
    bail(folly::format("opcode {} should{} have an ExtraData struct "
                       "but instruction {} does{}",
                       inst->op(),
                       opHasExtraData(inst->op()) ? "" : "n't",
                       *inst,
                       inst->rawExtra() ? "" : "n't").str());
  }

  auto src = [&]() -> SSATmp* {
    if (curSrc < inst->numSrcs()) {
      return inst->src(curSrc);
    }

    bail(folly::format(
      "Error: instruction had too few operands\n"
      "   instruction: {}\n",
        inst->toString()
      ).str()
    );
    not_reached();
  };

  // If expected is not nullptr, it will be used. Otherwise, t.toString() will
  // be used as the expected string.
  auto check = [&] (bool cond, const Type t, const char* expected) {
    if (cond) return true;

    std::string expectStr = expected ? expected : t.toString();

    bail(folly::format(
      "Error: failed type check on operand {}\n"
      "   instruction: {}\n"
      "   was expecting: {}\n"
      "   received: {}\n",
        curSrc,
        inst->toString(),
        expectStr,
        inst->src(curSrc)->type().toString()
      ).str()
    );
    return true;
  };

  auto checkNoArgs = [&]{
    if (inst->numSrcs() == 0) return true;
    bail(folly::format(
      "Error: instruction expected no operands\n"
      "   instruction: {}\n",
        inst->toString()
      ).str()
    );
    return true;
  };

  auto countCheck = [&]{
    if (inst->numSrcs() == curSrc) return true;
    bail(folly::format(
      "Error: instruction had too many operands\n"
      "   instruction: {}\n"
      "   expected {} arguments\n",
        inst->toString(),
        curSrc
      ).str()
    );
    return true;
  };

  auto checkDst = [&] (bool cond, const std::string& errorMessage) {
    if (cond) return true;

    bail(folly::format("Error: failed type check on dest operand\n"
                       "   instruction: {}\n"
                       "   message: {}\n",
                       inst->toString(),
                       errorMessage).str());
    return true;
  };

  auto requireTypeParam = [&] {
    checkDst(inst->hasTypeParam() || inst->is(DefConst),
             "Missing paramType for DParam instruction");
  };

  auto requireTypeParamPtr = [&] (Ptr kind) {
    checkDst(inst->hasTypeParam(),
      "Missing paramType for DParamPtr instruction");
    if (inst->hasTypeParam()) {
      checkDst(inst->typeParam() <= Type::Gen.ptr(kind),
               "Invalid paramType for DParamPtr instruction");
    }
  };

  auto checkVariadic = [&] (Type super) {
    for (; curSrc < inst->numSrcs(); ++curSrc) {
      auto const valid = (inst->src(curSrc)->type() <= super);
      check(valid, Type(), nullptr);
    }
  };

#define IRT(name, ...) UNUSED static const Type name = Type::name;
#define IRTP(name, ...) IRT(name)
  IR_TYPES
#undef IRT
#undef IRTP

#define NA            return checkNoArgs();
#define S(...)        {                                   \
                        Type t = buildUnion(__VA_ARGS__); \
                        check(src()->isA(t), t, nullptr); \
                        ++curSrc;                         \
                      }
#define AK(kind)      {                                                 \
                        Type t = Type::Array(ArrayData::k##kind##Kind); \
                        check(src()->isA(t), t, nullptr);               \
                        ++curSrc;                                       \
                      }
#define C(type)       check(src()->isConst() && \
                            src()->isA(type),   \
                            Type(),             \
                            "constant " #type); \
                      ++curSrc;
#define CStr          C(StaticStr)
#define SVar(...)     checkVariadic(buildUnion(__VA_ARGS__));
#define ND
#define DMulti
#define DSetElem
#define D(...)
#define DBuiltin
#define DSubtract(src, t)checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DofS(src)   checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DRefineS(src) checkDst(src < inst->numSrcs(),  \
                               "invalid src num");     \
                      requireTypeParam();
#define DParamMayRelax requireTypeParam();
#define DParam         requireTypeParam();
#define DParamPtr(k)   requireTypeParamPtr(Ptr::k);
#define DUnboxPtr
#define DBoxPtr
#define DAllocObj
#define DArrElem
#define DArrPacked
#define DThis
#define DCns

#define O(opcode, dstinfo, srcinfo, flags) \
  case opcode: dstinfo srcinfo countCheck(); return true;

  switch (inst->op()) {
    IR_OPCODES
  default: always_assert(false);
  }

#undef O

#undef NA
#undef S
#undef AK
#undef C
#undef CStr
#undef SVar

#undef ND
#undef D
#undef DBuiltin
#undef DSubtract
#undef DMulti
#undef DSetElem
#undef DofS
#undef DRefineS
#undef DParamMayRelax
#undef DParam
#undef DParamPtr
#undef DUnboxPtr
#undef DBoxPtr
#undef DAllocObj
#undef DArrElem
#undef DArrPacked
#undef DThis
#undef DCns

  return true;
}

//////////////////////////////////////////////////////////////////////

}}
