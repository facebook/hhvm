/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <util/logger.h>
#include <util/util.h>
#include <util/job_queue.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/repo.h>
#include <runtime/vm/as.h>
#include <runtime/vm/stats.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/eval/runtime/file_repository.h>

#include <compiler/builtin_symbols.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/emitter.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/peephole.h>

#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/array_pair_expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/expression/class_constant_expression.h>
#include <compiler/expression/closure_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/dynamic_variable.h>
#include <compiler/expression/encaps_list_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/include_expression.h>
#include <compiler/expression/list_assignment.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/expression/new_object_expression.h>
#include <compiler/expression/object_method_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/qop_expression.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/expression/static_member_expression.h>
#include <compiler/expression/unary_op_expression.h>

#include <compiler/statement/break_statement.h>
#include <compiler/statement/case_statement.h>
#include <compiler/statement/catch_statement.h>
#include <compiler/statement/class_constant.h>
#include <compiler/statement/class_variable.h>
#include <compiler/statement/do_statement.h>
#include <compiler/statement/echo_statement.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/statement/for_statement.h>
#include <compiler/statement/foreach_statement.h>
#include <compiler/statement/function_statement.h>
#include <compiler/statement/global_statement.h>
#include <compiler/statement/goto_statement.h>
#include <compiler/statement/if_branch_statement.h>
#include <compiler/statement/if_statement.h>
#include <compiler/statement/label_statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/return_statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/static_statement.h>
#include <compiler/statement/switch_statement.h>
#include <compiler/statement/try_statement.h>
#include <compiler/statement/unset_statement.h>
#include <compiler/statement/while_statement.h>
#include <compiler/statement/use_trait_statement.h>
#include <compiler/statement/trait_prec_statement.h>
#include <compiler/statement/trait_alias_statement.h>

#include <compiler/parser/parser.h>

#include <system/lib/systemlib.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace HPHP {
namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(emitter)

using boost::dynamic_pointer_cast;
using boost::static_pointer_cast;

namespace StackSym {
  static const char None = 0x00;

  static const char C = 0x01; // Cell symbolic flavor
  static const char V = 0x02; // Var symbolic flavor
  static const char A = 0x03; // Classref symbolic flavor
  static const char R = 0x04; // Return value symbolic flavor
  static const char F = 0x05; // Function argument symbolic flavor
  static const char L = 0x06; // Local symbolic flavor
  static const char T = 0x07; // String literal symbolic flavor
  static const char I = 0x08; // int literal symbolic flavor
  static const char H = 0x09; // $this symbolic flavor

  static const char N = 0x10; // Name marker
  static const char G = 0x20; // Global name marker
  static const char E = 0x30; // Element marker
  static const char W = 0x40; // New element marker
  static const char P = 0x50; // Property marker
  static const char S = 0x60; // Static property marker
  static const char M = 0x70; // Non elem/prop/W part of M-vector
  static const char K = 0x80; // Marker for information about a class base

  static const char CN = C | N;
  static const char CG = C | G;
  static const char CE = C | E;
  static const char CP = C | P;
  static const char CS = C | S;
  static const char LN = L | N;
  static const char LG = L | G;
  static const char LE = L | E;
  static const char LP = L | P;
  static const char LS = L | S;
  static const char AM = A | M;

  char GetSymFlavor(char sym) { return (sym & 0x0F); }
  char GetMarker(char sym) { return (sym & 0xF0); }
  std::string ToString(char sym) {
    char symFlavor = StackSym::GetSymFlavor(sym);
    std::string res;
    switch (symFlavor) {
      case StackSym::C: res = "C"; break;
      case StackSym::V: res = "V"; break;
      case StackSym::A: res = "A"; break;
      case StackSym::R: res = "R"; break;
      case StackSym::F: res = "F"; break;
      case StackSym::L: res = "L"; break;
      case StackSym::T: res = "T"; break;
      case StackSym::I: res = "I"; break;
      case StackSym::H: res = "H"; break;
      default: break;
    }
    char marker = StackSym::GetMarker(sym);
    switch (marker) {
      case StackSym::N: res += "N"; break;
      case StackSym::G: res += "G"; break;
      case StackSym::E: res += "E"; break;
      case StackSym::W: res += "W"; break;
      case StackSym::P: res += "P"; break;
      case StackSym::S: res += "S"; break;
      case StackSym::K: res += "K"; break;
      default: break;
    }
    if (res == "") {
      if (sym == StackSym::None) {
        res = "None";
      } else {
        res = "?";
      }
    }
    return res;
  }
}

//=============================================================================
// Emitter.

#define InvariantViolation(...) do {                        \
  Logger::Warning(__VA_ARGS__);                             \
  Logger::Warning("Eval stack at the time of error: %s",    \
                  m_evalStack.pretty().c_str());            \
  ASSERT(false);                                            \
} while (0)

// RAII guard for function creation.
class FuncFinisher {
  EmitterVisitor* m_ev;
  Emitter&        m_e;
  FuncEmitter*    m_fe;

 public:
  FuncFinisher(EmitterVisitor* ev, Emitter& e, FuncEmitter* fe)
    : m_ev(ev), m_e(e), m_fe(fe) {
      TRACE(1, "FuncFinisher constructed: %s %p\n", m_fe->name()->data(), m_fe);
    }

  ~FuncFinisher() {
    TRACE(1, "Finishing func: %s %p\n", m_fe->name()->data(), m_fe);
    m_ev->finishFunc(m_e, m_fe);
  }
};

// RAII guard for temporarily overriding an Emitter's location
class LocationGuard {
  Emitter& m_e;
  LocationPtr m_loc;

public:
  LocationGuard(Emitter& e, LocationPtr newLoc)
      : m_e(e), m_loc(e.getTempLocation()) {
    if (newLoc) m_e.setTempLocation(newLoc);
  }
  ~LocationGuard() {
    m_e.setTempLocation(m_loc);
  }
};

// Count the number of stack elements in an immediate vector.
static int32_t countStackValues(const std::vector<uchar>& immVec) {
  ASSERT(!immVec.empty());

  int count = 0;
  const uint8_t* vec = &immVec[0];

  // Count the location; the LS location type accounts for up to two
  // values on the stack, all other location types account for at most
  // one value on the stack.  Subtract the number that are actually
  // immediates.
  const LocationCode locCode = LocationCode(*vec++);
  count += numLocationCodeStackVals(locCode);
  const int numLocImms = numLocationCodeImms(locCode);
  for (int i = 0; i < numLocImms; ++i) {
    decodeVariableSizeImm(&vec);
  }

  // Count each of the members; MEC and MPC account for one value on
  // the stack, while MW/MEL/MPL/MET/MPT/MEI don't account for any
  // values on the stack.
  while (vec - &immVec[0] < int(immVec.size())) {
    MemberCode code = MemberCode(*vec++);
    if (memberCodeHasImm(code)) {
      decodeMemberCodeImm(&vec, code);
    } else if (code != MW) {
      ++count;
    }
  }
  ASSERT(vec - &immVec[0] == int(immVec.size()));
  return count;
}

#define O(name, imm, pop, push, flags) \
  void Emitter::name(imm) { \
    const Opcode opcode = Op##name; \
    Offset curPos UNUSED = getUnitEmitter().bcPos(); \
    getEmitterVisitor().prepareEvalStack(); \
    POP_##pop; \
    const int nIn UNUSED = COUNT_##pop; \
    POP_HA_##imm; \
    PUSH_##push; \
    getUnitEmitter().emitOp(Op##name); \
    IMPL_##imm; \
    getUnitEmitter().recordSourceLocation(m_tempLoc ? m_tempLoc.get() : \
                                          m_node->getLocation().get(), curPos); \
    if (flags & TF) getEmitterVisitor().restoreJumpTargetEvalStack(); \
    getEmitterVisitor().setPrevOpcode(opcode); \
  }

#define COUNT_NOV 0
#define COUNT_ONE(t) 1
#define COUNT_TWO(t1,t2) 2
#define COUNT_THREE(t1,t2,t3) 3
#define COUNT_FOUR(t1,t2,t3,t4) 4
#define COUNT_LMANY() 0
#define COUNT_C_LMANY() 0
#define COUNT_V_LMANY() 0
#define COUNT_FMANY 0
#define COUNT_CMANY 0

#define ONE(t) \
  DEC_##t a1
#define TWO(t1, t2) \
  DEC_##t1 a1, DEC_##t2 a2
#define THREE(t1, t2, t3) \
  DEC_##t1 a1, DEC_##t2 a2, DEC_##t3 a3
#define FOUR(t1, t2, t3, t4) \
  DEC_##t1 a1, DEC_##t2 a2, DEC_##t3 a3, DEC_##t4 a4
#define NA
#define DEC_MA std::vector<uchar>
#define DEC_BLA std::vector<Label*>&
#define DEC_SLA std::vector<StrOff>&
#define DEC_IVA int32
#define DEC_HA int32
#define DEC_IA int32
#define DEC_I64A int64
#define DEC_DA double
#define DEC_SA const StringData*
#define DEC_AA ArrayData*
#define DEC_BA Label&
#define DEC_OA uchar

#define POP_NOV
#define POP_ONE(t) \
  POP_##t(0)
#define POP_TWO(t1, t2) \
  POP_##t1(0);          \
  POP_##t2(1)
#define POP_THREE(t1, t2, t3) \
  POP_##t1(0);                \
  POP_##t2(1);                \
  POP_##t3(2)
#define POP_FOUR(t1, t2, t3, t4) \
  POP_##t1(0);                   \
  POP_##t2(1);                   \
  POP_##t3(2);                   \
  POP_##t4(3)
#define POP_LMANY() \
  getEmitterVisitor().popEvalStackLMany()
#define POP_C_LMANY() \
  getEmitterVisitor().popEvalStack(StackSym::C); \
  getEmitterVisitor().popEvalStackLMany()
#define POP_V_LMANY() \
  getEmitterVisitor().popEvalStack(StackSym::V); \
  getEmitterVisitor().popEvalStackLMany()
#define POP_FMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::F)
#define POP_CMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::C)

#define POP_CV(i) getEmitterVisitor().popEvalStack(StackSym::C, i, curPos)
#define POP_VV(i) getEmitterVisitor().popEvalStack(StackSym::V, i, curPos)
#define POP_AV(i) getEmitterVisitor().popEvalStack(StackSym::A, i, curPos)
#define POP_RV(i) getEmitterVisitor().popEvalStack(StackSym::R, i, curPos)
#define POP_FV(i) getEmitterVisitor().popEvalStack(StackSym::F, i, curPos)

// Pop of virtual "locs" on the stack that turn into immediates.
#define POP_HA_ONE(t) \
  POP_HA_##t(nIn)
#define POP_HA_TWO(t1, t2) \
  POP_HA_##t1(nIn);    \
  POP_HA_##t2(nIn)
#define POP_HA_THREE(t1, t2, t3) \
  POP_HA_##t1(nIn);          \
  POP_HA_##t2(nIn);          \
  POP_HA_##t3(nIn)
#define POP_HA_FOUR(t1, t2, t3, t4) \
  POP_HA_##t1(nIn);             \
  POP_HA_##t2(nIn);             \
  POP_HA_##t3(nIn);             \
  POP_HA_##t4(nIn)

#define POP_HA_NA
#define POP_HA_MA(i)
#define POP_HA_BLA(i)
#define POP_HA_SLA(i)
#define POP_HA_IVA(i)
#define POP_HA_IA(i)
#define POP_HA_I64A(i)
#define POP_HA_DA(i)
#define POP_HA_SA(i)
#define POP_HA_AA(i)
#define POP_HA_BA(i)
#define POP_HA_OA(i)

#define POP_HA_HA(i) \
  getEmitterVisitor().popSymbolicLocal(opcode, i, curPos)

#define PUSH_NOV
#define PUSH_ONE(t) \
  PUSH_##t
#define PUSH_TWO(t1, t2) \
  PUSH_##t2; \
  PUSH_##t1
#define PUSH_THREE(t1, t2, t3) \
  PUSH_##t3; \
  PUSH_##t2; \
  PUSH_##t1
#define PUSH_FOUR(t1, t2, t3, t4) \
  PUSH_##t4; \
  PUSH_##t3; \
  PUSH_##t2; \
  PUSH_##t1
#define PUSH_INS_1(t) PUSH_INS_1_##t
#define PUSH_INS_2(t) PUSH_INS_2_##t

#define PUSH_CV getEmitterVisitor().pushEvalStack(StackSym::C)
#define PUSH_VV getEmitterVisitor().pushEvalStack(StackSym::V)
#define PUSH_AV getEmitterVisitor().pushEvalStack(StackSym::A)
#define PUSH_RV getEmitterVisitor().pushEvalStack(StackSym::R)
#define PUSH_FV getEmitterVisitor().pushEvalStack(StackSym::F)

#define PUSH_INS_1_CV \
  getEmitterVisitor().getEvalStack().insertAt(1, StackSym::C);
#define PUSH_INS_1_AV \
  getEmitterVisitor().getEvalStack().insertAt(1, StackSym::A);

#define PUSH_INS_2_CV \
  getEmitterVisitor().getEvalStack().insertAt(2, StackSym::C);

#define IMPL_NA
#define IMPL_ONE(t) \
  IMPL1_##t
#define IMPL_TWO(t1, t2) \
  IMPL1_##t1; \
  IMPL2_##t2
#define IMPL_THREE(t1, t2, t3) \
  IMPL1_##t1; \
  IMPL2_##t2; \
  IMPL3_##t3
#define IMPL_FOUR(t1, t2, t3, t4) \
  IMPL1_##t1; \
  IMPL2_##t2; \
  IMPL3_##t3; \
  IMPL4_##t4

#define IMPL_MA(var) do {                             \
  getUnitEmitter().emitInt32(var.size());             \
  getUnitEmitter().emitInt32(countStackValues(var));  \
  for (unsigned int i = 0; i < var.size(); ++i) {     \
    getUnitEmitter().emitByte(var[i]);                \
  }                                                   \
} while (0)
#define IMPL1_MA IMPL_MA(a1)
#define IMPL2_MA IMPL_MA(a2)
#define IMPL3_MA IMPL_MA(a3)
#define IMPL4_MA IMPL_MA(a4)

#define IMPL_BLA(var) do {                            \
  getUnitEmitter().emitInt32(var.size());             \
  for (unsigned int i = 0; i < var.size(); ++i) {     \
    IMPL_BA(*var[i]);                                 \
  }                                                   \
} while (0)
#define IMPL1_BLA IMPL_BLA(a1)
#define IMPL2_BLA IMPL_BLA(a2)
#define IMPL3_BLA IMPL_BLA(a3)
#define IMPL4_BLA IMPL_BLA(a4)

#define IMPL_SLA(var) do {                      \
  auto& ue = getUnitEmitter();                  \
  ue.emitInt32(var.size());                     \
  for (auto& i : var) {                         \
    ue.emitInt32(i.str);                        \
    IMPL_BA(*i.dest);                           \
  }                                             \
} while (0)
#define IMPL1_SLA IMPL_SLA(a1)
#define IMPL2_SLA IMPL_SLA(a2)
#define IMPL3_SLA IMPL_SLA(a3)

#define IMPL_IVA(var) do { \
  getUnitEmitter().emitIVA(var); \
} while (0)
#define IMPL1_IVA IMPL_IVA(a1)
#define IMPL2_IVA IMPL_IVA(a2)
#define IMPL3_IVA IMPL_IVA(a3)
#define IMPL4_IVA IMPL_IVA(a4)

#define IMPL1_HA IMPL_IVA(a1)
#define IMPL2_HA IMPL_IVA(a2)
#define IMPL3_HA IMPL_IVA(a3)
#define IMPL4_HA IMPL_IVA(a4)

#define IMPL1_IA IMPL_IVA(a1)
#define IMPL2_IA IMPL_IVA(a2)
#define IMPL3_IA IMPL_IVA(a3)
#define IMPL4_IA IMPL_IVA(a4)

#define IMPL_I64A(var) getUnitEmitter().emitInt64(var)
#define IMPL1_I64A IMPL_I64A(a1)
#define IMPL2_I64A IMPL_I64A(a2)
#define IMPL3_I64A IMPL_I64A(a3)
#define IMPL4_I64A IMPL_I64A(a4)

#define IMPL_SA(var) \
  getUnitEmitter().emitInt32(getUnitEmitter().mergeLitstr(var))
#define IMPL1_SA IMPL_SA(a1)
#define IMPL2_SA IMPL_SA(a2)
#define IMPL3_SA IMPL_SA(a3)
#define IMPL4_SA IMPL_SA(a4)

#define IMPL_AA(var) \
  getUnitEmitter().emitInt32(getUnitEmitter().mergeArray(var))
#define IMPL1_AA IMPL_AA(a1)
#define IMPL2_AA IMPL_AA(a2)
#define IMPL3_AA IMPL_AA(a3)
#define IMPL4_AA IMPL_AA(a4)

#define IMPL_DA(var) getUnitEmitter().emitDouble(var)
#define IMPL1_DA IMPL_DA(a1)
#define IMPL2_DA IMPL_DA(a2)
#define IMPL3_DA IMPL_DA(a3)
#define IMPL4_DA IMPL_DA(a4)

#define IMPL_BA(var) \
  if ((var).getAbsoluteOffset() == InvalidAbsoluteOffset) { \
    /* For forward jumps, we store information about the */ \
    /* current instruction in the Label. When the Label is */ \
    /* set, it will fix up any instructions that reference */ \
    /* it, and then it will call recordJumpTarget */ \
    (var).bind(getEmitterVisitor(), curPos, getUnitEmitter().bcPos()); \
  } else { \
    /* For backward jumps, we simply call recordJumpTarget */ \
    getEmitterVisitor().recordJumpTarget((var).getAbsoluteOffset()); \
  } \
  getUnitEmitter().emitInt32((var).getAbsoluteOffset() - curPos);
#define IMPL1_BA IMPL_BA(a1)
#define IMPL2_BA IMPL_BA(a2)
#define IMPL3_BA IMPL_BA(a3)
#define IMPL4_BA IMPL_BA(a4)

#define IMPL_OA(var) getUnitEmitter().emitByte(var)
#define IMPL1_OA IMPL_OA(a1)
#define IMPL2_OA IMPL_OA(a2)
#define IMPL3_OA IMPL_OA(a3)
#define IMPL4_OA IMPL_OA(a4)
 OPCODES
#undef O
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef NA
#undef DEC_MA
#undef DEC_IVA
#undef DEC_HA
#undef DEC_IA
#undef DEC_I64A
#undef DEC_DA
#undef DEC_SA
#undef DEC_AA
#undef DEC_BA
#undef DEC_OA
#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE
#undef POP_FOUR
#undef POP_LMANY
#undef POP_C_LMANY
#undef POP_V_LMANY
#undef POP_CV
#undef POP_VV
#undef POP_HV
#undef POP_AV
#undef POP_RV
#undef POP_FV
#undef POP_LREST
#undef POP_FMANY
#undef POP_CMANY
#undef POP_HA_ONE
#undef POP_HA_TWO
#undef POP_HA_THREE
#undef POP_HA_FOUR
#undef POP_HA_NA
#undef POP_HA_MA
#undef POP_HA_IVA
#undef POP_HA_IA
#undef POP_HA_I64A
#undef POP_HA_DA
#undef POP_HA_SA
#undef POP_HA_AA
#undef POP_HA_BA
#undef POP_HA_OA
#undef POP_HA_HA
#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_THREE
#undef PUSH_FOUR
#undef PUSH_CV
#undef PUSH_VV
#undef PUSH_HV
#undef PUSH_AV
#undef PUSH_RV
#undef PUSH_FV
#undef IMPL_ONE
#undef IMPL_TWO
#undef IMPL_THREE
#undef IMPL_FOUR
#undef IMPL_NA
#undef IMPL_MA
#undef IMPL1_MA
#undef IMPL2_MA
#undef IMPL3_MA
#undef IMPL4_MA
#undef IMPL_BLA
#undef IMPL1_BLA
#undef IMPL2_BLA
#undef IMPL3_BLA
#undef IMPL4_BLA
#undef IMPL_SLA
#undef IMPL1_SLA
#undef IMPL2_SLA
#undef IMPL3_SLA
#undef IMPL4_SLA
#undef IMPL_IVA
#undef IMPL1_IVA
#undef IMPL2_IVA
#undef IMPL3_IVA
#undef IMPL4_IVA
#undef IMPL1_HA
#undef IMPL2_HA
#undef IMPL3_HA
#undef IMPL4_HA
#undef IMPL1_IA
#undef IMPL2_IA
#undef IMPL3_IA
#undef IMPL4_IA
#undef IMPL_I64A
#undef IMPL1_I64A
#undef IMPL2_I64A
#undef IMPL3_I64A
#undef IMPL4_I64A
#undef IMPL_DA
#undef IMPL1_DA
#undef IMPL2_DA
#undef IMPL3_DA
#undef IMPL4_DA
#undef IMPL_SA
#undef IMPL1_SA
#undef IMPL2_SA
#undef IMPL3_SA
#undef IMPL4_SA
#undef IMPL_AA
#undef IMPL1_AA
#undef IMPL2_AA
#undef IMPL3_AA
#undef IMPL4_AA
#undef IMPL_BA
#undef IMPL1_BA
#undef IMPL2_BA
#undef IMPL3_BA
#undef IMPL4_BA
#undef IMPL_OA
#undef IMPL1_OA
#undef IMPL2_OA
#undef IMPL3_OA
#undef IMPL4_OA

static void checkJmpTargetEvalStack(const SymbolicStack& source,
                                    const SymbolicStack& dest) {
  if (source.size() != dest.size()) {
    Logger::Warning("Emitter detected a point in the bytecode where the "
                    "depth of the stack is not the same for all possible "
                    "control flow paths");
    ASSERT(false);
    return;
  }

  for (unsigned int i = 0; i < source.size(); ++i) {
    char flavor = StackSym::GetSymFlavor(source.get(i));
    bool matches = source.get(i) == dest.get(i) &&
      (flavor != StackSym::L || source.getLoc(i) == dest.getLoc(i)) &&
      (flavor != StackSym::T || source.getName(i) == dest.getName(i)) &&
      (flavor != StackSym::I || source.getInt(i) == dest.getInt(i));
    if (!matches) {
      Logger::Warning("Emitter detected a point in the bytecode where the "
                      "symbolic flavor of a slot on the stack is not the same "
                      "for all possible control flow paths");
      Logger::Warning("src stack : %s", source.pretty().c_str());
      Logger::Warning("dest stack: %s", dest.pretty().c_str());
      ASSERT(false);
      return;
    }
  }
}

std::string SymbolicStack::pretty() const {
  std::ostringstream out;
  out << " [" << std::hex;
  size_t j = 0;
  for (size_t i = 0; i < m_symStack.size(); ++i) {
    while (j < m_actualStack.size() && m_actualStack[j] < int(i)) {
      ++j;
    }
    if (j < m_actualStack.size() && m_actualStack[j] == int(i)) {
      out << "*";
    }
    out << StackSym::ToString(m_symStack[i].sym);
    char flavor = StackSym::GetSymFlavor(m_symStack[i].sym);
    if (flavor == StackSym::L || flavor == StackSym::I) {
      out << ":" << m_symStack[i].intval;
    } else if (flavor == StackSym::T) {
      out << ":" << m_symStack[i].metaData.name->data();
    }
    out << ' ';
  }
  return out.str();
}

void SymbolicStack::push(char sym) {
  if (sym != StackSym::W && sym != StackSym::K && sym != StackSym::L &&
      sym != StackSym::T && sym != StackSym::I && sym != StackSym::H) {
    m_actualStack.push_back(m_symStack.size());
    *m_actualStackHighWaterPtr = MAX(*m_actualStackHighWaterPtr,
                                     (int)m_actualStack.size());
  }
  m_symStack.push_back(SymEntry(sym));
}

void SymbolicStack::pop() {
  // TODO(drew): ASSERT eval stack unknown is false?
  ASSERT(!m_symStack.empty());
  char sym = m_symStack.back().sym;
  char flavor = StackSym::GetSymFlavor(sym);
  if (StackSym::GetMarker(sym) != StackSym::W &&
      flavor != StackSym::L && flavor != StackSym::T && flavor != StackSym::I &&
      flavor != StackSym::H) {
    ASSERT(!m_actualStack.empty());
    m_actualStack.pop_back();
  }
  m_symStack.pop_back();
}

char SymbolicStack::top() const {
  ASSERT(!m_symStack.empty());
  return m_symStack.back().sym;
}

char SymbolicStack::get(int index) const {
  ASSERT(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].sym;
}

const StringData* SymbolicStack::getName(int index) const {
  ASSERT(index >= 0 && index < (int)m_symStack.size());
  if (m_symStack[index].metaType == META_LITSTR) {
    return m_symStack[index].metaData.name;
  }
  return NULL;
}

const StringData* SymbolicStack::getClsName(int index) const {
  ASSERT(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].className;
}

bool SymbolicStack::isCls(int index) const {
  ASSERT(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].className != nullptr;
}

void SymbolicStack::setString(const StringData* s) {
  ASSERT(m_symStack.size());
  SymEntry& se = m_symStack.back();
  if (se.metaType == META_LITSTR) {
    ASSERT(se.metaData.name == s);
  } else {
    ASSERT(se.metaType == META_NONE);
  }
  se.metaData.name = s;
  se.metaType = META_LITSTR;
}

void SymbolicStack::setKnownCls(const StringData* s, bool nonNull) {
  ASSERT(m_symStack.size());
  SymEntry& se = m_symStack.back();
  ASSERT(!se.className || se.className == s);
  if (se.metaType == META_DATA_TYPE) {
    ASSERT(se.metaData.dt == KindOfObject);
    nonNull = true;
  }
  se.className = s;
  se.notNull = se.notNull || nonNull;
}

void SymbolicStack::setNotRef() {
  ASSERT(m_symStack.size());
  SymEntry& se = m_symStack.back();
  se.notRef = true;
}

bool SymbolicStack::getNotRef() const {
  ASSERT(m_symStack.size());
  const SymEntry& se = m_symStack.back();
  return se.notRef;
}

void SymbolicStack::setInt(int64 v) {
  ASSERT(m_symStack.size());
  m_symStack.back().intval = v;
}

void SymbolicStack::setKnownType(DataType dt, bool predicted /* = false */) {
  ASSERT(m_symStack.size());
  SymEntry& se = m_symStack.back();
  if (se.className) {
    ASSERT(dt == KindOfObject);
    se.notNull = true;
  } else {
    ASSERT(se.metaType == META_NONE);
    se.metaType = META_DATA_TYPE;
    se.metaData.dt = dt;
  }
  se.dtPredicted = predicted;
}

DataType SymbolicStack::getKnownType(int index, bool noRef) const {
  if (index < 0) index += m_symStack.size();
  ASSERT((unsigned)index < m_symStack.size());
  const SymEntry& se = m_symStack[index];
  if (!noRef || se.notRef) {
    if (se.className && se.notNull) {
      return KindOfObject;
    } else if (se.metaType == META_DATA_TYPE) {
      return se.metaData.dt;
    }
  }
  return KindOfUnknown;
}

bool SymbolicStack::isTypePredicted(int index /* = -1, stack top */) const {
  if (index < 0) index += m_symStack.size();
  ASSERT((unsigned)index < m_symStack.size());
  return m_symStack[index].dtPredicted;
}

void SymbolicStack::cleanTopMeta() {
  SymEntry& se = m_symStack.back();
  se.clsBaseType = CLS_INVALID;
  se.metaType = META_NONE;
  se.notRef = false;
}

void SymbolicStack::setClsBaseType(ClassBaseType type) {
  ASSERT(!m_symStack.empty());
  m_symStack.back().clsBaseType = type;
}

