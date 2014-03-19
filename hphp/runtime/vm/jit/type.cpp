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

#include "folly/Conv.h"
#include "folly/Format.h"
#include "folly/MapUtil.h"
#include "folly/gen/Base.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/runtime-type.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator.h"

#include <vector>

namespace HPHP {  namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

#define IRT(name, ...) const Type Type::name(Type::k##name);
IR_TYPES
#undef IRT

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
  IR_TYPES
# undef IRT

  if (isBoxed()) {
    return folly::to<std::string>("Boxed", innerType().toString());
  }
  if (isPtr()) {
    auto ret = folly::to<std::string>("PtrTo", deref().toString());
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
      assert(m_class);
      parts.push_back(folly::to<std::string>(Type(m_bits & kAnyObj).toString(),
                                             '<', m_class->name()->data(),
                                             '>'));
      t -= AnyObj;
    } else if (canSpecializeArrayKind()) {
      assert(hasArrayKind());
      parts.push_back(
        folly::to<std::string>(Type(m_bits & kAnyArr).toString(), '<',
                               ArrayData::kindToString(getArrayKind()), '>'));
      t -= AnyArr;
    } else {
      not_reached();
    }
  }

  // Concat all of the primitive types in the custom union type
# define IRT(name, ...) if (name <= t) parts.push_back(#name);
  IRT_PRIMITIVE
# undef IRT
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
  static_assert(sizeof(m_arrayKind) == 1,
                "Type expects ArrayKind to be one byte");
  if (m_extra) {
    assert((!(m_bits & kAnyObj) || !(m_bits & kAnyArr)) &&
           "Conflicting specialization");

    if (canSpecializeArrayKind() && hasArrayKind() && !m_hasConstVal) {
      assert(!(m_extra & 0xffffffffffff0000) &&
             "Non-zero padding bits in Type with array kind");
    }
  }

  return true;
}

Type Type::unionOf(Type t1, Type t2) {
  if (t1 == t2 || t2 < t1) return t1;
  if (t1 < t2) return t2;
  static const Type union_types[] = {
#   define IRT(name, ...) name,
    IRT_PHP_UNIONS(IRT_BOXES)
#   undef IRT
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
  if (isBoxed()) {
    return KindOfRef;
  }

  // Order is important here: types must progress from more specific
  // to less specific to return the most specific DataType.
  if (subtypeOf(Uninit))        return KindOfUninit;
  if (subtypeOf(Null))          return KindOfNull;
  if (subtypeOf(Bool))          return KindOfBoolean;
  if (subtypeOf(Int))           return KindOfInt64;
  if (subtypeOf(Dbl))           return KindOfDouble;
  if (subtypeOf(StaticStr))     return KindOfStaticString;
  if (subtypeOf(Str))           return KindOfString;
  if (subtypeOf(Arr))           return KindOfArray;
  if (subtypeOf(Obj))           return KindOfObject;
  if (subtypeOf(Res))           return KindOfResource;
  if (subtypeOf(Cls))           return KindOfClass;
  if (subtypeOf(UncountedInit)) return KindOfUncountedInit;
  if (subtypeOf(Uncounted))     return KindOfUncounted;
  if (subtypeOf(Gen))           return KindOfAny;
  not_reached();
}

RuntimeType Type::toRuntimeType() const {
  assert(!isPtr());
  auto const outer = isBoxed() ? KindOfRef : toDataType();
  auto const inner = isBoxed() ? innerType().toDataType() : KindOfNone;
  auto rtt = RuntimeType{outer, inner};

  if (isSpecialized()) {
    if (subtypeOf(Type::Arr)) {
      return rtt.setArrayKind(getArrayKind());
    } else if (subtypeOf(Type::Obj)) {
      return rtt.setKnownClass(getClass());
    }
  }

  return rtt;
}

Type::Type(const RuntimeType& rtt)
  : m_bits(bitsFromDataType(rtt.outerType(), rtt.innerType()))
  , m_hasConstVal(false)
  , m_class(nullptr)
{
  if (rtt.outerType() == KindOfObject && rtt.hasKnownClass()) {
    m_class = rtt.knownClass();
  } else if (rtt.outerType() == KindOfArray && rtt.hasArrayKind()) {
    m_arrayKindValid = true;
    m_arrayKind = rtt.arrayKind();
  }
}

Type::Type(const DynLocation* dl)
  : Type(dl->rtt)
{}

Type::bits_t Type::bitsFromDataType(DataType outer, DataType inner) {
  assert(outer != KindOfInvalid);
  assert(inner != KindOfRef);
  assert(IMPLIES(inner == KindOfNone, outer != KindOfRef));

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
    case KindOfUncountedInit : return kUncountedInit;
    case KindOfUncounted     : return kUncounted;
    case KindOfAny           : return kGen;
    case KindOfRef: {
      if (inner == KindOfAny) {
        return kBoxedCell;
      } else {
        assert(inner != KindOfUninit);
        return bitsFromDataType(inner, KindOfNone) << kBoxShift;
      }
    }
    default                  : always_assert(false && "Unsupported DataType");
  }
}

