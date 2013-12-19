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

#include "folly/Conv.h"
#include "folly/Format.h"
#include "folly/experimental/Gen.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/runtime-type.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator.h"


namespace HPHP {  namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

#define IRT(name, ...) const Type Type::name(Type::k##name);
IR_TYPES
#undef IRT

std::string Type::toString() const {
  // Try to find an exact match to a predefined type
# define IRT(name, ...) if (*this == name) return #name;
  IR_TYPES
# undef IRT

  if (isBoxed()) {
    return folly::to<std::string>("Boxed", innerType().toString());
  }
  if (isPtr()) {
    return folly::to<std::string>("PtrTo", deref().toString());
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
                               ArrayData::kindToString(m_arrayKind), '>'));
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

Type Type::fromString(const std::string& str) {
  static hphp_string_map<Type> types;
  static bool init = false;
  if (UNLIKELY(!init)) {
#   define IRT(name, ...) types[#name] = name;
    IR_TYPES
#   undef IRT
    init = true;
  }
  return mapGet(types, str, Type::None);
}

bool Type::checkValid() const {
  static_assert(sizeof(m_arrayKind) == 1,
                "Type expects ArrayKind to be one byte");
  if (m_extra) {
    assert((!(m_bits & kAnyObj) || !(m_bits & kAnyArr)) &&
           "Conflicting specialization");

    if (canSpecializeArrayKind() && hasArrayKind()) {
      assert(!(m_extra & 0xffffffffffff0000) &&
             "Non-zero padding bits in Type with array kind");
    }
  }

  return true;
}

DataType Type::toDataType() const {
  assert(!isPtr());
  if (isBoxed()) {
    return KindOfRef;
  }

  // Order is important here: types must progress from more specific
  // to less specific to return the most specific DataType.
  if (subtypeOf(None))          return KindOfNone;
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
    if (isArray()) {
      return rtt.setArrayKind(getArrayKind());
    } else {
      return rtt.setKnownClass(getClass());
    }
  }

  return rtt;
}

Type::Type(const RuntimeType& rtt)
  : m_bits(bitsFromDataType(rtt.outerType(), rtt.innerType()))
  , m_class(nullptr)
{
  if (rtt.outerType() == KindOfObject && rtt.hasKnownClass()) {
    m_class = rtt.knownClass();
  } else if (rtt.outerType() == KindOfArray && rtt.hasArrayKind()) {
    m_arrayKindValid = true;
    m_arrayKind = rtt.arrayKind();
  }
}

Type::Type(const DynLocation* dl) {
  // Temporary stop-gap until we embrace gcc 4.8 fully, and only ok because
  // Type has no non-POD members. At that point we can switch to ctor
  // delegation.
  new (this) Type((assert(dl), dl->rtt));
}

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
// combineClass: called when at least one of *this or b is specialized and
//               they can both specialize on Class.
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

  /*
   * combineExtra returns a's m_extra field if the union of a and b would
   * contain a's specialization. Otherwise it returns 0. Assumes a and b can
   * specialize differently.
   */
  static uintptr_t combineExtra(Type a, Type b) {
    if (!a.isSpecialized()) return 0;
    assert(a.canSpecializeClass() != b.canSpecializeClass());

    if (a.getClass()) {
      // We know b can specialize on array kind so it can't contain any members
      // of AnyObj. a's class will be preserved in the union.
      assert(!(b.m_bits & kAnyObj));
      return a.m_extra;
    }

    if (a.hasArrayKind()) {
      // We know b can specialize on object class so it can't contain any members
      // of AnyArr. a's array kind will be preserved in the union.
      assert(!(b.m_bits & kAnyArr));
      return a.m_extra;
    }

    not_reached();
  }

  static Type combineDifferent(bits_t newBits, Type a, Type b) {
    // a and b can specialize differently. Figure out if each Type's
    // specialization would be preserved in the union operation, sanity check,
    // and keep the specialization that survived.
    auto const aExtra = combineExtra(a, b);
    auto const bExtra = combineExtra(b, a);

    assert(!(aExtra && bExtra) && "Conflicting specializations in operator|");
    return Type(newBits, aExtra ? aExtra : bExtra);
  }
};