void SymbolicStack::setUnnamedLocal(int index,
                                    int localId,
                                    Offset startOff) {
  ASSERT(size_t(index) < m_symStack.size());
  ASSERT(m_symStack[index].sym == StackSym::K);
  ASSERT(m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  m_symStack[index].intval = localId;
  m_symStack[index].unnamedLocalStart = startOff;
}

void SymbolicStack::set(int index, char sym) {
  ASSERT(index >= 0 && index < (int)m_symStack.size());
  // XXX Add ASSERT in debug build to make sure W is not getting
  // written or overwritten by something else
  m_symStack[index].sym = sym;
}

unsigned SymbolicStack::size() const {
  return m_symStack.size();
}

bool SymbolicStack::empty() const {
  return m_symStack.empty();
}

void SymbolicStack::clear() {
  m_symStack.clear();
  m_actualStack.clear();
  m_fdescCount = 0;
}

void SymbolicStack::consumeBelowTop(int depth) {
  if (int(m_symStack.size()) < depth + 1) {
    Logger::Warning(
      "Emitter tried to consumeBelowTop() when the symbolic "
      "stack did not have enough elements in it.");
    ASSERT(false);
    return;
  }
  ASSERT(int(m_symStack.size()) >= depth + 1);
  int index = m_symStack.size() - depth - 1;
  m_symStack.erase(m_symStack.begin() + index);

  /*
   * Update any indexes into the actual stack that pointed to or past
   * this element.
   *
   * (In practice they should all be past---we don't currently ever
   * remove below the top for actual stack elements.)
   */
  for (size_t i = 0; i < m_actualStack.size(); ++i) {
    if (m_actualStack[i] >= index) {
      --m_actualStack[i];
    }
  }
}

int SymbolicStack::getActualPos(int vpos) const {
  ASSERT(vpos >= 0 && vpos < int(m_symStack.size()));
  ASSERT(!m_actualStack.empty());
  for (int j = int(m_actualStack.size()) - 1; j >= 0; --j) {
    if (m_actualStack[j] == vpos) {
      return j;
    }
  }
  NOT_REACHED();
}

char SymbolicStack::getActual(int index) const {
  ASSERT(index >= 0 && index < (int)m_actualStack.size());
  return get(m_actualStack[index]);
}

void SymbolicStack::setActual(int index, char sym) {
  ASSERT(index >= 0 && index < (int)m_actualStack.size());
  set(m_actualStack[index], sym);
}

SymbolicStack::ClassBaseType
SymbolicStack::getClsBaseType(int index) const {
  ASSERT(m_symStack.size() > size_t(index));
  ASSERT(m_symStack[index].sym == StackSym::K);
  ASSERT(m_symStack[index].clsBaseType != CLS_INVALID);
  return m_symStack[index].clsBaseType;
}

int SymbolicStack::getLoc(int index) const {
  ASSERT(m_symStack.size() > size_t(index));
  ASSERT(StackSym::GetSymFlavor(m_symStack[index].sym) == StackSym::L ||
         m_symStack[index].clsBaseType == CLS_NAMED_LOCAL ||
         m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  ASSERT(m_symStack[index].intval != -1);
  return m_symStack[index].intval;
}

int64 SymbolicStack::getInt(int index) const {
  ASSERT(m_symStack.size() > size_t(index));
  ASSERT(StackSym::GetSymFlavor(m_symStack[index].sym) == StackSym::I);
  return m_symStack[index].intval;
}

Offset SymbolicStack::getUnnamedLocStart(int index) const {
  ASSERT(m_symStack.size() > size_t(index));
  ASSERT(m_symStack[index].sym == StackSym::K);
  ASSERT(m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  return m_symStack[index].unnamedLocalStart;
}

// Insert an element in the actual stack at the specified depth of the
// actual stack.
void SymbolicStack::insertAt(int depth, char sym) {
  ASSERT(depth <= sizeActual() && depth > 0);
  int virtIdx = m_actualStack[sizeActual() - depth];

  m_symStack.insert(m_symStack.begin() + virtIdx, SymEntry(sym));
  m_actualStack.insert(m_actualStack.end() - depth, virtIdx);

  for (size_t i = sizeActual() - depth + 1; i < m_actualStack.size(); ++i) {
    ++m_actualStack[i];
  }
}

int SymbolicStack::sizeActual() const {
  return m_actualStack.size();
}

void SymbolicStack::pushFDesc() {
  m_fdescCount += kNumActRecCells;
  *m_fdescHighWaterPtr = MAX(*m_fdescHighWaterPtr, m_fdescCount);
}

void SymbolicStack::popFDesc() {
  m_fdescCount -= kNumActRecCells;
}

void Label::set(Emitter& e) {
  if (isSet()) {
    InvariantViolation(
      "Label::set was called more than once on the same "
      "Label; originally set to %d; now %d",
      m_off,
      e.getUnitEmitter().bcPos());
    return;
  }
  m_off = e.getUnitEmitter().bcPos();
  // Fix up any forward jumps that reference to this Label
  for (std::vector<std::pair<Offset, Offset> >::const_iterator it =
      m_emittedOffs.begin(); it != m_emittedOffs.end(); ++it) {
    e.getUnitEmitter().emitInt32(m_off - it->first, it->second);
  }
  EmitterVisitor& ev = e.getEmitterVisitor();
  if (!m_emittedOffs.empty()) {
    // If there were forward jumps that referenced this Label,
    // compare the the eval stack from the first foward jump we
    // saw with the current eval stack
    if (!ev.evalStackIsUnknown()) {
      checkJmpTargetEvalStack(m_evalStack, ev.getEvalStack());
    } else {
      // Assume the current eval stack matches that of the forward branch
      ev.setEvalStack(m_evalStack);
    }
    // Fix up the EmitterVisitor's table of jump targets
    ev.recordJumpTarget(m_off, ev.getEvalStack());
  } else {
    // There were no forward jumps that referenced this label
    ev.prepareEvalStack();
    // Fix up the EmitterVisitor's table of jump targets
    ev.recordJumpTarget(m_off, ev.getEvalStack());
  }
}

bool Label::isUsed() {
  return (m_off != InvalidAbsoluteOffset || !m_emittedOffs.empty());
}

void Label::bind(EmitterVisitor& ev, Offset instrAddr, Offset offAddr) {
  if (m_off != InvalidAbsoluteOffset) {
    InvariantViolation("Label::bind was called on a Label that has already "
                       "been set to %d",
                       m_off);
    return;
  }
  bool labelHasEvalStack = !m_emittedOffs.empty();
  m_emittedOffs.push_back(std::pair<Offset, Offset>(instrAddr, offAddr));
  if (labelHasEvalStack) {
    checkJmpTargetEvalStack(m_evalStack, ev.getEvalStack());
  } else {
    m_evalStack = ev.getEvalStack();
  }
}

struct FPIRegionRecorder {
  FPIRegionRecorder(EmitterVisitor* ev, UnitEmitter& ue, SymbolicStack& stack,
                    Offset start)
      : m_ev(ev), m_ue(ue), m_stack(stack), m_fpOff(m_stack.sizeActual()),
        m_start(start) {
    m_stack.pushFDesc();
  }
  ~FPIRegionRecorder() {
    m_stack.popFDesc();
    m_ev->newFPIRegion(m_start, m_ue.bcPos(), m_fpOff);
  }
private:
  EmitterVisitor* m_ev;
  UnitEmitter& m_ue;
  SymbolicStack& m_stack;
  int m_fpOff;
  Offset m_start;
};

//=============================================================================
// EmitterVisitor.

void MetaInfoBuilder::add(int pos, Unit::MetaInfo::Kind kind,
                          bool mVector, int arg, Id data) {
  ASSERT(arg >= 0);
  if (arg > 127) return;
  if (mVector) arg |= Unit::MetaInfo::VectorArg;
  Vec& info = m_metaMap[pos];
  int i = info.size();
  if (kind == Unit::MetaInfo::NopOut) {
    info.clear();
  } else if (i == 1 && info[0].m_kind == Unit::MetaInfo::NopOut) {
    return;
  } else if (kind == Unit::MetaInfo::DataTypeInferred ||
             kind == Unit::MetaInfo::DataTypePredicted) {
    // Put DataType first, because if applyInputMetaData saw Class
    // first, it would call recordRead which mark the input as
    // needing a guard before we saw the DataType
    i = 0;
  }
  info.insert(info.begin() + i, Unit::MetaInfo(kind, arg, data));
}

void MetaInfoBuilder::addKnownDataType(DataType dt,
                                       bool     dtPredicted,
                                       int      pos,
                                       bool     mVector,
                                       int      arg) {
  if (dt != KindOfUnknown) {
    Unit::MetaInfo::Kind dtKind = (dtPredicted ?
                                   Unit::MetaInfo::DataTypePredicted :
                                   Unit::MetaInfo::DataTypeInferred);
    add(pos, dtKind, mVector, arg, dt);
  }
}

void MetaInfoBuilder::deleteInfo(Offset bcOffset) {
  m_metaMap.erase(bcOffset);
}

void MetaInfoBuilder::setForUnit(UnitEmitter& target) const {
  int entries = m_metaMap.size();
  if (!entries) return;

  vector<Offset> index1;
  vector<Offset> index2;
  vector<uint8> data;
  index1.push_back(entries);

  size_t sz1 = (2 + entries) * sizeof(Offset);
  size_t sz2 = (1 + entries) * sizeof(Offset);
  for (Map::const_iterator it = m_metaMap.begin(), end = m_metaMap.end();
      it != end; ++it) {
    index1.push_back(it->first);
    index2.push_back(sz1 + sz2 + data.size());

    const Vec& v = it->second;
    ASSERT(v.size());
    for (unsigned i = 0; i < v.size(); i++) {
      const Unit::MetaInfo& mi = v[i];
      data.push_back(mi.m_kind);
      data.push_back(mi.m_arg);
      if (mi.m_data < 0x80) {
        data.push_back(mi.m_data << 1);
      } else {
        union {
          uint32 val;
          uint8  bytes[4];
        } u;
        u.val = (mi.m_data << 1) | 1;
        for (int j = 0; j < 4; j++) {
          data.push_back(u.bytes[j]);
        }
      }
    }
  }
  index1.push_back(INT_MAX);
  index2.push_back(sz1 + sz2 + data.size());

  size_t size = sz1 + sz2 + data.size();
  uint8* meta = (uint8*)malloc(size);
  memcpy(meta, &index1[0], sz1);
  memcpy(meta + sz1, &index2[0], sz2);
  memcpy(meta + sz1 + sz2, &data[0], data.size());
  target.setBcMeta(meta, size);
  free(meta);
}

EmitterVisitor::EmitterVisitor(UnitEmitter& ue)
  : m_ue(ue), m_curFunc(ue.getMain()), m_evalStackIsUnknown(false),
    m_actualStackHighWater(0), m_fdescHighWater(0), m_closureCounter(0) {
  m_prevOpcode = OpLowInvalid;
  m_evalStack.m_actualStackHighWaterPtr = &m_actualStackHighWater;
  m_evalStack.m_fdescHighWaterPtr = &m_fdescHighWater;
}

EmitterVisitor::~EmitterVisitor() {
  for (LabelMap::const_iterator it = m_methLabels.begin();
       it != m_methLabels.end(); it++) {
    delete it->second;
  }
  // If a fatal occurs during emission, some extra cleanup is necessary.
  for (std::deque<ExnHandlerRegion*>::const_iterator it = m_exnHandlers.begin();
       it != m_exnHandlers.end(); ++it) {
    delete *it;
  }
}

bool EmitterVisitor::checkIfStackEmpty(const char* forInstruction) const {
  if (m_evalStack.empty()) {
    InvariantViolation("Emitter tried to emit a %s instruction when the "
                       "evaluation stack is empty (at offset %d)",
                       forInstruction,
                       m_ue.bcPos());
    return true;
  }
  return false;
}

void EmitterVisitor::unexpectedStackSym(char sym, const char* where) const {
  InvariantViolation("Emitter encountered an unexpected StackSym \"%s\""
                     " in %s() (at offset %d)",
                     StackSym::ToString(sym).c_str(),
                     where,
                     m_ue.bcPos());
}

void EmitterVisitor::popEvalStack(char expected, int arg, int pos) {
  // Pop a value off of the evaluation stack, and verify that it
  // matches the specified symbolic flavor
  if (m_evalStack.size() == 0) {
    InvariantViolation("Emitter emitted an instruction that tries to consume "
                       "a value from the stack when the stack is empty "
                       "(expected symbolic flavor \"%s\" at offset %d)",
                       StackSym::ToString(expected).c_str(),
                       m_ue.bcPos());
    return;
  }

  if (arg >= 0 && pos >= 0 &&
      (expected == StackSym::C || expected == StackSym::R)) {
    m_metaInfo.addKnownDataType(m_evalStack.getKnownType(),
                                m_evalStack.isTypePredicted(),
                                pos, false, arg);
  }

  char sym = m_evalStack.top();
  char actual = StackSym::GetSymFlavor(sym);
  m_evalStack.pop();
  if (actual != expected) {
    InvariantViolation(
      "Emitter emitted an instruction that tries to consume a "
      "value from the stack when the top of the stack does not "
      "match the symbolic flavor that the instruction expects "
      "(expected symbolic flavor \"%s\", actual symbolic flavor \"%s\" "
      "at offset %d)",
      StackSym::ToString(expected).c_str(),
      StackSym::ToString(actual).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::popSymbolicLocal(Opcode op, int arg, int pos) {
  // Special case for instructions that consume the loc below the
  // top.
  int belowTop = -1;
  if (op == OpCGetL3) {
    belowTop = 3;
  } else if (op == OpCGetL2) {
    belowTop = 2;
  }

  if (belowTop != -1) {
    char symFlavor = StackSym::GetSymFlavor(
      m_evalStack.get(m_evalStack.size() - belowTop));
    if (symFlavor != StackSym::L) {
      InvariantViolation("Operation tried to remove a local below the top of"
                         " the symbolic stack but instead found \"%s\"",
                         StackSym::ToString(symFlavor).c_str());
    }
    m_evalStack.consumeBelowTop(belowTop - 1);
  } else {
    if (arg >= 0 && pos >= 0) {
      m_metaInfo.addKnownDataType(m_evalStack.getKnownType(),
                                  m_evalStack.isTypePredicted(),
                                  pos, false, arg);
    }
    popEvalStack(StackSym::L);
  }
}

void EmitterVisitor::popEvalStackLMany() {
  while (!m_evalStack.empty()) {
    char sym = m_evalStack.top();
    char symFlavor = StackSym::GetSymFlavor(sym);
    char marker = StackSym::GetMarker(sym);
    if (marker == StackSym::E || marker == StackSym::P) {
      if (symFlavor != StackSym::C && symFlavor != StackSym::L &&
          symFlavor != StackSym::T && symFlavor != StackSym::I) {
        InvariantViolation(
          "Emitter emitted an instruction that tries to consume "
          "a value from the stack when the top of the stack "
          "does not match the symbolic flavor that the instruction "
          "expects (expected symbolic flavor \"C\", \"L\", \"T\", or \"I\", "
          "actual symbolic flavor \"%s\" at offset %d)",
          StackSym::ToString(symFlavor).c_str(),
          m_ue.bcPos());
      }
    } else if (marker == StackSym::W) {
      if (symFlavor != StackSym::None) {
        InvariantViolation(
          "Emitter emitted an instruction that tries to consume "
          "a value from the stack when the top of the stack "
          "does not match the symbolic flavor that the instruction "
          "expects (expected symbolic flavor \"None\", actual "
          "symbolic flavor \"%s\" at offset %d)",
          StackSym::ToString(symFlavor).c_str(),
          m_ue.bcPos());
      }
    } else if (marker == StackSym::M) {
      ASSERT(symFlavor == StackSym::A);
    } else {
      break;
    }
    m_evalStack.pop();
  }

  if (m_evalStack.empty()) {
    InvariantViolation("Emitter emitted an instruction that tries to consume "
                       "a value from the stack when the stack is empty "
                       "(at offset %d)",
                       m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  char symFlavor = StackSym::GetSymFlavor(sym);
  m_evalStack.pop();
  if (symFlavor != StackSym::C && symFlavor != StackSym::L &&
      symFlavor != StackSym::R && symFlavor != StackSym::H) {
    InvariantViolation(
      "Emitter emitted an instruction that tries to consume a "
      "value from the stack when the top of the stack does not "
      "match the symbolic flavor that the instruction expects "
      "(expected symbolic flavor \"C\", \"L\", \"R\", or \"H\", actual "
      "symbolic flavor \"%s\" at offset %d)",
      StackSym::ToString(symFlavor).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::popEvalStackMany(int len, char symFlavor) {
  for (int i = 0; i < len; ++i) {
    popEvalStack(symFlavor);
  }
}

void EmitterVisitor::pushEvalStack(char symFlavor) {
  // Push a value from the evaluation stack with the specified
  // symbolic flavor
  m_evalStack.push(symFlavor);
}

void EmitterVisitor::peekEvalStack(char expected, int depthActual) {
  int posActual = (m_evalStack.sizeActual() - depthActual - 1);
  if (posActual >= 0 && posActual < (int)m_evalStack.sizeActual()) {
    char sym = m_evalStack.getActual(posActual);
    char actual = StackSym::GetSymFlavor(sym);
    if (actual != expected) {
      InvariantViolation(
        "Emitter emitted an instruction that tries to consume a "
        "value from the stack whose symbolic flavor does not match "
        "the symbolic flavor that the instruction expects (expected "
        "symbolic flavor \"%s\", actual symbolic flavor \"%s\" at "
        "offset %d)",
        StackSym::ToString(expected).c_str(),
        StackSym::ToString(actual).c_str(),
        m_ue.bcPos());
    }
  }
}

void EmitterVisitor::pokeEvalStack(char symFlavor, int depthActual) {
  int sizeActual = m_evalStack.sizeActual();
  int posActual = sizeActual - depthActual - 1;
  if (posActual >= 0 && posActual < sizeActual) {
    m_evalStack.setActual(posActual, symFlavor);
  }
}

/*
 * Prior to making any changes to the evaluation stack in between
 * instructions, this function should be called.
 *
 * What this handles is recording the evaluation stack state at
 * instruction boundaries so that jumps know what their stack state
 * will be at the destination.
 *
 * When m_evalStackIsUnknown, it means we have hit a place in the
 * bytecode where the offset cannot be reached via fallthrough, but no
 * forward jumps targeted it either.  In this case, the stack must be
 * empty, and we need to record that this is the case so that later
 * backward jumps can check this is the case at their jump site.
 */
void EmitterVisitor::prepareEvalStack() {
  if (m_evalStackIsUnknown) {
    if (!m_evalStack.empty()) {
      InvariantViolation("Emitter expected to have an empty evaluation "
                         "stack because the eval stack was unknown, but"
                         "it was non-empty.");
      return;
    }
    // Record that we are assuming that the eval stack is empty
    recordJumpTarget(m_ue.bcPos(), m_evalStack);
    m_evalStackIsUnknown = false;
  }
}

void EmitterVisitor::recordJumpTarget(Offset target,
                                      const SymbolicStack& evalStack) {
  if (target == InvalidAbsoluteOffset) {
    InvariantViolation(
      "Offset passed to EmitterVisitor::recordJumpTarget was invalid");
  }
  hphp_hash_map<Offset, SymbolicStack>::iterator it =
    m_jumpTargetEvalStacks.find(target);
  if (it == m_jumpTargetEvalStacks.end()) {
    m_jumpTargetEvalStacks[target] = evalStack;
    return;
  }
  checkJmpTargetEvalStack(evalStack, it->second);
}

void EmitterVisitor::restoreJumpTargetEvalStack() {
  m_evalStack.clear();
  hphp_hash_map<Offset, SymbolicStack>::iterator it =
    m_jumpTargetEvalStacks.find(m_ue.bcPos());
  if (it == m_jumpTargetEvalStacks.end()) {
    m_evalStackIsUnknown = true;
    return;
  }
  m_evalStack = it->second;
}

bool EmitterVisitor::isJumpTarget(Offset target) {
  // Returns true iff one of the following conditions is true:
  //   1) We have seen an instruction that jumps to the specified offset
  //   2) We know of a Label that has been set to the specified offset
  //   3) We have seen a try region that ends at the specified offset
  hphp_hash_map<Offset, SymbolicStack>::iterator it =
    m_jumpTargetEvalStacks.find(target);
  return (it != m_jumpTargetEvalStacks.end());
}

#define CONTROL_BODY(brk, cnt, brkH, cntH) \
  ControlTargetPusher _cop(this, -1, brk, cnt, brkH, cntH)
#define FOREACH_BODY(itId, brk, cnt, brkH, cntH) \
  ControlTargetPusher _cop(this, itId, brk, cnt, brkH, cntH)

class IterFreeThunklet : public Thunklet {
public:
  IterFreeThunklet(Id iterId) : m_id(iterId) {}
  virtual void emit(Emitter& e) {
    e.IterFree(m_id);
    e.Unwind();
  }
private:
  Id m_id;
};

/**
 * A thunklet for the fault region protecting a silenced (@) expression.
 */
class RestoreErrorReportingThunklet : public Thunklet {
public:
  RestoreErrorReportingThunklet(Id loc) : m_oldLevelLoc(loc) {}
  virtual void emit(Emitter& e) {
    e.getEmitterVisitor().emitRestoreErrorReporting(e, m_oldLevelLoc);
    e.Unwind();
  }
private:
  Id m_oldLevelLoc;
};

class UnsetUnnamedLocalThunklet : public Thunklet {
public:
  UnsetUnnamedLocalThunklet(Id loc) : m_loc(loc) {}
  virtual void emit(Emitter& e) {
    e.getEmitterVisitor().emitVirtualLocal(m_loc);
    e.getEmitterVisitor().emitUnset(e);
    e.Unwind();
  }
private:
  Id m_loc;
};

/**
 * Helper to deal with emitting list assignment and keeping track of some
 * associated info.  A list assignment can be thought of as a list of "index
 * chains"; that is, sequences of indices that should be accessed for each
 * bottom-level expression in the list assignment.  We recursively walk down the
 * LHS, building up index chains and copying them into the top-level list as we
 * reach the leaves of the tree.
 */
void EmitterVisitor::visitListAssignmentLHS(Emitter& e, ExpressionPtr exp,
                                            IndexChain& indexChain,
                                            std::vector<IndexChain*>& all) {
  if (!exp) {
    // Empty slot
    return;
  }

  if (exp->is(Expression::KindOfListAssignment)) {
    // Nested assignment
    ListAssignmentPtr la = static_pointer_cast<ListAssignment>(exp);
    ExpressionListPtr lhs = la->getVariables();
    int n = lhs->getCount();
    for (int i = 0; i < n; ++i) {
      indexChain.push_back(i);
      visitListAssignmentLHS(e, (*lhs)[i], indexChain, all);
      indexChain.pop_back();
    }
  } else {
    // Reached a "leaf".  Lock in this index chain and deal with this exp.
    ASSERT(!indexChain.empty());
    all.push_back(new IndexChain(indexChain));
    visit(exp);
    emitClsIfSPropBase(e);
  }
}

/**
 * visitIfCondition() serves as a helper method for visiting the condition
 * of an "if" statement. This function recursively visits each node in an
 * expression, keeping track of where to jump if an "and" or "or" expression
 * short circuits. If an expression other than "and", "or", or "not" is
 * encountered, this method calls EmitterVisitor::visit() to handle the
 * expression.
 */
void EmitterVisitor::visitIfCondition(
  ExpressionPtr cond, Emitter& e, Label& tru, Label& fals,
  bool truFallthrough) {

  BinaryOpExpressionPtr binOpNode(
    dynamic_pointer_cast<BinaryOpExpression>(cond));

  if (binOpNode) {
    int op = binOpNode->getOp();
    // Short circuit && and ||
    if (op == T_LOGICAL_OR || op == T_LOGICAL_AND ||
        op == T_BOOLEAN_OR || op == T_BOOLEAN_AND) {
      bool isOr = (op == T_LOGICAL_OR || op == T_BOOLEAN_OR);
      Label localLabel;
      ExpressionPtr lhs = binOpNode->getExp1();
      if (isOr) {
        Emitter lhsEmitter(lhs, m_ue, *this);
        visitIfCondition(lhs, lhsEmitter, tru, localLabel, false);
        // falls through if that condition was false
      } else {
        Emitter lhsEmitter(lhs, m_ue, *this);
        visitIfCondition(lhs, lhsEmitter, localLabel, fals, true);
        // falls through if that condition was true
      }
      if (localLabel.isUsed()) {
        localLabel.set(e);
      }
      if (currentPositionIsReachable()) {
        ExpressionPtr rhs = binOpNode->getExp2();
        Emitter rhsEmitter(rhs, m_ue, *this);
        visitIfCondition(rhs, rhsEmitter, tru, fals, truFallthrough);
      }
      return;
    }
  }

  UnaryOpExpressionPtr unOpNode(dynamic_pointer_cast<UnaryOpExpression>(cond));
  if (unOpNode) {
    int op = unOpNode->getOp();
    // Logical not
    if (op == '!') {
      ExpressionPtr val = unOpNode->getExpression();
      Emitter valEmitter(val, m_ue, *this);
      visitIfCondition(val, valEmitter, fals, tru, !truFallthrough);
      return;
    }
  }

  Variant val;
  if (cond->getScalarValue(val)) {
    if (truFallthrough) {
      if (!val.toBoolean()) e.Jmp(fals);
    } else {
      if (val.toBoolean()) e.Jmp(tru);
    }
    return;
  }

  visit(cond);
  emitConvertToCell(e);
  if (truFallthrough) {
    e.JmpZ(fals);
  } else {
    e.JmpNZ(tru);
  }
}

void EmitterVisitor::assignLocalVariableIds(FunctionScopePtr fs) {
  VariableTablePtr variables = fs->getVariables();
  std::vector<std::string> localNames;
  variables->getLocalVariableNames(localNames);
  for (int i = 0; i < (int)localNames.size(); ++i) {
    StringData* nLiteral = StringData::GetStaticString(localNames[i].c_str());
    m_curFunc->allocVarId(nLiteral);
  }
}

void EmitterVisitor::visit(FileScopePtr file) {
  const std::string& filename = file->getName();
  m_ue.setFilepath(StringData::GetStaticString(filename));

  FunctionScopePtr func(file->getPseudoMain());
  if (!func) return;

  m_file = file;
  // Assign ids to all of the local variables eagerly. This gives us the
  // nice property that all named local variables will be assigned ids
  // 0 through k-1, while any unnamed local variable will have an id >= k.
  assignLocalVariableIds(func);

  AnalysisResultPtr ar(file->getContainingProgram());
  ASSERT(ar);
  MethodStatementPtr m(dynamic_pointer_cast<MethodStatement>(func->getStmt()));
  if (!m) return;
  StatementListPtr stmts(m->getStmts());
  if (!stmts) return;

  Emitter e(m, m_ue, *this);

  Label ctFatal;
  Label ctFatalWithPop;
  int i, nk = stmts->getCount();
  CONTROL_BODY(ctFatal, ctFatal, ctFatalWithPop, ctFatalWithPop);
  for (i = 0; i < nk; i++) {
    StatementPtr s = (*stmts)[i];
    if (MethodStatementPtr meth = dynamic_pointer_cast<MethodStatement>(s)) {
      // Create label for use with fast calls
      const StringData* methName =
        StringData::GetStaticString(meth->getOriginalName());
      m_methLabels[methName] = new Label();
      // Emit afterwards
      postponeMeth(meth, NULL, true);
    }
  }
  {
    FunctionScopePtr fsp = m->getFunctionScope();
    if (fsp->needsLocalThis()) {
      static const StringData* thisStr = StringData::GetStaticString("this");
      Id thisId = m_curFunc->lookupVarId(thisStr);
      e.InitThisLoc(thisId);
    }
    FuncFinisher ff(this, e, m_curFunc);
    TypedValue mainReturn;
    mainReturn.m_type = KindOfInvalid;
    bool notMergeOnly = false;
    PreClass::Hoistable allHoistable = PreClass::AlwaysHoistable;
    for (i = 0; i < nk; i++) {
      StatementPtr s = (*stmts)[i];
      switch (s->getKindOf()) {
        case Statement::KindOfMethodStatement:
        case Statement::KindOfFunctionStatement:
          break;
        case Statement::KindOfInterfaceStatement:
        case Statement::KindOfClassStatement: {
          // Handle classes directly here, since only top-level classes are
          // hoistable.
          ClassScopePtr cNode = s->getClassScope();
          PreClass::Hoistable h = emitClass(e, cNode, true);
          if (h < allHoistable) allHoistable = h;
          break;
        }
        case Statement::KindOfReturnStatement:
          if (mainReturn.m_type != KindOfInvalid) break;
          if (notMergeOnly) {
            tvWriteUninit(&mainReturn);
            m_ue.returnSeen();
            goto fail;
          } else {
            ReturnStatementPtr r(static_pointer_cast<ReturnStatement>(s));
            Variant v(Variant::nullInit);
            if (r->getRetExp() &&
                !r->getRetExp()->getScalarValue(v)) {
              tvWriteUninit(&mainReturn);
              goto fail;
            }
            if (v.isString()) {
              v = String(StringData::GetStaticString(v.asCStrRef().get()));
            } else if (v.isArray()) {
              v = Array(ArrayData::GetScalarArray(v.asCArrRef().get()));
            } else {
              ASSERT(!IS_REFCOUNTED_TYPE(v.getType()));
            }
            mainReturn = *v.getTypedAccessor();
            m_ue.returnSeen();
          }
          break;
        case Statement::KindOfExpStatement:
          if (mainReturn.m_type == KindOfInvalid) {
            ExpressionPtr e =
              static_pointer_cast<ExpStatement>(s)->getExpression();
            switch (e->getKindOf()) {
              case Expression::KindOfSimpleFunctionCall: {
                SimpleFunctionCallPtr func =
                  static_pointer_cast<SimpleFunctionCall>(e);
                StringData *name;
                TypedValue tv;
                if (func->isSimpleDefine(&name, &tv)) {
                  m_ue.pushMergeableDef(UnitMergeKindDefine, name, tv);
                  visit(s);
                  continue;
                }
                break;
              }
              case Expression::KindOfAssignmentExpression: {
                AssignmentExpressionPtr ae(
                  static_pointer_cast<AssignmentExpression>(e));
                StringData *name;
                TypedValue tv;
                if (ae->isSimpleGlobalAssign(&name, &tv)) {
                  m_ue.pushMergeableDef(UnitMergeKindGlobal, name, tv);
                  visit(s);
                  continue;
                }
                break;
              }
              case Expression::KindOfIncludeExpression: {
                IncludeExpressionPtr inc =
                  static_pointer_cast<IncludeExpression>(e);
                if (inc->isReqLit()) {
                  if (FileScopeRawPtr f = inc->getIncludedFile(ar)) {
                    if (StatementListPtr sl = f->getStmt()) {
                      FunctionScopeRawPtr ps DEBUG_ONLY =
                        sl->getFunctionScope();
                      ASSERT(ps && ps->inPseudoMain());
                      UnitMergeKind kind = inc->isPrivateScope() ?
                        (inc->isDocumentRoot() ?
                         UnitMergeKindReqMod : UnitMergeKindReqSrc) :
                        UnitMergeKindReqDoc;
                      m_ue.pushMergeableInclude(
                        kind,
                        StringData::GetStaticString(inc->includePath()));
                      visit(s);
                      continue;
                    }
                  }
                }
                break;
              }
              default:
                break;
            }
          } // fall through
        default:
          if (mainReturn.m_type != KindOfInvalid) break;
          // fall through
        fail:
          notMergeOnly = true;
          visit(s);
      }
    }
    if (mainReturn.m_type == KindOfInvalid) {
      tvWriteUninit(&mainReturn);
      tvAsVariant(&mainReturn) = 1;
    }
    // Use _count as a flag for VMExecutionContext::evalUnit
    // since its otherwise unused
    mainReturn._count = !notMergeOnly;
    m_ue.setMainReturn(&mainReturn);
    // If the exitHnd label was used, we need to emit some extra code
    // to handle stray breaks
    Label exit;
    if (ctFatal.isUsed() || ctFatalWithPop.isUsed()) {
      e.Jmp(exit);
      if (ctFatalWithPop.isUsed()) {
        ctFatalWithPop.set(e);
        e.PopC();  // the number of levels to jump up
      }
      if (ctFatal.isUsed()) {
        ctFatal.set(e);
      }
      static const StringData* msg =
        StringData::GetStaticString("Cannot break/continue");
      e.String(msg);
      e.Fatal();
    }
    if (exit.isUsed()) {
      exit.set(e);
    }
    // Pseudo-main returns the integer value 1 by default. If the
    // current position in the bytecode is reachable, emit code to
    // return 1.
    if (currentPositionIsReachable()) {
      e.Int(1);
      e.RetC();
    }
  }

  if (!m_evalStack.empty()) {
    InvariantViolation("Eval stack was not empty as expected before "
                       "emitPostponed* phase");
  }

  // Method bodies
  emitPostponedMeths();
  emitPostponedCtors();
  emitPostponedPinits();
  emitPostponedSinits();
  emitPostponedCinits();
  emitPostponedClosureCtors();
  Peephole peephole(m_ue, m_metaInfo);
  m_metaInfo.setForUnit(m_ue);
}

static StringData* getClassName(ExpressionPtr e) {
  ClassScopeRawPtr cls;
  if (e->isThis()) {
    cls = e->getOriginalClass();
    if (TypePtr t = e->getAssertedType()) {
      if (t->isSpecificObject()) {
        AnalysisResultConstPtr ar = e->getScope()->getContainingProgram();
        ClassScopeRawPtr c2 = t->getClass(ar, e->getScope());
        if (c2 && c2->derivesFrom(ar, cls->getName(), true, false)) {
          cls = c2;
        }
      }
    }
  } else if (TypePtr t = e->getActualType()) {
    if (t->isSpecificObject()) {
      cls = t->getClass(e->getScope()->getContainingProgram(), e->getScope());
    }
  }
  if (cls && !cls->isTrait()) {
    return StringData::GetStaticString(cls->getOriginalName());
  }
  return NULL;
}

static DataType getPredictedDataType(ExpressionPtr expr) {
  if (!expr->maybeInited()) {
    return KindOfUninit;
  }
  // Note that expr->isNonNull() may be false,
  // but that's ok since this is just a prediction.
  TypePtr act = expr->getActualType();
  if (!act) {
    return KindOfUnknown;
  }
  return act->getDataType();
}

void EmitterVisitor::fixReturnType(Emitter& e, FunctionCallPtr fn,
                                   bool isBuiltinCall) {
  int ref = -1;
  if (fn->hasAnyContext(Expression::RefValue |
                        Expression::DeepReference |
                        Expression::LValue |
                        Expression::OprLValue |
                        Expression::UnsetContext)) {
    return;
  }
  bool voidReturn = false;
  if (fn->isValid() && fn->getFuncScope()) {
    ref = fn->getFuncScope()->isRefReturn();
    if (!(fn->getFuncScope()->getReturnType())) {
      voidReturn = true;
    }
  } else if (!fn->getName().empty()) {
    FunctionScope::FunctionInfoPtr fi =
      FunctionScope::GetFunctionInfo(fn->getName());
    if (!fi || !fi->getMaybeRefReturn()) ref = false;
  }

  if (!fn->isUnused() &&
      ref >= 0 &&
      (!ref || !fn->hasAnyContext(Expression::AccessContext |
                                  Expression::ObjectContext))) {
    /* we dont support V in M-vectors, so leave it as an R in that
       case */
    ASSERT(m_evalStack.get(m_evalStack.size() - 1) == StackSym::R);
    Offset cur = m_ue.bcPos();
    if (ref) {
      e.BoxR();
    } else {
      e.UnboxR();
    }
    m_metaInfo.add(cur, Unit::MetaInfo::NopOut, false, 0, 0);
  }

  if (voidReturn) {
    m_evalStack.setKnownType(KindOfNull, false /* inferred */);
    m_evalStack.setNotRef();
  } else if (!ref) {
    DataType dt = getPredictedDataType(fn);
    if (dt != KindOfUnknown) {
      if (isBuiltinCall) {
        switch (dt) {
          case KindOfBoolean:
          case KindOfInt64:
          case KindOfDouble: /* inferred */
                             m_evalStack.setKnownType(dt, false);
                             break;
          default:           /* predicted */
                             m_evalStack.setKnownType(dt, true);
                             break;
        }
      } else {
        m_evalStack.setKnownType(dt, true /* predicted */);
      }
    }
    m_evalStack.setNotRef();
  }
}

void EmitterVisitor::visitKids(ConstructPtr c) {
  for (int i = 0, nk = c->getKidCount(); i < nk; i++) {
    ConstructPtr kid(c->getNthKid(i));
    visit(kid);
  }
}

/*
 * isTupleInit() returns true if this expression list looks like a vanilla
 * tuple-shaped array with no keys, no ref values; e.g. array(x,y,z),
 * where we can NewTuple to create the array.  In that case the elements are
 * pushed on the stack, so we arbitrarily limit this to a small multiple of
 * HphpArray::SmallSize (12).
 */
bool isTupleInit(ExpressionPtr init_expr, int* cap) {
  if (init_expr->getKindOf() != Expression::KindOfExpressionList) return false;
  ExpressionListPtr el = static_pointer_cast<ExpressionList>(init_expr);
  int n = el->getCount();
  if (n < 1 || n > int(4 * HphpArray::SmallSize)) return false;
  for (int i = 0, n = el->getCount(); i < n; ++i) {
    ExpressionPtr ex = (*el)[i];
    if (ex->getKindOf() != Expression::KindOfArrayPairExpression) return false;
    ArrayPairExpressionPtr ap = static_pointer_cast<ArrayPairExpression>(ex);
    if (ap->getName() != NULL || ap->isRef()) return false;
  }
  *cap = n;
  return true;
}

bool EmitterVisitor::visit(ConstructPtr node) {
  bool ret = visitImpl(node);
  if (!Option::WholeProgram || !ret) return ret;
  ExpressionPtr e = boost::dynamic_pointer_cast<Expression>(node);
  if (!e || e->isScalar()) return ret;
  DataType dt = KindOfUnknown;
  if (!e->maybeInited()) {
    dt = KindOfUninit;
  } else if (node->isNonNull()) {
    TypePtr act = e->getActualType();
    if (!act) return ret;
    dt = act->getDataType();
    if (dt == KindOfUnknown) return ret;
  } else {
    return ret;
  }
  char sym = m_evalStack.top();
  if (StackSym::GetMarker(sym)) return ret;
  switch (StackSym::GetSymFlavor(sym)) {
    case StackSym::C:
      m_evalStack.setNotRef();
      m_evalStack.setKnownType(dt);
      break;
    case StackSym::L:
      if (dt == KindOfUninit ||
          !e->maybeRefCounted() ||
          (e->is(Expression::KindOfSimpleVariable) &&
           !static_pointer_cast<SimpleVariable>(e)->couldBeAliased())) {
        m_evalStack.setNotRef();
      }
      m_evalStack.setKnownType(dt);
      break;
  }

  return ret;
}

bool EmitterVisitor::visitImpl(ConstructPtr node) {
  if (!node) return false;

  Emitter e(node, m_ue, *this);

  if (StatementPtr s = dynamic_pointer_cast<Statement>(node)) {
    switch (s->getKindOf()) {
      case Statement::KindOfBlockStatement:
      case Statement::KindOfStatementList:
        visitKids(node);
        return false;

      case Statement::KindOfContinueStatement:
      case Statement::KindOfBreakStatement: {
        BreakStatementPtr bs(static_pointer_cast<BreakStatement>(s));
        int64 n = bs->getDepth();
        if (n == 1) {
          // Plain old "break;" or "continue;"
          if (m_contTargets.empty()) {
            static const StringData* msg =
              StringData::GetStaticString("Cannot break/continue 1 level");
            e.String(msg);
            e.Fatal();
            return false;
          }
          if (bs->is(Statement::KindOfBreakStatement)) {
            if (m_contTargets.front().m_itId != -1) {
              e.IterFree(m_contTargets.front().m_itId);
            }
            e.Jmp(m_contTargets.front().m_brkTarg);
          } else {
            e.Jmp(m_contTargets.front().m_cntTarg);
          }
          return false;
        }

        // Dynamic break/continue.
        if (n == 0) {
          // Depth can't be statically determined.
          visit(bs->getNthKid(0));
          emitConvertToCell(e);
        } else {
          // Dynamic break/continue with statically known depth.
          if (n > (int64)m_contTargets.size()) {
            std::ostringstream msg;
            msg << "Cannot break/continue " << n << " levels";
            e.String(StringData::GetStaticString(msg.str()));
            e.Fatal();
            return false;
          }
          e.Int(n);
        }
        if (bs->is(Statement::KindOfBreakStatement)) {
          e.Jmp(m_contTargets.front().m_brkHand);
        } else {
          e.Jmp(m_contTargets.front().m_cntHand);
        }
        return false;
      }

      case Statement::KindOfDoStatement: {
        Label top(e);
        Label condition;
        Label exit;
        Label brkHand;
        Label cntHand;
        {
          CONTROL_BODY(exit, condition, brkHand, cntHand);
          visit(node->getNthKid(DoStatement::BodyStmt));
        }
        condition.set(e);
        {
          ExpressionPtr c(static_pointer_cast<Expression>(
                            node->getNthKid(DoStatement::CondExpr)));
          Emitter condEmitter(c, m_ue, *this);
          visitIfCondition(c, condEmitter, top, exit, false);
        }

        if (brkHand.isUsed() || cntHand.isUsed()) {
          e.Jmp(exit);
          emitBreakHandler(e, exit, condition, brkHand, cntHand);
        }
        if (exit.isUsed()) exit.set(e);
        return false;
      }

      case Statement::KindOfCaseStatement: {
        // Should never be called. Handled in visitSwitch.
        not_reached();
      }

      case Statement::KindOfCatchStatement: {
        // Store the current exception object in the appropriate local variable
        CatchStatementPtr cs(static_pointer_cast<CatchStatement>(node));
        StringData* vName =
          StringData::GetStaticString(cs->getVariable()->getName());
        Id i = m_curFunc->lookupVarId(vName);
        emitVirtualLocal(i);
        e.Catch();
        emitSet(e);
        emitPop(e);
        visit(cs->getStmt());
        return false;
      }

      case Statement::KindOfEchoStatement: {
        EchoStatementPtr es(static_pointer_cast<EchoStatement>(node));
        ExpressionListPtr exps = es->getExpressionList();
        int count = exps->getCount();
        for (int i = 0; i < count; i++) {
          visit((*exps)[i]);
          emitConvertToCell(e);
          e.Print();
          e.PopC();
        }
        return false;
      }

      case Statement::KindOfExpStatement: {
        ASSERT(node->getKidCount() == 1);
        if (visit(node->getNthKid(0))) {
          emitPop(e);
        }
        return false;
      }

      case Statement::KindOfForStatement: {
        if (visit(node->getNthKid(ForStatement::InitExpr))) {
          emitPop(e);
        }
        Label preCond(e);
        Label preInc;
        Label fail;
        Label brkHand;
        Label cntHand;
        if (ConstructPtr n = node->getNthKid(ForStatement::CondExpr)) {
          Label tru;
          Emitter condEmitter(static_pointer_cast<Expression>(n),
                              m_ue, *this);
          visitIfCondition(static_pointer_cast<Expression>(n),
                           condEmitter, tru, fail, true);
          if (tru.isUsed()) tru.set(e);
        }
        {
          CONTROL_BODY(fail, preInc, brkHand, cntHand);
          visit(node->getNthKid(ForStatement::BodyStmt));
        }
        preInc.set(e);
        if (visit(node->getNthKid(ForStatement::IncExpr))) {
          emitPop(e);
        }
        e.Jmp(preCond);
        emitBreakHandler(e, fail, preInc, brkHand, cntHand);
        if (fail.isUsed()) fail.set(e);
        return false;
      }

      case Statement::KindOfForEachStatement: {
        ForEachStatementPtr fe(static_pointer_cast<ForEachStatement>(node));
        emitForeach(e, fe);
        return false;
      }

      case Statement::KindOfGlobalStatement: {
        ExpressionListPtr vars(
          static_pointer_cast<GlobalStatement>(node)->getVars());
        for (int i = 0, n = vars->getCount(); i < n; i++) {
          ExpressionPtr var((*vars)[i]);
          if (var->is(Expression::KindOfSimpleVariable)) {
            SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(var));
            if (sv->isSuperGlobal()) {
              continue;
            }
            StringData* nLiteral = StringData::GetStaticString(sv->getName());
            Id i = m_curFunc->lookupVarId(nLiteral);
            emitVirtualLocal(i);
            e.String(nLiteral);
            markGlobalName(e);
            e.VGetG();
            emitBind(e);
            e.PopV();
          } else if (var->is(Expression::KindOfDynamicVariable)) {
            // global $<exp> =& $GLOBALS[<exp>]
            DynamicVariablePtr dv(static_pointer_cast<DynamicVariable>(var));
            // Get the variable name as a cell, for the LHS
            visit(dv->getSubExpression());
            emitConvertToCell(e);
            // Copy the variable name, for indexing into $GLOBALS
            e.Dup();
            markNameSecond(e);
            markGlobalName(e);
            e.VGetG();
            e.BindN();
            e.PopV();
          } else {
            not_implemented();
          }
        }
        return false;
      }

      case Statement::KindOfIfStatement: {
        IfStatementPtr ifp(static_pointer_cast<IfStatement>(node));
        StatementListPtr branches(ifp->getIfBranches());
        int nb = branches->getCount();
        Label done;
        for (int i = 0; i < nb; i++) {
          IfBranchStatementPtr branch(
            static_pointer_cast<IfBranchStatement>((*branches)[i]));
          Label fals;
          if (branch->getCondition()) {
            Label tru;
            Emitter condEmitter(branch->getCondition(), m_ue, *this);
            visitIfCondition(branch->getCondition(), condEmitter,
                             tru, fals, true);
            if (tru.isUsed()) {
              tru.set(e);
            }
          }
          visit(branch->getStmt());
          if (currentPositionIsReachable() && i + 1 < nb) {
            e.Jmp(done);
          }
          if (fals.isUsed()) {
            fals.set(e);
          }
        }
        if (done.isUsed()) {
          done.set(e);
        }
        return false;
      }

      case Statement::KindOfIfBranchStatement:
        not_reached(); // handled by KindOfIfStatement

      case Statement::KindOfReturnStatement: {
        ReturnStatementPtr r(static_pointer_cast<ReturnStatement>(node));
        bool retV = false;
        if (m_curFunc->isGenerator()) {
          e.ContExit();
          return false;
        }
        if (visit(r->getRetExp())) {
          if (r->getRetExp()->getContext() & Expression::RefValue) {
            emitConvertToVar(e);
            emitFreePendingIters(e);
            retV = true;
          } else {
            emitConvertToCell(e);
            emitFreePendingIters(e);
          }
        } else {
          emitFreePendingIters(e);
          e.Null();
        }
        if (r->isGuarded()) {
          m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                      false, 0, 0);
        }

        for (auto& l : r->nonRefcountedLocals()) {
          auto v = m_curFunc->lookupVarId(StringData::GetStaticString(l));
          m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NonRefCounted,
                         false, 0, v);
        }
        if (retV) {
          e.RetV();
        } else {
          e.RetC();
        }
        return false;
      }

      case Statement::KindOfStaticStatement: {
        ExpressionListPtr vars(
          static_pointer_cast<StaticStatement>(node)->getVars());
        for (int i = 0, n = vars->getCount(); i < n; i++) {
          ExpressionPtr se((*vars)[i]);
          ASSERT(se->is(Expression::KindOfAssignmentExpression));
          AssignmentExpressionPtr ae(
            static_pointer_cast<AssignmentExpression>(se));
          ExpressionPtr var(ae->getVariable());
          ExpressionPtr value(ae->getValue());
          ASSERT(var->is(Expression::KindOfSimpleVariable));
          SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(var));
          StringData* name = StringData::GetStaticString(sv->getName());
          Id local = m_curFunc->lookupVarId(name);

          Func::SVInfo svInfo;
          svInfo.name = name;
          std::ostringstream os;
          CodeGenerator cg(&os, CodeGenerator::PickledPHP);
          AnalysisResultPtr ar(new AnalysisResult());
          value->outputPHP(cg, ar);
          svInfo.phpCode = StringData::GetStaticString(os.str());
          m_curFunc->addStaticVar(svInfo);

          if (value->isScalar()) {
            visit(value);
            emitConvertToCell(e);
            e.StaticLocInit(local, name);
          } else {
            Label done;
            e.StaticLoc(local, name);
            e.JmpNZ(done);

            emitVirtualLocal(local);
            visit(value);
            emitConvertToCell(e);
            emitSet(e);
            emitPop(e);

            done.set(e);
          }
        }
        return false;
      }

      case Statement::KindOfSwitchStatement: {
        SwitchStatementPtr sw(static_pointer_cast<SwitchStatement>(node));
        StatementListPtr cases(sw->getCases());
        if (!cases) {
          visit(sw->getExp());
          emitPop(e);
          return false;
        }
        uint ncase = cases->getCount();
        std::vector<Label> caseLabels(ncase);
        Label done;
        Label brkHand;
        Label cntHand;

        // There are two different ways this can go.  If the subject is a simple
        // variable, then we have to evaluate it every time we compare against a
        // case condition.  Otherwise, we evaluate it once and store it in an
        // unnamed local.  This is because (a) switch statements are equivalent
        // to a series of if-elses, and (b) Zend has some weird evaluation order
        // rules.  For example, "$a == ++$a" is true but "$a[0] == ++$a[0]" is
        // false.  In particular, if a case condition modifies the switch
        // subject, things behave differently depending on whether the subject
        // is a simple variable.
        ExpressionPtr subject = sw->getExp();
        bool simpleSubject = subject->is(Expression::KindOfSimpleVariable)
          && !static_pointer_cast<SimpleVariable>(subject)->getAlwaysStash();
        Id tempLocal = -1;
        Offset start = InvalidAbsoluteOffset;

        bool enabled = RuntimeOption::EnableEmitSwitch;
        SimpleFunctionCallPtr
          call(dynamic_pointer_cast<SimpleFunctionCall>(subject));
        bool isGenerator = call &&
          call->isCompilerCallToFunction("hphp_unpack_continuation");
        if (enabled && isGenerator) {
          emitContinuationSwitch(e, sw);
          return false;
        }

        SwitchState state;
        bool didSwitch = false;
        if (enabled) {
          DataType stype = analyzeSwitch(sw, state);
          if (stype != KindOfInvalid) {
            e.incStat(stype == KindOfInt64 ? Stats::Switch_Integer
                                           : Stats::Switch_String,
                      1);
            if (state.cases.empty()) {
              // If there are no non-default cases, evaluate the subject for
              // side effects and fall through. If there's a default case it
              // will be emitted immediately after this.
              visit(sw->getExp());
              emitPop(e);
            } else if (stype == KindOfInt64) {
              emitIntegerSwitch(e, sw, caseLabels, done, state);
            } else {
              ASSERT(IS_STRING_TYPE(stype));
              emitStringSwitch(e, sw, caseLabels, done, state);
            }
            didSwitch = true;
          }
        }
        if (!didSwitch) {
          e.incStat(Stats::Switch_Generic, 1);
          if (!simpleSubject) {
            // Evaluate the subject once and stash it in a local
            tempLocal = m_curFunc->allocUnnamedLocal();
            emitVirtualLocal(tempLocal);
            visit(subject);
            emitConvertToCell(e);
            emitSet(e);
            emitPop(e);
            start = m_ue.bcPos();
          }

          int defI = -1;
          for (uint i = 0; i < ncase; i++) {
            CaseStatementPtr c(static_pointer_cast<CaseStatement>((*cases)[i]));
            ExpressionPtr condition = c->getCondition();
            if (condition) {
              if (simpleSubject) {
                // Evaluate the subject every time.
                visit(subject);
                emitConvertToCellOrLoc(e);
                visit(condition);
                emitConvertToCell(e);
                emitConvertSecondToCell(e);
              } else {
                emitVirtualLocal(tempLocal);
                emitCGet(e);
                visit(condition);
                emitConvertToCell(e);
              }
              e.Eq();
              e.JmpNZ(caseLabels[i]);
            } else {
              // Default clause. The last one wins.
              defI = i;
            }
          }
          if (defI != -1) {
            e.Jmp(caseLabels[defI]);
          } else {
            e.Jmp(done);
          }
        }
        for (uint i = 0; i < ncase; i++) {
          caseLabels[i].set(e);
          CaseStatementPtr c(static_pointer_cast<CaseStatement>((*cases)[i]));
          CONTROL_BODY(done, done, brkHand, cntHand);
          visit(c->getStatement());
        }
        if (brkHand.isUsed() || cntHand.isUsed()) {
          e.Jmp(done);
          emitBreakHandler(e, done, done, brkHand, cntHand);
        }
        done.set(e);
        if (!didSwitch && !simpleSubject) {
          // Null out temp local, to invoke any needed refcounting
          ASSERT(tempLocal >= 0);
          ASSERT(start != InvalidAbsoluteOffset);
          newFaultRegion(start, m_ue.bcPos(),
                         new UnsetUnnamedLocalThunklet(tempLocal));
          emitVirtualLocal(tempLocal);
          emitUnset(e);
          m_curFunc->freeUnnamedLocal(tempLocal);
        }
        return false;
      }

      case Statement::KindOfThrowStatement: {
        visitKids(node);
        emitConvertToCell(e);
        e.Throw();
        return false;
      }

      case Statement::KindOfFinallyStatement: {
        return false;
      }

      case Statement::KindOfTryStatement: {
        if (!m_evalStack.empty()) {
          InvariantViolation(
            "Emitter detected that the evaluation stack is not empty "
            "at the beginning of a try region: %d", m_ue.bcPos());
        }
        Label after;
        TryStatementPtr ts = static_pointer_cast<TryStatement>(node);

        Offset start = m_ue.bcPos();
        visit(ts->getNthKid(0));
        // include the jump out of the try-catch block in the
        // exception handler address range
        e.Jmp(after);
        Offset end = m_ue.bcPos();

        if (!m_evalStack.empty()) {
          InvariantViolation("Emitter detected that the evaluation stack "
                             "is not empty at the end of a try region: %d",
                             end);
        }

        StatementListPtr catches = ts->getCatches();
        ExnHandlerRegion* r = new ExnHandlerRegion(start, end);
        m_exnHandlers.push_back(r);

        int n = catches->getCount();
        bool firstHandler = true;
        for (int i = 0; i < n; i++) {
          CatchStatementPtr c(static_pointer_cast<CatchStatement>
                              ((*catches)[i]));
          StringData* eName = StringData::GetStaticString(c->getClassName());

          // If there's already a catch of this class, skip; the first one wins
          if (r->m_names.find(eName) == r->m_names.end()) {
            // Don't let execution of the try body, or the previous catch body,
            // fall into here.
            if (!firstHandler) {
              e.Jmp(after);
            } else {
              firstHandler = false;
            }

            Label* label = new Label(e);
            r->m_names.insert(eName);
            r->m_catchLabels.push_back(std::pair<StringData*, Label*>(eName,
                                                                      label));
            visit(c);
          }
        }

        after.set(e);
        return false;
      }

      case Statement::KindOfUnsetStatement: {
        ExpressionListPtr exps(
          static_pointer_cast<UnsetStatement>(node)->getExps());
        for (int i = 0, n = exps->getCount(); i < n; i++) {
          visit((*exps)[i]);
          emitUnset(e);
        }
        return false;
      }

      case Statement::KindOfWhileStatement: {
        Label preCond(e);
        Label fail;
        Label brkHand;
        Label cntHand;
        {
          Label tru;
          ExpressionPtr c(static_pointer_cast<Expression>(
                            node->getNthKid(WhileStatement::CondExpr)));
          Emitter condEmitter(c, m_ue, *this);
          visitIfCondition(c, condEmitter, tru, fail, true);
          if (tru.isUsed()) tru.set(e);
        }
        {
          CONTROL_BODY(fail, preCond, brkHand, cntHand);
          visit(node->getNthKid(WhileStatement::BodyStmt));
        }
        e.Jmp(preCond);
        emitBreakHandler(e, fail, preCond, brkHand, cntHand);
        if (fail.isUsed()) fail.set(e);
        return false;
      }

      case Statement::KindOfInterfaceStatement:
      case Statement::KindOfClassStatement: {
        emitClass(e, node->getClassScope(), false);
        return false;
      }

      case Statement::KindOfClassVariable:
      case Statement::KindOfClassConstant:
      case Statement::KindOfMethodStatement:
        // handled by emitClass
        not_reached();

      case Statement::KindOfFunctionStatement: {
        ASSERT(!node->getClassScope()); // Handled directly by emitClass().
        MethodStatementPtr m(static_pointer_cast<MethodStatement>(node));
        // Only called for fn defs not on the top level
        StringData* nName = StringData::GetStaticString(m->getOriginalName());
        if (m->getFunctionScope()->isGenerator()) {
          if (m->getFileScope() != m_file) {
            // the generator's definition is in another file typically
            // because it was defined in a trait that got inlined into
            // a class in this file. Nothing to do - it will be output
            // with its own file.
            return false;
          }
          Label*& label = m_methLabels[nName];
          if (!label) {
            // its possible to see the same generator more than once
            label = new Label();
          } else {
            return false;
          }

          postponeMeth(m, NULL, true);
        } else {
          FuncEmitter* fe = m_ue.newFuncEmitter(nName, false);
          e.DefFunc(fe->id());
          postponeMeth(m, fe, false);
        }
        return false;
      }

      case Statement::KindOfGotoStatement: {
        GotoStatementPtr g(static_pointer_cast<GotoStatement>(node));
        StringData* nName = StringData::GetStaticString(g->label());
        e.Jmp(m_gotoLabels[nName]);
        return false;
      }

      case Statement::KindOfLabelStatement: {
        LabelStatementPtr l(static_pointer_cast<LabelStatement>(node));
        StringData* nName = StringData::GetStaticString(l->label());
        Label& lab = m_gotoLabels[nName];
        lab.set(e);
        return false;
      }
      case Statement::KindOfUseTraitStatement:
      case Statement::KindOfTraitPrecStatement:
      case Statement::KindOfTraitAliasStatement: {
        not_implemented();
      }
    }
  } else {
    ExpressionPtr ex = static_pointer_cast<Expression>(node);
    switch (ex->getKindOf()) {
      case Expression::KindOfUnaryOpExpression: {
        UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(node));
        int op = u->getOp();

        if (op == T_UNSET) {
          // php doesnt have an unset expression, but hphp's optimizations
          // sometimes introduce them
          ExpressionPtr exp(u->getExpression());
          if (exp->is(Expression::KindOfExpressionList)) {
            ExpressionListPtr exps(
              static_pointer_cast<ExpressionList>(exp));
            if (exps->getListKind() == ExpressionList::ListKindParam) {
              for (int i = 0, n = exps->getCount(); i < n; i++) {
                visit((*exps)[i]);
                emitUnset(e);
              }
              e.Null();
              return true;
            }
          }
          visit(exp);
          emitUnset(e);
          e.Null();
          return true;
        }
        if (op == T_ARRAY) {
          int tuple_cap;
          if (u->isScalar()) {
            TypedValue tv;
            tvWriteUninit(&tv);
            initScalar(tv, u);
            if (m_staticArrays.size() == 0) {
              e.Array(tv.m_data.parr);
            }
          } else if (isTupleInit(u->getExpression(), &tuple_cap)) {
            // evaluate array values onto stack
            ExpressionListPtr el =
              static_pointer_cast<ExpressionList>(u->getExpression());
            for (int i = 0; i < tuple_cap; i++) {
              ArrayPairExpressionPtr ap =
                static_pointer_cast<ArrayPairExpression>((*el)[i]);
              visit(ap->getValue());
              emitConvertToCell(e);
            }
            e.NewTuple(tuple_cap);
          } else {
            ASSERT(m_staticArrays.size() == 0);
            ExpressionPtr ex = u->getExpression();
            if (ex->getKindOf() == Expression::KindOfExpressionList) {
              ExpressionListPtr el(static_pointer_cast<ExpressionList>(ex));
              int capacity = el->getCount();
              if (capacity > 0) {
                m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::ArrayCapacity,
                               false, 0, capacity);
              }
            }
            e.NewArray();
            visit(ex);
          }
          return true;
        } else if (op == T_ISSET) {
          ExpressionListPtr list =
            dynamic_pointer_cast<ExpressionList>(u->getExpression());
          if (list) {
            // isset($a, $b, ...)  ==>  isset($a) && isset($b) && ...
            Label done;
            int n = list->getCount();
            for (int i = 0; i < n - 1; ++i) {
              visit((*list)[i]);
              emitIsset(e);
              e.Dup();
              e.JmpZ(done);
              emitPop(e);
            }
            // Treat the last one specially; let it fall through
            visit((*list)[n - 1]);
            emitIsset(e);
            done.set(e);
          } else {
            // Simple case
            visit(u->getExpression());
            emitIsset(e);
          }
          return true;
        } else if (op == '+' || op == '-') {
          e.Int(0);
        }

        Id oldErrorLevelLoc = -1;
        Offset start = InvalidAbsoluteOffset;
        if (op == '@') {
          // XXX This ends up generating two boatloads of instructions; we may
          // be better off introducing new instructions for the silencer
          oldErrorLevelLoc = m_curFunc->allocUnnamedLocal();
          // Call error_reporting(0), and stash the return in an unnamed local
          emitVirtualLocal(oldErrorLevelLoc);
          static const StringData* s_error_reporting =
            StringData::GetStaticString("error_reporting");
          Offset fpiStart = m_ue.bcPos();
          e.FPushFuncD(1, s_error_reporting);
          {
            FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
            e.Int(0);
            m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
            e.FPassC(0);
          }
          e.FCall(1);
          e.UnboxR();
          emitSet(e); // set $oldErrorLevelLoc
          e.PopC();
          start = m_ue.bcPos();
        }

        ExpressionPtr exp = u->getExpression();
        if (exp && visit(exp)) {
          if (op != T_EMPTY && op != T_INC && op != T_DEC) {
            emitConvertToCell(e);
          }
        } else if (op == T_EXIT) {
          // exit without an expression is treated as exit(0)
          e.Int(0);
        } else {
          // __FILE__ and __DIR__ are special unary ops that don't
          // have expressions
          ASSERT(op == T_FILE || op == T_DIR);
        }
        switch (op) {
          case T_INC:
          case T_DEC: {
            int cop = IncDec_invalid;
            if (op == T_INC) {
              if (u->getFront()) {
                cop = PreInc;
              } else {
                cop = PostInc;
              }
            } else {
              if (u->getFront()) {
                cop = PreDec;
              } else {
                cop = PostDec;
              }
            }
            emitIncDec(e, cop);
            break;
          }
          case T_EMPTY: emitEmpty(e); break;
          case T_CLONE: e.Clone(); break;
          case '+': e.Add(); break;
          case '-': e.Sub(); break;
          case '!': e.Not(); break;
          case '~': e.BitNot(); break;
          case '(': break;
          case T_INT_CAST: e.CastInt(); break;
          case T_DOUBLE_CAST: e.CastDouble(); break;
          case T_STRING_CAST: e.CastString(); break;
          case T_ARRAY_CAST: e.CastArray(); break;
          case T_OBJECT_CAST: e.CastObject(); break;
          case T_BOOL_CAST: e.CastBool(); break;
          case T_UNSET_CAST: emitPop(e); e.Null(); break;
          case T_EXIT: e.Exit(); break;
          case '@': {
            ASSERT(oldErrorLevelLoc >= 0);
            ASSERT(start != InvalidAbsoluteOffset);
            newFaultRegion(start, m_ue.bcPos(),
                           new RestoreErrorReportingThunklet(oldErrorLevelLoc));
            emitRestoreErrorReporting(e, oldErrorLevelLoc);
            m_curFunc->freeUnnamedLocal(oldErrorLevelLoc);
            break;
          }
          case T_PRINT: e.Print(); break;
          case T_EVAL: e.Eval(); break;
          case T_FILE: {
            e.File();
            break;
          }
          case T_DIR: {
            e.Dir();
            break;
          }
          default:
            ASSERT(false);
        }
        return true;
      }

      case Expression::KindOfAssignmentExpression: {
        AssignmentExpressionPtr ae(
          static_pointer_cast<AssignmentExpression>(node));

        visit(ae->getVariable());
        emitClsIfSPropBase(e);
        visit(ae->getValue());
        if (ae->getValue()->hasContext(Expression::RefValue)) {
          emitConvertToVar(e);
          emitBind(e);
          if (ae->hasAnyContext(Expression::AccessContext|
                                Expression::ObjectContext|
                                Expression::ExistContext)) {
            /*
             * hphpc optimizations can result in
             * ($x =& $y)->foo or ($x =& $y)['foo'] or empty($x =& $y)
             */
            emitConvertToCellIfVar(e);
          }
        } else {
          emitConvertToCell(e);
          emitSet(e);
        }
        return true;
      }

      case Expression::KindOfBinaryOpExpression: {
        BinaryOpExpressionPtr b(static_pointer_cast<BinaryOpExpression>(node));
        int op = b->getOp();
        if (b->isAssignmentOp()) {
          visit(b->getExp1());
          emitClsIfSPropBase(e);
          visit(b->getExp2());
          emitConvertToCell(e);
          emitSetOp(e, op);
          return true;
        }

        if (b->isShortCircuitOperator()) {
          Label tru, fls, done;
          visitIfCondition(b, e, tru, fls, false);
          if (fls.isUsed()) fls.set(e);
          if (currentPositionIsReachable()) {
            e.False();
            e.Jmp(done);
          }
          if (tru.isUsed()) tru.set(e);
          if (currentPositionIsReachable()) {
            e.True();
          }
          done.set(e);
          return true;
        }

        if (op == T_INSTANCEOF) {
          visit(b->getExp1());
          emitConvertToCell(e);
          ExpressionPtr second = b->getExp2();
          if (second->isScalar()) {
            ScalarExpressionPtr scalar =
              dynamic_pointer_cast<ScalarExpression>(second);
            bool notQuoted = scalar && !scalar->isQuoted();
            std::string s = second->getLiteralString();
            if (s == "static" && notQuoted) {
              // Can't resolve this to a literal name at emission time
              static const StringData* fname
                = StringData::GetStaticString("get_called_class");
              Offset fpiStart = m_ue.bcPos();
              e.FPushFuncD(0, fname);
              {
                FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
              }
              e.FCall(0);
              e.UnboxR();
              e.InstanceOf();
            } else if (s != "") {
              ClassScopeRawPtr cls = second->getOriginalClass();
              if (cls) {
                if (s == "self" && notQuoted) {
                  s = cls->getOriginalName();
                } else if (s == "parent" && notQuoted) {
                  s = cls->getOriginalParent();
                }
              }
              StringData* nLiteral = StringData::GetStaticString(s);
              e.InstanceOfD(nLiteral);
            } else {
              visit(b->getExp2());
              emitConvertToCell(e);
              e.InstanceOf();
            }
          } else {
            visit(b->getExp2());
            emitConvertToCell(e);
            e.InstanceOf();
          }
          return true;
        }

        if (op == T_COLLECTION) {
          ScalarExpressionPtr cls =
            static_pointer_cast<ScalarExpression>(b->getExp1());
          const std::string* clsName = NULL;
          cls->getString(clsName);
          int cType = 0;
          if (!strcasecmp(clsName->c_str(), "vector")) {
            cType = Collection::VectorType;
          } else if (!strcasecmp(clsName->c_str(), "map")) {
            cType = Collection::MapType;
          } else if (!strcasecmp(clsName->c_str(), "stablemap")) {
            cType = Collection::StableMapType;
          } else {
            throw IncludeTimeFatalException(b,
              "Cannot use collection initialization for "
              "non-collection class");
          }
          bool kvPairs = (cType == Collection::MapType ||
                          cType == Collection::StableMapType);
          int nElems = 0;
          ExpressionListPtr el;
          if (b->getExp2()) {
            el = static_pointer_cast<ExpressionList>(b->getExp2());
            nElems = el->getCount();
          }
          e.NewCol(cType, nElems);
          if (kvPairs) {
            for (int i = 0; i < nElems; i++) {
              ArrayPairExpressionPtr ap(
                static_pointer_cast<ArrayPairExpression>((*el)[i]));
              ExpressionPtr key = ap->getName();
              if (!key) {
                throw IncludeTimeFatalException(ap,
                  "Keys must be specified for Map and StableMap "
                  "initialization");
              }
              visit(key);
              emitConvertToCell(e);
              visit(ap->getValue());
              emitConvertToCell(e);
              e.ColAddElemC();
            }
          } else {
            for (int i = 0; i < nElems; i++) {
              ArrayPairExpressionPtr ap(
                static_pointer_cast<ArrayPairExpression>((*el)[i]));
              ExpressionPtr key = ap->getName();
              if ((bool)key) {
                throw IncludeTimeFatalException(ap,
                  "Keys may not be specified for Vector initialization");
              }
              visit(ap->getValue());
              emitConvertToCell(e);
              e.ColAddNewElemC();
            }
          }
          return true;
        }

        visit(b->getExp1());
        emitConvertToCellOrLoc(e);
        visit(b->getExp2());
        emitConvertToCell(e);
        emitConvertSecondToCell(e);
        switch (op) {
          case T_LOGICAL_XOR: e.Xor(); break;
          case '|': e.BitOr(); break;
          case '&': e.BitAnd(); break;
          case '^': e.BitXor(); break;
          case '.': e.Concat(); break;
          case '+': e.Add(); break;
          case '-': e.Sub(); break;
          case '*': e.Mul(); break;
          case '/': e.Div(); break;
          case '%': e.Mod(); break;
          case T_SL: e.Shl(); break;
          case T_SR: e.Shr(); break;
          case T_IS_IDENTICAL: e.Same(); break;
          case T_IS_NOT_IDENTICAL: e.NSame(); break;
          case T_IS_EQUAL: e.Eq(); break;
          case T_IS_NOT_EQUAL: e.Neq(); break;
          case '<': e.Lt(); break;
          case T_IS_SMALLER_OR_EQUAL: e.Lte(); break;
          case '>': e.Gt(); break;
          case T_IS_GREATER_OR_EQUAL: e.Gte(); break;
          default: ASSERT(false);
        }
        return true;
      }

      case Expression::KindOfClassConstantExpression: {
        ClassConstantExpressionPtr cc(
          static_pointer_cast<ClassConstantExpression>(node));
        StringData* nName = StringData::GetStaticString(cc->getConName());
        if (cc->isStatic()) {
          // static::Constant
          e.LateBoundCls();
          e.ClsCns(nName);
        } else if (cc->getClass()) {
          // $x::Constant
          ExpressionPtr cls(cc->getClass());
          visit(cls);
          emitAGet(e);
          e.ClsCns(nName);
        } else if (cc->getOriginalClass() &&
                   !cc->getOriginalClass()->isTrait()) {
          // C::Constant inside a class
          const std::string& clsName = cc->getOriginalClassName();
          StringData* nCls = StringData::GetStaticString(clsName);
          e.ClsCnsD(nName, nCls);
        } else if (cc->isSelf()) {
          // self::Constant inside trait or pseudomain
          e.Self();
          e.ClsCns(nName);
        } else if (cc->isParent()) {
          // parent::Constant inside trait or pseudomain
          e.Parent();
          e.ClsCns(nName);
        } else {
          // C::Constant inside a trait or pseudomain
          // Be careful to keep this case here after the isSelf and
          // isParent cases because StaticClassName::resolveClass()
          // will set cc->originalClassName to the trait's name for
          // the isSelf and isParent cases, but self and parent must
          // be resolved dynamically when used inside of traits.
          const std::string& clsName = cc->getOriginalClassName();
          StringData* nCls = StringData::GetStaticString(clsName);
          e.ClsCnsD(nName, nCls);
        }
        return true;
      }

      case Expression::KindOfConstantExpression: {
        ConstantExpressionPtr c(static_pointer_cast<ConstantExpression>(node));
        if (c->isNull()) {
          e.Null();
        } else if (c->isBoolean()) {
          if (c->getBooleanValue()) {
            e.True();
          } else {
            e.False();
          }
          return true;
        } else {
          StringData* nName = StringData::GetStaticString(c->getName());
          e.Cns(nName);
        }
        return true;
      }

      case Expression::KindOfEncapsListExpression: {
        EncapsListExpressionPtr el(
          static_pointer_cast<EncapsListExpression>(node));
        ExpressionListPtr args(el->getExpressions());
        int n = args ? args->getCount() : 0;
        int i = 0;
        FPIRegionRecorder* fpi = NULL;
        if (el->getType() == '`') {
          const static StringData* s_shell_exec =
            StringData::GetStaticString("shell_exec");
          Offset fpiStart = m_ue.bcPos();
          e.FPushFuncD(1, s_shell_exec);
          fpi = new FPIRegionRecorder(this, m_ue, m_evalStack, fpiStart);
        }

        if (n) {
          visit((*args)[i++]);
          emitConvertToCellOrLoc(e);
          if (i == n) {
            emitConvertToCell(e);
            e.CastString();
          } else {
            while (i < n) {
              visit((*args)[i++]);
              emitConvertToCell(e);
              emitConvertSecondToCell(e);
              e.Concat();
            }
          }
        } else {
          e.String(empty_string.get());
        }

        if (el->getType() == '`') {
          emitConvertToCell(e);
          m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
          e.FPassC(0);
          delete fpi;
          e.FCall(1);
        }
        return true;
      }

      case Expression::KindOfArrayElementExpression: {
        ArrayElementExpressionPtr ae(
          static_pointer_cast<ArrayElementExpression>(node));
        if (!ae->isSuperGlobal() || !ae->getOffset()) {
          visit(ae->getVariable());
          // XHP syntax allows for expressions like "($a =& $b)[0]". We
          // handle this by unboxing the var produced by "($a =& $b)".
          emitConvertToCellIfVar(e);
        }

        ExpressionPtr offset = ae->getOffset();
        Variant v;
        if (!ae->isSuperGlobal() && offset &&
            offset->getScalarValue(v) && (v.isInteger() || v.isString())) {
          if (v.isString()) {
            m_evalStack.push(StackSym::T);
            m_evalStack.setString(
              StringData::GetStaticString(v.toCStrRef().get()));
          } else {
            m_evalStack.push(StackSym::I);
            m_evalStack.setInt(v.asInt64Val());
          }
          markElem(e);
        } else if (visit(offset)) {
          emitConvertToCellOrLoc(e);
          if (ae->isSuperGlobal()) {
            markGlobalName(e);
          } else {
            markElem(e);
          }
        } else {
          markNewElem(e);
        }
        if (!ae->hasAnyContext(Expression::AccessContext|
                               Expression::ObjectContext)) {
          m_tempLoc = ae->getLocation();
        }
        return true;
      }

      case Expression::KindOfSimpleFunctionCall: {
        SimpleFunctionCallPtr call(
          static_pointer_cast<SimpleFunctionCall>(node));
        ExpressionListPtr params = call->getParams();
        auto inputIsAnObject = [&](int inputIndex) {
          m_metaInfo.addKnownDataType(KindOfObject, /* predicted */ false,
                                      m_ue.bcPos(), false, inputIndex);
        };

        if (call->isFatalFunction()) {
          if (params && params->getCount() == 1) {
            ExpressionPtr p = (*params)[0];
            Variant v;
            if (p->getScalarValue(v)) {
              ASSERT(v.isString());
              StringData* msg = StringData::GetStaticString(v.toString());
              throw IncludeTimeFatalException(call, msg->data());
            }
            not_reached();
          }
        } else if (emitCallUserFunc(e, call)) {
          return true;
        } else if (call->isCallToFunction("array_key_exists")) {
          if (params && params->getCount() == 2) {
            visit((*params)[0]);
            emitConvertToCell(e);
            visit((*params)[1]);
            emitConvertToCell(e);
            e.AKExists();
            return true;
          }
        } else if (call->isCallToFunction("strlen")) {
          if (params && params->getCount() == 1) {
            visit((*params)[0]);
            emitConvertToCell(e);
            e.Strlen();
            return true;
          }
        } else if (call->isCallToFunction("define")) {
          if (params && params->getCount() == 2) {
            ExpressionPtr p0 = (*params)[0];
            Variant v0;
            if (p0->getScalarValue(v0) && v0.isString()) {
              const StringData* cname =
                StringData::GetStaticString(v0.toString());
              ExpressionPtr p1 = (*params)[1];
              Variant v1;
              if (p1->getScalarValue(v1) && v1.isAllowedAsConstantValue()) {
                m_ue.addPreConst(cname, *v1.getTypedAccessor());
              } else {
                m_ue.addPreConst(cname, *null_variant.getTypedAccessor());
              }

              visit(p1);
              emitConvertToCell(e);
              e.DefCns(cname);
              return true;
            }
          }
        } else if (call->isCompilerCallToFunction("hphp_unpack_continuation")) {
          ASSERT(params && params->getCount() == 1);
          inputIsAnObject(0);
          e.UnpackCont();
          return true;
        } else if (call->isCompilerCallToFunction("hphp_pack_continuation")) {
          ASSERT(params && params->getCount() == 3);
          ExpressionPtr label = (*params)[1];
          Variant lVar;
          UNUSED bool isScalar = label->getScalarValue(lVar);
          ASSERT(isScalar && lVar.isInteger());

          visit((*params)[2]);
          emitConvertToCell(e);
          inputIsAnObject(1);
          e.PackCont(lVar.asInt64Val());
          return false;
        } else if (call->isCompilerCallToFunction("hphp_create_continuation")) {
          ASSERT(params && (params->getCount() == 3 ||
                            params->getCount() == 4));
          ExpressionPtr name = (*params)[1];
          Variant nameVar;
          UNUSED bool isScalar = name->getScalarValue(nameVar);
          ASSERT(isScalar && nameVar.isString());
          const StringData* nameStr =
            StringData::GetStaticString(nameVar.getStringData());
          bool callGetArgs = params->getCount() == 4;
          e.CreateCont(callGetArgs, nameStr);
          return true;
        } else if (call->isCompilerCallToFunction("hphp_continuation_raised")) {
          inputIsAnObject(0);
          e.ContRaised();
          return false;
        } else if (
            call->isCompilerCallToFunction("hphp_continuation_receive")) {
          inputIsAnObject(0);
          e.ContReceive();
          return true;
        } else if (call->isCompilerCallToFunction("hphp_continuation_done")) {
          inputIsAnObject(0);
          e.ContDone();
          return false;
        } else if ((call->isCallToFunction("class_exists") ||
                    call->isCallToFunction("interface_exists") ||
                    call->isCallToFunction("trait_exists")) && params &&
                   (params->getCount() == 1 || params->getCount() == 2)) {
          // Push name
          emitNameString(e, (*params)[0]);
          emitConvertToCell(e);

          // Push autoload, defaulting to true
          if (params->getCount() == 1) {
            e.True();
          } else {
            visit((*params)[1]);
            emitConvertToCell(e);
          }
          if (call->isCallToFunction("class_exists")) {
            e.ClassExists();
          } else if (call->isCallToFunction("interface_exists")) {
            e.InterfaceExists();
          } else {
            ASSERT(call->isCallToFunction("trait_exists"));
            e.TraitExists();
          }
          return true;
        }
#define TYPE_CHECK_INSTR(what, What)                                  \
        else if (call->isCallToFunction("is_"#what) &&                \
                 params && params->getCount() == 1) {                 \
          visit((*call->getParams())[0]);                             \
          emitIs ## What(e);                                          \
          return true;                                                \
        }
      TYPE_CHECK_INSTR(null, Null)
      TYPE_CHECK_INSTR(object, Object)
      TYPE_CHECK_INSTR(array, Array)
      TYPE_CHECK_INSTR(string, String)
      TYPE_CHECK_INSTR(int, Int)
      TYPE_CHECK_INSTR(integer, Int)
      TYPE_CHECK_INSTR(long, Int)
      TYPE_CHECK_INSTR(bool, Bool)
      TYPE_CHECK_INSTR(double, Double)
      TYPE_CHECK_INSTR(real, Double)
      TYPE_CHECK_INSTR(float, Double)
#undef TYPE_CHECK_INSTR
        // fall through
      }
      case Expression::KindOfDynamicFunctionCall: {
        emitFuncCall(e, static_pointer_cast<FunctionCall>(node));
        return true;
      }

      case Expression::KindOfIncludeExpression: {
        IncludeExpressionPtr ie(static_pointer_cast<IncludeExpression>(node));
        if (ie->isReqLit()) {
          StringData* nValue = StringData::GetStaticString(ie->includePath());
          e.String(nValue);
        } else {
          visit(ie->getExpression());
          emitConvertToCell(e);
        }
        switch (ie->getOp()) {
          case T_INCLUDE:
            e.Incl();
            break;
          case T_INCLUDE_ONCE:
            e.InclOnce();
            break;
          case T_REQUIRE:
            e.Req();
            break;
          case T_REQUIRE_ONCE:
            if (ie->isDocumentRoot()) {
              if (ie->isPrivateScope()) {
                e.ReqMod();
              } else {
                e.ReqDoc();
              }
            } else {
              if (ie->isPrivateScope()) {
                e.ReqSrc();
              } else {
                e.ReqOnce();
              }
            }
            break;
        }
        return true;
      }

      case Expression::KindOfListAssignment: {
        ListAssignmentPtr la(static_pointer_cast<ListAssignment>(node));
        ExpressionPtr rhs = la->getArray();

        if (!rhs) {
          // visitListAssignmentLHS should have handled this
          ASSERT(false);
        }

        // We use "index chains" to deal with nested list assignment.  We will
        // end up with one index chain per expression we need to assign to.
        // The helper function will populate indexChains.
        std::vector<IndexChain*> indexChains;
        IndexChain workingChain;
        visitListAssignmentLHS(e, la, workingChain, indexChains);

        bool nullRHS = la->getRHSKind() == ListAssignment::Null;
        // Assign RHS to temp local, unless it's already a simple variable
        bool simpleRHS = rhs->is(Expression::KindOfSimpleVariable)
          && !static_pointer_cast<SimpleVariable>(rhs)->getAlwaysStash();
        Id tempLocal = -1;
        Offset start = InvalidAbsoluteOffset;
        if (!simpleRHS) {
          tempLocal = m_curFunc->allocUnnamedLocal();
          emitVirtualLocal(tempLocal);
          visit(rhs);
          emitConvertToCell(e);
          emitSet(e);
          emitPop(e);
          start = m_ue.bcPos();
        }

        // Assign elements, right-to-left
        for (int i = (int)indexChains.size() - 1; i >= 0; --i) {
          IndexChain* currIndexChain = indexChains[i];
          if (currIndexChain->empty()) {
            continue;
          }

          if (nullRHS) {
            e.Null();
          } else {
            if (simpleRHS) {
              visit(rhs);
            } else {
              emitVirtualLocal(tempLocal);
            }
            for (int j = 0; j < (int)currIndexChain->size(); ++j) {
              e.Int((*currIndexChain)[j]);
              markElem(e);
            }
            emitCGet(e);
          }
          emitSet(e);
          emitPop(e);

          delete currIndexChain;
        }

        // Leave the RHS on the stack
        if (simpleRHS) {
          visit(rhs);
        } else {
          emitVirtualLocal(tempLocal);
          emitCGet(e);
        }

        // Null out and free unnamed local
        if (!simpleRHS) {
          ASSERT(tempLocal >= 0);
          ASSERT(start != InvalidAbsoluteOffset);
          newFaultRegion(start, m_ue.bcPos(),
                         new UnsetUnnamedLocalThunklet(tempLocal));
          emitVirtualLocal(tempLocal);
          emitUnset(e);
          m_curFunc->freeUnnamedLocal(tempLocal);
        }
        return true;
      }

      case Expression::KindOfNewObjectExpression: {
        NewObjectExpressionPtr ne(
          static_pointer_cast<NewObjectExpression>(node));
        ExpressionListPtr params(ne->getParams());
        int numParams = params ? params->getCount() : 0;
        ClassScopeRawPtr cls = ne->getOriginalClass();

        Offset fpiStart;
        if (ne->isStatic()) {
          // new static()
          e.LateBoundCls();
          fpiStart = m_ue.bcPos();
          e.FPushCtor(numParams);
        } else if (ne->getOriginalName().empty()) {
          // new $x()
          visit(ne->getNameExp());
          emitAGet(e);
          fpiStart = m_ue.bcPos();
          e.FPushCtor(numParams);
        } else if ((ne->isSelf() || ne->isParent()) &&
                   (!cls || cls->isTrait() ||
                    (ne->isParent() && cls->getOriginalParent().empty()))) {
          if (ne->isSelf()) {
            // new self() inside a trait or code statically not inside any class
            e.Self();
          } else {
            // new parent() inside a trait, code statically not inside any
            // class, or a class with no parent
            e.Parent();
          }
          fpiStart = m_ue.bcPos();
          e.FPushCtor(numParams);
        } else {
          // new C() inside trait or pseudomain
          fpiStart = m_ue.bcPos();
          e.FPushCtorD(numParams,
                       StringData::GetStaticString(ne->getOriginalClassName()));
        }

        {
          FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
          for (int i = 0; i < numParams; i++) {
            emitFuncCallArg(e, (*params)[i], i);
          }
        }

        e.FCall(numParams);
        e.PopR();
        return true;
      }

      case Expression::KindOfObjectMethodExpression: {
        ObjectMethodExpressionPtr om(
          static_pointer_cast<ObjectMethodExpression>(node));
        // $obj->name(...)
        // ^^^^
        visit(om->getObject());
        m_tempLoc = om->getLocation();
        emitConvertToCell(e);
        StringData* clsName = getClassName(om->getObject());
        ExpressionListPtr params(om->getParams());
        int numParams = params ? params->getCount() : 0;

        Offset fpiStart;
        if (!om->getName().empty()) {
          // $obj->name(...)
          //       ^^^^
          // Use getOriginalName, which hasn't been case-normalized, since
          // __call() is case-preserving.
          StringData* nameLiteral =
            StringData::GetStaticString(om->getOriginalName());
          fpiStart = m_ue.bcPos();
          e.FPushObjMethodD(numParams, nameLiteral);
        } else {
          // $obj->{...}(...)
          //       ^^^^^
          visit(om->getNameExp());
          emitConvertToCell(e);
          fpiStart = m_ue.bcPos();
          e.FPushObjMethod(numParams);
        }
        if (clsName) {
          Id id = m_ue.mergeLitstr(clsName);
          m_metaInfo.add(fpiStart, Unit::MetaInfo::Class, false,
                         om->getName().empty() ? 1 : 0, id);
        }
        {
          FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
          // $obj->name(...)
          //           ^^^^^
          for (int i = 0; i < numParams; i++) {
            emitFuncCallArg(e, (*params)[i], i);
          }
        }
        e.FCall(numParams);
        if (Option::WholeProgram) {
          fixReturnType(e, om);
        }
        return true;
      }

      case Expression::KindOfObjectPropertyExpression: {
        ObjectPropertyExpressionPtr op(
          static_pointer_cast<ObjectPropertyExpression>(node));
        ExpressionPtr obj = op->getObject();
        SimpleVariablePtr sv = dynamic_pointer_cast<SimpleVariable>(obj);
        if (sv && sv->isThis() && sv->hasContext(Expression::ObjectContext)) {
          if (sv->isGuarded()) {
            m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                           false, 0, 0);
          }
          e.CheckThis();
          m_evalStack.push(StackSym::H);
        } else {
          visit(obj);
        }
        StringData* clsName = getClassName(op->getObject());
        if (clsName) {
          m_evalStack.setKnownCls(clsName, false);
        }
        emitNameString(e, op->getProperty(), true);
        if (!op->hasAnyContext(Expression::AccessContext|
                               Expression::ObjectContext)) {
          m_tempLoc = op->getLocation();
        }
        markProp(e);
        return true;
      }

      case Expression::KindOfQOpExpression: {
        QOpExpressionPtr q(static_pointer_cast<QOpExpression>(node));
        if (q->getYes()) {
          // <expr> ? <expr> : <expr>
          Label tru, fals, done;
          {
            Emitter condEmitter(q->getCondition(), m_ue, *this);
            visitIfCondition(q->getCondition(), condEmitter,
                             tru, fals, true);
          }
          if (tru.isUsed()) {
            tru.set(e);
          }
          if (currentPositionIsReachable()) {
            visit(q->getYes());
            emitConvertToCell(e);
            e.Jmp(done);
          }
          if (fals.isUsed()) fals.set(e);
          if (currentPositionIsReachable()) {
            visit(q->getNo());
            emitConvertToCell(e);
          }
          if (done.isUsed()) {
            done.set(e);
            m_evalStack.cleanTopMeta();
          }
        } else {
          // <expr> ?: <expr>
          Label done;
          visit(q->getCondition());
          emitConvertToCell(e);
          e.Dup();
          e.JmpNZ(done);
          e.PopC();
          visit(q->getNo());
          emitConvertToCell(e);
          done.set(e);
          m_evalStack.cleanTopMeta();
        }
        return true;
      }

      case Expression::KindOfScalarExpression: {
        Variant v;
        ex->getScalarValue(v);
        switch (v.getType()) {
          case KindOfString:
          case KindOfStaticString:
          {
            ScalarExpressionPtr
              scalarExp(static_pointer_cast<ScalarExpression>(node));
            // Inside traits, __class__ cannot be resolved yet,
            // so emit call to get_class.
            if (scalarExp->getType() == T_CLASS_C && m_curFunc &&
                m_curFunc->pce() &&
                (m_curFunc->pce()->attrs() & VM::AttrTrait)) {
              static const StringData* fname =
                StringData::GetStaticString("get_class");
              Offset fpiStart = m_ue.bcPos();
              e.FPushFuncD(0, fname);
              {
                FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
              }
              e.FCall(0);
              e.UnboxR();
            } else {
              StringData* nValue =
                StringData::GetStaticString(v.getStringData());
              e.String(nValue);
            }
            break;
          }
          case KindOfInt64:
            e.Int(v.getInt64());
            break;
          case KindOfDouble:
            e.Double(v.getDouble()); break;
          default:
            ASSERT(false);
        }
        return true;
      }

      case Expression::KindOfSimpleVariable: {
        SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(node));
        if (sv->isThis()) {
          if (sv->hasContext(Expression::ObjectContext)) {
            if (sv->isGuarded()) {
              m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                             false, 0, 0);
            }
            e.This();
          } else if (sv->getFunctionScope()->needsLocalThis()) {
            static const StringData* thisStr =
              StringData::GetStaticString("this");
            Id thisId = m_curFunc->lookupVarId(thisStr);
            emitVirtualLocal(thisId);
          } else {
            if (sv->isGuarded()) {
              m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                             false, 0, 0);
              e.This();
            } else {
              e.BareThis(!sv->hasContext(Expression::ExistContext));
            }
          }
        } else {
          StringData* nLiteral = StringData::GetStaticString(sv->getName());
          if (sv->isSuperGlobal()) {
            e.String(nLiteral);
            markGlobalName(e);
            return true;
          }
          Id i = m_curFunc->lookupVarId(nLiteral);
          emitVirtualLocal(i);
          if (!sv->couldBeAliased()) {
            m_evalStack.setNotRef();
          }
          if (sv->getAlwaysStash() &&
              !sv->hasAnyContext(Expression::ExistContext |
                                 Expression::RefValue |
                                 Expression::LValue |
                                 Expression::RefParameter)) {
            emitConvertToCell(e);
          }
        }

        return true;
      }

      case Expression::KindOfDynamicVariable: {
        DynamicVariablePtr dv(static_pointer_cast<DynamicVariable>(node));
        visit(dv->getSubExpression());
        emitConvertToCellOrLoc(e);
        markName(e);
        return true;
      }

      case Expression::KindOfStaticMemberExpression: {
        StaticMemberExpressionPtr sm(
          static_pointer_cast<StaticMemberExpression>(node));
        emitVirtualClassBase(e, sm.get());
        emitNameString(e, sm->getExp());
        markSProp(e);
        return true;
      }

      case Expression::KindOfArrayPairExpression: {
        ArrayPairExpressionPtr ap(
          static_pointer_cast<ArrayPairExpression>(node));

        ExpressionPtr key = ap->getName();
        if (m_staticArrays.size()) {
          HphpArray* a = m_staticArrays.back();
          ExpressionPtr val = ap->getValue();

          // Value.
          TypedValue tvVal;
          initScalar(tvVal, val);

          if (key != NULL) {
            // Key.
            ASSERT(key->isScalar());
            TypedValue tvKey;
            tvKey._count = 0;
            if (key->is(Expression::KindOfConstantExpression)) {
              ConstantExpressionPtr c(
                static_pointer_cast<ConstantExpression>(key));
              if (c->isNull()) {
                // PHP casts null keys to "".
                tvKey.m_data.pstr = empty_string.get();
                tvKey.m_type = KindOfString;
              } else if (c->isBoolean()) {
                // PHP casts bool keys to 0 or 1
                tvKey.m_data.num = c->getBooleanValue() ? 1 : 0;
                tvKey.m_type = KindOfInt64;
              } else {
                // Handle INF and NAN
                ASSERT(c->isDouble());
                Variant v;
                c->getScalarValue(v);
                tvKey.m_data.num = v.toInt64();
                tvKey.m_type = KindOfInt64;
              }
            } else if (key->is(Expression::KindOfScalarExpression)) {
              ScalarExpressionPtr sval(
                static_pointer_cast<ScalarExpression>(key));
              const std::string* s;
              int64 i;
              double d;
              if (sval->getString(s)) {
                StringData* sd = StringData::GetStaticString(*s);
                int64 n = 0;
                if (sd->isStrictlyInteger(n)) {
                  tvKey.m_data.num = n;
                  tvKey.m_type = KindOfInt64;
                } else {
                  tvKey.m_data.pstr = sd;
                  tvKey.m_type = KindOfString;
                }
              } else if (sval->getInt(i)) {
                tvKey.m_data.num = i;
                tvKey.m_type = KindOfInt64;
              } else if (sval->getDouble(d)) {
                tvKey.m_data.num = toInt64(d);
                tvKey.m_type = KindOfInt64;
              } else {
                not_implemented();
              }
            } else {
              not_implemented();
            }
            a->set(tvAsCVarRef(&tvKey), tvAsVariant(&tvVal), false);
          } else {
            a->append(tvAsCVarRef(&tvVal), false);
          }
        } else {
          // Assume new array is on top of stack
          bool hasKey = (bool)key;
          if (hasKey) {
            visit(key);
            emitConvertToCellOrLoc(e);
          }
          visit(ap->getValue());
          if (ap->isRef()) {
            emitConvertToVar(e);
            if (hasKey) {
              emitConvertSecondToCell(e);
              e.AddElemV();
            } else {
              e.AddNewElemV();
            }
          } else {
            emitConvertToCell(e);
            if (hasKey) {
              emitConvertSecondToCell(e);
              e.AddElemC();
            } else {
              e.AddNewElemC();
            }
          }
        }
        return true;
      }
      case Expression::KindOfExpressionList: {
        ExpressionListPtr el(static_pointer_cast<ExpressionList>(node));
        int nelem = el->getCount(), i;
        bool pop = el->getListKind() != ExpressionList::ListKindParam;
        int keep = el->getListKind() == ExpressionList::ListKindLeft ?
          0 : nelem - 1;
        int cnt = 0;
        for (i = 0; i < nelem; i++) {
          ExpressionPtr p((*el)[i]);
          if (visit(p)) {
            if (pop && i != keep) {
              emitPop(e);
            } else {
              cnt++;
            }
          }
        }
        return cnt != 0;
      }
      case Expression::KindOfParameterExpression: {
        not_implemented();
      }
      case Expression::KindOfModifierExpression: {
        not_implemented();
      }
      case Expression::KindOfUserAttribute: {
        not_implemented();
      }
      case Expression::KindOfClosureExpression: {
        // Closures are implemented by anonymous classes that extend Closure.
        // There is one anonymous class per closure body.
        ClosureExpressionPtr ce(static_pointer_cast<ClosureExpression>(node));

        // Build a convenient list of use-variables. Each one corresponds to:
        // (a) an instance variable, to store the value until call time
        // (b) a parameter of the generated constructor
        // (c) an argument to the constructor at the definition site
        // (d) a line of code in the generated constructor;
        // (e) a line of code in the generated prologue to the closure body
        ExpressionListPtr useList(ce->getClosureVariables());
        ClosureUseVarVec useVars;
        int useCount = (useList ? useList->getCount() : 0);
        if (useList) {
          for (int i = 0; i < useCount; ++i) {
            ParameterExpressionPtr var(
              static_pointer_cast<ParameterExpression>((*useList)[i]));
            StringData* varName = StringData::GetStaticString(var->getName());
            useVars.push_back(
              ClosureUseVar(varName, var->isRef()));
          }
        }

        StringData* className = newClosureName();
        const static StringData* parentName =
          StringData::GetStaticString("closure");
        const Location* sLoc = ce->getLocation().get();
        PreClassEmitter* pce = m_ue.newPreClassEmitter(
          className, PreClass::AlwaysHoistable);
        pce->init(sLoc->line0, sLoc->line1, m_ue.bcPos(),
                  AttrUnique | AttrPersistent, parentName, NULL);

        // We're still at the closure definition site. Emit code to instantiate
        // the new anonymous class, with the use variables as arguments.
        ExpressionListPtr valuesList(ce->getClosureValues());
        Offset fpiStart = m_ue.bcPos();
        e.FPushCtorD(useCount, className);
        {
          FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
          for (int i = 0; i < useCount; ++i) {
            emitFuncCallArg(e, (*valuesList)[i], i);
          }
        }
        e.FCall(useCount);
        emitPop(e);
        // From here on out, we're just building metadata for the closure.

        // Instance variables.
        TypedValue uninit;
        tvWriteUninit(&uninit);
        for (int i = 0; i < useCount; ++i) {
          pce->addProperty(useVars[i].first, AttrPrivate, NULL, &uninit);
        }

        // The constructor. This is entirely generated; all it does is stash its
        // arguments in the object's instance variables.
        static const StringData* ctorName =
          StringData::GetStaticString("__construct");
        FuncEmitter* ctor = m_ue.newMethodEmitter(ctorName, pce);
        pce->addMethod(ctor);
        m_postponedClosureCtors.push_back(
          PostponedClosureCtor(useVars, ce, ctor));

        // The __invoke method. This is the body of the closure, preceded by
        // code that pulls the object's instance variables into locals.
        static const StringData* invokeName =
          StringData::GetStaticString("__invoke");
        FuncEmitter* invoke = m_ue.newMethodEmitter(invokeName, pce);
        invoke->setIsClosureBody(true);
        pce->addMethod(invoke);
        MethodStatementPtr body(
          static_pointer_cast<MethodStatement>(ce->getClosureFunction()));
        postponeMeth(body, invoke, false, new ClosureUseVarVec(useVars));

        return true;
      }
    }
  }

  not_reached();
}

int EmitterVisitor::scanStackForLocation(int iLast) {
  ASSERT(iLast >= 0);
  ASSERT(iLast < (int)m_evalStack.size());
  for (int i = iLast; i >= 0; --i) {
    char marker = StackSym::GetMarker(m_evalStack.get(i));
    if (marker != StackSym::E && marker != StackSym::W &&
        marker != StackSym::P && marker != StackSym::M) {
      return i;
    }
  }
  InvariantViolation("Emitter expected a location on the stack but none "
                     "was found (at offset %d)",
                     m_ue.bcPos());
  return 0;
}

void EmitterVisitor::buildVectorImm(std::vector<uchar>& vectorImm,
                                    int iFirst, int iLast, bool allowW,
                                    Emitter& e) {
  ASSERT(iFirst >= 0);
  ASSERT(iFirst <= iLast);
  ASSERT(iLast < (int)m_evalStack.size());
  vectorImm.clear();
  vectorImm.reserve(iLast - iFirst + 1);

  int metaI = 1;

  /*
   * Because of php's order of evaluation rules, we store the classref
   * for certain types of S-vectors at the end, instead of the front.
   * See emitCls for details.
   */

  {
    char sym = m_evalStack.get(iFirst);
    char symFlavor = StackSym::GetSymFlavor(sym);
    char marker = StackSym::GetMarker(sym);
    m_metaInfo.addKnownDataType(m_evalStack.getKnownType(iFirst),
                                m_evalStack.isTypePredicted(iFirst),
                                m_ue.bcPos(), true, 0);
    if (const StringData* cls = m_evalStack.getClsName(iFirst)) {
      Id id = m_ue.mergeLitstr(cls);
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Class, true, 0, id);
    }
    switch (marker) {
      case StackSym::N: {
        if (symFlavor == StackSym::C) {
          vectorImm.push_back(LNC);
        } else if (symFlavor == StackSym::L) {
          vectorImm.push_back(LNL);
        } else {
          ASSERT(false);
        }
      } break;
      case StackSym::G: {
        if (symFlavor == StackSym::C) {
          vectorImm.push_back(LGC);
        } else if (symFlavor == StackSym::L) {
          vectorImm.push_back(LGL);
        } else {
          ASSERT(false);
        }
      } break;
      case StackSym::S: {
        if (symFlavor != StackSym::L && symFlavor != StackSym::C) {
          unexpectedStackSym(sym, "S-vector base, prop name");
        }
        if (m_evalStack.get(iLast) != StackSym::AM) {
          unexpectedStackSym(sym, "S-vector base, class ref");
        }
        const bool curIsLoc = symFlavor == StackSym::L;
        vectorImm.push_back(curIsLoc ? LSL : LSC);
      } break;
      case StackSym::None: {
        if (symFlavor == StackSym::L) {
          vectorImm.push_back(LL);
        } else if (symFlavor == StackSym::C) {
          vectorImm.push_back(LC);
        } else if (symFlavor == StackSym::R) {
          vectorImm.push_back(LR);
        } else if (symFlavor == StackSym::H) {
          vectorImm.push_back(LH);
        } else {
          not_reached();
        }
      } break;
      default: {
        not_reached();
      }
    }
    if (symFlavor == StackSym::L) {
      encodeIvaToVector(vectorImm, m_evalStack.getLoc(iFirst));
    }
  }

  int i = iFirst + 1;
  while (i <= iLast) {
    char sym = m_evalStack.get(i);
    char symFlavor = StackSym::GetSymFlavor(sym);
    char marker = StackSym::GetMarker(sym);
    Id strid = -1;

    if (const StringData* name = m_evalStack.getName(i)) {
      strid = m_ue.mergeLitstr(name);
      // If this string is an m-vector litstr it will be stored in the
      // m-vector later on in this function. Don't duplicate it in the
      // metadata table.
      if (symFlavor != StackSym::T) {
        m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::String,
                       true, metaI, strid);
      }
    }
    if (const StringData* cls = m_evalStack.getClsName(i)) {
      const int mcodeNum = i - (iFirst + 1);
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::MVecPropClass,
                     false, mcodeNum, m_ue.mergeLitstr(cls));
    }
    m_metaInfo.addKnownDataType(m_evalStack.getKnownType(i),
                                m_evalStack.isTypePredicted(i),
                                m_ue.bcPos(), true, i - iFirst);

    switch (marker) {
      case StackSym::M: {
        ASSERT(symFlavor == StackSym::A);
        break;
      }
      case StackSym::E: {
        if (symFlavor == StackSym::L) {
          vectorImm.push_back(MEL);
        } else if (symFlavor == StackSym::T) {
          vectorImm.push_back(MET);
        } else if (symFlavor == StackSym::I) {
          vectorImm.push_back(MEI);
        } else {
          vectorImm.push_back(MEC);
        }
      } break;
      case StackSym::W: {
        if (allowW) {
          vectorImm.push_back(MW);
        } else {
          throw IncludeTimeFatalException(e.getNode(),
                                          "Cannot use [] for reading");
        }
      } break;
      case StackSym::P: {
        if (symFlavor == StackSym::L) {
          vectorImm.push_back(MPL);
        } else if (symFlavor == StackSym::T) {
          vectorImm.push_back(MPT);
        } else {
          vectorImm.push_back(MPC);
        }
      } break;
      case StackSym::S: {
        ASSERT(false);
      }
      default: ASSERT(false); break;
    }

    if (symFlavor == StackSym::L) {
      encodeIvaToVector(vectorImm, m_evalStack.getLoc(i));
    } else if (symFlavor == StackSym::T) {
      ASSERT(strid != -1);
      encodeToVector<int32>(vectorImm, strid);
    } else if (symFlavor == StackSym::I) {
      encodeToVector<int64>(vectorImm, m_evalStack.getInt(i));
    }

    ++i;
    if (marker != StackSym::W) ++metaI;
  }
}

