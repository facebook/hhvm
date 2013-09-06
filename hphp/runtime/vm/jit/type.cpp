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

using namespace HPHP::Transl;

namespace HPHP {  namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

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
  if (isBoxed()) {
    return RuntimeType(KindOfRef, innerType().toDataType());
  }
  return RuntimeType(toDataType());
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
  // Temporary stop-gap until we embrace gcc 4.8 fully
  // At that point we can switch to ctor delegation
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

Type liveTVType(const TypedValue* tv) {
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
  // an exception). If the baes might be a str, the result wil be
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

#define D(type)   return Type::type;
#define DofS(n)   return inst->src(n)->type();
#define DUnbox(n) return inst->src(n)->type().unbox();
#define DBox(n)   return boxType(inst->src(n)->type());
#define DParam    return inst->typeParam();
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
#define DParam      checkDst(inst->typeParam() != Type::None ||      \
                             inst->op() == DefConst /* for DefNone */, \
                             "DParam with paramType None");
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
#undef DArith

}

//////////////////////////////////////////////////////////////////////

}}