namespace {
// ClassOps and ArrayOps are used below to write code that can perform set
// operations on both Class and ArrayKind specializations.
struct ClassOps {
  static bool subtypeOf(const Class* a, const Class* b) {
    return a->classof(b);
  }

  static folly::Optional<const Class*> commonAncestor(const Class* a,
                                                      const Class* b) {
    if (!isNormalClass(a) || !isNormalClass(b)) return folly::none;
    if (auto result = a->commonAncestor(b)) return result;

    return folly::none;
  }
};

struct ArrayOps {
  static bool subtypeOf(ArrayData::ArrayKind a, ArrayData::ArrayKind b) {
    return a == b;
  }

  static folly::Optional<ArrayData::ArrayKind> commonAncestor(
    ArrayData::ArrayKind a, ArrayData::ArrayKind b) {
    if (a == b) return a;
    return folly::none;
  }
};
}

// Union and Intersect implement part of the logic for operator| and operator&,
// respectively. Each has two static methods:
//
// combineSame: called when at least one of *this or b is specialized and
//              they can both specialize on the same type.
// combineDifferent: called when *this and b can specialize different ways
//                   and at least one of the two is specialized.

struct Type::Union {
  template<typename Ops, typename T>
  static Type combineSame(bits_t bits, bits_t typeMask,
                          folly::Optional<T> aOpt,
                          folly::Optional<T> bOpt) {
    // If one or both types are not specialized, the specialization is lost
    if (!(aOpt && bOpt)) return Type(bits);

    auto const a = *aOpt;
    auto const b = *bOpt;

    // If the specialization is the same, keep it.
    if (a == b)            return Type(bits, a);

    // If one is a subtype of the other, their union is the least specific of
    // the two.
    if (Ops::subtypeOf(a, b))     return Type(bits, b);
    if (Ops::subtypeOf(b, a))     return Type(bits, a);

    // Check for a common ancestor.
    if (auto p = Ops::commonAncestor(a, b)) return Type(bits, *p);

    // a and b are unrelated but we can't hold both of them in a Type. Dropping
    // the specialization returns a supertype of their true union. It's not
    // optimal but not incorrect.
    return Type(bits);
  }

  static Type combineDifferent(bits_t newBits, Type a, Type b) {
    // a and b can specialize differently, so their union can't have any
    // specialization (it would be an ambiguously specialized type).
    return Type(newBits);
  }
};

struct Type::Intersect {
  template<typename Ops, typename T>
  static Type combineSame(bits_t bits, bits_t typeMask,
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
      if (a == b)        return Type(bits, a);

      // If one is a subtype of the other, their intersection is the most
      // specific of the two.
      if (Ops::subtypeOf(a, b)) return Type(bits, a);
      if (Ops::subtypeOf(b, a)) return Type(bits, b);

      // a and b are unrelated so we have to remove the specialized type. This
      // means dropping the specialization and the bits that correspond to the
      // type that was specialized.
      return Type(bits & ~typeMask);
    }