void EmitterVisitor::emitPop(Emitter& e) {
  if (checkIfStackEmpty("Pop*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.reset();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::C:  e.PopC(); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CN: e.CGetN(); e.PopC(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CG: e.CGetG(); e.PopC(); break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i)); // fall through
      case StackSym::CS: e.CGetS(); e.PopC(); break;
      case StackSym::V:  e.PopV(); break;
      case StackSym::R:  e.PopR(); break;
      default: {
        unexpectedStackSym(sym, "emitPop");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.CGetM(vectorImm);
    e.PopC();
  }
}

void EmitterVisitor::emitCGetL2(Emitter& e) {
  ASSERT(m_evalStack.size() >= 2);
  ASSERT(m_evalStack.sizeActual() >= 1);
  ASSERT(StackSym::GetSymFlavor(m_evalStack.get(m_evalStack.size() - 2))
    == StackSym::L);
  int localIdx = m_evalStack.getLoc(m_evalStack.size() - 2);
  e.CGetL2(localIdx);
}

void EmitterVisitor::emitCGetL3(Emitter& e) {
  ASSERT(m_evalStack.size() >= 3);
  ASSERT(m_evalStack.sizeActual() >= 2);
  ASSERT(StackSym::GetSymFlavor(m_evalStack.get(m_evalStack.size() - 3))
    == StackSym::L);
  int localIdx = m_evalStack.getLoc(m_evalStack.size() - 3);
  e.CGetL3(localIdx);
}

void EmitterVisitor::emitAGet(Emitter& e) {
  if (checkIfStackEmpty("AGet*")) return;

  emitConvertToCellOrLoc(e);
  switch (char sym = m_evalStack.top()) {
  case StackSym::L:
    e.AGetL(m_evalStack.getLoc(m_evalStack.size() - 1));
    break;
  case StackSym::C:
    e.AGetC();
    break;
  default:
    unexpectedStackSym(sym, "emitAGet");
  }
}

void EmitterVisitor::emitCGet(Emitter& e) {
  if (checkIfStackEmpty("CGet*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.reset();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.CGetL(m_evalStack.getLoc(i));  break;
      case StackSym::C:  /* nop */   break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.CGetN();  break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.CGetG();  break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.CGetS();  break;
      case StackSym::V:  e.Unbox();  break;
      case StackSym::R:  e.UnboxR(); break;
      default: {
        unexpectedStackSym(sym, "emitCGet");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.CGetM(vectorImm);
  }
}

void EmitterVisitor::emitVGet(Emitter& e) {
  if (checkIfStackEmpty("VGet*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.reset();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.VGetL(m_evalStack.getLoc(i)); break;
      case StackSym::C:  e.Box();   break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CN: e.VGetN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i)); // fall through
      case StackSym::CG: e.VGetG(); break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i)); // fall through
      case StackSym::CS: e.VGetS(); break;
      case StackSym::V:  /* nop */  break;
      case StackSym::R:  e.BoxR();  break;
      default: {
        unexpectedStackSym(sym, "emitVGet");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.VGetM(vectorImm);
  }
}

namespace PassByRefKind {
  static const ssize_t AllowCell = 0;
  static const ssize_t WarnOnCell = 1;
  static const ssize_t ErrorOnCell = 2;
}

static ssize_t getPassByRefKind(ExpressionPtr exp) {
  // The PassByRefKind of a list assignment expression is determined
  // by the PassByRefKind of the RHS. This loop will repeatedly recurse
  // on the RHS until it encounters an expression other than a list
  // assignment expression.
  while (exp->is(Expression::KindOfListAssignment)) {
    ListAssignmentPtr la(static_pointer_cast<ListAssignment>(exp));
    exp = la->getArray();
  }
  switch (exp->getKindOf()) {
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfIncludeExpression:
      // New and include/require
      return PassByRefKind::AllowCell;
    case Expression::KindOfAssignmentExpression:
      // Assignment (=) and binding assignment (=&)
      return PassByRefKind::WarnOnCell;
    case Expression::KindOfBinaryOpExpression: {
      BinaryOpExpressionPtr b(static_pointer_cast<BinaryOpExpression>(exp));
      // Assignment op (+=, -=, *=, etc)
      if (b->isAssignmentOp()) return PassByRefKind::WarnOnCell;
    } break;
    case Expression::KindOfUnaryOpExpression: {
      UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(exp));
      int op = u->getOp();
      if (op == T_CLONE) {
        // clone
        return PassByRefKind::AllowCell;
      } else if (op == '@' || op == T_EVAL ||
                 ((op == T_INC || op == T_DEC) && u->getFront())) {
        // Silence operator, eval, preincrement, and predecrement
        return PassByRefKind::WarnOnCell;
      }
    } break;
    case Expression::KindOfExpressionList: {
      ExpressionListPtr el(static_pointer_cast<ExpressionList>(exp));
      if (el->getListKind() != ExpressionList::ListKindParam) {
        return PassByRefKind::WarnOnCell;
      }
    } break;
    default:
      break;
  }
  // All other cases
  return PassByRefKind::ErrorOnCell;
}

void EmitterVisitor::emitBuiltinCallArg(Emitter& e,
                                         ExpressionPtr exp,
                                         int paramId,
                                         bool byRef) {
  visit(exp);
  if (checkIfStackEmpty("BPass*")) return;
  if (byRef) {
    emitVGet(e);
    m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
    e.BPassV(paramId);
  } else {
    emitCGet(e);
    m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
    e.BPassC(paramId);
  }
  return;
}

void EmitterVisitor::emitBuiltinDefaultArg(Emitter& e, Variant& v,
                                           DataType t, int paramId) {
  switch (v.getType()) {
    case KindOfString:
    case KindOfStaticString: {
      StringData *nValue = StringData::GetStaticString(v.getStringData());
      e.String(nValue);
      break;
    }
    case KindOfInt64:
      e.Int(v.getInt64());
      break;
    case KindOfBoolean:
      if (v.getBoolean()) {
        e.True();
      } else {
        e.False();
      }
      break;
    case KindOfNull:
      switch (t) {
        case KindOfString:
        case KindOfStaticString:
        case KindOfObject:
        case KindOfArray:
          e.Int(0);
          break;
        case KindOfUnknown:
          e.NullUninit();
          break;
        default:
          not_reached();
      }
      break;
    default:
      not_reached();
  }
  m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
  e.BPassC(paramId);
}

void EmitterVisitor::emitFuncCallArg(Emitter& e,
                                     ExpressionPtr exp,
                                     int paramId) {
  visit(exp);
  if (checkIfStackEmpty("FPass*")) return;
  if (Option::WholeProgram && !exp->hasAnyContext(Expression::InvokeArgument |
                                                  Expression::RefParameter)) {
    if (exp->hasContext(Expression::RefValue)) {
      emitVGet(e);
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
      e.FPassV(paramId);
    } else {
      emitCGet(e);
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
      e.FPassC(paramId);
    }
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  // This ensures that the FPass instruction will be associated with
  // exp's source location.
  LocationGuard locGuard(e, m_tempLoc);
  m_tempLoc.reset();
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.FPassL(paramId, m_evalStack.getLoc(i)); break;
      case StackSym::C:
        switch (getPassByRefKind(exp)) {
          case PassByRefKind::AllowCell:   e.FPassC(paramId); break;
          case PassByRefKind::WarnOnCell:  e.FPassCW(paramId); break;
          case PassByRefKind::ErrorOnCell: e.FPassCE(paramId); break;
          default: ASSERT(false);
        }
        break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.FPassN(paramId); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.FPassG(paramId); break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.FPassS(paramId); break;
      case StackSym::V:  e.FPassV(paramId); break;
      case StackSym::R:  e.FPassR(paramId); break;
      default: {
        unexpectedStackSym(sym, "emitFPass");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.FPassM(paramId, vectorImm);
  }
}

void EmitterVisitor::emitIsset(Emitter& e) {
  if (checkIfStackEmpty("Isset*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.IssetL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.IssetN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.IssetG(); break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.IssetS(); break;
      //XXX: Zend does not allow isset() on the result
      // of a function call. We allow it here so that emitted
      // code is valid. Once the parser handles this correctly,
      // the R and C cases can go.
      case StackSym::R:  e.UnboxR(); // fall through
      case StackSym::C:  e.IsNullC(); e.Not(); break;
      default: {
        unexpectedStackSym(sym, "emitIsset");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.IssetM(vectorImm);
  }
}

#define EMIT_TYPE_CHECK_INSTR(What)                     \
void EmitterVisitor::emitIs ## What(Emitter& e) {       \
  if (checkIfStackEmpty("Is"#What)) return;             \
                                                        \
  emitConvertToCellOrLoc(e);                            \
  switch (char sym = m_evalStack.top()) {               \
  case StackSym::L:                                     \
    e.Is ## What ## L(                                  \
      m_evalStack.getLoc(m_evalStack.size() - 1));      \
    break;                                              \
  case StackSym::C:                                     \
    e.Is ## What ## C();                                \
    break;                                              \
  default:                                              \
    unexpectedStackSym(sym, "emitIs" #What);            \
  }                                                     \
}

EMIT_TYPE_CHECK_INSTR(Null);
EMIT_TYPE_CHECK_INSTR(Bool);
EMIT_TYPE_CHECK_INSTR(Int);
EMIT_TYPE_CHECK_INSTR(Double);
EMIT_TYPE_CHECK_INSTR(String);
EMIT_TYPE_CHECK_INSTR(Array);
EMIT_TYPE_CHECK_INSTR(Object);
#undef EMIT_TYPE_CHECK_INSTR

void EmitterVisitor::emitEmpty(Emitter& e) {
  if (checkIfStackEmpty("Empty*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.EmptyL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.EmptyN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.EmptyG(); break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.EmptyS(); break;
      //XXX: Zend does not allow empty() on the result
      // of a function call. We allow it here so that emitted
      // code is valid. Once the parser handles this correctly,
      // the R and C cases can go.
      case StackSym::R:  e.UnboxR(); // fall through
      case StackSym::C:  e.Not(); break;
      default: {
        unexpectedStackSym(sym, "emitEmpty");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.EmptyM(vectorImm);
  }
}

void EmitterVisitor::emitUnset(Emitter& e) {
  if (checkIfStackEmpty("Unset*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.UnsetL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.UnsetN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.UnsetG(); break;
      case StackSym::LS: // fall through
      case StackSym::CS:
        throw IncludeTimeFatalException(e.getNode(),
                                        "Cannot unset a static property");
      default: {
        unexpectedStackSym(sym, "emitUnset");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.UnsetM(vectorImm);
  }
}

void EmitterVisitor::emitSet(Emitter& e) {
  if (checkIfStackEmpty("Set*")) return;

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.SetL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: emitCGetL2(e); // fall through
      case StackSym::CN: e.SetN();   break;
      case StackSym::LG: emitCGetL2(e); // fall through
      case StackSym::CG: e.SetG();   break;
      case StackSym::LS: emitCGetL3(e); // fall through
      case StackSym::CS: e.SetS();   break;
      default: {
        unexpectedStackSym(sym, "emitSet");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.SetM(vectorImm);
  }
}

void EmitterVisitor::emitSetOp(Emitter& e, int op) {
  if (checkIfStackEmpty("SetOp*")) return;

  unsigned char cop = SetOp_invalid;
  switch (op) {
  case T_PLUS_EQUAL: cop = SetOpPlusEqual; break;
  case T_MINUS_EQUAL: cop = SetOpMinusEqual; break;
  case T_MUL_EQUAL: cop = SetOpMulEqual; break;
  case T_DIV_EQUAL: cop = SetOpDivEqual; break;
  case T_CONCAT_EQUAL: cop = SetOpConcatEqual; break;
  case T_MOD_EQUAL: cop = SetOpModEqual; break;
  case T_AND_EQUAL: cop = SetOpAndEqual; break;
  case T_OR_EQUAL: cop = SetOpOrEqual; break;
  case T_XOR_EQUAL: cop = SetOpXorEqual; break;
  case T_SL_EQUAL: cop = SetOpSlEqual; break;
  case T_SR_EQUAL: cop = SetOpSrEqual; break;
  default: ASSERT(false);
  }
  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.SetOpL(m_evalStack.getLoc(i), cop); break;
      case StackSym::LN: emitCGetL2(e); // fall through
      case StackSym::CN: e.SetOpN(cop); break;
      case StackSym::LG: emitCGetL2(e); // fall through
      case StackSym::CG: e.SetOpG(cop); break;
      case StackSym::LS: emitCGetL3(e); // fall through
      case StackSym::CS: e.SetOpS(cop); break;
      default: {
        unexpectedStackSym(sym, "emitSetOp");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.SetOpM(cop, vectorImm);
  }
}

void EmitterVisitor::emitBind(Emitter& e) {
  if (checkIfStackEmpty("Bind*")) return;

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.BindL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: emitCGetL2(e); // fall through
      case StackSym::CN: e.BindN();  break;
      case StackSym::LG: emitCGetL2(e); // fall through
      case StackSym::CG: e.BindG();  break;
      case StackSym::LS: emitCGetL3(e); // fall through
      case StackSym::CS: e.BindS();  break;
      default: {
        unexpectedStackSym(sym, "emitBind");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.BindM(vectorImm);
  }
}

void EmitterVisitor::emitIncDec(Emitter& e, unsigned char cop) {
  if (checkIfStackEmpty("IncDec*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.IncDecL(m_evalStack.getLoc(i), cop); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.IncDecN(cop); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.IncDecG(cop); break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.IncDecS(cop); break;
      default: {
        unexpectedStackSym(sym, "emitIncDec");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.IncDecM(cop, vectorImm);
  }
}

void EmitterVisitor::emitConvertToCell(Emitter& e) {
  emitCGet(e);
}

void EmitterVisitor::emitFreePendingIters(Emitter& e) {
  for (unsigned i = 0; i < m_pendingIters.size(); ++i) {
    e.IterFree(m_pendingIters[i]);
  }
}

void EmitterVisitor::emitConvertSecondToCell(Emitter& e) {
  if (m_evalStack.size() <= 1) {
    InvariantViolation(
      "Emitter encounted an empty evaluation stack when inside "
      "the emitConvertSecondToCell() function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.get(m_evalStack.size() - 2);
  char symFlavor = StackSym::GetSymFlavor(sym);
  if (symFlavor == StackSym::C) {
    // do nothing
  } else if (symFlavor == StackSym::L) {
    emitCGetL2(e);
  } else {
    // emitConvertSecondToCell() should never be used for symbolic flavors
    // other than C or L
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the emitConvertSecondToCell()"
      " function (at offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::emitConvertToCellIfVar(Emitter& e) {
  if (!m_evalStack.empty()) {
    char sym = m_evalStack.top();
    if (sym == StackSym::V) {
      emitConvertToCell(e);
    }
  }
}

void EmitterVisitor::emitConvertToCellOrLoc(Emitter& e) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encounted an empty evaluation stack when inside "
      "the emitConvertToCellOrLoc() function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::L) {
    // If the top of stack is a loc that is not marked, do nothing
  } else {
    // Otherwise, call emitCGet to convert the top of stack to cell
    emitCGet(e);
  }
}

void EmitterVisitor::emitConvertToVar(Emitter& e) {
  emitVGet(e);
}

/*
 * Class bases are stored on the symbolic stack in a "virtual" way so
 * we can resolve them later (here) in order to properly handle php
 * evaluation order.
 *
 * For example, in:
 *
 *      $cls = 'cls';
 *      $cls::$x[0][f()] = g();
 *
 * We need to evaluate f(), then resolve $cls to an A (possibly
 * invoking an autoload handler), then evaluate g(), then do the set.
 *
 * Complex cases involve unnamed local temporaries.  For example, in:
 *
 *     ${func()}::${f()} = g();
 *
 * We'll emit code which calls func() and stashes the result in a
 * unnamed local.  Then we call f(), then we turn the unnamed local
 * into an 'A' so that autoload handlers will run after f().  Then g()
 * is evaluated and then the set happens.
 */
void EmitterVisitor::emitResolveClsBase(Emitter& e, int pos) {
  switch (m_evalStack.getClsBaseType(pos)) {
  case SymbolicStack::CLS_STRING_NAME:
    e.String(m_evalStack.getName(pos));
    emitAGet(e);
    break;
  case SymbolicStack::CLS_LATE_BOUND:
    e.LateBoundCls();
    break;
  case SymbolicStack::CLS_SELF:
    e.Self();
    break;
  case SymbolicStack::CLS_PARENT:
    e.Parent();
    break;
  case SymbolicStack::CLS_NAMED_LOCAL: {
    int loc = m_evalStack.getLoc(pos);
    emitVirtualLocal(loc);
    emitAGet(e);
    break;
  }
  case SymbolicStack::CLS_UNNAMED_LOCAL: {
    int loc = m_evalStack.getLoc(pos);
    emitVirtualLocal(loc);
    emitAGet(e);
    emitVirtualLocal(loc);
    emitUnset(e);
    newFaultRegion(m_evalStack.getUnnamedLocStart(pos),
                   m_ue.bcPos(),
                   new UnsetUnnamedLocalThunklet(loc));
    m_curFunc->freeUnnamedLocal(loc);
    break;
  }
  case SymbolicStack::CLS_INVALID:
  default:
    ASSERT(false);
  }

  m_evalStack.consumeBelowTop(m_evalStack.size() - pos - 1);
}

void EmitterVisitor::emitClsIfSPropBase(Emitter& e) {
  // If the eval stack is empty, then there is no work to do
  if (m_evalStack.empty()) return;

  // Scan past any values marked with the Elem, NewElem, or Prop markers
  int pos = m_evalStack.size() - 1;
  for (;;) {
    char marker = StackSym::GetMarker(m_evalStack.get(pos));
    if (marker != StackSym::E && marker != StackSym::W &&
        marker != StackSym::P) {
      break;
    }
    --pos;
    if (pos < 0) {
      InvariantViolation("Emitter expected a location on the stack but none "
                         "was found (at offset %d)",
                         m_ue.bcPos());
      return;
    }
  }
  // After scanning, if we did not find a value marked with the SProp
  // marker then there is no work to do
  if (StackSym::GetMarker(m_evalStack.get(pos)) != StackSym::S) {
    return;
  }

  --pos;
  if (pos < 0) {
    InvariantViolation(
      "Emitter emitted an instruction that tries to consume "
      "a value from the stack when the stack is empty "
      "(expected symbolic flavor \"C\" or \"L\" at offset %d)",
      m_ue.bcPos());
  }

  emitResolveClsBase(e, pos);
  m_evalStack.set(m_evalStack.size() - 1,
                  m_evalStack.get(m_evalStack.size() - 1) | StackSym::M);
}

Label* EmitterVisitor::getContinuationGotoLabel(StatementPtr s) {
  Label* label = NULL;
  StatementListPtr stmts(static_pointer_cast<StatementList>(s));
  for (int i = 0; i < stmts->getCount(); i++) {
    StatementPtr s((*stmts)[i]);
    if (s->is(Statement::KindOfFunctionStatement)) {
      FunctionStatementPtr f(static_pointer_cast<FunctionStatement>(s));
      always_assert(f->getFunctionScope()->isGenerator());
      visit(f);
    } else if (s->is(Statement::KindOfGotoStatement)) {
      GotoStatementPtr g(static_pointer_cast<GotoStatement>(s));
      always_assert(label == NULL);
      label = &m_gotoLabels[StringData::GetStaticString(g->label())];
    } else {
      not_implemented();
    }
  }
  always_assert(label);
  return label;
}

void EmitterVisitor::emitContinuationSwitch(Emitter& e,
                                            SwitchStatementPtr sw) {
  StatementListPtr cases(sw->getCases());
  ASSERT(cases);
  const int ncase = cases->getCount();

  // There's an implicit fall-through "label 0" case in the switch
  // statement generated by the parser, so ncase is equal to the
  // number of yields in the body of the php function, which is one
  // less than the number of __yield__ labels.
  if (ncase == 1) {
    // Don't bother with the jump table when there are only two targets
    CaseStatementPtr c1(static_pointer_cast<CaseStatement>((*cases)[0]));
    visit(sw->getExp());
    e.JmpNZ(*getContinuationGotoLabel(c1->getStatement()));
  } else {
    Label case0;
    std::vector<Label*> targets(ncase + 1);
    targets[0] = &case0;
    for (int i = 0; i < ncase; ++i) {
      CaseStatementPtr c(static_pointer_cast<CaseStatement>((*cases)[i]));
      Variant v;
      c->getCondition()->getScalarValue(v);
      int caseIdx = v.asInt64Val();
      ASSERT(caseIdx > 0 && caseIdx <= ncase);
      targets[caseIdx] = getContinuationGotoLabel(c->getStatement());
    }
    visit(sw->getExp());
    e.Switch(targets, 0, 0);
    case0.set(e);
  }
}

DataType EmitterVisitor::analyzeSwitch(SwitchStatementPtr sw,
                                       SwitchState& state) {
  auto& caseMap = state.cases;
  DataType t = KindOfUninit;
  StatementListPtr cases(sw->getCases());
  const int ncase = cases->getCount();

  // Bail if the cases aren't homogeneous
  for (int i = 0; i < ncase; ++i) {
    CaseStatementPtr c(static_pointer_cast<CaseStatement>((*cases)[i]));
    ExpressionPtr condition = c->getCondition();
    if (condition) {
      Variant cval;
      DataType caseType;
      if (condition->getScalarValue(cval)) {
        caseType = cval.getType();
        if (caseType == KindOfStaticString) caseType = KindOfString;
        if ((caseType != KindOfInt64 && caseType != KindOfString) ||
            !IMPLIES(t != KindOfUninit, caseType == t)) {
          return KindOfInvalid;
        }
        t = caseType;
      } else {
        return KindOfInvalid;
      }
      int64 n;
      bool isNonZero;
      if (t == KindOfInt64) {
        n = cval.asInt64Val();
        isNonZero = n;
      } else {
        always_assert(t == KindOfString);
        n = m_ue.mergeLitstr(cval.asStrRef().get());
        isNonZero = false; // not used for string switches
      }
      if (!mapContains(caseMap, n)) {
        // If 'case n:' appears multiple times, only the first will
        // ever match
        caseMap[n] = i;
        if (t == KindOfString) {
          // We have to preserve the original order of the cases for string
          // switches because of insane things like 0 being equal to any string
          // that is not a nonzero numeric string.
          state.caseOrder.push_back(StrCase(safe_cast<Id>(n), i));
        }
      }
      if (state.nonZeroI == -1 && isNonZero) {
        // true is equal to any non-zero integer, so to preserve php's
        // switch semantics we have to remember the first non-zero
        // case to appear in the source text
        state.nonZeroI = i;
      }
    } else {
      // Last 'default:' wins
      state.defI = i;
    }
  }

  if (t == KindOfInt64) {
    int64 base = caseMap.begin()->first;
    int64 nTargets = caseMap.rbegin()->first - base + 1;
    // Fail if the cases are too sparse
    if ((float)caseMap.size() / nTargets < 0.5) {
      return KindOfInvalid;
    }
  } else if (t == KindOfString) {
    if (caseMap.size() < kMinStringSwitchCases) {
      return KindOfInvalid;
    }
  }

  return t;
}

void EmitterVisitor::emitIntegerSwitch(Emitter& e, SwitchStatementPtr sw,
                                       std::vector<Label>& caseLabels,
                                       Label& done, const SwitchState& state) {
  auto& caseMap = state.cases;
  int64 base = caseMap.begin()->first;
  int64 nTargets = caseMap.rbegin()->first - base + 1;

  // It's on. Map case values to Labels, filling in the blanks as
  // appropriate.
  Label* defLabel = state.defI == -1 ? &done : &caseLabels[state.defI];
  std::vector<Label*> labels(nTargets + 2);
  for (int i = 0; i < nTargets; ++i) {
    int caseIdx;
    if (mapGet(caseMap, base + i, &caseIdx)) {
      labels[i] = &caseLabels[caseIdx];
    } else {
      labels[i] = defLabel;
    }
  }

  // Fill in offsets for the first non-zero case and default
  labels[labels.size() - 2] =
    state.nonZeroI == -1 ? defLabel : &caseLabels[state.nonZeroI];
  labels[labels.size() - 1] = defLabel;

  visit(sw->getExp());
  emitConvertToCell(e);
  e.Switch(labels, base, 1);
}

void EmitterVisitor::emitStringSwitch(Emitter& e, SwitchStatementPtr sw,
                                      std::vector<Label>& caseLabels,
                                      Label& done, const SwitchState& state) {
  std::vector<Emitter::StrOff> labels;
  for (auto& pair : state.caseOrder) {
    labels.push_back(Emitter::StrOff(pair.first, &caseLabels[pair.second]));
  }

  // Default case comes last
  Label* defLabel = state.defI == -1 ? &done : &caseLabels[state.defI];
  labels.push_back(Emitter::StrOff(-1, defLabel));

  visit(sw->getExp());
  emitConvertToCell(e);
  e.SSwitch(labels);
}

void EmitterVisitor::markElem(Emitter& e) {
  if (m_evalStack.empty()) {
    InvariantViolation("Emitter encountered an empty evaluation stack inside"
                       " the markElem function (at offset %d)",
                       m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L || sym == StackSym::T ||
      sym == StackSym::I) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::E));
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markElem function (at "
      "offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::markNewElem(Emitter& e) {
  m_evalStack.push(StackSym::W);
}

void EmitterVisitor::markProp(Emitter& e) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encountered an empty evaluation stack inside "
      "the markProp function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L || sym == StackSym::T) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::P));
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markProp function (at "
      "offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::markSProp(Emitter& e) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encountered an empty evaluation stack inside "
      "the markSProp function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::S));
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markSProp function "
      "(at offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

#define MARK_NAME_BODY(index, requiredStackSize)            \
  if (m_evalStack.size() < requiredStackSize) {             \
    InvariantViolation(                                     \
      "Emitter encountered an evaluation stack with %d "    \
      "elements inside the %s function (at offset %d)",     \
      m_evalStack.size(), __FUNCTION__, m_ue.bcPos());      \
    return;                                                 \
  }                                                         \
  char sym = m_evalStack.get(index);                        \
  if (sym == StackSym::C || sym == StackSym::L) {           \
    m_evalStack.set(index, (sym | StackSym::N));            \
  } else {                                                  \
    InvariantViolation(                                     \
      "Emitter encountered an unsupported StackSym \"%s\" " \
      "on the evaluation stack inside the %s function (at " \
      "offset %d)",                                         \
      StackSym::ToString(sym).c_str(), __FUNCTION__,        \
      m_ue.bcPos());                                        \
}

void EmitterVisitor::markName(Emitter& e) {
  int index = m_evalStack.size() - 1;
  MARK_NAME_BODY(index, 1);
}

void EmitterVisitor::markNameSecond(Emitter& e) {
  int index = m_evalStack.size() - 2;
  MARK_NAME_BODY(index, 2);
}

#undef MARK_NAME_BODY

void EmitterVisitor::markGlobalName(Emitter& e) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encountered an empty evaluation stack inside "
      "the markGlobalName function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::G));
  } else {
    InvariantViolation(
      "Emitter encountered an unsupported StackSym \"%s\" on "
      "the evaluation stack inside the markGlobalName function "
      "(at offset %d)",
      StackSym::ToString(sym).c_str(),
      m_ue.bcPos());
  }
}

void EmitterVisitor::emitNameString(Emitter& e, ExpressionPtr n,
                                    bool allowLiteral) {
  Variant v;
  if (n->getScalarValue(v) && v.isString()) {
    StringData* nLiteral = StringData::GetStaticString(v.toCStrRef().get());
    if (allowLiteral) {
      m_evalStack.push(StackSym::T);
    } else {
      e.String(nLiteral);
    }
    m_evalStack.setString(nLiteral);
  } else {
    visit(n);
    emitConvertToCellOrLoc(e);
  }
}

void EmitterVisitor::postponeMeth(MethodStatementPtr m, FuncEmitter* fe,
                                  bool top,
                                  ClosureUseVarVec* useVars /* = NULL */) {
  m_postponedMeths.push_back(PostponedMeth(m, fe, top, useVars));
}

void EmitterVisitor::postponeCtor(InterfaceStatementPtr is, FuncEmitter* fe) {
  m_postponedCtors.push_back(PostponedCtor(is, fe));
}

void EmitterVisitor::postponePinit(InterfaceStatementPtr is, FuncEmitter* fe,
                                   NonScalarVec* v) {
  m_postponedPinits.push_back(PostponedNonScalars(is, fe, v));
}

void EmitterVisitor::postponeSinit(InterfaceStatementPtr is, FuncEmitter* fe,
                                   NonScalarVec* v) {
  m_postponedSinits.push_back(PostponedNonScalars(is, fe, v));
}

void EmitterVisitor::postponeCinit(InterfaceStatementPtr is, FuncEmitter* fe,
                                   NonScalarVec* v) {
  m_postponedCinits.push_back(PostponedNonScalars(is, fe, v));
}

static Attr buildAttrs(ModifierExpressionPtr mod, bool isRef = false) {
  int attrs = AttrNone;
  if (isRef) {
    attrs |= AttrReference;
  }
  if (mod) {
    attrs |= mod->isPublic() ? AttrPublic :
      mod->isPrivate() ? AttrPrivate :
      mod->isProtected() ? AttrProtected : AttrNone;
    if (mod->isStatic()) {
      attrs |= AttrStatic;
    }
    if (mod->isAbstract()) {
      attrs |= AttrAbstract;
    }
    if (mod->isFinal()) {
      attrs |= AttrFinal;
    }
  }
  return Attr(attrs);
}

void EmitterVisitor::emitPostponedMeths() {
  vector<FuncEmitter*> top_fes;
  while (!m_postponedMeths.empty()) {
    ASSERT(m_actualStackHighWater == 0);
    ASSERT(m_fdescHighWater == 0);
    PostponedMeth& p = m_postponedMeths.front();
    FunctionScopePtr funcScope = p.m_meth->getFunctionScope();
    FuncEmitter* fe = p.m_fe;
    if (!fe) {
      ASSERT(p.m_top);
      const StringData* methName =
        StringData::GetStaticString(p.m_meth->getOriginalName());
      fe = new FuncEmitter(m_ue, -1, -1, methName);
      fe->setIsGenerator(funcScope->isGenerator());
      fe->setIsGeneratorFromClosure(funcScope->isGeneratorFromClosure());
      p.m_fe = fe;
      top_fes.push_back(fe);
    }
    const FunctionScope::UserAttributeMap& userAttrs =
      funcScope->userAttributes();
    for (FunctionScope::UserAttributeMap::const_iterator it = userAttrs.begin();
         it != userAttrs.end(); ++it) {
      const StringData* uaName = StringData::GetStaticString(it->first);
      ExpressionPtr uaValue = it->second;
      ASSERT(uaValue);
      ASSERT(uaValue->isScalar());
      TypedValue tv;
      initScalar(tv, uaValue);
      fe->addUserAttribute(uaName, tv);
    }
    Emitter e(p.m_meth, m_ue, *this);
    if (p.m_top) {
      // Set label
      StringData* methName =
        StringData::GetStaticString(p.m_meth->getOriginalName());
      m_methLabels[methName]->set(e);
    }
    typedef std::pair<Id, ConstructPtr> DVInitializer;
    std::vector<DVInitializer> dvInitializers;
    ExpressionListPtr params = p.m_meth->getParams();
    int numParam = params ? params->getCount() : 0;
    for (int i = 0; i < numParam; i++) {
      ParameterExpressionPtr par(
        static_pointer_cast<ParameterExpression>((*params)[i]));
      StringData* parName = StringData::GetStaticString(par->getName());
      if (par->isOptional()) {
        dvInitializers.push_back(DVInitializer(i, par->defaultValue()));
      }
      // Will be fixed up later, when the DV initializers are emitted.
      FuncEmitter::ParamInfo pi;
      if (par->hasTypeHint()) {
        ConstantExpressionPtr ce =
          dynamic_pointer_cast<ConstantExpression>(par->defaultValue());
        bool nullable = ce && ce->isNull();
        TypeConstraint tc =
          TypeConstraint(StringData::GetStaticString(
                                     par->getOriginalTypeHint()),
                                     nullable);
        pi.setTypeConstraint(tc);
        TRACE(1, "Added constraint to %s\n", fe->name()->data());
      }
      // Store info about the default value if there is one.
      if (par->isOptional()) {
        const StringData* phpCode;
        ExpressionPtr vNode = par->defaultValue();
        if (vNode->isScalar()) {
          TypedValue dv;
          initScalar(dv, vNode);
          pi.setDefaultValue(dv);

          // Simple case: it's a scalar value so we just serialize it
          VariableSerializer vs(VariableSerializer::PHPOutput);
          String result = vs.serialize(tvAsCVarRef(&dv), true);
          phpCode = StringData::GetStaticString(result.data());
        } else {
          // Non-scalar, so we have to output PHP from the AST node
          std::ostringstream os;
          CodeGenerator cg(&os, CodeGenerator::PickledPHP);
          AnalysisResultPtr ar(new AnalysisResult());
          vNode->outputPHP(cg, ar);
          phpCode = StringData::GetStaticString(os.str());
        }
        pi.setPhpCode(phpCode);
      }
      ExpressionListPtr paramUserAttrs =
        dynamic_pointer_cast<ExpressionList>(par->userAttributeList());
      if (paramUserAttrs) {
        for (int j = 0; j < paramUserAttrs->getCount(); ++j) {
          UserAttributePtr a = dynamic_pointer_cast<UserAttribute>(
            (*paramUserAttrs)[j]);
          StringData* uaName = StringData::GetStaticString(a->getName());
          ExpressionPtr uaValue = a->getExp();
          ASSERT(uaValue);
          ASSERT(uaValue->isScalar());
          TypedValue tv;
          initScalar(tv, uaValue);
          pi.addUserAttribute(uaName, tv);
        }
      }

      pi.setRef(par->isRef());
      fe->appendParam(parName, pi);
    }

    m_curFunc = fe;

    // Assign ids to all of the local variables eagerly. This gives us the
    // nice property that all named local variables will be assigned ids
    // 0 through k-1, while any unnamed local variable will have an id >= k.
    // Note that the logic above already assigned ids to the parameters, so
    // we will still uphold the invariant that the n parameters will have
    // ids 0 through n-1 respectively.
    assignLocalVariableIds(funcScope);

    // set all the params and metadata etc on fe
    StringData* methDoc =
      StringData::GetStaticString(p.m_meth->getDocComment());
    ModifierExpressionPtr mod(p.m_meth->getModifiers());
    Attr attrs = buildAttrs(mod, p.m_meth->isRef());

    if (funcScope->mayUseVV()) {
      attrs = attrs | AttrMayUseVV;
    }

    if (Option::WholeProgram) {
      if (!funcScope->isRedeclaring()) {
        attrs = attrs | AttrUnique;
        if (p.m_top &&
            (!funcScope->isVolatile() ||
             funcScope->isPersistent() ||
             funcScope->isGenerator())) {
          attrs = attrs | AttrPersistent;
        }
      }
      if (ClassScopePtr cls = p.m_meth->getClassScope()) {
        if (p.m_meth->getName() == cls->getName() &&
            !cls->classNameCtor()) {
          /*
            In WholeProgram mode, we inline the traits into their
            classes. If a trait method name matches the class name
            it is NOT a constructor.
            We mark the method with AttrTrait so that we can avoid
            treating it as a constructor even though it looks like
            one.
          */
          attrs = attrs | AttrTrait;
        }
        if (!funcScope->hasOverride()) {
          attrs = attrs | AttrNoOverride;
        }
      }
    } else if (!SystemLib::s_inited) {
      // we're building systemlib. everything is unique
      attrs = attrs | AttrUnique | AttrPersistent;
    }

    // For closures, the MethodStatement didn't have real attributes; enforce
    // that the __invoke method is public here
    if (fe->isClosureBody()) {
      ASSERT(!(attrs & (AttrProtected | AttrPrivate)));
      attrs = attrs | AttrPublic;
    }

    Label topOfBody(e);
    const Location* sLoc = p.m_meth->getLocation().get();
    fe->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), attrs, p.m_top, methDoc);
    // --Method emission begins--
    {
      if (funcScope->needsLocalThis() &&
          !funcScope->isStatic() &&
          !funcScope->isGenerator()) {
        ASSERT(!p.m_top);
        static const StringData* thisStr = StringData::GetStaticString("this");
        Id thisId = fe->lookupVarId(thisStr);
        e.InitThisLoc(thisId);
      }
      for (uint i = 0; i < fe->params().size(); i++) {
        const TypeConstraint& tc = fe->params()[i].typeConstraint();
        if (!tc.exists()) continue;
        TRACE(2, "permanent home for tc %s, param %d of func %s: %p\n",
              tc.typeName()->data(), i, fe->name()->data(), &tc);
        ASSERT(tc.typeName()->data() != (const char*)0xdeadba5eba11f00d);
        e.VerifyParamType(i);
      }
      if (fe->isClosureBody()) {
        ASSERT(p.m_closureUseVars != NULL);
        // Emit code to unpack the instance variables (which store the
        // use-variables) into locals. Some of the use-variables may have the
        // same name, in which case the last one wins.
        unsigned n = p.m_closureUseVars->size();
        for (unsigned i = 0; i < n; ++i) {
          StringData* name = (*p.m_closureUseVars)[i].first;
          bool byRef = (*p.m_closureUseVars)[i].second;
          emitVirtualLocal(fe->lookupVarId(name));
          if (i) {
            m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                        false, 0, 0);
          }
          e.CheckThis();
          m_evalStack.push(StackSym::H);
          m_evalStack.push(StackSym::T);
          m_evalStack.setString(name);
          markProp(e);
          if (byRef) {
            emitVGet(e);
            emitBind(e);
          } else {
            emitCGet(e);
            emitSet(e);
          }
          emitPop(e);
        }
      }
    }
    Label ctFatal, ctFatalWithPop;
    {
      CONTROL_BODY(ctFatal, ctFatal, ctFatalWithPop, ctFatalWithPop);
      // emit body
      visit(p.m_meth->getStmts());
    }
    // If the break/continue label was used, we need to emit some extra code to
    // handle stray breaks
    Label exit;
    if (ctFatal.isUsed() || ctFatalWithPop.isUsed()) {
      e.Jmp(exit);
      if (ctFatalWithPop.isUsed()) {
        ctFatalWithPop.set(e);
        e.PopC();
      }
      if (ctFatal.isUsed()) {
        ctFatal.set(e);
      }
      static const StringData* msg =
        StringData::GetStaticString("Cannot break/continue");
      e.String(msg);
      e.Fatal();
    }
    if (exit.isUsed()) {
      exit.set(e);
    }
    // If the current position in the bytecode is reachable, emit code to
    // return null
    if (currentPositionIsReachable()) {
      if (p.m_meth->getFunctionScope()->isGenerator()) {
        e.ContExit();
      } else {
        e.Null();
        if ((p.m_meth->getStmts() && p.m_meth->getStmts()->isGuarded()) ||
            (fe->isClosureBody() && p.m_closureUseVars->size())) {
          m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                         false, 0, 0);
        }
        e.RetC();
      }
    } // -- Method emission ends --

    FuncFinisher ff(this, e, p.m_fe);

    // Default value initializers
    for (uint i = 0; i < dvInitializers.size(); ++i) {
      Label entryPoint(e);
      Id paramId = dvInitializers[i].first;
      ConstructPtr node = dvInitializers[i].second;
      emitVirtualLocal(paramId, KindOfUninit);
      visit(node);
      emitCGet(e);
      emitSet(e);
      e.PopC();
      p.m_fe->setParamFuncletOff(paramId, entryPoint.getAbsoluteOffset());
    }
    if (!dvInitializers.empty()) {
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NoSurprise, false, 0, 0);
      e.Jmp(topOfBody);
    }
    delete p.m_closureUseVars;
    m_postponedMeths.pop_front();
  }

  for (size_t i = 0; i < top_fes.size(); i++) {
    m_ue.appendTopEmitter(top_fes[i]);
  }
}

void EmitterVisitor::emitPostponedCtors() {
  while (!m_postponedCtors.empty()) {
    PostponedCtor& p = m_postponedCtors.front();

    Attr attrs = AttrPublic;
    StringData* methDoc = empty_string.get();
    const Location* sLoc = p.m_is->getLocation().get();
    p.m_fe->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), attrs, false, methDoc);
    Emitter e(p.m_is, m_ue, *this);
    FuncFinisher ff(this, e, p.m_fe);
    e.Null();
    e.RetC();

    m_postponedCtors.pop_front();
  }
}

void EmitterVisitor::emitPostponedPSinit(PostponedNonScalars& p, bool pinit) {
  Attr attrs = (Attr)(AttrPrivate | AttrStatic);
  StringData* methDoc = empty_string.get();
  const Location* sLoc = p.m_is->getLocation().get();
  p.m_fe->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), attrs, false, methDoc);
  {
    FuncEmitter::ParamInfo pi;
    pi.setRef(true);
    static const StringData* s_props = StringData::GetStaticString("props");
    p.m_fe->appendParam(s_props, pi);
  }
  if (pinit) {
    static const StringData* s_sentinel =
      StringData::GetStaticString("sentinel");
    p.m_fe->appendParam(s_sentinel,
                        FuncEmitter::ParamInfo());
  }

  Emitter e(p.m_is, m_ue, *this);
  FuncFinisher ff(this, e, p.m_fe);

  // Generate HHBC of the structure:
  //
  //   private static function 86pinit(&$props, $sentinel) {
  //     # Private instance properties.
  //     props["\0C\0p0"] = <non-scalar initialization>;
  //     props["\0C\0p1"] = <non-scalar initialization>;
  //     # ...
  //
  //     if (props["q0"]) === $sentinel) {
  //       props["q0"] = <non-scalar initialization>;
  //     }
  //     if (props["q1"] === $sentinel) {
  //       props["q1"] = <non-scalar initialization>;
  //     }
  //     # ...
  //   }
  //
  //   private static function 86sinit(&$props) {
  //     props["p0"] = <non-scalar initialization>;
  //     props["p1"] = <non-scalar initialization>;
  //     # ...
  //   }
  size_t nProps = p.m_vec->size();
  ASSERT(nProps > 0);
  for (size_t i = 0; i < nProps; ++i) {
    const StringData* propName =
      StringData::GetStaticString(((*p.m_vec)[i]).first);
    Label isset;

    bool conditional;
    if (pinit) {
      const PreClassEmitter::Prop& preProp =
        p.m_fe->pce()->lookupProp(propName);
      if ((preProp.attrs() & (AttrPrivate|AttrStatic)) == AttrPrivate) {
        conditional = false;
        propName = preProp.mangledName();
      } else {
        conditional = true;
      }
    } else {
      conditional = false;
    }

    if (conditional) {
      emitVirtualLocal(0);
      e.String((StringData*)propName);
      markElem(e);
      emitCGet(e);
      emitVirtualLocal(1);
      emitCGet(e);
      e.Same();
      e.JmpZ(isset);
    }

    emitVirtualLocal(0);
    e.String((StringData*)propName);
    markElem(e);
    visit((*p.m_vec)[i].second);
    emitSet(e);
    e.PopC();

    isset.set(e);
  }
  e.Null();
  e.RetC();
}

void EmitterVisitor::emitPostponedPinits() {
  while (!m_postponedPinits.empty()) {
    PostponedNonScalars& p = m_postponedPinits.front();
    emitPostponedPSinit(p, true);
    p.release(); // Manually trigger memory cleanup.
    m_postponedPinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedSinits() {
  while (!m_postponedSinits.empty()) {
    PostponedNonScalars& p = m_postponedSinits.front();
    emitPostponedPSinit(p, false);
    p.release(); // Manually trigger memory cleanup.
    m_postponedSinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedCinits() {
  while (!m_postponedCinits.empty()) {
    PostponedNonScalars& p = m_postponedCinits.front();

    Attr attrs = (Attr)(AttrPrivate | AttrStatic);
    StringData* methDoc = empty_string.get();
    const Location* sLoc = p.m_is->getLocation().get();
    p.m_fe->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), attrs, false, methDoc);
    static const StringData* s_constName =
      StringData::GetStaticString("constName");
    p.m_fe->appendParam(s_constName,
                        FuncEmitter::ParamInfo());

    Emitter e(p.m_is, m_ue, *this);
    FuncFinisher ff(this, e, p.m_fe);

    // Generate HHBC of the structure:
    //
    //   private static function 86cinit(constName) {
    //     if (constName == "FOO") {
    //       return <expr for FOO>;
    //     } else if (constName == "BAR") {
    //       return <expr for BAR>;
    //     } else { # (constName == "BAZ")
    //        return <expr for BAZ>;
    //     }
    //   }
    size_t nConsts = p.m_vec->size();
    ASSERT(nConsts > 0);
    Label retC;
    for (size_t i = 0; i < nConsts - 1; ++i) {
      Label mismatch;

      emitVirtualLocal(0);
      emitCGet(e);
      e.String((StringData*)StringData::GetStaticString(((*p.m_vec)[i]).first));
      e.Eq();
      e.JmpZ(mismatch);

      visit((*p.m_vec)[i].second);

      e.Jmp(retC);
      mismatch.set(e);
    }
    visit((*p.m_vec)[nConsts-1].second);
    retC.set(e);
    e.RetC();

    p.release(); // Manually trigger memory cleanup.
    m_postponedCinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedClosureCtors() {
  while (!m_postponedClosureCtors.empty()) {
    PostponedClosureCtor& ctor = m_postponedClosureCtors.front();
    ClosureUseVarVec& useVars = ctor.m_useVars;
    FuncEmitter* fe = ctor.m_fe;
    const Location* sLoc = ctor.m_expr->getLocation().get();
    fe->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), AttrPublic, false, NULL);

    unsigned n = useVars.size();
    Emitter e(ctor.m_expr, m_ue, *this);
    FuncFinisher ff(this, e, fe);
    if (n > 0) {
      for (unsigned i = 0; i < n; ++i) {
        // To ensure that we get a new local for every use var, we call
        // appendParam with an artificial uniquified name. Because there's no
        // user code here, the fact that the variable has a made-up name in the
        // metadata doesn't matter.
        std::ostringstream num;
        num << i;
        FuncEmitter::ParamInfo pi;
        pi.setRef(useVars[i].second);
        fe->appendParam(StringData::GetStaticString(num.str()), pi);
        if (i) {
          m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                      false, 0, 0);
        }
        e.CheckThis();
        m_evalStack.push(StackSym::H);
        m_evalStack.push(StackSym::T);
        m_evalStack.setString(useVars[i].first);
        markProp(e);
        emitVirtualLocal(i);
        if (useVars[i].second) {
          emitVGet(e);
          emitBind(e);
        } else {
          emitCGet(e);
          emitSet(e);
        }
        emitPop(e);
      }
    }
    e.Null();
    if (n > 0) {
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::GuardedThis,
                  false, 0, 0);
    }
    e.RetC();

    m_postponedClosureCtors.pop_front();
  }
}

void EmitterVisitor::emitVirtualLocal(int localId,
                                      DataType dt /* = KindOfUnknown */) {
  prepareEvalStack();

  m_evalStack.push(StackSym::L);
  m_evalStack.setInt(localId);
  if (dt != KindOfUnknown) {
    m_evalStack.setKnownType(dt);
    m_evalStack.setNotRef();
  }
}

template<class Expr>
void EmitterVisitor::emitVirtualClassBase(Emitter& e, Expr* node) {
  prepareEvalStack();

  m_evalStack.push(StackSym::K);

  if (node->isStatic()) {
    m_evalStack.setClsBaseType(SymbolicStack::CLS_LATE_BOUND);
  } else if (node->getClass()) {
    const ExpressionPtr& expr = node->getClass();
    SimpleVariable* sv = static_cast<SimpleVariable*>(expr.get());
    if (expr->is(Expression::KindOfSimpleVariable) &&
        !sv->isSuperGlobal() &&
        !sv->isThis()) {
      StringData* name = StringData::GetStaticString(sv->getName());
      Id locId = m_curFunc->lookupVarId(name);
      m_evalStack.setClsBaseType(SymbolicStack::CLS_NAMED_LOCAL);
      m_evalStack.setInt(locId);
    } else {
      /*
       * More complex expressions get stashed into an unnamed local so
       * we can evaluate them at the proper time.
       *
       * See emitResolveClsBase() for examples.
       */
      int unnamedLoc = m_curFunc->allocUnnamedLocal();
      int clsBaseIdx = m_evalStack.size() - 1;
      m_evalStack.setClsBaseType(SymbolicStack::CLS_UNNAMED_LOCAL);
      emitVirtualLocal(unnamedLoc);
      visit(node->getClass());
      emitConvertToCell(e);
      emitSet(e);
      m_evalStack.setUnnamedLocal(clsBaseIdx, unnamedLoc, m_ue.bcPos());
      emitPop(e);
    }
  } else if (!node->getOriginalClass() ||
             node->getOriginalClass()->isTrait()) {
    // In a trait or psuedo-main, we can't resolve self:: or parent::
    // yet, so we emit special instructions that do those lookups.
    if (node->isParent()) {
      m_evalStack.setClsBaseType(SymbolicStack::CLS_PARENT);
    } else if (node->isSelf()) {
      m_evalStack.setClsBaseType(SymbolicStack::CLS_SELF);
    } else {
      m_evalStack.setClsBaseType(SymbolicStack::CLS_STRING_NAME);
      m_evalStack.setString(
        StringData::GetStaticString(node->getClassName()));
    }
  } else if (node->isParent() &&
             node->getOriginalClass()->getOriginalParent().empty()) {
    // parent:: in a class without a parent.  We'll emit a Parent
    // opcode because it can handle this error case.
    m_evalStack.setClsBaseType(SymbolicStack::CLS_PARENT);
  } else {
    m_evalStack.setClsBaseType(SymbolicStack::CLS_STRING_NAME);
    m_evalStack.setString(
      StringData::GetStaticString(node->getClassName()));
  }
}

bool EmitterVisitor::emitCallUserFunc(Emitter& e, SimpleFunctionCallPtr func) {
  static struct {
    const char* name;
    int minParams, maxParams;
    CallUserFuncFlags flags;
  } cufTab[] = {
    { "call_user_func", 1, INT_MAX, CallUserFuncPlain },
    { "call_user_func_array", 2, 2, CallUserFuncArray },
    { "forward_static_call", 1, INT_MAX, CallUserFuncForward },
    { "forward_static_call_array", 2, 2, CallUserFuncForwardArray },
    { "fb_call_user_func_safe", 1, INT_MAX, CallUserFuncSafe },
    { "fb_call_user_func_array_safe", 2, 2, CallUserFuncSafeArray },
    { "fb_call_user_func_safe_return", 2, INT_MAX, CallUserFuncSafeReturn },
  };

  ExpressionListPtr params = func->getParams();
  if (!params) return false;
  int nParams = params->getCount();
  if (!nParams) return false;
  CallUserFuncFlags flags = CallUserFuncNone;
  for (unsigned i = 0; i < sizeof(cufTab) / sizeof(cufTab[0]); i++) {
    if (func->isCallToFunction(cufTab[i].name) &&
        nParams >= cufTab[i].minParams &&
        nParams <= cufTab[i].maxParams) {
      flags = cufTab[i].flags;
      break;
    }
  }
  if (flags == CallUserFuncNone) return false;

  int param = 1;
  ExpressionPtr callable = (*params)[0];
  visit(callable);
  emitConvertToCell(e);
  Offset fpiStart = m_ue.bcPos();
  if (flags & CallUserFuncForward) {
    e.FPushCufF(nParams - param);
  } else if (flags & CallUserFuncSafe) {
    if (flags & CallUserFuncReturn) {
      ASSERT(nParams >= 2);
      visit((*params)[param++]);
      emitConvertToCell(e);
    } else {
      e.Null();
    }
    fpiStart = m_ue.bcPos();
    e.FPushCufSafe(nParams - param);
  } else {
    e.FPushCuf(nParams - param);
  }

  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    for (int i = param; i < nParams; i++) {
      visit((*params)[i]);
      emitConvertToCell(e);
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
      e.FPassC(i - param);
    }
  }

  if (flags & CallUserFuncArray) {
    e.FCallArray();
  } else {
    e.FCall(nParams - param);
  }
  if (flags & CallUserFuncSafe) {
    if (flags & CallUserFuncReturn) {
      e.CufSafeReturn();
    } else {
      e.CufSafeArray();
    }
  }
  return true;
}

bool EmitterVisitor::canEmitBuiltinCall(FunctionCallPtr fn,
                                        const std::string& name,
                                        int numParams) {
  if (Option::JitEnableRenameFunction) {
    return false;
  }
  if (Option::DynamicInvokeFunctions.size()) {
    if (Option::DynamicInvokeFunctions.find(name) !=
        Option::DynamicInvokeFunctions.end()) {
      return false;
    }
  }
  FunctionScopePtr func = fn->getFuncScope();
  if (!func || func->getMaxParamCount() > kMaxBuiltinArgs) {
    return false;
  }
  if (!func->isUserFunction() && !func->needsActRec()
      && (uint)numParams >= func->getMinParamCount()
      && (uint)numParams <= func->getMaxParamCount()) {
    if (func->isVariableArgument() || func->isReferenceVariableArgument()
                || func->isMixedVariableArgument()) {
      return false;
    }
    TypePtr t = func->getReturnType();
    if (!t || t->getHhvmDataType() == KindOfDouble) {
      return false;
    }
    for (int i = 0; i < func->getMaxParamCount(); i++) {
      t = func->getParamType(i);
      if (!t || t->getHhvmDataType() == KindOfDouble) {
        return false;
      }
      // unserializable default values such as TimeStamp::Current()
      // are serialized as kUnserializableString ("\x01")
      if (i >= numParams && func->getParamDefault(i) == kUnserializableString) {
        return false;
      }
    }
    // Special handling for func_get_args and friends inside a generator.
    if (m_curFunc->isGenerator() &&
        (name == "func_get_args" || name == "func_num_args"
         || name == "func_get_arg")) {
      return false;
    }
    // in sandbox mode, don't emit FCallBuiltin for redefinable functions
    if (func->ignoreRedefinition() && !Option::WholeProgram) {
      return false;
    }
    return true;
  }
  return false;
}

void EmitterVisitor::emitFuncCall(Emitter& e, FunctionCallPtr node) {
  ExpressionPtr nameExp = node->getNameExp();
  const std::string& nameStr = node->getOriginalName();
  ExpressionListPtr params(node->getParams());
  int numParams = params ? params->getCount() : 0;
  bool isBuiltinCall = false;
  StringData* nLiteral = NULL;
  Offset fpiStart;
  if (node->getClass() || !node->getClassName().empty()) {
    bool isSelfOrParent = node->isSelf() || node->isParent();
    if (!node->isStatic() && !isSelfOrParent &&
        !node->getOriginalClassName().empty() && !nameStr.empty()) {
      // cls::foo()
      StringData* cLiteral =
        StringData::GetStaticString(node->getOriginalClassName());
      StringData* nLiteral = StringData::GetStaticString(nameStr);
      fpiStart = m_ue.bcPos();
      e.FPushClsMethodD(numParams, nLiteral, cLiteral);
      if (node->forcedPresent()) {
        m_metaInfo.add(fpiStart, Unit::MetaInfo::GuardedCls, false, 0, 0);
      }
    } else {
      emitVirtualClassBase(e, node.get());
      if (!nameStr.empty()) {
        // ...::foo()
        StringData* nLiteral = StringData::GetStaticString(nameStr);
        e.String(nLiteral);
      } else {
        // ...::$foo()
        visit(nameExp);
        emitConvertToCell(e);
      }
      emitResolveClsBase(e, m_evalStack.size() - 2);
      fpiStart = m_ue.bcPos();
      if (isSelfOrParent) {
        // self and parent are "forwarding" calls, so we need to
        // use FPushClsMethodF instead
        e.FPushClsMethodF(numParams);
      } else {
        e.FPushClsMethod(numParams);
      }
    }
  } else if (!nameStr.empty()) {
    // foo()
    nLiteral = StringData::GetStaticString(nameStr);
    isBuiltinCall = canEmitBuiltinCall(node, nameStr, numParams);

    if (isBuiltinCall) {
    } else if (!m_curFunc->isGenerator()) {
      fpiStart = m_ue.bcPos();
      e.FPushFuncD(numParams, nLiteral);
    } else {
      // Special handling for func_get_args and friends inside a generator.
      const StringData* specialMethodName = NULL;
      static const StringData* contName =
        StringData::GetStaticString(CONTINUATION_OBJECT_NAME);
      Id contId = m_curFunc->lookupVarId(contName);
      static const StringData* s_get_args =
        StringData::GetStaticString("get_args");
      static const StringData* s_num_args =
        StringData::GetStaticString("num_args");
      static const StringData* s_get_arg =
        StringData::GetStaticString("get_arg");
      if (nameStr == "func_get_args") {
        specialMethodName = s_get_args;
      } else if (nameStr == "func_num_args") {
        specialMethodName = s_num_args;
      } else if (nameStr == "func_get_arg") {
        specialMethodName = s_get_arg;
      }

      if (specialMethodName != NULL) {
        emitVirtualLocal(contId);
        emitCGet(e);
        fpiStart = m_ue.bcPos();
        e.FPushObjMethodD(numParams, specialMethodName);
      } else {
        fpiStart = m_ue.bcPos();
        e.FPushFuncD(numParams, nLiteral);
      }
    }
  } else {
    // $foo()
    visit(nameExp);
    emitConvertToCell(e);
    // FPushFunc consumes method name from stack
    fpiStart = m_ue.bcPos();
    e.FPushFunc(numParams);
  }
  if (isBuiltinCall) {
    FunctionScopePtr func = node->getFuncScope();
    ASSERT(func);
    ASSERT(numParams <= func->getMaxParamCount()
           && numParams >= func->getMinParamCount());
    int i = 0;
    for (; i < numParams; i++) {
      // for builtin calls, since we don't push the ActRec, we
      // must determine the reffiness statically
      bool byRef = func->isRefParam(i);
      emitBuiltinCallArg(e, (*params)[i], i, byRef);
    }
    for (; i < func->getMaxParamCount(); i++) {
      Variant v = f_unserialize(func->getParamDefault(i));
      TypePtr t = func->getParamType(i);
      emitBuiltinDefaultArg(e, v, t->getDataType(), i);
    }
    e.FCallBuiltin(func->getMaxParamCount(), numParams, nLiteral);
  } else {
    {
      FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
      for (int i = 0; i < numParams; i++) {
        emitFuncCallArg(e, (*params)[i], i);
      }
    }
    e.FCall(numParams);
  }
  if (Option::WholeProgram) {
    fixReturnType(e, node, isBuiltinCall);
  }
}

void EmitterVisitor::emitClassTraitPrecRule(PreClassEmitter* pce,
                                            TraitPrecStatementPtr stmt) {
  StringData* traitName  = StringData::GetStaticString(stmt->getTraitName());
  StringData* methodName = StringData::GetStaticString(stmt->getMethodName());

  PreClass::TraitPrecRule rule(traitName, methodName);

  std::set<std::string> otherTraitNames;
  stmt->getOtherTraitNames(otherTraitNames);
  for (std::set<std::string>::iterator it = otherTraitNames.begin();
       it != otherTraitNames.end(); it++) {
    rule.addOtherTraitName(StringData::GetStaticString(*it));
  }

  pce->addTraitPrecRule(rule);
}

void EmitterVisitor::emitClassTraitAliasRule(PreClassEmitter* pce,
                                             TraitAliasStatementPtr stmt) {
  StringData* traitName    = StringData::GetStaticString(stmt->getTraitName());
  StringData* origMethName = StringData::GetStaticString(stmt->getMethodName());
  StringData* newMethName  =
    StringData::GetStaticString(stmt->getNewMethodName());
  // If there are no modifiers, buildAttrs() defaults to AttrPublic.
  // Here we don't want that. Instead, set AttrNone so that the modifiers of the
  // original method are preserved.
  Attr attr = (stmt->getModifiers()->getCount() == 0 ? AttrNone :
               buildAttrs(stmt->getModifiers()));

  PreClass::TraitAliasRule rule(traitName, origMethName, newMethName, attr);

  pce->addTraitAliasRule(rule);
}

void EmitterVisitor::emitClassUseTrait(PreClassEmitter* pce,
                                       UseTraitStatementPtr useStmt) {
  StatementListPtr rules = useStmt->getStmts();
  for (int r = 0; r < rules->getCount(); r++) {
    StatementPtr rule = (*rules)[r];
    TraitPrecStatementPtr precStmt =
      dynamic_pointer_cast<TraitPrecStatement>(rule);
    if (precStmt) {
      emitClassTraitPrecRule(pce, precStmt);
    } else {
      TraitAliasStatementPtr aliasStmt =
        dynamic_pointer_cast<TraitAliasStatement>(rule);
      ASSERT(aliasStmt);
      emitClassTraitAliasRule(pce, aliasStmt);
    }
  }
}

PreClass::Hoistable EmitterVisitor::emitClass(Emitter& e, ClassScopePtr cNode,
                                              bool toplevel) {
  InterfaceStatementPtr is(
    static_pointer_cast<InterfaceStatement>(cNode->getStmt()));
  StringData* className = StringData::GetStaticString(cNode->getOriginalName());
  StringData* parentName =
    StringData::GetStaticString(cNode->getOriginalParent());
  StringData* classDoc = StringData::GetStaticString(cNode->getDocComment());
  Attr attr = cNode->isInterface() ? AttrInterface :
              cNode->isTrait()     ? AttrTrait     :
              cNode->isAbstract()  ? AttrAbstract  :
              cNode->isFinal()     ? AttrFinal     :
                                     AttrNone;
  if (Option::WholeProgram) {
    if (!cNode->isRedeclaring() &&
        !cNode->derivesFromRedeclaring()) {
      attr = attr | AttrUnique;
      if (!cNode->isVolatile()) {
        attr = attr | AttrPersistent;
      }
    }
    if (!cNode->getAttribute(ClassScope::NotFinal)) {
      attr = attr | AttrNoOverride;
    }
    if (cNode->getUsedTraitNames().size()) {
      attr = attr | AttrNoExpandTrait;
    }
  } else if (!SystemLib::s_inited) {
    // we're building systemlib. everything is unique
    attr = attr | AttrUnique | AttrPersistent;
  }

  const Location* sLoc = is->getLocation().get();
  const std::vector<std::string>& bases(cNode->getBases());
  int firstInterface = cNode->getOriginalParent().empty() ? 0 : 1;
  int nInterfaces = bases.size();
  PreClass::Hoistable hoistable = PreClass::NotHoistable;
  if (toplevel) {
    if (SystemLib::s_inited) {
      if (nInterfaces > firstInterface || cNode->getUsedTraitNames().size()) {
        hoistable = PreClass::Mergeable;
      } else if (firstInterface &&
                 !m_hoistables.count(cNode->getOriginalParent())) {
        hoistable = PreClass::MaybeHoistable;
      }
    }
    if (hoistable == PreClass::NotHoistable) {
      hoistable = attr & AttrUnique ?
        PreClass::AlwaysHoistable : PreClass::MaybeHoistable;
      m_hoistables.insert(cNode->getOriginalName());
    }
  }
  PreClassEmitter* pce = m_ue.newPreClassEmitter(className, hoistable);
  pce->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), attr, parentName,
            classDoc);
  if (hoistable != PreClass::AlwaysHoistable) {
    e.DefCls(pce->id());
  }
  for (int i = firstInterface; i < nInterfaces; ++i) {
    pce->addInterface(StringData::GetStaticString(bases[i]));
  }

  const std::vector<std::string>& usedTraits = cNode->getUsedTraitNames();
  for (size_t i = 0; i < usedTraits.size(); i++) {
    pce->addUsedTrait(StringData::GetStaticString(usedTraits[i]));
  }
  const ClassScope::UserAttributeMap& userAttrs = cNode->userAttributes();
  for (ClassScope::UserAttributeMap::const_iterator it = userAttrs.begin();
       it != userAttrs.end(); ++it) {
    const StringData* uaName = StringData::GetStaticString(it->first);
    ExpressionPtr uaValue = it->second;
    ASSERT(uaValue);
    ASSERT(uaValue->isScalar());
    TypedValue tv;
    initScalar(tv, uaValue);
    pce->addUserAttribute(uaName, tv);
  }

  NonScalarVec* nonScalarPinitVec = NULL;
  NonScalarVec* nonScalarSinitVec = NULL;
  NonScalarVec* nonScalarConstVec = NULL;
  if (StatementListPtr stmts = is->getStmts()) {
    int i, n = stmts->getCount();
    for (i = 0; i < n; i++) {
      if (MethodStatementPtr meth =
          dynamic_pointer_cast<MethodStatement>((*stmts)[i])) {
        StringData* methName =
          StringData::GetStaticString(meth->getOriginalName());
        FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
        bool isGenerator = meth->getFunctionScope()->isGenerator();
        fe->setIsGenerator(isGenerator);
        fe->setIsGeneratorFromClosure(
          meth->getFunctionScope()->isGeneratorFromClosure());
        bool added UNUSED = pce->addMethod(fe);
        ASSERT(added);
        postponeMeth(meth, fe, false);
      } else if (ClassVariablePtr cv =
                 dynamic_pointer_cast<ClassVariable>((*stmts)[i])) {
        ModifierExpressionPtr mod(cv->getModifiers());
        ExpressionListPtr el(cv->getVarList());
        Attr attrs = buildAttrs(mod);
        int nVars = el->getCount();
        for (int ii = 0; ii < nVars; ii++) {
          ExpressionPtr exp((*el)[ii]);
          ExpressionPtr vNode;
          SimpleVariablePtr var;
          if (exp->is(Expression::KindOfAssignmentExpression)) {
            AssignmentExpressionPtr ae(
              static_pointer_cast<AssignmentExpression>(exp));
            var = static_pointer_cast<SimpleVariable>(ae->getVariable());
            vNode = ae->getValue();
          } else {
            var = static_pointer_cast<SimpleVariable>(exp);
          }

          StringData* propName = StringData::GetStaticString(var->getName());
          StringData* propDoc = empty_string.get();
          TypedValue tvVal;
          if (vNode) {
            if (vNode->isScalar()) {
              initScalar(tvVal, vNode);
            } else {
              tvWriteUninit(&tvVal);
              if (!(attrs & AttrStatic)) {
                if (nonScalarPinitVec == NULL) {
                  nonScalarPinitVec = new NonScalarVec();
                }
                nonScalarPinitVec->push_back(NonScalarPair(propName, vNode));
              } else {
                if (nonScalarSinitVec == NULL) {
                  nonScalarSinitVec = new NonScalarVec();
                }
                nonScalarSinitVec->push_back(NonScalarPair(propName, vNode));
              }
            }
          } else {
            tvWriteNull(&tvVal);
          }
          bool added UNUSED =
            pce->addProperty(propName, attrs, propDoc, &tvVal);
          ASSERT(added);
        }
      } else if (ClassConstantPtr cc =
                 dynamic_pointer_cast<ClassConstant>((*stmts)[i])) {
        ExpressionListPtr el(cc->getConList());
        int nCons = el->getCount();
        for (int ii = 0; ii < nCons; ii++) {
          AssignmentExpressionPtr ae(
            static_pointer_cast<AssignmentExpression>((*el)[ii]));
          ConstantExpressionPtr con(
            static_pointer_cast<ConstantExpression>(ae->getVariable()));
          ExpressionPtr vNode(ae->getValue());
          StringData* constName = StringData::GetStaticString(con->getName());
          ASSERT(vNode);
          TypedValue tvVal;
          if (vNode->isArray()) {
            throw IncludeTimeFatalException(
              cc, "Arrays are not allowed in class constants");
          } else if (vNode->isScalar()) {
            initScalar(tvVal, vNode);
          } else {
            tvWriteUninit(&tvVal);
            if (nonScalarConstVec == NULL) {
              nonScalarConstVec = new NonScalarVec();
            }
            nonScalarConstVec->push_back(NonScalarPair(constName, vNode));
          }
          // Store PHP source code for constant initializer.
          std::ostringstream os;
          CodeGenerator cg(&os, CodeGenerator::PickledPHP);
          AnalysisResultPtr ar(new AnalysisResult());
          vNode->outputPHP(cg, ar);
          bool added UNUSED = pce->addConstant(
            constName, &tvVal, StringData::GetStaticString(os.str()));
          ASSERT(added);
        }
      } else if (UseTraitStatementPtr useStmt =
                 dynamic_pointer_cast<UseTraitStatement>((*stmts)[i])) {
        emitClassUseTrait(pce, useStmt);
      }
    }
  }

  if (!cNode->getAttribute(ClassScope::HasConstructor) &&
      !cNode->getAttribute(ClassScope::ClassNameConstructor)) {
    // cNode does not have a constructor; synthesize 86ctor() so that the class
    // will always have a method that can be called during construction.
    static const StringData* methName = StringData::GetStaticString("86ctor");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    bool added UNUSED = pce->addMethod(fe);
    ASSERT(added);
    postponeCtor(is, fe);
  }

  if (nonScalarPinitVec != NULL) {
    // Non-scalar property initializers require 86pinit() for run-time
    // initialization support.
    static const StringData* methName = StringData::GetStaticString("86pinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    pce->addMethod(fe);
    postponePinit(is, fe, nonScalarPinitVec);
  }

  if (nonScalarSinitVec != NULL) {
    // Non-scalar property initializers require 86sinit() for run-time
    // initialization support.
    static const StringData* methName = StringData::GetStaticString("86sinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    pce->addMethod(fe);
    postponeSinit(is, fe, nonScalarSinitVec);
  }

  if (nonScalarConstVec != NULL) {
    // Non-scalar constant initializers require 86cinit() for run-time
    // initialization support.
    static const StringData* methName = StringData::GetStaticString("86cinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    ASSERT(!(attr & VM::AttrTrait));
    bool added UNUSED = pce->addMethod(fe);
    ASSERT(added);
    postponeCinit(is, fe, nonScalarConstVec);
  }

  return hoistable;
}

void EmitterVisitor::emitBreakHandler(Emitter& e, Label& brkTarg,
    Label& cntTarg, Label& brkHand, Label& cntHand, Id iter /* = -1 */) {

  // Handle dynamic break
  if (brkHand.isUsed()) {
    brkHand.set(e);
    // Whatever happens, we have left this loop
    if (iter != -1) {
      e.IterFree(iter);
    }
    e.Int(1);
    e.Sub();
    e.Dup();
    e.Int(1);
    e.Lt();
    e.JmpZ(topBreakHandler());
    e.PopC(); // Pop break num
    e.Jmp(brkTarg);
  }

  // Handle dynamic continue
  if (cntHand.isUsed()) {
    cntHand.set(e);
    e.Int(1);
    e.Sub();
    e.Dup();
    e.Int(1);
    e.Lt();
    if (iter != -1) {
      // Freeing the iterator means another jump
      Label leaving;
      e.JmpZ(leaving);
      e.PopC();
      e.Jmp(cntTarg);
      leaving.set(e);
      // Leaving this loop
      e.IterFree(iter);
      e.Jmp(topContHandler());
    } else {
      e.JmpZ(topContHandler());
      e.PopC();
      e.Jmp(cntTarg);
    }
  }
}

class ForeachIterGuard {
  EmitterVisitor& m_ev;
 public:
  ForeachIterGuard(EmitterVisitor& ev, Id iterId) : m_ev(ev) {
    m_ev.pushIterId(iterId);
  }
  ~ForeachIterGuard() {
    m_ev.popIterId();
  }
};

void EmitterVisitor::emitForeach(Emitter& e, ForEachStatementPtr fe) {
  ExpressionPtr ae(static_pointer_cast<Expression>(
                      fe->getNthKid(ForEachStatement::ArrayExpr)));
  ExpressionPtr val(static_pointer_cast<Expression>(
                      fe->getNthKid(ForEachStatement::ValueExpr)));
  ExpressionPtr key(static_pointer_cast<Expression>(
                      fe->getNthKid(ForEachStatement::NameExpr)));
  StatementPtr body(static_pointer_cast<Statement>(
                      fe->getNthKid(ForEachStatement::BodyStmt)));
  int keyTempLocal;
  int valTempLocal;
  bool strong = fe->isStrong();
  Label exit;
  Label next;
  Label brkHand;
  Label cntHand;
  Label start;
  Offset bIterStart;
  Id itId = m_curFunc->allocIterator();
  ForeachIterGuard fig(*this, itId);
  bool simpleCase = (!key || key->is(Expression::KindOfSimpleVariable)) &&
                             val->is(Expression::KindOfSimpleVariable);

  if (simpleCase) {
    SimpleVariablePtr svVal(static_pointer_cast<SimpleVariable>(val));
    StringData* name = StringData::GetStaticString(svVal->getName());
    valTempLocal = m_curFunc->lookupVarId(name);
    if (key) {
      SimpleVariablePtr svKey(static_pointer_cast<SimpleVariable>(key));
      name = StringData::GetStaticString(svKey->getName());
      keyTempLocal = m_curFunc->lookupVarId(name);
      visit(key);
      // Meta info on the key local will confuse the translator (and
      // wouldn't be useful anyway)
      m_evalStack.cleanTopMeta();
    } else {
      // Make gcc happy
      keyTempLocal = -1;
    }
    visit(val);
    // Meta info on the value local will confuse the translator (and
    // wouldn't be useful anyway)
    m_evalStack.cleanTopMeta();
    visit(ae);
    if (strong) {
      emitConvertToVar(e);
      if (key) {
        e.IterInitMK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.IterInitM(itId, exit, valTempLocal);
      }
    } else {
      emitConvertToCell(e);
      if (key) {
        e.IterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.IterInit(itId, exit, valTempLocal);
      }
    }

    start.set(e);
    bIterStart = m_ue.bcPos();
  } else {
    keyTempLocal = key ? m_curFunc->allocUnnamedLocal() : -1;
    valTempLocal = m_curFunc->allocUnnamedLocal();
    if (key) {
      emitVirtualLocal(keyTempLocal);
    }
    emitVirtualLocal(valTempLocal);

    visit(ae);
    if (strong) {
      emitConvertToVar(e);
    } else {
      emitConvertToCell(e);
    }

    if (strong) {
      if (key) {
        e.IterInitMK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.IterInitM(itId, exit, valTempLocal);
      }
    } else {
      if (key) {
        e.IterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.IterInit(itId, exit, valTempLocal);
      }
    }

    // At this point, valTempLocal and keyTempLocal if applicable, contain the
    // key and value for the iterator.
    start.set(e);
    bIterStart = m_ue.bcPos();
    if (key) {
      visit(key);
    }
    visit(val);
    emitVirtualLocal(valTempLocal);
    if (strong) {
      emitVGet(e);
      emitBind(e);
    } else {
      emitCGet(e);
      emitSet(e);
    }
    emitPop(e);
    emitVirtualLocal(valTempLocal);
    emitUnset(e);
    newFaultRegion(bIterStart, m_ue.bcPos(),
                   new UnsetUnnamedLocalThunklet(valTempLocal));
    if (key) {
      ASSERT(keyTempLocal != -1);
      emitVirtualLocal(keyTempLocal);
      emitCGet(e);
      emitSet(e);
      emitPop(e);
      emitVirtualLocal(keyTempLocal);
      emitUnset(e);
      newFaultRegion(bIterStart, m_ue.bcPos(),
                     new UnsetUnnamedLocalThunklet(keyTempLocal));
    }
  }

  {
    FOREACH_BODY(itId, exit, next, brkHand, cntHand);
    if (body) visit(body);
  }
  bool needBreakHandler = (brkHand.isUsed() || cntHand.isUsed());
  if (next.isUsed() || needBreakHandler) {
    next.set(e);
  }
  if (key) {
    emitVirtualLocal(keyTempLocal);
    // Meta info on the key local will confuse the translator (and
    // wouldn't be useful anyway)
    m_evalStack.cleanTopMeta();
  }
  emitVirtualLocal(valTempLocal);
  // Meta info on the value local will confuse the translator (and
  // wouldn't be useful anyway)
  m_evalStack.cleanTopMeta();
  if (strong) {
    if (key) {
      e.IterNextMK(itId, start, valTempLocal, keyTempLocal);
    } else {
      e.IterNextM(itId, start, valTempLocal);
    }
  } else {
    if (key) {
      e.IterNextK(itId, start, valTempLocal, keyTempLocal);
    } else {
      e.IterNext(itId, start, valTempLocal);
    }
  }
  newFaultRegion(bIterStart, m_ue.bcPos(), new IterFreeThunklet(itId), itId);
  if (needBreakHandler) {
    e.Jmp(exit);
    emitBreakHandler(e, exit, next, brkHand, cntHand, itId);
  }
  if (!simpleCase) {
    m_curFunc->freeUnnamedLocal(valTempLocal);
    if (key) {
      m_curFunc->freeUnnamedLocal(keyTempLocal);
    }
  }
  exit.set(e);
  m_curFunc->freeIterator(itId);
}

/**
 * Emits bytecode that restores the previous error reporting level after
 * evaluating a silenced (@) expression, or in the fault funclet protecting such
 * an expression.  Requires a local variable id containing the previous error
 * reporting level.  The whole silenced expression looks like this:
 *   oldvalue = error_reporting(0)
 *   ...evaluate silenced expression...
 *   oldvalue = error_reporting(oldvalue)
 *   if oldvalue != 0:
 *     error_reporting(oldvalue)
 */
void EmitterVisitor::emitRestoreErrorReporting(Emitter& e, Id oldLevelLoc) {
  static const StringData* funcName =
    StringData::GetStaticString("error_reporting");
  Label dontRollback;
  // Optimistically call with the old value.  If this returns nonzero, call it
  // again with that return value.
  emitVirtualLocal(oldLevelLoc);
  Offset fpiStart = m_ue.bcPos();
  e.FPushFuncD(1, funcName);
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    emitVirtualLocal(oldLevelLoc);
    emitCGet(e);
    m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
    e.FPassC(0);
  }
  e.FCall(1);
  e.UnboxR();
  // stack is now: ...[Loc oldLevelLoc][return value]
  // save the return value in local, and leave it on the stack
  emitSet(e);
  e.Int(0);
  e.Eq();
  e.JmpNZ(dontRollback);
  fpiStart = m_ue.bcPos();
  e.FPushFuncD(1, funcName);
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    emitVirtualLocal(oldLevelLoc);
    emitCGet(e);
    m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::NopOut, false, 0, 0);
    e.FPassC(0);
  }
  e.FCall(1);
  e.PopR();
  dontRollback.set(e);
}

