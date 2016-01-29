/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <algorithm>
#include <deque>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>

#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#ifndef _MSC_VER
#include <folly/Subprocess.h>
#endif
#include <folly/String.h>

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/expression/class_expression.h"
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
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/expression/null_coalesce_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/yield_expression.h"
#include "hphp/compiler/expression/yield_from_expression.h"
#include "hphp/compiler/expression/await_expression.h"
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
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/typedef_statement.h"
#include "hphp/compiler/statement/declare_statement.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/parallel.h"

#include "hphp/util/trace.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/logger.h"
#include "hphp/util/job-queue.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/ext_hhvm/ext_hhvm.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(emitter);

const StaticString
  s_ini_get("ini_get"),
  s_is_deprecated("deprecated function"),
  s_trigger_error("trigger_error"),
  s_trigger_sampled_error("trigger_sampled_error"),
  s_zend_assertions("zend.assertions"),
  s_HH_WaitHandle("HH\\WaitHandle"),
  s_result("result");

using uchar = unsigned char;

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
  static const char K = (char)0x80u; // Marker for information about a class base
  static const char Q = (char)0x90u; // NullSafe Property marker

  static const char CN = C | N;
  static const char CG = C | G;
  static const char CS = C | S;
  static const char LN = L | N;
  static const char LG = L | G;
  static const char LS = L | S;
  static const char AM = A | M;

  char GetSymFlavor(char sym) { return (sym & 0x0F); }
  char GetMarker(char sym) { return (sym & 0xF0); }

  /*
   * Return whether or not sym represents a symbolic stack element, rather than
   * an actual stack element. Symbolic stack elements do not have corresponding
   * values on the real eval stack at runtime, and represent things like local
   * variable ids or literal ints and strings.
   */
  bool IsSymbolic(char sym) {
    auto const flavor = GetSymFlavor(sym);
    if (flavor == L || flavor == T || flavor == I || flavor == H) return true;

    auto const marker = GetMarker(sym);
    if (marker == W || marker == K) return true;

    return false;
  }

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
      case StackSym::Q: res += "Q"; break;
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
  assertx(false);                                            \
} while (0)

// RAII guard for function creation.
class FuncFinisher {
  EmitterVisitor* m_ev;
  Emitter&        m_e;
  FuncEmitter*    m_fe;
  int32_t         m_stackPad;

 public:
  FuncFinisher(EmitterVisitor* ev, Emitter& e, FuncEmitter* fe,
               int32_t stackPad = 0)
    : m_ev(ev), m_e(e), m_fe(fe), m_stackPad(stackPad)
  {}

  ~FuncFinisher() {
    m_ev->finishFunc(m_e, m_fe, m_stackPad);
  }
};

// RAII guard for temporarily overriding an Emitter's location
class LocationGuard {
  Emitter& m_e;
  OptLocation m_loc;

public:
  LocationGuard(Emitter& e, const OptLocation& newLoc)
      : m_e(e), m_loc(e.getTempLocation()) {
    if (newLoc) m_e.setTempLocation(newLoc);
  }
  ~LocationGuard() {
    m_e.setTempLocation(m_loc);
  }
};