    if (aOpt) return Type(bits, *aOpt);
    if (bOpt) return Type(bits, *bOpt);

    not_reached();
  }

  static Type combineDifferent(bits_t newBits, Type a, Type b) {
    // Since a and b are each eligible for different specializations, their
    // intersection can't have any specialization left.
    return Type(newBits);
  }
};

/*
 * combine handles the cases that have similar shapes between & and |: neither
 * is specialized or both have the same possible specialization type. Other
 * cases delegate back to Oper.
 */
template<typename Oper>
Type Type::combine(bits_t newBits, Type a, Type b) {
  static_assert(std::is_same<Oper, Union>::value ||
                std::is_same<Oper, Intersect>::value,
                "Type::combine given unsupported template argument");

  // If neither type is specialized, the result is simple.
  if (LIKELY(!a.isSpecialized() && !b.isSpecialized())) {
    return Type(newBits);
  }

  // If one of the types can't be specialized while the other is specialized,
  // preserve the specialization.
  if (!a.canSpecializeAny() || !b.canSpecializeAny()) {
    auto const specType = a.isSpecialized() ? a.specializedType()
                                            : b.specializedType();

    // If the specialized type doesn't exist in newBits, drop the
    // specialization.
    if (newBits & specType.m_bits) return Type(newBits, specType.m_extra);
    return Type(newBits);
  }

  // If both types are eligible for the same kind of specialization and at
  // least one is specialized, delegate to Oper::combineSame.
  if (a.canSpecializeClass() && b.canSpecializeClass()) {
    folly::Optional<const Class*> aClass, bClass;
    if (a.getClass()) aClass = a.getClass();
    if (b.getClass()) bClass = b.getClass();

    return Oper::template combineSame<ClassOps>(newBits, kAnyObj,
                                                aClass, bClass);
  }

  if (a.canSpecializeArrayKind() && b.canSpecializeArrayKind()) {
    folly::Optional<ArrayData::ArrayKind> aKind, bKind;
    if (a.hasArrayKind()) aKind = a.getArrayKind();
    if (b.hasArrayKind()) bKind = b.getArrayKind();

    return Oper::template combineSame<ArrayOps>(newBits, kAnyArr, aKind, bKind);
  }

  // The types are eligible for different kinds of specialization and at least
  // one is specialized, so delegate to Oper::combineDifferent.
  return Oper::combineDifferent(newBits, a, b);
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

  return combine<Union>(a.m_bits | b.m_bits, a, b);
}

Type Type::operator&(Type b) const {
  auto a = *this;
  auto const newBits = a.m_bits & b.m_bits;

  // When intersecting a constant value with another type, the result will be
  // the constant value if the other value is a supertype of the constant, and
  // Bottom otherwise.
  if (a.m_hasConstVal) return a <= b ? a : Bottom;
  if (b.m_hasConstVal) return b <= a ? b : Bottom;

  return combine<Intersect>(newBits, a, b);
}