void EmitterVisitor::emitMakeUnitFatal(Emitter& e, const std::string& msg) {
  StringData* sd = StringData::GetStaticString(msg);
  e.String(sd);
  e.Fatal();
}

void EmitterVisitor::addFunclet(Thunklet* body, Label* entry) {
  m_funclets.push_back(Funclet(body, entry));
}
void EmitterVisitor::emitFunclets(Emitter& e) {
  while (!m_funclets.empty()) {
    Funclet& f = m_funclets.front();
    f.m_entry->set(e);
    f.m_body->emit(e);
    delete f.m_body;
    m_funclets.pop_front();
  }
  m_funclets.clear();
}

void EmitterVisitor::newFaultRegion(Offset start, Offset end, Thunklet* t,
                                    Id iterId) {
  FaultRegion* r = new FaultRegion(start, end, iterId);
  m_faultRegions.push_back(r);
  addFunclet(t, &r->m_func);
}

void EmitterVisitor::newFPIRegion(Offset start, Offset end, Offset fpOff) {
  FPIRegion* r = new FPIRegion(start, end, fpOff);
  m_fpiRegions.push_back(r);
}

void EmitterVisitor::copyOverExnHandlers(FuncEmitter* fe) {
  for (std::deque<ExnHandlerRegion*>::const_iterator it = m_exnHandlers.begin();
       it != m_exnHandlers.end(); ++it) {
    EHEnt& e = fe->addEHEnt();
    e.m_ehtype = EHEnt::EHType_Catch;
    e.m_base = (*it)->m_start;
    e.m_past = (*it)->m_end;
    e.m_iterId = -1;
    for (std::vector<std::pair<StringData*, Label*> >::const_iterator it2
           = (*it)->m_catchLabels.begin();
         it2 != (*it)->m_catchLabels.end(); ++it2) {
      Id id = m_ue.mergeLitstr(it2->first);
      Offset off = it2->second->getAbsoluteOffset();
      e.m_catches.push_back(std::pair<Id, Offset>(id, off));
    }
    delete *it;
  }
  m_exnHandlers.clear();
  for (std::deque<FaultRegion*>::iterator it = m_faultRegions.begin();
      it != m_faultRegions.end(); ++it) {
    EHEnt& e = fe->addEHEnt();
    e.m_ehtype = EHEnt::EHType_Fault;
    e.m_base = (*it)->m_start;
    e.m_past = (*it)->m_end;
    e.m_iterId = (*it)->m_iterId;
    e.m_fault = (*it)->m_func.getAbsoluteOffset();
    delete *it;
  }
  m_faultRegions.clear();
}

