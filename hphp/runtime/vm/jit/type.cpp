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

  if (subtypeOf(Int)) {
    return folly::format("{}", m_intVal).str();
  } else if (subtypeOf(Dbl)) {
    // don't format doubles as integers.
    auto s = folly::format("{}", m_dblVal).str();
    if (!strchr(s.c_str(), '.') && !strchr(s.c_str(), 'e')) {
      return folly::format("{:.1f}", m_dblVal).str();
    }
    return s;
  } else if (subtypeOf(Bool)) {
    return m_boolVal ? "true" : "false";
  } else if (subtypeOf(StaticStr)) {
    auto str = m_strVal;
    return folly::format("\"{}\"", escapeStringForCPP(str->data(),
                                                      str->size())).str();
  } else if (subtypeOf(StaticArr)) {
    if (m_arrVal->empty()) {
      return "array()";
    }
    return folly::format("Array({})", m_arrVal).str();
  } else if (subtypeOf(Func)) {
    return folly::format("Func({})", m_funcVal ? m_funcVal->fullName()->data()
                                               : "nullptr").str();
  } else if (subtypeOf(Cls)) {
    return folly::format("Cls({})", m_clsVal ? m_clsVal->name()->data()
                                             : "nullptr").str();
  } else if (subtypeOf(Cctx)) {
    if (!m_intVal) {
      return "Cctx(Cls(nullptr))";
    }
    const Class* cls = m_cctxVal.cls();
    return folly::format("Cctx(Cls({}))", cls->name()->data()).str();
  } else if (subtypeOf(TCA)) {
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
  } else if (subtypeOf(RDSHandle)) {
    return folly::format("RDS::Handle({:#x})", m_rdsHandleVal).str();
  } else if (subtypeOfAny(Null, Nullptr) || isPtr()) {
    return toString();
  } else {
    not_reached();
  }
}