Type Type::operator-(Type other) const {
  auto const newBits = m_bits & ~other.m_bits;

  if (m_hasConstVal) {
    // If other is a constant of the same type, the result is Bottom or this
    // depending on whether or not it's the same constant.
    if (other.m_bits == m_bits && other.m_hasConstVal) {
      return other.m_extra == m_extra ? Bottom : *this;
    }

    // Otherwise, just check to see if the constant's type was removed in
    // newBits.
    return (newBits & m_bits) ? *this : Bottom;
  }

  // Rather than try to represent types like "all Ints except 24", treat t -
  // Int<24> as t - Int.
  other = other.dropConstVal();

  auto const spec1 = isSpecialized();
  auto const spec2 = other.isSpecialized();

  // The common easy case is when neither type is specialized.
  if (LIKELY(!spec1 && !spec2)) return Type(newBits);

  if (spec1 && spec2) {
    if (canSpecializeClass() != other.canSpecializeClass()) {
      // Both are specialized but in different ways. Our specialization is
      // preserved.
      return Type(newBits, m_extra);
    }

    // Subtracting different specializations of the same type could get messy
    // so we don't support it for now.
    always_assert(specializedType() == other.specializedType() &&
                  "Incompatible specialized types given to operator-");

    // If we got here, both types have the same specialization, so it's removed
    // from the result.
    return Type(newBits);
  }

  // If masking out other's bits removed all of the bits that correspond to our
  // specialization, take it out. Otherwise, preserve it.
  if (spec1) {
    if (canSpecializeClass()) {
      if (!(newBits & kAnyObj)) return Type(newBits);
      return Type(newBits, m_class);
    }
    if (canSpecializeArrayKind()) {
      if (!(newBits & kAnyArr)) return Type(newBits);
      return Type(newBits, getArrayKind());
    }
    not_reached();
  }

  // Only other is specialized. This is where things get a little fuzzy. We
  // want to be able to support things like Obj - Obj<C> but we can't represent
  // Obj<~C>. We compromise and return Bottom in cases like this, which means
  // we need to be careful because (a - b) == Bottom doesn't imply a <= b in
  // this world.
  if (other.canSpecializeClass()) return Type(newBits & ~kAnyObj);
  return Type(newBits & ~kAnyArr);
}

Type liveTVType(const TypedValue* tv) {
  assert(tv->m_type == KindOfClass || tvIsPlausible(*tv));

  if (tv->m_type == KindOfObject) {
    Class* cls = tv->m_data.pobj->getVMClass();
    // We only allow specialization on final classes for now.
    if (cls && !(cls->attrs() & AttrFinal)) cls = nullptr;
    return Type::Obj.specialize(cls);
  }
  if (tv->m_type == KindOfArray) {
    return Type::Arr.specialize(tv->m_data.parr->kind());
  }

  auto outer = tv->m_type;
  auto inner = KindOfInvalid;

  if (outer == KindOfStaticString) outer = KindOfString;
  if (outer == KindOfRef) {
    inner = tv->m_data.pref->tv()->m_type;
    if (inner == KindOfStaticString) inner = KindOfString;
  }
  return Type(outer, inner);
}

//////////////////////////////////////////////////////////////////////