void EmitterVisitor::copyOverFPIRegions(FuncEmitter* fe) {
  for (std::deque<FPIRegion*>::iterator it = m_fpiRegions.begin();
       it != m_fpiRegions.end(); ++it) {
    FPIEnt& e = fe->addFPIEnt();
    e.m_fpushOff = (*it)->m_start;
    e.m_fcallOff = (*it)->m_end;
    e.m_fpOff = (*it)->m_fpOff;
    delete *it;
  }
  m_fpiRegions.clear();
}

void EmitterVisitor::saveMaxStackCells(FuncEmitter* fe) {
  fe->setMaxStackCells(m_actualStackHighWater + kNumActRecCells
                       + m_fdescHighWater);
  m_actualStackHighWater = 0;
  m_fdescHighWater = 0;
}

// Are you sure you mean to be calling this directly? Would FuncFinisher
// be more appropriate?
void EmitterVisitor::finishFunc(Emitter& e, FuncEmitter* fe) {
  emitFunclets(e);
  saveMaxStackCells(fe);
  copyOverExnHandlers(fe);
  copyOverFPIRegions(fe);
  m_gotoLabels.clear();
  Offset past = e.getUnitEmitter().bcPos();
  fe->finish(past, false);
  e.getUnitEmitter().recordFunction(fe);
}