#define O(name, imm, pop, push, flags)                                  \
  void Emitter::name(imm) {                                             \
    auto const opcode = Op::name;                                       \
    ITRACE(2, "{}\n", #name);                                           \
    Trace::Indent indent;                                               \
    ITRACE(3, "before: {}\n", m_ev.getEvalStack().pretty());            \
    /* Process opcode's effects on the EvalStack and emit it */         \
    Offset curPos UNUSED = getUnitEmitter().bcPos();                    \
    {                                                                   \
      Trace::Indent indent;                                             \
      getEmitterVisitor().prepareEvalStack();                           \
      char idxAPop UNUSED;                                              \
      POP_##pop;                                                        \
      const int nIn UNUSED = COUNT_##pop;                               \
      POP_LA_##imm;                                                     \
      PUSH_##push;                                                      \
      getUnitEmitter().emitOp(Op##name);                                \
      IMPL_##imm;                                                       \
    }                                                                   \
    ITRACE(3, "after: {}\n", m_ev.getEvalStack().pretty());             \
    auto& loc = m_tempLoc ? *m_tempLoc : m_node->getRange();            \
    auto UNUSED pc = m_ue.bc() + curPos;                                \
    ITRACE(2, "lines [{},{}] chars [{},{}]\n",                          \
           loc.line0, loc.line1, loc.char0, loc.char1);                 \
    /* Update various other metadata */                                 \
    getUnitEmitter().recordSourceLocation(loc, curPos);                 \
    if (flags & TF) {                                                   \
      getEmitterVisitor().restoreJumpTargetEvalStack();                 \
      ITRACE(3, "   jmp: {}\n", m_ev.getEvalStack().pretty());          \
    }                                                                   \
    if (opcode == Op::FCall) getEmitterVisitor().recordCall();          \
    getEmitterVisitor().setPrevOpcode(opcode);                          \
  }

#define COUNT_NOV 0
#define COUNT_ONE(t) 1
#define COUNT_TWO(t1,t2) 2
#define COUNT_THREE(t1,t2,t3) 3
#define COUNT_FOUR(t1,t2,t3,t4) 4
#define COUNT_MFINAL 0
#define COUNT_F_MFINAL 0
#define COUNT_C_MFINAL 0
#define COUNT_V_MFINAL 0
#define COUNT_FMANY 0
#define COUNT_CVUMANY 0
#define COUNT_CMANY 0
#define COUNT_SMANY 0
#define COUNT_IDX_A 0

#define ONE(t) \
  DEC_##t a1
#define TWO(t1, t2) \
  DEC_##t1 a1, DEC_##t2 a2
#define THREE(t1, t2, t3) \
  DEC_##t1 a1, DEC_##t2 a2, DEC_##t3 a3
#define FOUR(t1, t2, t3, t4) \
  DEC_##t1 a1, DEC_##t2 a2, DEC_##t3 a3, DEC_##t4 a4
#define NA
#define DEC_BLA std::vector<Label*>&
#define DEC_SLA std::vector<StrOff>&
#define DEC_ILA std::vector<IterPair>&
#define DEC_IVA int32_t
#define DEC_LA int32_t
#define DEC_IA int32_t
#define DEC_I64A int64_t
#define DEC_DA double
#define DEC_SA const StringData*
#define DEC_RATA RepoAuthType
#define DEC_AA ArrayData*
#define DEC_BA Label&
#define DEC_OA(type) type
#define DEC_VSA std::vector<std::string>&
#define DEC_KA MemberKey

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
#define POP_MFINAL \
  getEmitterVisitor().popEvalStackMMany()
#define POP_F_MFINAL POP_MFINAL
#define POP_C_MFINAL \
  getEmitterVisitor().popEvalStack(StackSym::C); \
  getEmitterVisitor().popEvalStackMMany()
#define POP_V_MFINAL \
  getEmitterVisitor().popEvalStack(StackSym::V); \
  getEmitterVisitor().popEvalStackMMany()
#define POP_FMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::F)
#define POP_CVUMANY \
  getEmitterVisitor().popEvalStackCVMany(a1)
#define POP_CMANY \
  getEmitterVisitor().popEvalStackMany(a1, StackSym::C)
#define POP_SMANY \
  getEmitterVisitor().popEvalStackMany(a1.size(), StackSym::C)
#define POP_IDX_A \
  idxAPop = getEmitterVisitor().getEvalStack().top();     \
  if (a2 == 1) getEmitterVisitor().popEvalStackCVMany(1); \
  getEmitterVisitor().popEvalStack(StackSym::A)

#define POP_CV(i) getEmitterVisitor().popEvalStack(StackSym::C)
#define POP_VV(i) getEmitterVisitor().popEvalStack(StackSym::V)
#define POP_AV(i) getEmitterVisitor().popEvalStack(StackSym::A)
#define POP_RV(i) getEmitterVisitor().popEvalStack(StackSym::R)
#define POP_FV(i) getEmitterVisitor().popEvalStack(StackSym::F)

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
#define POP_LA_BLA(i)
#define POP_LA_SLA(i)
#define POP_LA_ILA(i)
#define POP_LA_IVA(i)
#define POP_LA_IA(i)
#define POP_LA_I64A(i)
#define POP_LA_DA(i)
#define POP_LA_SA(i)
#define POP_LA_RATA(i)
#define POP_LA_AA(i)
#define POP_LA_BA(i)
#define POP_LA_IMPL(x)
#define POP_LA_OA(i) POP_LA_IMPL
#define POP_LA_VSA(i)
#define POP_LA_KA(i)

#define POP_LA_LA(i) \
  getEmitterVisitor().popSymbolicLocal(opcode)

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
#define PUSH_CUV PUSH_CV
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

#define PUSH_IDX_A \
  if (a2 == 1) getEmitterVisitor().pushEvalStack(idxAPop);

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

// Emitting RATAs isn't supported here right now.  (They're only
// created in hhbbc.)
#define IMPL_RATA(var) not_reached()
#define IMPL1_RATA IMPL_RATA(a1)
#define IMPL2_RATA IMPL_RATA(a2)
#define IMPL3_RATA IMPL_RATA(a3)
#define IMPL4_RATA IMPL_RATA(a4)

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

#define IMPL_KA(var) encode_member_key(var, getUnitEmitter())
#define IMPL1_KA IMPL_KA(a1)
#define IMPL2_KA IMPL_KA(a2)
#define IMPL3_KA IMPL_KA(a3)
#define IMPL4_KA IMPL_KA(a4)

 OPCODES

#undef O
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef NA
#undef DEC_IVA
#undef DEC_LA
#undef DEC_IA
#undef DEC_I64A
#undef DEC_DA
#undef DEC_SA
#undef DEC_RATA
#undef DEC_AA
#undef DEC_BA
#undef DEC_OA
#undef DEC_KA
#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE
#undef POP_FOUR
#undef POP_MFINAL
#undef POP_F_MFINAL
#undef POP_C_MFINAL
#undef POP_V_MFINAL
#undef POP_CV
#undef POP_VV
#undef POP_HV
#undef POP_AV
#undef POP_RV
#undef POP_FV
#undef POP_LREST
#undef POP_FMANY
#undef POP_CVUMANY
#undef POP_CMANY
#undef POP_SMANY
#undef POP_IDX_A
#undef POP_LA_ONE
#undef POP_LA_TWO
#undef POP_LA_THREE
#undef POP_LA_FOUR
#undef POP_LA_NA
#undef POP_LA_IVA
#undef POP_LA_IA
#undef POP_LA_I64A
#undef POP_LA_DA
#undef POP_LA_SA
#undef POP_LA_RATA
#undef POP_LA_AA
#undef POP_LA_BA
#undef POP_LA_IMPL
#undef POP_LA_OA
#undef POP_LA_LA
#undef POP_LA_KA
#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_THREE
#undef PUSH_FOUR
#undef PUSH_CV
#undef PUSH_UV
#undef PUSH_CUV
#undef PUSH_VV
#undef PUSH_HV
#undef PUSH_AV
#undef PUSH_RV
#undef PUSH_FV
#undef PUSH_IDX_A
#undef IMPL_ONE
#undef IMPL_TWO
#undef IMPL_THREE
#undef IMPL_FOUR
#undef IMPL_NA
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
#undef IMPL_RATA
#undef IMPL1_RATA
#undef IMPL2_RATA
#undef IMPL3_RATA
#undef IMPL4_RATA
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
#undef IMPL_KA
#undef IMPL1_KA
#undef IMPL2_KA
#undef IMPL3_KA
#undef IMPL4_KA

static void checkJmpTargetEvalStack(const SymbolicStack& source,
                                    const SymbolicStack& dest) {
  if (source.size() != dest.size()) {
    Logger::FWarning("Emitter detected a point in the bytecode where the "
                     "depth of the stack is not the same for all possible "
                     "control flow paths. source size: {}. dest size: {}",
                     source.size(),
                     dest.size());
    Logger::Warning("src stack : %s", source.pretty().c_str());
    Logger::Warning("dest stack: %s", dest.pretty().c_str());
    assertx(false);
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

std::string SymbolicStack::SymEntry::pretty() const {
  std::string ret;
  ret += StackSym::ToString(sym);
  char flavor = StackSym::GetSymFlavor(sym);
  if (flavor == StackSym::L || flavor == StackSym::I) {
    folly::toAppend(':', intval, &ret);
  } else if (flavor == StackSym::T && name) {
    folly::toAppend(':', name->data(), &ret);
  }
  return ret;
}

std::string SymbolicStack::pretty() const {
  std::ostringstream out;
  out << "[" << std::hex;
  size_t j = 0;
  auto sep = "";
  for (size_t i = 0; i < m_symStack.size(); ++i) {
    out << sep;
    sep = " ";
    while (j < m_actualStack.size() && m_actualStack[j] < int(i)) {
      ++j;
    }
    if (j < m_actualStack.size() && m_actualStack[j] == int(i)) {
      out << "*";
    }
    out << m_symStack[i].pretty();
  }
  out << ']';
  return out.str();
}

void SymbolicStack::push(char sym) {
  if (!StackSym::IsSymbolic(sym)) {
    m_actualStack.push_back(m_symStack.size());
    *m_actualStackHighWaterPtr = std::max(*m_actualStackHighWaterPtr,
                                          (int)m_actualStack.size());
  }
  m_symStack.push_back(SymEntry(sym));
  ITRACE(4, "push: {}\n", m_symStack.back().pretty());
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
  ITRACE(4, "pop: {}\n", m_symStack.back().pretty());
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
  return m_symStack[index].name;
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
  assert(!se.name || se.name == s);
  se.name = s;
}

void SymbolicStack::setKnownCls(const StringData* s, bool nonNull) {
  assert(m_symStack.size());
  SymEntry& se = m_symStack.back();
  assert(!se.className || se.className == s);
  se.className = s;
}

void SymbolicStack::setInt(int64_t v) {
  assert(m_symStack.size());
  m_symStack.back().intval = v;
}

void SymbolicStack::cleanTopMeta() {
  SymEntry& se = m_symStack.back();
  se.clsBaseType = CLS_INVALID;
  se.name = nullptr;
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
  ITRACE(4, "   set: {} -> {}\n", index, m_symStack[index].pretty());
}

size_t SymbolicStack::size() const {
  return m_symStack.size();
}

size_t SymbolicStack::actualSize() const {
  return m_actualStack.size();
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
  not_reached();
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
  *m_fdescHighWaterPtr = std::max(*m_fdescHighWaterPtr, m_fdescCount);
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
      ITRACE(3, "bind: {}\n", m_evalStack.pretty());
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

void EmitterVisitor::emitJump(Emitter& e, IterVec& iters, Label& target) {
  if (!iters.empty()) {
    e.IterBreak(iters, target);
    iters.clear();
  } else {
    e.Jmp(target);
  }
}

void EmitterVisitor::emitReturn(Emitter& e, char sym, StatementPtr s) {
  Region* region = m_regions.back().get();
  registerReturn(s, region, sym);
  assert(getEvalStack().size() == 1);
  assert(region->m_returnTargets.count(sym));
  IterVec iters;
  for (Region* r = region; true; r = r->m_parent.get()) {
    auto& t = r->m_returnTargets[sym].target;
    if (r->m_parent == nullptr) {
      // At the top of the hierarchy, no more finally blocks to run.
      // Check return type, free pending iterators and actually return.
      if (sym == StackSym::C) {
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeC();
        }
        // IterFree must come after VerifyRetType, because VerifyRetType may
        // throw, in which case any Iters will be freed by the fault funclet.
        emitIterFree(e, iters);
        e.RetC();
      } else {
        assert(sym == StackSym::V);
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeV();
        }
        emitIterFree(e, iters);
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
    if (r->isFinally()) {
      throw EmitterVisitor::IncludeTimeFatalException(s,
        "Goto to a label outside a finally block is not supported");
    }
    if (r->isTryFinally()) {
      // We came across a try region, need to run a finally block.
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
  Id stateLocal = getStateLocal();
  emitVirtualLocal(stateLocal);
  e.IssetL(stateLocal);
  e.JmpZ(after);
  if (count >= 3) {
    // A switch is needed since there are more than two cases.
    emitVirtualLocal(stateLocal);
    e.CGetL(stateLocal);
    e.Switch(cases, 0, SwitchKind::Unbounded);
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
    // Some cases might get assigned state numbers but not actually
    // occur in the try block. We need to set /some/ target for them,
    // so point them here.
    if (!c->isSet()) c->set(e);
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
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeC();
        }
        e.RetC();
      } else {
        assert(sym == StackSym::V);
        e.VGetL(retLocal);
        if (shouldEmitVerifyRetType()) {
          e.VerifyRetTypeV();
        }
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
    if (region->m_gotoLabels.count(name)) {
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
    // Error, because the label is crossing a finally
    if (region->isFinally()) {
        throw EmitterVisitor::IncludeTimeFatalException(e.getNode(),
          "jump out of a finally block is disallowed");
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

bool EmitterVisitor::shouldEmitVerifyRetType() {
  return (m_curFunc->retTypeConstraint.hasConstraint() &&
          !m_curFunc->isGenerator);
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
// EmitterVisitor.

EmitterVisitor::EmittedClosures EmitterVisitor::s_emittedClosures;

EmitterVisitor::EmitterVisitor(UnitEmitter& ue)
  : m_ue(ue), m_curFunc(ue.getMain()),
    m_evalStackIsUnknown(false),
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

void EmitterVisitor::popEvalStack(char expected) {
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

void EmitterVisitor::popSymbolicLocal(Op op) {
  // A number of member instructions read locals without consuming an L from
  // the symbolic stack through the normal path.
  if (isMemberBaseOp(op) || isMemberDimOp(op) || isMemberFinalOp(op)) {
    return;
  }

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
    popEvalStack(StackSym::L);
  }
}

void EmitterVisitor::popEvalStackMMany() {
  ITRACE(3, "popEvalStackMMany()\n");
  Trace::Indent i;

  ITRACE(3, "popping member codes\n");
  while (!m_evalStack.empty()) {
    char sym = m_evalStack.top();
    char symFlavor = StackSym::GetSymFlavor(sym);
    char marker = StackSym::GetMarker(sym);
    if (marker == StackSym::E || marker == StackSym::P ||
        marker == StackSym::Q) {
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

  ITRACE(3, "popping location\n");
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
  m_curFunc->containsCalls = true;
}

bool EmitterVisitor::isJumpTarget(Offset target) {
  // Returns true iff one of the following conditions is true:
  //   1) We have seen an instruction that jumps to the specified offset
  //   2) We know of a Label that has been set to the specified offset
  //   3) We have seen a try region that ends at the specified offset
  auto it = m_jumpTargetEvalStacks.find(target);
  return (it != m_jumpTargetEvalStacks.end());
}

class IterFreeThunklet final : public Thunklet {
public:
  IterFreeThunklet(Id iterId, bool itRef)
    : m_id(iterId), m_itRef(itRef) {}
  void emit(Emitter& e) override {
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
class RestoreErrorReportingThunklet final : public Thunklet {
public:
  explicit RestoreErrorReportingThunklet(Id loc)
    : m_oldLevelLoc(loc) {}
  void emit(Emitter& e) override {
    e.getEmitterVisitor().emitRestoreErrorReporting(e, m_oldLevelLoc);
    e.Unwind();
  }
private:
  Id m_oldLevelLoc;
};

class UnsetUnnamedLocalThunklet final : public Thunklet {
public:
  explicit UnsetUnnamedLocalThunklet(Id loc)
    : m_loc(loc) {}
  void emit(Emitter& e) override {
    e.getEmitterVisitor().emitVirtualLocal(m_loc);
    e.getEmitterVisitor().emitUnset(e);
    e.Unwind();
  }
private:
  Id m_loc;
};

class UnsetUnnamedLocalsThunklet final : public Thunklet {
public:
  explicit UnsetUnnamedLocalsThunklet(std::vector<Id>&& locs)
    : m_locs(std::move(locs)) {}
  void emit(Emitter& e) override {
    auto& visitor = e.getEmitterVisitor();
    for (auto loc : m_locs) {
      visitor.emitVirtualLocal(loc);
      visitor.emitUnset(e);
    }
    e.Unwind();
  }
private:
  const std::vector<Id> m_locs;
};

class UnsetGeneratorDelegateThunklet final : public Thunklet {
public:
  explicit UnsetGeneratorDelegateThunklet(Id iterId)
    : m_id(iterId) {}
  void emit(Emitter& e) override {
    e.ContUnsetDelegate(m_id, true);
    e.Unwind();
  }
private:
  Id m_id;
};

class FinallyThunklet final : public Thunklet {
public:
  explicit FinallyThunklet(FinallyStatementPtr finallyStatement,
                           int numLiveIters)
      : m_finallyStatement(finallyStatement), m_numLiveIters(numLiveIters) {}
  void emit(Emitter& e) override {
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
void EmitterVisitor::listAssignmentVisitLHS(Emitter& e, ExpressionPtr exp,
                                            IndexChain& indexChain,
                                            std::vector<IndexPair>& all) {
  if (!exp) {
    // Empty slot
    return;
  }

  if (exp->is(Expression::KindOfListAssignment)) {
    // Nested assignment
    auto la = static_pointer_cast<ListAssignment>(exp);
    auto lhs = la->getVariables();
    int n = lhs->getCount();
    for (int i = 0; i < n; ++i) {
      indexChain.push_back(i);
      listAssignmentVisitLHS(e, (*lhs)[i], indexChain, all);
      indexChain.pop_back();
    }
  } else {
    // Reached a "leaf".  Lock in this index chain and deal with this exp.
    assert(!indexChain.empty());
    all.emplace_back(exp, IndexChain(indexChain));

    // First: the order we visit the LHS elements matters, as does whether we
    // do the RHS or LHS first, for things like:
    // list($a[$n++], $b[$n++]) = $c[$n++]
    //
    // In PHP5 mode, we visit the LHS elements of the list() now. This does
    // two things: it causes their side effects to happen in LTR order, but
    // since they are pushed onto the m_evalStack they are actually asigned to
    // in LIFO order, e.g., RTL, in listAssignmentAssignElements below.
    //
    // In PHP7 mode, we need to visit the elements in LTR order so their side
    // effects take place in that order, but we *also* need to assign them in
    // LTR order, so we can't push them onto the m_evalStack right now. Since
    // visit() does both of these things, we need to delay calling visit()
    // until listAssignmentAssignElements below. This also has the side effect
    // of making isRhsFirst() effectively always on in PHP7 mode when doing
    // list() assignment (since we delay the visit() until after the check
    // anyways), which turns out to be the right behavior.
    if (!RuntimeOption::PHP7_LTR_assign) {
      visit(exp);
      emitClsIfSPropBase(e);
    }
  }
}

void EmitterVisitor::listAssignmentAssignElements(
  Emitter& e,
  std::vector<IndexPair>& indexPairs,
  std::function<void()> emitSrc
) {

  // PHP5 does list() assignments RTL, PHP7 does them LTR, so this loop can go
  // either way and looks a little ugly. The assignment order normally isn't
  // visible, but it is if you do something like:
  // list($a[], $a[]) = $foo
  auto const ltr = RuntimeOption::PHP7_LTR_assign;
  for (int i = ltr ? 0 : (int)indexPairs.size() - 1;
       i >= 0 && i < (int)indexPairs.size();
       ltr ? i++ : i--) {
    if (ltr) {
      // Visit now, so we can both eval LTR and assign LTR. See comment in
      // listAssignmentVisitLHS.
      visit(indexPairs[i].first);
      emitClsIfSPropBase(e);
    }

    IndexChain& currIndexChain = indexPairs[i].second;
    if (currIndexChain.empty()) {
      continue;
    }

    if (emitSrc == nullptr) {
      e.Null();
    } else {
      emitSrc();
      for (int j = 0; j < (int)currIndexChain.size(); ++j) {
        m_evalStack.push(StackSym::I);
        m_evalStack.setInt(currIndexChain[j]);
        markElem(e);
      }
      emitCGet(e);
    }

    emitSet(e);
    emitPop(e);
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

  auto binOpNode = dynamic_pointer_cast<BinaryOpExpression>(cond);

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

  auto unOpNode = dynamic_pointer_cast<UnaryOpExpression>(cond);
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
  m_ue.m_filepath = makeStaticString(filename);
  m_ue.m_isHHFile = file->isHHFile();
  m_ue.m_useStrictTypes = file->useStrictTypes();

  FunctionScopePtr func(file->getPseudoMain());
  if (!func) return;

  SCOPE_ASSERT_DETAIL("visit FileScope") { return m_evalStack.pretty(); };
  ITRACE(1, "Emitting file {}\n", file->getName());
  Trace::Indent indent;

  m_file = file;
  assignLocalVariableIds(func);

  AnalysisResultPtr ar(file->getContainingProgram());
  assert(ar);
  auto m = dynamic_pointer_cast<MethodStatement>(func->getStmt());
  if (!m) return;
  StatementListPtr stmts(m->getStmts());
  if (!stmts) return;

  Emitter e(m, m_ue, *this);

  int i, nk = stmts->getCount();
  for (i = 0; i < nk; i++) {
    StatementPtr s = (*stmts)[i];
    if (auto meth = dynamic_pointer_cast<MethodStatement>(s)) {
      // Emit afterwards
      postponeMeth(meth, nullptr, true);
    }
  }
  {
    FunctionScopePtr fsp = m->getFunctionScope();
    if (fsp->needsLocalThis()) {
      static const StringData* thisStr = makeStaticString("this");
      Id thisId = m_curFunc->lookupVarId(thisStr);
      emitVirtualLocal(thisId);
      e.InitThisLoc(thisId);
    }
    if (fsp->needsFinallyLocals()) {
      assignFinallyVariableIds();
    }
    FuncFinisher ff(this, e, m_curFunc);
    TypedValue mainReturn;
    mainReturn.m_type = kInvalidDataType;
    bool notMergeOnly = false;

    if (Option::UseHHBBC && SystemLib::s_inited) notMergeOnly = true;

    auto region = createRegion(stmts, Region::Kind::Global);
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };

    for (auto cls : m_file->getAnonClasses()) {
      emitClass(e, cls->getClassScope(), true);
    }

    for (i = 0; i < nk; i++) {
      StatementPtr s = (*stmts)[i];
      e.setTempLocation(s->getRange());
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
        case Construct::KindOfDeclareStatement: {
          auto ds = static_pointer_cast<DeclareStatement>(s);
          for (auto& decl : ds->getDeclareMap()) {
            if (decl.first == "strict_types") {
              if (ds->getBlock()->getStmts()->getCount()) {
                emitMakeUnitFatal(e, "strict_types declaration must not use "
                                  "block mode");
                break;
              }
              if (!RuntimeOption::PHP7_ScalarTypes) {
                emitMakeUnitFatal(e, "strict_types can only be used when "
                                  "hhvm.php7.scalar_types = true");
                break;
              }
            }
          }

          visit(ds->getBlock());
          break;
        }
        case Statement::KindOfTypedefStatement: {
          auto const id =
            emitTypedef(e, static_pointer_cast<TypedefStatement>(s));
          m_ue.pushMergeableTypeAlias(Unit::MergeKind::TypeAlias, id);
          break;
        }
        case Statement::KindOfReturnStatement:
          if (mainReturn.m_type != kInvalidDataType) break;

          visit(s);
          if (notMergeOnly) {
            tvWriteUninit(&mainReturn);
            m_ue.m_returnSeen = true;
            continue;
          }

          {
            auto r = static_pointer_cast<ReturnStatement>(s);
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
            } else {
              assert(v.isInitialized());
              assert(!isRefcountedType(v.getType()));
            }
            mainReturn = *v.asCell();
            m_ue.m_returnSeen = true;
          }
          break;
        case Statement::KindOfExpStatement:
          if (mainReturn.m_type == kInvalidDataType) {
            auto e = static_pointer_cast<ExpStatement>(s)->getExpression();
            switch (e->getKindOf()) {
              case Expression::KindOfSimpleFunctionCall: {
                auto func = static_pointer_cast<SimpleFunctionCall>(e);
                StringData *name;
                TypedValue tv;
                if (func->isSimpleDefine(&name, &tv)) {
                  auto k = func->isDefineWithoutImpl(ar)
                    ? Unit::MergeKind::PersistentDefine
                    : Unit::MergeKind::Define;
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
                auto ae = static_pointer_cast<AssignmentExpression>(e);
                StringData *name;
                TypedValue tv;
                if (ae->isSimpleGlobalAssign(&name, &tv)) {
                  m_ue.pushMergeableDef(Unit::MergeKind::Global, name, tv);
                  visit(s);
                  continue;
                }
                break;
              }
              case Expression::KindOfIncludeExpression: {
                auto inc = static_pointer_cast<IncludeExpression>(e);
                if (inc->isReqLit()) {
                  if (FileScopeRawPtr f = inc->getIncludedFile(ar)) {
                    if (StatementListPtr sl = f->getStmt()) {
                      FunctionScopeRawPtr ps DEBUG_ONLY =
                        sl->getFunctionScope();
                      assert(ps && ps->inPseudoMain());
                      m_ue.pushMergeableInclude(
                        Unit::MergeKind::ReqDoc,
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
          if (mainReturn.m_type != kInvalidDataType) break;
          notMergeOnly = true;
          visit(s);
      }
    }

    if (!notMergeOnly) {
      m_ue.m_mergeOnly = true;
      if (mainReturn.m_type == kInvalidDataType) {
        tvWriteUninit(&mainReturn);
        if (boost::algorithm::ends_with(filename, EVAL_FILENAME_SUFFIX)) {
          tvAsVariant(&mainReturn) = init_null();
        } else {
          tvAsVariant(&mainReturn) = 1;
        }
      }
      m_ue.m_mainReturn = mainReturn;
    }

    // Pseudo-main returns the integer value 1 by default. If the
    // current position in the bytecode is reachable, emit code to
    // return 1.
    if (currentPositionIsReachable()) {
      Location::Range loc;
      if (m_ue.bcPos() > 0) loc.line0 = -1;
      e.setTempLocation(loc);
      if (boost::algorithm::ends_with(filename, EVAL_FILENAME_SUFFIX)) {
        e.Null();
      } else {
        e.Int(1);
      }
      e.RetC();
      e.setTempLocation(OptLocation());
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
}

static StringData* getClassName(ExpressionPtr e) {
  ClassScopeRawPtr cls;
  if (e->isThis()) {
    cls = e->getClassScope();
  }
  if (cls && !cls->isTrait()) {
    return makeStaticString(cls->getScopeName());
  }
  return nullptr;
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
  if (builtinFunc) {
    ref = (builtinFunc->attrs() & AttrReference) != 0;
  } else if (fn->isValid() && fn->getFuncScope()) {
    ref = fn->getFuncScope()->isRefReturn();
  } else if (!fn->getOriginalName().empty()) {
    FunctionScope::FunctionInfoPtr fi =
      FunctionScope::GetFunctionInfo(fn->getOriginalName());
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
}

void EmitterVisitor::visitKids(ConstructPtr c) {
  for (int i = 0, nk = c->getKidCount(); i < nk; i++) {
    ConstructPtr kid(c->getNthKid(i));
    visit(kid);
  }
}

template<typename ArrayType, class Fun>
bool checkKeys(ExpressionPtr init_expr, bool check_size, Fun fun) {
  if (init_expr->getKindOf() != Expression::KindOfExpressionList) {
    return false;
  }

  auto el = static_pointer_cast<ExpressionList>(init_expr);
  int n = el->getCount();
  if (n < 1 || (check_size && n > ArrayType::MaxMakeSize)) {
    return false;
  }

  for (int i = 0, n = el->getCount(); i < n; ++i) {
    ExpressionPtr ex = (*el)[i];
    if (ex->getKindOf() != Expression::KindOfArrayPairExpression) {
      return false;
    }
    auto ap = static_pointer_cast<ArrayPairExpression>(ex);
    if (ap->isRef()) return false;
    if (!fun(ap)) return false;
  }
  return true;
}

/*
 * isPackedInit() returns true if this expression list looks like an
 * array with no keys and no ref values; e.g. array(x,y,z).
 *
 * In this case we can NewPackedArray to create the array. The elements are
 * pushed on the stack, so we arbitrarily limit this to a small multiple of
 * MixedArray::SmallSize (12).
 */
bool isPackedInit(ExpressionPtr init_expr, int* size,
                  bool check_size = true) {
  *size = 0;
  return checkKeys<MixedArray>(init_expr, check_size,
    [&](ArrayPairExpressionPtr ap) {
      Variant key;

      // If we have a key...
      if (ap->getName() != nullptr) {
        // ...and it has no scalar value, bail.
        if (!ap->getName()->getScalarValue(key)) return false;

        if (key.isInteger()) {
          // If it's an integer key, check if it's the next packed index.
          if (key.asInt64Val() != *size) return false;
        } else if (key.isBoolean()) {
          // Bool to Int conversion
          if (static_cast<int>(key.asBooleanVal()) != *size) return false;
        } else {
          // Give up if it's not a string.
          if (!key.isString()) return false;

          int64_t i; double d;
          auto numtype = key.getStringData()->isNumericWithVal(i, d, false);

          // If it's a string of the next packed index,
          if (numtype != KindOfInt64 || i != *size) return false;
        }
      }

      (*size)++;
      return true;
    });
}

/*
 * isStructInit() is like isPackedInit(), but returns true if the keys are
 * all static strings with no duplicates.
 */
bool isStructInit(ExpressionPtr init_expr, std::vector<std::string>& keys) {
  return checkKeys<StructArray>(init_expr, true,
    [&](ArrayPairExpressionPtr ap) {
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

void EmitterVisitor::emitCall(Emitter& e,
                              FunctionCallPtr func,
                              ExpressionListPtr params,
                              Offset fpiStart) {
  auto const numParams = params ? params->getCount() : 0;
  auto const unpack = func->hasUnpack();
  if (!func->checkUnpackParams()) {
    throw IncludeTimeFatalException(
      func, "Only the last parameter in a function call is allowed to use ...");
  }
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    for (int i = 0; i < numParams; i++) {
      auto param = (*params)[i];
      emitFuncCallArg(e, param, i, param->isUnpack());
    }
  }

  if (unpack) {
    e.FCallUnpack(numParams);
  } else {
    e.FCall(numParams);
  }
}

bool EmitterVisitor::visit(ConstructPtr node) {
  if (!node) return false;

  SCOPE_ASSERT_DETAIL("visit Construct") { return node->getText(); };

  Emitter e(node, m_ue, *this);

  switch (node->getKindOf()) {
  case Construct::KindOfBlockStatement:
  case Construct::KindOfStatementList:
    visitKids(node);
    return false;

  case Construct::KindOfTypedefStatement: {
    emitMakeUnitFatal(e, "Type statements are currently only allowed at "
                         "the top-level");
    return false;
  }

  case Construct::KindOfDeclareStatement: {
    auto ds = static_pointer_cast<DeclareStatement>(node);
    for (auto& decl : ds->getDeclareMap()) {
      if (decl.first == "strict_types") {
        emitMakeUnitFatal(e, "strict_types declaration must not use "
                          "block mode");
      }
    }

    visit(ds->getBlock());
    return false;
  }

  case Construct::KindOfContinueStatement:
  case Construct::KindOfBreakStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto bs = static_pointer_cast<BreakStatement>(s);
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

    if (bs->is(Construct::KindOfBreakStatement)) {
      emitBreak(e, destLevel, bs);
    } else {
      emitContinue(e, destLevel, bs);
    }

    return false;
  }

  case Construct::KindOfDoStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto ds = static_pointer_cast<DoStatement>(s);

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

  case Construct::KindOfCaseStatement: {
    // Should never be called. Handled in visitSwitch.
    not_reached();
  }

  case Construct::KindOfCatchStatement: {
    // Store the current exception object in the appropriate local variable
    auto cs = static_pointer_cast<CatchStatement>(node);
    StringData* vName = makeStaticString(cs->getVariable()->getName());
    Id i = m_curFunc->lookupVarId(vName);
    emitVirtualLocal(i);
    e.Catch();
    emitSet(e);
    emitPop(e);
    visit(cs->getStmt());
    return false;
  }

  case Construct::KindOfEchoStatement: {
    auto es = static_pointer_cast<EchoStatement>(node);
    auto exps = es->getExpressionList();
    int count = exps->getCount();
    for (int i = 0; i < count; i++) {
      visit((*exps)[i]);
      emitConvertToCell(e);
      e.Print();
      e.PopC();
    }
    return false;
  }

  case Construct::KindOfExpStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto es = static_pointer_cast<ExpStatement>(s);
    if (visit(es->getExpression())) {
      emitPop(e);
    }
    return false;
  }

  case Construct::KindOfForStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto fs = static_pointer_cast<ForStatement>(s);

    if (visit(fs->getInitExp())) {
      emitPop(e);
    }
    Label preCond(e);
    Label& preInc = registerContinue(fs, region.get(), 1, false)->m_label;
    Label& fail = registerBreak(fs, region.get(), 1, false)->m_label;
    ExpressionPtr condExp = fs->getCondExp();
    auto emit_cond = [&] (Label& tru, bool truFallthrough) {
      if (!condExp) return;
      Emitter condEmitter(condExp, m_ue, *this);
      visitIfCondition(condExp, condEmitter, tru, fail, truFallthrough);
    };
    Label top;
    emit_cond(top, true);
    top.set(e);
    {
      enterRegion(region);
      SCOPE_EXIT { leaveRegion(region); };
      visit(fs->getBody());
    }
    preInc.set(e);
    if (visit(fs->getIncExp())) {
      emitPop(e);
    }
    if (!condExp) {
      e.Jmp(top);
    } else {
      emit_cond(top, false);
    }
    if (fail.isUsed()) fail.set(e);
    return false;
  }

  case Construct::KindOfForEachStatement: {
    auto fe = static_pointer_cast<ForEachStatement>(node);
    if (fe->isAwaitAs()) {
      emitForeachAwaitAs(e, fe);
    } else {
      emitForeach(e, fe);
    }
    return false;
  }

  case Construct::KindOfGlobalStatement: {
    auto vars = static_pointer_cast<GlobalStatement>(node)->getVars();
    for (int i = 0, n = vars->getCount(); i < n; i++) {
      ExpressionPtr var((*vars)[i]);
      if (var->is(Construct::KindOfSimpleVariable)) {
        auto sv = static_pointer_cast<SimpleVariable>(var);
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
      } else if (var->is(Construct::KindOfDynamicVariable)) {
        // global $<exp> =& $GLOBALS[<exp>]
        auto dv = static_pointer_cast<DynamicVariable>(var);
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

  case Construct::KindOfIfStatement: {
    auto ifp = static_pointer_cast<IfStatement>(node);
    StatementListPtr branches(ifp->getIfBranches());
    int nb = branches->getCount();
    Label done;
    for (int i = 0; i < nb; i++) {
      auto branch = static_pointer_cast<IfBranchStatement>((*branches)[i]);
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

  case Construct::KindOfIfBranchStatement:
    not_reached(); // handled by KindOfIfStatement

  case Construct::KindOfReturnStatement: {
    auto r = static_pointer_cast<ReturnStatement>(node);

    char retSym = StackSym::C;
    if (visit(r->getRetExp())) {
      if (r->getRetExp()->getContext() & Expression::RefValue &&
          // Generators don't support returning by references
          !m_curFunc->isGenerator) {
        emitConvertToVar(e);
        retSym = StackSym::V;
      } else {
        emitConvertToCell(e);
      }
    } else {
      e.Null();
    }
    assert(m_evalStack.size() == 1);
    assert(IMPLIES(m_curFunc->isAsync || m_curFunc->isGenerator,
                   retSym == StackSym::C));
    emitReturn(e, retSym, r);
    return false;
  }

  case Construct::KindOfStaticStatement: {
    auto vars = static_pointer_cast<StaticStatement>(node)->getVars();
    for (int i = 0, n = vars->getCount(); i < n; i++) {
      ExpressionPtr se((*vars)[i]);
      assert(se->is(Construct::KindOfAssignmentExpression));
      auto ae = static_pointer_cast<AssignmentExpression>(se);
      ExpressionPtr var(ae->getVariable());
      ExpressionPtr value(ae->getValue());
      assert(var->is(Construct::KindOfSimpleVariable));
      auto sv = static_pointer_cast<SimpleVariable>(var);
      StringData* name = makeStaticString(sv->getName());
      Id local = m_curFunc->lookupVarId(name);

      if (m_staticEmitted.insert(sv->getName()).second) {
        Func::SVInfo svInfo;
        svInfo.name = name;
        std::ostringstream os;
        CodeGenerator cg(&os, CodeGenerator::PickledPHP);
        auto ar = std::make_shared<AnalysisResult>();
        value->outputPHP(cg, ar);
        svInfo.phpCode = makeStaticString(os.str());
        m_curFunc->staticVars.push_back(svInfo);
      }

      if (value->isScalar()) {
        emitVirtualLocal(local);
        visit(value);
        emitConvertToCell(e);
        e.StaticLocInit(local, name);
      } else {
        Label done;
        emitVirtualLocal(local);
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

  case Construct::KindOfSwitchStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto sw = static_pointer_cast<SwitchStatement>(node);

    auto cases = sw->getCases();
    if (!cases) {
      visit(sw->getExp());
      emitPop(e);
      return false;
    }
    uint32_t ncase = cases->getCount();
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
    auto subject = sw->getExp();
    bool simpleSubject = subject->is(Construct::KindOfSimpleVariable)
      && !static_pointer_cast<SimpleVariable>(subject)->getAlwaysStash();
    Id tempLocal = -1;
    Offset start = InvalidAbsoluteOffset;

    bool enabled = RuntimeOption::EvalEmitSwitch;
    auto call = dynamic_pointer_cast<SimpleFunctionCall>(subject);

    SwitchState state;
    bool didSwitch = false;
    if (enabled) {
      MaybeDataType stype = analyzeSwitch(sw, state);
      if (stype) {
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
          assert(isStringType(*stype));
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
      for (uint32_t i = 0; i < ncase; i++) {
        auto c = static_pointer_cast<CaseStatement>((*cases)[i]);
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
        } else if (LIKELY(defI == -1)) {
          // Default clause.
          defI = i;
        } else {
          throw IncludeTimeFatalException(
            c, "Switch statements may only contain one default: clause");
        }
      }
      if (defI != -1) {
        e.Jmp(caseLabels[defI]);
      } else {
        e.Jmp(brkTarget);
      }
    }
    for (uint32_t i = 0; i < ncase; i++) {
      caseLabels[i].set(e);
      auto c = static_pointer_cast<CaseStatement>((*cases)[i]);
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

  case Construct::KindOfThrowStatement: {
    visitKids(node);
    emitConvertToCell(e);
    e.Throw();
    return false;
  }

  case Construct::KindOfFinallyStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::Finally);
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };

    auto fs = static_pointer_cast<FinallyStatement>(node);
    visit(fs->getBody());
    return false;
  }

  case Construct::KindOfTryStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::TryFinally);
    if (!m_evalStack.empty()) {
      InvariantViolation(
        "Emitter detected that the evaluation stack is not empty "
        "at the beginning of a try region: %d", m_ue.bcPos());
    }

    auto ts = static_pointer_cast<TryStatement>(node);
    auto f = static_pointer_cast<FinallyStatement>(ts->getFinally());

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
          auto c = static_pointer_cast<CatchStatement>((*catches)[i]);
          StringData* eName = makeStaticString(c->getOriginalClassName());

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

  case Construct::KindOfUnsetStatement: {
    auto exps = static_pointer_cast<UnsetStatement>(node)->getExps();
    for (int i = 0, n = exps->getCount(); i < n; i++) {
      emitVisitAndUnset(e, (*exps)[i]);
    }
    return false;
  }

  case Construct::KindOfWhileStatement: {
    auto s = static_pointer_cast<Statement>(node);
    auto region = createRegion(s, Region::Kind::LoopOrSwitch);
    auto ws = static_pointer_cast<WhileStatement>(s);
    ExpressionPtr condExp(ws->getCondExp());
    Label& lcontinue = registerContinue(ws, region.get(), 1,
      false)->m_label;
    Label& fail = registerBreak(ws, region.get(), 1, false)->m_label;
    Label top;
    auto emit_cond = [&] (Label& tru, bool truFallthrough) {
      Emitter condEmitter(condExp, m_ue, *this);
      visitIfCondition(condExp, condEmitter, tru, fail, truFallthrough);
    };
    emit_cond(top, true);
    top.set(e);
    {
      enterRegion(region);
      SCOPE_EXIT { leaveRegion(region); };
      visit(ws->getBody());
    }
    if (lcontinue.isUsed()) lcontinue.set(e);
    emit_cond(top, false);
    if (fail.isUsed()) fail.set(e);
    return false;
  }

  case Construct::KindOfInterfaceStatement:
  case Construct::KindOfClassStatement: {
    emitClass(e, node->getClassScope(), false);
    return false;
  }

  case Construct::KindOfClassVariable:
  case Construct::KindOfClassConstant:
  case Construct::KindOfMethodStatement:
    // handled by emitClass
    not_reached();

  case Construct::KindOfFunctionStatement: {
    auto m = static_pointer_cast<MethodStatement>(node);
    // Only called for fn defs not on the top level
    assert(!node->getClassScope()); // Handled directly by emitClass().
    StringData* nName = makeStaticString(m->getOriginalName());
    FuncEmitter* fe = m_ue.newFuncEmitter(nName);
    e.DefFunc(fe->id());
    postponeMeth(m, fe, false);
    return false;
  }

  case Construct::KindOfGotoStatement: {
    auto g = static_pointer_cast<GotoStatement>(node);
    StringData* nName = makeStaticString(g->label());
    emitGoto(e, nName, g);
    return false;
  }

  case Construct::KindOfLabelStatement: {
    auto l = static_pointer_cast<LabelStatement>(node);
    StringData* nName = makeStaticString(l->label());
    registerGoto(l, m_regions.back().get(), nName, false)
      ->m_label.set(e);
    return false;
  }
  case Construct::KindOfStatement:
  case Construct::KindOfUseTraitStatement:
  case Construct::KindOfClassRequireStatement:
  case Construct::KindOfTraitPrecStatement:
  case Construct::KindOfTraitAliasStatement: {
    not_implemented();
  }
  case Construct::KindOfUnaryOpExpression: {
    auto u = static_pointer_cast<UnaryOpExpression>(node);
    int op = u->getOp();

    if (op == T_UNSET) {
      // php doesnt have an unset expression, but hphp's optimizations
      // sometimes introduce them
      auto exp = u->getExpression();
      if (exp->is(Construct::KindOfExpressionList)) {
        auto exps = static_pointer_cast<ExpressionList>(exp);
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
      auto el = static_pointer_cast<ExpressionList>(u->getExpression());
      emitArrayInit(e, el);
      return true;
    } else if (op == T_ISSET) {
      auto list = dynamic_pointer_cast<ExpressionList>(u->getExpression());
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
      oldErrorLevelLoc = m_curFunc->allocUnnamedLocal();
      emitVirtualLocal(oldErrorLevelLoc);
      auto idx = m_evalStack.size() - 1;
      e.Silence(m_evalStack.getLoc(idx), SilenceOp::Start);
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
        // $this++ is a no-op
        if (auto var = dynamic_pointer_cast<SimpleVariable>(exp)) {
          if (var->isThis()) break;
        }

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

  case Construct::KindOfAssignmentExpression: {
    auto ae = static_pointer_cast<AssignmentExpression>(node);
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

  case Construct::KindOfBinaryOpExpression: {
    auto b = static_pointer_cast<BinaryOpExpression>(node);
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
        auto scalar = dynamic_pointer_cast<ScalarExpression>(second);
        bool notQuoted = scalar && !scalar->isQuoted();
        std::string s = second->getLiteralString();

        const auto isame =
          [](const std::string& a, const std::string& b) {
            return (a.size() == b.size()) &&
                   !strncasecmp(a.c_str(), b.c_str(), a.size());
          };

        if (notQuoted && isame(s, "static")) {
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
          ClassScopeRawPtr cls = second->getClassScope();
          bool isTrait = cls && cls->isTrait();
          bool isSelf = notQuoted && isame(s, "self");
          bool isParent = notQuoted && isame(s, "parent");

          if (isTrait && (isSelf || isParent)) {
            emitConvertToCell(e);
            if (isSelf) {
              e.Self();
            } else if (isParent) {
              e.Parent();
            }

            e.NameA();
            e.InstanceOf();
          } else {
            if (cls) {
              if (isSelf) {
                s = cls->getScopeName();
              } else if (isParent) {
                s = cls->getOriginalParent();
              }
            }

            StringData* nLiteral = makeStaticString(s);
            e.InstanceOfD(nLiteral);
          }
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
      emitCollectionInit(e, b);
      return true;
    }

    if (op == T_PIPE) {
      Id pipeVar = emitVisitAndSetUnnamedL(e, b->getExp1());
      allocPipeLocal(pipeVar);
      visit(b->getExp2());
      releasePipeLocal(pipeVar);
      emitPushAndFreeUnnamedL(e, pipeVar, m_ue.bcPos());
      e.PopC();
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
      case T_SPACESHIP: e.Cmp(); break;
      case T_POW: e.Pow(); break;
      default: assert(false);
    }
    return true;
  }

  case Construct::KindOfClassConstantExpression: {
    auto cc = static_pointer_cast<ClassConstantExpression>(node);
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
      throw IncludeTimeFatalException(cc, s.str().c_str());
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
    } else if (cc->getOriginalClassScope() &&
               !cc->getOriginalClassScope()->isTrait()) {
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

  case Construct::KindOfConstantExpression: {
    auto c = static_pointer_cast<ConstantExpression>(node);
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

  case Construct::KindOfEncapsListExpression: {
    auto el = static_pointer_cast<EncapsListExpression>(node);
    auto args = el->getExpressions();
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
      e.String(staticEmptyString());
    }

    if (el->getType() == '`') {
      emitConvertToCell(e);
      e.FPassC(0);
      delete fpi;
      e.FCall(1);
    }
    return true;
  }

  case Construct::KindOfArrayElementExpression: {
    auto ae = static_pointer_cast<ArrayElementExpression>(node);
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
      m_tempLoc = ae->getRange();
    }
    return true;
  }

  case Construct::KindOfSimpleFunctionCall: {
    auto call = static_pointer_cast<SimpleFunctionCall>(node);
    auto params = call->getParams();

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
    } else if (call->isCallToFunction("hh\\invariant")) {
      if (emitHHInvariant(e, call)) return true;
    } else if (call->isCallToFunction("hh\\idx") &&
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
    } else if (call->isCallToFunction("max")) {
      if (params && params->getCount() == 2) {
        emitFuncCall(e, call, "__SystemLib\\max2", params);
        return true;
      }
    } else if (call->isCallToFunction("min")) {
      if (params && params->getCount() == 2) {
        emitFuncCall(e, call, "__SystemLib\\min2", params);
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
    } else if (call->isCallToFunction("assert")) {
      // Special-case some logic around emitting assert(), or jumping around
      // it. This all applies only for direct calls to assert() -- dynamic
      // calls don't get this special logic, and don't in PHP7 either.

      if (!RuntimeOption::AssertEmitted) {
        e.True();
        return true;
      }

      // We need to emit an ini_get around all asserts to check if the
      // zend.assertions option is enabled -- you can switch between 0 and 1
      // at runtime, and having it set to 0 disables the assert from running,
      // including side effects of function arguments, so we need to jump
      // around it if so. (The -1 value of zend.assertions corresponds to
      // AssertEmitted being set to 0 above, and is not changeable at
      // runtime.)
      Label disabled, after;
      e.String(s_zend_assertions.get());
      e.FCallBuiltin(1, 1, s_ini_get.get());
      e.UnboxRNop();
      e.Int(0);
      e.Gt();
      e.JmpZ(disabled);

      emitFuncCall(e, call, "__SystemLib\\assert", call->getParams());
      emitConvertToCell(e);
      e.Jmp(after);

      disabled.set(e);
      e.True();

      after.set(e);
      return true;
    } else if (emitSystemLibVarEnvFunc(e, call)) {
      return true;
    } else if (call->isCallToFunction("array_slice") &&
               params && params->getCount() == 2 &&
               !Option::JitEnableRenameFunction) {
      ExpressionPtr p0 = (*params)[0];
      ExpressionPtr p1 = (*params)[1];
      Variant v1;
      if (p0->getKindOf() == Construct::KindOfSimpleFunctionCall &&
          p1->getScalarValue(v1) && v1.isInteger()) {
        auto innerCall = static_pointer_cast<SimpleFunctionCall>(p0);
        auto innerParams = innerCall->getParams();
        if (innerCall->isCallToFunction("func_get_args") &&
            (!innerParams || innerParams->getCount() == 0)) {
          params->removeElement(0);
          emitFuncCall(e, innerCall,
                       "__SystemLib\\func_slice_args", params);
          return true;
        }
      }
      // fall through
    } else if ((call->isCallToFunction("class_exists") ||
                call->isCallToFunction("interface_exists") ||
                call->isCallToFunction("trait_exists"))
               && params
               && (params->getCount() == 1 || params->getCount() == 2)) {
      // Push name
      emitNameString(e, (*params)[0]);
      emitConvertToCell(e);
      e.CastString();

      // Push autoload, defaulting to true
      if (params->getCount() == 1) {
        e.True();
      } else {
        visit((*params)[1]);
        emitConvertToCell(e);
        e.CastBool();
      }
      if (call->isCallToFunction("class_exists")) {
        e.OODeclExists(OODeclExistsOp::Class);
      } else if (call->isCallToFunction("interface_exists")) {
        e.OODeclExists(OODeclExistsOp::Interface);
      } else {
        assert(call->isCallToFunction("trait_exists"));
        e.OODeclExists(OODeclExistsOp::Trait);
      }
      return true;
    } else if (call->isCallToFunction("get_class") &&
               !params &&
               call->getClassScope() &&
               !call->getClassScope()->isTrait()) {
      StringData* name =
        makeStaticString(call->getClassScope()->getScopeName());
      e.String(name);
      return true;
    }
#define TYPE_CONVERT_INSTR(what, What)                             \
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

#define TYPE_CHECK_INSTR(what, What)                \
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
  case Construct::KindOfDynamicFunctionCall: {
    emitFuncCall(e, static_pointer_cast<FunctionCall>(node));
    return true;
  }

  case Construct::KindOfIncludeExpression: {
    auto ie = static_pointer_cast<IncludeExpression>(node);
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

  case Construct::KindOfListAssignment: {
    auto la = static_pointer_cast<ListAssignment>(node);
    auto rhs = la->getArray();

    // listAssignmentVisitLHS should have handled this
    assert(rhs);

    bool nullRHS = la->getRHSKind() == ListAssignment::Null;
    // If the RHS is not a simple variable, we need to evaluate it and assign
    // it to a temp local. If it is, whether or not we directly use it or copy
    // it into a temp local is visible in perverse statements like:
    // list($a, $b) = $a
    // The behavior of that changed between PHP5 and PHP7; in PHP5 we directly
    // use the temp local, in PHP7 we need to copy it.
    bool simpleRHS = rhs->is(Construct::KindOfSimpleVariable)
      && !static_pointer_cast<SimpleVariable>(rhs)->getAlwaysStash()
      && !RuntimeOption::PHP7_LTR_assign;
    Id tempLocal = -1;
    Offset start = InvalidAbsoluteOffset;

    if (!simpleRHS && la->isRhsFirst()) {
      tempLocal = emitVisitAndSetUnnamedL(e, rhs);
      start = m_ue.bcPos();
    }

    // We use "index chains" to deal with nested list assignment.  We will
    // end up with one index chain per expression we need to assign to.
    // The helper function will populate indexChains.
    //
    // In PHP5 mode, this will also evaluate the LHS; in PHP7 mode, that is
    // always delayed until listAssignmentAssignElements below. This means
    // that isRhsFirst() has no effect in PHP7 mode. See comments in
    // listAssignmentVisitLHS and listAssignmentAssignElements for more
    // explanation.
    std::vector<IndexPair> indexPairs;
    IndexChain workingChain;
    listAssignmentVisitLHS(e, la, workingChain, indexPairs);

    if (!simpleRHS && !la->isRhsFirst()) {
      assert(tempLocal == -1);
      assert(start == InvalidAbsoluteOffset);
      tempLocal = emitVisitAndSetUnnamedL(e, rhs);
      start = m_ue.bcPos();
    }

    // Assign elements.
    if (nullRHS) {
      listAssignmentAssignElements(e, indexPairs, nullptr);
    } else if (simpleRHS) {
      listAssignmentAssignElements(e, indexPairs, [&] { visit(rhs); });
    } else {
      listAssignmentAssignElements(
        e, indexPairs,
        [&] { emitVirtualLocal(tempLocal); }
      );
    }

    // Leave the RHS on the stack
    if (simpleRHS) {
      visit(rhs);
    } else {
      emitPushAndFreeUnnamedL(e, tempLocal, start);
    }

    return true;
  }

  case Construct::KindOfNewObjectExpression: {
    auto ne = static_pointer_cast<NewObjectExpression>(node);
    auto params = ne->getParams();
    int numParams = params ? params->getCount() : 0;
    ClassScopeRawPtr cls = ne->getClassScope();

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

    emitCall(e, ne, params, fpiStart);
    e.PopR();
    return true;
  }

  case Construct::KindOfObjectMethodExpression: {
    auto om = static_pointer_cast<ObjectMethodExpression>(node);
    // $obj->name(...)
    // ^^^^
    visit(om->getObject());
    m_tempLoc = om->getRange();
    emitConvertToCell(e);
    ExpressionListPtr params(om->getParams());
    int numParams = params ? params->getCount() : 0;

    Offset fpiStart = 0;
    ExpressionPtr methName = om->getNameExp();
    bool useDirectForm = false;
    if (methName->is(Construct::KindOfScalarExpression)) {
      auto sval = static_pointer_cast<ScalarExpression>(methName);
      const std::string& methStr = sval->getOriginalLiteralString();
      if (!methStr.empty()) {
        // $obj->name(...)
        //       ^^^^
        // Use getOriginalLiteralString(), which hasn't been
        // case-normalized, since __call() needs to preserve
        // the case.
        StringData* nameLiteral = makeStaticString(methStr);
        fpiStart = m_ue.bcPos();
        e.FPushObjMethodD(
          numParams,
          nameLiteral,
          om->isNullSafe() ? ObjMethodOp::NullSafe : ObjMethodOp::NullThrows
        );
        useDirectForm = true;
      }
    }
    if (!useDirectForm) {
      // $obj->{...}(...)
      //       ^^^^^
      visit(methName);
      emitConvertToCell(e);
      fpiStart = m_ue.bcPos();
      e.FPushObjMethod(
        numParams,
        om->isNullSafe() ? ObjMethodOp::NullSafe : ObjMethodOp::NullThrows
      );
    }
    // $obj->name(...)
    //           ^^^^^
    emitCall(e, om, params, fpiStart);
    return true;
  }

  case Construct::KindOfObjectPropertyExpression: {
    auto op = static_pointer_cast<ObjectPropertyExpression>(node);
    if (op->isNullSafe() &&
        op->hasAnyContext(
            Expression::RefValue
          | Expression::LValue
          | Expression::DeepReference
        ) && !op->hasContext(Expression::InvokeArgument)
    ) {
      throw IncludeTimeFatalException(op,
        Strings::NULLSAFE_PROP_WRITE_ERROR);
    }
    ExpressionPtr obj = op->getObject();
    auto sv = dynamic_pointer_cast<SimpleVariable>(obj);
    if (sv && sv->isThis() && sv->hasContext(Expression::ObjectContext)) {
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
      m_tempLoc = op->getRange();
    }
    markProp(
      e,
      op->isNullSafe()
        ? PropAccessType::NullSafe
        : PropAccessType::Normal
    );
    return true;
  }

  case Construct::KindOfQOpExpression: {
    auto q = static_pointer_cast<QOpExpression>(node);
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

  case Construct::KindOfNullCoalesceExpression: {
    auto q = static_pointer_cast<NullCoalesceExpression>(node);

    Label done;
    visit(q->getFirst());
    emitCGetQuiet(e);
    e.Dup();
    emitIsset(e);
    e.JmpNZ(done);
    e.PopC();
    visit(q->getSecond());
    emitConvertToCell(e);
    done.set(e);
    m_evalStack.cleanTopMeta();

    return true;
  }

  case Construct::KindOfScalarExpression: {
    auto ex = static_pointer_cast<Expression>(node);
    Variant v;
    ex->getScalarValue(v);
    switch (v.getType()) {
      case KindOfInt64:
        e.Int(v.getInt64());
        return true;

      case KindOfDouble:
        e.Double(v.getDouble());
        return true;

      case KindOfPersistentString:
      case KindOfString: {
        StringData* nValue = makeStaticString(v.getStringData());
        e.String(nValue);
        return true;
      }

      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfPersistentArray:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
      case KindOfRef:
      case KindOfClass:
        break;
    }
    not_reached();
  }

  case Construct::KindOfPipeVariable: {
    if (auto pipeVar = getPipeLocal()) {
      emitVirtualLocal(*pipeVar);
      return true;
    }

    throw IncludeTimeFatalException(
      node, "Pipe variables must occur only in the RHS of pipe expressions");
  }

  case Construct::KindOfSimpleVariable: {
    auto sv = static_pointer_cast<SimpleVariable>(node);
    if (sv->isThis()) {
      if (sv->hasContext(Expression::ObjectContext)) {
        e.This();
      } else if (sv->getFunctionScope()->needsLocalThis()) {
        static const StringData* thisStr = makeStaticString("this");
        Id thisId = m_curFunc->lookupVarId(thisStr);
        emitVirtualLocal(thisId);
      } else {
        auto const subop = sv->hasContext(Expression::ExistContext)
          ? BareThisOp::NoNotice
          : BareThisOp::Notice;
        e.BareThis(subop);
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

  case Construct::KindOfDynamicVariable: {
    auto dv = static_pointer_cast<DynamicVariable>(node);
    visit(dv->getSubExpression());
    emitConvertToCellOrLoc(e);
    markName(e);
    return true;
  }

  case Construct::KindOfStaticMemberExpression: {
    auto sm = static_pointer_cast<StaticMemberExpression>(node);
    emitVirtualClassBase(e, sm.get());
    emitNameString(e, sm->getExp());
    markSProp(e);
    return true;
  }

  case Construct::KindOfArrayPairExpression: {
    auto ap = static_pointer_cast<ArrayPairExpression>(node);

    auto key = ap->getName();
    if (!m_staticArrays.empty()) {
      auto val = ap->getValue();

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
        // Set/ImmSet, val is the key
        if (m_staticColType.back() == CollectionType::Set ||
            m_staticColType.back() == CollectionType::ImmSet) {
          m_staticArrays.back().set(tvAsVariant(&tvVal),
                                    tvAsVariant(&tvVal));
        } else {
          m_staticArrays.back().append(tvAsCVarRef(&tvVal));
        }
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
  case Construct::KindOfExpressionList: {
    auto el = static_pointer_cast<ExpressionList>(node);
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
  case Construct::KindOfParameterExpression: {
    not_implemented();
  }
  case Construct::KindOfModifierExpression: {
    not_implemented();
  }
  case Construct::KindOfUserAttribute: {
    not_implemented();
  }
  case Construct::KindOfClosureExpression: {
    // Closures are implemented by anonymous classes that extend Closure.
    // There is one anonymous class per closure body.
    auto ce = static_pointer_cast<ClosureExpression>(node);

    // Build a convenient list of use-variables. Each one corresponds to:
    // (a) an instance variable, to store the value until call time
    // (b) a parameter of the generated constructor
    // (c) an argument to the constructor at the definition site
    // (d) a line of code in the generated constructor;
    // (e) a line of code in the generated prologue to the closure body
    auto useList = ce->getClosureVariables();
    ClosureUseVarVec useVars;
    int useCount = (useList ? useList->getCount() : 0);
    if (useList) {
      for (int i = 0; i < useCount; ++i) {
        auto var = static_pointer_cast<ParameterExpression>((*useList)[i]);
        StringData* varName = makeStaticString(var->getName());
        useVars.push_back(ClosureUseVar(varName, var->isRef()));
      }
    }

    // We're still at the closure definition site. Emit code to instantiate
    // the new anonymous class, with the use variables as arguments.
    ExpressionListPtr valuesList(ce->getClosureValues());
    for (int i = 0; i < useCount; ++i) {
      ce->type() == ClosureType::Short
        ? emitLambdaCaptureArg(e, (*valuesList)[i])
        : (void)emitBuiltinCallArg(e, (*valuesList)[i], i,
                                   useVars[i].second, false);
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

    auto ssClsName = makeStaticString(clsName);
    e.CreateCl(useCount, ssClsName);

    // From here on out, we're creating a new class to hold the closure.
    const static StringData* parentName = makeStaticString("Closure");
    PreClassEmitter* pce = m_ue.newPreClassEmitter(
      ssClsName, PreClass::ClosureHoistable);

    auto const attrs = AttrNoOverride | AttrUnique | AttrPersistent;

    pce->init(ce->line0(), ce->line1(), m_ue.bcPos(),
              attrs, parentName, nullptr);

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
    invoke->isClosureBody = true;
    pce->addMethod(invoke);
    auto body = static_pointer_cast<MethodStatement>(ce->getClosureFunction());
    postponeMeth(body, invoke, false, new ClosureUseVarVec(useVars));

    return true;
  }
  case Construct::KindOfClassExpression: {
    auto ce = static_pointer_cast<ClassExpression>(node);
    // The parser generated a unique name for the class, use that
    std::string clsName = ce->getClass()->getOriginalName();
    auto ssClsName = makeStaticString(clsName);
    auto fpiStart = m_ue.bcPos();
    auto params = ce->getParams();
    int numParams = params ? params->getCount() : 0;
    e.FPushCtorD(numParams, ssClsName);
    emitCall(e, ce, ce->getParams(), fpiStart);
    e.PopR();
    return true;
  }
  case Construct::KindOfYieldExpression: {
    auto y = static_pointer_cast<YieldExpression>(node);

    registerYieldAwait(y);
    assert(m_evalStack.size() == 0);

    // evaluate key passed to yield, if applicable
    ExpressionPtr keyExp = y->getKeyExpression();
    if (keyExp) {
      m_curFunc->isPairGenerator = true;
      visit(keyExp);
      emitConvertToCell(e);
    }

    // evaluate value expression passed to yield
    visit(y->getValueExpression());
    emitConvertToCell(e);

    // suspend generator
    if (keyExp) {
      assert(m_evalStack.size() == 2);
      e.YieldK();
    } else {
      assert(m_evalStack.size() == 1);
      e.Yield();
    }

    // continue with the received result on the stack
    assert(m_evalStack.size() == 1);
    return true;
  }
  case Construct::KindOfYieldFromExpression: {
    auto yf = static_pointer_cast<YieldFromExpression>(node);

    registerYieldAwait(yf);
    assert(m_evalStack.size() == 0);

    emitYieldFrom(e, yf->getExpression());

    return true;
  }
  case Construct::KindOfAwaitExpression: {
    auto await = static_pointer_cast<AwaitExpression>(node);

    registerYieldAwait(await);
    assert(m_evalStack.size() == 0);

    auto expression = await->getExpression();
    if (emitInlineGenva(e, expression)) return true;

    Label resume;

    // evaluate expression passed to await
    visit(expression);
    emitConvertToCell(e);

    // if expr is null, just continue
    e.Dup();
    emitIsType(e, IsTypeOp::Null);
    e.JmpNZ(resume);

    assert(m_evalStack.size() == 1);

    e.Await();

    resume.set(e);
    return true;
  }
  case Construct::KindOfUseDeclarationStatementFragment:
  case Construct::KindOfExpression: {
    not_reached();
  }
  }

  not_reached();
}

void EmitterVisitor::emitConstMethodCallNoParams(Emitter& e,
                                                 const std::string& name) {
  auto const nameLit = makeStaticString(name);
  auto const fpiStart = m_ue.bcPos();
  e.FPushObjMethodD(0, nameLit, ObjMethodOp::NullThrows);
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
  }
  e.FCall(0);
  emitConvertToCell(e);
}

namespace {
const StaticString
  s_hh_invariant_violation("hh\\invariant_violation"),
  s_invariant_violation("invariant_violation"),
  s_gennull("HH\\Asio\\null"),
  s_fromArray("fromArray"),
  s_AwaitAllWaitHandle("HH\\AwaitAllWaitHandle")
  ;
}

bool EmitterVisitor::emitInlineGenva(
  Emitter& e,
  const ExpressionPtr expression
) {
  if (!m_ue.m_isHHFile || !Option::EnableHipHopSyntax ||
      !expression->is(Expression::KindOfSimpleFunctionCall) ||
      Option::JitEnableRenameFunction) {
    return false;
  }
  const auto call = static_pointer_cast<SimpleFunctionCall>(expression);
  assert(call);
  if (!call->isCallToFunction("genva")) return false;
  const auto params = call->getParams();
  if (!params) {
    e.Array(staticEmptyArray());
    return true;
  }
  if (params->containsUnpack()) {
    throw IncludeTimeFatalException(params, "do not use ...$args with genva()");
  }
  const auto num_params = params->getCount();
  assertx(num_params > 0);

  for (auto i = int{0}; i < num_params; i++) {
    Label gwh;

    visit((*params)[i]);
    emitConvertToCell(e);

    // ($_ !== null ? HH\Asio\null() : $_)->getWaitHandle()
    e.Dup();
    emitIsType(e, IsTypeOp::Null);
    e.JmpZ(gwh);
    emitPop(e);

    Offset fpiStart = m_ue.bcPos();
    e.FPushFuncD(0, s_gennull.get());
    {
      FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    }
    e.FCall(0);
    e.UnboxR();
    gwh.set(e);
    emitConstMethodCallNoParams(e, "getWaitHandle");
  }

  std::vector<Id> waithandles(num_params);
  for (auto i = int{num_params - 1}; i >= 0; --i) {
    waithandles[i] = emitSetUnnamedL(e);
  }
  assertx(waithandles.size() == num_params);

  // AwaitAllWaitHandle::fromArray() always returns a WaitHandle.
  Offset fpiStart = m_ue.bcPos();
  e.FPushClsMethodD(1, s_fromArray.get(), s_AwaitAllWaitHandle.get());
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    // create a packed array of the waithandles
    for (const auto wh : waithandles) {
      emitVirtualLocal(wh);
      emitCGet(e);
    }
    e.NewPackedArray(num_params);
    emitFPass(e, 0, PassByRefKind::ErrorOnCell);
  }
  e.FCall(1);
  e.UnboxR();

  e.Await();
  // result of AwaitAllWaitHandle does not matter
  emitPop(e);

  for (const auto wh : waithandles) {
    emitVirtualLocal(wh);
    emitPushL(e);
    e.WHResult();
  }
  e.NewPackedArray(num_params);

  for (auto wh : waithandles) {
    m_curFunc->freeUnnamedLocal(wh);
  }

  newFaultRegionAndFunclet(
    fpiStart, m_ue.bcPos(),
    new UnsetUnnamedLocalsThunklet(std::move(waithandles)));

  return true;
}

bool EmitterVisitor::emitHHInvariant(Emitter& e, SimpleFunctionCallPtr call) {
  if (!m_ue.m_isHHFile && !RuntimeOption::EnableHipHopSyntax) return false;

  auto const params = call->getParams();
  if (!params || params->getCount() < 1) return false;

  Label ok;

  visit((*params)[0]);
  emitCGet(e);
  e.JmpNZ(ok);

  auto const fpiStart = m_ue.bcPos();
  e.FPushFuncD(params->getCount() - 1, s_hh_invariant_violation.get());
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    for (auto i = uint32_t{1}; i < params->getCount(); ++i) {
      emitFuncCallArg(e, (*params)[i], i - 1, false);
    }
  }
  e.FCall(params->getCount() - 1);
  emitPop(e);
  // The invariant_violation can't return; but bytecode invariants mandate an
  // opcode that can't fall through:
  e.String(s_invariant_violation.get());
  e.Fatal(FatalOp::Runtime);

  ok.set(e);
  e.Null(); // invariant returns null if used in an expression, void according
            // to the typechecker.
  return true;
}

int EmitterVisitor::scanStackForLocation(int iLast) {
  assertx(iLast >= 0);
  assertx(iLast < (int)m_evalStack.size());
  for (int i = iLast; i >= 0; --i) {
    char marker = StackSym::GetMarker(m_evalStack.get(i));
    if (marker != StackSym::E && marker != StackSym::W &&
        marker != StackSym::P && marker != StackSym::M &&
        marker != StackSym::Q) {
      return i;
    }
  }
  InvariantViolation("Emitter expected a location on the stack but none "
                     "was found (at offset %d)",
                     m_ue.bcPos());
  return 0;
}

size_t EmitterVisitor::emitMOp(
  int iFirst,
  int& iLast,
  Emitter& e,
  MInstrOpts opts
) {
  auto stackIdx = [&](int i) {
    return m_evalStack.actualSize() - 1 - m_evalStack.getActualPos(i);
  };

  auto const baseFlags =
    opts.fpass ? MOpFlags::None
               : MOpFlags(uint8_t(opts.flags) & uint8_t(MOpFlags::WarnDefine));

  // Emit the base location operation.
  auto sym = m_evalStack.get(iFirst);
  auto flavor = StackSym::GetSymFlavor(sym);
  switch (StackSym::GetMarker(sym)) {
    case StackSym::N:
      switch (flavor) {
        case StackSym::C:
          if (opts.fpass) {
            e.FPassBaseNC(opts.paramId, stackIdx(iFirst));
          } else {
            e.BaseNC(stackIdx(iFirst), baseFlags);
          }
          break;
        case StackSym::L:
          if (opts.fpass) {
            e.FPassBaseNL(opts.paramId, m_evalStack.getLoc(iFirst));
          } else {
            e.BaseNL(m_evalStack.getLoc(iFirst), baseFlags);
          }
          break;
        default:
          always_assert(false);
      }
      break;

    case StackSym::G:
      switch (flavor) {
        case StackSym::C:
          if (opts.fpass) {
            e.FPassBaseGC(opts.paramId, stackIdx(iFirst));
          } else {
            e.BaseGC(stackIdx(iFirst), baseFlags);
          }
          break;
        case StackSym::L:
          if (opts.fpass) {
            e.FPassBaseGL(opts.paramId, m_evalStack.getLoc(iFirst));
          } else {
            e.BaseGL(m_evalStack.getLoc(iFirst), baseFlags);
          }
          break;
        default:
          always_assert(false);
      }
      break;

    case StackSym::S: {
      if (m_evalStack.get(iLast) != StackSym::AM) {
        unexpectedStackSym(sym, "S-vector base, class ref");
      }

      auto const clsIdx = opts.rhsVal ? 1 : 0;
      switch (flavor) {
        case StackSym::C:
          e.BaseSC(stackIdx(iFirst), clsIdx);
          break;
        case StackSym::L:
          e.BaseSL(m_evalStack.getLoc(iFirst), clsIdx);
          break;
        default:
          unexpectedStackSym(sym, "S-vector base, prop name");
          break;
      }
      // The BaseS* bytecodes consume the Class from the eval stack so the
      // final operations don't have to expect an A-flavored input. Adjust
      // iLast accordingly.
      --iLast;
      break;
    }

    case StackSym::None:
      switch (flavor) {
        case StackSym::L:
          if (opts.fpass) {
            e.FPassBaseL(opts.paramId, m_evalStack.getLoc(iFirst));
          } else {
            e.BaseL(m_evalStack.getLoc(iFirst), baseFlags);
          }
          break;
        case StackSym::C:
          e.BaseC(stackIdx(iFirst));
          break;
        case StackSym::R:
          e.BaseR(stackIdx(iFirst));
          break;
        case StackSym::H:
          e.BaseH();
          break;
        default:
          always_assert(false);
      }
      break;

    default:
      always_assert(false && "Bad base marker");
  }

  assert(StackSym::GetMarker(m_evalStack.get(iLast)) != StackSym::M);

  // Emit all intermediate operations, leaving the final operation up to our
  // caller.
  for (auto i = iFirst + 1; i < iLast; ++i) {
    if (opts.fpass) {
      e.FPassDim(opts.paramId, symToMemberKey(e, i, opts.allowW));
    } else {
      e.Dim(opts.flags, symToMemberKey(e, i, opts.allowW));
    }
  }

  size_t stackCount = 0;
  for (int i = iFirst; i <= iLast; ++i) {
    if (!StackSym::IsSymbolic(m_evalStack.get(i))) ++stackCount;
  }
  return stackCount;
}

MemberKey EmitterVisitor::symToMemberKey(Emitter& e, int i, bool allowW) {
  auto const sym = m_evalStack.get(i);
  auto const marker = StackSym::GetMarker(sym);
  if (marker == StackSym::W) {
    if (allowW) return MemberKey{};

    throw EmitterVisitor::IncludeTimeFatalException(
      e.getNode(), "Cannot use [] for reading"
    );
  }

  switch (StackSym::GetSymFlavor(sym)) {
    case StackSym::L: {
      auto const local = m_evalStack.getLoc(i);
      switch (marker) {
        case StackSym::E: return MemberKey{MEL, local};
        case StackSym::P: return MemberKey{MPL, local};
        default:          always_assert(false);
      }
    }
    case StackSym::C: {
      auto const idx =
        int32_t(m_evalStack.actualSize() - 1 - m_evalStack.getActualPos(i));
      switch (marker) {
        case StackSym::E: return MemberKey{MEC, idx};
        case StackSym::P: return MemberKey{MPC, idx};
        default:          always_assert(false);
      }
    }
    case StackSym::I: {
      auto const int64 = m_evalStack.getInt(i);
      switch (marker) {
        case StackSym::E: return MemberKey{MEI, int64};
        default:          always_assert(false);
      }
    }
    case StackSym::T: {
      auto const str = m_evalStack.getName(i);
      switch (marker) {
        case StackSym::E: return MemberKey{MET, str};
        case StackSym::P: return MemberKey{MPT, str};
        case StackSym::Q: return MemberKey{MQT, str};
        default:          always_assert(false);
      }
    }
    default:
      always_assert(false);
  }
}

void EmitterVisitor::emitPop(Emitter& e) {
  if (checkIfStackEmpty("Pop*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

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
    emitQueryMOp(i, iLast, e, QueryMOp::CGet);
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

void EmitterVisitor::emitQueryMOp(int iFirst, int iLast, Emitter& e,
                                  QueryMOp op) {
  auto const flags = getQueryMOpFlags(op);
  auto const stackCount = emitMOp(iFirst, iLast, e, MInstrOpts{flags});
  e.QueryM(stackCount, op, symToMemberKey(e, iLast, false /* allowW */));
}

void EmitterVisitor::emitCGet(Emitter& e) {
  if (checkIfStackEmpty("CGet*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

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
    emitQueryMOp(i, iLast, e, QueryMOp::CGet);
  }
}

void EmitterVisitor::emitCGetQuiet(Emitter& e) {
  if (checkIfStackEmpty("CGetQuiet*")) return;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.CGetQuietL(m_evalStack.getLoc(i));  break;
      case StackSym::C:  /* nop */   break;
      case StackSym::LN: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CN: e.CGetQuietN();  break;
      case StackSym::LG: e.CGetL(m_evalStack.getLoc(i));  // fall through
      case StackSym::CG: e.CGetQuietG();  break;
      case StackSym::LS: e.CGetL2(m_evalStack.getLoc(i));  // fall through
      case StackSym::CS: e.CGetS();  break;
      case StackSym::V:  e.Unbox();  break;
      case StackSym::R:  e.UnboxR(); break;
      default: {
        unexpectedStackSym(sym, "emitCGetQuiet");
        break;
      }
    }

  } else {
    emitQueryMOp(i, iLast, e, QueryMOp::CGetQuiet);
  }
}

bool EmitterVisitor::emitVGet(Emitter& e, bool skipCells) {
  if (checkIfStackEmpty("VGet*")) return false;
  LocationGuard loc(e, m_tempLoc);
  m_tempLoc.clear();

  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  assert(sz >= 0);
  char sym = m_evalStack.get(i);
  if (sz == 0 || (sz == 1 && StackSym::GetMarker(sym) == StackSym::S)) {
    switch (sym) {
      case StackSym::L:  e.VGetL(m_evalStack.getLoc(i)); break;
      case StackSym::C:  if (skipCells) return true; e.Box(); break;
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
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpFlags::DefineReffy});
    e.VGetM(stackCount, symToMemberKey(e, iLast, true /* allowW */));
  }
  return false;
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
    exp = static_pointer_cast<ListAssignment>(exp)->getArray();
    permissiveKind = PassByRefKind::WarnOnCell;
  }

  switch (exp->getKindOf()) {
    case Expression::KindOfSimpleFunctionCall: {
      auto sfc = static_pointer_cast<SimpleFunctionCall>(exp);
      // this only happens for calls that have been morphed into bytecode
      // e.g. idx(), abs(), strlen(), etc..
      // It is to allow the following code to work
      // function f(&$arg) {...}
      // f(idx($array, 'key')); <- this fails otherwise
      if (sfc->hasBeenChangedToBytecode()) {
        return PassByRefKind::AllowCell;
      }
    } break;
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
      auto b = static_pointer_cast<BinaryOpExpression>(exp);
      // Assignment op (+=, -=, *=, etc)
      if (b->isAssignmentOp()) return PassByRefKind::WarnOnCell;
    } break;
    case Expression::KindOfUnaryOpExpression: {
      auto u = static_pointer_cast<UnaryOpExpression>(exp);
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
      auto el = static_pointer_cast<ExpressionList>(exp);
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

bool EmitterVisitor::emitBuiltinCallArg(Emitter& e,
                                        ExpressionPtr exp,
                                        int paramId,
                                        bool byRef,
                                        bool mustBeRef) {
  visit(exp);
  if (checkIfStackEmpty("Builtin arg*")) return true;
  if (byRef) {
    auto wasCell = emitVGet(e, true);
    if (wasCell && mustBeRef) {
      auto kind = getPassByRefKind(exp);
      switch (kind) {
        case PassByRefKind::AllowCell:
          // nop
          break;
        case PassByRefKind::WarnOnCell:
          e.String(
            makeStaticString("Only variables should be passed by reference"));
          e.Int(k_E_STRICT);
          e.FCallBuiltin(2, 2, s_trigger_error.get());
          emitPop(e);
          break;
        case PassByRefKind::ErrorOnCell:
          auto save = m_evalStack;
          e.String(
            makeStaticString("Only variables can be passed by reference"));
          e.Fatal(FatalOp::Runtime);
          m_evalStackIsUnknown = false;
          m_evalStack = save;
          return false;
      }
    }
  } else {
    emitCGet(e);
  }
  return true;
}

static bool isNormalLocalVariable(const ExpressionPtr& expr) {
  SimpleVariable* sv = static_cast<SimpleVariable*>(expr.get());
  return (expr->is(Expression::KindOfSimpleVariable) &&
          !sv->isSuperGlobal() &&
          !sv->isThis());
}

void EmitterVisitor::emitLambdaCaptureArg(Emitter& e, ExpressionPtr exp) {
  // Constant folding may lead this to be not a var anymore,
  // so we should not be emitting *GetL in this case.
  if (!isNormalLocalVariable(exp)) {
    visit(exp);
    return;
  }
  auto const sv = static_cast<SimpleVariable*>(exp.get());
  Id locId = m_curFunc->lookupVarId(makeStaticString(sv->getName()));
  emitVirtualLocal(locId);
  e.CUGetL(locId);
}

void EmitterVisitor::emitBuiltinDefaultArg(Emitter& e, Variant& v,
                                           MaybeDataType t, int paramId) {
  switch (v.getType()) {
    case KindOfNull:
      if (t) {
        [&] {
          switch (*t) {
            case KindOfPersistentString:
            case KindOfString:
            case KindOfPersistentArray:
            case KindOfArray:
            case KindOfObject:
            case KindOfResource:
              e.Int(0);
              return;
            case KindOfUninit:
            case KindOfNull:
            case KindOfBoolean:
            case KindOfInt64:
            case KindOfDouble:
            case KindOfRef:
            case KindOfClass:
              break;
          }
          not_reached();
        }();
      } else {
        e.NullUninit();
      }
      return;

    case KindOfBoolean:
      if (v.getBoolean()) {
        e.True();
      } else {
        e.False();
      }
      return;

    case KindOfInt64:
      e.Int(v.getInt64());
      return;

    case KindOfDouble:
      e.Double(v.toDouble());
      return;

    case KindOfPersistentString:
    case KindOfString: {
      StringData *nValue = makeStaticString(v.getStringData());
      e.String(nValue);
      return;
    }

    case KindOfPersistentArray:
    case KindOfArray:
      e.Array(v.getArrayData());
      return;

    case KindOfUninit:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

void EmitterVisitor::emitFuncCallArg(Emitter& e,
                                     ExpressionPtr exp,
                                     int paramId,
                                     bool isUnpack) {
  visit(exp);
  if (checkIfStackEmpty("FPass*")) return;

  // TODO(4599379): if dealing with an unpack, here is where we'd want to
  // emit a bytecode to traverse any containers;

  auto kind = getPassByRefKind(exp);
  if (isUnpack) {
    // This deals with the case where the called function has a
    // by ref param at the index of the unpack (because we don't
    // want to box the unpack itself).
    // But note that unless the user created the array manually,
    // and added reference params at the correct places, we'll
    // still get warnings, and the array elements will not be
    // passed by reference.
    emitConvertToCell(e);
    kind = PassByRefKind::AllowCell;
  }
  emitFPass(e, paramId, kind);
}

void EmitterVisitor::emitFPass(Emitter& e, int paramId,
                               PassByRefKind passByRefKind) {
  if (checkIfStackEmpty("FPass*")) return;
  LocationGuard locGuard(e, m_tempLoc);
  m_tempLoc.clear();

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
    auto const stackCount = emitMOp(i, iLast, e, MInstrOpts{paramId});
    e.FPassM(paramId, stackCount, symToMemberKey(e, iLast, true /* allowW */));
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
    emitQueryMOp(i, iLast, e, QueryMOp::Isset);
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
    emitQueryMOp(i, iLast, e, QueryMOp::Empty);
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
    auto const stackCount = emitMOp(i, iLast, e, MInstrOpts{MOpFlags::Unset});
    e.UnsetM(stackCount, symToMemberKey(e, iLast, false /* allowW */));
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
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpFlags::Define}.rhs());
    return e.SetM(stackCount, symToMemberKey(e, iLast, true /* allowW */));
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
    case T_POW_EQUAL:    return SetOpOp::PowEqual;
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
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpFlags::Define}.rhs());
    e.SetOpM(stackCount, op, symToMemberKey(e, iLast, true /* allowW */));
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
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpFlags::DefineReffy}.rhs());
    e.BindM(stackCount, symToMemberKey(e, iLast, true /* allowW */));
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
    auto const stackCount =
      emitMOp(i, iLast, e, MInstrOpts{MOpFlags::Define});
    e.IncDecM(stackCount, op, symToMemberKey(e, iLast, true /* allowW */));
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
        marker != StackSym::P && marker != StackSym::Q) {
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

MaybeDataType EmitterVisitor::analyzeSwitch(SwitchStatementPtr sw,
                                            SwitchState& state) {
  auto& caseMap = state.cases;
  DataType t = KindOfUninit;
  StatementListPtr cases(sw->getCases());
  const int ncase = cases->getCount();

  // Bail if the cases aren't homogeneous
  for (int i = 0; i < ncase; ++i) {
    auto c = static_pointer_cast<CaseStatement>((*cases)[i]);
    auto condition = c->getCondition();
    if (condition) {
      Variant cval;
      DataType caseType;
      if (condition->getScalarValue(cval)) {
        caseType = cval.getType();
        if (caseType == KindOfPersistentString) caseType = KindOfString;
        if ((caseType != KindOfInt64 && caseType != KindOfString) ||
            !IMPLIES(t != KindOfUninit, caseType == t)) {
          return folly::none;
        }
        t = caseType;
      } else {
        return folly::none;
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
    } else if (LIKELY(state.defI == -1)) {
      state.defI = i;
    } else {
      // Multiple defaults are not allowed
      throw IncludeTimeFatalException(
        c, "Switch statements may only contain one default: clause");
    }
  }

  if (t == KindOfInt64) {
    int64_t base = caseMap.begin()->first;
    int64_t nTargets = caseMap.rbegin()->first - base + 1;
    // Fail if the cases are too sparse. We emit Switch even for absurdly small
    // cases to allow the jit to decide when to lower back to comparisons.
    if ((float)caseMap.size() / nTargets < 0.5) {
      return folly::none;
    }
  } else if (t == KindOfString) {
    if (caseMap.size() < kMinStringSwitchCases) {
      return folly::none;
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
  e.Switch(labels, base, SwitchKind::Bounded);
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

void EmitterVisitor::markProp(Emitter& e, PropAccessType propAccessType) {
  if (m_evalStack.empty()) {
    InvariantViolation(
      "Emitter encountered an empty evaluation stack inside "
      "the markProp function (at offset %d)",
      m_ue.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::L || sym == StackSym::T) {
    m_evalStack.set(
      m_evalStack.size()-1,
      (sym | (propAccessType == PropAccessType::NullSafe
        ? StackSym::Q
        : StackSym::P
      ))
    );
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
      "Emitter encountered an evaluation stack with %lu"    \
      " elements inside the %s function (at offset %d)",    \
      (unsigned long)m_evalStack.size(),                    \
      __FUNCTION__, m_ue.bcPos());                          \
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
  s_IsFoldable("__IsFoldable"),
  s_ParamCoerceModeNull("__ParamCoerceModeNull"),
  s_ParamCoerceModeFalse("__ParamCoerceModeFalse");

static void parseUserAttributes(FuncEmitter* fe, Attr& attrs) {
  if (fe->userAttributes.count(s_IsFoldable.get())) {
    attrs = attrs | AttrIsFoldable;
  }
  if (fe->userAttributes.count(s_ParamCoerceModeNull.get())) {
    attrs = attrs | AttrParamCoerceModeNull;
  } else if (fe->userAttributes.count(s_ParamCoerceModeFalse.get())) {
    attrs = attrs | AttrParamCoerceModeFalse;
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
       (hash_string_i_unsafe(fullName.c_str(), fullName.size()) & 8))) {
    attrs = attrs | AttrHot;
  }

  if (!SystemLib::s_inited) {
    // we're building systemlib. everything is unique
    attrs = attrs | AttrBuiltin | AttrUnique | AttrPersistent;
  } else if (Option::WholeProgram) {
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
  }

  // For closures, the MethodStatement didn't have real attributes; enforce
  // that the __invoke method is public here
  if (fe->isClosureBody) {
    assert(!(attrs & (AttrProtected | AttrPrivate)));
    attrs = attrs | AttrPublic;
  }

  // Coerce memoized methods to private. This is needed for code that uses
  // parent:: to call through to the correct underlying function
  if (meth->is(Statement::KindOfMethodStatement) && fe->isMemoizeImpl) {
    attrs = static_cast<Attr>(attrs & ~(AttrPublic | AttrProtected));
    attrs = attrs | AttrPrivate;
  }

  parseUserAttributes(fe, attrs);
  // Not supported except in __Native functions
  attrs = static_cast<Attr>(
    attrs & ~(AttrParamCoerceModeNull | AttrParamCoerceModeFalse));

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
    if (annot->isTypeAccess()) {
      flags = flags | TypeConstraint::TypeConstant;
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
  std::vector<FuncEmitter*> top_fes;
  while (!m_postponedMeths.empty()) {
    assert(m_actualStackHighWater == 0);
    assert(m_fdescHighWater == 0);
    PostponedMeth& p = m_postponedMeths.front();
    MethodStatementPtr meth = p.m_meth;
    FuncEmitter* fe = p.m_fe;

    ITRACE(1, "Emitting postponed method {}\n", meth->getOriginalFullName());
    Trace::Indent indent;

    if (!fe) {
      assert(p.m_top);
      const StringData* methName = makeStaticString(meth->getOriginalName());
      fe = new FuncEmitter(m_ue, -1, -1, methName);
      auto oldFunc = m_topMethodEmitted.find(meth->getOriginalName());
      if (oldFunc != m_topMethodEmitted.end()) {
        throw IncludeTimeFatalException(
          meth,
          "Cannot redeclare %s() (previously declared in %s:%d)",
          meth->getOriginalName().c_str(),
          oldFunc->second->ue().m_filepath->data(),
          oldFunc->second->getLocation().second);
      }
      m_topMethodEmitted.emplace(meth->getOriginalName(), fe);

      p.m_fe = fe;
      top_fes.push_back(fe);
    }

    auto funcScope = meth->getFunctionScope();
    m_curFunc = fe;
    fe->isAsync = funcScope->isAsync();
    fe->isGenerator = funcScope->isGenerator();

    if (fe->isAsync && !fe->isGenerator && meth->retTypeAnnotation()) {
      auto rta = meth->retTypeAnnotation();
      auto nTypeArgs = rta->numTypeArgs();
      if (!rta->isAwaitable() && !rta->isWaitHandle()) {
        if (fe->isClosureBody) {
          throw IncludeTimeFatalException(
            meth,
            "Return type hint for async closure must be awaitable"
          );
        } else {
          throw IncludeTimeFatalException(
            meth,
            "Return type hint for async %s %s() must be awaitable",
            meth->getClassScope() ? "method" : "function",
            meth->getOriginalFullName().c_str()
          );
        }
      }
      if (nTypeArgs >= 2) {
        throw IncludeTimeFatalException(
          meth,
          "Awaitable interface expects 1 type argument, %d given",
          nTypeArgs);
      }
    }

    if (funcScope->userAttributes().count("__Memoize") &&
        !funcScope->isAbstract()) {
      auto const originalName = fe->name;
      auto const rewrittenName = makeStaticString(
        folly::sformat("{}$memoize_impl", fe->name->data()));

      FuncEmitter* memoizeFe = nullptr;
      if (meth->is(Statement::KindOfFunctionStatement)) {
        if (!p.m_top) {
          throw IncludeTimeFatalException(meth,
            "<<__Memoize>> cannot be applied to closures and inline functions");
        }

        memoizeFe = new FuncEmitter(m_ue, -1, -1, originalName);
        fe->name = rewrittenName;
        top_fes.push_back(memoizeFe);
      } else {
        // Rename the method and create a new method with the original name
        fe->pce()->renameMethod(originalName, rewrittenName);
        memoizeFe = m_ue.newMethodEmitter(originalName, fe->pce());
        bool added UNUSED = fe->pce()->addMethod(memoizeFe);
        assert(added);
      }

      // Emit the new method that handles the memoization
      m_curFunc = memoizeFe;
      m_curFunc->isMemoizeWrapper = true;
      addMemoizeProp(meth);
      emitMethodMetadata(meth, p.m_closureUseVars, p.m_top);
      emitMemoizeMethod(meth, rewrittenName);

      // Switch back to the original method and mark it as a memoize
      // implementation
      m_curFunc = fe;
      m_curFunc->isMemoizeImpl = true;
    }

    if (funcScope->isNative()) {
      bindNativeFunc(meth, fe);
    } else {
      emitMethodMetadata(meth, p.m_closureUseVars, p.m_top);
      emitMethod(meth);
    }

    if (fe->isClosureBody) {
      TypedValue uninit;
      tvWriteUninit(&uninit);
      for (auto& sv : m_curFunc->staticVars) {
        auto const str = makeStaticString(
          folly::format("86static_{}", sv.name->data()).str());
        fe->pce()->addProperty(str, AttrPrivate, nullptr, nullptr,
                               &uninit, RepoAuthType{});
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
    fe->userAttributes[uaName] = tv;
  }
}

const StaticString s_Void("HH\\void");
const char* attr_Deprecated = "__Deprecated";
const StaticString s_attr_Deprecated(attr_Deprecated);

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
  if (!(attributes & (AttrParamCoerceModeFalse | AttrParamCoerceModeNull))) {
    attributes = attributes | AttrParamCoerceModeNull;
  }

  fe->setLocation(meth->line0(), meth->line1());
  fe->docComment = makeStaticString(
    Option::GenerateDocComments ? meth->getDocComment().c_str() : ""
  );
  auto retType = meth->retTypeAnnotation();
  assert(retType ||
         meth->isNamed("__construct") ||
         meth->isNamed("__destruct"));
  fe->returnType = retType ? retType->dataType() : KindOfNull;
  fe->retUserType = makeStaticString(meth->getReturnTypeConstraint());

  FunctionScopePtr funcScope = meth->getFunctionScope();
  const char *funcname  = funcScope->getScopeName().c_str();
  const char *classname = pce ? pce->name()->data() : nullptr;
  auto const& info = Native::GetBuiltinFunction(funcname, classname,
                                                modifiers->isStatic());

  if (!classname && (
        !strcasecmp(funcname, "fb_call_user_func_safe") ||
        !strcasecmp(funcname, "fb_call_user_func_safe_return") ||
        !strcasecmp(funcname, "fb_call_user_func_array_safe"))) {
    // Legacy optimization functions
    funcScope->setOptFunction(hphp_opt_fb_call_user_func);
  }

  int nativeAttrs = fe->parseNativeAttributes(attributes);
  BuiltinFunction bif = nullptr, nif = nullptr;
  Native::getFunctionPointers(info, nativeAttrs, bif, nif);
  if (nif && !(nativeAttrs & Native::AttrZendCompat)) {
    if (retType) {
      fe->retTypeConstraint =
        determine_type_constraint_from_annot(retType, true);
    } else {
      fe->retTypeConstraint = TypeConstraint {
        s_Void.get(),
        TypeConstraint::ExtendedHint | TypeConstraint::HHType
      };
    }
  }

  Emitter e(meth, m_ue, *this);
  Label topOfBody(e);

  Offset base = m_ue.bcPos();

  if (meth->getFunctionScope()->userAttributes().count(attr_Deprecated)) {
    emitDeprecationWarning(e, meth);
  }

  fe->setBuiltinFunc(bif, nif, attributes, base);
  fillFuncEmitterParams(fe, meth->getParams(), true);
  int32_t stackPad = 0;
  if (nativeAttrs & Native::AttrOpCodeImpl) {
    stackPad = emitNativeOpCodeImpl(meth, funcname, classname, fe);
  } else {
    e.NativeImpl();
  }
  FuncFinisher ff(this, e, fe, stackPad);
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
    auto par =
      static_pointer_cast<ParameterExpression>((*meth->getParams())[i]);
    fe->allocVarId(makeStaticString(par->getName()));
  }

  // assign ids to 0Closure and use parameters (closures)
  if (fe->isClosureBody) {
    fe->allocVarId(makeStaticString("0Closure"));

    for (auto& useVar : *useVars) {
      fe->allocVarId(useVar.first);
    }
  }

  // assign id to 86metadata local representing frame metadata
  if (meth->mayCallSetFrameMetadata()) {
    fe->allocVarId(makeStaticString("86metadata"));
  }

  // assign ids to local variables
  if (!fe->isMemoizeWrapper) {
    assignLocalVariableIds(meth->getFunctionScope());
  }

  // add parameter info
  fillFuncEmitterParams(fe, meth->getParams(),
                        meth->getFunctionScope()->isParamCoerceMode());

  // copy declared return type (hack)
  fe->retUserType = makeStaticString(meth->getReturnTypeConstraint());

  auto annot = meth->retTypeAnnotation();
  // For a non-generator async function with a return annotation of the form
  // "Awaitable<T>", we set m_retTypeConstraint to T. For all other async
  // functions, we leave m_retTypeConstraint empty.
  if (annot && fe->isAsync && !fe->isGenerator) {
    // Semantic checks ensure that the return annotation is "Awaitable" or
    // "WaitHandle" and that it has at most one type parameter
    assert(annot->isAwaitable() || annot->isWaitHandle());
    assert(annot->numTypeArgs() <= 1);
    bool isSoft = annot->isSoft();
    // If annot was "Awaitable" with no type args, getTypeArg() will return an
    // empty annotation
    annot = annot->getTypeArg(0);
    // If the original annotation was soft, make sure we preserve the softness
    if (annot && isSoft) annot->setSoft();
  }
  // Ideally we should handle the void case in TypeConstraint::check. This
  // should however get done in a different diff, since it could impact
  // perf in a negative way (#3145038)
  if (annot && !annot->isVoid() && !annot->isThis()) {
    fe->retTypeConstraint = determine_type_constraint_from_annot(annot, true);
  }

  // add the original filename for flattened traits
  auto const originalFilename = meth->getOriginalFilename();
  if (!originalFilename.empty()) {
    fe->originalFilename = makeStaticString(originalFilename);
  }

  StringData* methDoc = Option::GenerateDocComments ?
    makeStaticString(meth->getDocComment()) : staticEmptyString();

  fe->init(meth->line0(),
           meth->line1(),
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
                                           bool coerce_params /*= false */) {
  int numParam = params ? params->getCount() : 0;
  for (int i = 0; i < numParam; i++) {
    auto par = static_pointer_cast<ParameterExpression>((*params)[i]);
    StringData* parName = makeStaticString(par->getName());

    FuncEmitter::ParamInfo pi;
    auto const typeConstraint = determine_type_constraint(par);
    if (typeConstraint.hasConstraint()) {
      pi.typeConstraint = typeConstraint;
    }
    if (coerce_params) {
      if (auto const typeAnnotation = par->annotation()) {
        pi.builtinType = typeAnnotation->dataType();
      }
    }

    if (par->hasUserType()) {
      pi.userType = makeStaticString(par->getUserTypeHint());
    }

    // Store info about the default value if there is one.
    if (par->isOptional()) {
      const StringData* phpCode;
      ExpressionPtr vNode = par->defaultValue();
      if (vNode->isScalar()) {
        TypedValue dv;
        initScalar(dv, vNode);
        pi.defaultValue = dv;

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
        auto ar = std::make_shared<AnalysisResult>();
        vNode->outputPHP(cg, ar);
        phpCode = makeStaticString(os.str());
      }
      pi.phpCode = phpCode;
    }

    auto paramUserAttrs =
      dynamic_pointer_cast<ExpressionList>(par->userAttributeList());
    if (paramUserAttrs) {
      for (int j = 0; j < paramUserAttrs->getCount(); ++j) {
        auto a = dynamic_pointer_cast<UserAttribute>((*paramUserAttrs)[j]);
        StringData* uaName = makeStaticString(a->getName());
        ExpressionPtr uaValue = a->getExp();
        assert(uaValue);
        assert(uaValue->isScalar());
        TypedValue tv;
        initScalar(tv, uaValue);
        pi.userAttributes[uaName] = tv;
      }
    }

    pi.byRef = par->isRef();
    pi.variadic = par->isVariadic();
    fe->appendParam(parName, pi);
  }
}

void EmitterVisitor::emitMethodPrologue(Emitter& e, MethodStatementPtr meth) {
  FunctionScopePtr funcScope = meth->getFunctionScope();

  if (!m_curFunc->isMemoizeWrapper &&
      funcScope->needsLocalThis() && !funcScope->isStatic()) {
    assert(!m_curFunc->top);
    static const StringData* thisStr = makeStaticString("this");
    Id thisId = m_curFunc->lookupVarId(thisStr);
    emitVirtualLocal(thisId);
    e.InitThisLoc(thisId);
  }

  if (!m_curFunc->isMemoizeImpl) {
    for (uint32_t i = 0; i < m_curFunc->params.size(); i++) {
      const TypeConstraint& tc = m_curFunc->params[i].typeConstraint;
      if (!tc.hasConstraint()) continue;
      emitVirtualLocal(i);
      e.VerifyParamType(i);
    }
  }

  if (funcScope->isAbstract()) {
    std::ostringstream s;
    s << "Cannot call abstract method " << meth->getOriginalFullName() << "()";
    emitMakeUnitFatal(e, s.str().c_str(), FatalOp::RuntimeOmitFrame);
  }
}

void EmitterVisitor::emitDeprecationWarning(Emitter& e,
                                            MethodStatementPtr meth) {
  auto funcScope = meth->getFunctionScope();

  auto userAttributes DEBUG_ONLY = funcScope->userAttributes();
  assert(userAttributes.find(attr_Deprecated) != userAttributes.end());

  // Include the message from <<__Deprecated('<message>')>> in the warning
  auto deprArgs = funcScope->getUserAttributeParams(attr_Deprecated);
  auto deprMessage = deprArgs.empty()
    ? s_is_deprecated.data()
    : deprArgs.front()->getString();

  // how often to display the warning (1 / rate)
  auto rate = deprArgs.size() > 1 ? deprArgs[1]->getLiteralInteger() : 1;
  if (rate <= 0) {
    // deprecation warnings disabled
    return;
  }

  { // preface the message with the name of the offending function
    auto funcName = funcScope->getScopeName();
    BlockScopeRawPtr b = funcScope->getOuterScope();
    if (b && b->is(BlockScope::ClassScope)) {
      auto clsScope = dynamic_pointer_cast<ClassScope>(b);
      if (clsScope->isTrait()) {
        e.Self();
        e.NameA();
        e.String(makeStaticString("::" + funcName + ": " + deprMessage));
        e.Concat();
      } else {
        e.String(makeStaticString(
                   clsScope->getScopeName() + "::" + funcName
                   + ": " + deprMessage));
      }
    } else {
      e.String(makeStaticString(funcName + ": " + deprMessage));
    }
  }

  e.Int(rate);
  e.Int((funcScope->isSystem() || funcScope->isNative())
        ? k_E_DEPRECATED : k_E_USER_DEPRECATED);
  e.FCallBuiltin(3, 3, s_trigger_sampled_error.get());
  emitPop(e);
}

void EmitterVisitor::emitMethod(MethodStatementPtr meth) {
  auto region = createRegion(meth, Region::Kind::FuncBody);
  enterRegion(region);
  SCOPE_EXIT { leaveRegion(region); };

  Emitter e(meth, m_ue, *this);
  Label topOfBody(e);
  emitMethodPrologue(e, meth);

  if (meth->getFunctionScope()->userAttributes().count(attr_Deprecated)) {
    emitDeprecationWarning(e, meth);
  }

  // emit code to create generator object
  if (m_curFunc->isGenerator) {
    e.CreateCont();
    e.PopC();
  }

  // emit method body
  visit(meth->getStmts());
  assert(m_evalStack.size() == 0);

  // if the current position is reachable, emit code to return null
  if (currentPositionIsReachable()) {
    auto r = meth->getRange();
    r.line0 = r.line1;
    r.char0 = r.char1 - 1;
    e.setTempLocation(r);
    e.Null();
    if (shouldEmitVerifyRetType()) {
      e.VerifyRetTypeC();
    }
    e.RetC();
    e.setTempLocation(OptLocation());
  }

  FuncFinisher ff(this, e, m_curFunc);
  if (!m_curFunc->isMemoizeImpl) {
    emitMethodDVInitializers(e, meth, topOfBody);
  }
}

void EmitterVisitor::emitMethodDVInitializers(Emitter& e,
                                              MethodStatementPtr& meth,
                                              Label& topOfBody) {
  bool hasOptional = false;
  ExpressionListPtr params = meth->getParams();
  int numParam = params ? params->getCount() : 0;
  for (int i = 0; i < numParam; i++) {
    auto par = static_pointer_cast<ParameterExpression>((*params)[i]);
    if (par->isOptional()) {
      hasOptional = true;
      Label entryPoint(e);
      emitVirtualLocal(i);
      visit(par->defaultValue());
      emitCGet(e);
      emitSet(e);
      e.PopC();
      m_curFunc->params[i].funcletOff = entryPoint.getAbsoluteOffset();
    }
  }
  if (hasOptional) e.JmpNS(topOfBody);
}

void EmitterVisitor::addMemoizeProp(MethodStatementPtr meth) {
  assert(m_curFunc->isMemoizeWrapper);

  if (meth->is(Statement::KindOfFunctionStatement)) {
    // Functions use statics within themselves. So all we need to do here is
    // set the name
    m_curFunc->memoizePropName = makeStaticString("static$memoize_cache");
    return;
  }

  auto pce = m_curFunc->pce();
  auto classScope = meth->getClassScope();
  auto funcScope = meth->getFunctionScope();
  bool useSharedProp = !funcScope->isStatic();

  std::string propNameBase;
  if (useSharedProp) {
    propNameBase = "$shared";
    m_curFunc->hasMemoizeSharedProp = true;
    m_curFunc->memoizeSharedPropIndex = pce->getNextMemoizeCacheKey();
  } else {
    propNameBase = toLower(funcScope->getScopeName());
  }

  // The prop definition in traits conflicts with the definition in a class
  // so make a different prop for each trait
  std::string traitNamePart;
  if (classScope && classScope->isTrait()) {
    traitNamePart = toLower(classScope->getScopeName());
    // the backslash comes from namespaces. @jan thought that would cause
    // issues, so use $ instead
    for (char &c: traitNamePart) {
      c = (c == '\\' ? '$' : c);
    }
    traitNamePart += "$";
  }

  m_curFunc->memoizePropName = makeStaticString(
    folly::sformat("{}${}memoize_cache", propNameBase, traitNamePart));

  TypedValue tvProp;
  if (useSharedProp ||
      (meth->getParams() && meth->getParams()->getCount() > 0)) {
    tvProp = make_tv<KindOfPersistentArray>(staticEmptyArray());
  } else {
    tvWriteNull(&tvProp);
  }

  Attr attrs = AttrPrivate | AttrBuiltin;
  attrs = attrs | (funcScope->isStatic() ? AttrStatic : AttrNone);
  pce->addProperty(m_curFunc->memoizePropName, attrs, nullptr, nullptr, &tvProp,
                   RepoAuthType{});
}

void EmitterVisitor::emitMemoizeProp(Emitter& e,
                                     MethodStatementPtr meth,
                                     Id localID,
                                     const std::vector<Id>& paramIDs,
                                     uint32_t numParams) {
  assert(m_curFunc->isMemoizeWrapper);

  if (meth->is(Statement::KindOfFunctionStatement)) {
    emitVirtualLocal(localID);
  } else if (meth->getFunctionScope()->isStatic()) {
    m_evalStack.push(StackSym::K);
    m_evalStack.setClsBaseType(SymbolicStack::CLS_SELF);
    e.String(m_curFunc->memoizePropName);
    markSProp(e);
  } else {
    m_evalStack.push(StackSym::H);
    m_evalStack.setKnownCls(m_curFunc->pce()->name(), false);
    m_evalStack.push(StackSym::T);
    m_evalStack.setString(m_curFunc->memoizePropName);
    markProp(e, PropAccessType::Normal);
  }

  assert(numParams <= paramIDs.size());
  for (uint32_t i = 0; i < numParams; i++) {
    if (i == 0 && m_curFunc->hasMemoizeSharedProp) {
      e.Int(m_curFunc->memoizeSharedPropIndex);
    } else {
      emitVirtualLocal(paramIDs[i]);
    }
    markElem(e);
  }
}

void EmitterVisitor::emitMemoizeMethod(MethodStatementPtr meth,
                                       const StringData* methName) {
  assert(m_curFunc->isMemoizeWrapper);

  if (meth->getFunctionScope()->isRefReturn()) {
    throw IncludeTimeFatalException(meth,
      "<<__Memoize>> cannot be used on functions that return by reference");
  }
  if (meth->getFunctionScope()->allowsVariableArguments()) {
    throw IncludeTimeFatalException(meth,
      "<<__Memoize>> cannot be used on functions with variable arguments");
  }

  auto classScope = meth->getClassScope();
  if (classScope && classScope->isInterface()) {
    throw IncludeTimeFatalException(meth,
      "<<__Memoize>> cannot be used in interfaces");
  }

  bool isFunc = meth->is(Statement::KindOfFunctionStatement);
  int numParams = m_curFunc->params.size();
  std::vector<Id> cacheLookup;

  auto region = createRegion(meth, Region::Kind::FuncBody);
  enterRegion(region);
  SCOPE_EXIT { leaveRegion(region); };

  Emitter e(meth, m_ue, *this);
  Label topOfBody(e);
  Label cacheMiss;

  emitMethodPrologue(e, meth);

  // Function start
  int staticLocalID = 0;
  if (isFunc) {
    // static ${propName} = {numParams > 0 ? array() : null};
    staticLocalID = m_curFunc->allocUnnamedLocal();
    emitVirtualLocal(staticLocalID);
    if (numParams == 0) {
      e.Null();
    } else {
      e.Array(staticEmptyArray());
    }
    e.StaticLocInit(staticLocalID, m_curFunc->memoizePropName);
  } else if (!meth->getFunctionScope()->isStatic()) {
    e.CheckThis();
  }

  if (m_curFunc->hasMemoizeSharedProp) {
    // The code below depends on cacheLookup having the right number of elements
    // Push a dummy value even though we'll use the cacheID as an int instead
    // instead of emitting a local
    cacheLookup.push_back(0);
  }

  if (numParams == 0 && cacheLookup.size() == 0) {
    // if (${propName} !== null)
    emitMemoizeProp(e, meth, staticLocalID, cacheLookup, 0);
    emitIsType(e, IsTypeOp::Null);
    e.JmpNZ(cacheMiss);
  } else {
    // Serialize all the params into something we can use for the key
    for (int i = 0; i < numParams; i++) {
      if (m_curFunc->params[i].byRef) {
        throw IncludeTimeFatalException(meth,
          "<<__Memoize>> cannot be used on functions with args passed by "
          "reference");
      }

      // Translate the arg to a memoize key
      int serResultLocal = m_curFunc->allocUnnamedLocal();
      cacheLookup.push_back(serResultLocal);

      emitVirtualLocal(serResultLocal);
      emitVirtualLocal(i);
      emitCGet(e);
      e.GetMemoKey();
      emitSet(e);
      emitPop(e);
    }

    // isset returns false for null values. Given that:
    //  - If we know that we can't return null, we can do a single isset check
    //  - If we could return null, but there's only one arg, we can do a single
    //    array_key_exists() check
    //  - Otherwise we need an isset check to make sure we can dereference the
    //    first N - 1 args, and then an array_key_exists() check
    int cacheLookupLen = cacheLookup.size();
    bool noRetNull =
      meth->getFunctionScope()->isAsync() ||
        (m_curFunc->retTypeConstraint.hasConstraint() &&
        !m_curFunc->retTypeConstraint.isSoft() &&
        !m_curFunc->retTypeConstraint.isNullable());

    if (cacheLookupLen > 1 || noRetNull) {
      // if (isset(${propName}[$param1]...[noRetNull ? $paramN : $paramN-1]))
      emitMemoizeProp(e, meth, staticLocalID, cacheLookup,
                      noRetNull ? cacheLookupLen : cacheLookupLen - 1);
      emitIsset(e);
      e.JmpZ(cacheMiss);
    }

    if (!noRetNull) {
      // if (array_key_exists($paramN, ${propName}[$param1][...][$paramN-1]))
      if (cacheLookupLen == 1 && m_curFunc->hasMemoizeSharedProp) {
        e.Int(m_curFunc->memoizeSharedPropIndex);
      } else {
        emitVirtualLocal(cacheLookup[cacheLookupLen - 1]);
        emitCGet(e);
      }
      emitMemoizeProp(e, meth, staticLocalID, cacheLookup, cacheLookupLen - 1);
      emitCGet(e);
      e.AKExists();
      e.JmpZ(cacheMiss);
    }
  }

  // return $<propName>[$param1][...][$paramN]
  int cacheLookupLen = cacheLookup.size();
  emitMemoizeProp(e, meth, staticLocalID, cacheLookup, cacheLookupLen);
  emitCGet(e);
  e.RetC();

  // Otherwise, call the memoized func, store the result, and return it
  cacheMiss.set(e);
  emitMemoizeProp(e, meth, staticLocalID, cacheLookup, cacheLookupLen);
  auto fpiStart = m_ue.bcPos();
  if (isFunc) {
    e.FPushFuncD(numParams, methName);
  } else if (meth->getFunctionScope()->isStatic()) {
    emitClsIfSPropBase(e);

    if (classScope && classScope->isTrait()) {
      e.String(methName);
      e.Self();
      fpiStart = m_ue.bcPos();
      e.FPushClsMethodF(numParams);
    } else {
      fpiStart = m_ue.bcPos();
      e.FPushClsMethodD(numParams, methName, m_curFunc->pce()->name());
    }
  } else {
    e.This();
    fpiStart = m_ue.bcPos();
    e.FPushObjMethodD(numParams, methName, ObjMethodOp::NullThrows);
  }
  {
    FPIRegionRecorder fpi(this, m_ue, m_evalStack, fpiStart);
    for (uint32_t i = 0; i < numParams; i++) {
      emitVirtualLocal(i);
      emitFPass(e, i, PassByRefKind::ErrorOnCell);
    }
  }
  e.FCall(numParams);
  emitConvertToCell(e);

  emitSet(e);
  e.RetC();

  assert(m_evalStack.size() == 0);

  FuncFinisher ff(this, e, m_curFunc);
  emitMethodDVInitializers(e, meth, topOfBody);
}

void EmitterVisitor::emitPostponedCtors() {
  while (!m_postponedCtors.empty()) {
    PostponedCtor& p = m_postponedCtors.front();

    Attr attrs = AttrPublic;
    if (!SystemLib::s_inited || p.m_is->getClassScope()->isSystem()) {
      attrs = attrs | AttrBuiltin;
    }
    StringData* methDoc = staticEmptyString();
    p.m_fe->init(p.m_is->line0(), p.m_is->line1(),
                 m_ue.bcPos(), attrs, false, methDoc);
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
  StringData* methDoc = staticEmptyString();
  p.m_fe->init(p.m_is->line0(), p.m_is->line1(),
               m_ue.bcPos(), attrs, false, methDoc);

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
    StringData* methDoc = staticEmptyString();
    p.m_fe->init(p.m_is->line0(), p.m_is->line1(),
                 m_ue.bcPos(), attrs, false, methDoc);
    static const StringData* s_constName = makeStaticString("constName");
    p.m_fe->appendParam(s_constName, FuncEmitter::ParamInfo());

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

void EmitterVisitor::emitVirtualLocal(int localId) {
  prepareEvalStack();

  m_evalStack.push(StackSym::L);
  m_evalStack.setInt(localId);
}

template<class Expr>
void EmitterVisitor::emitVirtualClassBase(Emitter& e, Expr* node) {
  prepareEvalStack();

  m_evalStack.push(StackSym::K);
  auto const func = node->getFunctionScope();

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
  } else if (!node->getClassScope() ||
             node->getClassScope()->isTrait() ||
             (func && func->isClosure())) {
    // In a trait, a potentially rebound closure or psuedo-main, we can't
    // resolve self:: or parent:: yet, so we emit special instructions that do
    // those lookups.
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
             node->getClassScope()->getOriginalParent().empty()) {
    // parent:: in a class without a parent.  We'll emit a Parent
    // opcode because it can handle this error case.
    m_evalStack.setClsBaseType(SymbolicStack::CLS_PARENT);
  } else {
    m_evalStack.setClsBaseType(SymbolicStack::CLS_STRING_NAME);
    m_evalStack.setString(
      makeStaticString(node->getOriginalClassName()));
  }
}

bool EmitterVisitor::emitSystemLibVarEnvFunc(Emitter& e,
                                             SimpleFunctionCallPtr call) {
  if (call->isCallToFunction("extract")) {
    emitFuncCall(e, call,
                 "__SystemLib\\extract", call->getParams());
    return true;
  } else if (call->isCallToFunction("parse_str")) {
    emitFuncCall(e, call, "__SystemLib\\parse_str", call->getParams());
    return true;
  } else if (call->isCallToFunction("compact")) {
    emitFuncCall(e, call,
                 "__SystemLib\\compact_sl", call->getParams());
    return true;
  } else if (call->isCallToFunction("get_defined_vars")) {
    emitFuncCall(e, call,
                 "__SystemLib\\get_defined_vars", call->getParams());
    return true;
  } else if (call->isCallToFunction("func_get_args")) {
    emitFuncCall(e, call,
                 "__SystemLib\\func_get_args_sl", call->getParams());
    return true;
  } else if (call->isCallToFunction("func_get_arg")) {
    emitFuncCall(e, call,
                 "__SystemLib\\func_get_arg_sl", call->getParams());
    return true;
  } else if (call->isCallToFunction("func_num_args")) {
    emitFuncCall(e, call,
                 "__SystemLib\\func_num_arg_", call->getParams());
    return true;
  }
  return false;
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
  if (func->hasUnpack()) {
    throw EmitterVisitor::IncludeTimeFatalException(
      func,
      "Using argument unpacking for a call_user_func is not supported"
    );
  }
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
  if (Option::JitEnableRenameFunction ||
      !RuntimeOption::EvalEnableCallBuiltin) {
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
      f->isMethod() ||
      (f->numParams() > Native::maxFCallBuiltinArgs()) ||
      (f->userAttributes().count(
        LowStringPtr(s_attr_Deprecated.get())))) return nullptr;

  auto variadic = f->hasVariadicCaptureParam();

  // Only allowed to overrun the signature if we have somewhere to put it
  if ((numParams > f->numParams()) && !variadic) return nullptr;

  if ((f->returnType() == KindOfDouble) &&
       !Native::allowFCallBuiltinDoubles()) return nullptr;

  if (f->methInfo()) {
    // IDL style builtin
    const ClassInfo::MethodInfo* info = f->methInfo();
    if (info->attribute & (ClassInfo::NoFCallBuiltin |
                           ClassInfo::VariableArguments |
                           ClassInfo::RefVariableArguments)) {
      return nullptr;
    }
  } else if (!(f->attrs() & AttrNative)) {
    // HNI only enables Variable args via ActRec which in turn
    // is captured by the f->nativeFuncPtr() == nullptr,
    // so there's nothing additional to check in the HNI case
    return nullptr;
  }

  bool allowDoubleArgs = Native::allowFCallBuiltinDoubles();
  auto concrete_params = f->numParams();
  if (variadic) {
    assertx(!f->methInfo());
    assertx(concrete_params > 0);
    --concrete_params;
  }
  for (int i = 0; i < concrete_params; i++) {
    if ((!allowDoubleArgs) &&
        (f->params()[i].builtinType == KindOfDouble)) {
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
        if (pi.isVariadic()) continue;
        if (!pi.hasDefaultValue()) {
          return nullptr;
        }
        if (pi.defaultValue.m_type == KindOfUninit) {
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
  auto const unpack = node->hasUnpack();
  assert(!paramsOverride || !unpack);

  Func* fcallBuiltin = nullptr;
  StringData* nLiteral = nullptr;
  Offset fpiStart = 0;
  if (node->getClass() || node->hasStaticClass()) {
    bool isSelfOrParent = node->isSelf() || node->isParent();
    if (!node->isStatic() && !isSelfOrParent &&
        !node->getOriginalClassName().empty() && !nameStr.empty()) {
      // cls::foo()
      StringData* cLiteral =
        makeStaticString(node->getOriginalClassName());
      StringData* nLiteral = makeStaticString(nameStr);
      fpiStart = m_ue.bcPos();
      e.FPushClsMethodD(numParams, nLiteral, cLiteral);
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
    if (unpack &&
        fcallBuiltin &&
        (!fcallBuiltin->hasVariadicCaptureParam() ||
         numParams != fcallBuiltin->numParams())) {
      fcallBuiltin = nullptr;
    }
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
    auto variadic = !unpack && fcallBuiltin->hasVariadicCaptureParam();
    assertx((numParams <= fcallBuiltin->numParams()) || variadic);

    auto concreteParams = fcallBuiltin->numParams();
    if (variadic) {
      assertx(concreteParams > 0);
      --concreteParams;
    }

    int i = 0;
    for (; i < numParams; i++) {
      // for builtin calls, since we don't push the ActRec, we
      // must determine the reffiness statically
      bool byRef = fcallBuiltin->byRef(i);
      bool mustBeRef = fcallBuiltin->mustBeRef(i);
      if (!emitBuiltinCallArg(e, (*params)[i], i, byRef, mustBeRef)) {
        while (i--) emitPop(e);
        return;
      }
    }

    if (fcallBuiltin->methInfo()) {
      // IDL style
      for (; i < concreteParams; i++) {
        const ClassInfo::ParameterInfo* pi =
          fcallBuiltin->methInfo()->parameters[i];
        Variant v = unserialize_from_string(
          String(pi->value, pi->valueLen, CopyString));
        emitBuiltinDefaultArg(e, v, pi->argType, i);
      }
    } else {
      // HNI style
      for (; i < concreteParams; i++) {
        auto &pi = fcallBuiltin->params()[i];
        assert(pi.hasDefaultValue());
        auto &def = pi.defaultValue;
        emitBuiltinDefaultArg(e, tvAsVariant(const_cast<TypedValue*>(&def)),
                              pi.builtinType, i);
      }
    }
    if (variadic) {
      if (numParams <= concreteParams) {
        e.Array(staticEmptyArray());
      } else {
        e.NewPackedArray(numParams - concreteParams);
      }
    }
    e.FCallBuiltin(fcallBuiltin->numParams(),
                   std::min<int32_t>(numParams, fcallBuiltin->numParams()),
                   nLiteral);
  } else {
    emitCall(e, node, params, fpiStart);
  }
  if (fcallBuiltin) {
    fixReturnType(e, node, fcallBuiltin);
  }
}

void EmitterVisitor::emitClassTraitPrecRule(PreClassEmitter* pce,
                                            TraitPrecStatementPtr stmt) {
  StringData* traitName  = makeStaticString(stmt->getTraitName());
  StringData* methodName = makeStaticString(stmt->getMethodName());

  PreClass::TraitPrecRule rule(traitName, methodName);

  hphp_string_iset otherTraitNames;
  stmt->getOtherTraitNames(otherTraitNames);
  for (auto const& name : otherTraitNames) {
    rule.addOtherTraitName(makeStaticString(name));
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
  auto rules = useStmt->getStmts();
  for (int r = 0; r < rules->getCount(); r++) {
    auto rule = (*rules)[r];
    auto precStmt = dynamic_pointer_cast<TraitPrecStatement>(rule);
    if (precStmt) {
      emitClassTraitPrecRule(pce, precStmt);
    } else {
      auto aliasStmt = dynamic_pointer_cast<TraitAliasStatement>(rule);
      assert(aliasStmt);
      emitClassTraitAliasRule(pce, aliasStmt);
    }
  }
}

Id EmitterVisitor::emitTypedef(Emitter& e, TypedefStatementPtr td) {
  auto const nullable = td->annot->isNullable();
  auto const annot = td->annot->stripNullable();
  auto const valueStr = annot.vanillaName();

  // We have to merge the strings as litstrs to ensure namedentity
  // creation.
  auto const name = makeStaticString(td->name);
  auto const value = makeStaticString(valueStr);
  m_ue.mergeLitstr(name);
  m_ue.mergeLitstr(value);

  AnnotType type;
  if (annot.isFunction() || annot.isMixed()) {
    type = AnnotType::Mixed;
  } else {
    auto const at = nameToAnnotType(value);
    type = at ? *at : AnnotType::Object;
    // Type aliases are always defined at top-level scope, so
    // they're not allowed to reference "self" or "parent" (and
    // "static" is already disallowed by the parser, so we don't
    // need to worry about it here).
    if (UNLIKELY(type == AnnotType::Self || type == AnnotType::Parent)) {
      throw IncludeTimeFatalException(
        td,
        "Cannot access %s when no class scope is active",
        type == AnnotType::Self ? "self" : "parent");
    }
  }

  UserAttributeMap userAttrs;
  ExpressionListPtr attrList = td->attrList;
  if (attrList) {
    for (int i = 0; i < attrList->getCount(); ++i) {
      auto attr = dynamic_pointer_cast<UserAttribute>((*attrList)[i]);
      auto const uaName = makeStaticString(attr->getName());
      auto uaValue = attr->getExp();
      assert(uaValue);
      assert(uaValue->isScalar());
      TypedValue tv;
      initScalar(tv, uaValue);
      userAttrs[uaName] = tv;
    }
  }

  TypeAlias record;
  record.typeStructure = Array(td->annot->getScalarArrayRep());
  record.name = name;
  record.value = value;
  record.type = type;
  record.nullable = nullable;
  record.userAttrs = userAttrs;
  record.attrs = !SystemLib::s_inited ? AttrPersistent : AttrNone;
  Id id = m_ue.addTypeAlias(record);
  e.DefTypeAlias(id);

  return id;
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

  auto is = static_pointer_cast<InterfaceStatement>(cNode->getStmt());
  StringData* className = makeStaticString(cNode->getOriginalName());
  StringData* parentName = makeStaticString(cNode->getOriginalParent());
  StringData* classDoc = Option::GenerateDocComments ?
    makeStaticString(cNode->getDocComment()) : staticEmptyString();
  Attr attr = cNode->isInterface() ? AttrInterface :
              cNode->isTrait()     ? AttrTrait     :
              cNode->isAbstract()  ? AttrAbstract  :
              cNode->isEnum()      ? (AttrEnum | AttrFinal) :
                                     AttrNone;
  if (cNode->isFinal()) {
    attr = attr | AttrFinal;
  }
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

  const std::vector<std::string>& bases(cNode->getBases());
  int firstInterface = cNode->getOriginalParent().empty() ? 0 : 1;
  int nInterfaces = bases.size();
  PreClass::Hoistable hoistable = PreClass::NotHoistable;
  if (toplevel) {
    if (SystemLib::s_inited && !cNode->isSystem()) {
      if (nInterfaces > firstInterface
          || cNode->getUsedTraitNames().size()
          || cNode->getClassRequiredExtends().size()
          || cNode->getClassRequiredImplements().size()
          || cNode->isEnum()
         ) {
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
  pce->init(is->line0(), is->line1(), m_ue.bcPos(), attr, parentName,
            classDoc);
  auto r = is->getRange();
  r.line1 = r.line0;
  r.char1 = r.char0;
  e.setTempLocation(r);
  if (hoistable != PreClass::AlwaysHoistable) {
    e.DefCls(pce->id());
  } else {
    // To attach the line number to for error reporting.
    e.DefClsNop(pce->id());
  }
  e.setTempLocation(OptLocation());
  for (int i = firstInterface; i < nInterfaces; ++i) {
    pce->addInterface(makeStaticString(bases[i]));
  }

  const std::vector<std::string>& usedTraits = cNode->getUsedTraitNames();
  for (size_t i = 0; i < usedTraits.size(); i++) {
    pce->addUsedTrait(makeStaticString(usedTraits[i]));
  }
  pce->setNumDeclMethods(cNode->getNumDeclMethods());
  if (cNode->isTrait() || cNode->isInterface() || Option::WholeProgram) {
    for (auto& reqExtends : cNode->getClassRequiredExtends()) {
      pce->addClassRequirement(
        PreClass::ClassRequirement(makeStaticString(reqExtends), true));
    }
    for (auto& reqImplements : cNode->getClassRequiredImplements()) {
      pce->addClassRequirement(
        PreClass::ClassRequirement(makeStaticString(reqImplements), false));
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
      if (auto meth = dynamic_pointer_cast<MethodStatement>((*stmts)[i])) {
        StringData* methName = makeStaticString(meth->getOriginalName());
        FuncEmitter* fe = m_ue.newMethodEmitter(methName, pce);
        bool added UNUSED = pce->addMethod(fe);
        assert(added);
        postponeMeth(meth, fe, false);
      } else if (auto cv = dynamic_pointer_cast<ClassVariable>((*stmts)[i])) {
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
            auto ae = static_pointer_cast<AssignmentExpression>(exp);
            var = static_pointer_cast<SimpleVariable>(ae->getVariable());
            vNode = ae->getValue();
          } else {
            var = static_pointer_cast<SimpleVariable>(exp);
          }

          auto const propName = makeStaticString(var->getName());
          auto const propDoc = Option::GenerateDocComments ?
            makeStaticString(var->getDocComment()) : staticEmptyString();
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
                             propDoc, &tvVal, RepoAuthType{});
          assert(added);
        }
      } else if (auto cc = dynamic_pointer_cast<ClassConstant>((*stmts)[i])) {

        ExpressionListPtr el(cc->getConList());
        StringData* typeConstraint =
          makeStaticString(cc->getTypeConstraint());
        int nCons = el->getCount();

        if (cc->isAbstract()) {
          for (int ii = 0; ii < nCons; ii++) {
            auto con = static_pointer_cast<ConstantExpression>((*el)[ii]);
            StringData* constName = makeStaticString(con->getName());
            bool added UNUSED =
              pce->addAbstractConstant(constName, typeConstraint,
                                       cc->isTypeconst());
            assert(added);
          }
        } else {
          for (int ii = 0; ii < nCons; ii++) {
            auto ae = static_pointer_cast<AssignmentExpression>((*el)[ii]);
            auto con =
              static_pointer_cast<ConstantExpression>(ae->getVariable());
            auto vNode = ae->getValue();
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
            auto ar = std::make_shared<AnalysisResult>();
            vNode->outputPHP(cg, ar);
            bool added UNUSED = pce->addConstant(
              constName, typeConstraint, &tvVal,
              makeStaticString(os.str()),
              cc->isTypeconst(),
              cc->getTypeStructure());
            assert(added);
          }
        }
      } else if (auto useStmt =
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

  // If this is an enum, get its type constraint.
  if (cNode->isEnum()) {
    auto cs = static_pointer_cast<ClassStatement>(is);
    auto const typeConstraint =
      determine_type_constraint_from_annot(cs->getEnumBaseTy(), true);
    pce->setEnumBaseTy(typeConstraint);
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
                                               std::function<void()> emitSrc) {
  std::vector<IndexPair> indexPairs;
  IndexChain workingChain;
  listAssignmentVisitLHS(e, la, workingChain, indexPairs);

  if (indexPairs.size() == 0) {
    throw IncludeTimeFatalException(la, "Cannot use empty list");
  }

  listAssignmentAssignElements(e, indexPairs, emitSrc);
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
    auto svVal = static_pointer_cast<SimpleVariable>(val);
    StringData* name = makeStaticString(svVal->getName());
    valTempLocal = m_curFunc->lookupVarId(name);
    if (key) {
      auto svKey = static_pointer_cast<SimpleVariable>(key);
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
      emitClsIfSPropBase(e);
    }
    if (listVal) {
      emitForeachListAssignment(
        e,
        ListAssignmentPtr(static_pointer_cast<ListAssignment>(val)),
        [&] { emitVirtualLocal(valTempLocal); }
      );
    } else {
      visit(val);
      emitClsIfSPropBase(e);
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
          [&] { emitVirtualLocal(keyTempLocal); }
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

void EmitterVisitor::emitForeachAwaitAs(Emitter& e,
                                        ForEachStatementPtr fe) {
  assert(!fe->isStrong());
  auto region = createRegion(fe, Region::Kind::LoopOrSwitch);
  Label& exit = registerBreak(fe, region.get(), 1, false)->m_label;
  Label& next = registerContinue(fe, region.get(), 1, false)->m_label;

  // Evaluate the AsyncIterator object and store it into unnamed local.
  auto const iterTempLocal = m_curFunc->allocUnnamedLocal();
  emitVirtualLocal(iterTempLocal);
  visit(fe->getArrayExp());
  emitConvertToCell(e);
  emitSet(e);
  auto const iterTempStartUse = m_ue.bcPos();

  // Make sure it actually is an AsyncIterator.
  e.InstanceOfD(makeStaticString("HH\\AsyncIterator"));
  e.JmpNZ(next);
  e.String(makeStaticString(
    "Unable to iterate non-AsyncIterator asynchronously"));
  e.Fatal(FatalOp::Runtime);

  // Start of the next iteration.
  next.set(e);

  // Await the next value.
  emitVirtualLocal(iterTempLocal);
  emitCGet(e);
  emitConstMethodCallNoParams(e, "next");
  e.Await();
  auto const resultTempLocal = emitSetUnnamedL(e);

  // Did we finish yet?
  emitVirtualLocal(resultTempLocal);
  emitIsType(e, IsTypeOp::Null);
  e.JmpNZ(exit);

  auto const populate = [&](ExpressionPtr target, int index) {
    auto const emitSrc = [&] {
      emitVirtualLocal(resultTempLocal);
      m_evalStack.push(StackSym::I);
      m_evalStack.setInt(index);
      markElem(e);
    };

    if (target->is(Expression::KindOfListAssignment)) {
      emitForeachListAssignment(
        e,
        ListAssignmentPtr(static_pointer_cast<ListAssignment>(target)),
        emitSrc
      );
    } else {
      // Obtain target to be set.
      visit(target);

      // Put $result[index] on the stack.
      emitSrc();
      emitCGet(e);

      // Set target.
      emitSet(e);
      emitPop(e);
    }
  };

  auto const resultTempStartUse = m_ue.bcPos();

  // Set the key.
  if (fe->getNameExp()) {
    populate(fe->getNameExp(), 0);
  }

  // Set the value.
  populate(fe->getValueExp(), 1);

  newFaultRegionAndFunclet(resultTempStartUse, m_ue.bcPos(),
                           new UnsetUnnamedLocalThunklet(resultTempLocal));
  emitVirtualLocal(resultTempLocal);
  emitUnset(e);

  // Run body.
  {
    enterRegion(region);
    SCOPE_EXIT { leaveRegion(region); };
    if (fe->getBody()) visit(fe->getBody());
  }

  // Continue iteration.
  e.Jmp(next);

  // Exit cleanup.
  exit.set(e);

  emitVirtualLocal(resultTempLocal);
  emitUnset(e);
  m_curFunc->freeUnnamedLocal(resultTempLocal);

  newFaultRegionAndFunclet(iterTempStartUse, m_ue.bcPos(),
                           new UnsetUnnamedLocalThunklet(iterTempLocal));
  emitVirtualLocal(iterTempLocal);
  emitUnset(e);
  m_curFunc->freeUnnamedLocal(iterTempLocal);
}

void EmitterVisitor::emitYieldFrom(Emitter& e, ExpressionPtr exp) {
  Id itId = m_curFunc->allocIterator();

  // Set the delegate to the result of visiting our expression
  visit(exp);
  emitConvertToCell(e);
  e.ContAssignDelegate(itId);

  Offset bDelegateAssigned = m_ue.bcPos();

  // Pass null to ContEnterDelegate initially.
  e.Null();

  Label loopBeginning(e);
  e.ContEnterDelegate();
  e.YieldFromDelegate(itId, loopBeginning);
  newFaultRegionAndFunclet(bDelegateAssigned, m_ue.bcPos(),
                           new UnsetGeneratorDelegateThunklet(itId));

  // Now that we're done with it, remove the delegate. This lets us enforce
  // the invariant that if we have a delegate set, we should be using it.
  e.ContUnsetDelegate(itId, false);
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
  emitVirtualLocal(oldLevelLoc);
  auto idx = m_evalStack.size() - 1;
  e.Silence(m_evalStack.getLoc(idx), SilenceOp::End);
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
  m_funclets.push_back(new Funclet(body));
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
    auto& e = fe->addEHEnt();
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
    auto& e = fe->addEHEnt();
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

void EmitterVisitor::saveMaxStackCells(FuncEmitter* fe, int32_t stackPad) {
  fe->maxStackCells = m_actualStackHighWater +
                      fe->numIterators() * kNumIterCells +
                      fe->numLocals() +
                      m_fdescHighWater +
                      stackPad;
  m_actualStackHighWater = 0;
  m_fdescHighWater = 0;
}

// Are you sure you mean to be calling this directly? Would FuncFinisher
// be more appropriate?
void EmitterVisitor::finishFunc(Emitter& e, FuncEmitter* fe, int32_t stackPad) {
  emitFunclets(e);
  saveMaxStackCells(fe, stackPad);
  copyOverCatchAndFaultRegions(fe);
  copyOverFPIRegions(fe);
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

void EmitterVisitor::initScalar(TypedValue& tvVal, ExpressionPtr val,
                                folly::Optional<CollectionType> ct) {
  assert(val->isScalar());
  tvVal.m_type = KindOfUninit;
  // static array initilization
  auto initArray = [&](ExpressionPtr el) {
    m_staticArrays.push_back(Array::attach(PackedArray::MakeReserve(0)));
    m_staticColType.push_back(ct);
    visit(el);
    tvVal = make_tv<KindOfPersistentArray>(
      ArrayData::GetScalarArray(m_staticArrays.back().get())
    );
    m_staticArrays.pop_back();
    m_staticColType.pop_back();
  };
  switch (val->getKindOf()) {
    case Expression::KindOfConstantExpression: {
      auto ce = static_pointer_cast<ConstantExpression>(val);
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
      auto sval = static_pointer_cast<ScalarExpression>(val);
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
    case Expression::KindOfExpressionList: {
      // Array, possibly for collection initialization.
      initArray(val);
      break;
    }
    case Expression::KindOfUnaryOpExpression: {
      auto u = static_pointer_cast<UnaryOpExpression>(val);
      if (u->getOp() == T_ARRAY) {
        initArray(u->getExpression());
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

void EmitterVisitor::emitArrayInit(Emitter& e, ExpressionListPtr el,
                                   folly::Optional<CollectionType> ct) {
  assert(m_staticArrays.empty());

  if (el == nullptr) {
    e.Array(staticEmptyArray());
    return;
  }

  if (el->isScalar()) {
    TypedValue tv;
    tvWriteUninit(&tv);
    initScalar(tv, el, ct);
    e.Array(tv.m_data.parr);
    return;
  }

  auto const allowPacked = !ct || isVectorCollection(*ct);

  int nElms;
  if (allowPacked && isPackedInit(el, &nElms)) {
    for (int i = 0; i < nElms; ++i) {
      auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
      visit(ap->getValue());
      emitConvertToCell(e);
    }
    e.NewPackedArray(nElms);
    return;
  }

  // Don't emit struct arrays for a collection initializers.  HashCollection
  // can't handle that yet.  Also ignore RuntimeOption::DisableStructArray here.
  // The VM can handle the NewStructArray bytecode when struct arrays are
  // disabled.
  auto const allowStruct = !ct;
  std::vector<std::string> keys;
  if (allowStruct && isStructInit(el, keys)) {
    for (int i = 0, n = keys.size(); i < n; i++) {
      auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
      visit(ap->getValue());
      emitConvertToCell(e);
    }
    e.NewStructArray(keys);
    return;
  }

  auto capacityHint = MixedArray::SmallSize;
  auto const capacity = el->getCount();
  if (capacity > 0) capacityHint = capacity;
  if (allowPacked && isPackedInit(el, &nElms, false /* ignore size */)) {
    e.NewArray(capacityHint);
  } else {
    e.NewMixedArray(capacityHint);
  }
  visit(el);
}

void EmitterVisitor::emitPairInit(Emitter& e, ExpressionListPtr el) {
  if (el->getCount() != 2) {
    throw IncludeTimeFatalException(el,
      "Pair objects must have exactly 2 elements");
  }
  e.NewCol(static_cast<int>(CollectionType::Pair));
  for (int i = 0; i < 2; i++) {
    auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
    if (ap->getName() != nullptr) {
      throw IncludeTimeFatalException(ap,
        "Keys may not be specified for Pair initialization");
    }
    visit(ap->getValue());
    emitConvertToCell(e);
    e.ColAddNewElemC();
  }
}

void EmitterVisitor::emitVectorInit(Emitter&e, CollectionType ct,
                                    ExpressionListPtr el) {
  // Do not allow specification of keys even if the resulting array is
  // packed. It doesn't make sense to specify keys for Vectors.
  for (int i = 0; i < el->getCount(); i++) {
    auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
    if (ap->getName() != nullptr) {
      throw IncludeTimeFatalException(ap,
        "Keys may not be specified for Vector initialization");
    }
  }
  emitArrayInit(e, el, ct);
  e.ColFromArray(static_cast<int>(ct));
  return;
}

void EmitterVisitor::emitSetInit(Emitter&e, CollectionType ct,
                                 ExpressionListPtr el) {
  /*
   * Use an array to initialize the Set only if all the following conditional
   * are met:
   * 1. non-empty initializer;
   * 2. no integer-like string values (keys are the same as values for Set);
   * 3. !arr->isVectorData() to guarantee that we have a MixedArray.
   *
   * Effectively, we use array for Set initialization only when it is a static
   * array for now.
   */
  auto const nElms = el->getCount();
  auto useArray = !!nElms;
  auto hasVectorData = true;
  for (int i = 0; i < nElms; i++) {
    auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
    auto key = ap->getName();
    if ((bool)key) {
      throw IncludeTimeFatalException(ap,
        "Keys may not be specified for Set initialization");
    }
    if (!useArray) continue;
    auto val = ap->getValue();
    Variant v;
    if (val->getScalarValue(v)) {
      if (v.isString()) {
        hasVectorData = false;
        int64_t intVal;
        if (v.getStringData()->isStrictlyInteger(intVal)) {
          useArray = false;
        }
      } else if (v.isInteger()) {
        if (v.asInt64Val() != i) hasVectorData = false;
      } else {
        useArray = false;
      }
    } else {
      useArray = false;
    }
  }
  if (hasVectorData) useArray = false;

  if (useArray) {
    emitArrayInit(e, el, ct);
    e.ColFromArray(static_cast<int>(ct));
  } else {
    if (nElms == 0) {
      // Will use the static empty mixed array to avoid allocation.
      e.NewCol(static_cast<int>(ct));
      return;
    }
    e.NewMixedArray(nElms);
    e.ColFromArray(static_cast<int>(ct));
    for (int i = 0; i < nElms; i++) {
      auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
      visit(ap->getValue());
      emitConvertToCell(e);
      e.ColAddNewElemC();
    }
  }
}

void EmitterVisitor::emitMapInit(Emitter&e, CollectionType ct,
                                 ExpressionListPtr el) {
  /*
   * Use an array to initialize the Map only when all the following conditional
   * are met:
   * 1. non-empty initializer;
   * 2. no integer-like string keys;
   * 3. !arr->isVectorData() to guarantee that we have a MixedArray.
   */
  auto nElms = el->getCount();
  auto useArray = !!nElms;
  auto hasVectorData = true;
  int64_t max = 0;
  for (int i = 0; i < nElms; i++) {
    auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
    auto key = ap->getName();
    if (key == nullptr) {
      throw IncludeTimeFatalException(ap,
        "Keys must be specified for Map initialization");
    }
    if (!useArray) continue;
    Variant vkey;
    if (key->getScalarValue(vkey)) {
      if (vkey.isString()) {
        hasVectorData = false;
        int64_t intKey;
        if (vkey.getStringData()->isStrictlyInteger(intKey)) {
          useArray = false;
        }
      } else if (vkey.isInteger()) {
        auto val = vkey.asInt64Val();
        if (val > max || val < 0) {
          hasVectorData = false;
        } else if (val == max) {
          ++max;
        }
      } else {
        useArray = false;
      }
    } else {
      useArray = false;
    }
  }
  if (hasVectorData) useArray = false;

  if (useArray) {
    emitArrayInit(e, el, ct);
    e.ColFromArray(static_cast<int>(ct));
  } else {
    if (nElms == 0) {
      e.NewCol(static_cast<int>(ct));
      return;
    }
    e.NewMixedArray(nElms);
    e.ColFromArray(static_cast<int>(ct));
    for (int i = 0; i < nElms; i++) {
      auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
      visit(ap->getName());
      emitConvertToCell(e);
      visit(ap->getValue());
      emitConvertToCell(e);
      e.MapAddElemC();
    }
  }
}

void EmitterVisitor::emitCollectionInit(Emitter& e, BinaryOpExpressionPtr b) {
  auto cls = static_pointer_cast<ScalarExpression>(b->getExp1());
  const std::string* clsName = nullptr;
  cls->getString(clsName);
  auto ct = collections::stringToType(*clsName);
  if (!ct) {
    throw IncludeTimeFatalException(b,
      "Cannot use collection initialization for non-collection class");
  }

  ExpressionListPtr el = nullptr;
  if (b->getExp2()) {
    el = static_pointer_cast<ExpressionList>(b->getExp2());
  } else {
    if (ct == CollectionType::Pair) {
      throw IncludeTimeFatalException(b, "Initializer needed for Pair object");
    }
    e.NewCol(static_cast<int>(*ct));
    return;
  }

  if (ct == CollectionType::Pair) {
    return emitPairInit(e, el);
  }

  if (ct == CollectionType::Vector || ct == CollectionType::ImmVector) {
    return emitVectorInit(e, *ct, el);
  }

  if (ct == CollectionType::Map || ct == CollectionType::ImmMap) {
    return emitMapInit(e, *ct, el);
  }

  if (ct == CollectionType::Set || ct == CollectionType::ImmSet) {
    return emitSetInit(e, *ct, el);
  }

  not_reached();
}

bool EmitterVisitor::requiresDeepInit(ExpressionPtr initExpr) const {
  switch (initExpr->getKindOf()) {
    case Expression::KindOfScalarExpression:
      return false;
    case Expression::KindOfClassConstantExpression:
    case Expression::KindOfConstantExpression:
      return !initExpr->isScalar();
    case Expression::KindOfUnaryOpExpression: {
      auto u = static_pointer_cast<UnaryOpExpression>(initExpr);
      if (u->getOp() == T_ARRAY) {
        auto el = static_pointer_cast<ExpressionList>(u->getExpression());
        if (el) {
          int n = el->getCount();
          for (int i = 0; i < n; i++) {
            auto ap = static_pointer_cast<ArrayPairExpression>((*el)[i]);
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
      return true;
    }
    case Expression::KindOfBinaryOpExpression: {
      auto b = static_pointer_cast<BinaryOpExpression>(initExpr);
      return requiresDeepInit(b->getExp1()) || requiresDeepInit(b->getExp2());
    }
    default:
      return true;
  }
}

Thunklet::~Thunklet() {}

static ConstructPtr doOptimize(ConstructPtr c, AnalysisResultConstPtr ar) {
  for (int i = 0, n = c->getKidCount(); i < n; i++) {
    if (ConstructPtr k = c->getNthKid(i)) {
      if (ConstructPtr rep = doOptimize(k, ar)) {
        c->setNthKid(i, rep);
      }
    }
  }
  if (auto e = dynamic_pointer_cast<Expression>(c)) {
    switch (e->getKindOf()) {
      case Expression::KindOfBinaryOpExpression:
      case Expression::KindOfUnaryOpExpression:
      case Expression::KindOfIncludeExpression:
      case Expression::KindOfSimpleFunctionCall:
        return e->preOptimize(ar);
      case Expression::KindOfClosureExpression: {
        auto cl = static_pointer_cast<ClosureExpression>(e);
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

  auto msp =
    dynamic_pointer_cast<MethodStatement>(fsp->getPseudoMain()->getStmt());
  UnitEmitter* ue = new UnitEmitter(md5);
  ue->m_preloadPriority = fsp->preloadPriority();
  ue->initMain(msp->line0(), msp->line1());
  EmitterVisitor ev(*ue);
  try {
    ev.visit(fsp);
  } catch (EmitterVisitor::IncludeTimeFatalException& ex) {
    // Replace the unit with an empty one, but preserve its file path.
    UnitEmitter* nue = new UnitEmitter(md5);
    nue->initMain(msp->line0(), msp->line1());
    nue->m_filepath = ue->m_filepath;
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

const StaticString s_systemlibNativeFunc("/:systemlib.nativefunc");
const StaticString s_systemlibNativeCls("/:systemlib.nativecls");

const MD5 s_nativeFuncMD5("11111111111111111111111111111111");
const MD5 s_nativeClassMD5("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");

static std::unique_ptr<UnitEmitter>
emitHHBCNativeFuncUnit(const HhbcExtFuncInfo* builtinFuncs,
                       ssize_t numBuiltinFuncs) {
  auto ue = folly::make_unique<UnitEmitter>(s_nativeFuncMD5);
  ue->m_filepath = s_systemlibNativeFunc.get();
  ue->addTrivialPseudoMain();

  Attr attrs = AttrBuiltin | AttrUnique | AttrPersistent;

  for (ssize_t i = 0; i < numBuiltinFuncs; ++i) {
    const HhbcExtFuncInfo* info = &builtinFuncs[i];
    StringData* name = makeStaticString(info->m_name);
    BuiltinFunction bif = (BuiltinFunction)info->m_builtinFunc;
    BuiltinFunction nif = (BuiltinFunction)info->m_nativeFunc;
    const ClassInfo::MethodInfo* mi = ClassInfo::FindFunction(String{name});
    assert(mi &&
      "MethodInfo not found; may be a problem with the .idl.json files");

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
    assert(!fe->numIterators());
    fe->maxStackCells = fe->numLocals();
    fe->attrs |= attrs;
    fe->finish(ue->bcPos(), false);
    ue->recordFunction(fe);
  }

  return ue;
}

enum GeneratorMethod {
  METH_NEXT,
  METH_SEND,
  METH_RAISE,
  METH_VALID,
  METH_CURRENT,
  METH_KEY,
  METH_REWIND,
  METH_GETRETURN,
};

typedef hphp_hash_map<const StringData*, GeneratorMethod,
                      string_data_hash, string_data_same> ContMethMap;
typedef std::map<StaticString, GeneratorMethod> ContMethMapT;

namespace {
StaticString s_next("next");
StaticString s_send("send");
StaticString s_raise("raise");
StaticString s_valid("valid");
StaticString s_current("current");
StaticString s_key("key");
StaticString s_throw("throw");
StaticString s_rewind("rewind");
StaticString s_getReturn("getReturn");

StaticString genCls("Generator");
StaticString asyncGenCls("HH\\AsyncGenerator");

ContMethMapT s_asyncGenMethods = {
    {s_next, GeneratorMethod::METH_NEXT},
    {s_send, GeneratorMethod::METH_SEND},
    {s_raise, GeneratorMethod::METH_RAISE}
  };
ContMethMapT s_genMethods = {
    {s_next, GeneratorMethod::METH_NEXT},
    {s_send, GeneratorMethod::METH_SEND},
    {s_raise, GeneratorMethod::METH_RAISE},
    {s_valid, GeneratorMethod::METH_VALID},
    {s_current, GeneratorMethod::METH_CURRENT},
    {s_key, GeneratorMethod::METH_KEY},
    {s_throw, GeneratorMethod::METH_RAISE},
    {s_rewind, GeneratorMethod::METH_REWIND},
    {s_getReturn, GeneratorMethod::METH_GETRETURN}
  };
}

static int32_t emitGeneratorMethod(UnitEmitter& ue,
                                   FuncEmitter* fe,
                                   GeneratorMethod m,
                                   bool isAsync) {
  Attr attrs = (Attr)(AttrBuiltin | AttrPublic);
  fe->init(0, 0, ue.bcPos(), attrs, false, staticEmptyString());

  if (!isAsync && RuntimeOption::AutoprimeGenerators) {
    // Create a dummy Emitter, so it's possible to emit jump instructions
    EmitterVisitor ev(ue);
    Emitter e(ConstructPtr(), ue, ev);
    Location::Range loc;
    if (ue.bcPos() > 0) loc.line0 = -1;
    e.setTempLocation(loc);

    // Check if the generator has started yet
    Label started;
    e.ContStarted();
    e.JmpNZ(started);

    // If it hasn't started, perform one "next" operation before
    // the actual operation (auto-priming)
    e.ContCheck(false);
    e.Null();
    e.ContEnter();
    e.PopC();
    started.set(e);
  }

  if (!RuntimeOption::AutoprimeGenerators && m == METH_REWIND) {
    // In non-autopriming mode the rewind function will always call next, when
    // autopriming is enabled, rewind matches PHP behavior and will only advance
    // the generator when it has not yet been started.
    m = METH_NEXT;
  }

  switch (m) {
    case METH_SEND:
    case METH_RAISE:
    case METH_NEXT: {
      // We always want these methods to be cloned with new funcids in
      // subclasses so we can burn Class*s and Func*s into the
      // translations
      fe->attrs |= AttrClone;

      // check generator status; send()/raise() also checks started
      ue.emitOp(OpContCheck);
      ue.emitIVA(m == METH_SEND || m == METH_RAISE);

      switch (m) {
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

      // Backtrace has off-by-one bug when determining whether we are
      // in returning opcode; add Nop to avoid it
      ue.emitOp(OpNop);
      ue.emitOp(OpRetC);
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
    case METH_REWIND: {
      ue.emitOp(OpNull);
      ue.emitOp(OpRetC);
      break;
    }
    case METH_GETRETURN: {
      ue.emitOp(OpContGetReturn);
      ue.emitOp(OpRetC);
      break;
    }
    default:
      not_reached();
  }

  return 1;  // Above cases push at most one stack cell.
}

// HH\WaitHandle::result()
static int32_t emitWaitHandleResult(UnitEmitter& ue,
                                    FuncEmitter* fe) {
  Attr attrs = (Attr)(AttrBuiltin | AttrPublic);
  fe->init(0, 0, ue.bcPos(), attrs, false, staticEmptyString());
  ue.emitOp(OpThis);
  ue.emitOp(OpWHResult);
  ue.emitOp(OpRetC);
  return 1;
}

// Emit byte codes to implement methods. Return the maximum stack cell count.
int32_t EmitterVisitor::emitNativeOpCodeImpl(MethodStatementPtr meth,
                                             const char* funcName,
                                             const char* className,
                                             FuncEmitter* fe) {
  GeneratorMethod* cmeth;
  StaticString s_func(funcName);
  StaticString s_class(className);

  if (genCls.same(s_class) &&
      (cmeth = folly::get_ptr(s_genMethods, s_func))) {
    return emitGeneratorMethod(m_ue, fe, *cmeth, false);
  } else if (asyncGenCls.same(s_class) &&
      (cmeth = folly::get_ptr(s_asyncGenMethods, s_func))) {
    return emitGeneratorMethod(m_ue, fe, *cmeth, true);
  } else if (s_HH_WaitHandle.same(s_class) &&
             s_result.same(s_func)) {
    return emitWaitHandleResult(m_ue, fe);
  }

  throw IncludeTimeFatalException(meth,
    "OpCodeImpl attribute is not applicable to %s", funcName);
}

StaticString s_construct("__construct");
static std::unique_ptr<UnitEmitter>
emitHHBCNativeClassUnit(const HhbcExtClassInfo* builtinClasses,
                        ssize_t numBuiltinClasses) {
  auto ue = folly::make_unique<UnitEmitter>(s_nativeClassMD5);
  ue->m_filepath = s_systemlibNativeCls.get();
  ue->addTrivialPseudoMain();

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
      e.ci = ClassInfo::FindSystemClassInterfaceOrTrait(String{e.name});
      assert(e.ci);
      StringData* parentName
        = makeStaticString(e.ci->getParentClass().get());
      if (parentName->empty()) {
        // If this class doesn't have a base class, it's eligible to be
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
      StringData* methName = makeStaticString(methodInfo->m_name);

      FuncEmitter* fe = ue->newMethodEmitter(methName, pce);
      pce->addMethod(fe);
      auto stackPad = int32_t{0};
      if (e.name->isame(s_construct.get())) {
        hasCtor = true;
      }

      // Build the function
      BuiltinFunction bcf = (BuiltinFunction)methodInfo->m_pGenericMethod;
      auto nativeFunc = methodInfo->m_nativeFunc;
      const ClassInfo::MethodInfo* mi =
        e.ci->getMethodInfo(std::string(methodInfo->m_name));
      Offset base = ue->bcPos();
      fe->setBuiltinFunc(mi,
        bcf,
        reinterpret_cast<BuiltinFunction>(nativeFunc),
        base
      );
      ue->emitOp(OpNativeImpl);

      Offset past = ue->bcPos();
      assert(!fe->numIterators());
      fe->maxStackCells = fe->numLocals() + stackPad;
      fe->finish(past, false);
      ue->recordFunction(fe);
    }
    if (!hasCtor) {
      static const StringData* methName = makeStaticString("86ctor");
      FuncEmitter* fe = ue->newMethodEmitter(methName, pce);
      bool added UNUSED = pce->addMethod(fe);
      assert(added);
      fe->init(0, 0, ue->bcPos(), AttrBuiltin|AttrPublic,
               false, staticEmptyString());
      ue->emitOp(OpNull);
      ue->emitOp(OpRetC);
      fe->maxStackCells = 1;
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
        // We are not supporting type constants for native classes
        // AFAIK emitHHBCNativeClassUnit is only used for legacy idl files
        pce->addConstant(
          cnsInfo->name.get(),
          nullptr,
          (TypedValue*)(&val),
          staticEmptyString(),
          /* typeconst = */ false);
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

  return ue;
}

static UnitEmitter* emitHHBCVisitor(AnalysisResultPtr ar, FileScopeRawPtr fsp) {
  auto md5 = fsp->getMd5();

  if (!Option::WholeProgram) {
    // The passed-in ar is only useful in whole-program mode, so create a
    // distinct ar to be used only for emission of this unit, and perform
    // unit-level (non-global) optimization.
    ar = std::make_shared<AnalysisResult>();
    fsp->setOuterScope(ar);

    ar->setPhase(AnalysisResult::AnalyzeAll);
    fsp->analyzeProgram(ar);
  }

  auto ue = emitHHBCUnitEmitter(ar, fsp, md5);
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
  void doJob(JobType job) override {
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
  void onThreadEnter() override {
    g_context.getCheck();
  }
  void onThreadExit() override {
    hphp_memory_cleanup();
  }
 private:
  bool m_ret;
};

static void addEmitterWorker(AnalysisResultPtr ar, StatementPtr sp,
                             void *data) {
  ((JobQueueDispatcher<EmitterWorker>*)data)->enqueue(sp->getFileScope());
}

static void
commitGlobalData(std::unique_ptr<ArrayTypeTable::Builder> arrTable) {
  auto gd                     = Repo::GlobalData{};
  gd.HardTypeHints            = Option::HardTypeHints;
  gd.HardReturnTypeHints      = Option::HardReturnTypeHints;
  gd.UsedHHBBC                = Option::UseHHBBC;
  gd.PHP7_IntSemantics        = RuntimeOption::PHP7_IntSemantics;
  gd.PHP7_ScalarTypes         = RuntimeOption::PHP7_ScalarTypes;
  gd.AutoprimeGenerators      = RuntimeOption::AutoprimeGenerators;
  gd.HardPrivatePropInference = true;

  if (arrTable) gd.arrayTypeTable.repopulate(*arrTable);
  Repo::get().saveGlobalData(gd);
}

/*
 * This is the entry point for offline bytecode generation.
 */
void emitAllHHBC(AnalysisResultPtr&& ar) {
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

  Compiler::ClearErrors();

  JobQueueDispatcher<EmitterWorker>
    dispatcher(threadCount, true, 0, false, ar.get());

  auto setPreloadPriority = [&ar](const std::string& f, int p) {
    auto fs = ar->findFileScope(f);
    if (fs) fs->setPreloadPriority(p);
  };

  /*
   * Mark files that are referenced from the autoload map
   * so they get preloaded via preloadRepo.
   * Higher priorities are preloaded first.
   * Classes, then functions, then constants mimics
   * the order of our existing warmup scripts
   */
  for (const auto& ent : Option::AutoloadConstMap) {
    setPreloadPriority(ent.second, 1);
  }
  for (const auto& ent : Option::AutoloadFuncMap) {
    setPreloadPriority(ent.second, 2);
  }
  for (const auto& ent : Option::AutoloadClassMap) {
    setPreloadPriority(ent.second, 3);
  }

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

    if (!Option::UseHHBBC) {
      commitGlobalData(std::unique_ptr<ArrayTypeTable::Builder>{});
    }
  } else {
    dispatcher.waitEmpty();
  }

  assert(Option::UseHHBBC || ues.empty());

  // We need to put the native func units in the repo so hhbbc can
  // find them.
  auto nfunc = emitHHBCNativeFuncUnit(hhbc_ext_funcs, hhbc_ext_funcs_count);
  auto ncls  = emitHHBCNativeClassUnit(hhbc_ext_classes,
                                       hhbc_ext_class_count);
  ues.push_back(std::move(nfunc));
  ues.push_back(std::move(ncls));

  ar->finish();
  ar.reset();

  if (!Option::UseHHBBC) {
    batchCommit(std::move(ues));
    return;
  }

  RuntimeOption::EvalJit = false; // For HHBBC to invoke builtins.
  auto pair = HHBBC::whole_program(std::move(ues));
  batchCommit(std::move(pair.first));
  commitGlobalData(std::move(pair.second));
}

namespace {

bool startsWith(const char* big, const char* small) {
  return strncmp(big, small, strlen(small)) == 0;
}
bool isFileHack(const char* code, int codeLen, bool allowPartial) {
  if (allowPartial) {
    return codeLen > strlen("<?hh") && startsWith(code, "<?hh");
  } else {
    return codeLen > strlen("<?hh // strict") &&
      (startsWith(code, "<?hh // strict") || startsWith(code, "<?hh //strict"));
  }
}

UnitEmitter* makeFatalUnit(const char* filename, const MD5& md5,
                           const std::string& msg) {
  // basically duplicated from as.cpp but is maybe too janky to be
  // a common routine somewhere...

  // The line numbers we output are bogus, but it's not totally clear
  // what line numbers to put. It might be worth adding a mechanism for
  // the external emitter to emit a line number when it fails that we can
  // use when available.
  UnitEmitter* ue = new UnitEmitter(md5);
  ue->m_filepath = makeStaticString(filename);
  ue->initMain(1, 1);
  ue->emitOp(OpString);
  ue->emitInt32(ue->mergeLitstr(makeStaticString(msg)));
  ue->emitOp(OpFatal);
  ue->emitByte(static_cast<uint8_t>(FatalOp::Runtime));
  FuncEmitter* fe = ue->getMain();
  fe->maxStackCells = 1;
  fe->finish(ue->bcPos(), false);
  ue->recordFunction(fe);

  return ue;
}

UnitEmitter* useExternalEmitter(const char* code, int codeLen,
                                const char* filename, const MD5& md5) {
#ifdef _MSC_VER
  Logger::Error("The external emitter is not supported under MSVC!");
  return nullptr;
#else
  std::string hhas, errorOutput;

  try {
    std::vector<std::string> cmd({
        RuntimeOption::EvalUseExternalEmitter, "--stdin", filename});
    auto options = folly::Subprocess::pipeStdin().pipeStdout().pipeStderr();

    // Run the external emitter, sending the code to its stdin
    folly::Subprocess proc(cmd, options);
    std::tie(hhas, errorOutput) = proc.communicate(std::string(code, codeLen));
    proc.waitChecked();

    // External emitter succeeded; assemble its output
    // If assembly fails (probably because of malformed emitter
    // output), the assembler will return a unit that Fatals and we
    // won't do a fallback to the regular emitter. We may want to
    // revisit this.
    return assemble_string(hhas.data(), hhas.length(), filename, md5);

  } catch (const std::exception& e) {
    std::string errorMsg = e.what();
    if (!errorOutput.empty()) {
      // Add the stderr to the output
      errorMsg += folly::format(". Output: '{}'",
                                folly::trimWhitespace(errorOutput)).str();
    }

    // If we aren't going to fall back to the internal emitter, generate a
    // Fatal'ing unit.
    if (!RuntimeOption::EvalExternalEmitterFallback) {
      auto msg =
        folly::format("Failure running external emitter: {}", errorMsg);
      return makeFatalUnit(filename, md5, msg.str());
    }

    // Unless we have fallback at the highest level, print a
    // diagnostic when we fail
    if (RuntimeOption::EvalExternalEmitterFallback < 2) {
      Logger::Warning("Failure running external emitter for %s: %s",
                      filename,
                      errorMsg.c_str());
    }
    return nullptr;
  }
#endif
}

}

extern "C" {

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
    Option::HardReturnTypeHints =
      (RuntimeOption::EvalCheckReturnTypeHints >= 3);
    Option::EnableZendCompat = RuntimeOption::EnableZendCompat;
    Option::JitEnableRenameFunction = RuntimeOption::EvalJitEnableRenameFunction;
    for (auto& i : RuntimeOption::DynamicInvokeFunctions) {
      Option::DynamicInvokeFunctions.insert(i);
    }
    Option::IntsOverflowToInts = RuntimeOption::IntsOverflowToInts;
    Option::RecordErrors = false;
    Option::ParseTimeOpts = false;
    Option::WholeProgram = false;
    BuiltinSymbols::LoadSuperGlobals();
    TypeConstraint tc;
    return nullptr;
  }

  SCOPE_ASSERT_DETAIL("hphp_compiler_parse") { return filename; };

  try {
    UnitOrigin unitOrigin = UnitOrigin::File;
    if (!filename) {
      filename = "";
      unitOrigin = UnitOrigin::Eval;
    }

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

    // If we are configured to use an external emitter and we are compiling
    // a strict mode hack file, try external emitting. Don't externally emit
    // systemlib because the external emitter can't handle that yet.
    if (!ue &&
        !RuntimeOption::EvalUseExternalEmitter.empty() &&
        isFileHack(code, codeLen,
                   RuntimeOption::EvalExternalEmitterAllowPartial) &&
        SystemLib::s_inited) {
      ue.reset(useExternalEmitter(code, codeLen, filename, md5));
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

      auto ar = std::make_shared<AnalysisResult>();
      FileScopePtr fsp = parseit(ar);
      fsp->setOuterScope(ar);

      ar->setPhase(AnalysisResult::AnalyzeAll);
      fsp->analyzeProgram(ar);

      ue.reset(emitHHBCUnitEmitter(ar, fsp, md5));
    }
    Repo::get().commitUnit(ue.get(), unitOrigin);

    auto unit = ue->create();
    ue.reset();

    if (unit->sn() == -1) {
      // the unit was not committed to the Repo, probably because
      // another thread did it first. Try to use the winner.
      auto u = Repo::get().loadUnit(filename ? filename : "", md5);
      if (u != nullptr) {
        return u.release();
      }
    }
    return unit.release();
  } catch (const std::exception&) {
    // extern "C" function should not be throwing exceptions...
    return nullptr;
  }
}

Unit* hphp_build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                                  ssize_t numBuiltinFuncs) {
  return emitHHBCNativeFuncUnit(builtinFuncs, numBuiltinFuncs)->create()
    .release();
}

Unit* hphp_build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                                   ssize_t numBuiltinClasses) {
  auto const ue = emitHHBCNativeClassUnit(builtinClasses, numBuiltinClasses);

  /*
   * In RepoAuthoritative mode, we may have serialized additional
   * information in attr bits about the native units.  We still
   * rebuild it, but we'll clobber attr bits with what static analysis
   * thought.
   *
   * Note: this is a bit of a short-term hack that we won't need once
   * HNI conversion is completed.  We only pull things out of the repo
   * here that we've explicitly decided we want.
   */
  if (!RuntimeOption::RepoAuthoritative) return ue->create().release();
  auto const staticAnalysisUnit = Repo::get().urp().loadEmitter(
    "/:systemlib:static_analysis",
    s_nativeClassMD5
  );
  if (!staticAnalysisUnit) return ue->create().release();

  // Make a map of the preclasses in `ue', so we can find them.
  std::map<const StringData*,PreClassEmitter*> uePreClasses;
  for (auto id = Id{0}; id < ue->numPreClasses(); ++id) {
    auto const pce = ue->pce(id);
    always_assert_flog(
      !uePreClasses.count(pce->name()),
      "IDL-based native class unit is expected to only have unique "
      "classes.  {} was non-unique.",
      pce->name()->data()
    );
    uePreClasses[pce->name()] = pce;
  }

  always_assert_flog(
    staticAnalysisUnit->numPreClasses() == uePreClasses.size(),
    "Static analysis unit didn't have the same number of classes as "
    "our native class unit; repo probably build with a different hhvm build."
  );

  // Right now, the only thing we do here is clobber all the Attr bits
  // with the ones we found from static analysis.  (To get things like
  // AttrNoOverride.)
  for (auto id = Id{0}; id < staticAnalysisUnit->numPreClasses(); ++id) {
    auto const staticAnalysisPce = staticAnalysisUnit->pce(id);
    auto const uePce = uePreClasses[staticAnalysisPce->name()];
    always_assert_flog(
      uePce != nullptr,
      "Static analysis unit had a PreClass we don't have at runtime ({}); "
      "repo probably was built with a different hhvm build.",
      staticAnalysisPce->name()->data()
    );
    uePce->setAttrs(staticAnalysisPce->attrs());

    // Set all the method bits.
    for (auto methID = uint32_t{0};
         methID < staticAnalysisPce->methods().size();
         ++methID) {
      auto const& staticAnalysisMethod = staticAnalysisPce->methods()[methID];
      auto const pceMethod = uePce->findMethod(staticAnalysisMethod->name);
      always_assert_flog(
        pceMethod != nullptr,
        "Static analysis unit had a PreClass method that we don't have at "
        "runtime ({}::{}); repo probably was built with a different hhvm "
        "build.",
        staticAnalysisPce->name()->data(),
        staticAnalysisMethod->name->data()
      );
      pceMethod->attrs = staticAnalysisMethod->attrs;
    }
  }

  return ue->create().release();
}

} // extern "C"

///////////////////////////////////////////////////////////////////////////////
}
}
