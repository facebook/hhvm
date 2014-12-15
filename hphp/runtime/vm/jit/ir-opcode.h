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

#ifndef incl_HPHP_VM_IR_OPCODE_H_
#define incl_HPHP_VM_IR_OPCODE_H_

#include <type_traits>
#include <vector>

#include <folly/Range.h>

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////
struct StringData;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct IRUnit;
struct IRInstruction;
struct SSATmp;
struct LocalStateHook;
struct FrameStateMgr;

//////////////////////////////////////////////////////////////////////

/*
 * The instruction table below uses the following notation.  To use
 * it, you have to define these symbols to do what you want, and then
 * instantiate IR_OPCODES.
 *
 * dstinfo:
 *
 *   Contains a description of how to compute the type of the
 *   destination(s) of an instruction from its sources.
 *
 *     ND           instruction has no destination
 *     D(type)      single dst has a specific type
 *     DofS(N)      single dst has the type of src N
 *     DBox(N)      single dst has boxed type of src N
 *     DRefineS(N)  single dst's type is intersection of src N and paramType
 *     DParam       single dst has type of the instruction's type parameter
 *     DParamMayRelax like DParam, except type may relax
 *     DParamPtr(k) like DParam, but the param must be a PtrTo* of kind k
 *     DUnboxPtr    Unboxed PtrTo*T; adds possibility of pointing into a ref
 *     DBoxPtr      Boxed PtrTo*T
 *     DAllocObj    single dst has a type of a newly allocated object; may be a
 *                    specialized object type if the class is known
 *     DArrPacked   single dst has a packed array type
 *     DArrElem     single dst has type based on reading an array element,
 *                    intersected with an optional type parameter
 *     DThis        single dst has type Obj<ctx>, where ctx is the
 *                    current context class
 *     DMulti       multiple dests. type and number depend on instruction
 *     DSetElem     single dst is a subset of CountedStr|Nullptr depending on
 *                    sources
 *     DStk(x)      up to two dests. x should be another D* macro and indicates
 *                    the type of the first dest, if any. the second (or first,
 *                    depending on the presence of a primary destination), will
 *                    be of type Type::StkPtr. implies ModifiesStack.
 *     DBuiltin     single dst for CallBuiltin. This can return complex data
 *                    types such as (Type::Str | Type::Null)
 *     DSubtract(N,t) single dest has type of src N with t removed
 *     DCns         single dst's type is the union of legal types for PHP
 *                    constants
 *
 * srcinfo:
 *
 *   Contains a series of tests on the source parameters in order.
 *
 *     NA               instruction takes no sources
 *     S(t1,...,tn)     source must be a subtype of {t1|..|tn}
 *     AK(<kind>)       source must be an array with specified kind
 *     C(type)          source must be a constant, and subtype of type
 *     CStr             same as C(StaticStr)
 *     SVar(t1,...,tn)  variadic source list, all subtypes of {t1|..|tn}
 *
 * flags:
 *
 *   See doc/ir.specification for the meaning of these flags.
 *
 *   The flags in this opcode table supply default values for the
 *   querying functions in IRInstruction---those functions involve
 *   additional logic (based on operand types, etc) on a
 *   per-instruction basis.
 *
 *   The following abbreviations are used in this table:
 *
 *      NF    no flags
 *      C     canCSE
 *      E     isEssential
 *      Er    mayRaiseError
 *      PRc   producesRC
 *      CRc   consumesRC
 *      T     isTerminal
 *      B     isBranch
 *      P     passthrough
 *      K     killsSource
 *      MProp MInstrProp
 *      MElem MInstrElem
 */

