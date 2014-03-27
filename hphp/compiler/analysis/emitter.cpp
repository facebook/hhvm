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
#include "hphp/compiler/analysis/emitter.h"

#include <memory>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <memory>
#include <deque>
#include <exception>
#include <set>
#include <utility>

#include "folly/MapUtil.h"
#include "folly/Memory.h"
#include "folly/ScopeGuard.h"

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/peephole.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/expression/encaps_list_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/expression/list_assignment.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/new_object_expression.h"
#include "hphp/compiler/expression/object_method_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/yield_expression.h"
#include "hphp/compiler/expression/await_expression.h"
#include "hphp/compiler/expression/query_expression.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/statement/break_statement.h"
#include "hphp/compiler/statement/case_statement.h"
#include "hphp/compiler/statement/catch_statement.h"
#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/do_statement.h"
#include "hphp/compiler/statement/echo_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/for_statement.h"
#include "hphp/compiler/statement/foreach_statement.h"
#include "hphp/compiler/statement/finally_statement.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/global_statement.h"
#include "hphp/compiler/statement/goto_statement.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/if_statement.h"
#include "hphp/compiler/statement/label_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/static_statement.h"
#include "hphp/compiler/statement/switch_statement.h"
#include "hphp/compiler/statement/try_statement.h"
#include "hphp/compiler/statement/unset_statement.h"
#include "hphp/compiler/statement/while_statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/trait_require_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/typedef_statement.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/hhbbc/hhbbc.h"

#include "hphp/util/safe-cast.h"
#include "hphp/util/logger.h"
#include "hphp/util/job-queue.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/ext_hhvm/ext_hhvm.h"
#include "hphp/runtime/vm/preclass-emit.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(emitter)

using uchar = unsigned char;

namespace {
  const StringData* s_continuationVarArgsLocal
    = makeStaticString("0ContinuationVarArgsLocal");
}

namespace StackSym {
  static const char None = 0x00;

  /*
   * We don't actually track the U flavor (we treat it as a C),
   * because there's nothing important to do with it for emission.
   * The verifier will check they are only created at the appropriate
   * times.
   */
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
  assert(false);                                            \
} while (0)

// RAII guard for function creation.
class FuncFinisher {
  EmitterVisitor* m_ev;
  Emitter&        m_e;
  FuncEmitter*    m_fe;

 public:
  FuncFinisher(EmitterVisitor* ev, Emitter& e, FuncEmitter* fe)
    : m_ev(ev), m_e(e), m_fe(fe) {
      TRACE(2, "FuncFinisher constructed: %s\n", m_fe->name()->data());
    }

  ~FuncFinisher() {
    TRACE(2, "Finishing func: %s\n", m_fe->name()->data());
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
  assert(!immVec.empty());

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
  assert(vec - &immVec[0] == int(immVec.size()));
  return count;
}

#define O(name, imm, pop, push, flags)                                  \
  void Emitter::name(imm) {                                             \
    auto const opcode = Op::name;                                       \
    Offset curPos UNUSED = getUnitEmitter().bcPos();                    \
    getEmitterVisitor().prepareEvalStack();                             \
    POP_##pop;                                                          \
    const int nIn UNUSED = COUNT_##pop;                                 \
    POP_LA_##imm;                                                       \
    PUSH_##push;                                                        \
    getUnitEmitter().emitOp(Op##name);                                  \
    IMPL_##imm;                                                         \
    getUnitEmitter().recordSourceLocation(m_tempLoc ? m_tempLoc.get() : \
                                          m_node->getLocation().get(), curPos); \
    if (flags & TF) getEmitterVisitor().restoreJumpTargetEvalStack();   \
    if (opcode == Op::FCall) getEmitterVisitor().recordCall();          \
    getEmitterVisitor().setPrevOpcode(opcode);                          \
  }

#define COUNT_NOV 0
#define COUNT_ONE(t) 1
#define COUNT_TWO(t1,t2) 2
#define COUNT_THREE(t1,t2,t3) 3
#define COUNT_FOUR(t1,t2,t3,t4) 4
#define COUNT_MMANY 0
#define COUNT_C_MMANY 0
#define COUNT_R_MMANY 0
#define COUNT_V_MMANY 0
#define COUNT_FMANY 0
#define COUNT_CVMANY 0
#define COUNT_CVUMANY 0
#define COUNT_CMANY 0
#define COUNT_SMANY 0

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
#define DEC_ILA std::vector<IterPair>&
#define DEC_IVA int32_t
#define DEC_LA int32_t
#define DEC_IA int32_t
#define DEC_I64A int64_t
#define DEC_DA double
#define DEC_SA const StringData*
#define DEC_AA ArrayData*
#define DEC_BA Label&
#define DEC_OA(type) type
#define DEC_VSA std::vector<std::string>&

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
#define POP_MMANY \
  getEmitterVisitor().popEvalStackMMany()
#define POP_C_MMANY \
  getEmitterVisitor().popEvalStack(StackSym::C); \
  getEmitterVisitor().popEvalStackMMany()
#define POP_V_MMANY \
  getEmitterVisitor().popEvalStack(StackSym::V); \
  getEmitterVisitor().popEvalStackMMany()
#define POP_R_MMANY \
  getEmitterVisitor().popEvalStack(StackSym::R); \
  getEmitterVisitor().popEvalStackMMany()
#define POP_FMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::F)
#define POP_CVMANY \
  getEmitterVisitor().popEvalStackCVMany(a1)
#define POP_CVUMANY POP_CVMANY
#define POP_CMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::C)
#define POP_SMANY \
  getEmitterVisitor().popEvalStackMany(a1.size(), StackSym::C)

#define POP_CV(i) getEmitterVisitor().popEvalStack(StackSym::C, i, curPos)
#define POP_VV(i) getEmitterVisitor().popEvalStack(StackSym::V, i, curPos)
#define POP_AV(i) getEmitterVisitor().popEvalStack(StackSym::A, i, curPos)
#define POP_RV(i) getEmitterVisitor().popEvalStack(StackSym::R, i, curPos)
#define POP_FV(i) getEmitterVisitor().popEvalStack(StackSym::F, i, curPos)

// Pop of virtual "locs" on the stack that turn into immediates.
#define POP_LA_ONE(t) \
  POP_LA_##t(nIn)
#define POP_LA_TWO(t1, t2) \
  POP_LA_##t1(nIn);    \
  POP_LA_##t2(nIn)
#define POP_LA_THREE(t1, t2, t3) \
  POP_LA_##t1(nIn);          \
  POP_LA_##t2(nIn);          \
  POP_LA_##t3(nIn)
#define POP_LA_FOUR(t1, t2, t3, t4) \
  POP_LA_##t1(nIn);             \
  POP_LA_##t2(nIn);             \
  POP_LA_##t3(nIn);             \
  POP_LA_##t4(nIn)

#define POP_LA_NA
#define POP_LA_MA(i)
#define POP_LA_BLA(i)
#define POP_LA_SLA(i)
#define POP_LA_ILA(i)
#define POP_LA_IVA(i)
#define POP_LA_IA(i)
#define POP_LA_I64A(i)
#define POP_LA_DA(i)
#define POP_LA_SA(i)
#define POP_LA_AA(i)
#define POP_LA_BA(i)
#define POP_LA_IMPL(x)
#define POP_LA_OA(i) POP_LA_IMPL
#define POP_LA_VSA(i)

#define POP_LA_LA(i) \
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
#define PUSH_UV PUSH_CV
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

#define IMPL_ILA(var) do {     \
  auto& ue = getUnitEmitter(); \
  ue.emitInt32(var.size());    \
  for (auto& i : var) {        \
    ue.emitInt32(i.kind);      \
    ue.emitInt32(i.id);        \
  }                            \
} while(0)
#define IMPL1_ILA IMPL_ILA(a1)
#define IMPL2_ILA IMPL_ILA(a2)
#define IMPL3_ILA IMPL_ILA(a3)
#define IMPL4_ILA IMPL_ILA(a4)

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

#define IMPL_VSA(var) do {                          \
  auto n = var.size();                              \
  getUnitEmitter().emitInt32(n);                    \
  for (size_t i = 0; i < n; ++i) {                  \
    IMPL_SA((HPHP::String(var[i])).get());          \
  }                                                 \
} while (0)
#define IMPL1_VSA IMPL_VSA(a1)
#define IMPL2_VSA IMPL_VSA(a2)
#define IMPL3_VSA IMPL_VSA(a3)
#define IMPL4_VSA IMPL_VSA(a4)

#define IMPL_IVA(var) do { \
  getUnitEmitter().emitIVA(var); \
} while (0)
#define IMPL1_IVA IMPL_IVA(a1)
#define IMPL2_IVA IMPL_IVA(a2)
#define IMPL3_IVA IMPL_IVA(a3)
#define IMPL4_IVA IMPL_IVA(a4)

#define IMPL1_LA IMPL_IVA(a1)
#define IMPL2_LA IMPL_IVA(a2)
#define IMPL3_LA IMPL_IVA(a3)
#define IMPL4_LA IMPL_IVA(a4)

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

#define IMPL_OA(var) getUnitEmitter().emitByte(static_cast<uint8_t>(var))
#define IMPL1_OA(type) IMPL_OA(a1)
#define IMPL2_OA(type) IMPL_OA(a2)
#define IMPL3_OA(type) IMPL_OA(a3)
#define IMPL4_OA(type) IMPL_OA(a4)
 OPCODES
#undef O
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef NA
#undef DEC_MA
#undef DEC_IVA
#undef DEC_LA
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
#undef POP_MMANY
#undef POP_C_MMANY
#undef POP_V_MMANY
#undef POP_R_MMANY
#undef POP_CV
#undef POP_VV
#undef POP_HV
#undef POP_AV
#undef POP_RV
#undef POP_FV
#undef POP_LREST
#undef POP_FMANY
#undef POP_CVMANY
#undef POP_CVUMANY
#undef POP_CMANY
#undef POP_SMANY
#undef POP_LA_ONE
#undef POP_LA_TWO
#undef POP_LA_THREE
#undef POP_LA_FOUR
#undef POP_LA_NA
#undef POP_LA_MA
#undef POP_LA_IVA
#undef POP_LA_IA
#undef POP_LA_I64A
#undef POP_LA_DA
#undef POP_LA_SA
#undef POP_LA_AA
#undef POP_LA_BA
#undef POP_LA_IMPL
#undef POP_LA_OA
#undef POP_LA_LA
#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_THREE
#undef PUSH_FOUR
#undef PUSH_CV
#undef PUSH_UV
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
#undef IMPL_ILA
#undef IMPL1_ILA
#undef IMPL2_ILA
#undef IMPL3_ILA
#undef IMPL4_ILA
#undef IMPL_IVA
#undef IMPL1_IVA
#undef IMPL2_IVA
#undef IMPL3_IVA
#undef IMPL4_IVA
#undef IMPL1_LA
#undef IMPL2_LA
#undef IMPL3_LA
#undef IMPL4_LA
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
                    "control flow paths. source size: %d. dest size: %d",
                    source.size(),
                    dest.size());
    Logger::Warning("src stack : %s", source.pretty().c_str());
    Logger::Warning("dest stack: %s", dest.pretty().c_str());
    assert(false);
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
      assert(false);
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
  // TODO(drew): assert eval stack unknown is false?
  assert(!m_symStack.empty());
  char sym = m_symStack.back().sym;
  char flavor = StackSym::GetSymFlavor(sym);
  if (StackSym::GetMarker(sym) != StackSym::W &&
      flavor != StackSym::L && flavor != StackSym::T && flavor != StackSym::I &&
      flavor != StackSym::H) {
    assert(!m_actualStack.empty());
    m_actualStack.pop_back();
  }
  m_symStack.pop_back();
}

char SymbolicStack::top() const {
  assert(!m_symStack.empty());
  return m_symStack.back().sym;
}

char SymbolicStack::get(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].sym;
}

const StringData* SymbolicStack::getName(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  if (m_symStack[index].metaType == META_LITSTR) {
    return m_symStack[index].metaData.name;
  }
  return nullptr;
}

const StringData* SymbolicStack::getClsName(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].className;
}

bool SymbolicStack::isCls(int index) const {
  assert(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index].className != nullptr;
}

void SymbolicStack::setString(const StringData* s) {
  assert(m_symStack.size());
  SymEntry& se = m_symStack.back();
  if (se.metaType == META_LITSTR) {
    assert(se.metaData.name == s);
  } else {
    assert(se.metaType == META_NONE);
  }
  se.metaData.name = s;
  se.metaType = META_LITSTR;
}

void SymbolicStack::setKnownCls(const StringData* s, bool nonNull) {
  assert(m_symStack.size());
  SymEntry& se = m_symStack.back();
  assert(!se.className || se.className == s);
  if (se.metaType == META_DATA_TYPE) {
    assert(se.metaData.dt == KindOfObject);
    nonNull = true;
  }
  se.className = s;
  se.notNull = se.notNull || nonNull;
}

void SymbolicStack::setNotRef() {
  assert(m_symStack.size());
  SymEntry& se = m_symStack.back();
  se.notRef = true;
}

bool SymbolicStack::getNotRef() const {
  assert(m_symStack.size());
  const SymEntry& se = m_symStack.back();
  return se.notRef;
}

void SymbolicStack::setInt(int64_t v) {
  assert(m_symStack.size());
  m_symStack.back().intval = v;
}

void SymbolicStack::setKnownType(DataType dt, bool predicted /* = false */) {
  assert(m_symStack.size());
  SymEntry& se = m_symStack.back();
  if (se.className) {
    assert(dt == KindOfObject);
    se.notNull = true;
  } else {
    assert(se.metaType == META_NONE);
    se.metaType = META_DATA_TYPE;
    se.metaData.dt = dt;
  }
  se.dtPredicted = predicted;
}

DataType SymbolicStack::getKnownType(int index, bool noRef) const {
  if (index < 0) index += m_symStack.size();
  assert((unsigned)index < m_symStack.size());
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
  assert((unsigned)index < m_symStack.size());
  return m_symStack[index].dtPredicted;
}

void SymbolicStack::cleanTopMeta() {
  SymEntry& se = m_symStack.back();
  se.clsBaseType = CLS_INVALID;
  se.metaType = META_NONE;
  se.notRef = false;
}

void SymbolicStack::setClsBaseType(ClassBaseType type) {
  assert(!m_symStack.empty());
  m_symStack.back().clsBaseType = type;
}

void SymbolicStack::setUnnamedLocal(int index,
                                    int localId,
                                    Offset startOff) {
  assert(size_t(index) < m_symStack.size());
  assert(m_symStack[index].sym == StackSym::K);
  assert(m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  m_symStack[index].intval = localId;
  m_symStack[index].unnamedLocalStart = startOff;
}

void SymbolicStack::set(int index, char sym) {
  assert(index >= 0 && index < (int)m_symStack.size());
  // XXX Add assert in debug build to make sure W is not getting
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
    assert(false);
    return;
  }
  assert(int(m_symStack.size()) >= depth + 1);
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
  assert(vpos >= 0 && vpos < int(m_symStack.size()));
  assert(!m_actualStack.empty());
  for (int j = int(m_actualStack.size()) - 1; j >= 0; --j) {
    if (m_actualStack[j] == vpos) {
      return j;
    }
  }
  NOT_REACHED();
}

char SymbolicStack::getActual(int index) const {
  assert(index >= 0 && index < (int)m_actualStack.size());
  return get(m_actualStack[index]);
}

void SymbolicStack::setActual(int index, char sym) {
  assert(index >= 0 && index < (int)m_actualStack.size());
  set(m_actualStack[index], sym);
}

SymbolicStack::ClassBaseType
SymbolicStack::getClsBaseType(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(m_symStack[index].sym == StackSym::K);
  assert(m_symStack[index].clsBaseType != CLS_INVALID);
  return m_symStack[index].clsBaseType;
}

int SymbolicStack::getLoc(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(StackSym::GetSymFlavor(m_symStack[index].sym) == StackSym::L ||
         m_symStack[index].clsBaseType == CLS_NAMED_LOCAL ||
         m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  assert(m_symStack[index].intval != -1);
  return m_symStack[index].intval;
}

int64_t SymbolicStack::getInt(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(StackSym::GetSymFlavor(m_symStack[index].sym) == StackSym::I);
  return m_symStack[index].intval;
}

Offset SymbolicStack::getUnnamedLocStart(int index) const {
  assert(m_symStack.size() > size_t(index));
  assert(m_symStack[index].sym == StackSym::K);
  assert(m_symStack[index].clsBaseType == CLS_UNNAMED_LOCAL);
  return m_symStack[index].unnamedLocalStart;
}

// Insert an element in the actual stack at the specified depth of the
// actual stack.
void SymbolicStack::insertAt(int depth, char sym) {
  assert(depth <= sizeActual() && depth > 0);
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
// ControlTarget.

const int ControlTarget::k_unsetState = -1;

ControlTarget::ControlTarget(EmitterVisitor* router)
  : m_visitor(router), m_label(), m_state(k_unsetState) {
  assert(m_visitor != nullptr);
}

ControlTarget::~ControlTarget() {
  // The scope of states used in finally router is controled
  // using shared pointer refcounting. State numbers can be reused once
  // all the references are released.
  if (isRegistered()) {
    m_visitor->unregisterControlTarget(this);
    m_state = k_unsetState;
  }
}

bool ControlTarget::isRegistered() {
  return m_state != k_unsetState;
}

//=============================================================================
// Region.

Region::Region(Region::Kind kind, RegionPtr parent)
  : m_kind(kind),
    m_iterId(-1),
    m_iterKind(KindOfIter),
    m_parent(parent) {
}

void
EmitterVisitor::registerReturn(StatementPtr s, Region* region, char sym) {
  ControlTargetPtr t;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
              "Return inside a finally block is not supported");
    }
    if (r->m_returnTargets.count(sym)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_returnTargets[sym].target;
      break;
    }
    // Haven't registered the control target with region r yet.
    if (r->m_parent.get() == nullptr) {
      // Top of the region hierarchy - allocate a fresh control target.
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
  }
  assert(t != nullptr);
  if (!t->isRegistered()) {
    registerControlTarget(t.get());
  }
  // For all entries we visited that did not have this control target in
  // m_returnTargets, add this control target to these entries' m_returnTargets
  // fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_returnTargets[sym] = ControlTargetInfo(t, r->isTryFinally());
  }
}

ControlTargetPtr
EmitterVisitor::registerGoto(StatementPtr s, Region* region, StringData* name,
                             bool alloc) {
  ControlTargetPtr t;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
              "Goto inside a finally block is not supported");
    }
    if (r->m_gotoTargets.count(name)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_gotoTargets[name].target;
      if (alloc && r->isTryFinally()) {
        r->m_gotoTargets[name].used = true;
      }
      break;
    }
    // Haven't registered the control target in this region yet.
    if (r->m_parent.get() == nullptr) {
      // Top of the region hierarchy - allocate a fresh control target.
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
  }
  assert(t != nullptr);
  if (alloc && !t->isRegistered()) {
    registerControlTarget(t.get());
  }
  // For all entries we visited that did not have this control target in
  // m_gotoTargets, add this control target to these entries' m_gotoTargets
  // fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_gotoTargets[name] = ControlTargetInfo(t, alloc && r->isTryFinally());
  }
  return t;
}

void EmitterVisitor::registerYieldAwait(ExpressionPtr e) {
  Region* region = m_regions.back().get();
  for (; region; region = region->m_parent.get()) {
    if (region->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(e,
              "Yield expression inside a finally block is not supported");
    }
  }
}

ControlTargetPtr
EmitterVisitor::registerBreak(StatementPtr s, Region* region, int depth,
                              bool alloc) {
  ControlTargetPtr t;
  assert(depth >= 1);
  int d = depth;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
              "Break jump is not allowed to leave a finally block");
    }
    if (r->m_breakTargets.count(d)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_breakTargets[d].target;
      if (alloc && r->isTryFinally()) {
        r->m_breakTargets[d].used = true;
      }
      break;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (d == 1) {
      // We should never reach this case if alloc == true, since the loop or
      // switch should have registered its break target in advance
      assert(!alloc);
      // If this is a loop, and depth is one, just allocate a fresh
      // control target, since there are no more entries to delegate to.
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
    // Otherwise, delegate to the parent. One break level has been
    // taken care of by this region.
    --d;
  }
  assert(t != nullptr);
  if (alloc) {
    if (!t->isRegistered()) {
      registerControlTarget(t.get());
    }
  }
  // For all of the entries that did not have this control target in
  // m_breakTargets, add this control target to these entries' m_breakTargets
  // fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_breakTargets[depth] =
      ControlTargetInfo(t, alloc && r->isTryFinally());
    if (r->m_kind == Region::Kind::LoopOrSwitch) {
      --depth;
    }
  }
  return t;
}

ControlTargetPtr
EmitterVisitor::registerContinue(StatementPtr s, Region* region, int depth,
                                 bool alloc) {
  ControlTargetPtr t;
  assert(depth >= 1);
  int d = depth;
  Region* r;
  for (r = region; true; r = r->m_parent.get()) {
    assert(r);
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
              "Continue jump is not allowed to leave a finally block");
    }
    if (r->m_continueTargets.count(d)) {
      // We registered the control target before. Just return the existing one.
      t = r->m_continueTargets[d].target;
      if (alloc && r->isTryFinally()) {
        r->m_continueTargets[d].used = true;
      }
      break;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (d == 1) {
      // We should never reach this case if alloc == true, since the loop or
      // switch should have registered its continue target in advance
      assert(!alloc);
      t = std::make_shared<ControlTarget>(this);
      r = r->m_parent.get();
      break;
    }
    // Otherwise, delegate to the parent. One continue level has been
    // taken care of by this region.
    --d;
  }
  assert(t != nullptr);
  if (alloc && !t->isRegistered()) {
    registerControlTarget(t.get());
  }
  // For all of the entries that did not have this control target in
  // m_continueTargets, add this control target to these entries'
  // m_continueTargets fields as appropriate.
  Region* end = r;
  for (r = region; r != end; r = r->m_parent.get()) {
    r->m_continueTargets[depth] =
      ControlTargetInfo(t, alloc && r->isTryFinally());
    if (r->m_kind == Region::Kind::LoopOrSwitch) {
      --depth;
    }
  }
  return t;
}