namespace {

Type setElemReturn(const IRInstruction* inst) {
  assert(inst->op() == SetElem || inst->op() == SetElemStk);
  auto baseType = inst->src(minstrBaseIdx(inst))->type().strip();

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

Type ldRefReturn(const IRInstruction* inst) {
  // Guarding on specialized types and uncommon unions like {Int|Bool} is
  // expensive enough that we only want to do it in situations where we've
  // manually confirmed the benefit.
  auto type = inst->typeParam().unspecialize();

  if (type.isKnownDataType())      return type;
  if (type <= Type::UncountedInit) return Type::UncountedInit;
  if (type <= Type::Uncounted)     return Type::Uncounted;
  always_assert(type <= Type::Cell);
  return Type::Cell;
}

Type thisReturn(const IRInstruction* inst) {
  auto fpInst = inst->src(0)->inst();

  // Find the instruction that created the current frame and grab the context
  // class from it. $this, if present, is always going to be the context class
  // or a subclass of the context.
  while (!fpInst->is(DefFP, DefInlineFP)) {
    assert(fpInst->dst()->isA(Type::FramePtr));
    assert(fpInst->is(GuardLoc, CheckLoc, AssertLoc, SideExitGuardLoc, PassFP));
    fpInst = fpInst->src(0)->inst();
  }
  auto const func = fpInst->is(DefFP) ? fpInst->marker().func
                                      : fpInst->extra<DefInlineFP>()->target;
  func->validate();
  assert(func->isMethod() || func->isPseudoMain());
  return Type::Obj.specialize(func->cls());
}

Type allocObjReturn(const IRInstruction* inst) {
  switch (inst->op()) {
    case ConstructInstance:
      return Type::Obj.specialize(inst->extra<ConstructInstance>()->cls);
    case NewInstanceRaw:
      return Type::Obj.specialize(inst->extra<NewInstanceRaw>()->cls);
    case CustomInstanceInit:
    case AllocObj:
      return inst->src(0)->isConst()
        ? Type::Obj.specialize(inst->src(0)->clsVal())
        : Type::Obj;
    default:
      always_assert(false && "Invalid opcode returning AllocObj");
  }
}

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
  // Everything else is just a pure type-system boxing operation.
  return t.box();
}

Type convertToType(RepoAuthType ty) {
  using T = RepoAuthType::Tag;
  switch (ty.tag()) {
  case T::OptBool:        return Type::Bool      | Type::InitNull;
  case T::OptInt:         return Type::Int       | Type::InitNull;
  case T::OptSArr:        return Type::StaticArr | Type::InitNull;
  case T::OptArr:         return Type::Arr       | Type::InitNull;
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
  case T::SArr:           return Type::StaticArr;
  case T::Arr:            return Type::Arr;
  case T::Obj:            return Type::Obj;

  case T::Cell:           return Type::Cell;
  case T::Ref:            return Type::BoxedCell;
  case T::InitUnc:        return Type::UncountedInit;
  case T::Unc:            return Type::Uncounted;
  case T::InitCell:       return Type::InitCell;
  case T::InitGen:        return Type::Init;
  case T::Gen:            return Type::Gen;

  case T::SubObj:
  case T::ExactObj:
    {
      if (auto const cls = Unit::lookupUniqueClass(ty.clsName())) {
        return Type::Obj.specialize(cls);
      }
      return Type::Obj;
    }
  case T::OptSubObj:
  case T::OptExactObj:
    {
      if (auto const cls = Unit::lookupUniqueClass(ty.clsName())) {
        return Type::Obj.specialize(cls) | Type::InitNull;
      }
      return Type::Obj | Type::InitNull;
    }
  }
  not_reached();
}

Type outputType(const IRInstruction* inst, int dstId) {
#define IRT(name, ...) UNUSED static const Type name = Type::name;
  IR_TYPES
#undef IRT

#define D(type)   return type;
#define DofS(n)   return inst->src(n)->type();
#define DUnbox(n) return inst->src(n)->type().unbox();
#define DBox(n)   return boxType(inst->src(n)->type());
#define DFilterS(n) return inst->src(n)->type() & inst->typeParam();
#define DParam    return inst->typeParam();
#define DAllocObj return allocObjReturn(inst);
#define DLdRef    return ldRefReturn(inst);
#define DThis     return thisReturn(inst);
#define DMulti    return Type::Bottom;
#define DStk(in)  return stkReturn(inst, dstId,                         \
                                   [&]() -> Type { in not_reached(); });
#define DSetElem  return setElemReturn(inst);
#define ND        assert(0 && "outputType requires HasDest or NaryDest");
#define DBuiltin  return builtinReturn(inst);
#define DSubtract(n, t) return inst->src(n)->type() - t;
#define DLdRaw    return inst->extra<RawMemData>()->info().type;

#define O(name, dstinfo, srcinfo, flags) case name: dstinfo not_reached();