#define O_STK(name, dst, src, flags)            \
  O(name, dst, src, StkFlags(flags))            \
  O(name ## Stk, DStk(dst), src S(StkPtr), flags)

// The IR opcode table is generated from lines that start with | in
// hphp/doc/ir.specification.
#include "hphp/runtime/ir-opcode-generated.h"

enum class Opcode : uint16_t {
#define O(name, ...) name,
  IR_OPCODES
#undef O
};
#define O(name, ...) UNUSED auto constexpr name = Opcode::name;
  IR_OPCODES
#undef O

#define O(...) +1
size_t constexpr kNumOpcodes = IR_OPCODES;
#undef O

/*
 * Returns true for instructions that perform calls.
 */
bool isCallOp(Opcode opc);

/*
 * Returns true for instructions that refine the types of values with
 * a runtime check.
 */
bool isGuardOp(Opcode opc);

/*
 * A "query op" is any instruction returning Type::Bool that is
 * negateable.
 */
bool isQueryOp(Opcode opc);

/*
 * Return true if opc is an int comparison operator
 */
bool isIntQueryOp(Opcode opc);

/*
 * Return the int-query opcode for the given non-int-query opcode
 */
Opcode queryToIntQueryOp(Opcode opc);

/*
 * Return true if opc is a dbl comparison operator
 */
bool isDblQueryOp(Opcode opc);

/*
 * Return the dbl-query opcode for the given non-dbl-query opcode
 */
Opcode queryToDblQueryOp(Opcode opc);

/*
 * A "fusable query op" is any instruction returning Type::Bool that
 * has a corresponding "query jump op" for branch fusion.
 */
bool isFusableQueryOp(Opcode opc);

/*
 * A "query jump op" is a conditional jump instruction that
 * corresponds to one of the fusable query op instructions.
 */
bool isQueryJmpOp(Opcode opc);

/*
 * Translate a query op into a conditional jump that does the same
 * test (a "query jump op").
 *
 * Pre: isFusableQueryOp(opc)
 */
Opcode queryToJmpOp(Opcode opc);

/*
 * Translate a "query jump op" to a query op.
 *
 * Pre: isQueryJmpOp(opc);
 */
Opcode queryJmpToQueryOp(Opcode opc);

/*
 * Convert a jump to its corresponding side exit.
 */
Opcode jmpToSideExitJmp(Opcode opc);

/*
 * Convert a jump operation to its corresponding conditional
 * ReqBindJmp.
 *
 * Pre: opc is a conditional jump.
 */
Opcode jmpToReqBindJmp(Opcode opc);

/*
 * Return the opcode that corresponds to negation of opc.
 */
Opcode negateQueryOp(Opcode opc);

/*
 * Return the opcode that corresponds to commuting the arguments of
 * opc.
 *
 * Pre: opc is a 2-argument query op.
 */
Opcode commuteQueryOp(Opcode opc);

const char* opcodeName(Opcode opcode);

bool opHasExtraData(Opcode op);

enum OpcodeFlag : uint64_t {
  NoFlags          = 0,
  HasDest          = 1ULL <<  0,
  CanCSE           = 1ULL <<  1,
  Essential        = 1ULL <<  2,
  Branch           = 1ULL <<  3,
  HasStackVersion  = 1ULL <<  4,
  ConsumesRC       = 1ULL <<  5,
  ProducesRC       = 1ULL <<  6,
  MInstrProp       = 1ULL <<  7,
  MInstrElem       = 1ULL <<  8,
  MayRaiseError    = 1ULL <<  9,
  Terminal         = 1ULL << 10, // has no next instruction
  NaryDest         = 1ULL << 11, // has 0 or more destinations
  HasExtra         = 1ULL << 12,
  Passthrough      = 1ULL << 13,
  KillsSources     = 1ULL << 14,
  ModifiesStack    = 1ULL << 15,
};

bool hasEdges(Opcode opc);
bool opcodeHasFlags(Opcode opc, uint64_t flags);
Opcode getStackModifyingOpcode(Opcode opc);

using SrcRange = folly::Range<SSATmp**>;
using DstRange = folly::Range<SSATmp*>;

/*
 * Given an SSATmp of type Cls, try to find the name of the class.
 * Returns nullptr if can't find it.
 */
const StringData* findClassName(SSATmp* cls);

/*
 * Return the output type from a given IRInstruction.
 *
 * The destination type is always predictable from the types of the inputs, any
 * type parameters to the instruction, and the id of the dest.
 */
Type outputType(const IRInstruction*, int dstId = 0);

/*
 * Check that an instruction has operands of allowed types.
 */
bool checkOperandTypes(const IRInstruction*, const IRUnit* unit = nullptr);

using TcaRange = folly::Range<TCA>;

/*
 * Counts the number of cells a SpillStack will logically push.  (Not
 * including the number it pops.)  That is, for each SSATmp in the
 * spill sources, this totals up whether it is an ActRec or a cell.
 */
int32_t spillValueCells(const IRInstruction* spillStack);

} // namespace jit
} // namespace HPHP

namespace std {
  template<> struct hash<HPHP::jit::Opcode> {
    size_t operator()(HPHP::jit::Opcode op) const { return uint16_t(op); }
  };
  template<> struct hash<HPHP::jit::Type> {
    size_t operator()(HPHP::jit::Type t) const { return t.hash(); }
  };
}

namespace folly {
template<> struct FormatValue<HPHP::jit::Opcode> {
  explicit FormatValue(HPHP::jit::Opcode op) : m_op(op) {}

  template<typename Callback> void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(opcodeName(m_op), arg, cb);
  }

 private:
  HPHP::jit::Opcode m_op;
};
}

#include "hphp/runtime/vm/jit/ir-opcode-inl.h"

#endif