int Region::getCaseCount() {
  int count = 1; // The fall-through case.
  for (auto& t : m_returnTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  for (auto& t : m_breakTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  for (auto& t : m_continueTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  for (auto& t : m_gotoTargets) {
    if (t.second.target->isRegistered()) ++count;
  }
  return count;
}

void EmitterVisitor::emitIterFree(Emitter& e, IterVec& iters) {
  for (auto& iter : iters) {
    assert(iter.id != -1);
    if (iter.kind == KindOfMIter) {
      e.MIterFree(iter.id);
    } else {
      assert(iter.kind == KindOfIter);
      e.IterFree(iter.id);
    }
  }
}

void EmitterVisitor::emitIterFreeForReturn(Emitter& e) {
  Region* region = m_regions.back().get();
  IterVec iters;
  while (region != nullptr) {
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    region = region->m_parent.get();
  }
  emitIterFree(e, iters);
}

void EmitterVisitor::emitJump(Emitter& e, IterVec& iters, Label& target) {
  if (!iters.empty()) {
    e.IterBreak(iters, target);
    iters.clear();
  } else {
    e.Jmp(target);
  }
}

void EmitterVisitor::emitReturn(Emitter& e, char sym, bool hasConstraint,
                                StatementPtr s) {
  Region* region = m_regions.back().get();
  registerReturn(s, region, sym);
  assert(getEvalStack().size() == 1);
  assert(region->m_returnTargets.count(sym));
  IterVec iters;
  for (Region* r = region; true; r = r->m_parent.get()) {
    auto& t = r->m_returnTargets[sym].target;
    if (r->m_parent == nullptr) {
      // At the top of the hierarchy, no more finally blocks to run.
      // Free the pending iterators and perform the actual return.
      emitIterFree(e, iters);
      if (sym == StackSym::C) {
        if (hasConstraint) {
          e.VerifyRetTypeC();
        }
        e.RetC();
      } else {
        assert(sym == StackSym::V);
        if (hasConstraint) {
          e.VerifyRetTypeV();
        }
        e.RetV();
      }
      return;
    }

    if (r->isTryFinally()) {
      // We encountered a try block - a finally needs to be run
      // before returning.
      Id stateLocal = getStateLocal();
      Id retLocal = getRetLocal();
      // Set the unnamed "state" local to the appropriate identifier
      emitVirtualLocal(retLocal);
      assert(t->isRegistered());
      e.Int(t->m_state);
      e.SetL(stateLocal);
      e.PopC();
      // Emit code stashing the current return value in the "ret" unnamed
      // local
      if (sym == StackSym::C) {
        // For legacy purposes, SetL expects its immediate argument to
        // be present on the symbolic stack. In reality, retLocal is
        // an immediate argument. The following pop and push instructions
        // ensure that the arguments are place on the symbolic stack
        // in a correct order. In reality the following three calls are
        // a no-op.
        popEvalStack(StackSym::C);
        emitVirtualLocal(retLocal);
        pushEvalStack(StackSym::C);
        e.SetL(retLocal);
        e.PopC();
      } else {
        assert(sym == StackSym::V);
        popEvalStack(StackSym::V);
        emitVirtualLocal(retLocal);
        pushEvalStack(StackSym::V);
        e.BindL(retLocal);
        e.PopV();
      }
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
  }
}

void EmitterVisitor::emitGoto(Emitter& e, StringData* name, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerGoto(s, region, name, true);
  assert(!region->isFinally());
  assert(region->m_gotoTargets.count(name));
  IterVec iters;
  for (Region* r = region; true; r = r->m_parent.get()) {
    auto t = r->m_gotoTargets[name].target;
    if (r->m_gotoLabels.count(name)) {
      // If only the destination label is within the statement
      // associated with the current statement, just perform a
      // direct jump. Free the pending iterators on the way.
      emitJump(e, iters, t->m_label);
      return;
    }
    if (r->isTryFinally()) {
      // We came across a try region, need for run a finally block.
      // Store appropriate value inside the state local.
      Id stateLocal = getStateLocal();
      emitVirtualLocal(stateLocal);
      assert(t->isRegistered());
      e.Int(t->m_state);
      e.SetL(stateLocal);
      e.PopC();
      // Jump to the finally block and free any pending iterators on the
      // way.
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
  }
}

void EmitterVisitor::emitBreak(Emitter& e, int depth, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerBreak(s, region, depth, true);
  assert(depth >= 1);
  assert(!region->isFinally());
  assert(region->m_parent != nullptr);
  assert(region->m_breakTargets.count(depth));
  IterVec iters;

  for (Region* r = region; true; r = r->m_parent.get()) {
    auto t = r->m_breakTargets[depth].target;
    if (r->isTryFinally()) {
      // Encountered a try block, need to run finally.
      assert(r->m_breakTargets.count(depth));
      assert(t->isRegistered());
      Id stateLocal = getStateLocal();
      emitVirtualLocal(stateLocal);
      e.Int(t->m_state);
      e.SetL(stateLocal);
      e.PopC();
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    // Free iterator for the current loop whether or not
    // this is the last loop that we jump out of.
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
    if (depth == 1) {
      // Last loop to jumpt out of. Performa direct jump to the
      // break lable and free any pending iterators left.
      emitJump(e, iters, t->m_label);
      return;
    }
    --depth;
  }
}

void EmitterVisitor::emitContinue(Emitter& e, int depth, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerContinue(s, region, depth, true);
  assert(depth >= 1);
  assert(!region->isFinally());
  assert(region->m_parent != nullptr);
  assert(region->m_continueTargets.count(depth));
  IterVec iters;

  for (Region* r = region; true; r = r->m_parent.get()) {
    auto t = r->m_continueTargets[depth].target;
    if (r->isTryFinally()) {
      // Encountered a try block, need to run finally.
      assert(r->m_continueTargets.count(depth));
      Id stateLocal = getStateLocal();
      emitVirtualLocal(stateLocal);
      assert(t->isRegistered());
      e.Int(t->m_state);
      e.SetL(stateLocal);
      e.PopC();
      emitJump(e, iters, r->m_finallyLabel);
      return;
    }
    if (r->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (depth == 1) {
      // Last level. Don't free the iterator for the current loop
      // however free any earlier pending iterators.
      emitJump(e, iters, t->m_label);
      return;
    }
    // Only free the iterator for the current loop if this is
    // NOT the last level to continue out of.
    if (r->isForeach()) {
      iters.push_back(IterPair(r->m_iterKind, r->m_iterId));
    }
    --depth;
  }
}

void EmitterVisitor::emitFinallyEpilogue(Emitter& e, Region* region) {
  assert(region != nullptr);
  assert(region->isTryFinally());
  assert(region->m_finallyLabel.isSet());
  int count = region->getCaseCount();
  assert(count >= 1);
  Label after;
  if (count == 1) {
    // If there is only one case (the fall-through case) then we're done
    after.set(e);
    return;
  }
  // Otherwise, we need to emit some conditional jumps/switches to handle
  // the different cases. We start by builing up a vector of Label* that
  // we'll use for the Switch instruction and/or for conditional branches.
  int maxState = region->getMaxState();
  std::vector<Label*> cases;
  while (cases.size() <= maxState) {
    cases.push_back(new Label());
  }
  // Now that we have our vector of Label*'s ready, we can emit a
  // Switch instruction and/or conditional branches, and we can
  // emit the body of each case.
  auto& evalStack = getEvalStack();
  Id stateLocal = getStateLocal();
  emitVirtualLocal(stateLocal);
  e.IssetL(stateLocal);
  e.JmpZ(after);
  if (count >= 3) {
    // A switch is needed since there are more than two cases.
    emitVirtualLocal(stateLocal);
    evalStack.setKnownType(KindOfInt64);
    e.CGetL(stateLocal);
    e.Switch(cases, 0, 0);
  }
  for (auto& p : region->m_returnTargets) {
    if (p.second.used) emitReturnTrampoline(e, region, cases, p.first);
  }
  assert(region->isTryFinally());
  int max_depth = region->getBreakContinueDepth();
  for (int i = 1; i <= max_depth; ++i) {
    if (region->isBreakUsed(i)) emitBreakTrampoline(e, region, cases, i);
    if (region->isContinueUsed(i)) emitContinueTrampoline(e, region, cases, i);
  }
  for (auto& p : region->m_gotoTargets) {
    if (p.second.used) emitGotoTrampoline(e, region, cases, p.first);
  }
  for (auto c : cases) {
    delete c;
  }
  after.set(e);
}

void EmitterVisitor::emitReturnTrampoline(Emitter& e,
                                          Region* region,
                                          std::vector<Label*>& cases,
                                          char sym) {
  assert(region->isTryFinally());
  assert(region->m_parent != nullptr);
  assert(region->m_returnTargets.count(sym));
  auto& t = region->m_returnTargets[sym].target;
  cases[t->m_state]->set(e);

  IterVec iters;
  // We are emitting a case in a finally epilogue, therefore skip
  // the current try region and start from its parent
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(region->m_returnTargets.count(sym));
    assert(region->m_returnTargets[sym].target->isRegistered());
    // Add pending iterator if applicable
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    if (region->m_parent == nullptr) {
      // At the bottom of the hierarchy. Restore the return value
      // and perform the actual return.
      Id retLocal = getRetLocal();
      emitVirtualLocal(retLocal);
      if (sym == StackSym::C) {
        e.CGetL(retLocal);
        e.RetC();
      } else {
        assert(sym == StackSym::V);
        e.VGetL(retLocal);
        e.RetV();
      }
      return;
    }
    if (region->isTryFinally()) {
      // Encountered another try block, jump to its finally and free
      // iterators on the way.
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
  }
}

void EmitterVisitor::emitGotoTrampoline(Emitter& e,
                                        Region* region,
                                        std::vector<Label*>& cases,
                                        StringData* name) {
  assert(region->m_gotoTargets.count(name));
  auto t = region->m_gotoTargets[name].target;
  cases[t->m_state]->set(e);
  assert(region->m_parent != nullptr);
  IterVec iters;
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(region->m_gotoTargets.count(name));
    auto t = region->m_gotoTargets[name].target;
    if (region->m_parent->m_gotoLabels.count(name)) {
      // If only there is the appropriate label inside the current region
      // perform a jump.
      Id stateLocal = getStateLocal();
      emitVirtualLocal(stateLocal);
      // We need to unset the state unnamed local in order to correctly
      // fall through any future finally blocks.
      e.UnsetL(stateLocal);
      // Jump to the label and free any pending iterators.
      emitJump(e, iters, t->m_label);
      return;
    }
    if (region->isTryFinally()) {
      // Encountered a finally block, jump and free any pending iterators
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
    // Otherwise we will be jumping out of the current context,
    // therefore if we are in a loop, we need to free the iterator.
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    // We should never break out of a function, therefore there
    // should always be a parent
    assert(region->m_parent != nullptr);
  }
}

void EmitterVisitor::emitBreakTrampoline(Emitter& e, Region* region,
                                         std::vector<Label*>& cases,
                                         int depth) {
  assert(depth >= 1);
  assert(region->isTryFinally());
  assert(region->m_breakTargets.count(depth));
  auto t = region->m_breakTargets[depth].target;
  cases[t->m_state]->set(e);
  assert(region->m_parent != nullptr);
  IterVec iters;
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(depth >= 1);
    assert(!region->isFinally());
    assert(region->m_parent != nullptr);
    assert(region->m_breakTargets.count(depth));
    auto t = region->m_breakTargets[depth].target;
    if (region->isTryFinally()) {
      // We encountered another try block, jump to the corresponding
      // finally, freeing any iterators on the way.
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
    if (region->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    // Whether or not this is the last loop to break out of, we
    // will be freeing the current iterator
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    if (depth == 1) {
      // This is the last loop to break out of. Unset the state local in
      // order to correctly fall through any future finally blocks
      Id stateLocal = getStateLocal();
      emitVirtualLocal(stateLocal);
      e.UnsetL(stateLocal);
      // Jump to the break label and free any pending iterators on the
      // way.
      emitJump(e, iters, t->m_label);
      return;
    }
    // Otherwise just delegate to the parent. One loop level has been
    // taken care of.
    --depth;
  }
}

void EmitterVisitor::emitContinueTrampoline(Emitter& e, Region* region,
                                            std::vector<Label*>& cases,
                                            int depth) {
  assert(depth >= 1);
  assert(region->isTryFinally());
  assert(region->m_continueTargets.count(depth));
  auto t = region->m_continueTargets[depth].target;
  cases[t->m_state]->set(e);
  assert(region->m_parent != nullptr);
  IterVec iters;
  for (region = region->m_parent.get(); true; region = region->m_parent.get()) {
    assert(depth >= 1);
    assert(region->m_parent != nullptr);
    assert(!region->isFinally());
    auto t = region->m_continueTargets[depth].target;
    if (region->isTryFinally()) {
      emitJump(e, iters, region->m_finallyLabel);
      return;
    }
    if (region->m_kind != Region::Kind::LoopOrSwitch) {
      continue;
    }
    if (depth == 1) {
      // This is the last loop level to continue out of. Don't free the
      // iterator for the current loop. We need to free the state unnamed
      // local in order to fall through any future finallies correctly
      Id stateLocal = getStateLocal();
      emitVirtualLocal(stateLocal);
      e.UnsetL(stateLocal);
      // Jump to the continue label and free any pending iterators
      emitJump(e, iters, t->m_label);
      return;
    }
    // This is not the last loop level, therefore the current
    // iterator should be freed.
    if (region->isForeach()) {
      iters.push_back(IterPair(region->m_iterKind, region->m_iterId));
    }
    --depth;
  }
}

int Region::getMaxBreakContinueDepth() {
  if (m_parent == nullptr || isFinally()) {
    return 0;
  } else if (m_kind == Region::Kind::LoopOrSwitch) {
    return m_parent->getMaxBreakContinueDepth() + 1;
  } else {
    return m_parent->getMaxBreakContinueDepth();
  }
}

int Region::getBreakContinueDepth() {
  int depth = 0;
  for (auto& p : m_breakTargets) {
    depth = std::max(depth, p.first);
  }
  for (auto& p : m_continueTargets) {
    depth = std::max(depth, p.first);
  }
  return depth;
}

int Region::getMaxState() {
  int maxState = -1;
  for (auto& p : m_returnTargets) {
    if (p.second.used) {
      maxState = std::max(maxState, p.second.target->m_state);
    }
  }
  int max_depth = getBreakContinueDepth();
  for (int i = 1; i <= max_depth; ++i) {
    if (isBreakUsed(i)) {
      maxState = std::max(maxState, m_breakTargets[i].target->m_state);
    }
    if (isContinueUsed(i)) {
      maxState = std::max(maxState, m_continueTargets[i].target->m_state);
    }
  }
  for (auto& p : m_gotoTargets) {
    if (p.second.used) {
      maxState = std::max(maxState, p.second.target->m_state);
    }
  }
  return maxState;
}

RegionPtr
EmitterVisitor::createRegion(StatementPtr s, Region::Kind kind) {
  RegionPtr parent = nullptr;
  if (kind != Region::Kind::FuncBody && kind != Region::Kind::FaultFunclet &&
      kind != Region::Kind::Global && !m_regions.empty()) {
    parent = m_regions.back();
  }
  auto region = std::make_shared<Region>(kind, parent);
  // We preregister all the labels occurring in the provided statement
  // ahead of the time. Therefore at the time of emitting the actual
  // goto instructions we can reliably tell which finally blocks to
  // run.
  for (auto& label : s->getLabelScope()->getLabels()) {
    StringData* nName = makeStaticString(label.getName().c_str());
    if (!region->m_gotoLabels.count(nName)) {
      region->m_gotoLabels.insert(nName);
    }
  }
  return region;
}

void EmitterVisitor::enterRegion(RegionPtr region) {
  assert(region != nullptr);
  m_regions.push_back(region);
}

void EmitterVisitor::leaveRegion(RegionPtr region) {
  assert(region != nullptr);
  assert(m_regions.size() > 0);
  assert(m_regions.back() == region);
  m_regions.pop_back();
}

void EmitterVisitor::registerControlTarget(ControlTarget* t) {
  assert(!t->isRegistered());
  int state = 0;
  while (m_states.count(state)) {
    ++state;
  }
  m_states.insert(state);
  t->m_state = state;
}

void EmitterVisitor::unregisterControlTarget(ControlTarget* t) {
  assert(t->isRegistered());
  int state = t->m_state;
  assert(m_states.count(state));
  m_states.erase(state);
  t->m_state = ControlTarget::k_unsetState;
}

//=============================================================================

void MetaInfoBuilder::add(int pos, Unit::MetaInfo::Kind kind,
                          bool mVector, int arg, Id data) {
  assert(arg >= 0);
  if (arg > 127) return;
  if (mVector) arg |= Unit::MetaInfo::VectorArg;
  Vec& info = m_metaMap[pos];
  int i = info.size();
  if (kind == Unit::MetaInfo::Kind::DataTypeInferred ||
      kind == Unit::MetaInfo::Kind::DataTypePredicted) {
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
                                   Unit::MetaInfo::Kind::DataTypePredicted :
                                   Unit::MetaInfo::Kind::DataTypeInferred);
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
  vector<uint8_t> data;
  index1.push_back(entries);

  size_t sz1 = (2 + entries) * sizeof(Offset);
  size_t sz2 = (1 + entries) * sizeof(Offset);
  for (Map::const_iterator it = m_metaMap.begin(), end = m_metaMap.end();
      it != end; ++it) {
    index1.push_back(it->first);
    index2.push_back(sz1 + sz2 + data.size());

    const Vec& v = it->second;
    assert(v.size());
    for (unsigned i = 0; i < v.size(); i++) {
      const Unit::MetaInfo& mi = v[i];
      data.push_back(static_cast<uint8_t>(mi.m_kind));
      data.push_back(mi.m_arg);
      if (mi.m_data < 0x80) {
        data.push_back(mi.m_data << 1);
      } else {
        union {
          uint32_t val;
          uint8_t  bytes[4];
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
  uint8_t* meta = (uint8_t*)malloc(size);
  memcpy(meta, &index1[0], sz1);
  memcpy(meta + sz1, &index2[0], sz2);
  memcpy(meta + sz1 + sz2, &data[0], data.size());
  target.setBcMeta(meta, size);
  free(meta);
}

//=============================================================================
// EmitterVisitor.

EmitterVisitor::EmittedClosures EmitterVisitor::s_emittedClosures;

EmitterVisitor::EmitterVisitor(UnitEmitter& ue)
  : m_ue(ue), m_curFunc(ue.getMain()),
    m_inGenerator(false), m_evalStackIsUnknown(false),
    m_actualStackHighWater(0), m_fdescHighWater(0), m_stateLocal(-1),
    m_retLocal(-1) {
  m_prevOpcode = OpLowInvalid;
  m_evalStack.m_actualStackHighWaterPtr = &m_actualStackHighWater;
  m_evalStack.m_fdescHighWaterPtr = &m_fdescHighWater;
}

EmitterVisitor::~EmitterVisitor() {
  // If a fatal occurs during emission, some extra cleanup is necessary.
  for (std::deque<CatchRegion*>::const_iterator it = m_catchRegions.begin();
       it != m_catchRegions.end(); ++it) {
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

void EmitterVisitor::popSymbolicLocal(Op op, int arg, int pos) {
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

void EmitterVisitor::popEvalStackMMany() {
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
      assert(symFlavor == StackSym::A);
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

void EmitterVisitor::popEvalStackCVMany(int len) {
  for (int i = 0; i < len; i++) {
    if (m_evalStack.size() == 0) {
      InvariantViolation("Emitter emitted an instruction that tries to consume "
                         "a value from the stack when the stack is empty "
                         "(expected symbolic flavor C or V at offset %d)",
                         m_ue.bcPos());
      return;
    }

    char sym = m_evalStack.top();
    char actual = StackSym::GetSymFlavor(sym);
    m_evalStack.pop();
    if (actual != StackSym::C && actual != StackSym::V) {
      InvariantViolation(
        "Emitter emitted an instruction that tries to consume a "
        "value from the stack when the top of the stack does not "
        "match the symbolic flavor that the instruction expects "
        "(expected symbolic flavor C or V, actual symbolic flavor \"%s\" "
        "at offset %d)",
        StackSym::ToString(actual).c_str(),
        m_ue.bcPos());
    }
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
                         "stack because the eval stack was unknown, but "
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
  auto it = m_jumpTargetEvalStacks.find(target);
  if (it == m_jumpTargetEvalStacks.end()) {
    m_jumpTargetEvalStacks[target] = evalStack;
    return;
  }
  checkJmpTargetEvalStack(evalStack, it->second);
}

void EmitterVisitor::restoreJumpTargetEvalStack() {
  m_evalStack.clear();
  auto it = m_jumpTargetEvalStacks.find(m_ue.bcPos());
  if (it == m_jumpTargetEvalStacks.end()) {
    m_evalStackIsUnknown = true;
    return;
  }
  m_evalStack = it->second;
}

void EmitterVisitor::recordCall() {
  m_curFunc->setContainsCalls();
}

bool EmitterVisitor::isJumpTarget(Offset target) {
  // Returns true iff one of the following conditions is true:
  //   1) We have seen an instruction that jumps to the specified offset
  //   2) We know of a Label that has been set to the specified offset
  //   3) We have seen a try region that ends at the specified offset
  auto it = m_jumpTargetEvalStacks.find(target);
  return (it != m_jumpTargetEvalStacks.end());
}

class IterFreeThunklet : public Thunklet {
public:
  IterFreeThunklet(Id iterId, bool itRef)
    : m_id(iterId), m_itRef(itRef) {}
  virtual void emit(Emitter& e) {
    if (m_itRef) {
      e.MIterFree(m_id);
    } else {
      e.IterFree(m_id);
    }
    e.Unwind();
  }
private:
  Id m_id;
  bool m_itRef;
};

/**
 * A thunklet for the fault region protecting a silenced (@) expression.
 */
class RestoreErrorReportingThunklet : public Thunklet {
public:
  explicit RestoreErrorReportingThunklet(Id loc)
    : m_oldLevelLoc(loc) {}
  virtual void emit(Emitter& e) {
    e.getEmitterVisitor().emitRestoreErrorReporting(e, m_oldLevelLoc);
    e.Unwind();
  }
private:
  Id m_oldLevelLoc;
};

class UnsetUnnamedLocalThunklet : public Thunklet {
public:
  explicit UnsetUnnamedLocalThunklet(Id loc)
    : m_loc(loc) {}
  virtual void emit(Emitter& e) {
    e.getEmitterVisitor().emitVirtualLocal(m_loc);
    e.getEmitterVisitor().emitUnset(e);
    e.Unwind();
  }
private:
  Id m_loc;
};

class FinallyThunklet : public Thunklet {
public:
  explicit FinallyThunklet(FinallyStatementPtr finallyStatement,
                           int numLiveIters)
      : m_finallyStatement(finallyStatement), m_numLiveIters(numLiveIters) {}
  virtual void emit(Emitter& e) {
    auto& visitor = e.getEmitterVisitor();
    auto region =
      visitor.createRegion(m_finallyStatement, Region::Kind::FaultFunclet);
    visitor.enterRegion(region);
    SCOPE_EXIT { visitor.leaveRegion(region); };
    Id stateLocal = visitor.getStateLocal();
    visitor.emitVirtualLocal(stateLocal);
    e.UnsetL(stateLocal);
    Id retLocal = visitor.getStateLocal();
    visitor.emitVirtualLocal(retLocal);
    e.UnsetL(retLocal);
    auto* func = visitor.getFuncEmitter();
    int oldNumLiveIters = func->numLiveIterators();
    func->setNumLiveIterators(m_numLiveIters);
    SCOPE_EXIT { func->setNumLiveIterators(oldNumLiveIters); };
    visitor.visit(m_finallyStatement);
    e.Unwind();
  }
private:
  FinallyStatementPtr m_finallyStatement;
  int m_numLiveIters;
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
    assert(!indexChain.empty());
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

// Assigns ids to all of the local variables eagerly. This gives us the
// nice property that all named local variables will be assigned ids
// 0 through k-1, while any unnamed local variable will have an id >= k.
void EmitterVisitor::assignLocalVariableIds(FunctionScopePtr fs) {
  VariableTablePtr variables = fs->getVariables();
  std::vector<std::string> localNames;
  variables->getLocalVariableNames(localNames);
  for (int i = 0; i < (int)localNames.size(); ++i) {
    StringData* nLiteral = makeStaticString(localNames[i].c_str());
    m_curFunc->allocVarId(nLiteral);
  }
}

void EmitterVisitor::assignFinallyVariableIds() {
  assert(m_stateLocal < 0);
  m_stateLocal = m_curFunc->allocUnnamedLocal();
  assert(m_retLocal < 0);
  m_retLocal = m_curFunc->allocUnnamedLocal();
}

void EmitterVisitor::visit(FileScopePtr file) {
  const std::string& filename = file->getName();
  m_ue.setFilepath(makeStaticString(filename));

  FunctionScopePtr func(file->getPseudoMain());
  if (!func) return;

  m_file = file;
  assignLocalVariableIds(func);

  AnalysisResultPtr ar(file->getContainingProgram());
  assert(ar);
  MethodStatementPtr m(dynamic_pointer_cast<MethodStatement>(func->getStmt()));
  if (!m) return;
  StatementListPtr stmts(m->getStmts());
  if (!stmts) return;

  Emitter e(m, m_ue, *this);

  int i, nk = stmts->getCount();
  for (i = 0; i < nk; i++) {
    StatementPtr s = (*stmts)[i];
    if (MethodStatementPtr meth = dynamic_pointer_cast<MethodStatement>(s)) {
      // Emit afterwards
      postponeMeth(meth, nullptr, true);
    }
  }
  {
    FunctionScopePtr fsp = m->getFunctionScope();
    if (fsp->needsLocalThis()) {
      static const StringData* thisStr = makeStaticString("this");
      Id thisId = m_curFunc->lookupVarId(thisStr);
      e.InitThisLoc(thisId);
    }
    if (fsp->needsFinallyLocals()) {
      assignFinallyVariableIds();
    }
    FuncFinisher ff(this, e, m_curFunc);
    TypedValue mainReturn;
    mainReturn.m_type = KindOfInvalid;
    bool notMergeOnly = false;

    if (Option::UseHHBBC && SystemLib::s_inited) notMergeOnly = true;

    auto region = createRegion(stmts, Region::Kind::Global);
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };

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
          emitClass(e, cNode, true);
          if (cNode->getFatalMessage()) {
            notMergeOnly = true;
          }
          break;
        }
        case Statement::KindOfTypedefStatement:
          emitTypedef(e, static_pointer_cast<TypedefStatement>(s));
          notMergeOnly = true; // TODO(#2103206): typedefs should be mergable
          break;
        case Statement::KindOfReturnStatement:
          if (mainReturn.m_type != KindOfInvalid) break;

          visit(s);
          if (notMergeOnly) {
            tvWriteUninit(&mainReturn);
            m_ue.returnSeen();
            continue;
          }

          {
            ReturnStatementPtr r(static_pointer_cast<ReturnStatement>(s));
            Variant v((Variant::NullInit()));
            if (r->getRetExp() &&
                !r->getRetExp()->getScalarValue(v)) {
              tvWriteUninit(&mainReturn);
              notMergeOnly = true;
              continue;
            }
            if (v.isString()) {
              v = String(makeStaticString(v.asCStrRef().get()));
            } else if (v.isArray()) {
              v = Array(ArrayData::GetScalarArray(v.asCArrRef().get()));
            } else if (v.isNull()) {
              // getScalarValue() sets this to uninit_null, so lets set it back
              v = init_null_variant;
            } else {
              assert(!IS_REFCOUNTED_TYPE(v.getType()));
            }
            mainReturn = *v.asCell();
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
                  UnitMergeKind k = func->isDefineWithoutImpl(ar) ?
                    UnitMergeKindPersistentDefine : UnitMergeKindDefine;
                  if (tv.m_type == KindOfUninit) {
                    tv.m_type = KindOfNull;
                  }
                  m_ue.pushMergeableDef(k, name, tv);
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
                      assert(ps && ps->inPseudoMain());
                      UnitMergeKind kind = UnitMergeKindReqDoc;
                      m_ue.pushMergeableInclude(
                        kind,
                        makeStaticString(inc->includePath()));
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
          notMergeOnly = true;
          visit(s);
      }
    }

    if (!notMergeOnly) {
      m_ue.setMergeOnly(true);
      if (mainReturn.m_type == KindOfInvalid) {
        tvWriteUninit(&mainReturn);
        tvAsVariant(&mainReturn) = 1;
      }
      m_ue.setMainReturn(&mainReturn);
    }

    // Pseudo-main returns the integer value 1 by default. If the
    // current position in the bytecode is reachable, emit code to
    // return 1.
    if (currentPositionIsReachable()) {
      LocationPtr loc(new Location());
      e.setTempLocation(loc);
      e.Int(1);
      e.RetC();
      e.setTempLocation(LocationPtr());
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
    return makeStaticString(cls->getOriginalName());
  }
  return nullptr;
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
                                   Func* builtinFunc) {
  int ref = -1;
  if (fn->hasAnyContext(Expression::RefValue |
                        Expression::DeepReference |
                        Expression::LValue |
                        Expression::OprLValue |
                        Expression::UnsetContext)) {
    return;
  }
  bool voidReturn = false;
  if (builtinFunc) {
    ref = (builtinFunc->attrs() & AttrReference) != 0;
    voidReturn = builtinFunc->returnType() == KindOfNull;
  } else if (fn->isValid() && fn->getFuncScope()) {
    ref = fn->getFuncScope()->isRefReturn();
    if (!(fn->getActualType()) && !fn->getFuncScope()->isNative()) {
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
    assert(m_evalStack.get(m_evalStack.size() - 1) == StackSym::R);
    if (ref) {
      e.BoxRNop();
    } else {
      e.UnboxRNop();
    }
  }

  if (voidReturn) {
    m_evalStack.setKnownType(KindOfNull, false /* inferred */);
    m_evalStack.setNotRef();
  } else if (!ref) {
    DataType dt = builtinFunc ?
      builtinFunc->returnType() :
      getPredictedDataType(fn);

    if (dt != KindOfUnknown) {
      if (builtinFunc) {
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

template<class Fun>
bool checkKeys(ExpressionPtr init_expr, Fun fun) {
  if (init_expr->getKindOf() != Expression::KindOfExpressionList) return false;
  ExpressionListPtr el = static_pointer_cast<ExpressionList>(init_expr);
  int n = el->getCount();
  if (n < 1 || n > HphpArray::MaxMakeSize) return false;
  for (int i = 0, n = el->getCount(); i < n; ++i) {
    ExpressionPtr ex = (*el)[i];
    if (ex->getKindOf() != Expression::KindOfArrayPairExpression) return false;
    ArrayPairExpressionPtr ap = static_pointer_cast<ArrayPairExpression>(ex);
    if (ap->isRef()) return false;
    if (!fun(ap)) return false;
  }
  return true;
}

/*
 * isPackedInit() returns true if this expression list looks like an
 * array with no keys and no ref values; e.g. array(x,y,z).
 * In this case we can NewPackedArray to create the array. The elements are
 * pushed on the stack, so we arbitrarily limit this to a small multiple of
 * HphpArray::SmallSize (12).
 */
bool isPackedInit(ExpressionPtr init_expr, int* size) {
  *size = 0;
  return checkKeys(init_expr, [&](ArrayPairExpressionPtr ap) {
    if (ap->getName() != nullptr) return false;
    (*size)++;
    return true;
  });
}

/*
 * isStructInit() is like isPackedInit(), but returns true if the keys are
 * all static strings with no duplicates.
 */
bool isStructInit(ExpressionPtr init_expr, std::vector<std::string>& keys) {
  return checkKeys(init_expr, [&](ArrayPairExpressionPtr ap) {
    auto key = ap->getName();
    if (key == nullptr || !key->isLiteralString()) return false;
    auto name = key->getLiteralString();
    int64_t ival;
    double dval;
    auto kind = is_numeric_string(name.data(), name.size(), &ival, &dval, 0);
    if (kind != KindOfNull) return false; // don't allow numeric keys
    if (std::find(keys.begin(), keys.end(), name) != keys.end()) return false;
    keys.push_back(name);
    return true;
  });
}

bool EmitterVisitor::visit(ConstructPtr node) {
  bool ret = visitImpl(node);
  if (!Option::WholeProgram || !ret) return ret;
  ExpressionPtr e = dynamic_pointer_cast<Expression>(node);
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

      case Statement::KindOfTypedefStatement: {
        emitMakeUnitFatal(e, "Type statements are currently only allowed at "
                             "the top-level");
        return false;
      }

      case Statement::KindOfContinueStatement:
      case Statement::KindOfBreakStatement: {
        BreakStatementPtr bs(static_pointer_cast<BreakStatement>(s));
        uint64_t destLevel = bs->getDepth();

        if (destLevel > m_regions.back()->getMaxBreakContinueDepth()) {
          std::ostringstream msg;
          msg << "Cannot break/continue " << destLevel << " level";
          if (destLevel > 1) {
            msg << "s";
          }
          emitMakeUnitFatal(e, msg.str().c_str());
          return false;
        }

        if (bs->is(Statement::KindOfBreakStatement)) {
          emitBreak(e, destLevel, bs);
        } else {
          emitContinue(e, destLevel, bs);
        }

        return false;
      }

      case Statement::KindOfDoStatement: {
        auto region = createRegion(s, Region::Kind::LoopOrSwitch);
        DoStatementPtr ds(static_pointer_cast<DoStatement>(s));

        Label top(e);
        Label& condition =
          registerContinue(ds, region.get(), 1, false)->m_label;
        Label& exit =
          registerBreak(ds, region.get(), 1, false)->m_label;
        {
          enterRegion(region);
          SCOPE_EXIT { leaveRegion(region); };
          visit(ds->getBody());
        }
        condition.set(e);
        {
          ExpressionPtr c = ds->getCondExp();
          Emitter condEmitter(c, m_ue, *this);
          visitIfCondition(c, condEmitter, top, exit, false);
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
        StringData* vName = makeStaticString(cs->getVariable()->getName());
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
        ExpStatementPtr es(static_pointer_cast<ExpStatement>(s));
        if (visit(es->getExpression())) {
          emitPop(e);
        }
        return false;
      }

      case Statement::KindOfForStatement: {
        auto region = createRegion(s, Region::Kind::LoopOrSwitch);
        ForStatementPtr fs(static_pointer_cast<ForStatement>(s));

        if (visit(fs->getInitExp())) {
          emitPop(e);
        }
        Label preCond(e);
        Label& preInc = registerContinue(fs, region.get(), 1, false)->m_label;
        Label& fail = registerBreak(fs, region.get(), 1, false)->m_label;
        if (ExpressionPtr condExp = fs->getCondExp()) {
          Label tru;
          Emitter condEmitter(condExp, m_ue, *this);
          visitIfCondition(condExp, condEmitter, tru, fail, true);
          if (tru.isUsed()) tru.set(e);
        }
        {
          enterRegion(region);
          SCOPE_EXIT { leaveRegion(region); };
          visit(fs->getBody());
        }
        preInc.set(e);
        if (visit(fs->getIncExp())) {
          emitPop(e);
        }
        e.Jmp(preCond);
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
            StringData* nLiteral = makeStaticString(sv->getName());
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

        char retSym = StackSym::C;
        if (visit(r->getRetExp())) {
          if (r->getRetExp()->getContext() & Expression::RefValue) {
            emitConvertToVar(e);
            retSym = StackSym::V;
          } else {
            emitConvertToCell(e);
          }
        } else {
          e.Null();
        }

        assert(m_evalStack.size() == 1);

        // continuations and resumed async functions
        if (m_inGenerator) {
          assert(retSym == StackSym::C);
          emitIterFreeForReturn(e);
          e.ContRetC();
          return false;
        }

        // eagerly executed async functions
        if (m_curFunc->isAsync()) {
          assert(retSym == StackSym::C);
          emitIterFreeForReturn(e);
          e.AsyncWrapResult();
          e.RetC();
          return false;
        }

        if (r->isGuarded()) {
          m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Kind::GuardedThis,
                      false, 0, 0);
        }

        auto tc = m_curFunc->returnTypeConstraint();
        emitReturn(e, retSym, tc.hasConstraint(), r);

        return false;
      }

      case Statement::KindOfStaticStatement: {
        ExpressionListPtr vars(
          static_pointer_cast<StaticStatement>(node)->getVars());
        for (int i = 0, n = vars->getCount(); i < n; i++) {
          ExpressionPtr se((*vars)[i]);
          assert(se->is(Expression::KindOfAssignmentExpression));
          AssignmentExpressionPtr ae(
            static_pointer_cast<AssignmentExpression>(se));
          ExpressionPtr var(ae->getVariable());
          ExpressionPtr value(ae->getValue());
          assert(var->is(Expression::KindOfSimpleVariable));
          SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(var));
          StringData* name = makeStaticString(sv->getName());
          Id local = m_curFunc->lookupVarId(name);

          if (m_staticEmitted.insert(sv->getName()).second) {
            Func::SVInfo svInfo;
            svInfo.name = name;
            std::ostringstream os;
            CodeGenerator cg(&os, CodeGenerator::PickledPHP);
            AnalysisResultPtr ar(new AnalysisResult());
            value->outputPHP(cg, ar);
            svInfo.phpCode = makeStaticString(os.str());
            m_curFunc->addStaticVar(svInfo);
          }

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
        auto region = createRegion(s, Region::Kind::LoopOrSwitch);
        SwitchStatementPtr sw(static_pointer_cast<SwitchStatement>(node));

        StatementListPtr cases(sw->getCases());
        if (!cases) {
          visit(sw->getExp());
          emitPop(e);
          return false;
        }
        uint ncase = cases->getCount();
        std::vector<Label> caseLabels(ncase);
        Label& brkTarget = registerBreak(sw, region.get(), 1, false)->m_label;
        Label& contTarget =
          registerContinue(sw, region.get(), 1, false)->m_label;
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
              emitIntegerSwitch(e, sw, caseLabels, brkTarget, state);
            } else {
              assert(IS_STRING_TYPE(stype));
              emitStringSwitch(e, sw, caseLabels, brkTarget, state);
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
            e.Jmp(brkTarget);
          }
        }
        for (uint i = 0; i < ncase; i++) {
          caseLabels[i].set(e);
          CaseStatementPtr c(static_pointer_cast<CaseStatement>((*cases)[i]));
          enterRegion(region);
          SCOPE_EXIT { leaveRegion(region); };
          visit(c->getStatement());
        }
        if (brkTarget.isUsed()) brkTarget.set(e);
        if (contTarget.isUsed()) contTarget.set(e);
        if (!didSwitch && !simpleSubject) {
          // Null out temp local, to invoke any needed refcounting
          assert(tempLocal >= 0);
          assert(start != InvalidAbsoluteOffset);
          newFaultRegionAndFunclet(start, m_ue.bcPos(),
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
        auto region = createRegion(s, Region::Kind::Finally);
        enterRegion(region);
        SCOPE_EXIT { leaveRegion(region); };

        FinallyStatementPtr fs = static_pointer_cast<FinallyStatement>(node);
        visit(fs->getBody());
        return false;
      }

      case Statement::KindOfTryStatement: {
        auto region = createRegion(s, Region::Kind::TryFinally);
        if (!m_evalStack.empty()) {
          InvariantViolation(
            "Emitter detected that the evaluation stack is not empty "
            "at the beginning of a try region: %d", m_ue.bcPos());
        }

        TryStatementPtr ts = static_pointer_cast<TryStatement>(node);

        FinallyStatementPtr f(static_pointer_cast<FinallyStatement>
                              (ts->getFinally()));

        Offset start = m_ue.bcPos();
        Offset end;
        Label after;

        {
          if (f) {
            enterRegion(region);
          }
          SCOPE_EXIT {
            if (f) {
              leaveRegion(region);
            }
          };

          visit(ts->getBody());

          StatementListPtr catches = ts->getCatches();
          int catch_count = catches->getCount();
          if (catch_count > 0) {
            // include the jump out of the try-catch block in the
            // exception handler address range
            e.Jmp(after);
          }
          end = m_ue.bcPos();
          if (!m_evalStack.empty()) {
            InvariantViolation("Emitter detected that the evaluation stack "
                               "is not empty at the end of a try region: %d",
                               end);
          }

          if (catch_count > 0) {
            CatchRegion* r = new CatchRegion(start, end);
            m_catchRegions.push_back(r);

            bool firstHandler = true;
            for (int i = 0; i < catch_count; i++) {
              CatchStatementPtr c(static_pointer_cast<CatchStatement>
                                  ((*catches)[i]));
              StringData* eName = makeStaticString(c->getClassName());

              // If there's already a catch of this class, skip;
              // the first one wins
              if (r->m_names.find(eName) == r->m_names.end()) {
                // Don't let execution of the try body, or the
                // previous catch body,
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
          }
        }

        Offset end_catches = m_ue.bcPos();
        if (after.isUsed()) after.set(e);

        if (f) {
          region->m_finallyLabel.set(e);
          visit(f);
          emitFinallyEpilogue(e, region.get());
          auto func = getFunclet(f);
          if (func == nullptr) {
            auto thunklet =
              new FinallyThunklet(f, m_curFunc->numLiveIterators());
            func = addFunclet(f, thunklet);
          }
          newFaultRegion(start, end_catches, &func->m_entry);
        }

        return false;
      }

      case Statement::KindOfUnsetStatement: {
        ExpressionListPtr exps(
          static_pointer_cast<UnsetStatement>(node)->getExps());
        for (int i = 0, n = exps->getCount(); i < n; i++) {
          emitVisitAndUnset(e, (*exps)[i]);
        }
        return false;
      }

      case Statement::KindOfWhileStatement: {
        auto region = createRegion(s, Region::Kind::LoopOrSwitch);
        WhileStatementPtr ws(static_pointer_cast<WhileStatement>(s));
        Label& preCond = registerContinue(ws, region.get(), 1, false)->m_label;
        preCond.set(e);
        Label& fail = registerBreak(ws, region.get(), 1, false)->m_label;
        {
          Label tru;
          ExpressionPtr c(ws->getCondExp());
          Emitter condEmitter(c, m_ue, *this);
          visitIfCondition(c, condEmitter, tru, fail, true);
          if (tru.isUsed()) tru.set(e);
        }
        {
          enterRegion(region);
          SCOPE_EXIT { leaveRegion(region); };
          visit(ws->getBody());
        }
        e.Jmp(preCond);
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
        MethodStatementPtr m(static_pointer_cast<MethodStatement>(node));
        // Only called for fn defs not on the top level
        assert(!node->getClassScope()); // Handled directly by emitClass().
        StringData* nName = makeStaticString(m->getOriginalName());
        FuncEmitter* fe = m_ue.newFuncEmitter(nName);
        e.DefFunc(fe->id());
        postponeMeth(m, fe, false);
        return false;
      }

      case Statement::KindOfGotoStatement: {
        GotoStatementPtr g(static_pointer_cast<GotoStatement>(node));
        StringData* nName = makeStaticString(g->label());
        emitGoto(e, nName, g);
        return false;
      }

      case Statement::KindOfLabelStatement: {
        LabelStatementPtr l(static_pointer_cast<LabelStatement>(node));
        StringData* nName = makeStaticString(l->label());
        registerGoto(l, m_regions.back().get(), nName, false)
          ->m_label.set(e);
        return false;
      }
      case Statement::KindOfUseTraitStatement:
      case Statement::KindOfTraitRequireStatement:
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
                emitVisitAndUnset(e, (*exps)[i]);
              }
              e.Null();
              return true;
            }
          }
          emitVisitAndUnset(e, exp);
          e.Null();
          return true;
        }
        if (op == T_ARRAY) {
          int num_elems;
          std::vector<std::string> keys;
          if (u->isScalar()) {
            TypedValue tv;
            tvWriteUninit(&tv);
            initScalar(tv, u);
            if (m_staticArrays.empty()) {
              e.Array(tv.m_data.parr);
            }
          } else if (isPackedInit(u->getExpression(), &num_elems)) {
            // evaluate array values onto stack
            ExpressionListPtr el =
              static_pointer_cast<ExpressionList>(u->getExpression());
            for (int i = 0; i < num_elems; i++) {
              ArrayPairExpressionPtr ap =
                static_pointer_cast<ArrayPairExpression>((*el)[i]);
              visit(ap->getValue());
              emitConvertToCell(e);
            }
            e.NewPackedArray(num_elems);
          } else if (isStructInit(u->getExpression(), keys)) {
            ExpressionListPtr el =
              static_pointer_cast<ExpressionList>(u->getExpression());
            for (int i = 0, n = keys.size(); i < n; i++) {
              ArrayPairExpressionPtr ap =
                static_pointer_cast<ArrayPairExpression>((*el)[i]);
              visit(ap->getValue());
              emitConvertToCell(e);
            }
            e.NewStructArray(keys);
          } else {
            assert(m_staticArrays.empty());
            ExpressionPtr ex = u->getExpression();
            int capacityHint = -1;
            if (ex->getKindOf() == Expression::KindOfExpressionList) {
              ExpressionListPtr el(static_pointer_cast<ExpressionList>(ex));
              int capacity = el->getCount();
              if (capacity > 0) {
                capacityHint = capacity;
              }
            }
            if (capacityHint != -1) {
              e.NewArray(capacityHint);
            } else {
              e.NewArray(HphpArray::SmallSize);
            }
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
            makeStaticString("error_reporting");
          Offset fpiStart = m_ue.bcPos();
          e.FPushFuncD(1, s_error_reporting);
          {
            FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
            e.Int(0);
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
          assert(op == T_FILE || op == T_DIR);
        }
        switch (op) {
          case T_INC:
          case T_DEC: {
            auto const cop = [&] {
              if (op == T_INC) {
                if (RuntimeOption::IntsOverflowToInts) {
                  return u->getFront() ? IncDecOp::PreInc : IncDecOp::PostInc;
                }
                return u->getFront() ? IncDecOp::PreIncO : IncDecOp::PostIncO;
              }
              if (RuntimeOption::IntsOverflowToInts) {
                return u->getFront() ? IncDecOp::PreDec : IncDecOp::PostDec;
              }
              return u->getFront() ? IncDecOp::PreDecO : IncDecOp::PostDecO;
            }();
            emitIncDec(e, cop);
            break;
          }
          case T_EMPTY: emitEmpty(e); break;
          case T_CLONE: e.Clone(); break;
          case '+':
            RuntimeOption::IntsOverflowToInts ? e.Add() : e.AddO();
            break;
          case '-':
            RuntimeOption::IntsOverflowToInts ? e.Sub() : e.SubO();
            break;
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
            assert(oldErrorLevelLoc >= 0);
            assert(start != InvalidAbsoluteOffset);
            newFaultRegionAndFunclet(start, m_ue.bcPos(),
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
            assert(false);
        }
        return true;
      }

      case Expression::KindOfAssignmentExpression: {
        AssignmentExpressionPtr ae(
          static_pointer_cast<AssignmentExpression>(node));
        ExpressionPtr rhs = ae->getValue();
        Id tempLocal = -1;
        Offset start = InvalidAbsoluteOffset;

        if (ae->isRhsFirst()) {
          assert(!rhs->hasContext(Expression::RefValue));
          tempLocal = emitVisitAndSetUnnamedL(e, rhs);
          start = m_ue.bcPos();
        }

        visit(ae->getVariable());
        emitClsIfSPropBase(e);

        if (ae->isRhsFirst()) {
          emitPushAndFreeUnnamedL(e, tempLocal, start);
        } else {
          visit(rhs);
        }

        if (rhs->hasContext(Expression::RefValue)) {
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
                = makeStaticString("get_called_class");
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
              StringData* nLiteral = makeStaticString(s);
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
          int nElms = 0;
          ExpressionListPtr el;
          if (b->getExp2()) {
            el = static_pointer_cast<ExpressionList>(b->getExp2());
            nElms = el->getCount();
          }
          const std::string* clsName = nullptr;
          cls->getString(clsName);
          int cType = Collection::stringToType(*clsName);
          if (cType == Collection::PairType) {
            if (nElms != 2) {
              throw IncludeTimeFatalException(b,
                "Pair objects must have exactly 2 elements");
            }
          } else if (cType == Collection::InvalidType) {
            throw IncludeTimeFatalException(b,
              "Cannot use collection initialization for non-collection class");
          }
          bool kvPairs = (cType == Collection::MapType ||
                          cType == Collection::ImmMapType);
          e.NewCol(cType, nElms);
          if (kvPairs) {
            for (int i = 0; i < nElms; i++) {
              ArrayPairExpressionPtr ap(
                static_pointer_cast<ArrayPairExpression>((*el)[i]));
              ExpressionPtr key = ap->getName();
              if (!key) {
                throw IncludeTimeFatalException(ap,
                  "Keys must be specified for Map initialization");
              }
              visit(key);
              emitConvertToCell(e);
              visit(ap->getValue());
              emitConvertToCell(e);
              e.ColAddElemC();
            }
          } else {
            for (int i = 0; i < nElms; i++) {
              ArrayPairExpressionPtr ap(
                static_pointer_cast<ArrayPairExpression>((*el)[i]));
              ExpressionPtr key = ap->getName();
              if ((bool)key) {
                throw IncludeTimeFatalException(ap,
                  "Keys may not be specified for Vector, Set, or Pair "
                  "initialization");
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
          case '+':
            RuntimeOption::IntsOverflowToInts ? e.Add() : e.AddO();
            break;
          case '-':
            RuntimeOption::IntsOverflowToInts ? e.Sub() : e.SubO();
            break;
          case '*':
            RuntimeOption::IntsOverflowToInts ? e.Mul() : e.MulO();
            break;
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
          default: assert(false);
        }
        return true;
      }

      case Expression::KindOfClassConstantExpression: {
        ClassConstantExpressionPtr cc(
          static_pointer_cast<ClassConstantExpression>(node));
        auto const nName = makeStaticString(cc->getConName());
        auto const getOriginalClassName = [&] {
          const std::string& clsName = cc->getOriginalClassName();
          return makeStaticString(clsName);
        };

        // We treat ::class as a class constant in the AST and the
        // parser, but at the bytecode and runtime level it isn't
        // one.
        auto const emitClsCns = [&] {
          if (cc->isColonColonClass()) {
            e.NameA();
            return;
          }
          e.ClsCns(nName);
        };
        auto const noClassAllowed = [&] {
          auto const nCls = getOriginalClassName();
          std::ostringstream s;
          s << "Cannot access " << nCls->data() << "::" << nName->data() <<
               " when no class scope is active";
          throw IncludeTimeFatalException(e.getNode(), s.str().c_str());
        };

        if (cc->isStatic()) {
          // static::Constant
          e.LateBoundCls();
          emitClsCns();
        } else if (cc->getClass()) {
          // $x::Constant
          ExpressionPtr cls(cc->getClass());
          visit(cls);
          emitAGet(e);
          emitClsCns();
        } else if (cc->getOriginalClass() &&
                   !cc->getOriginalClass()->isTrait()) {
          // C::Constant inside a class
          auto nCls = getOriginalClassName();
          if (cc->isColonColonClass()) {
            e.String(nCls);
          } else {
            e.ClsCnsD(nName, nCls);
          }
        } else if (cc->isSelf()) {
          // self::Constant inside trait or pseudomain
          e.Self();
          if (cc->isColonColonClass() &&
              cc->getFunctionScope()->inPseudoMain()) {
            noClassAllowed();
          }
          emitClsCns();
        } else if (cc->isParent()) {
          // parent::Constant inside trait or pseudomain
          e.Parent();
          if (cc->isColonColonClass() &&
              cc->getFunctionScope()->inPseudoMain()) {
            noClassAllowed();
          }
          emitClsCns();
        } else {
          // C::Constant inside a trait or pseudomain
          // Be careful to keep this case here after the isSelf and
          // isParent cases because StaticClassName::resolveClass()
          // will set cc->originalClassName to the trait's name for
          // the isSelf and isParent cases, but self and parent must
          // be resolved dynamically when used inside of traits.
          auto nCls = getOriginalClassName();
          if (cc->isColonColonClass()) noClassAllowed();
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
          std::string nameStr = c->getOriginalName();
          StringData* nName = makeStaticString(nameStr);
          if (c->hadBackslash()) {
            e.CnsE(nName);
          } else {
            const std::string& nonNSName = c->getNonNSOriginalName();
            if (nonNSName != nameStr) {
              StringData* nsName = nName;
              nName = makeStaticString(nonNSName);
              e.CnsU(nsName, nName);
            } else {
              e.Cns(makeStaticString(c->getName()));
            }
          }
        }
        return true;
      }

      case Expression::KindOfEncapsListExpression: {
        EncapsListExpressionPtr el(
          static_pointer_cast<EncapsListExpression>(node));
        ExpressionListPtr args(el->getExpressions());
        int n = args ? args->getCount() : 0;
        int i = 0;
        FPIRegionRecorder* fpi = nullptr;
        if (el->getType() == '`') {
          const static StringData* s_shell_exec =
            makeStaticString("shell_exec");
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
              makeStaticString(v.toCStrRef().get()));
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

        if (call->isFatalFunction()) {
          if (params && params->getCount() == 1) {
            ExpressionPtr p = (*params)[0];
            Variant v;
            if (p->getScalarValue(v)) {
              assert(v.isString());
              StringData* msg = makeStaticString(v.toString());
              auto exn = IncludeTimeFatalException(call, "%s", msg->data());
              exn.setParseFatal(call->isParseFatalFunction());
              throw exn;
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
            call->changeToBytecode();
            e.AKExists();
            return true;
          }
        } else if (call->isCallToFunction("idx") &&
                   call->isOptimizable() &&
                   !Option::JitEnableRenameFunction) {
          if (params && (params->getCount() == 2 || params->getCount() == 3)) {
            visit((*params)[0]);
            emitConvertToCell(e);
            visit((*params)[1]);
            emitConvertToCell(e);
            if (params->getCount() == 2) {
              e.Null();
            } else {
              visit((*params)[2]);
              emitConvertToCell(e);
            }
            call->changeToBytecode();
            e.Idx();
            return true;
          }
        } else if (call->isCallToFunction("hphp_array_idx")) {
          if (params && params->getCount() == 3) {
            visit((*params)[0]);
            emitConvertToCell(e);
            visit((*params)[1]);
            emitConvertToCell(e);
            visit((*params)[2]);
            emitConvertToCell(e);
            call->changeToBytecode();
            e.ArrayIdx();
            return true;
          }
        } else if (call->isCallToFunction("strlen")) {
          if (params && params->getCount() == 1) {
            visit((*params)[0]);
            emitConvertToCell(e);
            call->changeToBytecode();
            e.Strlen();
            return true;
          }
        } else if (call->isCallToFunction("floor")) {
          if (params && params->getCount() == 1) {
            visit((*params)[0]);
            emitConvertToCell(e);
            call->changeToBytecode();
            e.Floor();
            return true;
          }
        } else if (call->isCallToFunction("ceil")) {
          if (params && params->getCount() == 1) {
            visit((*params)[0]);
            emitConvertToCell(e);
            call->changeToBytecode();
            e.Ceil();
            return true;
          }
        } else if (call->isCallToFunction("sqrt")) {
          if (params && params->getCount() == 1) {
            visit((*params)[0]);
            emitConvertToCell(e);
            call->changeToBytecode();
            e.Sqrt();
            return true;
          }
        } else if (call->isCallToFunction("define")) {
          if (params && params->getCount() == 2) {
            ExpressionPtr p0 = (*params)[0];
            Variant v0;
            if (p0->getScalarValue(v0) && v0.isString()) {
              const StringData* cname =
                makeStaticString(v0.toString());
              visit((*params)[1]);
              emitConvertToCell(e);
              e.DefCns(cname);
              return true;
            }
          }
        } else if (call->isCallToFunction("func_num_args") && m_inGenerator) {
          static const StringData* s_count =
            makeStaticString("count");

          emitVirtualLocal(m_curFunc->lookupVarId(s_continuationVarArgsLocal));
          emitConvertToCell(e);
          e.False();
          e.FCallBuiltin(2, 1, s_count);
          e.UnboxRNop();
          return true;
        } else if (call->isCallToFunction("func_get_args") && m_inGenerator) {
          emitVirtualLocal(m_curFunc->lookupVarId(s_continuationVarArgsLocal));
          emitConvertToCell(e);
          return true;
        } else if (call->isCallToFunction("func_get_arg") && m_inGenerator) {
          if (!params || params->getCount() == 0) {
            e.Null();
            return true;
          }

          emitVirtualLocal(m_curFunc->lookupVarId(s_continuationVarArgsLocal));
          emitConvertToCell(e);
          visit((*params)[0]);
          emitConvertToCell(e);
          e.CastInt();
          e.False();
          e.ArrayIdx();
          return true;
        } else if (call->isCallToFunction("array_slice") && !m_inGenerator &&
                   params && params->getCount() == 2 &&
                   !Option::JitEnableRenameFunction) {
          ExpressionPtr p0 = (*params)[0];
          ExpressionPtr p1 = (*params)[1];
          Variant v1;
          if (p0->getKindOf() == Expression::KindOfSimpleFunctionCall &&
              p1->getScalarValue(v1) && v1.isInteger()) {
            SimpleFunctionCallPtr innerCall(
              static_pointer_cast<SimpleFunctionCall>(p0));
            ExpressionListPtr innerParams = innerCall->getParams();
            if (innerCall->isCallToFunction("func_get_args") &&
                (!innerParams || innerParams->getCount() == 0)) {
              params->removeElement(0);
              emitFuncCall(e, innerCall,
                           "__SystemLib\\func_slice_args", params);
              return true;
            }
          }
          // fall through
        } else if (call->isCallToFunction("abs")) {
          if (params && params->getCount() == 1) {
            visit((*params)[0]);
            emitConvertToCell(e);
            call->changeToBytecode();
            e.Abs();
            return true;
          }
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
            assert(call->isCallToFunction("trait_exists"));
            e.TraitExists();
          }
          return true;
        }
#define TYPE_CONVERT_INSTR(what, What)                                 \
        else if (call->isCallToFunction(#what"val") &&                 \
                 params && params->getCount() == 1) {                  \
          visit((*params)[0]);                                         \
          emitConvertToCell(e);                                        \
          e.Cast ## What();                                            \
          return true;                                                 \
        }
      TYPE_CONVERT_INSTR(bool, Bool)
      TYPE_CONVERT_INSTR(int, Int)
      TYPE_CONVERT_INSTR(double, Double)
      TYPE_CONVERT_INSTR(float, Double)
      TYPE_CONVERT_INSTR(str, String)
#undef TYPE_CONVERT_INSTR

#define TYPE_CHECK_INSTR(what, What)                    \
        else if (call->isCallToFunction("is_"#what) &&  \
                 params && params->getCount() == 1) {   \
          visit((*call->getParams())[0]);               \
          emitIsType(e, IsTypeOp::What);                \
          return true;                                  \
        }

      TYPE_CHECK_INSTR(null, Null)
      TYPE_CHECK_INSTR(object, Obj)
      TYPE_CHECK_INSTR(array, Arr)
      TYPE_CHECK_INSTR(string, Str)
      TYPE_CHECK_INSTR(int, Int)
      TYPE_CHECK_INSTR(integer, Int)
      TYPE_CHECK_INSTR(long, Int)
      TYPE_CHECK_INSTR(bool, Bool)
      TYPE_CHECK_INSTR(double, Dbl)
      TYPE_CHECK_INSTR(real, Dbl)
      TYPE_CHECK_INSTR(float, Dbl)
      TYPE_CHECK_INSTR(scalar, Scalar)
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
          StringData* nValue = makeStaticString(ie->includePath());
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
              e.ReqDoc();
            } else {
              e.ReqOnce();
            }
            break;
        }
        return true;
      }

      case Expression::KindOfListAssignment: {
        ListAssignmentPtr la(static_pointer_cast<ListAssignment>(node));
        ExpressionPtr rhs = la->getArray();

        // visitListAssignmentLHS should have handled this
        assert(rhs);

        bool nullRHS = la->getRHSKind() == ListAssignment::Null;
        // Assign RHS to temp local, unless it's already a simple variable
        bool simpleRHS = rhs->is(Expression::KindOfSimpleVariable)
          && !static_pointer_cast<SimpleVariable>(rhs)->getAlwaysStash();
        Id tempLocal = -1;
        Offset start = InvalidAbsoluteOffset;

        if (!simpleRHS && la->isRhsFirst()) {
          tempLocal = emitVisitAndSetUnnamedL(e, rhs);
          start = m_ue.bcPos();
        }

        // We use "index chains" to deal with nested list assignment.  We will
        // end up with one index chain per expression we need to assign to.
        // The helper function will populate indexChains.
        std::vector<IndexChain*> indexChains;
        IndexChain workingChain;
        visitListAssignmentLHS(e, la, workingChain, indexChains);

        if (!simpleRHS && !la->isRhsFirst()) {
          assert(tempLocal == -1);
          assert(start == InvalidAbsoluteOffset);
          tempLocal = emitVisitAndSetUnnamedL(e, rhs);
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
              m_evalStack.push(StackSym::I);
              m_evalStack.setInt((*currIndexChain)[j]);
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
          emitPushAndFreeUnnamedL(e, tempLocal, start);
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
                       makeStaticString(ne->getOriginalClassName()));
        }

        {
          FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
          for (int i = 0; i < numParams; i++) {
            emitFuncCallArg(e, (*params)[i], i);
          }
        }

        e.FCall(numParams);
        bool inferred = false;
        if (Option::WholeProgram && Option::GenerateInferredTypes) {
          FunctionScopePtr fs = ne->getFuncScope();
          if (fs && !fs->getReturnType()) {
            m_evalStack.setKnownType(KindOfNull, false /* it's inferred */);
            m_evalStack.setNotRef();
            inferred = true;
          }
        }
        if (!inferred) {
          m_evalStack.setKnownType(KindOfNull, true /* it's predicted */);
          m_evalStack.setNotRef();
        }
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

        Offset fpiStart = 0;
        ExpressionPtr methName = om->getNameExp();
        bool useDirectForm = false;
        if (methName->is(Expression::KindOfScalarExpression)) {
          ScalarExpressionPtr sval(
            static_pointer_cast<ScalarExpression>(methName));
          const std::string& methStr = sval->getOriginalLiteralString();
          if (!methStr.empty()) {
            // $obj->name(...)
            //       ^^^^
            // Use getOriginalLiteralString(), which hasn't been
            // case-normalized, since __call() needs to preserve
            // the case.
            StringData* nameLiteral = makeStaticString(methStr);
            fpiStart = m_ue.bcPos();
            e.FPushObjMethodD(numParams, nameLiteral);
            useDirectForm = true;
          }
        }
        if (!useDirectForm) {
          // $obj->{...}(...)
          //       ^^^^^
          visit(methName);
          emitConvertToCell(e);
          fpiStart = m_ue.bcPos();
          e.FPushObjMethod(numParams);
        }
        if (clsName) {
          Id id = m_ue.mergeLitstr(clsName);
          m_metaInfo.add(fpiStart, Unit::MetaInfo::Kind::Class, false,
                         useDirectForm ? 0 : 1, id);
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
            m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Kind::GuardedThis,
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
            if (scalarExp->getType() == T_CLASS_C &&
                ex->getFunctionScope()->getContainingClass() &&
                ex->getFunctionScope()->getContainingClass()->isTrait()) {
              static const StringData* fname =
                makeStaticString("get_class");
              Offset fpiStart = m_ue.bcPos();
              e.FPushFuncD(0, fname);
              {
                FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
              }
              e.FCall(0);
              e.UnboxR();
            } else {
              StringData* nValue = makeStaticString(v.getStringData());
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
            assert(false);
        }
        return true;
      }

      case Expression::KindOfSimpleVariable: {
        SimpleVariablePtr sv(static_pointer_cast<SimpleVariable>(node));
        if (sv->isThis()) {
          if (sv->hasContext(Expression::ObjectContext)) {
            if (sv->isGuarded()) {
              m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Kind::GuardedThis,
                             false, 0, 0);
            }
            e.This();
          } else if (sv->getFunctionScope()->needsLocalThis()) {
            static const StringData* thisStr = makeStaticString("this");
            Id thisId = m_curFunc->lookupVarId(thisStr);
            emitVirtualLocal(thisId);
          } else {
            if (sv->isGuarded()) {
              m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Kind::GuardedThis,
                             false, 0, 0);
              e.This();
            } else {
              auto const subop = sv->hasContext(Expression::ExistContext)
                ? BareThisOp::NoNotice
                : BareThisOp::Notice;
              e.BareThis(subop);
            }
          }
        } else {
          StringData* nLiteral = makeStaticString(sv->getName());
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
        if (!m_staticArrays.empty()) {
          ExpressionPtr val = ap->getValue();

          TypedValue tvVal;
          initScalar(tvVal, val);

          if (key != nullptr) {
            assert(key->isScalar());
            TypedValue tvKey = make_tv<KindOfNull>();
            if (!key->getScalarValue(tvAsVariant(&tvKey))) {
              InvariantViolation("Expected scalar value for array key\n");
              always_assert(0);
            }
            m_staticArrays.back().set(tvAsCVarRef(&tvKey),
                                      tvAsVariant(&tvVal));
          } else {
            m_staticArrays.back().append(tvAsCVarRef(&tvVal));
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
            StringData* varName = makeStaticString(var->getName());
            useVars.push_back(ClosureUseVar(varName, var->isRef()));
          }
        }

        // We're still at the closure definition site. Emit code to instantiate
        // the new anonymous class, with the use variables as arguments.
        ExpressionListPtr valuesList(ce->getClosureValues());
        for (int i = 0; i < useCount; ++i) {
          emitBuiltinCallArg(e, (*valuesList)[i], i, useVars[i].second);
        }

        if (m_curFunc->isAsync() && m_inGenerator) {
          // Closure definition in the body of async function. The closure
          // body was already emitted, so we just create the object here.
          assert(ce->getClosureClassName());
          e.CreateCl(useCount, ce->getClosureClassName());
          return true;
        }

        // The parser generated a unique name for the function,
        // use that for the class
        std::string clsName = ce->getClosureFunction()->getOriginalName();

        if (m_curFunc->isPseudoMain()) {
          std::ostringstream oss;
          oss << clsName << '$' << std::hex <<
            m_curFunc->ue().md5().q[1] << m_curFunc->ue().md5().q[0] << '$';
          clsName = oss.str();
        }

        if (Option::WholeProgram) {
          int my_id;
          {
            EmittedClosures::accessor acc;
            s_emittedClosures.insert(acc, makeStaticString(clsName));
            my_id = ++acc->second;
          }
          if (my_id > 1) {
            // The closure was from a trait, so we need a unique name in the
            // implementing class. _ is different from the #, which is used for
            // many closures in the same func in ParserBase::newClosureName
            folly::toAppend('_', my_id, &clsName);
          }
        }

        ce->setClosureClassName(makeStaticString(clsName));
        e.CreateCl(useCount, ce->getClosureClassName());

        // From here on out, we're creating a new class to hold the closure.
        const static StringData* parentName = makeStaticString("Closure");
        const Location* sLoc = ce->getLocation().get();
        PreClassEmitter* pce = m_ue.newPreClassEmitter(
          ce->getClosureClassName(), PreClass::ClosureHoistable);
        pce->init(sLoc->line0, sLoc->line1, m_ue.bcPos(),
                  AttrUnique | AttrPersistent, parentName, nullptr);

        // Instance properties---one for each use var, and one for
        // each static local.
        TypedValue uninit;
        tvWriteUninit(&uninit);
        for (auto& useVar : useVars) {
          pce->addProperty(useVar.first, AttrPrivate, nullptr, nullptr,
                           &uninit, RepoAuthType{});
        }

        // The __invoke method. This is the body of the closure, preceded by
        // code that pulls the object's instance variables into locals.
        static const StringData* invokeName = makeStaticString("__invoke");
        FuncEmitter* invoke = m_ue.newMethodEmitter(invokeName, pce);
        invoke->setIsClosureBody(true);
        pce->addMethod(invoke);
        MethodStatementPtr body(
          static_pointer_cast<MethodStatement>(ce->getClosureFunction()));
        postponeMeth(body, invoke, false, new ClosureUseVarVec(useVars));

        return true;
      }
      case Expression::KindOfYieldExpression: {
        YieldExpressionPtr y(static_pointer_cast<YieldExpression>(node));

        registerYieldAwait(y);
        assert(m_evalStack.size() == 0);

        // evaluate key passed to yield, if applicable
        ExpressionPtr keyExp = y->getKeyExpression();
        if (keyExp) {
          m_curFunc->setIsPairGenerator(true);
          visit(keyExp);
          emitConvertToCell(e);
        }

        // evaluate value expression passed to yield
        visit(y->getValueExpression());
        emitConvertToCell(e);

        // suspend continuation
        if (keyExp) {
          assert(m_evalStack.size() == 2);
          e.ContSuspendK();
        } else {
          assert(m_evalStack.size() == 1);
          e.ContSuspend();
        }

        // continue with the received result on the stack
        assert(m_evalStack.size() == 1);
        return true;
      }
      case Expression::KindOfAwaitExpression: {
        AwaitExpressionPtr await(static_pointer_cast<AwaitExpression>(node));

        registerYieldAwait(await);

        assert(m_evalStack.size() == 0);

        // If we know statically that it's a subtype of WaitHandle, we
        // don't need to make a call.
        bool const isKnownWaitHandle = [&] {
          auto const expr = await->getExpression();
          auto const ar = expr->getScope()->getContainingProgram();
          auto const type = expr->getActualType();
          return type && Type::SubType(ar, type,
                           Type::GetType(Type::KindOfObject, "WaitHandle"));
        }();

        Label resume;

        // evaluate expression passed to await
        visit(await->getExpression());
        emitConvertToCell(e);

        // if expr is null, just continue
        e.Dup();
        emitIsType(e, IsTypeOp::Null);
        e.JmpNZ(resume);

        if (!isKnownWaitHandle) {
          emitConstMethodCallNoParams(e, "getWaitHandle");
        }
        assert(m_evalStack.size() == 1);

        // TODO(#3197024): if isKnownWaitHandle, we should put an
        // AssertObjStk so the AsyncAwait type check can be avoided.
        e.AsyncAwait();
        e.JmpNZ(resume);

        // Suspend if it is not finished.
        if (m_inGenerator) {
          // Suspend resumed execution.
          e.ContSuspend();
          m_awaitLabels[await].set(e);
          e.AsyncResume();
        } else {
          // Suspend eagerly executed async function.
          e.AsyncESuspend(m_awaitLabels[await], m_pendingIters.size());
          e.RetC();
        }

        resume.set(e);
        return true;
      }
      case Expression::KindOfQueryExpression: {
        QueryExpressionPtr query(static_pointer_cast<QueryExpression>(node));
        auto args = *query->getQueryArguments();
        auto numArgs = args.getCount();
        visit(args[0]);
        emitConvertToCell(e);
        auto fpiStart = m_ue.bcPos();
        StringData* executeQuery = makeStaticString("executeQuery");
        e.FPushObjMethodD(numArgs+1, executeQuery);
        {
          FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
          e.String(query->getQueryString());
          emitFPass(e, 0, PassByRefKind::ErrorOnCell);
          auto selectCallback = query->getSelectClosure();
          if (selectCallback != nullptr) {
            visit(selectCallback);
            emitConvertToCell(e);
          } else {
            e.Null();
          }
          emitFPass(e, 1, PassByRefKind::ErrorOnCell);
          for (int i = 1; i < numArgs; i++) {
            visit(args[i]);
            emitConvertToCell(e);
            emitFPass(e, i+1, PassByRefKind::ErrorOnCell);
          }
        }
        e.FCall(numArgs+1);
        e.UnboxR();
        return true;
      }
      case Expression::KindOfFromClause:
      case Expression::KindOfLetClause:
      case Expression::KindOfWhereClause:
      case Expression::KindOfSelectClause:
      case Expression::KindOfIntoClause:
      case Expression::KindOfJoinClause:
      case Expression::KindOfGroupClause:
      case Expression::KindOfOrderbyClause:
      case Expression::KindOfOrdering: {
        not_reached();
      }
    }
  }

  not_reached();
}

void EmitterVisitor::emitConstMethodCallNoParams(Emitter& e, string name) {
  auto const nameLit = makeStaticString(name);
  auto const fpiStart = m_ue.bcPos();
  e.FPushObjMethodD(0, nameLit);
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
  }
  e.FCall(0);
  emitConvertToCell(e);
}

int EmitterVisitor::scanStackForLocation(int iLast) {
  assert(iLast >= 0);
  assert(iLast < (int)m_evalStack.size());
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
  assert(iFirst >= 0);
  assert(iFirst <= iLast);
  assert(iLast < (int)m_evalStack.size());
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
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Kind::Class, true, 0, id);
    }
    switch (marker) {
      case StackSym::N: {
        if (symFlavor == StackSym::C) {
          vectorImm.push_back(LNC);
        } else if (symFlavor == StackSym::L) {
          vectorImm.push_back(LNL);
        } else {
          assert(false);
        }
      } break;
      case StackSym::G: {
        if (symFlavor == StackSym::C) {
          vectorImm.push_back(LGC);
        } else if (symFlavor == StackSym::L) {
          vectorImm.push_back(LGL);
        } else {
          assert(false);
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
    }
    if (const StringData* cls = m_evalStack.getClsName(i)) {
      const int mcodeNum = i - (iFirst + 1);
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Kind::MVecPropClass,
                     false, mcodeNum, m_ue.mergeLitstr(cls));
    }
    m_metaInfo.addKnownDataType(m_evalStack.getKnownType(i),
                                m_evalStack.isTypePredicted(i),
                                m_ue.bcPos(), true, i - iFirst);

    switch (marker) {
      case StackSym::M: {
        assert(symFlavor == StackSym::A);
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
        assert(false);
      }
      default: assert(false); break;
    }

    if (symFlavor == StackSym::L) {
      encodeIvaToVector(vectorImm, m_evalStack.getLoc(i));
    } else if (symFlavor == StackSym::T) {
      assert(strid != -1);
      encodeToVector<int32_t>(vectorImm, strid);
    } else if (symFlavor == StackSym::I) {
      encodeToVector<int64_t>(vectorImm, m_evalStack.getInt(i));
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
  assert(sz >= 0);
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
  assert(m_evalStack.size() >= 2);
  assert(m_evalStack.sizeActual() >= 1);
  assert(StackSym::GetSymFlavor(m_evalStack.get(m_evalStack.size() - 2))
    == StackSym::L);
  int localIdx = m_evalStack.getLoc(m_evalStack.size() - 2);
  e.CGetL2(localIdx);
}

void EmitterVisitor::emitCGetL3(Emitter& e) {
  assert(m_evalStack.size() >= 3);
  assert(m_evalStack.sizeActual() >= 2);
  assert(StackSym::GetSymFlavor(m_evalStack.get(m_evalStack.size() - 3))
    == StackSym::L);
  int localIdx = m_evalStack.getLoc(m_evalStack.size() - 3);
  e.CGetL3(localIdx);
}

void EmitterVisitor::emitPushL(Emitter& e) {
  assert(m_evalStack.size() >= 1);
  auto const idx = m_evalStack.size() - 1;
  assert(StackSym::GetSymFlavor(m_evalStack.get(idx)) == StackSym::L);
  e.PushL(m_evalStack.getLoc(idx));
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
  assert(sz >= 0);
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
  assert(sz >= 0);
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

Id EmitterVisitor::emitVisitAndSetUnnamedL(Emitter& e, ExpressionPtr exp) {
  visit(exp);
  emitConvertToCell(e);

  return emitSetUnnamedL(e);
}

Id EmitterVisitor::emitSetUnnamedL(Emitter& e) {
  // HACK: emitVirtualLocal would pollute m_evalStack before visiting exp,
  //       YieldExpression won't be happy
  Id tempLocal = m_curFunc->allocUnnamedLocal();
  auto& ue = e.getUnitEmitter();
  ue.emitOp(OpSetL);
  ue.emitIVA(tempLocal);

  emitPop(e);
  return tempLocal;
}

void EmitterVisitor::emitPushAndFreeUnnamedL(Emitter& e, Id tempLocal,
                                             Offset start) {
  assert(tempLocal >= 0);
  assert(start != InvalidAbsoluteOffset);
  newFaultRegionAndFunclet(start, m_ue.bcPos(),
                           new UnsetUnnamedLocalThunklet(tempLocal));
  emitVirtualLocal(tempLocal);
  emitPushL(e);
  m_curFunc->freeUnnamedLocal(tempLocal);
}

EmitterVisitor::PassByRefKind
EmitterVisitor::getPassByRefKind(ExpressionPtr exp) {
  auto permissiveKind = PassByRefKind::AllowCell;

  // The PassByRefKind of a list assignment expression is determined
  // by the PassByRefKind of the RHS. This loop will repeatedly recurse
  // on the RHS until it encounters an expression other than a list
  // assignment expression.
  while (exp->is(Expression::KindOfListAssignment)) {
    ListAssignmentPtr la(static_pointer_cast<ListAssignment>(exp));
    exp = la->getArray();
    permissiveKind = PassByRefKind::WarnOnCell;
  }

  // this only happens for calls that have been morphed into bytecode
  // e.g. idx(), abs(), strlen(), etc..
  // It is to allow the following code to work
  // function f(&$arg) {...}
  // f(idx($array, 'key')); <- this fails otherwise
  if (exp->allowCellByRef()) {
    return PassByRefKind::AllowCell;
  }

  switch (exp->getKindOf()) {
    case Expression::KindOfNewObjectExpression:
    case Expression::KindOfIncludeExpression:
    case Expression::KindOfSimpleVariable:
      // New and include/require
      return PassByRefKind::AllowCell;
    case Expression::KindOfArrayElementExpression:
      // Allow if bare; warn if inside list assignment
      return permissiveKind;
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
  if (checkIfStackEmpty("Builtin arg*")) return;
  if (byRef) {
    emitVGet(e);
  } else {
    emitCGet(e);
  }
  return;
}

void EmitterVisitor::emitBuiltinDefaultArg(Emitter& e, Variant& v,
                                           DataType t, int paramId) {
  switch (v.getType()) {
    case KindOfString:
    case KindOfStaticString: {
      StringData *nValue = makeStaticString(v.getStringData());
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
        case KindOfResource:
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
    case KindOfArray:
      e.Array(v.getArrayData());
      break;
    default:
      not_reached();
  }
}

void EmitterVisitor::emitFuncCallArg(Emitter& e,
                                     ExpressionPtr exp,
                                     int paramId) {
  visit(exp);
  if (checkIfStackEmpty("FPass*")) return;
  PassByRefKind passByRefKind = getPassByRefKind(exp);
  if (Option::WholeProgram && !exp->hasAnyContext(Expression::InvokeArgument |
                                                  Expression::RefParameter)) {
    if (exp->hasContext(Expression::RefValue)) {
      if (passByRefKind == PassByRefKind::AllowCell ||
          m_evalStack.get(m_evalStack.size() - 1) != StackSym::C) {
        emitVGet(e);
        e.FPassVNop(paramId);
        return;
      }
    } else {
      emitCGet(e);
      e.FPassC(paramId);
      return;
    }
  }
  emitFPass(e, paramId, getPassByRefKind(exp));
}

void EmitterVisitor::emitFPass(Emitter& e, int paramId,
                               PassByRefKind passByRefKind) {
  if (checkIfStackEmpty("FPass*")) return;
  LocationGuard locGuard(e, m_tempLoc);
  m_tempLoc.reset();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.FPassL(paramId, m_evalStack.getLoc(i)); break;
      case StackSym::C:
        switch (passByRefKind) {
          case PassByRefKind::AllowCell:   e.FPassC(paramId); break;
          case PassByRefKind::WarnOnCell:  e.FPassCW(paramId); break;
          case PassByRefKind::ErrorOnCell: e.FPassCE(paramId); break;
          default: assert(false);
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
  assert(sz >= 0);
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
      case StackSym::C:
        e.IsTypeC(IsTypeOp::Null);
        e.Not();
        break;
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

void EmitterVisitor::emitIsType(Emitter& e, IsTypeOp op) {
  if (checkIfStackEmpty("IsType")) return;

  emitConvertToCellOrLoc(e);
  switch (char sym = m_evalStack.top()) {
  case StackSym::L:
    e.IsTypeL(m_evalStack.getLoc(m_evalStack.size() - 1), op);
    break;
  case StackSym::C:
    e.IsTypeC(op);
    break;
  default:
    unexpectedStackSym(sym, "emitIsType");
  }
}

void EmitterVisitor::emitEmpty(Emitter& e) {
  if (checkIfStackEmpty("Empty*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
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

void EmitterVisitor::emitUnset(Emitter& e,
                               ExpressionPtr exp /* = ExpressionPtr() */) {
  if (checkIfStackEmpty("Unset*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.UnsetL(m_evalStack.getLoc(i)); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.UnsetN(); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.UnsetG(); break;
      case StackSym::LS: // fall through
      case StackSym::CS: {
        assert(exp);

        std::ostringstream s;
        s << "Attempt to unset static property " << exp->getText();
        emitMakeUnitFatal(e, s.str().c_str());
        break;
      }
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

void EmitterVisitor::emitVisitAndUnset(Emitter& e, ExpressionPtr exp) {
  visit(exp);
  emitUnset(e, exp);
}

void EmitterVisitor::emitSet(Emitter& e) {
  if (checkIfStackEmpty("Set*")) return;

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
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

void EmitterVisitor::emitSetOp(Emitter& e, int tokenOp) {
  if (checkIfStackEmpty("SetOp*")) return;

  auto ifIntOverflow = [](SetOpOp trueVal, SetOpOp falseVal) {
    return RuntimeOption::IntsOverflowToInts ? trueVal : falseVal;
  };

  auto const op = [&] {
    switch (tokenOp) {
    case T_PLUS_EQUAL:
      return ifIntOverflow(SetOpOp::PlusEqual, SetOpOp::PlusEqualO);
    case T_MINUS_EQUAL:
      return ifIntOverflow(SetOpOp::MinusEqual, SetOpOp::MinusEqualO);
    case T_MUL_EQUAL:
      return ifIntOverflow(SetOpOp::MulEqual, SetOpOp::MulEqualO);
    case T_DIV_EQUAL:    return SetOpOp::DivEqual;
    case T_CONCAT_EQUAL: return SetOpOp::ConcatEqual;
    case T_MOD_EQUAL:    return SetOpOp::ModEqual;
    case T_AND_EQUAL:    return SetOpOp::AndEqual;
    case T_OR_EQUAL:     return SetOpOp::OrEqual;
    case T_XOR_EQUAL:    return SetOpOp::XorEqual;
    case T_SL_EQUAL:     return SetOpOp::SlEqual;
    case T_SR_EQUAL:     return SetOpOp::SrEqual;
    default:             break;
    }
    not_reached();
  }();

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.SetOpL(m_evalStack.getLoc(i), op); break;
      case StackSym::LN: emitCGetL2(e); // fall through
      case StackSym::CN: e.SetOpN(op); break;
      case StackSym::LG: emitCGetL2(e); // fall through
      case StackSym::CG: e.SetOpG(op); break;
      case StackSym::LS: emitCGetL3(e); // fall through
      case StackSym::CS: e.SetOpS(op); break;
      default: {
        unexpectedStackSym(sym, "emitSetOp");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.SetOpM(op, vectorImm);
  }
}

void EmitterVisitor::emitBind(Emitter& e) {
  if (checkIfStackEmpty("Bind*")) return;

  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
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

void EmitterVisitor::emitIncDec(Emitter& e, IncDecOp op) {
  if (checkIfStackEmpty("IncDec*")) return;

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L: e.IncDecL(m_evalStack.getLoc(i), op); break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.IncDecN(op); break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.IncDecG(op); break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.IncDecS(op); break;
      default: {
        unexpectedStackSym(sym, "emitIncDec");
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.IncDecM(op, vectorImm);
  }
}

void EmitterVisitor::emitConvertToCell(Emitter& e) {
  emitCGet(e);
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
    newFaultRegionAndFunclet(m_evalStack.getUnnamedLocStart(pos),
                             m_ue.bcPos(),
                             new UnsetUnnamedLocalThunklet(loc));
    m_curFunc->freeUnnamedLocal(loc);
    break;
  }
  case SymbolicStack::CLS_INVALID:
  default:
    assert(false);
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
      int64_t n;
      bool isNonZero;
      if (t == KindOfInt64) {
        n = cval.asInt64Val();
        isNonZero = n;
      } else {
        always_assert(t == KindOfString);
        n = m_ue.mergeLitstr(cval.asStrRef().get());
        isNonZero = false; // not used for string switches
      }
      if (!caseMap.count(n)) {
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
    int64_t base = caseMap.begin()->first;
    int64_t nTargets = caseMap.rbegin()->first - base + 1;
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
  int64_t base = caseMap.begin()->first;
  int64_t nTargets = caseMap.rbegin()->first - base + 1;

  // It's on. Map case values to Labels, filling in the blanks as
  // appropriate.
  Label* defLabel = state.defI == -1 ? &done : &caseLabels[state.defI];
  std::vector<Label*> labels(nTargets + 2);
  for (int i = 0; i < nTargets; ++i) {
    if (auto const caseIdx = folly::get_ptr(caseMap, base + i)) {
      labels[i] = &caseLabels[*caseIdx];
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
    StringData* nLiteral = makeStaticString(v.toCStrRef().get());
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

/*
 * <<__HipHopSpecific>> user attribute marks funcs/methods as HipHop specific
 * for reflection.
 * <<__IsFoldable>> Function has no side-effects and may be called at
 * compile time with constant input to get deterministic output.
 */
const StaticString
  s_HipHopSpecific("__HipHopSpecific"),
  s_IsFoldable("__IsFoldable");

static void parseUserAttributes(FuncEmitter* fe, Attr& attrs) {
  if (fe->hasUserAttribute(s_HipHopSpecific.get())) {
    attrs = attrs | AttrHPHPSpecific;
  }
  if (fe->hasUserAttribute(s_IsFoldable.get())) {
    attrs = attrs | AttrIsFoldable;
  }
}

static Attr buildMethodAttrs(MethodStatementPtr meth, FuncEmitter* fe,
                             bool top, bool allowOverride) {
  FunctionScopePtr funcScope = meth->getFunctionScope();
  ModifierExpressionPtr mod(meth->getModifiers());
  Attr attrs = buildAttrs(mod, meth->isRef());

  if (allowOverride) {
    attrs = attrs | AttrAllowOverride;
  }

  // if hasCallToGetArgs() or if mayUseVV
  if (meth->hasCallToGetArgs() || funcScope->mayUseVV()) {
    attrs = attrs | AttrMayUseVV;
  }

  auto fullName = meth->getOriginalFullName();
  auto it = Option::FunctionSections.find(fullName);
  if ((it != Option::FunctionSections.end() && it->second == "hot") ||
      (RuntimeOption::EvalRandomHotFuncs &&
       (hash_string_i(fullName.c_str()) & 8))) {
    attrs = attrs | AttrHot;
  }

  if (Option::WholeProgram) {
    if (!funcScope->isRedeclaring()) {
      attrs = attrs | AttrUnique;
      if (top &&
          (!funcScope->isVolatile() ||
           funcScope->isPersistent())) {
        attrs = attrs | AttrPersistent;
      }
    }
    if (meth->getClassScope() && !funcScope->hasOverride()) {
      attrs = attrs | AttrNoOverride;
    }
    if (funcScope->isSystem()) {
      assert((attrs & AttrPersistent) || meth->getClassScope());
      attrs = attrs | AttrBuiltin;
    }
  } else if (!SystemLib::s_inited) {
    // we're building systemlib. everything is unique
    attrs = attrs | AttrBuiltin | AttrUnique | AttrPersistent;
  }

  // For closures, the MethodStatement didn't have real attributes; enforce
  // that the __invoke method is public here
  if (fe->isClosureBody()) {
    assert(!(attrs & (AttrProtected | AttrPrivate)));
    attrs = attrs | AttrPublic;
  }

  parseUserAttributes(fe, attrs);
  return attrs;
}

/**
 * The code below is used for both, function/method parameter type as well as
 * for function/method return type.
 */
static TypeConstraint
determine_type_constraint_from_annot(const TypeAnnotationPtr annot,
                                     bool is_return) {
  if (annot) {
    auto flags = TypeConstraint::ExtendedHint | TypeConstraint::HHType;

    // We only care about a subset of extended type constaints:
    // typevar, nullable, soft, return types.
    //
    // For everything else, we return {}. We also return {} for annotations
    // we don't know how to handle.
    if (annot->isFunction() || annot->isMixed()) {
      return {};
    }
    if (annot->isTypeVar()) {
      flags = flags | TypeConstraint::TypeVar;
    }
    if (annot->isNullable()) {
      flags = flags | TypeConstraint::Nullable;
    }
    if (annot->isSoft()) {
      flags = flags | TypeConstraint::Soft;
    }
    if (!is_return &&
        (flags == (TypeConstraint::ExtendedHint | TypeConstraint::HHType))) {
      return {};
    }

    auto strippedName = annot->stripNullable().stripSoft().vanillaName();

    return TypeConstraint{
      makeStaticString(strippedName),
      flags
    };
  }

  return {};
}

static TypeConstraint
determine_type_constraint(const ParameterExpressionPtr& par) {
  if (par->hasTypeHint()) {
    auto ce = dynamic_pointer_cast<ConstantExpression>(par->defaultValue());
    auto flags = TypeConstraint::NoFlags;
    if (ce && ce->isNull()) {
      flags = flags|TypeConstraint::Nullable;
    }
    if (par->hhType()) {
      flags = flags|TypeConstraint::HHType;
    }
    return TypeConstraint{
      makeStaticString(par->getOriginalTypeHint()),
      flags
    };
  }

  return determine_type_constraint_from_annot(par->annotation(), false);
}

void EmitterVisitor::emitPostponedMeths() {
  vector<FuncEmitter*> top_fes;
  while (!m_postponedMeths.empty()) {
    assert(m_actualStackHighWater == 0);
    assert(m_fdescHighWater == 0);
    PostponedMeth& p = m_postponedMeths.front();
    MethodStatementPtr meth = p.m_meth;
    FuncEmitter* fe = p.m_fe;

    if (!fe) {
      assert(p.m_top);
      if (!m_topMethodEmitted.insert(meth->getOriginalName()).second) {
        throw IncludeTimeFatalException(
          meth,
          "Function already defined: %s",
          meth->getOriginalName().c_str());
      }

      const StringData* methName = makeStaticString(meth->getOriginalName());
      p.m_fe = fe = new FuncEmitter(m_ue, -1, -1, methName);
      top_fes.push_back(fe);
    }

    auto funcScope = meth->getFunctionScope();
    if (funcScope->isGenerator()) {
      // emit the generator
      m_curFunc = fe;
      fe->setIsGenerator(true);
      emitMethodMetadata(meth, p.m_closureUseVars, p.m_top);
      emitGeneratorMethod(meth);
    } else if (funcScope->isAsync()) {
      // emit the outer function (which creates continuation if blocked)
      m_curFunc = fe;
      fe->setIsAsync(true);
      emitMethodMetadata(meth, p.m_closureUseVars, p.m_top);
      emitAsyncMethod(meth);
    } else {
      m_curFunc = fe;
      if (funcScope->isNative()) {
        bindNativeFunc(meth, fe);
      } else {
        emitMethodMetadata(meth, p.m_closureUseVars, p.m_top);
        emitMethod(meth);
      }
    }

    if (fe->isClosureBody()) {
      TypedValue uninit;
      tvWriteUninit(&uninit);
      for (auto& sv : m_curFunc->svInfo()) {
        auto const str = makeStaticString(
          folly::format("86static_{}", sv.name->data()).str());
        fe->pce()->addProperty(str, AttrPrivate, nullptr, nullptr,
                               &uninit, RepoAuthType{});
        if (m_curFunc != fe && !fe->isAsync()) {
          // In the case of a generator (these func emitters will be
          // different), we need to propagate the information about
          // static locals from the generator implementation function
          // to the generator creation function.  (This is necessary
          // for the runtime to know how many of the properties on a
          // closure are for static locals vs. use vars.)
          fe->addStaticVar(sv);
        }
      }
    }

    delete p.m_closureUseVars;
    m_postponedMeths.pop_front();
  }

  for (size_t i = 0; i < top_fes.size(); i++) {
    m_ue.appendTopEmitter(top_fes[i]);
  }
}

void EmitterVisitor::bindUserAttributes(MethodStatementPtr meth,
                                        FuncEmitter *fe,
                                        bool &allowOverride) {
  auto const& userAttrs = meth->getFunctionScope()->userAttributes();
  for (auto& attr : userAttrs) {
    if (attr.first == "__Overridable") {
      allowOverride = true;
      continue;
    }
    const StringData* uaName = makeStaticString(attr.first);
    ExpressionPtr uaValue = attr.second;
    assert(uaValue);
    assert(uaValue->isScalar());
    TypedValue tv;
    initScalar(tv, uaValue);
    fe->addUserAttribute(uaName, tv);
  }
}

void EmitterVisitor::bindNativeFunc(MethodStatementPtr meth,
                                    FuncEmitter *fe) {
  if (SystemLib::s_inited &&
      !(Option::WholeProgram && meth->isSystem())) {
    throw IncludeTimeFatalException(meth,
          "Native functions/methods may only be defined in systemlib");
  }

  auto modifiers = meth->getModifiers();
  bool allowOverride = false;
  bindUserAttributes(meth, fe, allowOverride);

  Attr attributes = AttrBuiltin | AttrNative | AttrUnique | AttrPersistent;
  if (meth->isRef()) {
    attributes = attributes | AttrReference;
  }
  auto pce = fe->pce();
  if (pce) {
    if (modifiers->isStatic()) {
      attributes = attributes | AttrStatic;
    }
    if (modifiers->isFinal()) {
      attributes = attributes | AttrFinal;
    }
    if (modifiers->isAbstract()) {
      attributes = attributes | AttrAbstract;
    }
    if (modifiers->isPrivate()) {
      attributes = attributes | AttrPrivate;
    } else {
      attributes = attributes | (modifiers->isProtected()
                              ? AttrProtected : AttrPublic);
    }
  } else {
    if (allowOverride) {
      attributes = attributes | AttrAllowOverride;
    }
  }
  parseUserAttributes(fe, attributes);

  const Location* sLoc = meth->getLocation().get();
  fe->setLocation(sLoc->line0, sLoc->line1);
  fe->setDocComment(
    Option::GenerateDocComments ? meth->getDocComment().c_str() : "");
  fe->setReturnType(meth->retTypeAnnotation()->dataType());
  fe->setMaxStackCells(kNumActRecCells + 1);
  fe->setReturnUserType(makeStaticString(meth->getReturnTypeConstraint()));

  FunctionScopePtr funcScope = meth->getFunctionScope();
  const char *funcname  = funcScope->getName().c_str();
  const char *classname = pce ? pce->name()->data() : nullptr;
  BuiltinFunction nif = Native::GetBuiltinFunction(funcname, classname,
                                                   modifiers->isStatic());
  BuiltinFunction bif = pce ? Native::methodWrapper
                            : Native::functionWrapper;

  if (!nif) {
    bif = Native::unimplementedWrapper;
  } else if (fe->parseNativeAttributes(attributes) & Native::AttrActRec) {
    // Call this native function with a raw ActRec*
    // rather than pulling out args for normal func calling
    bif = nif;
    nif = nullptr;
  }

  Emitter e(meth, m_ue, *this);
  Label topOfBody(e);

  Offset base = m_ue.bcPos();
  fe->setBuiltinFunc(bif, nif, attributes, base);
  fillFuncEmitterParams(fe, meth->getParams(), true);
  e.NativeImpl();
  FuncFinisher ff(this, e, fe);
  emitMethodDVInitializers(e, meth, topOfBody);
}

void EmitterVisitor::emitMethodMetadata(MethodStatementPtr meth,
                                        ClosureUseVarVec* useVars,
                                        bool top) {
  FuncEmitter* fe = m_curFunc;
  bool allowOverride = false;
  bindUserAttributes(meth, fe, allowOverride);

  // assign ids to parameters (all methods)
  int numParam = meth->getParams() ? meth->getParams()->getCount() : 0;
  for (int i = 0; i < numParam; i++) {
    ParameterExpressionPtr par(
      static_pointer_cast<ParameterExpression>((*meth->getParams())[i]));
    fe->allocVarId(makeStaticString(par->getName()));
  }

  // assign ids to 0Closure and use parameters (closures)
  if (fe->isClosureBody()) {
    fe->allocVarId(makeStaticString("0Closure"));

    for (auto& useVar : *useVars) {
      fe->allocVarId(useVar.first);
    }
  }

  // assign id to continuationVarArgsLocal (generators/async - both methods)
  if (meth->hasCallToGetArgs() &&
      (meth->getFunctionScope()->isGenerator() ||
       meth->getFunctionScope()->isAsync())) {
    fe->allocVarId(s_continuationVarArgsLocal);
  }

  // assign ids to local variables
  assignLocalVariableIds(meth->getFunctionScope());

  // add parameter info
  fillFuncEmitterParams(fe, meth->getParams());

  // copy declared return type (hack)
  fe->setReturnUserType(makeStaticString(meth->getReturnTypeConstraint()));

  auto annot = meth->retTypeAnnotation();
  // Ideally we should handle the void case in TypeConstraint::check. This
  // should however get done in a different diff, since it could impact
  // perf in a negative way (#3145038)
  if (annot && !annot->isVoid() && !annot->isThis()) {
    fe->setReturnTypeConstraint(
      determine_type_constraint_from_annot(annot, true));
  }

  // add the original filename for flattened traits
  auto const originalFilename = meth->getOriginalFilename();
  if (!originalFilename.empty()) {
    fe->setOriginalFilename(makeStaticString(originalFilename));
  }

  const Location* sLoc = meth->getLocation().get();
  StringData* methDoc = Option::GenerateDocComments ?
    makeStaticString(meth->getDocComment()) : empty_string.get();

  fe->init(sLoc->line0,
           sLoc->line1,
           m_ue.bcPos(),
           buildMethodAttrs(meth, fe, top, allowOverride),
           top,
           methDoc);

  if (meth->getFunctionScope()->needsFinallyLocals()) {
    assignFinallyVariableIds();
  }
}

void EmitterVisitor::fillFuncEmitterParams(FuncEmitter* fe,
                                           ExpressionListPtr params,
                                           bool builtin /*= false */) {
  int numParam = params ? params->getCount() : 0;
  for (int i = 0; i < numParam; i++) {
    ParameterExpressionPtr par(
      static_pointer_cast<ParameterExpression>((*params)[i]));
    StringData* parName = makeStaticString(par->getName());

    FuncEmitter::ParamInfo pi;
    auto const typeConstraint = determine_type_constraint(par);
    if (typeConstraint.hasConstraint()) {
      pi.setTypeConstraint(typeConstraint);
    }
    if (builtin) {
      if (auto const typeAnnotation = par->annotation()) {
        pi.setBuiltinType(typeAnnotation->dataType());
      }
    }

    if (par->hasUserType()) {
      pi.setUserType(makeStaticString(par->getUserTypeHint()));
    }

    // Store info about the default value if there is one.
    if (par->isOptional()) {
      const StringData* phpCode;
      ExpressionPtr vNode = par->defaultValue();
      if (vNode->isScalar()) {
        TypedValue dv;
        initScalar(dv, vNode);
        pi.setDefaultValue(dv);

        std::string orig = vNode->getComment();
        if (orig.empty()) {
          // Simple case: it's a scalar value so we just serialize it
          VariableSerializer vs(VariableSerializer::Type::PHPOutput);
          String result = vs.serialize(tvAsCVarRef(&dv), true);
          phpCode = makeStaticString(result.get());
        } else {
          // This was optimized from a Constant, or ClassConstant
          // use the original string
          phpCode = makeStaticString(orig);
        }
      } else {
        // Non-scalar, so we have to output PHP from the AST node
        std::ostringstream os;
        CodeGenerator cg(&os, CodeGenerator::PickledPHP);
        AnalysisResultPtr ar(new AnalysisResult());
        vNode->outputPHP(cg, ar);
        phpCode = makeStaticString(os.str());
      }
      pi.setPhpCode(phpCode);
    }

    ExpressionListPtr paramUserAttrs =
      dynamic_pointer_cast<ExpressionList>(par->userAttributeList());
    if (paramUserAttrs) {
      for (int j = 0; j < paramUserAttrs->getCount(); ++j) {
        UserAttributePtr a = dynamic_pointer_cast<UserAttribute>(
          (*paramUserAttrs)[j]);
        StringData* uaName = makeStaticString(a->getName());
        ExpressionPtr uaValue = a->getExp();
        assert(uaValue);
        assert(uaValue->isScalar());
        TypedValue tv;
        initScalar(tv, uaValue);
        pi.addUserAttribute(uaName, tv);
      }
    }

    pi.setRef(par->isRef());
    fe->appendParam(parName, pi);
  }
}

void EmitterVisitor::emitMethodPrologue(Emitter& e, MethodStatementPtr meth) {
  FunctionScopePtr funcScope = meth->getFunctionScope();

  if (funcScope->needsLocalThis() && !funcScope->isStatic()) {
    assert(!m_curFunc->top());
    static const StringData* thisStr = makeStaticString("this");
    Id thisId = m_curFunc->lookupVarId(thisStr);
    e.InitThisLoc(thisId);
  }
  for (uint i = 0; i < m_curFunc->params().size(); i++) {
    const TypeConstraint& tc = m_curFunc->params()[i].typeConstraint();
    if (!tc.hasConstraint()) continue;
    e.VerifyParamType(i);
  }

  if (funcScope->isAbstract()) {
    std::ostringstream s;
    s << "Cannot call abstract method " << meth->getOriginalFullName() << "()";
    emitMakeUnitFatal(e, s.str().c_str(), FatalOp::RuntimeOmitFrame);
  }
}

void EmitterVisitor::emitMethod(MethodStatementPtr meth) {
  auto region = createRegion(meth, Region::Kind::FuncBody);
  enterRegion(region);
  SCOPE_EXIT { leaveRegion(region); };

  Emitter e(meth, m_ue, *this);
  Label topOfBody(e);
  emitMethodPrologue(e, meth);

  // emit method body
  visit(meth->getStmts());
  assert(m_evalStack.size() == 0);

  // if the current position is reachable, emit code to return null
  if (currentPositionIsReachable()) {
    LocationPtr loc(new Location(*meth->getLocation().get()));
    loc->line0 = loc->line1;
    loc->char0 = loc->char1-1;
    e.setTempLocation(loc);
    e.Null();
    if ((meth->getStmts() && meth->getStmts()->isGuarded())) {
      m_metaInfo.add(m_ue.bcPos(), Unit::MetaInfo::Kind::GuardedThis,
                     false, 0, 0);
    }
    e.RetC();
    e.setTempLocation(LocationPtr());
  }

  FuncFinisher ff(this, e, m_curFunc);
  emitMethodDVInitializers(e, meth, topOfBody);
}

void EmitterVisitor::emitAsyncMethod(MethodStatementPtr meth) {
  auto region = createRegion(meth, Region::Kind::FuncBody);
  enterRegion(region);
  SCOPE_EXIT { leaveRegion(region); };

  Emitter e(meth, m_ue, *this);
  Label topOfBody(e);
  emitMethodPrologue(e, meth);
  emitSetFuncGetArgs(e);

  // emit method body executed in eager-execution mode
  visit(meth->getStmts());
  assert(m_evalStack.size() == 0);

  // if the current position is reachable, emit code to return null
  if (currentPositionIsReachable()) {
    e.Null();
    e.AsyncWrapResult();
    e.RetC();
  }

  // emit method body executed in resumed mode
  {
    SCOPE_EXIT {
      assert(m_inGenerator);
      m_inGenerator = false;
    };
    assert(!m_inGenerator);
    m_inGenerator = true;

    visit(meth->getStmts());
    assert(m_evalStack.size() == 0);

    // if the current position is reachable, emit code to return null
    if (currentPositionIsReachable()) {
      e.Null();
      e.ContRetC();
    }
  }

  FuncFinisher ff(this, e, m_curFunc);
  emitMethodDVInitializers(e, meth, topOfBody);
}

void EmitterVisitor::emitGeneratorMethod(MethodStatementPtr meth) {
  auto region = createRegion(meth, Region::Kind::FuncBody);
  enterRegion(region);
  SCOPE_EXIT { leaveRegion(region); };

  Emitter e(meth, m_ue, *this);
  Label topOfBody(e);
  emitMethodPrologue(e, meth);
  emitSetFuncGetArgs(e);

  // emit code to create generator object
  Label topOfResumedBody;
  e.CreateCont(topOfResumedBody);
  e.RetC();

  // emit generator body
  {
    SCOPE_EXIT {
      assert(m_inGenerator);
      m_inGenerator = false;
    };
    assert(!m_inGenerator);
    m_inGenerator = true;

    topOfResumedBody.set(e);
    e.PopC();
    visit(meth->getStmts());
    assert(m_evalStack.size() == 0);

    // if the current position is reachable, emit code to return null
    if (currentPositionIsReachable()) {
      e.Null();
      e.ContRetC();
    }
  }

  FuncFinisher ff(this, e, m_curFunc);
  emitMethodDVInitializers(e, meth, topOfBody);
}

void EmitterVisitor::emitSetFuncGetArgs(Emitter& e) {
  if (m_curFunc->hasVar(s_continuationVarArgsLocal)) {
    static const StringData* s_func_get_args =
      makeStaticString("func_get_args");

    Id local = m_curFunc->lookupVarId(s_continuationVarArgsLocal);
    emitVirtualLocal(local);
    e.FCallBuiltin(0, 0, s_func_get_args);
    e.UnboxRNop();
    emitSet(e);
    e.PopC();
  }
}

void EmitterVisitor::emitMethodDVInitializers(Emitter& e,
                                              MethodStatementPtr& meth,
                                              Label& topOfBody) {
  // Default value initializers
  bool hasOptional = false;
  ExpressionListPtr params = meth->getParams();
  int numParam = params ? params->getCount() : 0;
  for (int i = 0; i < numParam; i++) {
    ParameterExpressionPtr par(
      static_pointer_cast<ParameterExpression>((*params)[i]));
    if (par->isOptional()) {
      hasOptional = true;
      Label entryPoint(e);
      emitVirtualLocal(i, KindOfUninit);
      visit(par->defaultValue());
      emitCGet(e);
      emitSet(e);
      e.PopC();
      m_curFunc->setParamFuncletOff(i, entryPoint.getAbsoluteOffset());
    }
  }
  if (hasOptional) e.JmpNS(topOfBody);
}

void EmitterVisitor::emitPostponedCtors() {
  while (!m_postponedCtors.empty()) {
    PostponedCtor& p = m_postponedCtors.front();

    Attr attrs = AttrPublic;
    if (!SystemLib::s_inited || p.m_is->getClassScope()->isSystem()) {
      attrs = attrs | AttrBuiltin;
    }
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
  if (!SystemLib::s_inited || p.m_is->getClassScope()->isSystem()) {
    attrs = attrs | AttrBuiltin;
  }
  StringData* methDoc = empty_string.get();
  const Location* sLoc = p.m_is->getLocation().get();
  p.m_fe->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), attrs, false, methDoc);

  Emitter e(p.m_is, m_ue, *this);
  FuncFinisher ff(this, e, p.m_fe);

  // Private instance and static properties are initialized using
  // InitProp.
  size_t nProps = p.m_vec->size();
  assert(nProps > 0);
  for (size_t i = 0; i < nProps; ++i) {
    const StringData* propName =
      makeStaticString(((*p.m_vec)[i]).first);

    Label isset;
    InitPropOp op = InitPropOp::NonStatic;
    const PreClassEmitter::Prop& preProp =
      p.m_fe->pce()->lookupProp(propName);
    if ((preProp.attrs() & AttrStatic) == AttrStatic) {
      op = InitPropOp::Static;
    } else if ((preProp.attrs() & (AttrPrivate|AttrStatic)) != AttrPrivate) {
      e.CheckProp(const_cast<StringData*>(propName));
      e.JmpNZ(isset);
    }
    visit((*p.m_vec)[i].second);
    e.InitProp(const_cast<StringData*>(propName), op);
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
    if (!SystemLib::s_inited || p.m_is->getClassScope()->isSystem()) {
      attrs = attrs | AttrBuiltin;
    }
    StringData* methDoc = empty_string.get();
    const Location* sLoc = p.m_is->getLocation().get();
    p.m_fe->init(sLoc->line0, sLoc->line1, m_ue.bcPos(), attrs, false, methDoc);
    static const StringData* s_constName =
      makeStaticString("constName");
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
    assert(nConsts > 0);
    Label retC;
    for (size_t i = 0; i < nConsts - 1; ++i) {
      Label mismatch;

      emitVirtualLocal(0);
      emitCGet(e);
      e.String((StringData*)makeStaticString(((*p.m_vec)[i]).first));
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

static bool isNormalLocalVariable(const ExpressionPtr& expr) {
  SimpleVariable* sv = static_cast<SimpleVariable*>(expr.get());
  return (expr->is(Expression::KindOfSimpleVariable) &&
          !sv->isSuperGlobal() &&
          !sv->isThis());
}

template<class Expr>
void EmitterVisitor::emitVirtualClassBase(Emitter& e, Expr* node) {
  prepareEvalStack();

  m_evalStack.push(StackSym::K);

  if (node->isStatic()) {
    m_evalStack.setClsBaseType(SymbolicStack::CLS_LATE_BOUND);
  } else if (node->getClass()) {
    const ExpressionPtr& expr = node->getClass();
    if (isNormalLocalVariable(expr)) {
      SimpleVariable* sv = static_cast<SimpleVariable*>(expr.get());
      StringData* name = makeStaticString(sv->getName());
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
        makeStaticString(node->getOriginalClassName()));
    }
  } else if (node->isParent() &&
             node->getOriginalClass()->getOriginalParent().empty()) {
    // parent:: in a class without a parent.  We'll emit a Parent
    // opcode because it can handle this error case.
    m_evalStack.setClsBaseType(SymbolicStack::CLS_PARENT);
  } else {
    m_evalStack.setClsBaseType(SymbolicStack::CLS_STRING_NAME);
    m_evalStack.setString(
      makeStaticString(node->getOriginalClassName()));
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
      assert(nParams >= 2);
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

Func* EmitterVisitor::canEmitBuiltinCall(const std::string& name,
                                         int numParams) {
  if (Option::JitEnableRenameFunction) {
    return nullptr;
  }
  if (Option::DynamicInvokeFunctions.size()) {
    if (Option::DynamicInvokeFunctions.find(name) !=
        Option::DynamicInvokeFunctions.end()) {
      return nullptr;
    }
  }
  Func* f = Unit::lookupFunc(makeStaticString(name));
  if (!f ||
      (f->attrs() & AttrNoFCallBuiltin) ||
      !f->nativeFuncPtr() ||
      (f->numParams() > Native::maxFCallBuiltinArgs()) ||
      (numParams > f->numParams()) ||
      (f->returnType() == KindOfDouble)) return nullptr;

  if (f->methInfo()) {
    // IDL style builtin
    const ClassInfo::MethodInfo* info = f->methInfo();
    if (info->attribute & (ClassInfo::NoFCallBuiltin |
                           ClassInfo::VariableArguments |
                           ClassInfo::RefVariableArguments |
                           ClassInfo::MixedVariableArguments)) {
      return nullptr;
    }
  } else if (!(f->attrs() & AttrNative)) {
    // HNI only enables Variable args via ActRec which in turn
    // is captured by the f->nativeFuncPtr() == nullptr,
    // so there's nothing additional to check in the HNI case
    return nullptr;
  }

  for (int i = 0; i < f->numParams(); i++) {
    if (f->params()[i].builtinType() == KindOfDouble) {
      return nullptr;
    }
    if (i >= numParams) {
      if (f->methInfo()) {
        // IDL-style
        auto pi = f->methInfo()->parameters[i];
        if (!pi->valueLen) {
          return nullptr;
        }
        // unserializable default values such as TimeStamp::Current()
        // are serialized as kUnserializableString ("\x01")
        if (!strcmp(f->methInfo()->parameters[i]->value,
                    kUnserializableString)) return nullptr;
      } else {
        // HNI style
        auto &pi = f->params()[i];
        if (!pi.hasDefaultValue()) {
          return nullptr;
        }
        if (pi.defaultValue().m_type == KindOfUninit) {
          // TODO: Resolve persistent constants
          return nullptr;
        }
      }
    }
  }

  return f;
}

void EmitterVisitor::emitFuncCall(Emitter& e, FunctionCallPtr node,
                                  const char* nameOverride,
                                  ExpressionListPtr paramsOverride) {
  ExpressionPtr nameExp = node->getNameExp();
  const std::string& nameStr = nameOverride ? nameOverride :
                                              node->getOriginalName();
  ExpressionListPtr params(paramsOverride ? paramsOverride :
                                            node->getParams());
  int numParams = params ? params->getCount() : 0;
  Func* fcallBuiltin = nullptr;
  StringData* nLiteral = nullptr;
  Offset fpiStart = 0;
  if (node->getClass() || !node->getClassName().empty()) {
    bool isSelfOrParent = node->isSelf() || node->isParent();
    if (!node->isStatic() && !isSelfOrParent &&
        !node->getOriginalClassName().empty() && !nameStr.empty()) {
      // cls::foo()
      StringData* cLiteral =
        makeStaticString(node->getOriginalClassName());
      StringData* nLiteral = makeStaticString(nameStr);
      fpiStart = m_ue.bcPos();
      e.FPushClsMethodD(numParams, nLiteral, cLiteral);
      if (node->forcedPresent()) {
        m_metaInfo.add(fpiStart, Unit::MetaInfo::Kind::GuardedCls, false, 0, 0);
      }
    } else {
      emitVirtualClassBase(e, node.get());
      if (!nameStr.empty()) {
        // ...::foo()
        StringData* nLiteral = makeStaticString(nameStr);
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
    nLiteral = makeStaticString(nameStr);
    fcallBuiltin = canEmitBuiltinCall(nameStr, numParams);
    if (fcallBuiltin && (fcallBuiltin->attrs() & AttrAllowOverride)) {
      if (!Option::WholeProgram ||
          (node->getFuncScope() && node->getFuncScope()->isUserFunction())) {
        // In non-WholeProgram mode, we can't tell whether the function
        // will be overridden, so never use FCallBuiltin.
        // In WholeProgram mode, don't use FCallBuiltin if it *has* been
        // overridden.
        fcallBuiltin = nullptr;
      }
    }
    StringData* nsName = nullptr;
    if (!node->hadBackslash() && !nameOverride) {
      // nameOverride is to be used only when there's an exact function
      // to be called ... supporting a fallback doesn't make sense
      const std::string& nonNSName = node->getNonNSOriginalName();
      if (nonNSName != nameStr) {
        nsName = nLiteral;
        nLiteral = makeStaticString(nonNSName);
        fcallBuiltin = nullptr;
      }
    }

    if (!fcallBuiltin) {
      fpiStart = m_ue.bcPos();
      if (nsName == nullptr) {
        e.FPushFuncD(numParams, nLiteral);
      } else {
        assert(!nameOverride);
        e.FPushFuncU(numParams, nsName, nLiteral);
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
  if (fcallBuiltin) {
    assert(numParams <= fcallBuiltin->numParams());
    int i = 0;
    for (; i < numParams; i++) {
      // for builtin calls, since we don't push the ActRec, we
      // must determine the reffiness statically
      bool byRef = fcallBuiltin->byRef(i);
      emitBuiltinCallArg(e, (*params)[i], i, byRef);
    }
    if (fcallBuiltin->methInfo()) {
      // IDL style
      for (; i < fcallBuiltin->numParams(); i++) {
        const ClassInfo::ParameterInfo* pi =
          fcallBuiltin->methInfo()->parameters[i];
        Variant v = unserialize_from_string(
          String(pi->value, pi->valueLen, CopyString));
        emitBuiltinDefaultArg(e, v, pi->argType, i);
      }
    } else {
      // HNI style
      for (; i < fcallBuiltin->numParams(); i++) {
        auto &pi = fcallBuiltin->params()[i];
        assert(pi.hasDefaultValue());
        auto &def = pi.defaultValue();
        emitBuiltinDefaultArg(e, tvAsVariant(const_cast<TypedValue*>(&def)),
                              pi.builtinType(), i);
      }
    }
    e.FCallBuiltin(fcallBuiltin->numParams(), numParams, nLiteral);
  } else {
    {
      FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
      for (int i = 0; i < numParams; i++) {
        emitFuncCallArg(e, (*params)[i], i);
      }
    }
    e.FCall(numParams);
  }
  if (Option::WholeProgram || fcallBuiltin) {
    fixReturnType(e, node, fcallBuiltin);
  }
}

void EmitterVisitor::emitClassTraitPrecRule(PreClassEmitter* pce,
                                            TraitPrecStatementPtr stmt) {
  StringData* traitName  = makeStaticString(stmt->getTraitName());
  StringData* methodName = makeStaticString(stmt->getMethodName());

  PreClass::TraitPrecRule rule(traitName, methodName);

  std::set<std::string> otherTraitNames;
  stmt->getOtherTraitNames(otherTraitNames);
  for (std::set<std::string>::iterator it = otherTraitNames.begin();
       it != otherTraitNames.end(); it++) {
    rule.addOtherTraitName(makeStaticString(*it));
  }

  pce->addTraitPrecRule(rule);
}

void EmitterVisitor::emitClassTraitAliasRule(PreClassEmitter* pce,
                                             TraitAliasStatementPtr stmt) {
  StringData* traitName    = makeStaticString(stmt->getTraitName());
  StringData* origMethName = makeStaticString(stmt->getMethodName());
  StringData* newMethName  = makeStaticString(stmt->getNewMethodName());
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
      assert(aliasStmt);
      emitClassTraitAliasRule(pce, aliasStmt);
    }
  }
}

void EmitterVisitor::emitTypedef(Emitter& e, TypedefStatementPtr td) {
  auto const nullable = td->annot->isNullable();
  auto const valueStr = td->annot->stripNullable().vanillaName();
  auto const kind =
    td->annot->stripNullable().isFunction()  ? KindOfAny :
    td->annot->stripNullable().isMixed()     ? KindOfAny :
    !strcasecmp(valueStr.c_str(), "array")   ? KindOfArray :
    !strcasecmp(valueStr.c_str(), "HH\\int") ? KindOfInt64 :
    !strcasecmp(valueStr.c_str(), "HH\\bool") ? KindOfBoolean :
    !strcasecmp(valueStr.c_str(), "HH\\string") ? KindOfString :
    !strcasecmp(valueStr.c_str(), "HH\\float") ? KindOfDouble :
    KindOfObject;

  // We have to merge the strings as litstrs to ensure namedentity
  // creation.
  auto const name = makeStaticString(td->name);
  auto const value = makeStaticString(valueStr);
  m_ue.mergeLitstr(name);
  m_ue.mergeLitstr(value);

  TypeAlias record;
  record.name     = name;
  record.value    = value;
  record.kind     = kind;
  record.nullable = nullable;
  Id id = m_ue.addTypeAlias(record);
  e.DefTypeAlias(id);
}

static RepoAuthType repoAuthTypeForHphpcType(DataType dtype) {
  if (!Option::WholeProgram) return RepoAuthType{};
  switch (dtype) {
  case KindOfBoolean:
    return RepoAuthType{RepoAuthType::Tag::OptBool};
  case KindOfInt64:
    return RepoAuthType{RepoAuthType::Tag::OptInt};
  case KindOfDouble:
    return RepoAuthType{RepoAuthType::Tag::OptDbl};
  case KindOfArray:
    return RepoAuthType{RepoAuthType::Tag::OptArr};
  case KindOfObject:
    return RepoAuthType{RepoAuthType::Tag::OptObj};
  case KindOfString:
  case KindOfStaticString:
    return RepoAuthType{RepoAuthType::Tag::OptStr};
  case KindOfResource:
    return RepoAuthType{RepoAuthType::Tag::OptRes};
  default:
    return RepoAuthType{};
  }
}

void EmitterVisitor::emitClass(Emitter& e,
                               ClassScopePtr cNode,
                               bool toplevel) {

  const StringData* fatal_msg = cNode->getFatalMessage();
  if (fatal_msg != nullptr) {
    e.String(fatal_msg);
    e.Fatal(FatalOp::Runtime);
    return;
  }

  InterfaceStatementPtr is(
    static_pointer_cast<InterfaceStatement>(cNode->getStmt()));
  StringData* className = makeStaticString(cNode->getOriginalName());
  StringData* parentName = makeStaticString(cNode->getOriginalParent());
  StringData* classDoc = Option::GenerateDocComments ?
    makeStaticString(cNode->getDocComment()) : empty_string.get();
  Attr attr = cNode->isInterface() ? AttrInterface :
              cNode->isTrait()     ? AttrTrait     :
              cNode->isAbstract()  ? AttrAbstract  :
              cNode->isFinal()     ? AttrFinal     :
                                     AttrNone;
  if (Option::WholeProgram) {
    if (!cNode->isRedeclaring() &&
        cNode->derivesFromRedeclaring() == Derivation::Normal) {
      attr = attr | AttrUnique;
      if (!cNode->isVolatile()) {
        attr = attr | AttrPersistent;
      }
    }
    if (cNode->isSystem()) {
      assert(attr & AttrPersistent);
      attr = attr | AttrBuiltin;
    }
    if (!cNode->getAttribute(ClassScope::NotFinal)) {
      attr = attr | AttrNoOverride;
    }
    if (cNode->getUsedTraitNames().size()) {
      attr = attr | AttrNoExpandTrait;
    }
  } else if (!SystemLib::s_inited) {
    // we're building systemlib. everything is unique
    attr = attr | AttrBuiltin | AttrUnique | AttrPersistent;
  }

  const Location* sLoc = is->getLocation().get();
  const std::vector<std::string>& bases(cNode->getBases());
  int firstInterface = cNode->getOriginalParent().empty() ? 0 : 1;
  int nInterfaces = bases.size();
  PreClass::Hoistable hoistable = PreClass::NotHoistable;
  if (toplevel) {
    if (SystemLib::s_inited && !cNode->isSystem()) {
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
  LocationPtr loc(new Location(*sLoc));
  loc->line1 = loc->line0;
  loc->char1 = loc->char0;
  e.setTempLocation(loc);
  if (hoistable != PreClass::AlwaysHoistable) {
    e.DefCls(pce->id());
  } else {
    // To attach the line number to for error reporting.
    e.NopDefCls(pce->id());
  }
  e.setTempLocation(LocationPtr());
  for (int i = firstInterface; i < nInterfaces; ++i) {
    pce->addInterface(makeStaticString(bases[i]));
  }

  const std::vector<std::string>& usedTraits = cNode->getUsedTraitNames();
  for (size_t i = 0; i < usedTraits.size(); i++) {
    pce->addUsedTrait(makeStaticString(usedTraits[i]));
  }
  if (cNode->isTrait()) {
    for (auto& reqExtends : cNode->getTraitRequiredExtends()) {
      pce->addTraitRequirement(
        PreClass::TraitRequirement(makeStaticString(reqExtends), true));
    }
    for (auto& reqImplements : cNode->getTraitRequiredImplements()) {
      pce->addTraitRequirement(
        PreClass::TraitRequirement(makeStaticString(reqImplements), false));
    }
  }
  auto const& userAttrs = cNode->userAttributes();
  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    const StringData* uaName = makeStaticString(it->first);
    ExpressionPtr uaValue = it->second;
    assert(uaValue);
    assert(uaValue->isScalar());
    TypedValue tv;
    initScalar(tv, uaValue);
    pce->addUserAttribute(uaName, tv);
  }

  NonScalarVec* nonScalarPinitVec = nullptr;
  NonScalarVec* nonScalarSinitVec = nullptr;
  NonScalarVec* nonScalarConstVec = nullptr;
  if (StatementListPtr stmts = is->getStmts()) {
    int i, n = stmts->getCount();
    for (i = 0; i < n; i++) {
      if (MethodStatementPtr meth =
          dynamic_pointer_cast<MethodStatement>((*stmts)[i])) {
        StringData* methName = makeStaticString(meth->getOriginalName());
        FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
        bool added UNUSED = pce->addMethod(fe);
        assert(added);
        postponeMeth(meth, fe, false);
      } else if (ClassVariablePtr cv =
                 dynamic_pointer_cast<ClassVariable>((*stmts)[i])) {
        ModifierExpressionPtr mod(cv->getModifiers());
        ExpressionListPtr el(cv->getVarList());
        Attr declAttrs = buildAttrs(mod);
        StringData* typeConstraint = makeStaticString(
          cv->getTypeConstraint());
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

          /*
           * Translate what hphpc can infer about a property type into
           * a RepoAuthType for the runtime.  The type hphpc has
           * implies non-uninit, but is always nullable.
           *
           * Note: for hphpc's types it appears this doesn't actually
           * imply the type is unboxed, either.
           */
          auto const hphpcType = repoAuthTypeForHphpcType(
            var->getSymbol()
              ? var->getSymbol()->getFinalType()->getDataType()
              : KindOfInvalid);

          auto const propName = makeStaticString(var->getName());
          auto const propDoc = Option::GenerateDocComments ?
            makeStaticString(var->getDocComment()) : empty_string.get();
          TypedValue tvVal;
          // Some properties may need to be marked with the AttrDeepInit
          // attribute, while other properties should not be marked with
          // this attrbiute. We copy declAttrs into propAttrs for each loop
          // iteration so that we can safely add AttrDeepInit to propAttrs
          // without mutating the original declAttrs.
          Attr propAttrs = declAttrs;
          if (vNode) {
            if (vNode->isScalar()) {
              initScalar(tvVal, vNode);
            } else {
              tvWriteUninit(&tvVal);
              if (!(declAttrs & AttrStatic)) {
                if (requiresDeepInit(vNode)) {
                  propAttrs = propAttrs | AttrDeepInit;
                }
                if (nonScalarPinitVec == nullptr) {
                  nonScalarPinitVec = new NonScalarVec();
                }
                nonScalarPinitVec->push_back(NonScalarPair(propName, vNode));
              } else {
                if (nonScalarSinitVec == nullptr) {
                  nonScalarSinitVec = new NonScalarVec();
                }
                nonScalarSinitVec->push_back(NonScalarPair(propName, vNode));
              }
            }
          } else {
            tvWriteNull(&tvVal);
          }
          bool added UNUSED =
            pce->addProperty(propName, propAttrs, typeConstraint,
                             propDoc, &tvVal, hphpcType);
          assert(added);
        }
      } else if (ClassConstantPtr cc =
                 dynamic_pointer_cast<ClassConstant>((*stmts)[i])) {
        ExpressionListPtr el(cc->getConList());
        StringData* typeConstraint = makeStaticString(
          cc->getTypeConstraint());
        int nCons = el->getCount();
        for (int ii = 0; ii < nCons; ii++) {
          AssignmentExpressionPtr ae(
            static_pointer_cast<AssignmentExpression>((*el)[ii]));
          ConstantExpressionPtr con(
            static_pointer_cast<ConstantExpression>(ae->getVariable()));
          ExpressionPtr vNode(ae->getValue());
          StringData* constName = makeStaticString(con->getName());
          assert(vNode);
          TypedValue tvVal;
          if (vNode->isArray()) {
            throw IncludeTimeFatalException(
              cc, "Arrays are not allowed in class constants");
          } else if (vNode->isCollection()) {
            throw IncludeTimeFatalException(
              cc, "Collections are not allowed in class constants");
          } else if (vNode->isScalar()) {
            initScalar(tvVal, vNode);
          } else {
            tvWriteUninit(&tvVal);
            if (nonScalarConstVec == nullptr) {
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
            constName, typeConstraint, &tvVal,
            makeStaticString(os.str()));
          assert(added);
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
    static const StringData* methName = makeStaticString("86ctor");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    bool added UNUSED = pce->addMethod(fe);
    assert(added);
    postponeCtor(is, fe);
  }

  if (nonScalarPinitVec != nullptr) {
    // Non-scalar property initializers require 86pinit() for run-time
    // initialization support.
    static const StringData* methName = makeStaticString("86pinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    pce->addMethod(fe);
    postponePinit(is, fe, nonScalarPinitVec);
  }

  if (nonScalarSinitVec != nullptr) {
    // Non-scalar property initializers require 86sinit() for run-time
    // initialization support.
    static const StringData* methName = makeStaticString("86sinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    pce->addMethod(fe);
    postponeSinit(is, fe, nonScalarSinitVec);
  }

  if (nonScalarConstVec != nullptr) {
    // Non-scalar constant initializers require 86cinit() for run-time
    // initialization support.
    static const StringData* methName = makeStaticString("86cinit");
    FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
    assert(!(attr & AttrTrait));
    bool added UNUSED = pce->addMethod(fe);
    assert(added);
    postponeCinit(is, fe, nonScalarConstVec);
  }
}

namespace {

class ForeachIterGuard {
  EmitterVisitor& m_ev;
 public:
  ForeachIterGuard(EmitterVisitor& ev,
                   Id iterId,
                   IterKind kind)
    : m_ev(ev)
  {
    m_ev.pushIterScope(iterId, kind);
  }
  ~ForeachIterGuard() {
    m_ev.popIterScope();
  }
};

}

void EmitterVisitor::emitForeachListAssignment(Emitter& e,
                                               ListAssignmentPtr la,
                                               int vLocalId) {
  std::vector<IndexChain*> indexChains;
  IndexChain workingChain;
  visitListAssignmentLHS(e, la, workingChain, indexChains);

  if (indexChains.size() == 0) {
    throw IncludeTimeFatalException(la, "Cannot use empty list");
  }

  for (int i = (int)indexChains.size() - 1; i >= 0; --i) {
    IndexChain* currIndexChain = indexChains[i];
    if (currIndexChain->empty()) {
      continue;
    }

    emitVirtualLocal(vLocalId);
    for (int j = 0; j < (int)currIndexChain->size(); ++j) {
      m_evalStack.push(StackSym::I);
      m_evalStack.setInt((*currIndexChain)[j]);
      markElem(e);
    }
    emitCGet(e);
    emitSet(e);
    emitPop(e);

    delete currIndexChain;
  }
}

void EmitterVisitor::emitForeach(Emitter& e,
                                 ForEachStatementPtr fe) {
  auto region = createRegion(fe, Region::Kind::LoopOrSwitch);
  ExpressionPtr ae(fe->getArrayExp());
  ExpressionPtr val(fe->getValueExp());
  ExpressionPtr key(fe->getNameExp());
  StatementPtr body(fe->getBody());
  int keyTempLocal;
  int valTempLocal;
  bool strong = fe->isStrong();
  Label& exit = registerBreak(fe, region.get(), 1, false)->m_label;
  Label& next = registerContinue(fe, region.get(), 1, false)->m_label;
  Label start;
  Offset bIterStart;
  Id itId = m_curFunc->allocIterator();
  ForeachIterGuard fig(*this, itId, strong ? KindOfMIter : KindOfIter);
  bool simpleCase = (!key || isNormalLocalVariable(key)) &&
                    isNormalLocalVariable(val);
  bool listKey = key ? key->is(Expression::KindOfListAssignment) : false;
  bool listVal = val->is(Expression::KindOfListAssignment);

  if (simpleCase) {
    SimpleVariablePtr svVal(static_pointer_cast<SimpleVariable>(val));
    StringData* name = makeStaticString(svVal->getName());
    valTempLocal = m_curFunc->lookupVarId(name);
    if (key) {
      SimpleVariablePtr svKey(static_pointer_cast<SimpleVariable>(key));
      name = makeStaticString(svKey->getName());
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
        e.MIterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.MIterInit(itId, exit, valTempLocal);
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
        e.MIterInitK(itId, exit, valTempLocal, keyTempLocal);
      } else {
        e.MIterInit(itId, exit, valTempLocal);
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
    if (key && !listKey) {
      visit(key);
    }
    if (listVal) {
      emitForeachListAssignment(
        e,
        ListAssignmentPtr(static_pointer_cast<ListAssignment>(val)),
        valTempLocal
      );
    } else {
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
    }
    emitVirtualLocal(valTempLocal);
    emitUnset(e);
    newFaultRegionAndFunclet(bIterStart, m_ue.bcPos(),
                             new UnsetUnnamedLocalThunklet(valTempLocal));
    if (key) {
      assert(keyTempLocal != -1);
      if (listKey) {
        emitForeachListAssignment(
          e,
          ListAssignmentPtr(static_pointer_cast<ListAssignment>(key)),
          keyTempLocal
        );
      } else {
        emitVirtualLocal(keyTempLocal);
        emitCGet(e);
        emitSet(e);
        emitPop(e);
      }
      emitVirtualLocal(keyTempLocal);
      emitUnset(e);
      newFaultRegionAndFunclet(bIterStart, m_ue.bcPos(),
                               new UnsetUnnamedLocalThunklet(keyTempLocal));
    }
  }

  {
    region->m_iterId = itId;
    region->m_iterKind = strong ? KindOfMIter : KindOfIter;
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };
    if (body) visit(body);
  }
  if (next.isUsed()) {
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
      e.MIterNextK(itId, start, valTempLocal, keyTempLocal);
    } else {
      e.MIterNext(itId, start, valTempLocal);
    }
  } else {
    if (key) {
      e.IterNextK(itId, start, valTempLocal, keyTempLocal);
    } else {
      e.IterNext(itId, start, valTempLocal);
    }
  }
  newFaultRegionAndFunclet(bIterStart, m_ue.bcPos(),
                           new IterFreeThunklet(itId, strong),
                           { itId, strong ? KindOfMIter : KindOfIter });
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
  static const StringData* funcName = makeStaticString("error_reporting");
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
    e.FPassC(0);
  }
  e.FCall(1);
  e.PopR();
  dontRollback.set(e);
}

void EmitterVisitor::emitMakeUnitFatal(Emitter& e,
                                       const char* msg,
                                       FatalOp k) {
  const StringData* sd = makeStaticString(msg);
  e.String(sd);
  e.Fatal(k);
}

Funclet* EmitterVisitor::addFunclet(StatementPtr stmt, Thunklet* body) {
  Funclet* f = addFunclet(body);
  m_memoizedFunclets.insert(std::make_pair(stmt, f));
  return f;
}

Funclet* EmitterVisitor::addFunclet(Thunklet* body) {
  m_funclets.push_back(new Funclet(body, m_inGenerator));
  return m_funclets.back();
}

Funclet* EmitterVisitor::getFunclet(StatementPtr stmt) {
  if (m_memoizedFunclets.count(stmt)) {
    return m_memoizedFunclets[stmt];
  } else {
    return nullptr;
  }
}

void EmitterVisitor::emitFunclets(Emitter& e) {
  // TODO (#3271358): New fault funclets might appear while emitting
  // finally fault funclets. This is because we currently don't memoize
  // fault funclets other than finally fault fuclets. See task
  // description for more details.
  for (int i = 0; i < m_funclets.size(); ++i) {
    Funclet* f = m_funclets[i];
    SCOPE_EXIT { m_inGenerator = false; };
    m_inGenerator = f->m_inGenerator;
    f->m_entry.set(e);
    f->m_body->emit(e);
    delete f->m_body;
    f->m_body = nullptr;
  }
}

void EmitterVisitor::newFaultRegion(Offset start,
                                    Offset end,
                                    Label* entry,
                                    FaultIterInfo iter) {
  auto r = new FaultRegion(start, end, entry, iter.iterId, iter.kind);
  m_faultRegions.push_back(r);
}

void EmitterVisitor::newFaultRegionAndFunclet(Offset start,
                                              Offset end,
                                              Thunklet* t,
                                              FaultIterInfo iter) {
  Funclet* f = addFunclet(t);
  newFaultRegion(start, end, &f->m_entry, iter);
}

void EmitterVisitor::newFaultRegionAndFunclet(StatementPtr stmt,
                                              Offset start,
                                              Offset end,
                                              Thunklet* t,
                                              FaultIterInfo iter) {
  Funclet* f = addFunclet(stmt, t);
  newFaultRegion(start, end, &f->m_entry, iter);
}

void EmitterVisitor::newFPIRegion(Offset start, Offset end, Offset fpOff) {
  FPIRegion* r = new FPIRegion(start, end, fpOff);
  m_fpiRegions.push_back(r);
}

void EmitterVisitor::copyOverCatchAndFaultRegions(FuncEmitter* fe) {
  for (auto& eh : m_catchRegions) {
    EHEnt& e = fe->addEHEnt();
    e.m_type = EHEnt::Type::Catch;
    e.m_base = eh->m_start;
    e.m_past = eh->m_end;
    assert(e.m_base != kInvalidOffset);
    assert(e.m_past != kInvalidOffset);
    e.m_iterId = -1;
    for (auto& c : eh->m_catchLabels) {
      Id id = m_ue.mergeLitstr(c.first);
      Offset off = c.second->getAbsoluteOffset();
      e.m_catches.push_back(std::pair<Id, Offset>(id, off));
    }
    delete eh;
  }
  m_catchRegions.clear();
  for (auto& fr : m_faultRegions) {
    EHEnt& e = fe->addEHEnt();
    e.m_type = EHEnt::Type::Fault;
    e.m_base = fr->m_start;
    e.m_past = fr->m_end;
    assert(e.m_base != kInvalidOffset);
    assert(e.m_past != kInvalidOffset);
    e.m_iterId = fr->m_iterId;
    e.m_itRef = fr->m_iterKind == KindOfMIter;
    e.m_fault = fr->m_func->getAbsoluteOffset();
    assert(e.m_fault != kInvalidOffset);
    delete fr;
  }
  m_faultRegions.clear();
  for (auto f : m_funclets) {
    delete f;
  }
  m_funclets.clear();
  m_memoizedFunclets.clear();
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
  // Max stack cells is used for stack overflow checks.  We need to
  // count all the locals, and all cells due to ActRecs.  We don't
  // need to count this function's own ActRec because whoever called
  // it already included it in its "max stack cells".
  fe->setMaxStackCells(m_actualStackHighWater +
                       fe->numLocals() +
                       m_fdescHighWater);
  m_actualStackHighWater = 0;
  m_fdescHighWater = 0;
}

// Are you sure you mean to be calling this directly? Would FuncFinisher
// be more appropriate?
void EmitterVisitor::finishFunc(Emitter& e, FuncEmitter* fe) {
  emitFunclets(e);
  saveMaxStackCells(fe);
  copyOverCatchAndFaultRegions(fe);
  copyOverFPIRegions(fe);
  m_awaitLabels.clear();
  m_staticEmitted.clear();
  Offset past = e.getUnitEmitter().bcPos();
  fe->finish(past, false);
  e.getUnitEmitter().recordFunction(fe);
  if (m_stateLocal >= 0) {
    m_stateLocal = -1;
  }
  if (m_retLocal >= 0) {
    m_retLocal = -1;
  }
}

void EmitterVisitor::initScalar(TypedValue& tvVal, ExpressionPtr val) {
  assert(val->isScalar());
  tvVal.m_type = KindOfUninit;
  switch (val->getKindOf()) {
    case Expression::KindOfConstantExpression: {
      ConstantExpressionPtr ce(static_pointer_cast<ConstantExpression>(val));
      if (ce->isNull()) {
        tvVal.m_data.num = 0;
        tvVal.m_type = KindOfNull;
      } else if (ce->isBoolean()) {
        tvVal = make_tv<KindOfBoolean>(ce->getBooleanValue());
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
        StringData* sd = makeStaticString(*s);
        tvVal = make_tv<KindOfString>(sd);
        break;
      }
      int64_t i;
      if (sval->getInt(i)) {
        tvVal = make_tv<KindOfInt64>(i);
        break;
      }
      double d;
      if (sval->getDouble(d)) {
        tvVal = make_tv<KindOfDouble>(d);
        break;
      }
      assert(false);
      break;
    }
    case Expression::KindOfUnaryOpExpression: {
      UnaryOpExpressionPtr u(static_pointer_cast<UnaryOpExpression>(val));
      if (u->getOp() == T_ARRAY) {
        m_staticArrays.push_back(Array::attach(HphpArray::MakeReserve(0)));
        visit(u->getExpression());
        tvVal = make_tv<KindOfArray>(
          ArrayData::GetScalarArray(m_staticArrays.back().get())
        );
        m_staticArrays.pop_back();
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

bool EmitterVisitor::requiresDeepInit(ExpressionPtr initExpr) const {
  switch (initExpr->getKindOf()) {
    case Expression::KindOfScalarExpression:
    case Expression::KindOfConstantExpression:
    case Expression::KindOfClassConstantExpression:
      return false;
    case Expression::KindOfUnaryOpExpression: {
      UnaryOpExpressionPtr u(
        static_pointer_cast<UnaryOpExpression>(initExpr));
      if (u->getOp() == T_ARRAY) {
        ExpressionListPtr el =
          static_pointer_cast<ExpressionList>(u->getExpression());
        if (el) {
          int n = el->getCount();
          for (int i = 0; i < n; i++) {
            ArrayPairExpressionPtr ap =
              static_pointer_cast<ArrayPairExpression>((*el)[i]);
            ExpressionPtr key = ap->getName();
            if (requiresDeepInit(ap->getValue()) ||
                (key && requiresDeepInit(key))) {
              return true;
            }
          }
        }
        return false;
      } else if (u->getOp() == '+' || u->getOp() == '-') {
        return requiresDeepInit(u->getExpression());
      } else if (u->getOp() == T_FILE || u->getOp() == T_DIR) {
        return false;
      }
      // fall through
    }
    default:
      return true;
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
      case Expression::KindOfClosureExpression: {
        ClosureExpressionPtr cl = static_pointer_cast<ClosureExpression>(e);
        auto UNUSED exp = doOptimize(cl->getClosureFunction(), ar);
        assert(!exp);
        break;
      }
      default: break;
    }
  }
  return ConstructPtr();
}

static UnitEmitter* emitHHBCUnitEmitter(AnalysisResultPtr ar, FileScopePtr fsp,
                                        const MD5& md5) {
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
    auto kind = ex.m_parseFatal ? FatalOp::Parse : FatalOp::Runtime;
    fev.emitMakeUnitFatal(emitter, ex.getMessage().c_str(), kind);
  }
  return ue;
}

struct Entry {
  StringData* name;
  const HhbcExtClassInfo* info;
  const ClassInfo* ci;
};

static std::unique_ptr<UnitEmitter>
emitHHBCNativeFuncUnit(const HhbcExtFuncInfo* builtinFuncs,
                       ssize_t numBuiltinFuncs) {
  MD5 md5("11111111111111111111111111111111");
  auto ue = folly::make_unique<UnitEmitter>(md5);
  ue->setFilepath(makeStaticString(""));
  ue->addTrivialPseudoMain();

  Attr attrs = AttrBuiltin | AttrUnique | AttrPersistent;
  /*
    Special function used by FPushCuf* when its argument
    is not callable.
  */
  StringData* name = makeStaticString("86null");
  FuncEmitter* fe = ue->newFuncEmitter(name);
  fe->init(0, 0, ue->bcPos(), attrs, true, empty_string.get());
  ue->emitOp(OpNull);
  ue->emitOp(OpRetC);
  fe->setMaxStackCells(1);
  fe->finish(ue->bcPos(), false);
  ue->recordFunction(fe);

  for (ssize_t i = 0; i < numBuiltinFuncs; ++i) {
    const HhbcExtFuncInfo* info = &builtinFuncs[i];
    StringData* name = makeStaticString(info->m_name);
    BuiltinFunction bif = (BuiltinFunction)info->m_builtinFunc;
    BuiltinFunction nif = (BuiltinFunction)info->m_nativeFunc;
    const ClassInfo::MethodInfo* mi = ClassInfo::FindFunction(name);
    assert(mi &&
      "MethodInfo not found; may be a problem with the .idl.json files");
    if ((mi->attribute & ClassInfo::ZendCompat) &&
        !RuntimeOption::EnableZendCompat) {
      continue;
    }

    // We already provide array_map by the hhas systemlib.  Rename
    // because that hhas implementation delegates back to the C++
    // implementation for some edge cases.  This works for any
    // similarly defined function (already defined not as a builtin),
    // and requires that the hhas systemlib is already loaded.
    if (auto const existing = Unit::lookupFunc(name)) {
      if (!existing->isCPPBuiltin()) {
        name = makeStaticString("__builtin_" + name->toCppString());
      }
    }

    FuncEmitter* fe = ue->newFuncEmitter(name);
    Offset base = ue->bcPos();
    fe->setBuiltinFunc(mi, bif, nif, base);
    ue->emitOp(OpNativeImpl);
    fe->setMaxStackCells(kNumActRecCells + 1);
    fe->setAttrs(fe->attrs() | attrs);
    fe->finish(ue->bcPos(), false);
    ue->recordFunction(fe);
  }

  return ue;
}

enum ContinuationMethod {
  METH_NEXT,
  METH_SEND,
  METH_RAISE,
  METH_VALID,
  METH_CURRENT,
  METH_KEY,
};
typedef hphp_hash_map<const StringData*, ContinuationMethod,
                      string_data_hash, string_data_same> ContMethMap;

static void emitContinuationMethod(UnitEmitter& ue, FuncEmitter* fe,
                                   ContinuationMethod m) {
  static const StringData* valStr = makeStaticString("value");
  static const StringData* exnStr = makeStaticString("exception");

  Attr attrs = (Attr)(AttrBuiltin | AttrPublic | AttrMayUseVV);
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

      // check continuation status; send()/raise() also checks started
      ue.emitOp(OpContCheck);
      ue.emitIVA(m == METH_SEND || m == METH_RAISE);

      const Offset ehStart = ue.bcPos();
      switch(m) {
        case METH_NEXT:
          ue.emitOp(OpNull);
          ue.emitOp(OpContEnter);
          break;
        case METH_SEND:
          ue.emitOp(OpPushL); ue.emitIVA(0);
          ue.emitOp(OpContEnter);
          break;
        case METH_RAISE:
          ue.emitOp(OpPushL); ue.emitIVA(0);
          ue.emitOp(OpContRaise);
          break;
        default:
          not_reached();
      }

      ue.emitOp(OpContStopped);
      ue.emitOp(OpNull);
      ue.emitOp(OpRetC);

      EHEnt& eh = fe->addEHEnt();
      eh.m_type = EHEnt::Type::Catch;
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
    case METH_KEY: {
      ue.emitOp(OpContKey);
      ue.emitOp(OpRetC);
      break;
    }

    default:
      not_reached();
  }
}

StaticString s_construct("__construct");
static std::unique_ptr<UnitEmitter>
emitHHBCNativeClassUnit(const HhbcExtClassInfo* builtinClasses,
                        ssize_t numBuiltinClasses) {
  MD5 md5("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
  auto ue = folly::make_unique<UnitEmitter>(md5);
  ue->setFilepath(makeStaticString(""));
  ue->addTrivialPseudoMain();

  ContMethMap contMethods;
  contMethods[makeStaticString("next")] = METH_NEXT;
  contMethods[makeStaticString("send")] = METH_SEND;
  contMethods[makeStaticString("raise")] = METH_RAISE;
  contMethods[makeStaticString("valid")] = METH_VALID;
  contMethods[makeStaticString("current")] = METH_CURRENT;
  contMethods[makeStaticString("key")] = METH_KEY;

  // Build up extClassHash, a hashtable that maps class names to structures
  // containing C++ function pointers for the class's methods and constructors
  if (!Class::s_extClassHash.empty()) {
    Class::s_extClassHash.clear();
  }
  for (long long i = 0; i < numBuiltinClasses; ++i) {
    const HhbcExtClassInfo* info = builtinClasses + i;
    StringData *s = makeStaticString(info->m_name);
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
      e.ci = ClassInfo::FindSystemClassInterfaceOrTrait(e.name);
      assert(e.ci);
      if ((e.ci->getAttribute() & ClassInfo::ZendCompat) &&
          !RuntimeOption::EnableZendCompat) {
        continue;
      }
      StringData* parentName
        = makeStaticString(e.ci->getParentClass().get());
      if (parentName->empty()) {
        // If this class doesn't have a base class, it's eligible to be
        // loaded now
        classEntries.push_back(e);
      } else if ((e.ci->getAttribute() & ClassInfo::ZendCompat) &&
                 Unit::loadClass(parentName)) {
        // You can really hurt yourself trying to extend a systemlib class from
        // a normal IDL (like overriding the property vector with your own C++
        // properties). ZendCompat things don't do any of that so they are cool.
        classEntries.push_back(e);
      } else {
        // If this class has a base class, we can't load it until its
        // base class has been loaded
        pending[parentName].push_back(e);
      }
    }
    for (unsigned k = 0; k < classEntries.size(); ++k) {
      Entry& e = classEntries[k];
      // Any classes that derive from this class are now eligible to be
      // loaded
      PendingMap::iterator pendingIt = pending.find(e.name);
      if (pendingIt != pending.end()) {
        for (unsigned i = 0; i < pendingIt->second.size(); ++i) {
          classEntries.push_back(pendingIt->second[i]);
        }
        pending.erase(pendingIt);
      }
    }
    assert(pending.empty());
  }

  for (unsigned int i = 0; i < classEntries.size(); ++i) {
    Entry& e = classEntries[i];
    StringData* parentName = makeStaticString(e.ci->getParentClass().get());
    PreClassEmitter* pce = ue->newPreClassEmitter(e.name,
                                                  PreClass::AlwaysHoistable);
    pce->init(0, 0, ue->bcPos(), AttrBuiltin|AttrUnique|AttrPersistent,
              parentName, nullptr);
    pce->setBuiltinClassInfo(
      e.ci,
      e.info->m_instanceCtor,
      e.info->m_instanceDtor,
      BuiltinObjExtents { e.info->m_totalSize, e.info->m_objectDataOffset }
    );
    {
      ClassInfo::InterfaceVec intfVec = e.ci->getInterfacesVec();
      for (unsigned i = 0; i < intfVec.size(); ++i) {
        const StringData* intf = makeStaticString(intfVec[i].get());
        pce->addInterface(intf);
      }
    }

    bool hasCtor = false;
    for (ssize_t j = 0; j < e.info->m_methodCount; ++j) {
      const HhbcExtMethodInfo* methodInfo = &(e.info->m_methods[j]);
      static const StringData* continuationCls =
        makeStaticString("continuation");
      StringData* methName = makeStaticString(methodInfo->m_name);
      ContinuationMethod* cmeth;

      FuncEmitter* fe = ue->newMethodEmitter(methName, pce);
      pce->addMethod(fe);
      if (e.name->isame(continuationCls) &&
          (cmeth = folly::get_ptr(contMethods, methName))) {
        auto methCpy = *cmeth;
        emitContinuationMethod(*ue, fe, methCpy);
      } else {
        if (e.name->isame(s_construct.get())) {
          hasCtor = true;
        }

        // Build the function
        BuiltinFunction bcf = (BuiltinFunction)methodInfo->m_pGenericMethod;
        const ClassInfo::MethodInfo* mi =
          e.ci->getMethodInfo(std::string(methodInfo->m_name));
        Offset base = ue->bcPos();
        fe->setBuiltinFunc(mi, bcf, nullptr, base);
        ue->emitOp(OpNativeImpl);
      }
      Offset past = ue->bcPos();
      fe->setMaxStackCells(kNumActRecCells + 1);
      fe->finish(past, false);
      ue->recordFunction(fe);
    }
    if (!hasCtor) {
      static const StringData* methName = makeStaticString("86ctor");
      FuncEmitter* fe = ue->newMethodEmitter(methName, pce);
      bool added UNUSED = pce->addMethod(fe);
      assert(added);
      fe->init(0, 0, ue->bcPos(), AttrBuiltin|AttrPublic,
               false, empty_string.get());
      ue->emitOp(OpNull);
      ue->emitOp(OpRetC);
      fe->setMaxStackCells(1);
      fe->finish(ue->bcPos(), false);
      ue->recordFunction(fe);
    }

    {
      ClassInfo::ConstantVec cnsVec = e.ci->getConstantsVec();
      for (unsigned i = 0; i < cnsVec.size(); ++i) {
        const ClassInfo::ConstantInfo* cnsInfo = cnsVec[i];
        assert(cnsInfo);
        Variant val;
        try {
          val = cnsInfo->getValue();
        } catch (Exception& e) {
          assert(false);
        }
        pce->addConstant(
          cnsInfo->name.get(),
          nullptr,
          (TypedValue*)(&val),
          empty_string.get());
      }
    }
    {
      ClassInfo::PropertyVec propVec = e.ci->getPropertiesVec();
      for (unsigned i = 0; i < propVec.size(); ++i) {
        const ClassInfo::PropertyInfo* propInfo = propVec[i];
        assert(propInfo);
        int attr = AttrNone;
        if (propInfo->attribute & ClassInfo::IsProtected) {
          attr |= AttrProtected;
        } else if (propInfo->attribute & ClassInfo::IsPrivate) {
          attr |= AttrPrivate;
        } else {
          attr |= AttrPublic;
        }
        if (propInfo->attribute & ClassInfo::IsStatic) attr |= AttrStatic;

        TypedValue tvNull;
        tvWriteNull(&tvNull);
        pce->addProperty(
          propInfo->name.get(),
          Attr(attr),
          nullptr,
          propInfo->docComment ?
          makeStaticString(propInfo->docComment) : nullptr,
          &tvNull,
          RepoAuthType{}
        );
      }
    }
  }

  MetaInfoBuilder metaInfo;
  Peephole peephole(*ue, metaInfo);
  metaInfo.setForUnit(*ue);

  return ue;
}

static UnitEmitter* emitHHBCVisitor(AnalysisResultPtr ar, FileScopeRawPtr fsp) {
  MD5 md5 = fsp->getMd5();

  if (!Option::WholeProgram) {
    // The passed-in ar is only useful in whole-program mode, so create a
    // distinct ar to be used only for emission of this unit, and perform
    // unit-level (non-global) optimization.
    ar = AnalysisResultPtr(new AnalysisResult());
    fsp->setOuterScope(ar);

    ar->setPhase(AnalysisResult::AnalyzeAll);
    fsp->analyzeProgram(ar);
  }

  UnitEmitter* ue = emitHHBCUnitEmitter(ar, fsp, md5);
  assert(ue != nullptr);

  if (Option::GenerateTextHHBC) {
    // TODO(#2973538): Move HHBC text generation to after all the
    // units are created, and get rid of the LitstrTable locking,
    // since it won't be needed in that case.
    LitstrTable::get().mutex().lock();
    LitstrTable::get().setReading();
    std::unique_ptr<Unit> unit(ue->create());
    std::string fullPath = AnalysisResult::prepareFile(
      ar->getOutputPath().c_str(), Option::UserFilePrefix + fsp->getName(),
      true, false) + ".hhbc.txt";

    std::ofstream f(fullPath.c_str());
    if (!f) {
      Logger::Error("Unable to open %s for write", fullPath.c_str());
    } else {
      CodeGenerator cg(&f, CodeGenerator::TextHHBC);
      cg.printf("Hash: %" PRIx64 "%016" PRIx64 "\n", md5.q[0], md5.q[1]);
      cg.printRaw(unit->toString().c_str());
      f.close();
    }
    LitstrTable::get().setWriting();
    LitstrTable::get().mutex().unlock();
  }

  return ue;
}

class UEQ : public Synchronizable {
 public:
  void push(UnitEmitter* ue) {
    assert(ue != nullptr);
    Lock lock(this);
    m_ues.push_back(ue);
    notify();
  }
  UnitEmitter* tryPop(long sec, long long nsec) {
    Lock lock(this);
    if (m_ues.empty()) {
      // Check for empty() after wait(), in case of spurious wakeup.
      if (!wait(sec, nsec) || m_ues.empty()) {
        return nullptr;
      }
    }
    assert(m_ues.size() > 0);
    UnitEmitter* ue = m_ues.front();
    assert(ue != nullptr);
    m_ues.pop_front();
    return ue;
  }
 private:
  std::deque<UnitEmitter*> m_ues;
};
static UEQ s_ueq;

class EmitterWorker
  : public JobQueueWorker<FileScopeRawPtr, void*, true, true> {
 public:
  EmitterWorker() : m_ret(true) {}
  virtual void doJob(JobType job) {
    try {
      AnalysisResultPtr ar = ((AnalysisResult*)m_context)->shared_from_this();
      UnitEmitter* ue = emitHHBCVisitor(ar, job);
      if (Option::GenerateBinaryHHBC) {
        s_ueq.push(ue);
      } else {
        delete ue;
      }
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
      m_ret = false;
    } catch (...) {
      Logger::Error("Fatal: An unexpected exception was thrown");
      m_ret = false;
    }
  }
 private:
  bool m_ret;
};

static void addEmitterWorker(AnalysisResultPtr ar, StatementPtr sp,
                             void *data) {
  ((JobQueueDispatcher<EmitterWorker>*)data)->enqueue(sp->getFileScope());
}

static void commitGlobalData() {
  auto gd                     = Repo::GlobalData{};
  gd.HardTypeHints            = Option::HardTypeHints;
  gd.UsedHHBBC                = Option::UseHHBBC;
  gd.HardPrivatePropInference = true;

  Repo::get().saveGlobalData(gd);
}

/*
 * This is the entry point for offline bytecode generation.
 */
void emitAllHHBC(AnalysisResultPtr ar) {
  unsigned int threadCount = Option::ParserThreadCount;
  unsigned int nFiles = ar->getAllFilesVector().size();
  if (threadCount > nFiles) {
    threadCount = nFiles;
  }
  if (!threadCount) threadCount = 1;

  LitstrTable::get().setWriting();

  /* there is a race condition in the first call to
     makeStaticString. Make sure we dont hit it */
  {
    makeStaticString("");
    /* same for TypeConstraint */
    TypeConstraint tc;
  }

  JobQueueDispatcher<EmitterWorker>
    dispatcher(threadCount, true, 0, false, ar.get());

  dispatcher.start();
  ar->visitFiles(addEmitterWorker, &dispatcher);

  std::vector<std::unique_ptr<UnitEmitter>> ues;

  if (Option::GenerateBinaryHHBC) {
    // kBatchSize needs to strike a balance between reducing transaction commit
    // overhead (bigger batches are better), and limiting the cost incurred by
    // failed commits due to identical units that require rollback and retry
    // (smaller batches have less to lose).  Empirical results indicate that a
    // value in the 2-10 range is reasonable.
    static const unsigned kBatchSize = 8;

    // Gather up units created by the worker threads and commit them in
    // batches.
    bool didPop;
    bool inShutdown = false;
    while (true) {
      // Poll, but with a 100ms timeout so that this thread doesn't spin wildly
      // if it gets ahead of the workers.
      UnitEmitter* ue = s_ueq.tryPop(0, 100 * 1000 * 1000);
      if ((didPop = (ue != nullptr))) {
        ues.push_back(std::unique_ptr<UnitEmitter>{ue});
      }
      if (!Option::UseHHBBC &&
          (ues.size() == kBatchSize ||
           (!didPop && inShutdown && ues.size() > 0))) {
        batchCommit(std::move(ues));
      }
      if (!inShutdown) {
        inShutdown = dispatcher.pollEmpty();
      } else if (!didPop) {
        assert(Option::UseHHBBC || ues.size() == 0);
        break;
      }
    }

    if (!Option::UseHHBBC) commitGlobalData();
  } else {
    dispatcher.waitEmpty();
  }

  assert(Option::UseHHBBC || ues.empty());

  // We need to put the native func units in the repo so hhbbc can
  // find them, even though they won't be used at runtime.  (We just
  // regenerate them during process init.)
  auto nfunc = emitHHBCNativeFuncUnit(hhbc_ext_funcs, hhbc_ext_funcs_count);
  auto ncls  = emitHHBCNativeClassUnit(hhbc_ext_classes,
                                       hhbc_ext_class_count);
  ues.push_back(std::move(nfunc));
  ues.push_back(std::move(ncls));

  if (!Option::UseHHBBC) {
    batchCommit(std::move(ues));
    return;
  }

  ues = HHBBC::whole_program(std::move(ues));
  batchCommit(std::move(ues));
  commitGlobalData();
}

extern "C" {

String hphp_compiler_serialize_code_model_for(String code, String prefix) {
  AnalysisResultPtr ar(new AnalysisResult());
  auto statements = Parser::ParseString(code, ar, nullptr, false);
  if (statements != nullptr) {
    LabelScopePtr labelScope(new LabelScope());
    auto block = BlockStatementPtr(
      new BlockStatement(
        BlockScopePtr(), labelScope, statements->getLocation(), statements
      )
    );
    std::ostringstream serialized;
    CodeGenerator cg(&serialized, CodeGenerator::Output::CodeModel);
    cg.setAstClassPrefix(prefix.data());
    block->outputCodeModel(cg);
    std::string r(serialized.str().c_str(), serialized.str().length());
    return r;
  } else {
    return "";
  }
}

/**
 * This is the entry point from the runtime; i.e. online bytecode generation.
 * The 'filename' parameter may be NULL if there is no file associated with
 * the source code.
 *
 * Before being actually used, hphp_compiler_parse must be called with
 * a NULL `code' parameter to do initialization.
 */

Unit* hphp_compiler_parse(const char* code, int codeLen, const MD5& md5,
                          const char* filename) {
  if (UNLIKELY(!code)) {
    // Do initialization when code is null; see above.
    Option::EnableHipHopSyntax = RuntimeOption::EnableHipHopSyntax;
    Option::EnableZendCompat = RuntimeOption::EnableZendCompat;
    Option::JitEnableRenameFunction = RuntimeOption::EvalJitEnableRenameFunction;
    for (auto& i : RuntimeOption::DynamicInvokeFunctions) {
      Option::DynamicInvokeFunctions.insert(i);
    }
    Option::IntsOverflowToInts = RuntimeOption::IntsOverflowToInts;
    Option::RecordErrors = false;
    Option::ParseTimeOpts = false;
    Option::WholeProgram = false;
    Type::InitTypeHintMap();
    BuiltinSymbols::LoadSuperGlobals();
    TypeConstraint tc;
    return nullptr;
  }

  try {
    UnitOrigin unitOrigin = UnitOrigin::File;
    if (!filename) {
      filename = "";
      unitOrigin = UnitOrigin::Eval;
    }
    SCOPE_EXIT { SymbolTable::Purge(); };

    std::unique_ptr<UnitEmitter> ue;
    // Check if this file contains raw hip hop bytecode instead of
    // php.  This is dictated by a special file extension.
    if (RuntimeOption::EvalAllowHhas) {
      if (const char* dot = strrchr(filename, '.')) {
        const char hhbc_ext[] = "hhas";
        if (!strcmp(dot + 1, hhbc_ext)) {
          ue.reset(assemble_string(code, codeLen, filename, md5));
        }
      }
    }

    if (!ue) {
      auto parseit = [=] (AnalysisResultPtr ar) {
        Scanner scanner(code, codeLen,
                        RuntimeOption::GetScannerType(), filename);
        Parser parser(scanner, filename, ar, codeLen);
        parser.parse();
        return parser.getFileScope();
      };

      if (BuiltinSymbols::s_systemAr) {
        parseit(BuiltinSymbols::s_systemAr)->setMd5(md5);
      }

      AnalysisResultPtr ar(new AnalysisResult());
      FileScopePtr fsp = parseit(ar);
      fsp->setOuterScope(ar);

      ar->setPhase(AnalysisResult::AnalyzeAll);
      fsp->analyzeProgram(ar);

      ue.reset(emitHHBCUnitEmitter(ar, fsp, md5));
    }
    Repo::get().commitUnit(ue.get(), unitOrigin);

    auto const unit = ue->create();
    ue.reset();

    if (unit->sn() == -1) {
      // the unit was not committed to the Repo, probably because
      // another thread did it first. Try to use the winner.
      Unit* u = Repo::get().loadUnit(filename ? filename : "", md5);
      if (u != nullptr) {
        delete unit;
        return u;
      }
    }
    return unit;
  } catch (const std::exception&) {
    // extern "C" function should not be throwing exceptions...
    return nullptr;
  }
}

Unit* hphp_build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                                  ssize_t numBuiltinFuncs) {
  return emitHHBCNativeFuncUnit(builtinFuncs, numBuiltinFuncs)->create();
}

Unit* hphp_build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                                   ssize_t numBuiltinClasses) {
  return emitHHBCNativeClassUnit(builtinClasses, numBuiltinClasses)->create();
}

} // extern "C"

///////////////////////////////////////////////////////////////////////////////
}
}