StringData* EmitterVisitor::newClosureName() {
  std::ostringstream str;
  str << "closure$";
  if (m_curFunc->pce() != NULL) {
    str << m_curFunc->pce()->name()->data();
  }
  str << '$';
  if (m_curFunc->isPseudoMain()) {
    str << "__pseudoMain";
  } else {
    str << m_curFunc->name()->data();
  }
  /*
   * Uniquify the name
   */
  str << '$'
      << std::hex
      << m_curFunc->ue().md5().q[1] << m_curFunc->ue().md5().q[0]
      << std::dec
      << '$' << m_closureCounter++;

  return StringData::GetStaticString(str.str());
}

void EmitterVisitor::initScalar(TypedValue& tvVal, ExpressionPtr val) {
  ASSERT(val->isScalar());
  tvVal.m_type = KindOfUninit;
  tvVal._count = 0;
  switch (val->getKindOf()) {
    case Expression::KindOfConstantExpression: {
      ConstantExpressionPtr ce(static_pointer_cast<ConstantExpression>(val));
      if (ce->isNull()) {
        tvVal.m_data.num = 0;
        tvVal.m_type = KindOfNull;
      } else if (ce->isBoolean()) {
        tvVal.m_data.num = ce->getBooleanValue() ? 1 : 0;
        tvVal.m_type = KindOfBoolean;
      } else if (ce->isScalar()) {
        ce->getScalarValue(tvAsVariant(&tvVal));
      } else {
        not_implemented();
      }
      break;
    }
    case Expression::KindOfScalarExpression: {
      ScalarExpressionPtr sval = static_pointer_cast<ScalarExpression>(val);
      const std::string* s;
      if (sval->getString(s)) {
        StringData* sd = StringData::GetStaticString(*s);
        tvVal.m_data.pstr = sd;
        tvVal.m_type = KindOfString;
        break;
      }
      int64 i;
      if (sval->getInt(i)) {
        tvVal.m_data.num = i;
        tvVal.m_type = KindOfInt64;
        break;
      }
      double d;
      if (sval->getDouble(d)) {
        tvVal.m_data.dbl = d;
        tvVal.m_type = KindOfDouble;
        break;
      }
      ASSERT(false);
      break;
    }
    case Expression::KindOfUnaryOpExpression: {
      UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(val));
      if (u->getOp() == T_ARRAY) {
        HphpArray* a = NEW(HphpArray)(0);
        a->incRefCount();
        m_staticArrays.push_back(a);

        visit(u->getExpression());

        HphpArray* va = m_staticArrays.back();
        m_staticArrays.pop_back();
        ASSERT(IsHphpArray(ArrayData::GetScalarArray(va)));
        va = static_cast<HphpArray*>(ArrayData::GetScalarArray(va));

        tvVal.m_data.parr = va;
        tvVal.m_type = KindOfArray;

        decRefArr(a);
        break;
      }
      // Fall through
    }
    default: {
      if (val->getScalarValue(tvAsVariant(&tvVal))) {
        if (tvAsVariant(&tvVal).isArray()) {
          not_implemented();
        }
        break;
      }
      not_reached();
    }
  }
}

