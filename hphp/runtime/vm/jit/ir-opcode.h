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
 *     DCtx         single dst has type Cctx|Obj<=ctx, where ctx is the
 *                    current context class
 *     DMulti       multiple dests. type and number depend on instruction
 *     DSetElem     single dst is a subset of CountedStr|Nullptr depending on
 *                    sources
 *     DBuiltin     single dst for CallBuiltin. This can return complex data
 *                    types such as (TStr | TNull)
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
 * A "query op" is any instruction returning TBool that is
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
 * Return true if opc is a str comparison operator
 */
bool isStrQueryOp(Opcode opc);

/*
 * Return the str-query opcode for the given non-str-query opcode
 */
Opcode queryToStrQueryOp(Opcode opc);

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
  Branch           = 1ULL <<  1,
  ConsumesRC       = 1ULL <<  2,
  ProducesRC       = 1ULL <<  3,
  MInstrProp       = 1ULL <<  4,
  MInstrElem       = 1ULL <<  5,
  MayRaiseError    = 1ULL <<  6,
  Terminal         = 1ULL <<  7, // has no next instruction
  NaryDest         = 1ULL <<  8, // has 0 or more destinations
  HasExtra         = 1ULL <<  9,
  Passthrough      = 1ULL << 10,
};

bool hasEdges(Opcode opc);
bool opcodeHasFlags(Opcode opc, uint64_t flags);

/*
 * Given an SSATmp of type Cls, try to find the name of the class.
 * Returns nullptr if can't find it.
 */
const StringData* findClassName(SSATmp* cls);

using TcaRange = folly::Range<TCA>;

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