  switch (inst->op()) {
  IR_OPCODES
  default: not_reached();
  }

#undef O

#undef D
#undef DofS
#undef DUnbox
#undef DBox
#undef DFilterS
#undef DParam
#undef DAllocObj
#undef DLdRef
#undef DThis
#undef DMulti
#undef DStk
#undef DSetElem
#undef ND
#undef DBuiltin
#undef DSubtract
#undef DLdRaw

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
 * This is generated using the table in ir.h.  We instantiate
 * IR_OPCODES after defining all the various source forms to do type
 * assertions according to their form (see ir.h for documentation on
 * the notation).  The checkers appear in argument order, so each one
 * increments curSrc, and at the end we can check that the argument
 * count was also correct.
 */
void assertOperandTypes(const IRInstruction* inst) {
  if (!debug) return;

  int curSrc = 0;

  auto bail = [&] (const std::string& msg) {
    FTRACE(1, "{}", msg);
    fprintf(stderr, "%s\n", msg.c_str());
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
    if (cond) return;

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
  };

  auto checkNoArgs = [&]{
    if (inst->numSrcs() == 0) return;
    bail(folly::format(
      "Error: instruction expected no operands\n"
      "   instruction: {}\n",
        inst->toString()
      ).str()
    );
  };

  auto countCheck = [&]{
    if (inst->numSrcs() == curSrc) return;
    bail(folly::format(
      "Error: instruction had too many operands\n"
      "   instruction: {}\n"
      "   expected {} arguments\n",
        inst->toString(),
        curSrc
      ).str()
    );
  };

  auto checkDst = [&] (bool cond, const std::string& errorMessage) {
    if (cond) return;

    bail(folly::format("Error: failed type check on dest operand\n"
                       "   instruction: {}\n"
                       "   message: {}\n",
                       inst->toString(),
                       errorMessage).str());
  };

  auto requireTypeParam = [&] {
    checkDst(inst->hasTypeParam() || inst->is(DefConst),
             "Invalid paramType for DParam instruction");
    if (inst->hasTypeParam()) {
      checkDst(inst->typeParam() != Type::Bottom,
             "Invalid paramType for DParam instruction");
    }
  };

  auto checkSpills = [&] {
    for (; curSrc < inst->numSrcs(); ++curSrc) {
      auto const valid = (inst->src(curSrc)->type() <= Type::StackElem);
      check(valid, Type(), "Gen|Cls");
    }
  };

#define IRT(name, ...) UNUSED static const Type name = Type::name;
  IR_TYPES
#undef IRT

#define NA       return checkNoArgs();
#define S(...)   {                                        \
                   Type t = buildUnion(__VA_ARGS__);      \
                   check(src()->isA(t), t, nullptr);      \
                   ++curSrc;                              \
                 }
#define C(type)  check(src()->isConst() &&          \
                       src()->isA(type),            \
                       Type(),                      \
                       "constant " #type);          \
                  ++curSrc;
#define CStr     C(StaticStr)
#define SUnk     return;
#define SSpills  checkSpills();
#define ND
#define DMulti
#define DStk(...)
#define DSetElem
#define D(...)
#define DBuiltin
#define DSubtract(src, t)checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DUnbox(src) checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DBox(src)   checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DofS(src)   checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DFilterS(src) checkDst(src < inst->numSrcs(),  \
                               "invalid src num");     \
                      requireTypeParam();
#define DParam      requireTypeParam();
#define DLdRef      requireTypeParam();
#define DAllocObj
#define DThis
#define DLdRaw

#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode: dstinfo srcinfo countCheck(); return;

  switch (inst->op()) {
    IR_OPCODES
  default: assert(0);
  }

#undef O

#undef NA
#undef S
#undef C
#undef CStr
#undef SUnk
#undef SSpills

#undef ND
#undef D
#undef DBuiltin
#undef DSubtract
#undef DUnbox
#undef DMulti
#undef DStk
#undef DSetElem
#undef DBox
#undef DofS
#undef DFilterS
#undef DParam
#undef DAllocObj
#undef DLdRef
#undef DThis
#undef DLdRaw

}

std::string TypeConstraint::toString() const {
  std::string catStr = typeCategoryName(category);

  if (innerCat > DataTypeGeneric) {
    folly::toAppend(",inner:", typeCategoryName(innerCat), &catStr);
  }

  return folly::format("<{},{}>", catStr, assertedType).str();
}

//////////////////////////////////////////////////////////////////////

}}