Thunklet::~Thunklet() {}

using HPHP::Eval::PhpFile;

static ConstructPtr doOptimize(ConstructPtr c, AnalysisResultConstPtr ar) {
  for (int i = 0, n = c->getKidCount(); i < n; i++) {
    if (ConstructPtr k = c->getNthKid(i)) {
      if (ConstructPtr rep = doOptimize(k, ar)) {
        c->setNthKid(i, rep);
      }
    }
  }
  if (ExpressionPtr e = dynamic_pointer_cast<Expression>(c)) {
    switch (e->getKindOf()) {
      case Expression::KindOfBinaryOpExpression:
      case Expression::KindOfUnaryOpExpression:
      case Expression::KindOfIncludeExpression:
      case Expression::KindOfSimpleFunctionCall:
        return e->preOptimize(ar);

      default: break;
    }
  }
  return ConstructPtr();
}

static Unit* emitHHBCUnit(AnalysisResultPtr ar, FileScopePtr fsp,
                          const MD5& md5, UnitOrigin unitOrigin, bool commit) {
  if (fsp->getPseudoMain() && !Option::WholeProgram) {
    ar->setPhase(AnalysisResult::FirstPreOptimize);
    doOptimize(fsp->getPseudoMain()->getStmt(), ar);
  }

  if (RuntimeOption::EvalDumpAst) {
    if (fsp->getPseudoMain()) {
      fsp->getPseudoMain()->getStmt()->dump(0, ar);
    }
  }

  MethodStatementPtr msp(dynamic_pointer_cast<MethodStatement>(
                         fsp->getPseudoMain()->getStmt()));
  UnitEmitter* ue = new UnitEmitter(md5);
  const Location* sLoc = msp->getLocation().get();
  ue->initMain(sLoc->line0, sLoc->line1);
  EmitterVisitor ev(*ue);
  try {
    ev.visit(fsp);
  } catch (EmitterVisitor::IncludeTimeFatalException& ex) {
    // Replace the unit with an empty one, but preserve its file path.
    UnitEmitter* nue = new UnitEmitter(md5);
    nue->initMain(sLoc->line0, sLoc->line1);
    nue->setFilepath(ue->getFilepath());
    delete ue;
    ue = nue;

    EmitterVisitor fev(*ue);
    Emitter emitter(ex.m_node, *ue, fev);
    FuncFinisher ff(&fev, emitter, ue->getMain());
    fev.emitMakeUnitFatal(emitter, ex.getMessage());
  }

  if (commit) {
    HPHP::VM::Repo::get().commitUnit(ue, unitOrigin);
  }
  Unit* unit = ue->create();
  delete ue;
  return unit;
}

