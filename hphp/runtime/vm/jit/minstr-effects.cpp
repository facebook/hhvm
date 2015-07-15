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
#include "hphp/runtime/vm/jit/minstr-effects.h"

#include "hphp/runtime/vm/jit/frame-state.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

template<typename T> T bad_value() { not_reached(); }

Opcode canonicalOp(Opcode op) {
  if (op == ElemUX || op == UnsetElem) {
    return UnsetElem;
  }
  if (op == SetWithRefElem || op == SetWithRefNewElem) {
    return SetWithRefElem;
  }
  return opcodeHasFlags(op, MInstrProp) ? SetProp
       : opcodeHasFlags(op, MInstrElem) ? SetElem
       : bad_value<Opcode>();
}

void getBaseType(Opcode rawOp, bool predict,
                 Type& baseType, bool& baseValChanged) {
  always_assert(baseType <= TCell);
  auto const op = canonicalOp(rawOp);

  // Deal with possible promotion to stdClass or array
  if ((op == SetElem || op == SetProp || op == SetWithRefElem) &&
      baseType.maybe(TNull | TBool | TStr)) {
    auto newBase = op == SetProp ? TObj : TArr;

    if (predict) {
      /* If the output type will be used as a prediction and not as fact, we
       * can be optimistic here. Assume no promotion for string bases and
       * promotion in other cases. */
      baseType = baseType <= TStr ? TStr : newBase;
    } else if (baseType <= TStr && rawOp == SetElem) {
      /* If the base is known to be a string and the operation is exactly
       * SetElem, we're guaranteed that either the base will end as a
       * CountedStr or the instruction will throw an exception and side
       * exit. */
      baseType = TCountedStr;
    } else if (baseType <= TStr && rawOp == SetNewElem) {
      /* If the string base is empty, it will be promoted to an
       * array. Otherwise the base will be left alone and we'll fatal. */
      baseType = TArr;
    } else {
      /* Regardless of whether or not promotion happens, we know the base
       * cannot be Null after the operation. If the base was a subtype of Null
       * this will give newBase. */
      baseType = (baseType - TNull) | newBase;
    }

    baseValChanged = true;
  }

  if ((op == SetElem || op == UnsetElem || op == SetWithRefElem) &&
      baseType.maybe(TArr | TStr)) {
    /* Modifying an array or string element, even when COW doesn't kick in,
     * produces a new SSATmp for the base. StaticArr/StaticStr may be promoted
     * to CountedArr/CountedStr. */
    baseValChanged = true;
    if (baseType.maybe(TStaticArr)) baseType |= TCountedArr;
    if (baseType.maybe(TStaticStr)) baseType |= TCountedStr;
  }
}

//////////////////////////////////////////////////////////////////////

}

// minstrBaseIdx returns the src index for inst's base operand.
int minstrBaseIdx(Opcode opc) {
  return opcodeHasFlags(opc, MInstrProp) ? 0 :
         opcodeHasFlags(opc, MInstrElem) ? 0 :
         bad_value<int>();
}

bool MInstrEffects::supported(Opcode op) {
  return opcodeHasFlags(op, MInstrProp | MInstrElem);
}
bool MInstrEffects::supported(const IRInstruction* inst) {
  return supported(inst->op());
}

MInstrEffects::MInstrEffects(const Opcode rawOp, const Type origBase) {
  // Note: MInstrEffects wants to manipulate pointer types in some situations
  // for historical reasons.  We'll eventually change that.
  bool const is_ptr = origBase <= TPtrToGen;
  auto const basePtr = is_ptr ? origBase.ptrKind() : Ptr::Unk;
  baseType = origBase.derefIfPtr();

  // Only certain types of bases are supported now but this list may expand in
  // the future.
  assert_not_implemented(
      is_ptr || baseType.subtypeOfAny(TObj, TArr));

  baseTypeChanged = baseValChanged = false;

  // Process the inner and outer types separately and then recombine them,
  // since the minstr operations all operate on the inner cell of boxed bases.
  // We treat the new inner type as a prediction because it will be verified
  // the next time we load from the box.
  auto inner = (baseType & TBoxedCell).inner();
  auto outer = baseType & TCell;
  getBaseType(rawOp, false, outer, baseValChanged);
  getBaseType(rawOp, true, inner, baseValChanged);

  baseType = inner.box() | outer;
  baseType = is_ptr ? baseType.ptr(basePtr) : baseType;

  baseTypeChanged = baseType != origBase;

  /* Boxed bases may have their inner value changed but the value of the box
   * will never change. */
  baseValChanged = !(origBase <= TBoxedCell) &&
                   (baseValChanged || baseTypeChanged);
}

//////////////////////////////////////////////////////////////////////

}}