struct Type::Intersect {
  template<typename Ops, typename T>
  static Type combineSame(bits_t bits, bits_t typeMask,
                          folly::Optional<T> aOpt,
                          folly::Optional<T> bOpt) {
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
Type Type::combine(bits_t newBits, Type other) const {
  static_assert(std::is_same<Oper, Union>::value ||
                std::is_same<Oper, Intersect>::value,
                "Type::combine given unsupported template argument");

  // If neither type is specialized, the result is simple.
  if (LIKELY(!isSpecialized() && !other.isSpecialized())) {
    return Type(newBits);
  }

  // If one of the types can't be specialized while the other is specialized,
  // preserve the specialization.
  if (!canSpecializeAny() || !other.canSpecializeAny()) {
    auto const specType = isSpecialized() ? specializedType()
                                          : other.specializedType();

    // If the specialized type doesn't exist in newBits, drop the
    // specialization.
    if (newBits & specType.m_bits) return Type(newBits, specType.m_extra);
    return Type(newBits);
  }

  // If both types are eligible for the same kind of specialization and at
  // least one is specialized, delegate to Oper::combineSame.
  if (canSpecializeClass() && other.canSpecializeClass()) {
    folly::Optional<const Class*> aClass, bClass;
    if (getClass()) aClass = getClass();
    if (other.getClass()) bClass = other.getClass();

    return Oper::template combineSame<ClassOps>(newBits, kAnyObj,
                                                aClass, bClass);
  }

  if (canSpecializeArrayKind() && other.canSpecializeArrayKind()) {
    folly::Optional<ArrayData::ArrayKind> aKind, bKind;
    if (hasArrayKind()) aKind = getArrayKind();
    if (other.hasArrayKind()) bKind = other.getArrayKind();

    return Oper::template combineSame<ArrayOps>(newBits, kAnyArr, aKind, bKind);
  }

  // The types are eligible for different kinds of specialization and at least
  // one is specialized, so delegate to Oper::combineDifferent.
  return Oper::combineDifferent(newBits, *this, other);
}

Type Type::operator|(Type other) const {
  auto const newBits = m_bits | other.m_bits;
  return combine<Union>(newBits, other);
}
Type Type::operator&(Type other) const {
  auto const newBits = m_bits & other.m_bits;
  return combine<Intersect>(newBits, other);
}

Type Type::operator-(Type other) const {
  auto const newBits = m_bits & ~other.m_bits;
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
    assert(specializedType() == other.specializedType() &&
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
      return Type(newBits, m_arrayKind);
    }
    not_reached();
  }

  // Only other is specialized. This is fine as long as none of the bits
  // corresponding to other's specialization are set in *this. Otherwise we'd
  // have to represent things like "all classes except X".
  assert(IMPLIES(other.canSpecializeArrayKind(), !(m_bits & kAnyArr)) &&
         IMPLIES(other.canSpecializeClass(), !(m_bits & kAnyObj)) &&
         "Unsupported specialization subtraction");
  return Type(newBits);
}

Type liveTVType(const TypedValue* tv) {
  assert(tv->m_type == KindOfClass || tvIsPlausible(*tv));

  if (tv->m_type == KindOfObject) {
    return Type::Obj.specialize(tv->m_data.pobj->getVMClass());
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

Type binArithResultType(Opcode op, Type t1, Type t2) {
  if (op == Mod) {
    return Type::Int;
  }
  assert(op == Add || op == Sub || op == Mul);
  if (t1.subtypeOf(Type::Dbl) || t2.subtypeOf(Type::Dbl)) {
    return Type::Dbl;
  }
  return Type::Int;
}

Type ldRefReturn(const IRInstruction* inst) {
  // Guarding on specific classes/array kinds is expensive enough that we only
  // want to do it in situations we've confirmed the benefit.
  return inst->typeParam().unspecialize();
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
  if (inst->op() == AllocObjFast) {
    return Type::Obj.specialize(inst->extra<AllocObjFast>()->cls);
  }
  if (inst->op() == AllocObj) {
    return inst->src(0)->isConst()
      ? Type::Obj.specialize(inst->src(0)->getValClass())
      : Type::Obj;
  }
  always_assert(0);
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

Type outputType(const IRInstruction* inst, int dstId) {
#define IRT(name, ...) UNUSED static const Type name = Type::name;
  IR_TYPES
#undef IRT

#define D(type)   return type;
#define DofS(n)   return inst->src(n)->type();
#define DUnbox(n) return inst->src(n)->type().unbox();
#define DBox(n)   return boxType(inst->src(n)->type());
#define DParam    return inst->typeParam();
#define DAllocObj return allocObjReturn(inst);
#define DLdRef    return ldRefReturn(inst);
#define DThis     return thisReturn(inst);
#define DMulti    return Type::None;
#define DStk(in)  return stkReturn(inst, dstId,                         \
                                   [&]() -> Type { in not_reached(); });
#define DSetElem  return setElemReturn(inst);
#define ND        assert(0 && "outputType requires HasDest or NaryDest");
#define DBuiltin  return builtinReturn(inst);
#define DSubtract(n, t) return inst->src(n)->type() - t;
#define DArith    return binArithResultType(inst->op(), \
                                            inst->src(0)->type(),  \
                                            inst->src(1)->type());

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
#undef DArith

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
    auto const t = inst->typeParam();
    checkDst(t != Type::Bottom &&
             (t != Type::None || inst->is(DefConst)),
             "Invalid paramType for DParam instruction");
  };

  auto checkSpills = [&] {
    for (; curSrc < inst->numSrcs(); ++curSrc) {
      // SpillStack slots may be stack types or None, if the
      // simplifier removed some.
      auto const valid = inst->src(curSrc)->type()
        .subtypeOfAny(Type::StackElem, Type::None);
      check(valid, Type(), "Gen|Cls|None");
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
#define SNumInt  S(Int, Bool)
#define SNum     S(Int, Bool, Dbl)
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
#define DParam      requireTypeParam();
#define DLdRef      requireTypeParam();
#define DAllocObj
#define DThis
#define DArith      checkDst(inst->typeParam() == Type::None, \
                             "DArith should have no type parameter");

#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode: dstinfo srcinfo countCheck(); return;

  switch (inst->op()) {
    IR_OPCODES
  default: assert(0);
  }

#undef O

#undef NA
#undef SAny
#undef S
#undef C
#undef CStr
#undef SNum
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
#undef DParam
#undef DAllocObj
#undef DLdRef
#undef DThis
#undef DArith

}

std::string TypeConstraint::toString() const {
  std::string catStr;
  if (innerCat) {
    catStr = folly::to<std::string>("inner:",
                                    typeCategoryName(innerCat.get()));
  } else {
    catStr = typeCategoryName(category);
  }

  return folly::format("<{},{}>",
                       catStr, knownType).str();
}

//////////////////////////////////////////////////////////////////////

}}