struct Entry {
  StringData* name;
  const HhbcExtClassInfo* info;
  const ClassInfo* ci;
};

static Unit* emitHHBCNativeFuncUnit(const HhbcExtFuncInfo* builtinFuncs,
                                    ssize_t numBuiltinFuncs) {
  MD5 md5("11111111111111111111111111111111");
  UnitEmitter* ue = new UnitEmitter(md5);
  ue->setFilepath(StringData::GetStaticString(""));
  ue->initMain(0, 0);
  FuncEmitter* mfe = ue->getMain();
  ue->emitOp(OpInt);
  ue->emitInt64(1);
  ue->emitOp(OpRetC);
  mfe->setMaxStackCells(1);
  mfe->finish(ue->bcPos(), false);
  ue->recordFunction(mfe);

  /*
    Special function used by FPushCuf* when its argument
    is not callable.
  */
  StringData* name = StringData::GetStaticString("86null");
  FuncEmitter* fe = ue->newFuncEmitter(name, /*top*/ true);
  /*
    Dont mark it AttrPersistent, because it would be
    deleted, and we need to be able to find it in the
    unit's m_mergeInfo
  */
  fe->init(0, 0, ue->bcPos(), AttrUnique,
           true, empty_string.get());
  ue->emitOp(OpNull);
  ue->emitOp(OpRetC);
  fe->setMaxStackCells(1);
  fe->finish(ue->bcPos(), false);
  ue->recordFunction(fe);

  for (ssize_t i = 0LL; i < numBuiltinFuncs; ++i) {
    const HhbcExtFuncInfo* info = &builtinFuncs[i];
    StringData* name = StringData::GetStaticString(info->m_name);
    BuiltinFunction bif = (BuiltinFunction)info->m_builtinFunc;
    BuiltinFunction nif = (BuiltinFunction)info->m_nativeFunc;
    const ClassInfo::MethodInfo* mi = ClassInfo::FindFunction(name);
    ASSERT(mi &&
      "MethodInfo not found; probably need to rebuild src/system");
    FuncEmitter* fe = ue->newFuncEmitter(name, /*top*/ true);
    Offset base = ue->bcPos();
    fe->setBuiltinFunc(mi, bif, nif, base);
    ue->emitOp(OpNativeImpl);
    fe->setMaxStackCells(kNumActRecCells + 1);
    fe->setAttrs(fe->attrs() | AttrUnique | AttrPersistent);
    fe->finish(ue->bcPos(), false);
    ue->recordFunction(fe);
  }

  Unit* unit = ue->create();
  delete ue;
  return unit;
}

enum ContinuationMethod {
  METH_NEXT,
  METH_SEND,
  METH_RAISE,
  METH_VALID,
  METH_CURRENT,
};
typedef hphp_hash_map<const StringData*, ContinuationMethod,
                      string_data_hash, string_data_same> ContMethMap;

static void emitContinuationMethod(UnitEmitter& ue, FuncEmitter* fe,
                                   ContinuationMethod m,
                                   MetaInfoBuilder& metaInfo) {
  static const StringData* valStr = StringData::GetStaticString("value");
  static const StringData* exnStr = StringData::GetStaticString("exception");

  Attr attrs = (Attr)(AttrPublic | AttrMayUseVV);
  fe->init(0, 0, ue.bcPos(), attrs, false, empty_string.get());
  switch (m) {
    case METH_SEND:
    case METH_RAISE:
      fe->appendParam(valStr, FuncEmitter::ParamInfo());
    case METH_NEXT: {
      // We always want these methods to be cloned with new funcids in
      // subclasses so we can burn Class*s and Func*s into the
      // translations
      fe->setAttrs(Attr(fe->attrs() | AttrClone));

      static Op mOps[] = {
        OpContNext,
        OpContSend,
        OpContRaise,
      };
      ue.emitOp(mOps[m]);
      const Offset ehStart = ue.bcPos();
      ue.emitOp(OpContEnter);
      ue.emitOp(OpContStopped);
      ue.emitOp(OpNull);
      ue.emitOp(OpRetC);

      EHEnt& eh = fe->addEHEnt();
      eh.m_ehtype = EHEnt::EHType_Catch;
      eh.m_base = ehStart;
      eh.m_past = ue.bcPos();
      eh.m_catches.push_back(
        std::pair<Id, Offset>(ue.mergeLitstr(exnStr), ue.bcPos()));
      ue.emitOp(OpCatch);
      ue.emitOp(OpContHandle);
      break;
    }
    case METH_VALID: {
      ue.emitOp(OpContValid);
      ue.emitOp(OpRetC);
      break;
    }
    case METH_CURRENT: {
      ue.emitOp(OpContCurrent);
      ue.emitOp(OpRetC);
      break;
    }

    default:
      not_reached();
  }
}

static Unit* emitHHBCNativeClassUnit(const HhbcExtClassInfo* builtinClasses,
                                     ssize_t numBuiltinClasses) {
  MD5 md5("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
  UnitEmitter* ue = new UnitEmitter(md5);
  ue->setFilepath(StringData::GetStaticString(""));
  ue->initMain(0, 0);
  FuncEmitter* mfe = ue->getMain();
  ue->emitOp(OpInt);
  ue->emitInt64(1);
  ue->emitOp(OpRetC);
  Offset past = ue->bcPos();
  mfe->setMaxStackCells(1);
  mfe->finish(past, false);
  ue->recordFunction(mfe);

  TypedValue mainReturn;
  mainReturn.m_data.num = 1;
  mainReturn.m_type = KindOfBoolean;
  // _count is the "Unit::isMergeOnly()" flag
  mainReturn._count = 1;
  ue->setMainReturn(&mainReturn);

  MetaInfoBuilder metaInfo;

  ContMethMap contMethods;
  contMethods[StringData::GetStaticString("next")] = METH_NEXT;
  contMethods[StringData::GetStaticString("send")] = METH_SEND;
  contMethods[StringData::GetStaticString("raise")] = METH_RAISE;
  contMethods[StringData::GetStaticString("valid")] = METH_VALID;
  contMethods[StringData::GetStaticString("current")] = METH_CURRENT;

  // Build up extClassHash, a hashtable that maps class names to structures
  // containing C++ function pointers for the class's methods and constructors
  ASSERT(Class::s_extClassHash.size() == 0);
  for (long long i = 0LL; i < numBuiltinClasses; ++i) {
    const HhbcExtClassInfo* info = builtinClasses + i;
    StringData *s = StringData::GetStaticString(info->m_name);
    Class::s_extClassHash[s] = info;
  }
  // If a given class has a base class, then we can't load that class
  // before we load the base class. Build up some structures so that
  // we can load the C++ builtin classes in the right order.
  std::vector<Entry> classEntries;
  {
    typedef hphp_hash_map<StringData*, std::vector<Entry>,
                          string_data_hash, string_data_isame> PendingMap;
    PendingMap pending;
    hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
                  string_data_hash, string_data_isame>::iterator it;
    for (it = Class::s_extClassHash.begin();
         it != Class::s_extClassHash.end(); ++it) {
      Entry e;
      e.name = const_cast<StringData*>(it->first);
      e.info = it->second;
      e.ci = ClassInfo::FindClass(e.name);
      ASSERT(e.ci);
      StringData* parentName
        = StringData::GetStaticString(e.ci->getParentClass().get());
      if (parentName->empty()) {
        // If this class doesn't have a base class, it's classEntries to be
        // loaded now
        classEntries.push_back(e);
      } else {
        // If this class has a base class, we can't load it until its
        // base class has been loaded
        pending[parentName].push_back(e);
      }
    }
    for (unsigned k = 0; k < classEntries.size(); ++k) {
      Entry& e = classEntries[k];
      // Any classes that derive from this class are now classEntries to be
      // loaded
      PendingMap::iterator pendingIt = pending.find(e.name);
      if (pendingIt != pending.end()) {
        for (unsigned i = 0; i < pendingIt->second.size(); ++i) {
          classEntries.push_back(pendingIt->second[i]);
        }
        pending.erase(pendingIt);
      }
    }
    ASSERT(pending.empty());
  }

  for (unsigned int i = 0LL; i < classEntries.size(); ++i) {
    Entry& e = classEntries[i];
    StringData* parentName =
      StringData::GetStaticString(e.ci->getParentClass().get());
    PreClassEmitter* pce = ue->newPreClassEmitter(e.name,
                                                  PreClass::AlwaysHoistable);
    pce->init(0, 0, ue->bcPos(), AttrUnique | AttrPersistent, parentName, NULL);
    pce->setBuiltinClassInfo(e.ci, e.info->m_InstanceCtor, e.info->m_sizeof);
    {
      ClassInfo::InterfaceVec intfVec = e.ci->getInterfacesVec();
      for (unsigned i = 0; i < intfVec.size(); ++i) {
        const StringData* intf = StringData::GetStaticString(intfVec[i].get());
        pce->addInterface(intf);
      }
    }
    for (ssize_t j = 0; j < e.info->m_methodCount; ++j) {
      const HhbcExtMethodInfo* methodInfo = &(e.info->m_methods[j]);
      static const StringData* continuationCls =
        StringData::GetStaticString("continuation");
      StringData* methName =
        StringData::GetStaticString(methodInfo->m_name);
      ContinuationMethod cmeth;

      FuncEmitter* fe = ue->newMethodEmitter(methName, pce);
      pce->addMethod(fe);
      if (e.name->isame(continuationCls) &&
          mapGet(contMethods, methName, &cmeth)) {
        emitContinuationMethod(*ue, fe, cmeth, metaInfo);
      } else {
        // Build the function
        BuiltinFunction bcf =
          (BuiltinFunction)methodInfo->m_pGenericMethod;
        const ClassInfo::MethodInfo* mi =
          e.ci->getMethodInfo(std::string(methodInfo->m_name));
        Offset base = ue->bcPos();
        fe->setBuiltinFunc(mi, bcf, NULL, base);
        ue->emitOp(OpNativeImpl);
      }
      Offset past = ue->bcPos();
      fe->setMaxStackCells(kNumActRecCells + 1);
      fe->finish(past, false);
      ue->recordFunction(fe);
    }
    {
      ClassInfo::ConstantVec cnsVec = e.ci->getConstantsVec();
      for (unsigned i = 0; i < cnsVec.size(); ++i) {
        const ClassInfo::ConstantInfo* cnsInfo = cnsVec[i];
        ASSERT(cnsInfo);
        Variant val;
        try {
          val = cnsInfo->getValue();
        } catch (Exception& e) {
          ASSERT(false);
        }
        pce->addConstant(
          cnsInfo->name.get(), (TypedValue*)(&val), empty_string.get());
      }
    }
  }

  Peephole peephole(*ue, metaInfo);
  metaInfo.setForUnit(*ue);

  Unit* unit = ue->create();
  delete ue;
  return unit;
}

static void emitHHBCVisitor(AnalysisResultPtr ar, FileScopeRawPtr fsp) {
  MD5 md5 = fsp->getMd5();

  HPHP::VM::Unit* unit = emitHHBCUnit(ar, fsp, md5, UnitOriginFile,
                                      Option::GenerateBinaryHHBC);
  if (unit == NULL) {
    return;
  }

  if (Option::GenerateTextHHBC) {
    std::string fullPath = AnalysisResult::prepareFile(
      ar->getOutputPath().c_str(), Option::UserFilePrefix + fsp->getName(),
      true, false) + ".hhbc.txt";

    std::ofstream f(fullPath.c_str());
    if (!f) {
      Logger::Error("Unable to open %s for write", fullPath.c_str());
      delete unit;
      return;
    }

    CodeGenerator cg(&f, CodeGenerator::TextHHBC);
    cg.printf("Hash: %llx%016llx\n", md5.q[0], md5.q[1]);
    cg.printRaw(unit->toString().c_str());
    f.close();
  }

  delete unit;
}

class EmitterWorker :
    public JobQueueWorker<FileScopeRawPtr, true, true> {
public:
  EmitterWorker() : m_ret(true) {}
  virtual void doJob(JobType job) {
    try {
      AnalysisResultPtr ar = ((AnalysisResult*)m_opaque)->shared_from_this();
      emitHHBCVisitor(ar, job);
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
      m_ret = false;
    } catch (...) {
      Logger::Error("Fatal: An unexpected exception was thrown");
      m_ret = false;
    }
  }
  bool m_ret;
};

static void addEmitterWorker(AnalysisResultPtr ar, StatementPtr sp,
                             void *data) {
  ((JobQueueDispatcher<EmitterWorker::JobType,
    EmitterWorker>*)data)->enqueue(sp->getFileScope());
}

/**
 * This is the entry point for offline bytecode generation.
 */
void emitAllHHBC(AnalysisResultPtr ar) {
  unsigned int threadCount = Option::ParserThreadCount;
  unsigned int nFiles = ar->getAllFilesVector().size();
  if (threadCount > nFiles) {
    threadCount = nFiles;
  }
  if (!threadCount) threadCount = 1;

  /* there is a race condition in the first call to
     GetStaticString. Make sure we dont hit it */
  StringData::GetStaticString("");
  /* same for TypeConstraint */
  TypeConstraint tc;

  JobQueueDispatcher<EmitterWorker::JobType, EmitterWorker>
    dispatcher(threadCount, true, 0, false, ar.get());

  dispatcher.start();
  ar->visitFiles(addEmitterWorker, &dispatcher);
  dispatcher.waitEmpty();
}


/**
 * This is the entry point from the runtime; i.e. online bytecode generation.
 * The 'filename' parameter may be NULL if there is no file associated with
 * the source code.
 *
 * Before being actually used, hphp_compiler_parse must be called with
 * a NULL `code' parameter to do initialization.
 */

extern "C" {

Unit* hphp_compiler_parse(const char* code, int codeLen, const MD5& md5,
                          const char* filename) {
  if (UNLIKELY(!code)) {
    // Do initialization when code is null; see above.
    Option::EnableHipHopSyntax = RuntimeOption::EnableHipHopSyntax;
    Option::JitEnableRenameFunction =
      RuntimeOption::EvalJitEnableRenameFunction;
    for (auto& i : RuntimeOption::DynamicInvokeFunctions) {
      Option::DynamicInvokeFunctions.insert(i);
    }
    Option::OutputHHBC = true;
    Option::ParseTimeOpts = false;
    Option::WholeProgram = false;
    Type::InitTypeHintMap();
    BuiltinSymbols::LoadSuperGlobals();
    AnalysisResultPtr ar(new AnalysisResult());
    BuiltinSymbols::Load(ar, true);
    BuiltinSymbols::NoSuperGlobals = false;
    TypeConstraint tc;
    return NULL;
  }

  try {
    UnitOrigin unitOrigin = UnitOriginFile;
    if (!filename) {
      filename = "";
      unitOrigin = UnitOriginEval;
    }
    ScopeGuard sg(SymbolTable::Purge);

    // Check if this file contains raw hip hop bytecode instead of php.
    // For now this is just dictated by file extension, and doesn't ever
    // commit to the repo.
    if (RuntimeOption::EvalAllowHhas) {
      if (const char* dot = strrchr(filename, '.')) {
        const char hhbc_ext[] = "hhas";
        if (!strcmp(dot + 1, hhbc_ext)) {
          return VM::assemble_file(filename, md5);
        }
      }
    }

    AnalysisResultPtr ar(new AnalysisResult());
    Scanner scanner(code, codeLen, RuntimeOption::ScannerType, filename);
    Parser parser(scanner, filename, ar, codeLen);
    parser.parse();
    FileScopePtr fsp = parser.getFileScope();
    fsp->setOuterScope(ar);

    ar->loadBuiltins();
    ar->setPhase(AnalysisResult::AnalyzeAll);
    fsp->analyzeProgram(ar);

    return emitHHBCUnit(ar, fsp, md5, unitOrigin, RuntimeOption::RepoCommit);
  } catch (const std::exception&) {
    // extern "C" function should not be throwing exceptions...
    return NULL;
  }
}

Unit* hphp_build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                                  ssize_t numBuiltinFuncs) {
  return emitHHBCNativeFuncUnit(builtinFuncs, numBuiltinFuncs);
}

Unit* hphp_build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                                   ssize_t numBuiltinClasses) {
  return emitHHBCNativeClassUnit(builtinClasses, numBuiltinClasses);
}

} // extern "C"

///////////////////////////////////////////////////////////////////////////////
}
}
