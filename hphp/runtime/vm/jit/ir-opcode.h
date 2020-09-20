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

#pragma once

#include <type_traits>
#include <vector>

#include <folly/Range.h>

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////
struct StringData;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct IRUnit;
struct IRInstruction;
struct SSATmp;

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
 *     DParam(t)    single dst has type of the instruction's type parameter,
 *                    which must be a subtype of t
 *     DParamMayRelax(t) like DParam, except type may relax
 *     DAllocObj    single dst has a type of a newly allocated object; may be a
 *                    specialized object type if the class is known
 *     DVArr        single dst is either a packed array type or vec, depending
                      on configuration
 *     DDArr        single dst is either a mixed array type or dict, depending
                      on configuration
 *     DVecElem    single dst has type based on reading a vec element,
 *                    intersected with an optional type parameter
 *     DDictElem   single dst has type based on reading a dict element,
                      intersected with an optional type parameter
 *     DKeysetElem single dst has type int or string, intersected with an
 *                   optional type parameter.
 *     DMulti       multiple dests. type and number depend on instruction
 *     DSetElem     single dst is a subset of StaticStr|Nullptr depending on
 *                    sources
 *     DBuiltin     single dst for CallBuiltin. This can return complex data
 *                    types such as (TStr | TNull)
 *     DCall        single dst for non-builtin calls. This can return different
 *                     types depending on static analysis.
 *     DGenIter     single dst for generator iteration. This can return
 *                     different types based on whether the generator is async.
 *     DSubtract(N,t) single dest has type of src N with t removed
 *     DCns         single dst's type is the union of legal types for PHP
 *                    constants
 *     DUnion(N1,...) single dest has type that is the union of the specified
 *                      N srcs.
 *     DMemoKey     single dst for memoization key generation. Type depends on
 *                    source type.
 *     DLvalOfPtr   single dst with the Ptr type of src 0 converted to an
 *                    equivalent Lval, preserving all inner type information.
 *
 * srcinfo:
 *
 *   Contains a series of tests on the source parameters in order.
 *
 *     NA               instruction takes no sources
 *     S(t1,...,tn)     source must be a subtype of {t1|..|tn}
 *     S(AK(<kind>))    source must be an array with specified kind
 *     C(type)          source must be a constant, and subtype of type
 *     CStr             same as C(StaticStr)
 *     SVar(t1,...,tn)  variadic source list, all subtypes of {t1|..|tn}
 *     SVArr            source must be a packed array type or vec, depending
 *                      on configuration
 *     SDArr            source must be a mixed array type or dict, depending
 *                      on configuration
 *     CDArr            source must be a constant mixed array type or dict,
 *                      depending on configuration
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
 *      PRc   producesRC
 *      CRc   consumesRC
 *      T     isTerminal
 *      B     isBranch
 *      P     passthrough
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
 * Returns the negated version of the specified opcode, if its a comparison
 * opcode and can be negated (not all comparisons can be negated).
 */
folly::Optional<Opcode> negateCmpOp(Opcode opc);

const char* opcodeName(Opcode opcode);
folly::Optional<Opcode> nameToOpcode(const std::string&);

bool opHasExtraData(Opcode op);

enum OpcodeFlag : uint64_t {
  NoFlags          = 0,
  HasDest          = 1ULL << 0,
  Branch           = 1ULL << 1,
  ConsumesRC       = 1ULL << 2,
  ProducesRC       = 1ULL << 3,
  MInstrProp       = 1ULL << 4,
  MInstrElem       = 1ULL << 5,
  Terminal         = 1ULL << 6, // has no next instruction
  NaryDest         = 1ULL << 7, // has 0 or more destinations
  HasExtra         = 1ULL << 8,
  Passthrough      = 1ULL << 9,
  LayoutAgnostic   = 1ULL << 10,
  LayoutPreserving = 1ULL << 11,
};

bool hasEdges(Opcode opc);
bool opcodeHasFlags(Opcode opc, uint64_t flags);
bool opcodeMayRaise(Opcode opc);

/*
 * Given an SSATmp of type Cls, try to find the name of the class.
 * Returns nullptr if can't find it.
 */
const StringData* findClassName(SSATmp* cls);

} // namespace jit
} // namespace HPHP

namespace std {
  template<> struct hash<HPHP::jit::Opcode> {
    size_t operator()(HPHP::jit::Opcode op) const { return uint16_t(op); }
  };
}

namespace folly {
template<> class FormatValue<HPHP::jit::Opcode> {
 public:
  explicit FormatValue(HPHP::jit::Opcode op) : m_op(op) {}

  template<typename Callback> void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(opcodeName(m_op), arg, cb);
  }

 private:
  HPHP::jit::Opcode m_op;
};
}

#include "hphp/runtime/vm/jit/ir-opcode-inl.h"