std::string Type::toString() const {
  // Try to find an exact match to a predefined type
# define IRT(name, ...) if (*this == name) return #name;
# define IRTP(name, ...) IRT(name)
  IR_TYPES
# undef IRT
# undef IRTP

  if (maybe(Type::Nullptr)) {
    return folly::to<std::string>(
      "Nullptr|",
      (*this - Type::Nullptr).toString()
    );
  }

  if (isBoxed()) {
    return folly::to<std::string>("Boxed", innerType().toString());
  }
  if (isPtr()) {
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
    if (canSpecializeClass()) {
      assert(getClass());

      auto const base = Type(m_bits & kAnyObj, Ptr::Unk).toString();
      auto const exact = getExactClass() ? "=" : "<=";
      auto const name = getClass()->name()->data();
      auto const partStr = folly::to<std::string>(base, exact, name);

      parts.push_back(partStr);
      t -= AnyObj;
    } else if (canSpecializeArray()) {
      auto str = folly::to<std::string>(
        Type(m_bits & kAnyArr, Ptr::Unk).toString());
      if (hasArrayKind()) {
        str += "=";
        str += ArrayData::kindToString(getArrayKind());
      }
      if (auto ty = getArrayType()) {
        str += folly::to<std::string>(':', show(*ty));
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

Type Type::unionOf(Type t1, Type t2) {
  if (t1 == t2 || t2 < t1) return t1;
  if (t1 < t2) return t2;

  if (t1.isPtr() && t2.isPtr()) {
    return unionOf(t1.deref(), t2.deref()).ptr(
      ptr_union(t1.ptrKind(), t2.ptrKind())
    );
  }

  static const Type union_types[] = {
#   define IRT(name, ...) name,
#   define IRTP(name, ...) IRT(name)
    IRT_PHP(IRT_BOXES_AND_PTRS)
    IRT_PHP_UNIONS(IRT_BOXES_AND_PTRS)
#   undef IRT
#   undef IRTP
    Gen,
    PtrToGen,
  };
  Type t12 = t1 | t2;
  for (auto u : union_types) {
    if (t12 <= u) return u;
  }
  not_reached();
}

DataType Type::toDataType() const {
  assert(!isPtr());
  assert(isKnownDataType());

  // Order is important here: types must progress from more specific
  // to less specific to return the most specific DataType.
  if (subtypeOf(Uninit))        return KindOfUninit;
  if (subtypeOf(InitNull))      return KindOfNull;
  if (subtypeOf(Bool))          return KindOfBoolean;
  if (subtypeOf(Int))           return KindOfInt64;
  if (subtypeOf(Dbl))           return KindOfDouble;
  if (subtypeOf(StaticStr))     return KindOfStaticString;
  if (subtypeOf(Str))           return KindOfString;
  if (subtypeOf(Arr))           return KindOfArray;
  if (subtypeOf(Obj))           return KindOfObject;
  if (subtypeOf(Res))           return KindOfResource;
  if (subtypeOf(BoxedCell))     return KindOfRef;
  if (subtypeOf(Cls))           return KindOfClass;
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

// ClassOps and ArrayOps are used below to write code that can perform set
// operations on both Class and ArrayKind specializations.
struct Type::ClassOps {
  static bool subtypeOf(ClassInfo a, ClassInfo b) {
    if (a == b) return true;
    return !b.isExact() && a.get()->classof(b.get());
  }

  static folly::Optional<ClassInfo> commonAncestor(ClassInfo a, ClassInfo b) {
    if (!isNormalClass(a.get()) || !isNormalClass(b.get())) return folly::none;
    if (auto result = a.get()->commonAncestor(b.get())) {
      return ClassInfo(result, ClassTag::Sub);
    }

    return folly::none;
  }

  static bool canIntersect(ClassInfo a, ClassInfo b) {
    // If either is an interface, we'd need to explore all implementing classes
    // in the program to know if they have a non-empty intersection.  Easy
    // cases where one is a subtype of the other still will have been handled,
    // though.
    return isNormalClass(a.get()) && isNormalClass(b.get());
  }
  static ClassInfo conservativeHeuristic(ClassInfo a, ClassInfo b) {
    /*
     * When we can't intersect, we try to take the "better" of the two.  We
     * consider a non-interface better than an interface, because it might
     * influence important things like method dispatch or property accesses
     * better than an interface type could.  (Note that at least one of our
     * ClassInfo's is not a normal class if we're in this code path.)
     *
     * If they are both interfaces we have to pick one arbitrarily, but we must
     * do so in a way that is stable regardless of which one was passed as a or
     * b (to guarantee that operator& is commutative).  We use the class name
     * in that last case to ensure that the ordering is dependent only on the
     * source program (Class* or something like that seems less desirable).
     */
    if (isNormalClass(b.get())) return b;
    if (isNormalClass(a.get())) return a;
    return a.get()->name()->compare(b.get()->name()) < 0 ? a : b;
  }

  static folly::Optional<ClassInfo> intersect(ClassInfo a, ClassInfo b) {
    assert(canIntersect(a, b));
    // Since these classes are not interfaces, they must have a non-empty
    // intersection if we failed the subtype checks.
    return folly::none;
  }
};

struct Type::ArrayOps {
  static bool subtypeOf(ArrayInfo a, ArrayInfo b) {
    if (a == b) return true;
    if (!arrayType(b) && !arrayKindValid(b)) return true;
    return false;
  }

  static folly::Optional<ArrayInfo> commonAncestor(ArrayInfo a, ArrayInfo b) {
    if (a == b) return a;
    auto const sameKind = [&]() -> folly::Optional<ArrayData::ArrayKind> {
      if (arrayKindValid(a)) {
        if (arrayKindValid(b)) {
          if (a == b) return arrayKind(a);
          return folly::none;
        }
        return arrayKind(a);
      }
      if (arrayKindValid(b)) return arrayKind(b);
      return folly::none;
    }();
    auto const ty = [&]() -> const RepoAuthType::Array* {
      auto ata = arrayType(a);
      auto atb = arrayType(b);
      return ata && atb ? (ata == atb ? ata : nullptr) :
             ata ? ata : atb;
    }();
    if (ty || sameKind) return makeArrayInfo(sameKind, ty);
    return folly::none;
  }

  static bool canIntersect(ArrayInfo a, ArrayInfo b) { return true; }
  static ArrayInfo conservativeHeuristic(ArrayInfo, ArrayInfo) {
    not_reached();
  }

  static folly::Optional<ArrayInfo> intersect(ArrayInfo a, ArrayInfo b) {
    assert(a != b);

    auto const aka = okind(a);
    auto const akb = okind(b);
    auto const ata = arrayType(a);
    auto const atb = arrayType(b);
    if (aka == akb) {
      // arrayType must be non-equal by above assertion.  Since the
      // kinds are the same, as long as one is null we can keep the
      // other.
      assert(ata != atb);
      if (ata && atb) return makeArrayInfo(aka, nullptr);
      return makeArrayInfo(aka, ata ? ata : atb);
    }
    if (aka && akb) {
      assert(aka != akb);
      return folly::none;  // arrays of different kinds don't intersect.
    }
    assert(aka.hasValue() || akb.hasValue());
    assert(!(aka.hasValue() && akb.hasValue()));
    if (akb && !aka) return intersect(b, a);
    assert(aka.hasValue() && !akb.hasValue());

    if (!atb) return makeArrayInfo(aka, ata /* could be null */);
    if (!ata) return makeArrayInfo(aka, atb /* could be null */);
    return makeArrayInfo(aka, ata == atb ? ata : nullptr);
  }

private:
  static folly::Optional<ArrayData::ArrayKind> okind(ArrayInfo in) {
    if (arrayKindValid(in)) return arrayKind(in);
    return folly::none;
  }
};

// Union and Intersect implement part of the logic for operator| and operator&,
// respectively. Each has two static methods:
//
// combineSame: called when at least one of *this or b is specialized and
//              they can both specialize on the same type.
// combineDifferent: called when *this and b can specialize different ways
//                   and at least one of the two is specialized.

struct Type::Union {
  template<typename Ops, typename T>
  static Type combineSame(bits_t bits,
                          bits_t typeMask,
                          Ptr newPtrKind,
                          folly::Optional<T> aOpt,
                          folly::Optional<T> bOpt) {
    // If one or both types are not specialized, the specialization is lost
    if (!(aOpt && bOpt)) return Type(bits, newPtrKind);

    auto const a = *aOpt;
    auto const b = *bOpt;

    // If the specialization is the same, keep it.
    if (a == b) return Type(bits, newPtrKind, a);

    // If one is a subtype of the other, their union is the least specific of
    // the two.
    if (Ops::subtypeOf(a, b)) return Type(bits, newPtrKind, b);
    if (Ops::subtypeOf(b, a)) return Type(bits, newPtrKind, a);

    // Check for a common ancestor.
    if (auto p = Ops::commonAncestor(a, b)) return Type(bits, newPtrKind, *p);

    // a and b are unrelated but we can't hold both of them in a Type. Dropping
    // the specialization returns a supertype of their true union. It's not
    // optimal but not incorrect.
    return Type(bits, newPtrKind);
  }

  static Type combineDifferent(bits_t newBits,
                               Ptr newPtrKind,
                               Type a,
                               Type b) {
    // a and b can specialize differently, so their union can't have any
    // specialization (it would be an ambiguously specialized type).
    return Type(newBits, newPtrKind);
  }
};

struct Type::Intersect {
  template<typename Ops, typename T>
  static Type combineSame(bits_t bits,
                          bits_t typeMask,
                          Ptr newPtrKind,
                          folly::Optional<T> aOpt,
                          folly::Optional<T> bOpt) {
    if (!bits) return Type::Bottom;

    // We shouldn't get here if neither is specialized.
    assert(aOpt || bOpt);

    // If we know both, attempt to combine them.
    if (aOpt && bOpt) {
      auto const a = *aOpt;
      auto const b = *bOpt;

      // When a and b are the same, keep the specialization.
      if (a == b) return Type(bits, newPtrKind, a);

      // If one is a subtype of the other, their intersection is the most
      // specific of the two.
      if (Ops::subtypeOf(a, b)) return Type(bits, newPtrKind, a);
      if (Ops::subtypeOf(b, a)) return Type(bits, newPtrKind, b);

      // If we can intersect the specializations, use that.
      if (Ops::canIntersect(a, b)) {
        if (auto info = Ops::intersect(a, b)) {
          return Type(bits, newPtrKind, *info);
        }
        // a and b are unrelated so we have to remove the specialized type.
        // This means dropping the specialization and the bits that correspond
        // to the type that was specialized.
        return Type(bits & ~typeMask, newPtrKind);
      }

      // We can't represent the true intersection of these two types, but
      // whatever the intersection is it must be smaller than one of the two.
      // It's not incorrect to just pick one arbitrarily, but we can pick the
      // "better" one by a Ops-specific heuristic.
      return Type(bits, newPtrKind, Ops::conservativeHeuristic(a, b));
    }

    if (aOpt) return Type(bits, newPtrKind, *aOpt);
    if (bOpt) return Type(bits, newPtrKind, *bOpt);

    not_reached();
  }

  static Type combineDifferent(bits_t newBits,
                               Ptr newPtrKind,
                               Type a,
                               Type b) {
    // Since a and b are each eligible for different specializations, their
    // intersection can't have any specialization left.
    return Type(newBits, newPtrKind);
  }
};

/*
 * combine handles the cases that have similar shapes between & and |: neither
 * is specialized or both have the same possible specialization type. Other
 * cases delegate back to Oper.
 */
template<typename Oper>
Type Type::combine(bits_t newBits, Ptr newPtrKind, Type a, Type b) {
  static_assert(std::is_same<Oper, Union>::value ||
                std::is_same<Oper, Intersect>::value,
                "Type::combine given unsupported template argument");

  // If neither type is specialized, the result is simple.
  if (LIKELY(!a.isSpecialized() && !b.isSpecialized())) {
    return Type(newBits, newPtrKind);
  }

  // If one of the types can't be specialized while the other is specialized,
  // preserve the specialization.
  if (!a.canSpecializeAny() || !b.canSpecializeAny()) {
    auto const specType = a.isSpecialized() ? a.specializedType()
                                            : b.specializedType();

    // If the specialized type doesn't exist in newBits, or newBits can be
    // specialized as an object or as an array, then drop the specialization.
    auto const either = (newBits & kAnyObj) && (newBits & kAnyArr);
    if ((newBits & specType.m_bits) && !either) {
      return Type(newBits, newPtrKind, specType.m_extra);
    }
    return Type(newBits, newPtrKind);
  }

  // If both types are eligible for the same kind of specialization and at
  // least one is specialized, delegate to Oper::combineSame.
  if (a.canSpecializeClass() && b.canSpecializeClass()) {
    folly::Optional<ClassInfo> aClass, bClass;
    if (a.getClass()) aClass = a.m_class;
    if (b.getClass()) bClass = b.m_class;

    return Oper::template combineSame<ClassOps>(
      newBits, kAnyObj, newPtrKind, aClass, bClass);
  }

  if (a.canSpecializeArray() && b.canSpecializeArray()) {
    folly::Optional<ArrayInfo> aInfo, bInfo;
    if (a.hasArrayKind() || a.getArrayType()) {
      aInfo = a.m_arrayInfo;
    }
    if (b.hasArrayKind() || b.getArrayType()) {
      bInfo = b.m_arrayInfo;
    }

    return Oper::template combineSame<ArrayOps>(
      newBits, kAnyArr, newPtrKind, aInfo, bInfo);
  }

  // The types are eligible for different kinds of specialization and at least
  // one is specialized, so delegate to Oper::combineDifferent.
  return Oper::combineDifferent(newBits, newPtrKind, a, b);
}

Type Type::operator|(Type b) const {
  auto a = *this;

  // Representing types like {Int<12>|Arr} could get messy and isn't useful in
  // practice, so unless we're unioning a constant type with itself or Bottom,
  // drop the constant value(s).
  if (a == b || b == Bottom) return a;
  if (a == Bottom) return b;

  a = a.dropConstVal();
  b = b.dropConstVal();

  auto const usePtrKind = [&] {
    // Handle cases where one of the types has no intersection with pointer
    // types.  We don't need to widen the resulting pointer kind at all in that
    // case.
    if (!a.maybe(Type::PtrToGen)) return b.rawPtrKind();
    if (!b.maybe(Type::PtrToGen)) return a.rawPtrKind();
    return ptr_union(a.rawPtrKind(), b.rawPtrKind());
  }();

  return combine<Union>(a.m_bits | b.m_bits, usePtrKind, a, b);
}

Type Type::operator&(Type b) const {
  auto a = *this;
  auto const newBits = a.m_bits & b.m_bits;

  // When intersecting a constant value with another type, the result will be
  // the constant value if the other value is a supertype of the constant, and
  // Bottom otherwise.
  if (a.m_hasConstVal) return a <= b ? a : Bottom;
  if (b.m_hasConstVal) return b <= a ? b : Bottom;

  bool const newIsPtr = newBits & Type::PtrToGen.m_bits;
  auto const pisect = ptr_isect(a.rawPtrKind(), b.rawPtrKind());
  if (!pisect) return Bottom;
  return combine<Intersect>(newBits, newIsPtr ? *pisect : Ptr::Unk, a, b);
}

Type Type::operator-(Type other) const {
  if (m_hasConstVal) {
    /*
     * If other is a constant of the same type, the result is Bottom or this
     * depending on whether or not it's the same constant. It is ok to use
     * m_bits == other.m_bits for this because m_hasConstVal implies only one
     * type bit is set.
     */
    if (other.m_bits == m_bits && other.m_hasConstVal) {
      return other.m_extra == m_extra ? Bottom : *this;
    }
    // Bits are different, and m_bits has a single bit set.

    /*
     * Now we're going to try to handle the case where other removes the whole
     * type that this is a constant of.  But we have to be careful because of
     * overlap between constants and specialized values.
     *
     * If the other type is neither constant nor a specialized array, we can
     * just check if removing that type removes the type of this's constant,
     * and return either this or Bottom depending on that.
     *
     * For the case of arrays, it only matters if this is a subtype of Array,
     * and in that case we can just potentially conservatively return this:
     * this is always as big as this - x for any x.
     */
    if (!other.m_hasConstVal &&
        (!other.isSpecialized() || !other.maybe(Arr) || !subtypeOf(Arr))) {
      return m_bits & ~other.m_bits ? *this : Bottom;
    }
    return *this;
  }

  // If the other value has a constant, but this doesn't, just (conservatively)
  // return this, rather than trying to represent things like "everything
  // except Int<24>".
  if (other.m_hasConstVal) return *this;

  // If we have pointers to different kinds of things, be conservative unless
  // other is an unknown pointer type.  Then we can just subtract the pointers
  // but keep our kind.
  if (rawPtrKind() != other.rawPtrKind()) {
    if (other.rawPtrKind() != Ptr::Unk) return *this;
  }
  auto const newPtrKind = rawPtrKind();

  auto const newBits = m_bits & ~other.m_bits;
  auto const spec1 = isSpecialized();
  auto const spec2 = other.isSpecialized();

  // The common easy case is when neither type is specialized.
  if (LIKELY(!spec1 && !spec2)) return Type(newBits, newPtrKind);

  if (spec1 && spec2) {
    if (canSpecializeClass() != other.canSpecializeClass()) {
      // Both are specialized but in different ways.  Take our specialization.
      return Type(newBits, newPtrKind, m_extra);
    }

    // If we got here, both types have the same kind of specialization (array
    // vs class).  We don't know how to deal with this yet, so just return
    // *this conservatively.
    return *this;
  }

  // If masking out other's bits removed all of the bits that correspond to our
  // specialization, take it out. Otherwise, preserve it.
  if (spec1) {
    if (canSpecializeClass()) {
      if (!(newBits & kAnyObj)) return Type(newBits, newPtrKind);
      return Type(newBits, newPtrKind, m_class);
    }
    if (canSpecializeArray()) {
      if (!(newBits & kAnyArr)) return Type(newBits, newPtrKind);
      return Type(newBits, newPtrKind, m_arrayInfo);
    }
    not_reached();
  }

  // Only other is specialized. This is where things get a little fuzzy. We
  // want to be able to support things like Obj - Obj<C> but we can't represent
  // Obj<~C>. We compromise and just return *this in cases like this, to make
  // sure we don't return a type that is too small.
  return *this;
}

bool Type::subtypeOfSpecialized(Type t2) const {
  assert((m_bits & t2.m_bits) == m_bits);
  assert(!t2.m_hasConstVal);
  assert(t2.isSpecialized());

  // Since t2 is specialized, we must either not be eligible for the same kind
  // of specialization (Int <= {Int|Arr<Packed>}) or have a specialization
  // that is a subtype of t2's specialization.
  if (t2.canSpecializeClass()) {
    if (!isSpecialized()) return false;

    //  Obj=A <:  Obj=A
    // Obj<=A <: Obj<=A
    if (m_class.isExact() == t2.m_class.isExact() &&
        getClass() == t2.getClass()) {
      return true;
    }

    //      A <: B
    // ----------------
    //  Obj=A <: Obj<=B
    // Obj<=A <: Obj<=B
    if (!t2.m_class.isExact()) return getClass()->classof(t2.getClass());
    return false;
  }

  assert(t2.canSpecializeArray());
  if (!canSpecializeArray()) return true;
  if (!isSpecialized()) return false;

  // Both types are specialized Arr types. "Specialized" in this context
  // means it has at least one of a RepoAuthType::Array* or (const ArrayData*
  // or ArrayData::ArrayKind). We may return false erroneously in cases where
  // a 100% accurate comparison of the specializations would be prohibitively
  // expensive.
  if (m_arrayInfo == t2.m_arrayInfo) return true;
  auto rat1 = getArrayType();
  auto rat2 = t2.getArrayType();

  if (rat1 != rat2 && !(rat1 && !rat2)) {
    // Different rats are only ok if rat1 is present and rat2 isn't. It's
    // possible for one rat to be a subtype of another rat or array kind, but
    // checking that can be very expensive.
    return false;
  }

  auto kind1 = getOptArrayKind();
  auto kind2 = t2.getOptArrayKind();
  assert(kind1 || kind2);
  if (kind1 && !kind2) return true;
  if (kind2 && !kind1) return false;
  if (*kind1 != *kind2) return false;

  // Same kinds but we still have to check for const arrays. a <= b iff they
  // have the same const array or a has a const array and b doesn't. If they
  // have the same non-nullptr const array the m_arrayInfo check up above
  // should've triggered.
  auto const1 = isConst() ? arrVal() : nullptr;
  auto const2 = t2.isConst() ? t2.arrVal() : nullptr;
  assert((!const1 && !const2) || const1 != const2);
  return const1 == const2 || (const1 && !const2);
}

Type liveTVType(const TypedValue* tv) {
  assert(tv->m_type == KindOfClass || tvIsPlausible(*tv));

  if (tv->m_type == KindOfObject) {
    Class* cls = tv->m_data.pobj->getVMClass();

    // We only allow specialization on classes that can't be
    // overridden for now. If this changes, then this will need to
    // specialize on sub object types instead.
    if (!cls || !(cls->attrs() & AttrNoOverride)) return Type::Obj;
    return Type::Obj.specializeExact(cls);
  }
  if (tv->m_type == KindOfArray) {
    return Type::Arr.specialize(tv->m_data.parr->kind());
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

  if (ty.subtypeOf(UncountedInit)) return Type::UncountedInit;
  if (ty.subtypeOf(Uncounted)) return Type::Uncounted;
  if (ty.subtypeOf(Cell)) return Type::Cell;
  if (ty.subtypeOf(BoxedCell)) return Type::BoxedCell;
  if (ty.subtypeOf(Gen)) return Type::Gen;
  not_reached();
}

//////////////////////////////////////////////////////////////////////

namespace {

Type setElemReturn(const IRInstruction* inst) {
  assert(inst->op() == SetElem || inst->op() == SetElemStk);
  auto baseType = inst->src(minstrBaseIdx(inst->op()))->type().strip();

  // If the base is a Str, the result will always be a CountedStr (or
  // an exception). If the base might be a str, the result wil be
  // CountedStr or Nullptr. Otherwise, the result is always Nullptr.
  if (baseType.subtypeOf(Type::Str)) {
    return Type::CountedStr;
  } else if (baseType.maybe(Type::Str)) {
    return Type::CountedStr | Type::Nullptr;
  }
  return Type::Nullptr;
}

Type builtinReturn(const IRInstruction* inst) {
  assert(inst->op() == CallBuiltin);

  Type t = inst->typeParam();
  if (t.isSimpleType() || t.equals(Type::Cell)) {
    return t;
  }
  if (t.isReferenceType() || t.equals(Type::BoxedCell)) {
    return t | Type::InitNull;
  }
  not_reached();
}

Type stkReturn(const IRInstruction* inst, int dstId,
               std::function<Type()> inner) {
  assert(inst->modifiesStack());
  if (dstId == 0 && inst->hasMainDst()) {
    // Return the type of the main dest (if one exists) as dst 0
    return inner();
  }
  // The instruction modifies the stack and this isn't the main dest,
  // so it's a StkPtr.
  return Type::StkPtr;
}

Type thisReturn(const IRInstruction* inst) {
  auto const func = inst->marker().func();

  // If the function is a cloned closure which may have a re-bound $this which
  // is not a subclass of the context return an unspecialized type.
  if (func->hasForeignThis()) return Type::Obj;

  if (auto const cls = func->cls()) {
    return Type::Obj.specialize(cls);
  }
  return Type::Obj;
}

Type allocObjReturn(const IRInstruction* inst) {
  switch (inst->op()) {
    case ConstructInstance:
      return Type::Obj.specialize(inst->extra<ConstructInstance>()->cls);

    case NewInstanceRaw:
      return Type::Obj.specializeExact(inst->extra<NewInstanceRaw>()->cls);

    case AllocObj:
      return inst->src(0)->isConst()
        ? Type::Obj.specializeExact(inst->src(0)->clsVal())
        : Type::Obj;

    default:
      always_assert(false && "Invalid opcode returning AllocObj");
  }
}

Type arrElemReturn(const IRInstruction* inst) {
  assert(inst->op() == LdPackedArrayElem);
  auto const tyParam = inst->hasTypeParam() ? inst->typeParam() : Type::Gen;
  assert(!inst->hasTypeParam() || inst->typeParam() <= Type::Gen);

  auto const arrTy = inst->src(0)->type().getArrayType();
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
  return t.deref().unbox().box().ptr(remove_ref(t.ptrKind()));
}

}

Type ldRefReturn(Type typeParam) {
  assert(typeParam.notBoxed());
  // Guarding on specialized types and uncommon unions like {Int|Bool} is
  // expensive enough that we only want to do it in situations where we've
  // manually confirmed the benefit.

  if (typeParam.strictSubtypeOf(Type::Obj) &&
      typeParam.isSpecialized() &&
      typeParam.getClass()->attrs() & AttrFinal &&
      typeParam.getClass()->isCollectionClass()) {
    /*
     * This case is needed for the minstr-translator.
     * see MInstrTranslator::checkMIState().
     */
    return typeParam;
  }

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
  if (t.subtypeOf(Type::Str)) {
    t = Type::Str;
  } else if (t.subtypeOf(Type::Arr)) {
    t = Type::Arr;
  }
  // When boxing an Object, if the inner class does not have AttrNoOverride,
  // drop the class specialization.
  if (t < Type::Obj && !(t.getClass()->attrs() & AttrNoOverride)) {
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
  case T::Ref:            return Type::BoxedCell;
  case T::InitUnc:        return Type::UncountedInit;
  case T::Unc:            return Type::Uncounted;
  case T::InitCell:       return Type::InitCell;
  case T::InitGen:        return Type::Init;
  case T::Gen:            return Type::Gen;

  // TODO(#4205897): option specialized array types
  case T::OptArr:         return Type::Arr       | Type::InitNull;
  case T::OptSArr:        return Type::StaticArr | Type::InitNull;

  case T::SArr:
    if (auto const ar = ty.array()) return Type::StaticArr.specialize(ar);
    return Type::StaticArr;
  case T::Arr:
    if (auto const ar = ty.array()) return Type::Arr.specialize(ar);
    return Type::Arr;

  case T::SubObj:
  case T::ExactObj: {
    auto const base = Type::Obj;
    if (auto const cls = Unit::lookupUniqueClass(ty.clsName())) {
      return ty.tag() == T::ExactObj ?
        base.specializeExact(cls) :
        base.specialize(cls);
    }
    return base;
  }
  case T::OptSubObj:
  case T::OptExactObj: {
    auto const base = Type::Obj | Type::InitNull;
    if (auto const cls = Unit::lookupUniqueClass(ty.clsName())) {
      return ty.tag() == T::OptExactObj ?
        base.specializeExact(cls) :
        base.specialize(cls);
    }
    return base;
  }
  }
  not_reached();
}

Type refineTypeNoCheck(Type oldType, Type newType) {
  return oldType & newType;
}

Type refineType(Type oldType, Type newType) {
  Type result = refineTypeNoCheck(oldType, newType);
  always_assert_flog(result != Type::Bottom,
                     "refineType({}, {}) failed", oldType, newType);
  return result;
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
#define DBox(n)         return Type::BoxedInitCell;
#define DRefineS(n)     return refineTypeNoCheck(inst->src(n)->type(), \
                                                 inst->typeParam());
#define DParamMayRelax  return inst->typeParam();
#define DParam          return inst->typeParam();
#define DParamPtr(k)    assert(inst->typeParam() <= Type::Gen.ptr(Ptr::k)); \
                        return inst->typeParam();
#define DUnboxPtr       return unboxPtr(inst->src(0)->type());
#define DBoxPtr         return boxPtr(inst->src(0)->type());
#define DAllocObj       return allocObjReturn(inst);
#define DArrElem        return arrElemReturn(inst);
#define DArrPacked      return Type::Arr.specialize(ArrayData::kPackedKind);
#define DThis           return thisReturn(inst);
#define DMulti          return Type::Bottom;
#define DStk(in)        return stkReturn(inst, dstId, \
                                   [&]() -> Type { in not_reached(); });
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
#undef DBox
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
#undef DStk
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
    if (unit) print(*unit);
    always_assert(false && "instruction operand type check failure");
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
    if (inst->hasTypeParam()) {
      checkDst(inst->typeParam() != Type::Bottom,
             "Invalid paramType for DParam instruction");
    }
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
                        Type t = Type::Arr.specialize(                  \
                          ArrayData::k##kind##Kind);                    \
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
#define DStk(...)
#define DSetElem
#define D(...)
#define DBuiltin
#define DSubtract(src, t)checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DBox(src)   checkDst(src < inst->numSrcs(),  \
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
#undef DStk
#undef DSetElem
#undef DBox
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

std::string TypeConstraint::toString() const {
  std::string ret = "<" + typeCategoryName(category);

  if (category == DataTypeSpecialized) {
    if (wantArrayKind()) ret += ",ArrayKind";
    if (wantClass()) {
      folly::toAppend("Cls:", desiredClass()->name()->data(), &ret);
    }
  }

  if (weak) ret += ",weak";

  return ret + '>';
}

//////////////////////////////////////////////////////////////////////

}}
