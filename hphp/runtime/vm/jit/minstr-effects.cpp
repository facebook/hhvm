/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
  if (op == ElemUX || op == UnsetElem)     return UnsetElem;
  if (op == SetRange || op == SetRangeRev) return SetRange;

  return opcodeHasFlags(op, MInstrProp) ? SetProp
       : opcodeHasFlags(op, MInstrElem) ? SetElem
       : bad_value<Opcode>();
}

void getBaseType(Opcode rawOp, bool predict,
                 Type& baseType, bool& baseValChanged) {
  always_assert(baseType <= TCell);
  auto const op = canonicalOp(rawOp);

  if ((op == SetElem || op == SetProp) &&
      baseType.maybe(TNull | TBool | TStr)) {
    if (predict && baseType.maybe(TStr)) {
      /* If the output type will be used as a prediction and not as fact, we
       * can be optimistic here. Assume no promotion for string bases and
       * an exception otherwise */
      baseType = TStr;
    } else if (baseType <= TStr && rawOp == SetNewElem) {
      /* new-elem to string will always raise, empty or not */
      baseType = TBottom;
    } else if (baseType.maybe(TNull | TBool)) {
      /* we used to promote falsey things to array or an instance of stdClass,
       * now these always raise, except a base of `true` gets coerced to null */
      baseType -= TNull;
    }

    if (baseType != TBottom) baseValChanged = true;
  }

  if ((op == SetElem || op == SetRange || op == UnsetElem) &&
      baseType.maybe(TArrLike | TStr | TRecord | TClsMeth)) {
    /* Member operations never cause us to lose the vanilla bit. */
    auto const vanilla = baseType.arrSpec().vanilla();

    /* Modifying an array or string element, even when COW doesn't kick in,
     * produces a new SSATmp for the base. StaticArr/StaticStr may be promoted
     * to CountedArr/CountedStr. */
    baseValChanged = true;

    if (baseType.maybe(TVec)) baseType |= TCountedVec;
    if (baseType.maybe(TDict)) baseType |= TCountedDict;
    if (baseType.maybe(TKeyset)) baseType |= TCountedKeyset;
    if (baseType.maybe(TStr)) baseType |= TCountedStr;
    if (baseType.maybe(TClsMeth)) baseType |= TCountedVec;

    if (vanilla) baseType = baseType.narrowToVanilla();
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
  bool const is_ptr = origBase <= TLvalToCell;
  auto const basePtr = is_ptr ? origBase.ptrKind() : Ptr::Bottom;
  baseType = origBase.derefIfPtr();

  baseTypeChanged = baseValChanged = false;

  getBaseType(rawOp, false, baseType, baseValChanged);

  baseType = is_ptr ? baseType.lval(basePtr) : baseType;
  baseTypeChanged = baseType != origBase;
  baseValChanged = baseValChanged || (baseTypeChanged && baseType != TBottom);
}

//////////////////////////////////////////////////////////////////////

}}
