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
#include <util/parser/hphp.tab.hpp>
#include <runtime/base/tv_macros.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/peephole.h>
#include <runtime/base/runtime_option.h>
#include <runtime/eval/runtime/file_repository.h>

#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/emitter.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>

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
#include <compiler/expression/static_member_expression.h>
#include <compiler/expression/unary_op_expression.h>

#include <compiler/statement/break_statement.h>
#include <compiler/statement/case_statement.h>
#include <compiler/statement/catch_statement.h>
#include <compiler/statement/class_constant.h>
#include <compiler/statement/class_variable.h>
#include <compiler/statement/do_statement.h>
#include <compiler/statement/echo_statement.h>
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

  static const char C = 0x01; // Cell protoflavor
  static const char V = 0x02; // Var protoflavor
  static const char H = 0x03; // Home protoflavor
  static const char A = 0x04; // Classref protoflavor
  static const char R = 0x05; // Return value protoflavor
  static const char F = 0x06; // Function argument protoflavor

  static const char N = 0x10; // Name marker
  static const char G = 0x20; // Global name marker
  static const char E = 0x30; // Element marker
  static const char W = 0x40; // New element marker
  static const char P = 0x50; // Property marker
  static const char S = 0x60; // Static property marker

  static const char CN = C | N;
  static const char CG = C | G;
  static const char CE = C | E;
  static const char CP = C | P;
  static const char CS = C | S;
  static const char HN = H | N;
  static const char HG = H | G;
  static const char HE = H | E;
  static const char HP = H | P;
  static const char HS = H | S;

  char GetProtoflavor(char sym) { return (sym & 0x0F); }
  char GetMarker(char sym) { return (sym & 0xF0); }
  std::string ToString(char sym) {
    char protoflavor = StackSym::GetProtoflavor(sym);
    std::string res;
    switch (protoflavor) {
      case StackSym::C: res = "C"; break;
      case StackSym::V: res = "V"; break;
      case StackSym::H: res = "H"; break;
      case StackSym::A: res = "A"; break;
      case StackSym::R: res = "R"; break;
      case StackSym::F: res = "F"; break;
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

// RAII guard for function creation.
class FuncFinisher {
  EmitterVisitor* m_ev;
  Emitter&        m_e;
  Func*           m_f;
  Offset          m_off;

 public:
  FuncFinisher(EmitterVisitor* ev, Emitter& e, Func* f,
               Offset off = InvalidAbsoluteOffset)
    : m_ev(ev), m_e(e), m_f(f), m_off(off) {
      TRACE(1, "FuncFinisher constructed: %s %p\n", m_f->m_name->data(), m_f);
    }

  ~FuncFinisher() {
    TRACE(1, "Finishing func: %s %p\n", m_f->m_name->data(), m_f);
    m_ev->finishFunc(m_e, m_f, m_off);
  }
};

#define O(name, imm, pop, push, flags) \
  void Emitter::name(imm) { \
    Offset curPos __attribute__((unused)) = getUnit().bcPos(); \
    getEmitterVisitor().prepareEvalStack(); \
    POP_##pop; \
    PUSH_##push; \
    getUnit().emitOp(Op##name); \
    IMPL_##imm; \
    getUnit().recordSourceLocation(m_node->getLocation().get(), curPos, \
                                   getUnit().bcPos()); \
    if (flags & UF) getEmitterVisitor().restoreJumpTargetEvalStack(); \
    getEmitterVisitor().setPrevOpcode(Op##name); \
  }

#define ONE(t) \
  DEC_##t a1
#define TWO(t1, t2) \
  DEC_##t1 a1, DEC_##t2 a2
#define THREE(t1, t2, t3) \
  DEC_##t1 a1, DEC_##t2 a2, DEC_##t3 a3
#define NA
#define DEC_LA std::vector<uchar>
#define DEC_IVA int32
#define DEC_I64A int64
#define DEC_DA double
#define DEC_SA StringData*
#define DEC_AA ArrayData*
#define DEC_BA Label&
#define DEC_OA uchar

#define POP_NOV
#define POP_ONE(t) \
  POP_##t
#define POP_TWO(t1, t2) \
  POP_##t1; \
  POP_##t2
#define POP_THREE(t1, t2, t3) \
  POP_##t1; \
  POP_##t2; \
  POP_##t3
#define POP_POS_1(t) \
  POP_POS_1_##t
#define POP_POS_N(t) \
  POP_POS_N_##t
#define POP_LMANY() \
  getEmitterVisitor().popEvalStackLMany()
#define POP_C_LMANY() \
  getEmitterVisitor().popEvalStack(StackSym::C); \
  getEmitterVisitor().popEvalStackLMany()
#define POP_V_LMANY() \
  getEmitterVisitor().popEvalStack(StackSym::V); \
  getEmitterVisitor().popEvalStackLMany()
#define POP_FMANY \
  getEmitterVisitor().popEvalStackFMany(a1) \

#define POP_CV getEmitterVisitor().popEvalStack(StackSym::C)
#define POP_VV getEmitterVisitor().popEvalStack(StackSym::V)
#define POP_HV getEmitterVisitor().popEvalStack(StackSym::H)
#define POP_AV getEmitterVisitor().popEvalStack(StackSym::A)
#define POP_RV getEmitterVisitor().popEvalStack(StackSym::R)
#define POP_FV getEmitterVisitor().popEvalStack(StackSym::F)

#define POP_POS_1_CV getEmitterVisitor().peekEvalStack(StackSym::C, 1)
#define POP_POS_1_VV getEmitterVisitor().peekEvalStack(StackSym::V, 1)
#define POP_POS_1_HV getEmitterVisitor().peekEvalStack(StackSym::H, 1)
#define POP_POS_1_AV getEmitterVisitor().peekEvalStack(StackSym::A, 1)
#define POP_POS_1_RV getEmitterVisitor().peekEvalStack(StackSym::R, 1)
#define POP_POS_1_FV getEmitterVisitor().peekEvalStack(StackSym::F, 1)

#define POP_POS_N_CV getEmitterVisitor().peekEvalStack(StackSym::C, a1)
#define POP_POS_N_VV getEmitterVisitor().peekEvalStack(StackSym::V, a1)
#define POP_POS_N_HV getEmitterVisitor().peekEvalStack(StackSym::H, a1)
#define POP_POS_N_AV getEmitterVisitor().peekEvalStack(StackSym::A, a1)
#define POP_POS_N_RV getEmitterVisitor().peekEvalStack(StackSym::R, a1)
#define POP_POS_N_FV getEmitterVisitor().peekEvalStack(StackSym::F, a1)

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
#define PUSH_POS_1(t) \
  PUSH_POS_1_##t
#define PUSH_POS_N(t) \
  PUSH_POS_N_##t

#define PUSH_CV getEmitterVisitor().pushEvalStack(StackSym::C)
#define PUSH_VV getEmitterVisitor().pushEvalStack(StackSym::V)
#define PUSH_HV getEmitterVisitor().pushEvalStack(StackSym::H)
#define PUSH_AV getEmitterVisitor().pushEvalStack(StackSym::A)
#define PUSH_RV getEmitterVisitor().pushEvalStack(StackSym::R)
#define PUSH_FV getEmitterVisitor().pushEvalStack(StackSym::F)

#define PUSH_POS_1_CV getEmitterVisitor().pokeEvalStack(StackSym::C, 1)
#define PUSH_POS_1_VV getEmitterVisitor().pokeEvalStack(StackSym::V, 1)
#define PUSH_POS_1_HV getEmitterVisitor().pokeEvalStack(StackSym::H, 1)
#define PUSH_POS_1_AV getEmitterVisitor().pokeEvalStack(StackSym::A, 1)
#define PUSH_POS_1_RV getEmitterVisitor().pokeEvalStack(StackSym::R, 1)
#define PUSH_POS_1_FV getEmitterVisitor().pokeEvalStack(StackSym::F, 1)

#define PUSH_POS_N_CV getEmitterVisitor().pokeEvalStack(StackSym::C, a1)
#define PUSH_POS_N_VV getEmitterVisitor().pokeEvalStack(StackSym::V, a1)
#define PUSH_POS_N_HV getEmitterVisitor().pokeEvalStack(StackSym::H, a1)
#define PUSH_POS_N_AV getEmitterVisitor().pokeEvalStack(StackSym::A, a1)
#define PUSH_POS_N_RV getEmitterVisitor().pokeEvalStack(StackSym::R, a1)
#define PUSH_POS_N_FV getEmitterVisitor().pokeEvalStack(StackSym::F, a1)

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

#define IMPL_LA(var) do { \
  getUnit().emitInt32(var.size()); \
  for (unsigned int i = 0; i < var.size(); ++i) { \
    getUnit().emitByte(var[i]); \
  } \
  } while (0)
#define IMPL1_LA IMPL_LA(a1)
#define IMPL2_LA IMPL_LA(a2)
#define IMPL3_LA IMPL_LA(a3)

#define IMPL_IVA(var) do { \
  if (LIKELY((var & 0x7F) == var)) {                     \
    getUnit().emitByte(((unsigned char)var) << 1);       \
  } else {                                               \
    getUnit().emitInt32((var << 1) | 0x1);               \
  }                                                      \
} while (0)
#define IMPL1_IVA IMPL_IVA(a1)
#define IMPL2_IVA IMPL_IVA(a2)
#define IMPL3_IVA IMPL_IVA(a3)

#define IMPL_I64A(var) getUnit().emitInt64(var)
#define IMPL1_I64A IMPL_I64A(a1)
#define IMPL2_I64A IMPL_I64A(a2)
#define IMPL3_I64A IMPL_I64A(a3)

#define IMPL_SA(var) \
  getUnit().emitInt32(getUnit().mergeLitstr(var))
#define IMPL1_SA IMPL_SA(a1)
#define IMPL2_SA IMPL_SA(a2)
#define IMPL3_SA IMPL_SA(a3)

#define IMPL_AA(var) \
  getUnit().emitInt32(getUnit().mergeArray(var))
#define IMPL1_AA IMPL_AA(a1)
#define IMPL2_AA IMPL_AA(a2)
#define IMPL3_AA IMPL_AA(a3)

#define IMPL_DA(var) getUnit().emitDouble(var)
#define IMPL1_DA IMPL_DA(a1)
#define IMPL2_DA IMPL_DA(a2)
#define IMPL3_DA IMPL_DA(a3)

#define IMPL_BA(var) \
  if (var.getAbsoluteOffset() == InvalidAbsoluteOffset) { \
    /* For forward jumps, we store information about the */ \
    /* current instruction in the Label. When the Label is */ \
    /* set, it will fix up any instructions that reference */ \
    /* it, and then it will call recordJumpTarget */ \
    var.bind(getEmitterVisitor(), curPos, getUnit().bcPos()); \
  } else { \
    /* For backward jumps, we simply call recordJumpTarget */ \
    getEmitterVisitor().recordJumpTarget(var.getAbsoluteOffset()); \
  } \
  getUnit().emitInt32(var.getAbsoluteOffset() - curPos);
#define IMPL1_BA IMPL_BA(a1)
#define IMPL2_BA IMPL_BA(a2)
#define IMPL3_BA IMPL_BA(a3)

#define IMPL_OA(var) getUnit().emitByte(var)
#define IMPL1_OA IMPL_OA(a1)
#define IMPL2_OA IMPL_OA(a2)
#define IMPL3_OA IMPL_OA(a3)
 OPCODES
#undef O
#undef ONE
#undef TWO
#undef THREE
#undef NA
#undef DEC_LA
#undef DEC_IVA
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
#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_THREE
#undef PUSH_CV
#undef PUSH_VV
#undef PUSH_HV
#undef PUSH_AV
#undef PUSH_RV
#undef PUSH_FV
#undef IMPL_ONE
#undef IMPL_TWO
#undef IMPL_THREE
#undef IMPL_NA
#undef IMPL_LA
#undef IMPL1_LA
#undef IMPL2_LA
#undef IMPL3_LA
#undef IMPL_IVA
#undef IMPL1_IVA
#undef IMPL2_IVA
#undef IMPL3_IVA
#undef IMPL_I64A
#undef IMPL1_I64A
#undef IMPL2_I64A
#undef IMPL3_I64A
#undef IMPL_DA
#undef IMPL1_DA
#undef IMPL2_DA
#undef IMPL3_DA
#undef IMPL_SA
#undef IMPL1_SA
#undef IMPL2_SA
#undef IMPL3_SA
#undef IMPL_AA
#undef IMPL1_AA
#undef IMPL2_AA
#undef IMPL3_AA
#undef IMPL_BA
#undef IMPL1_BA
#undef IMPL2_BA
#undef IMPL3_BA
#undef IMPL_OA
#undef IMPL1_OA
#undef IMPL2_OA
#undef IMPL3_OA

static void assertEvalStacksMatch(const SymbolicStack& evalStack1,
                                  const SymbolicStack& evalStack2) {
  if (evalStack1.size() != evalStack2.size()) {
    Logger::Warning("Emitter detected a point in the bytecode where the "
                    "depth of the stack is not the same for all possible "
                    "control flow paths");
    return;
  }
  for (unsigned int i = 0; i < evalStack1.size(); ++i) {
    if (evalStack1.get(i) != evalStack2.get(i)) {
      Logger::Warning("Emitter detected a point in the bytecode where the "
                      "protoflavor of a value on the stack is not the same "
                      "for all possible control flow paths");
      return;
    }
  }
}

void SymbolicStack::push(char sym) {
  if (sym != StackSym::W) {
    m_actualStack.push_back(m_symStack.size());
    *m_actualStackHighWaterPtr = MAX(*m_actualStackHighWaterPtr,
                                     (int)m_actualStack.size());
  }
  m_symStack.push_back(sym);
}

void SymbolicStack::pop() {
  ASSERT(!m_symStack.empty() && !m_actualStack.empty());
  char sym = m_symStack.back();
  if (sym != StackSym::W) {
    m_actualStack.pop_back();
  }
  m_symStack.pop_back();
}

char SymbolicStack::top() const {
  ASSERT(!m_symStack.empty());
  return m_symStack.back();
}

char SymbolicStack::get(int index) const {
  ASSERT(index >= 0 && index < (int)m_symStack.size());
  return m_symStack[index];
}

void SymbolicStack::set(int index, char sym) {
  ASSERT(index >= 0 && index < (int)m_symStack.size());
  // XXX Add ASSERT in debug build to make sure W is not getting
  // written or overwritten by something else
  m_symStack[index] = sym;
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

char SymbolicStack::getActual(int index) const {
  ASSERT(index >= 0 && index < (int)m_actualStack.size());
  return get(m_actualStack[index]);
}

void SymbolicStack::setActual(int index, char sym) {
  ASSERT(index >= 0 && index < (int)m_actualStack.size());
  set(m_actualStack[index], sym);
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

void Label::set(Emitter &e) {
  if (isSet()) {
    Logger::Warning("Label::set was called more than once on the same Label; "
                    "originally set to %d; now %d",
                    m_off,
                    e.getUnit().bcPos());
    return;
  }
  m_off = e.getUnit().bcPos();
  // Fix up any forward jumps that reference to this Label
  for (std::vector<std::pair<Offset, Offset> >::const_iterator it =
      m_emittedOffs.begin(); it != m_emittedOffs.end(); ++it) {
    e.getUnit().emitInt32(m_off - it->first, it->second);
  }
  EmitterVisitor & ev = e.getEmitterVisitor();
  if (!m_emittedOffs.empty()) {
    // If there were forward jumps that referenced this Label,
    // compare the the eval stack from the first foward jump we
    // saw with the current eval stack
    if (!ev.evalStackIsUnknown()) {
      assertEvalStacksMatch(ev.getEvalStack(), m_evalStack);
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

void Label::bind(EmitterVisitor & ev, Offset instrAddr, Offset offAddr) {
  if (m_off != InvalidAbsoluteOffset) {
    Logger::Warning("Label::bind was called on a Label that has already "
                    "been set to %d",
                    m_off);
    return;
  }
  bool labelHasEvalStack = !m_emittedOffs.empty();
  m_emittedOffs.push_back(std::pair<Offset, Offset>(instrAddr, offAddr));
  if (labelHasEvalStack) {
    assertEvalStacksMatch(ev.getEvalStack(), m_evalStack);
  } else {
    m_evalStack = ev.getEvalStack();
  }
}

struct FPIRegionRecorder {
  FPIRegionRecorder(EmitterVisitor* ev, Unit& unit, SymbolicStack& stack)
      : m_ev(ev), m_unit(unit), m_stack(stack),
        m_fpOff(m_stack.sizeActual()), m_start(m_unit.bcPos()) {
    m_stack.pushFDesc();
  }
  ~FPIRegionRecorder() {
    m_stack.popFDesc();
    m_ev->newFPIRegion(m_start, m_unit.bcPos(), m_fpOff);
  }
private:
  EmitterVisitor* m_ev;
  Unit& m_unit;
  SymbolicStack& m_stack;
  int m_fpOff;
  Offset m_start;
};

//=============================================================================
// EmitterVisitor.

EmitterVisitor::EmitterVisitor(Unit &u)
  : m_unit(u), m_curFunc(u.getMain()), m_evalStackIsUnknown(false),
    m_actualStackHighWater(0), m_fdescHighWater(0), m_closureCounter(0) {
  m_prevOpcode = OpLowInvalid;
  m_evalStack.m_actualStackHighWaterPtr = &m_actualStackHighWater;
  m_evalStack.m_fdescHighWaterPtr = &m_fdescHighWater;
}

EmitterVisitor::~EmitterVisitor() {
  for (std::map<StringData*, Label*,
                string_data_lt>::const_iterator it = m_methLabels.begin();
       it != m_methLabels.end(); it++) {
    delete it->second;
  }
  // If a fatal occurs during emission, some extra cleanup is necessary.
  for (std::deque<ExnHandlerRegion*>::const_iterator it = m_exnHandlers.begin();
       it != m_exnHandlers.end(); ++it) {
    delete *it;
  }
}

void EmitterVisitor::popEvalStack(char expected) {
  // Pop a value off of the evaluation stack, and verify that it
  // matches the specified protoflavor
  if (m_evalStack.size() == 0) {
    Logger::Warning("Emitter emitted an instruction that tries to consume "
                    "a value from the stack when the stack is empty "
                    "(expected protoflavor \"%s\" at offset %d)",
                    StackSym::ToString(expected).c_str(),
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  char actual = StackSym::GetProtoflavor(sym);
  m_evalStack.pop();
  if (actual != expected) {
    Logger::Warning("Emitter emitted an instruction that tries to consume a "
                    "value from the stack when the top of the stack does not "
                    "match the protoflavor that the instruction expects "
                    "(expected protoflavor \"%s\", actual protoflavor \"%s\" "
                    "at offset %d)",
                    StackSym::ToString(expected).c_str(),
                    StackSym::ToString(actual).c_str(),
                    m_unit.bcPos());
  }
}

void EmitterVisitor::popEvalStackLMany() {
  while (!m_evalStack.empty()) {
    char sym = m_evalStack.top();
    char protoflavor = StackSym::GetProtoflavor(sym);
    char marker = StackSym::GetMarker(sym);
    if (marker == StackSym::E || marker == StackSym::P) {
      if (protoflavor != StackSym::C && protoflavor != StackSym::H) {
        Logger::Warning("Emitter emitted an instruction that tries to consume "
                        "a value from the stack when the top of the stack "
                        "does not match the protoflavor that the instruction "
                        "expects (expected protoflavor \"C\" or \"H\", actual "
                        "protoflavor \"%s\" at offset %d)",
                        StackSym::ToString(protoflavor).c_str(),
                        m_unit.bcPos());
      }
    } else if (marker == StackSym::W) {
      if (protoflavor != StackSym::None) {
        Logger::Warning("Emitter emitted an instruction that tries to consume "
                        "a value from the stack when the top of the stack "
                        "does not match the protoflavor that the instruction "
                        "expects (expected protoflavor \"None\", actual "
                        "protoflavor \"%s\" at offset %d)",
                        StackSym::ToString(protoflavor).c_str(),
                        m_unit.bcPos());
      }
    } else {
      break;
    }
    m_evalStack.pop();
  }
  // Pop a value off of the evaluation stack, and verify that it
  // matches the specified protoflavor
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter emitted an instruction that tries to consume "
                    "a value from the stack when the stack is empty "
                    "(at offset %d)",
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  char protoflavor = StackSym::GetProtoflavor(sym);
  char marker = StackSym::GetMarker(sym);
  m_evalStack.pop();
  if (protoflavor != StackSym::C && protoflavor != StackSym::H &&
      protoflavor != StackSym::R) {
    Logger::Warning("Emitter emitted an instruction that tries to consume a "
                    "value from the stack when the top of the stack does not "
                    "match the protoflavor that the instruction expects "
                    "(expected protoflavor \"C\", \"H\", or \"R\", actual "
                    "protoflavor \"%s\" at offset %d)",
                    StackSym::ToString(protoflavor).c_str(),
                    m_unit.bcPos());
  }
  if (marker == StackSym::S) {
    // For static property locations, we also need to pop off the classref
    popEvalStack(StackSym::A);
  }
}

void EmitterVisitor::popEvalStackFMany(int len) {
  for (int i = 0; i < len; ++i) {
    popEvalStack(StackSym::F);
  }
}

void EmitterVisitor::pushEvalStack(char protoflavor) {
  // Push a value from the evaluation stack with the specified
  // specified protoflavor
  m_evalStack.push(protoflavor);
}

void EmitterVisitor::peekEvalStack(char expected, int depthActual) {
  int posActual = (m_evalStack.sizeActual() - depthActual - 1);
  if (posActual >= 0 && posActual < (int)m_evalStack.sizeActual()) {
    // Push a value from the evaluation stack with the specified
    // specified protoflavor
    char sym = m_evalStack.getActual(posActual);
    char actual = StackSym::GetProtoflavor(sym);
    if (actual != expected) {
      Logger::Warning("Emitter emitted an instruction that tries to consume a "
                      "value from the stack whose protoflavor does not match "
                      "the protoflavor that the instruction expects (expected "
                      "protoflavor \"%s\", actual protoflavor \"%s\" at "
                      "offset %d)",
                      StackSym::ToString(expected).c_str(),
                      StackSym::ToString(actual).c_str(),
                      m_unit.bcPos());
    }
  }
}

void EmitterVisitor::pokeEvalStack(char protoflavor, int depthActual) {
  int sizeActual = m_evalStack.sizeActual();
  int posActual = sizeActual - depthActual - 1;
  if (posActual >= 0 && posActual < sizeActual) {
    // Push a value from the evaluation stack with the specified
    // specified protoflavor
    m_evalStack.setActual(posActual, protoflavor);
  }
}

void EmitterVisitor::prepareEvalStack() {
  if (m_evalStackIsUnknown) {
    ASSERT(m_evalStack.empty());
    // Record that we are assuming that the eval stack is empty
    recordJumpTarget(m_unit.bcPos(), m_evalStack);
    m_evalStackIsUnknown = false;
  }
}

void EmitterVisitor::recordJumpTarget(Offset target,
                                      const SymbolicStack& evalStack) {
  if (target == InvalidAbsoluteOffset) {
    Logger::Warning("Offset passed to EmitterVisitor::recordJumpTarget was "
                    "invalid");
  }
  hphp_hash_map<Offset, SymbolicStack>::iterator it =
    m_jumpTargetEvalStacks.find(target);
  if (it == m_jumpTargetEvalStacks.end()) {
    m_jumpTargetEvalStacks[target] = evalStack;
    return;
  }
  assertEvalStacksMatch(evalStack, it->second);
}

void EmitterVisitor::restoreJumpTargetEvalStack() {
  m_evalStack.clear();
  hphp_hash_map<Offset, SymbolicStack>::iterator it =
    m_jumpTargetEvalStacks.find(m_unit.bcPos());
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

StringData* EmitterVisitor::mangleStaticName(const std::string& varName) {
  std::ostringstream str;
  str << varName << "$";
  str << std::hex << m_curFunc;

  StringData* mangled = NEW(StringData)(str.str().c_str(), str.str().length(),
                                        CopyString);
  return StringData::GetStaticString(mangled);
}

#define CONTROL_BODY(brk, cnt, brkH, cntH) \
  ControlTargetPusher _cop(this, -1, brk, cnt, brkH, cntH)
#define FOREACH_BODY(itId, brk, cnt, brkH, cntH) \
  ControlTargetPusher _cop(this, itId, brk, cnt, brkH, cntH)

class IterFreeThunklet : public Thunklet {
public:
  IterFreeThunklet(Id iterId) : m_id(iterId) {}
  virtual void emit(Emitter &e) {
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
    e.Loc(m_loc);
    e.UnsetH();
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
        Emitter lhsEmitter(lhs, m_unit, *this);
        visitIfCondition(lhs, lhsEmitter, tru, localLabel, false);
        // falls through if that condition was false
      } else {
        Emitter lhsEmitter(lhs, m_unit, *this);
        visitIfCondition(lhs, lhsEmitter, localLabel, fals, true);
        // falls through if that condition was true
      }
      if (localLabel.isUsed()) {
        localLabel.set(e);
      }
      ExpressionPtr rhs = binOpNode->getExp2();
      Emitter rhsEmitter(rhs, m_unit, *this);
      visitIfCondition(rhs, rhsEmitter, tru, fals, truFallthrough);
      return;
    }
  }

  UnaryOpExpressionPtr unOpNode(dynamic_pointer_cast<UnaryOpExpression>(cond));
  if (unOpNode) {
    int op = unOpNode->getOp();
    // Logical not
    if (op == '!') {
      ExpressionPtr val = unOpNode->getExpression();
      Emitter valEmitter(val, m_unit, *this);
      visitIfCondition(val, valEmitter, fals, tru, !truFallthrough);
      return;
    }
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
    StringData* nLiteral = m_unit.lookupLitstrStr(localNames[i].c_str());
    m_curFunc->allocVarId(nLiteral);
  }
}

void EmitterVisitor::visit(FileScopePtr file) {
  const std::string& filename = file->getName();
  m_unit.m_filepath = m_unit.lookupLitstrStr(filename);
  const std::string& dirname = Util::safe_dirname(filename);
  m_unit.m_dirpath = m_unit.lookupLitstrStr(dirname);

  FunctionScopePtr func(file->getPseudoMain());
  if (!func) return;

  // Assign ids to all of the local variables eagerly. This gives us the
  // nice property that all named local variables will be assigned ids
  // 0 through k-1, while any unnamed local variable will have an id >= k.
  assignLocalVariableIds(func);

  MethodStatementPtr m(dynamic_pointer_cast<MethodStatement>(func->getStmt()));
  if (!m) return;
  StatementListPtr stmts(m->getStmts());
  if (!stmts) return;

  Emitter e(m, m_unit, *this);

  Label ctFatal;
  Label ctFatalWithPop;
  int i, nk = stmts->getCount();
  CONTROL_BODY(ctFatal, ctFatal, ctFatalWithPop, ctFatalWithPop);
  for (i = 0; i < nk; i++) {
    StatementPtr s = (*stmts)[i];
    if (MethodStatementPtr meth = dynamic_pointer_cast<MethodStatement>(s)) {
      // Create label for use with fast calls
      StringData* methName = m_unit.lookupLitstrStr(meth->getOriginalName());
      m_methLabels[methName] = new Label();
      // Emit afterwards
      Func* f = m_unit.newFunc(methName, true);
      f->m_isGenerator = meth->getFunctionScope()->isGenerator();
      postponeMeth(meth, f, true);
    }
  }
  {
    if (m->getFunctionScope()->containsBareThis()) {
      StringData* thisStr = m_unit.lookupLitstrStr("this");
      Id thisId = m_curFunc->lookupVarId(thisStr);
      e.InitThisLoc(thisId);
    }
    FuncFinisher ff(this, e, m_curFunc);
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
          break;
        }
        default: visit(s);
      }
    }
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
      StringData* msg = StringData::GetStaticString("Cannot break/continue");
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

  // Method bodies
  emitPostponedMeths();
  emitPostponedCtors();
  emitPostponedPinits();
  emitPostponedSinits();
  emitPostponedCinits();
  emitPostponedClosureCtors();
}

void EmitterVisitor::visitKids(ConstructPtr c) {
  for (int i = 0, nk = c->getKidCount(); i < nk; i++) {
    ConstructPtr kid(c->getNthKid(i));
    visit(kid);
  }
}

bool EmitterVisitor::visit(ConstructPtr node) {
  if (!node) return false;

  Emitter e(node, m_unit, *this);

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
            StringData* msg =
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
        visit(node->getNthKid(DoStatement::CondExpr));
        emitConvertToCell(e);
        e.JmpNZ(top);
        if (brkHand.isUsed() || cntHand.isUsed()) {
          e.Jmp(exit);
          emitBreakHandler(e, exit, condition, brkHand, cntHand);
        }
        exit.set(e);
        return false;
      }

      case Statement::KindOfCaseStatement: {
        // Should never be called. Handled in visitSwitch.
        ASSERT(false);
        return false;
      }

      case Statement::KindOfCatchStatement: {
        // Store the current exception object in the appropriate local variable
        CatchStatementPtr cs(static_pointer_cast<CatchStatement>(node));
        StringData* vName =
          m_unit.lookupLitstrStr(cs->getVariable()->getName());
        Id i = m_curFunc->lookupVarId(vName);
        e.Loc(i);
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
        visitKids(node);
        emitPop(e);
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
        if (visit(node->getNthKid(ForStatement::CondExpr))) {
          emitConvertToCell(e);
          e.JmpZ(fail);
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
        fail.set(e);
        return false;
      }

      case Statement::KindOfForEachStatement: {
        ForEachStatementPtr fe(static_pointer_cast<ForEachStatement>(node));
        visit(node->getNthKid(ForEachStatement::ArrayExpr));
        if (fe->isStrong()) {
          emitConvertToVar(e);
        } else {
          emitConvertToCell(e);
        }
        ExpressionPtr val(static_pointer_cast<Expression>(
                            node->getNthKid(ForEachStatement::ValueExpr)));
        ExpressionPtr name(static_pointer_cast<Expression>(
                             node->getNthKid(ForEachStatement::NameExpr)));
        StatementPtr body(static_pointer_cast<Statement>(
                            node->getNthKid(ForEachStatement::BodyStmt)));
        emitForeach(e, val, name, body, fe->isStrong());
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
            StringData* nLiteral = m_unit.lookupLitstrStr(sv->getName());
            Id i = m_curFunc->lookupVarId(nLiteral);
            e.Loc(i);
            e.String(nLiteral);
            markGlobalName(e);
            e.VGetG();
            e.BindH();
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
            Emitter condEmitter(branch->getCondition(), m_unit, *this);
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
        assert(false); // handled by KindOfIfStatement
        return false;

      case Statement::KindOfReturnStatement: {
        ReturnStatementPtr r(static_pointer_cast<ReturnStatement>(node));
        if (visit(r->getRetExp())) {
          if (r->getRetExp()->getContext() & Expression::RefValue) {
            emitConvertToVar(e);
            e.RetV();
          } else {
            emitConvertToCell(e);
            e.RetC();
          }
        } else {
          e.Null();
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
          StringData* name = m_unit.lookupLitstrStr(sv->getName());
          Id local = m_curFunc->lookupVarId(name);
          StringData* mangledName = mangleStaticName(sv->getName());

          Func::SVInfo svInfo;
          svInfo.name = name;
          svInfo.mangledName = mangledName;
          std::ostringstream os;
          CodeGenerator cg(&os, CodeGenerator::PickledPHP);
          AnalysisResultPtr ar(new AnalysisResult());
          value->outputPHP(cg, ar);
          svInfo.phpCode = StringData::GetStaticString(os.str());
          m_curFunc->m_staticVars.push_back(svInfo);

          if (value->isScalar()) {
            visit(value);
            emitConvertToCell(e);
            e.StaticLocInit(local, mangledName);
          } else {
            Label done;
            e.StaticLoc(local, mangledName);
            e.JmpNZ(done);

            e.Loc(local);
            visit(value);
            emitConvertToCell(e);
            emitSet(e);
            emitPop(e);

            done.set(e);
          }
        }
        // This function contains a static local. It'll need static local
        // context (see the definition of ActRec) if it's a non-private method,
        // a closure, or a generator from a closure.
        m_curFunc->m_needsStaticLocalCtx =
          (m_curFunc->m_preClass != NULL &&
           (m_curFunc->m_attrs & (AttrPublic | AttrProtected))) ||
          ParserBase::IsContinuationFromClosureName(
            m_curFunc->m_name->toCPPString());

        // Closures should have been covered by the above criteria.
        ASSERT(!m_curFunc->m_isClosureBody || m_curFunc->m_needsStaticLocalCtx);
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
        if (!simpleSubject) {
          // Evaluate the subject once and stash it in a local
          tempLocal = m_curFunc->allocUnnamedLocal();
          e.Loc(tempLocal);
          visit(subject);
          emitConvertToCell(e);
          e.SetH();
          emitPop(e);
          start = m_unit.bcPos();
        }

        int defI = -1;
        uint i = 0;
        for (i = 0; i < ncase; i++) {
          CaseStatementPtr c(static_pointer_cast<CaseStatement>((*cases)[i]));
          if (c->getCondition()) {
            if (simpleSubject) {
              // Evaluate the subject every time.
              visit(subject);
              emitConvertToCellOrHome(e);
              visit(c->getCondition());
              emitConvertToCell(e);
              emitConvertSecondToCell(e);
            } else {
              e.Loc(tempLocal);
              e.CGetH();
              visit(c->getCondition());
              emitConvertToCell(e);
            }
            e.Eq();
            Label next;
            e.JmpZ(next);
            e.Jmp(caseLabels[i]);
            next.set(e);
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
        for (i = 0; i < ncase; i++) {
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
        if (!simpleSubject) {
          // Null out temp local, to invoke any needed refcounting
          ASSERT(tempLocal >= 0);
          ASSERT(start != InvalidAbsoluteOffset);
          newFaultRegion(start, m_unit.bcPos(),
                         new UnsetUnnamedLocalThunklet(tempLocal));
          e.Loc(tempLocal);
          e.UnsetH();
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

      case Statement::KindOfTryStatement: {
        if (!m_evalStack.empty()) {
          Logger::Warning("Emitter detected that the evaluation stack "
                          "is not empty at the beginning of a try region: %d",
                          m_unit.bcPos());
        }
        Label after;
        TryStatementPtr ts = static_pointer_cast<TryStatement>(node);

        Offset start = m_unit.bcPos();
        visit(ts->getNthKid(0));
        // include the jump out of the try-catch block in the
        // exception handler address range
        e.Jmp(after);
        Offset end = m_unit.bcPos();

        if (!m_evalStack.empty()) {
          Logger::Warning("Emitter detected that the evaluation stack "
                          "is not empty at the end of a try region: %d",
                          end);
        }

        StatementListPtr catches = ts->getCatches();
        ExnHandlerRegion *r = new ExnHandlerRegion(start, end);
        m_exnHandlers.push_back(r);

        int n = catches->getCount();
        bool firstHandler = true;
        for (int i = 0; i < n; i++) {
          CatchStatementPtr c(static_pointer_cast<CatchStatement>
                              ((*catches)[i]));
          StringData* eName = m_unit.lookupLitstrStr(c->getClassName());

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
        visit(node->getNthKid(WhileStatement::CondExpr));
        emitConvertToCell(e);
        e.JmpZ(fail);
        {
          CONTROL_BODY(fail, preCond, brkHand, cntHand);
          visit(node->getNthKid(WhileStatement::BodyStmt));
        }
        e.Jmp(preCond);
        emitBreakHandler(e, fail, preCond, brkHand, cntHand);
        fail.set(e);
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
        assert(false);
        return false;

      case Statement::KindOfFunctionStatement: {
        MethodStatementPtr m(static_pointer_cast<MethodStatement>(node));
        // Only called for fn defs not on the top level
        StringData* nName = m_unit.lookupLitstrStr(m->getOriginalName());
        Func *f = m_unit.newFunc(nName, false);
        f->m_isGenerator = m->getFunctionScope()->isGenerator();
        // Body of fn postponed until after current body
        postponeMeth(m, f, false);
        ASSERT(!node->getClassScope()); // Handled directly by emitClass().
        e.DefFunc(m_unit.m_funcs.size() - 1);
        return false;
      }

      case Statement::KindOfGotoStatement: {
        GotoStatementPtr g(static_pointer_cast<GotoStatement>(node));
        StringData *nName = m_unit.lookupLitstrStr(g->label());
        e.Jmp(m_gotoLabels[nName]);
        return false;
      }

      case Statement::KindOfLabelStatement: {
        LabelStatementPtr l(static_pointer_cast<LabelStatement>(node));
        StringData *nName = m_unit.lookupLitstrStr(l->label());
        Label &lab = m_gotoLabels[nName];
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

        // Should be handled by KindOfUnsetStatement
        ASSERT(op != T_UNSET);

        if (op == T_ARRAY) {
          if (u->isScalar()) {
            TypedValue tv;
            TV_WRITE_UNINIT(&tv);
            initScalar(tv, u);
            if (m_staticArrays.size() == 0) {
              e.Array(tv.m_data.parr);
            }
          } else {
            ASSERT(m_staticArrays.size() == 0);
            e.NewArray();
            visit(u->getExpression());
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
          e.Loc(oldErrorLevelLoc);
          {
            e.FPushFuncD(1, m_unit.lookupLitstrStr("error_reporting"));
            FPIRegionRecorder fpi(this, m_unit, m_evalStack);
            e.Int(0);
            e.FPassC(0);
            e.FCall(1);
          }
          e.UnboxR();
          e.SetH();
          e.PopC();
          start = m_unit.bcPos();
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
            newFaultRegion(start, m_unit.bcPos(),
                           new RestoreErrorReportingThunklet(oldErrorLevelLoc));
            emitRestoreErrorReporting(e, oldErrorLevelLoc);
            m_curFunc->freeUnnamedLocal(oldErrorLevelLoc);
            break;
          }
          case T_PRINT: e.Print(); break;
          case T_EVAL: e.Eval(); break;
          case T_FILE: {
            e.String(const_cast<StringData*>(m_unit.m_filepath));
            break;
          }
          case T_DIR: {
            e.String(const_cast<StringData*>(m_unit.m_dirpath));
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
          // Kinda crappy but I don't have to descend into the tree this way
          // will revisit
          bool isOr = op == T_LOGICAL_OR || op == T_BOOLEAN_OR;
          visit(b->getExp1());
          emitConvertToCell(e);
          Label shortCirc;
          if (isOr) {
            e.JmpNZ(shortCirc);
          } else {
            e.JmpZ(shortCirc);
          }
          visit(b->getExp2());
          emitConvertToCell(e);
          e.CastBool();
          Label done;
          e.Jmp(done);
          shortCirc.set(e);
          if (isOr) {
            e.True();
          } else {
            e.False();
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
              StringData* fname
                = StringData::GetStaticString("get_called_class");
              e.FPushFuncD(0, fname);
              {
                FPIRegionRecorder fpi(this, m_unit, m_evalStack);
                e.FCall(0);
              }
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
              StringData* nLiteral = m_unit.lookupLitstrStr(s);
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

        visit(b->getExp1());
        emitConvertToCellOrHome(e);
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
        StringData* nName = m_unit.lookupLitstrStr(cc->getConName());
        if (cc->isStatic()) {
          e.LateBoundCls();
          e.ClsCns(nName);
        } else {
          ExpressionPtr cls(cc->getClass());
          if (!cls) {
            const std::string &clsName = cc->getOriginalClassName();
            StringData* nCls = m_unit.lookupLitstrStr(clsName);
            e.ClsCnsD(nName, nCls);
          } else {
            visit(cls);
            emitConvertToCellOrHome(e);
            emitCls(e, 0);
            e.ClsCns(nName);
          }
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
          StringData* nName = m_unit.lookupLitstrStr(c->getName());
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
          e.FPushFuncD(1, m_unit.lookupLitstrStr("shell_exec"));
          fpi = new FPIRegionRecorder(this, m_unit, m_evalStack);
        }

        if (n) {
          visit((*args)[i++]);
          emitConvertToCellOrHome(e);
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
          e.String(m_unit.lookupLitstrStr(std::string("")));
        }

        if (el->getType() == '`') {
          emitConvertToCell(e);
          e.FPassC(0);
          e.FCall(1);
          delete fpi;
        }
        return true;
      }

      case Expression::KindOfArrayElementExpression: {
        ArrayElementExpressionPtr ae(
          static_pointer_cast<ArrayElementExpression>(node));
        // If this Dim is of the form "$GLOBALS[<literal string>]", we can
        // emit more efficient bytecode that directly operates on the global
        // specified by the literal string.
        if (ae->isSuperGlobal() && !ae->isDynamicGlobal()) {
          // TODO: Consider moving this optimization into the peephole optimizer
          StringData* nValue = m_unit.lookupLitstrStr(ae->getGlobalName());
          e.String(nValue);
          markGlobalName(e);
          return true;
        }

        visit(ae->getVariable());
        // XHP syntax allows for expressions like "($a =& $b)[0]". We
        // handle this by unboxing the var produced by "($a =& $b)".
        emitConvertToCellIfVar(e);
        if (visit(ae->getOffset())) {
          emitConvertToCellOrHome(e);
          markElem(e);
        } else {
          markNewElem(e);
        }
        return true;
      }

      case Expression::KindOfSimpleFunctionCall:
      case Expression::KindOfDynamicFunctionCall: {
        emitFuncCall(e, static_pointer_cast<FunctionCall>(node));
        return true;
      }

      case Expression::KindOfIncludeExpression: {
        IncludeExpressionPtr ie(static_pointer_cast<IncludeExpression>(node));
        visit(ie->getExpression());
        emitConvertToCell(e);
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
            e.ReqOnce();
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
          e.Loc(tempLocal);
          visit(rhs);
          emitConvertToCell(e);
          e.SetH();
          emitPop(e);
          start = m_unit.bcPos();
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
              e.Loc(tempLocal);
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
          e.Loc(tempLocal);
          emitCGet(e);
        }

        // Null out and free unnamed local
        if (!simpleRHS) {
          ASSERT(tempLocal >= 0);
          ASSERT(start != InvalidAbsoluteOffset);
          newFaultRegion(start, m_unit.bcPos(),
                         new UnsetUnnamedLocalThunklet(tempLocal));
          e.Loc(tempLocal);
          e.UnsetH();
          m_curFunc->freeUnnamedLocal(tempLocal);
        }
        return true;
      }

      case Expression::KindOfNewObjectExpression: {
        NewObjectExpressionPtr ne(
          static_pointer_cast<NewObjectExpression>(node));

        ExpressionListPtr params(ne->getParams());
        int numParams = params ? params->getCount() : 0;
        if (ne->isStatic()) {
          e.LateBoundCls();
          e.FPushCtor(numParams);
        } else if (ne->isSelf() || ne->isParent()) {
          ClassScopeRawPtr cls = ne->getOriginalClass();
          if (cls && cls->isTrait()) {
            StringData* fname = StringData::GetStaticString(ne->isSelf() ?
                                            "get_class" : "get_parent_class");
            e.FPushFuncD(0, fname);
            FPIRegionRecorder fpi(this, m_unit, m_evalStack);
            e.FCall(0);
            e.UnboxR();
            emitCls(e, 0);
            e.FPushCtor(numParams);
          } else {
            StringData* nameLiteral;
            if (cls) {
              nameLiteral = m_unit.lookupLitstrStr(ne->isSelf() ?
                cls->getOriginalName() : cls->getOriginalParent());
            } else {
              nameLiteral = m_unit.lookupLitstrStr(ne->getOriginalName());
            }
            e.FPushCtorD(numParams, nameLiteral);
          }
        } else {
          if (!ne->getOriginalName().empty()) {
            StringData* nameLiteral
              = m_unit.lookupLitstrStr(ne->getOriginalName());
            e.FPushCtorD(numParams, nameLiteral);
          } else {
            visit(ne->getNameExp());
            emitConvertToCellOrHome(e);
            emitCls(e, 0);
            e.FPushCtor(numParams);
          }
        }

        {
          FPIRegionRecorder fpi(this, m_unit, m_evalStack);
          for (int i = 0; i < numParams; i++) {
            emitFuncCallArg(e, (*params)[i], i);
          }
          e.FCall(numParams);
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
        emitConvertToCell(e);
        ExpressionListPtr params(om->getParams());
        int numParams = params ? params->getCount() : 0;

        if (!om->getName().empty()) {
          // $obj->name(...)
          //       ^^^^
          // Use getOriginalName, which hasn't been case-normalized, since
          // __call() is case-preserving.
          StringData* nameLiteral =
            m_unit.lookupLitstrStr(om->getOriginalName());
          e.FPushObjMethodD(numParams, nameLiteral);
        } else {
          // $obj->{...}(...)
          //       ^^^^^
          visit(om->getNameExp());
          emitConvertToCell(e);
          e.FPushObjMethod(numParams);
        }
        {
          FPIRegionRecorder fpi(this, m_unit, m_evalStack);
          // $obj->name(...)
          //           ^^^^^
          for (int i = 0; i < numParams; i++) {
            emitFuncCallArg(e, (*params)[i], i);
          }
          e.FCall(numParams);
        }
        return true;
      }

      case Expression::KindOfObjectPropertyExpression: {
        ObjectPropertyExpressionPtr op(
          static_pointer_cast<ObjectPropertyExpression>(node));
        visit(op->getObject());
        emitNameString(e, op->getProperty());
        emitConvertToCellOrHome(e);
        markProp(e);
        return true;
      }

      case Expression::KindOfQOpExpression: {
        QOpExpressionPtr q(static_pointer_cast<QOpExpression>(node));
        if (q->getYes()) {
          // <expr> ? <expr> : <expr>
          Label tru, fals, done;
          {
            Emitter condEmitter(q->getCondition(), m_unit, *this);
            visitIfCondition(q->getCondition(), condEmitter,
                             tru, fals, true);
          }
          if (tru.isUsed()) {
            tru.set(e);
          }
          visit(q->getYes());
          emitConvertToCell(e);
          e.Jmp(done);
          fals.set(e);
          visit(q->getNo());
          emitConvertToCell(e);
          done.set(e);
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
                m_curFunc->m_preClass &&
                (m_curFunc->m_preClass->m_attrs & VM::AttrTrait)) {
              StringData* fname = StringData::GetStaticString("get_class");
              e.FPushFuncD(0, fname);
              FPIRegionRecorder fpi(this, m_unit, m_evalStack);
              e.FCall(0);
              e.UnboxR();
            } else {
              StringData* nValue = m_unit.lookupLitstrStr(v.getStringData());
              e.String(nValue);
            }
            break;
          }
          case KindOfInt32:
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
            e.This();
          } else {
            StringData* thisStr = m_unit.lookupLitstrStr("this");
            Id thisId = m_curFunc->lookupVarId(thisStr);
            e.Loc(thisId);
          }
        } else {
          StringData* nLiteral = m_unit.lookupLitstrStr(sv->getName());
          if (sv->isSuperGlobal()) {
            e.String(nLiteral);
            markGlobalName(e);
            return true;
          }
          Id i = m_curFunc->lookupVarId(nLiteral);
          e.Loc(i);
        }

        return true;
      }

      case Expression::KindOfDynamicVariable: {
        DynamicVariablePtr dv(static_pointer_cast<DynamicVariable>(node));
        visit(dv->getSubExpression());
        emitConvertToCellOrHome(e);
        markName(e);
        return true;
      }

      case Expression::KindOfStaticMemberExpression: {
        StaticMemberExpressionPtr sm(
          static_pointer_cast<StaticMemberExpression>(node));
        if (sm->isStatic()) {
          e.LateBoundCls();
        } else {
          if (sm->getClass()) {
            visit(sm->getClass());
            emitConvertToCellOrHome(e);
          } else {
            // For traits, class name cannot be resolved at this point yet,
            // so emit call to get_class
            ClassScopeRawPtr cls = sm->getOriginalClass();
            if (cls && cls->isTrait()) {
              StringData* fname = StringData::GetStaticString("get_class");
              e.FPushFuncD(0, fname);
              FPIRegionRecorder fpi(this, m_unit, m_evalStack);
              e.FCall(0);
              e.UnboxR();
            } else {
              StringData* nLiteral =
                m_unit.lookupLitstrStr(sm->getClassName());
              e.String(nLiteral);
            }
          }
        }
        emitNameString(e, sm->getExp());
        emitConvertToCellOrHome(e);
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
                StringData* sd = StringData::GetStaticString(
                  NEW(StringData)("", 0, CopyString));
                tvKey.m_data.pstr = sd;
                tvKey.m_type = KindOfString;
              } else if (c->isBoolean()) {
                tvKey.m_data.num = c->getBooleanValue() ? 1 : 0;
                tvKey.m_type = KindOfBoolean;
              } else {
                // Handle INF and NAN
                ASSERT(c->isDouble());
                tvKey.m_data.num = 0x8000000000000000;
                tvKey.m_type = KindOfInt64;
              }
            } else if (key->is(Expression::KindOfScalarExpression)) {
              ScalarExpressionPtr sval(
                static_pointer_cast<ScalarExpression>(key));
              const std::string *s;
              int64 i;
              double d;
              if (sval->getString(s)) {
                StringData* sd = StringData::GetStaticString(
                  NEW(StringData)(s->c_str(), s->size(), CopyString));
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
                tvKey.m_data.num = int64(d);
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
            emitConvertToCell(e);
          }
          visit(ap->getValue());
          if (ap->isRef()) {
            emitConvertToVar(e);
            if (hasKey) {
              e.AddElemV();
            } else {
              e.AddNewElemV();
            }
          } else {
            emitConvertToCell(e);
            if (hasKey) {
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
          visit(p);
          if (pop && i != keep) {
            emitPop(e);
          } else {
            cnt++;
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
        ExpressionListPtr useList(
          static_pointer_cast<ExpressionList>(ce->getNthKid(0)));
        ClosureUseVarVec useVars;
        int useCount = (useList ? useList->getCount() : 0);
        if (useList) {
          for (int i = 0; i < useCount; ++i) {
            SimpleVariablePtr var(
              static_pointer_cast<SimpleVariable>((*useList)[i]));
            StringData* varName = m_unit.lookupLitstrStr(var->getName());
            useVars.push_back(
              ClosureUseVar(varName, var->getContext() & Expression::RefValue));
          }
        }

        StringData* className = newClosureName();
        StringData* parentName = m_unit.lookupLitstrStr("closure");
        PreClass* preClass =
          m_unit.newPreClass(className, AttrNone, parentName, NULL,
                             ce->getLocation().get(), m_unit.bcPos(),
                             /* hoistable = */ false);
        e.DefCls(preClass->m_id);

        // We're still at the closure definition site. Emit code to instantiate
        // the new anonymous class, with the use variables as arguments.
        {
          e.FPushCtorD(useCount, className);
          FPIRegionRecorder fpi(this, m_unit, m_evalStack);
          for (int i = 0; i < useCount; ++i) {
            e.Loc(m_curFunc->lookupVarId(useVars[i].first));
            e.FPassH(i);
          }
          e.FCall(useCount);
        }
        emitPop(e);
        // From here on out, we're just building metadata for the closure.

        // Instance variables.
        TypedValue uninit;
        TV_WRITE_UNINIT(&uninit);
        for (int i = 0; i < useCount; ++i) {
          preClass->addProperty(useVars[i].first, AttrPrivate, NULL, &uninit);
        }

        // The constructor. This is entirely generated; all it does is stash its
        // arguments in the object's instance variables.
        StringData* ctorName = m_unit.lookupLitstrStr("__construct");
        Func* ctor = new Func(m_unit, ctorName, preClass);
        preClass->addMethod(ctor);
        m_postponedClosureCtors.push_back(
          PostponedClosureCtor(useVars, ce, ctor));

        // The __invoke method. This is the body of the closure, preceded by
        // code that pulls the object's instance variables into locals.
        StringData* invokeName = m_unit.lookupLitstrStr("__invoke");
        Func* invoke = new Func(m_unit, invokeName, preClass);
        invoke->m_isClosureBody = true;
        preClass->addMethod(invoke);
        MethodStatementPtr body(
          static_pointer_cast<MethodStatement>(ce->getClosureFunction()));
        postponeMeth(body, invoke, false, new ClosureUseVarVec(useVars));

        return true;
      }
    }
  }

  assert(false);
  return false;
}

int EmitterVisitor::scanStackForLocation(int iLast) {
  ASSERT(iLast >= 0);
  ASSERT(iLast < (int)m_evalStack.size());
  for (int i = iLast; i >= 0; --i) {
    char marker = StackSym::GetMarker(m_evalStack.get(i));
    if (marker != StackSym::E && marker != StackSym::W &&
        marker != StackSym::P) {
      return i;
    }
  }
  Logger::Warning("Emitter expected a location on the stack but none "
                  "was found (at offset %d)",
                  m_unit.bcPos());
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

  {
    char sym = m_evalStack.get(iFirst);
    char protoflavor = StackSym::GetProtoflavor(sym);
    char marker = StackSym::GetMarker(sym);
    switch (marker) {
      case StackSym::N: {
        ASSERT(protoflavor == StackSym::C || protoflavor == StackSym::H);
        vectorImm.push_back(LN);
      } break;
      case StackSym::G: {
        ASSERT(protoflavor == StackSym::C || protoflavor == StackSym::H);
        vectorImm.push_back(LG);
      } break;
      case StackSym::S: {
        ASSERT(protoflavor == StackSym::C || protoflavor == StackSym::H);
        vectorImm.push_back(LS);
      } break;
      case StackSym::None: {
        if (protoflavor == StackSym::H) {
          vectorImm.push_back(LH);
        } else if (protoflavor == StackSym::C) {
          vectorImm.push_back(LC);
        } else if (protoflavor == StackSym::R) {
          vectorImm.push_back(LR);
        } else {
          ASSERT(false);
        }
      } break;
      default: {
        ASSERT(false);
        break;
      }
    }
  }
  int i = iFirst + 1;
  while (i <= iLast) {
    char sym = m_evalStack.get(i);
    char marker = StackSym::GetMarker(sym);
    switch (marker) {
      case StackSym::E: {
        vectorImm.push_back(ME);
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
        vectorImm.push_back(MP);
      } break;
      case StackSym::S: {
        ASSERT(false);
      }
      default: ASSERT(false); break;
    }
    ++i;
  }
}

void EmitterVisitor::emitPop(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a Pop* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.CGetH();  // fall through
      case StackSym::C:  e.PopC(); break;
      case StackSym::HN: e.CGetH();  // fall through
      case StackSym::CN: e.CGetN(); e.PopC(); break;
      case StackSym::HG: e.CGetH();  // fall through
      case StackSym::CG: e.CGetG(); e.PopC(); break;
      case StackSym::HS: e.CGetH();  // fall through
      case StackSym::CS: e.CGetS(); e.PopC(); break;
      case StackSym::V:  e.PopV(); break;
      case StackSym::R:  e.PopR(); break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitPop() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
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

void EmitterVisitor::emitCGet(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a CGet* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.CGetH();  break;
      case StackSym::C:  /* nop */   break;
      case StackSym::HN: e.CGetH();  // fall through
      case StackSym::CN: e.CGetN();  break;
      case StackSym::HG: e.CGetH();  // fall through
      case StackSym::CG: e.CGetG();  break;
      case StackSym::HS: e.CGetH();  // fall through
      case StackSym::CS: e.CGetS();  break;
      case StackSym::V:  e.Unbox();  break;
      case StackSym::R:  e.UnboxR(); break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitCGet() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.CGetM(vectorImm);
  }
}

void EmitterVisitor::emitVGet(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a VGet* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.VGetH(); break;
      case StackSym::C:  e.Box();   break;
      case StackSym::HN: e.CGetH(); // fall through
      case StackSym::CN: e.VGetN(); break;
      case StackSym::HG: e.CGetH(); // fall through
      case StackSym::CG: e.VGetG(); break;
      case StackSym::HS: e.CGetH(); // fall through
      case StackSym::CS: e.VGetS(); break;
      case StackSym::V:  /* nop */  break;
      case StackSym::R:  e.BoxR();  break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitVGet() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
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
    default:
      break;
  }
  // All other cases
  return PassByRefKind::ErrorOnCell;
}

void EmitterVisitor::emitFuncCallArg(Emitter& e,
                                     ExpressionPtr exp,
                                     int paramId) {
  visit(exp);
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a FPass* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.FPassH(paramId); break;
      case StackSym::C:
        switch (getPassByRefKind(exp)) {
          case PassByRefKind::AllowCell:   e.FPassC(paramId); break;
          case PassByRefKind::WarnOnCell:  e.FPassCW(paramId); break;
          case PassByRefKind::ErrorOnCell: e.FPassCE(paramId); break;
          default: ASSERT(false);
        }
        break;
      case StackSym::HN: e.CGetH();  // fall through
      case StackSym::CN: e.FPassN(paramId); break;
      case StackSym::HG: e.CGetH();  // fall through
      case StackSym::CG: e.FPassG(paramId); break;
      case StackSym::HS: e.CGetH();  // fall through
      case StackSym::CS: e.FPassS(paramId); break;
      case StackSym::V:  e.FPassV(paramId); break;
      case StackSym::R:  e.FPassR(paramId); break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitFPass() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.FPassM(paramId, vectorImm);
  }
}

void EmitterVisitor::emitIsset(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a Isset* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.IssetH(); break;
      case StackSym::HN: e.CGetH();  // fall through
      case StackSym::CN: e.IssetN(); break;
      case StackSym::HG: e.CGetH();  // fall through
      case StackSym::CG: e.IssetG(); break;
      case StackSym::HS: e.CGetH();  // fall through
      case StackSym::CS: e.IssetS(); break;
      //XXX: Zend does not allow isset() on the result
      // of a function call. We allow it here so that emitted
      // code is valid. Once the parser handles this correctly,
      // the R and C cases can go.
      case StackSym::R:  e.UnboxR(); // fall through
      case StackSym::C:  e.Null(); e.NSame(); break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitIsset() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.IssetM(vectorImm);
  }
}

void EmitterVisitor::emitEmpty(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a Empty* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.EmptyH(); break;
      case StackSym::HN: e.CGetH();  // fall through
      case StackSym::CN: e.EmptyN(); break;
      case StackSym::HG: e.CGetH();  // fall through
      case StackSym::CG: e.EmptyG(); break;
      case StackSym::HS: e.CGetH();  // fall through
      case StackSym::CS: e.EmptyS(); break;
      //XXX: Zend does not allow empty() on the result
      // of a function call. We allow it here so that emitted
      // code is valid. Once the parser handles this correctly,
      // the R and C cases can go.
      case StackSym::R:  e.UnboxR(); // fall through
      case StackSym::C:  e.Not(); break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitEmpty() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.EmptyM(vectorImm);
  }
}

void EmitterVisitor::emitUnset(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a Unset* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.UnsetH(); break;
      case StackSym::HN: e.CGetH();  // fall through
      case StackSym::CN: e.UnsetN(); break;
      case StackSym::HG: e.CGetH();  // fall through
      case StackSym::CG: e.UnsetG(); break;
      case StackSym::HS: // fall through
      case StackSym::CS:
        throw IncludeTimeFatalException(e.getNode(),
                                        "Cannot unset a static property");
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitUnset() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, false, e);
    e.UnsetM(vectorImm);
  }
}

void EmitterVisitor::emitSet(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a Set* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.SetH();   break;
      case StackSym::HN: e.CGetH2(); // fall through
      case StackSym::CN: e.SetN();   break;
      case StackSym::HG: e.CGetH2(); // fall through
      case StackSym::CG: e.SetG();   break;
      case StackSym::HS: e.CGetH2(); // fall through
      case StackSym::CS: e.SetS();   break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitSet() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.SetM(vectorImm);
  }
}

void EmitterVisitor::emitSetOp(Emitter &e, int op) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a SetOp* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
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
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.SetOpH(cop); break;
      case StackSym::HN: e.CGetH2(); // fall through
      case StackSym::CN: e.SetOpN(cop); break;
      case StackSym::HG: e.CGetH2(); // fall through
      case StackSym::CG: e.SetOpG(cop); break;
      case StackSym::HS: e.CGetH2(); // fall through
      case StackSym::CS: e.SetOpS(cop); break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitSetOp() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.SetOpM(cop, vectorImm);
  }
}

void EmitterVisitor::emitBind(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a Bind* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  int iLast = m_evalStack.size()-2;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.BindH();  break;
      case StackSym::HN: e.CGetH2(); // fall through
      case StackSym::CN: e.BindN();  break;
      case StackSym::HG: e.CGetH2(); // fall through
      case StackSym::CG: e.BindG();  break;
      case StackSym::HS: e.CGetH2(); // fall through
      case StackSym::CS: e.BindS();  break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitBind() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.BindM(vectorImm);
  }
}

void EmitterVisitor::emitIncDec(Emitter &e, unsigned char cop) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter tried to emit a IncDec* instruction when the "
                    "evaluation stack is empty (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  emitClsIfSPropBase(e);
  int iLast = m_evalStack.size()-1;
  int i = scanStackForLocation(iLast);
  int sz = iLast - i;
  ASSERT(sz >= 0);
  if (sz == 0) {
    char sym = m_evalStack.get(i);
    switch (sym) {
      case StackSym::H:  e.IncDecH(cop); break;
      case StackSym::HN: e.CGetH();  // fall through
      case StackSym::CN: e.IncDecN(cop); break;
      case StackSym::HG: e.CGetH();  // fall through
      case StackSym::CG: e.IncDecG(cop); break;
      case StackSym::HS: e.CGetH();  // fall through
      case StackSym::CS: e.IncDecS(cop); break;
      default: {
        Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                        "emitIncDec() (at offset %d)",
                        StackSym::ToString(sym).c_str(),
                        m_unit.bcPos());
        break;
      }
    }
  } else {
    std::vector<uchar> vectorImm;
    buildVectorImm(vectorImm, i, iLast, true, e);
    e.IncDecM(cop, vectorImm);
  }
}

void EmitterVisitor::emitConvertToCell(Emitter &e) {
  emitCGet(e);
}

void EmitterVisitor::emitConvertSecondToCell(Emitter &e) {
  if (m_evalStack.size() <= 1) {
    Logger::Warning("Emitter encounted an empty evaluation stack when inside "
                    "the emitConvertSecondToCell() function (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.get(m_evalStack.size()-2);
  char protoflavor = StackSym::GetProtoflavor(sym);
  if (protoflavor == StackSym::C) {
    // do nothing
  } else if (protoflavor == StackSym::H) {
    e.CGetH2();
  } else {
    // emitConvertSecondToCell() should never be used for protoflavors other
    // than C or H
    Logger::Warning("Emitter encountered an unsupported StackSym \"%s\" on "
                    "the evaluation stack inside the emitConvertSecondToCell()"
                    " function (at offset %d)",
                    StackSym::ToString(sym).c_str(),
                    m_unit.bcPos());
  }
}

void EmitterVisitor::emitConvertToCellIfVar(Emitter &e) {
  if (!m_evalStack.empty()) {
    char sym = m_evalStack.top();
    if (sym == StackSym::V) {
      emitConvertToCell(e);
    }
  }
}

void EmitterVisitor::emitConvertToCellOrHome(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter encounted an empty evaluation stack when inside "
                    "the emitConvertToCellOrHome() function (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::H) {
    // If the top of stack is a home that is not marked, do nothing
  } else {
    // Otherwise, call emitCGet to convert the top of stack to cell
    emitCGet(e);
  }
}

void EmitterVisitor::emitConvertToVar(Emitter &e) {
  emitVGet(e);
}

void EmitterVisitor::emitCls(Emitter &e, int depthActual) {
  ASSERT(depthActual >= 0);
  int posActual = (m_evalStack.sizeActual() - depthActual - 1);
  if (posActual < 0) {
    Logger::Warning("Emitter tried to emit a Cls* instruction with an "
                    "invalid argument %d (at offset %d)",
                    depthActual, m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.getActual(posActual);
  if (sym == StackSym::C) {
    e.Cls(depthActual);
  } else if (sym == StackSym::H) {
    e.ClsH(depthActual);
  } else if (sym == StackSym::A) {
    // do nothing
  } else {
    Logger::Warning("Emitter encounted unexpected StackSym \"%s\" in "
                    "emitCls() (at offset %d)",
                    StackSym::ToString(sym).c_str(),
                    m_unit.bcPos());
  }
}

void EmitterVisitor::emitClsIfSPropBase(Emitter &e) {
  // If the eval stack is empty, then there is no work to do
  if (m_evalStack.empty()) return;
  // Scan past any values marked with the Elem, NewElem, or Prop markers
  int posActual = m_evalStack.sizeActual() - 1;
  for (;;) {
    char marker = StackSym::GetMarker(m_evalStack.getActual(posActual));
    if (marker != StackSym::E && marker != StackSym::W &&
        marker != StackSym::P) {
      break;
    }
    --posActual;
    if (posActual < 0) {
      Logger::Warning("Emitter expected a location on the stack but none "
                      "was found (at offset %d)",
                      m_unit.bcPos());
      return;
    }
  }
  // After scanning, if we did not find a value marked with the SProp
  // marker then there is no work to do
  if (StackSym::GetMarker(m_evalStack.getActual(posActual)) != StackSym::S) {
    return;
  }
  --posActual;
  if (posActual < 0) {
    Logger::Warning("Emitter emitted an instruction that tries to consume "
                    "a value from the stack when the stack is empty "
                  "(expected protoflavor \"C\" or \"H\" at offset %d)",
                  m_unit.bcPos());
  }
  int depthActual = m_evalStack.sizeActual() - posActual - 1;
  emitCls(e, depthActual);
}

void EmitterVisitor::markElem(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter encountered an empty evaluation stack inside "
                    "the markElem function (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::H) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::E));
  } else {
    Logger::Warning("Emitter encountered an unsupported StackSym \"%s\" on "
                    "the evaluation stack inside the markElem function (at "
                    "offset %d)",
                    StackSym::ToString(sym).c_str(),
                    m_unit.bcPos());
  }
}

void EmitterVisitor::markNewElem(Emitter &e) {
  m_evalStack.push(StackSym::W);
}

void EmitterVisitor::markProp(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter encountered an empty evaluation stack inside "
                    "the markProp function (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::H) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::P));
  } else {
    Logger::Warning("Emitter encountered an unsupported StackSym \"%s\" on "
                    "the evaluation stack inside the markProp function (at "
                    "offset %d)",
                    StackSym::ToString(sym).c_str(),
                    m_unit.bcPos());
  }
}

void EmitterVisitor::markSProp(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter encountered an empty evaluation stack inside "
                    "the markSProp function (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::H) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::S));
  } else {
    Logger::Warning("Emitter encountered an unsupported StackSym \"%s\" on "
                    "the evaluation stack inside the markSProp function "
                    "(at offset %d)",
                    StackSym::ToString(sym).c_str(),
                    m_unit.bcPos());
  }
}

#define MARK_NAME_BODY(index, requiredStackSize)                          \
  if (m_evalStack.size() < requiredStackSize) {                           \
    Logger::Warning("Emitter encountered an evaluation stack with %d "    \
                    "elements inside the %s function (at offset %d)",     \
                    m_evalStack.size(), __FUNCTION__, m_unit.bcPos());    \
    return;                                                               \
  }                                                                       \
  char sym = m_evalStack.get(index);                                      \
  if (sym == StackSym::C || sym == StackSym::H) {                         \
    m_evalStack.set(index, (sym | StackSym::N));                          \
  } else {                                                                \
    Logger::Warning("Emitter encountered an unsupported StackSym \"%s\" " \
                    "on the evaluation stack inside the %s function (at " \
                    "offset %d)",                                         \
                    StackSym::ToString(sym).c_str(), __FUNCTION__,        \
                    m_unit.bcPos());                                      \
}

void EmitterVisitor::markName(Emitter &e) {
  int index = m_evalStack.size() - 1;
  MARK_NAME_BODY(index, 1);
}

void EmitterVisitor::markNameSecond(Emitter& e) {
  int index = m_evalStack.size() - 2;
  MARK_NAME_BODY(index, 2);
}

#undef MARK_NAME_BODY

void EmitterVisitor::markGlobalName(Emitter &e) {
  if (m_evalStack.empty()) {
    Logger::Warning("Emitter encountered an empty evaluation stack inside "
                    "the markGlobalName function (at offset %d)",
                    m_unit.bcPos());
    return;
  }
  char sym = m_evalStack.top();
  if (sym == StackSym::C || sym == StackSym::H) {
    m_evalStack.set(m_evalStack.size()-1, (sym | StackSym::G));
  } else {
    Logger::Warning("Emitter encountered an unsupported StackSym \"%s\" on "
                    "the evaluation stack inside the markGlobalName function "
                    "(at offset %d)",
                    StackSym::ToString(sym).c_str(),
                    m_unit.bcPos());
  }
}

void EmitterVisitor::emitNameString(Emitter &e, ExpressionPtr n) {
  if (n->isLiteralString()) {
    StringData* nLiteral = m_unit.lookupLitstrStr(n->getLiteralString());
    e.String(nLiteral);
  } else {
    visit(n);
  }
}

void EmitterVisitor::postponeMeth(MethodStatementPtr m, Func *f, bool top,
                                  ClosureUseVarVec* useVars /* = NULL */) {
  m_postponedMeths.push_back(PostponedMeth(m, f, top, useVars));
}

void EmitterVisitor::postponeCtor(InterfaceStatementPtr is, Func *f) {
  m_postponedCtors.push_back(PostponedCtor(is, f));
}

void EmitterVisitor::postponePinit(InterfaceStatementPtr is, Func *f,
                                   NonScalarVec *v) {
  m_postponedPinits.push_back(PostponedNonScalars(is, f, v));
}

void EmitterVisitor::postponeSinit(InterfaceStatementPtr is, Func *f,
                                   NonScalarVec *v) {
  m_postponedSinits.push_back(PostponedNonScalars(is, f, v));
}

void EmitterVisitor::postponeCinit(InterfaceStatementPtr is, Func *f,
                                   NonScalarVec *v) {
  m_postponedCinits.push_back(PostponedNonScalars(is, f, v));
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
  while (!m_postponedMeths.empty()) {
    ASSERT(m_actualStackHighWater == 0);
    ASSERT(m_fdescHighWater == 0);
    PostponedMeth &p = m_postponedMeths.front();
    Func* func = p.m_func;
    Emitter e(p.m_meth, m_unit, *this);
    if (p.m_top) {
      // Set label
      StringData* methName =
        m_unit.lookupLitstrStr(p.m_meth->getOriginalName());
      m_methLabels[methName]->set(e);
    }
    typedef std::pair<Id, ConstructPtr> DVInitializer;
    std::vector<DVInitializer> dvInitializers;
    ExpressionListPtr params = p.m_meth->getParams();
    int numParam = params ? params->getCount() : 0;
    for (int i = 0; i < numParam; i++) {
      ParameterExpressionPtr par(
        static_pointer_cast<ParameterExpression>((*params)[i]));
      StringData* parName = m_unit.lookupLitstrStr(par->getName());
      if (par->isOptional()) {
        dvInitializers.push_back(DVInitializer(i, par->defaultValue()));
      }
      // Will be fixed up later, when the DV initializers are emitted.
      Func::ParamInfo paramInfo;
      paramInfo.m_funcletOff = InvalidAbsoluteOffset;
      paramInfo.m_phpCode = NULL;
      if (par->hasTypeHint()) {
        ConstantExpressionPtr ce =
          dynamic_pointer_cast<ConstantExpression>(par->defaultValue());
        bool nullable = ce && ce->isNull();
        paramInfo.m_typeConstraint = TypeConstraint(par->getTypeHint(),
                                                    nullable);
        TRACE(1, "Added constraint to %s\n", func->m_name->data());
      }
      func->m_params.push_back(paramInfo);
      func->appendParam(parName, par->isRef());
    }

    m_curFunc = func;

    // Assign ids to all of the local variables eagerly. This gives us the
    // nice property that all named local variables will be assigned ids
    // 0 through k-1, while any unnamed local variable will have an id >= k.
    // Note that the logic above already assigned ids to the parameters, so
    // we will still uphold the invariant that the n parameters will have
    // ids 0 through n-1 respectively.
    assignLocalVariableIds(p.m_meth->getFunctionScope());

    // set all the params and metadata etc on func
    StringData* methDoc = m_unit.lookupLitstrStr(p.m_meth->getDocComment());
    ModifierExpressionPtr mod(p.m_meth->getModifiers());
    Attr attrs = buildAttrs(mod, p.m_meth->isRef());
    Label topOfBody(e);
    func->init(p.m_meth->getLocation().get(), m_unit.bcPos(), attrs,
               p.m_top, methDoc);
    // --Method emission begins--
    {
      if (p.m_meth->getFunctionScope()->containsBareThis()) {
        ASSERT(!p.m_top);
        StringData* thisStr = m_unit.lookupLitstrStr("this");
        Id thisId = func->lookupVarId(thisStr);
        e.InitThisLoc(thisId);
      }
      for (uint i = 0; i < func->m_params.size(); i++) {
        const TypeConstraint& tc = func->m_params[i].m_typeConstraint;
        if (!tc.exists()) continue;
        TRACE(2, "permanent home for tc %s, param %d of func %s: %p\n",
              tc.typeName(), i, func->m_name->data(), &tc);
        ASSERT(tc.typeName() != (const char*)0xdeadba5eba11f00d);
        e.VerifyParamType(i);
      }
      if (func->m_isClosureBody) {
        // The MethodStatement didn't have real attributes; enforce that the
        // __invoke method is public here
        func->m_attrs = AttrPublic;
        ASSERT(p.m_closureUseVars != NULL);
        // Emit code to unpack the instance variables (which store the
        // use-variables) into locals. Some of the use-variables may have the
        // same name, in which case the last one wins.
        unsigned n = (*p.m_closureUseVars).size();
        for (unsigned i = 0; i < n; ++i) {
          StringData* name = (*p.m_closureUseVars)[i].first;
          bool byRef = (*p.m_closureUseVars)[i].second;
          e.Loc(func->lookupVarId(name));
          e.This();
          e.String(name);
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
        delete p.m_closureUseVars;
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
      StringData* msg = StringData::GetStaticString("Cannot break/continue");
      e.String(msg);
      e.Fatal();
    }
    if (exit.isUsed()) {
      exit.set(e);
    }
    // If the current position in the bytecode is reachable, emit code to
    // return null
    if (currentPositionIsReachable()) {
      e.Null();
      e.RetC();
    } // -- Method emission ends --

    FuncFinisher ff(this, e, p.m_func, e.getUnit().bcPos());

    // Default value initializers
    for (uint i = 0; i < dvInitializers.size(); ++i) {
      Label entryPoint(e);
      Id paramId = dvInitializers[i].first;
      ConstructPtr node = dvInitializers[i].second;
      e.Loc(paramId);
      e.getEmitterVisitor().visit(node);
      e.getEmitterVisitor().emitCGet(e);
      e.getEmitterVisitor().emitSet(e);
      e.PopC();
      p.m_func->m_params[paramId].m_funcletOff = entryPoint.getAbsoluteOffset();
      // Store PHP source code for default value.
      std::ostringstream os;
      CodeGenerator cg(&os, CodeGenerator::PickledPHP);
      AnalysisResultPtr ar(new AnalysisResult());
      node->outputPHP(cg, ar);
      p.m_func->m_params[paramId].m_phpCode
        = StringData::GetStaticString(os.str());
    }
    if (!dvInitializers.empty()) {
      e.Jmp(topOfBody);
    }
    m_postponedMeths.pop_front();
  }
}

void EmitterVisitor::emitPostponedCtors() {
  while (!m_postponedCtors.empty()) {
    PostponedCtor &p = m_postponedCtors.front();

    Attr attrs = AttrPublic;
    StringData* methDoc = m_unit.lookupLitstrStr("");
    p.m_func->init(p.m_is->getLocation().get(), m_unit.bcPos(), attrs, false,
                   methDoc);
    Emitter e(p.m_is, m_unit, *this);
    FuncFinisher ff(this, e, p.m_func);
    e.Null();
    e.RetC();

    m_postponedCtors.pop_front();
  }
}

void EmitterVisitor::emitPostponedPSinit(PostponedNonScalars& p, bool pinit) {
  Attr attrs = (Attr)(AttrPrivate | AttrStatic);
  StringData* methDoc = m_unit.lookupLitstrStr("");
  p.m_func->init(p.m_is->getLocation().get(), m_unit.bcPos(), attrs, false,
                 methDoc);
  p.m_func->appendParam(m_unit.lookupLitstrStr("props"), true);
  if (pinit) {
    p.m_func->appendParam(m_unit.lookupLitstrStr("sentinel"), false);
  }

  Emitter e(p.m_is, m_unit, *this);
  FuncFinisher ff(this, e, p.m_func);

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
    const StringData* propName = m_unit.lookupLitstrStr(((*p.m_vec)[i]).first);
    Label isset;

    bool conditional;
    if (pinit) {
      PreClass::Prop* preProp = p.m_func->m_preClass->m_propertyMap[propName];
      if ((preProp->m_attrs & (AttrPrivate|AttrStatic)) == AttrPrivate) {
        conditional = false;
        propName = preProp->m_mangledName;
      } else {
        conditional = true;
      }
    } else {
      conditional = false;
    }

    if (conditional) {
      e.Loc(0);
      e.String((StringData*)propName);
      markElem(e);
      emitCGet(e);
      e.Loc(1);
      e.CGetH();
      e.Same();
      e.JmpZ(isset);
    }

    e.Loc(0);
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
    PostponedNonScalars &p = m_postponedPinits.front();
    emitPostponedPSinit(p, true);
    p.release(); // Manually trigger memory cleanup.
    m_postponedPinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedSinits() {
  while (!m_postponedSinits.empty()) {
    PostponedNonScalars &p = m_postponedSinits.front();
    emitPostponedPSinit(p, false);
    p.release(); // Manually trigger memory cleanup.
    m_postponedSinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedCinits() {
  while (!m_postponedCinits.empty()) {
    PostponedNonScalars &p = m_postponedCinits.front();

    Attr attrs = (Attr)(AttrPrivate | AttrStatic);
    StringData* methDoc = m_unit.lookupLitstrStr("");
    p.m_func->init(p.m_is->getLocation().get(), m_unit.bcPos(), attrs, false,
                   methDoc);
    p.m_func->appendParam(m_unit.lookupLitstrStr("constName"), false);

    Emitter e(p.m_is, m_unit, *this);
    FuncFinisher ff(this, e, p.m_func);

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
    for (size_t i = 0; i < nConsts - 1; ++i) {
      Label mismatch;

      e.Loc(0);
      e.CGetH();
      e.String((StringData*)m_unit.lookupLitstrStr(((*p.m_vec)[i]).first));
      e.Eq();
      e.JmpZ(mismatch);

      visit((*p.m_vec)[i].second);

      e.RetC();
      mismatch.set(e);
    }
    visit((*p.m_vec)[nConsts-1].second);
    e.RetC();

    p.release(); // Manually trigger memory cleanup.
    m_postponedCinits.pop_front();
  }
}

void EmitterVisitor::emitPostponedClosureCtors() {
  while (!m_postponedClosureCtors.empty()) {
    PostponedClosureCtor& ctor = m_postponedClosureCtors.front();
    ClosureUseVarVec& useVars = ctor.m_useVars;
    Func* f = ctor.m_func;
    f->init(ctor.m_expr->getLocation().get(), m_unit.bcPos(), AttrPublic, false,
            NULL);

    unsigned n = useVars.size();
    Emitter e(ctor.m_expr, m_unit, *this);
    FuncFinisher ff(this, e, f);
    if (n > 0) {
      for (unsigned i = 0; i < n; ++i) {
        // To ensure that we get a new local for every use var, we call
        // appendParam with an artificial uniquified name. Because there's no
        // user code here, the fact that the variable has a made-up name in the
        // metadata doesn't matter.
        std::ostringstream num;
        num << i;
        f->appendParam(StringData::GetStaticString(num.str()),
                       useVars[i].second);

        e.This();
        e.String(useVars[i].first);
        markProp(e);
        e.Loc(i);
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
    e.RetC();

    m_postponedClosureCtors.pop_front();
  }
}

void EmitterVisitor::emitFuncCall(Emitter &e, FunctionCallPtr node) {
  ExpressionPtr nameExp = node->getNameExp();
  const std::string &nameStr = node->getOriginalName();
  ExpressionListPtr params(node->getParams());
  int numParams = params ? params->getCount() : 0;
  if (node->getClass() || !node->getClassName().empty()) {
    bool isSelfOrParent = node->isSelf() || node->isParent();
    if (!node->isStatic() && !isSelfOrParent &&
        !node->getOriginalClassName().empty() && !nameStr.empty()) {
      // cls::foo()
      StringData* cLiteral =
        m_unit.lookupLitstrStr(node->getOriginalClassName());
      StringData* nLiteral = m_unit.lookupLitstrStr(nameStr);
      e.FPushClsMethodD(numParams, nLiteral, cLiteral);
    } else {
      if (node->isStatic()) {
        // static::...
        e.LateBoundCls();
      } else if (!node->getOriginalClassName().empty()) {
        // cls::...()
        // For trait methods, self:: and parent:: cannot be resolved yet.
        // So we generate calls to get_class and get_parent_class instead.
        if (isSelfOrParent && node->getOriginalClass()->isTrait()) {
          StringData* fname =
            StringData::GetStaticString(node->isSelf() ? "get_class" :
                                        "get_parent_class");
          e.FPushFuncD(0, fname);
          FPIRegionRecorder fpi(this, m_unit, m_evalStack);
          e.FCall(0);
          e.UnboxR();
        } else {
          StringData* cLiteral =
            m_unit.lookupLitstrStr(node->getClassName());
          e.String(cLiteral);
        }
      } else {
        // $cls::...()
        visit(node->getClass());
        emitConvertToCellOrHome(e);
      }
      if (!nameStr.empty()) {
        // ...::foo()
        StringData* nLiteral = m_unit.lookupLitstrStr(nameStr);
        e.String(nLiteral);
      } else {
        // ...::$foo()
        visit(nameExp);
        emitConvertToCell(e);
      }
      emitCls(e, 1);
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
    StringData* nLiteral = m_unit.lookupLitstrStr(nameStr);

    if (!m_curFunc->m_isGenerator) {
      e.FPushFuncD(numParams, nLiteral);
    } else {
      // Special handling for func_get_args and friends inside a generator.
      StringData* specialMethodName = NULL;
      StringData* contName = m_unit.lookupLitstrStr(CONTINUATION_OBJECT_NAME);
      Id contId = m_curFunc->lookupVarId(contName);
      if (nameStr == "func_get_args") {
        specialMethodName = StringData::GetStaticString("get_args");
      } else if (nameStr == "func_num_args") {
        specialMethodName = StringData::GetStaticString("num_args");
      } else if (nameStr == "func_get_arg") {
        specialMethodName = StringData::GetStaticString("get_arg");
      }

      if (specialMethodName != NULL) {
        e.Loc(contId);
        emitConvertToCell(e);
        e.FPushObjMethodD(numParams, specialMethodName);
      } else {
        // Normal case -- it wasn't func_get_args and friends
        e.FPushFuncD(numParams, nLiteral);
      }
    }
  } else {
    // $foo()
    visit(nameExp);
    emitConvertToCell(e);
    // FPushFunc consumes method name from stack
    e.FPushFunc(numParams);
  }
  {
    FPIRegionRecorder fpi(this, m_unit, m_evalStack);
    for (int i = 0; i < numParams; i++) {
      emitFuncCallArg(e, (*params)[i], i);
    }
    e.FCall(numParams);
  }
}

void EmitterVisitor::emitClassTraitPrecRule(PreClass* preClass,
                                            TraitPrecStatementPtr stmt) {
  StringData* traitName  = m_unit.lookupLitstrStr(stmt->getTraitName());
  StringData* methodName = m_unit.lookupLitstrStr(stmt->getMethodName());

  PreClass::TraitPrecRule rule(traitName, methodName);

  std::set<std::string> otherTraitNames;
  stmt->getOtherTraitNames(otherTraitNames);
  for (std::set<std::string>::iterator it = otherTraitNames.begin();
       it != otherTraitNames.end(); it++) {
    rule.addOtherTraitName(m_unit.lookupLitstrStr(*it));
  }

  preClass->addTraitPrecRule(rule);
}

void EmitterVisitor::emitClassTraitAliasRule(PreClass* preClass,
                                             TraitAliasStatementPtr stmt) {
  StringData* traitName    = m_unit.lookupLitstrStr(stmt->getTraitName());
  StringData* origMethName = m_unit.lookupLitstrStr(stmt->getMethodName());
  StringData* newMethName  = m_unit.lookupLitstrStr(stmt->getNewMethodName());
  Attr attr = buildAttrs(stmt->getModifiers());

  PreClass::TraitAliasRule rule(traitName, origMethName, newMethName, attr);

  preClass->addTraitAliasRule(rule);
}

void EmitterVisitor::emitClassUseTrait(PreClass* preClass,
                                       UseTraitStatementPtr useStmt) {
  StatementListPtr rules = useStmt->getStmts();
  for (int r = 0; r < rules->getCount(); r++) {
    StatementPtr rule = (*rules)[r];
    TraitPrecStatementPtr precStmt =
      dynamic_pointer_cast<TraitPrecStatement>(rule);
    if (precStmt) {
      emitClassTraitPrecRule(preClass, precStmt);
    } else {
      TraitAliasStatementPtr aliasStmt =
        dynamic_pointer_cast<TraitAliasStatement>(rule);
      ASSERT(aliasStmt);
      emitClassTraitAliasRule(preClass, aliasStmt);
    }
  }
}

void EmitterVisitor::emitClass(Emitter& e, ClassScopePtr cNode,
                               bool hoistable) {
  InterfaceStatementPtr is(
    static_pointer_cast<InterfaceStatement>(cNode->getStmt()));
  StringData* className = m_unit.lookupLitstrStr(cNode->getOriginalName());
  StringData* parentName = m_unit.lookupLitstrStr(cNode->getOriginalParent());
  StringData* classDoc = m_unit.lookupLitstrStr(cNode->getDocComment());
  Attr attr = cNode->isInterface() ? AttrInterface :
              cNode->isTrait()     ? AttrTrait     :
              cNode->isAbstract()  ? AttrAbstract  :
              cNode->isFinal()     ? AttrFinal     :
                                     AttrNone;
  PreClass* preClass = m_unit.newPreClass(className, attr,
                                          parentName, classDoc,
                                          is->getLocation().get(),
                                          m_unit.bcPos(), hoistable);
  e.DefCls(preClass->m_id);
  const std::vector<std::string> &bases(cNode->getBases());
  for (int i = cNode->getOriginalParent().empty()
               ? 0 : 1, nInterfaces = bases.size();
       i < nInterfaces; ++i) {
    preClass->addInterface(m_unit.lookupLitstrStr(bases[i]));
  }
  const std::vector<std::string> &usedTraits = cNode->getUsedTraitNames();
  for (size_t i = 0; i < usedTraits.size(); i++) {
    preClass->addUsedTrait(m_unit.lookupLitstrStr(usedTraits[i]));
  }

  NonScalarVec* nonScalarPinitVec = NULL;
  NonScalarVec* nonScalarSinitVec = NULL;
  NonScalarVec* nonScalarConstVec = NULL;
  if (StatementListPtr stmts = is->getStmts()) {
    int i, n = stmts->getCount();
    for (i = 0; i < n; i++) {
      if (MethodStatementPtr meth =
          dynamic_pointer_cast<MethodStatement>((*stmts)[i])) {
        StringData* methName = m_unit.lookupLitstrStr(meth->getOriginalName());
        ModifierExpressionPtr modifiers = meth->getModifiers();
        if (preClass->m_attrs & AttrInterface) {
          if (modifiers->isProtected()
              /* GO: The following fatal in Zend, but hphpi currently allows.
                 TODO: enable these checks when safe:
                 || modifiers->isPublic()    || modifiers->isPrivate()
                 || modifiers->isAbstract()  || modifiers->isFinal() */
             ) {
            throw IncludeTimeFatalException(meth,
                    "Access type for interface method %s::%s() must be omitted",
                    preClass->m_name->data(), methName->data());
          }
        }
        if (modifiers->isAbstract()) {
          if (modifiers->isPrivate() || modifiers->isFinal()) {
            throw IncludeTimeFatalException(meth,
                          "Cannot declare abstract method %s::%s() %s",
                          preClass->m_name->data(),
                          methName->data(),
                          modifiers->isPrivate() ? "private" : "final");
          }
          if (!(preClass->m_attrs & AttrAbstract) &&
              !(preClass->m_attrs & (AttrTrait | AttrInterface))) {
            throw IncludeTimeFatalException(meth,
                                  "Class %s contains abstract method %s and "
                                  "must therefore be declared abstract",
                                  preClass->m_name->data(), methName->data());
          }
          if (meth->getStmts()) {
            throw IncludeTimeFatalException(meth,
                          "Abstract method %s::%s() cannot contain body",
                          preClass->m_name->data(), methName->data());
          }
        }
        Func* func = new Func(m_unit, methName, preClass);
        func->m_isGenerator = meth->getFunctionScope()->isGenerator();
        bool added = preClass->addMethod(func);
        if (!added) {
          throw IncludeTimeFatalException(meth,
                                          "Method already declared: %s::%s",
                                          preClass->m_name->data(),
                                          methName->data());
        }
        postponeMeth(meth, func, false);
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

          StringData* propName = m_unit.lookupLitstrStr(var->getName());
          StringData* propDoc = m_unit.lookupLitstrStr("");
          TypedValue tvVal;
          if (vNode) {
            if (vNode->isScalar()) {
              initScalar(tvVal, vNode);
            } else {
              TV_WRITE_UNINIT(&tvVal);
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
            TV_WRITE_NULL(&tvVal);
          }
          bool added = preClass->addProperty(propName, attrs, propDoc, &tvVal);
          if (!added) {
            throw IncludeTimeFatalException(cv,
                                            "Property already declared: %s::%s",
                                            preClass->m_name->data(),
                                            propName->data());
          }
        }
      } else if (ClassConstantPtr cc =
                 dynamic_pointer_cast<ClassConstant>((*stmts)[i])) {
        if (preClass->m_attrs & AttrTrait) {
          throw IncludeTimeFatalException(cc, "Traits cannot have constants");
        }
        ExpressionListPtr el(cc->getConList());
        int nCons = el->getCount();
        for (int ii = 0; ii < nCons; ii++) {
          AssignmentExpressionPtr ae(
            static_pointer_cast<AssignmentExpression>((*el)[ii]));
          ConstantExpressionPtr con(
            static_pointer_cast<ConstantExpression>(ae->getVariable()));
          ExpressionPtr vNode(ae->getValue());
          StringData* constName = m_unit.lookupLitstrStr(con->getName());
          ASSERT(vNode);
          TypedValue tvVal;
          if (vNode->isScalar()) {
            initScalar(tvVal, vNode);
          } else {
            TV_WRITE_UNINIT(&tvVal);
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
          bool added = preClass->addConstant(constName, &tvVal,
                                             StringData::GetStaticString(
                                               os.str()));
          if (!added) {
            throw IncludeTimeFatalException(cc,
                                            "Constant already declared: %s::%s",
                                            preClass->m_name->data(),
                                            constName->data());
          }
        }
      } else if (UseTraitStatementPtr useStmt =
                 dynamic_pointer_cast<UseTraitStatement>((*stmts)[i])) {
        emitClassUseTrait(preClass, useStmt);
      }
    }
  }

  if (!cNode->getAttribute(ClassScope::HasConstructor) &&
      !cNode->getAttribute(ClassScope::ClassNameConstructor)) {
    // cNode does not have a constructor; synthesize 86ctor() so that the class
    // will always have a method that can be called during construction.
    StringData* methName = m_unit.lookupLitstrStr("86ctor");
    Func* func = new Func(m_unit, methName, preClass);
    bool added = preClass->addMethod(func);
    if (!added) {
      throw IncludeTimeFatalException(is,
                                      "Method already declared: %s::%s",
                                      preClass->m_name->data(),
                                      methName->data());
    }
    postponeCtor(is, func);
  }

  if (nonScalarPinitVec != NULL) {
    // Non-scalar property initializers require 86pinit() for run-time
    // initialization support.
    StringData* methName = m_unit.lookupLitstrStr("86pinit");
    Func* func = new Func(m_unit, methName, preClass);
    ASSERT(!(attr & VM::AttrTrait));
    preClass->addMethod(func);
    postponePinit(is, func, nonScalarPinitVec);
  }

  if (nonScalarSinitVec != NULL) {
    // Non-scalar property initializers require 86sinit() for run-time
    // initialization support.
    StringData* methName = m_unit.lookupLitstrStr("86sinit");
    Func* func = new Func(m_unit, methName, preClass);
    ASSERT(!(attr & VM::AttrTrait));
    preClass->addMethod(func);
    postponeSinit(is, func, nonScalarSinitVec);
  }

  if (nonScalarConstVec != NULL) {
    // Non-scalar constant initializers require 86cinit() for run-time
    // initialization support.
    StringData* methName = m_unit.lookupLitstrStr("86cinit");
    Func* func = new Func(m_unit, methName, preClass);
    ASSERT(!(attr & VM::AttrTrait));
    bool added = preClass->addMethod(func);
    if (!added) {
      throw IncludeTimeFatalException(is,
                                      "Method already declared: %s::%s",
                                      preClass->m_name->data(),
                                      methName->data());
    }
    postponeCinit(is, func, nonScalarConstVec);
  }
}

void EmitterVisitor::emitBreakHandler(Emitter &e, Label &brkTarg,
    Label &cntTarg, Label &brkHand, Label &cntHand, Id iter /* = -1 */) {

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

void EmitterVisitor::emitForeach(Emitter &e,
                                 ExpressionPtr val, ExpressionPtr key,
                                 StatementPtr body, bool strong) {
  Label exit;
  Label next;
  Label brkHand;
  Label cntHand;
  Id itId = m_curFunc->allocIterator();
  if (strong) {
    e.IterInitM(itId, exit);
  } else {
    e.IterInit(itId, exit);
  }
  Label start(e);
  Offset bIterStart = m_unit.bcPos();

  // The evaluation order for the beginning of each foreach iteration is
  // IterValue*, IterKey, the key expression, and finally the value expression.
  // In the general case we need to use unnamed locals to get evaluation order
  // right. Fortunately, in the common case the key and value expressions are
  // simple variables and so we can evaluate them out of order without causing
  // observable changes in program behavior and avoid the need for unnamed
  // locals.
  bool simpleCase = (!key || key->is(Expression::KindOfSimpleVariable)) &&
                    val->is(Expression::KindOfSimpleVariable);

  if (simpleCase) {
    if (key) {
      visit(key);
    }
    visit(val);
    if (strong) {
      e.IterValueV(itId);
      e.BindH();
    } else {
      e.IterValueC(itId);
      e.SetH();
    }
    emitPop(e);
    if (key) {
      e.IterKey(itId);
      emitSet(e);
      emitPop(e);
    }
  } else {
    // Allocate unnamed locals keyTempLocal (if there is a key
    // expression) and valTempLocal
    int keyTempLocal = key ? m_curFunc->allocUnnamedLocal() : -1;
    int valTempLocal = m_curFunc->allocUnnamedLocal();
    // Evaluate IterValue* and stash the result in valTempLocal
    if (key) {
      e.Loc(keyTempLocal);
    }
    e.Loc(valTempLocal);
    if (strong) {
      e.IterValueV(itId);
      e.BindH();
    } else {
      e.IterValueC(itId);
      e.SetH();
    }
    emitPop(e);
    if (key) {
      // Evaluate IterKey and stash the result in keyTempLocal
      e.IterKey(itId);
      e.SetH();
      e.PopC();
      // Evaluate the key expression
      visit(key);
    }
    // Evaluate the value expression
    visit(val);
    // Push valTempLocal onto the stack
    e.Loc(valTempLocal);
    if (strong) {
      e.VGetH();
    } else {
      e.CGetH();
    }
    // Unset valTempLocal
    e.Loc(valTempLocal);
    e.UnsetH();
    // We're done with valTempLocal, set up a fault region
    // for it and free it
    newFaultRegion(bIterStart, m_unit.bcPos(),
                   new UnsetUnnamedLocalThunklet(valTempLocal));
    m_curFunc->freeUnnamedLocal(valTempLocal);
    // Do the assignment for the value
    if (strong) {
      emitBind(e);
    } else {
      emitSet(e);
    }
    emitPop(e);
    if (key) {
      ASSERT(keyTempLocal != -1);
      // Push keyTempLocal onto the stack
      e.Loc(keyTempLocal);
      e.CGetH();
      // Unset keyTempLocal
      e.Loc(keyTempLocal);
      e.UnsetH();
      // We're done with keyTempLocal, set up a fault region
      // for it and free it
      newFaultRegion(bIterStart, m_unit.bcPos(),
                     new UnsetUnnamedLocalThunklet(keyTempLocal));
      m_curFunc->freeUnnamedLocal(keyTempLocal);
      // Do the assignment for the key
      emitSet(e);
      emitPop(e);
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
  e.IterNext(itId, start);
  // Set up a fault region and FE region
  newFaultRegion(bIterStart, m_unit.bcPos(), new IterFreeThunklet(itId));
  newForEachRegion(bIterStart, m_unit.bcPos(), itId);
  if (needBreakHandler) {
    e.Jmp(exit);
    emitBreakHandler(e, exit, next, brkHand, cntHand, itId);
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
  StringData* funcName = m_unit.lookupLitstrStr("error_reporting");
  Label dontRollback;
  // Optimistically call with the old value.  If this returns nonzero, call it
  // again with that return value.
  e.Loc(oldLevelLoc);
  {
    e.FPushFuncD(1, funcName);
    FPIRegionRecorder fpi(this, m_unit, m_evalStack);
    e.Loc(oldLevelLoc);
    e.CGetH();
    e.FPassC(0);
    e.FCall(1);
  }
  e.UnboxR();
  // stack is now: ...[Loc oldLevelLoc][return value]
  e.SetH();  // save the return value in local, and leave it on the stack
  e.Int(0);
  e.Eq();
  e.JmpNZ(dontRollback);
  {
    e.FPushFuncD(1, funcName);
    FPIRegionRecorder fpi(this, m_unit, m_evalStack);
    e.Loc(oldLevelLoc);
    e.CGetH();
    e.FPassC(0);
    e.FCall(1);
  }
  e.PopR();
  dontRollback.set(e);
}

void EmitterVisitor::emitMakeUnitFatal(Emitter& e, const std::string& msg) {
  StringData* sd = m_unit.lookupLitstrStr(msg);
  e.String(sd);
  e.Fatal();
}

void EmitterVisitor::addFunclet(Thunklet *body, Label *entry) {
  m_funclets.push_back(Funclet(body, entry));
}
void EmitterVisitor::emitFunclets(Emitter &e) {
  while (!m_funclets.empty()) {
    Funclet &f = m_funclets.front();
    f.m_entry->set(e);
    f.m_body->emit(e);
    delete f.m_body;
    m_funclets.pop_front();
  }
  m_funclets.clear();
}

void EmitterVisitor::newFaultRegion(Offset start, Offset end, Thunklet *t) {
  FaultRegion *r = new FaultRegion(start, end);
  m_faultRegions.push_back(r);
  addFunclet(t, &r->m_func);
}

void EmitterVisitor::newForEachRegion(Offset start, Offset end, Id iterId) {
  ForEachRegion *r = new ForEachRegion(start, end, iterId);
  m_feRegions.push_back(r);
}

void EmitterVisitor::newFPIRegion(Offset start, Offset end, Offset fpOff) {
  FPIRegion *r = new FPIRegion(start, end, fpOff);
  m_fpiRegions.push_back(r);
}

void EmitterVisitor::copyOverExnHandlers(Func *f) {
  for (std::deque<ExnHandlerRegion*>::const_iterator it = m_exnHandlers.begin();
       it != m_exnHandlers.end(); ++it) {
    EHEnt &e = f->addEHEnt();
    e.m_ehtype = EHEnt::EHType_Catch;
    e.m_base = (*it)->m_start;
    e.m_past = (*it)->m_end;
    for (std::vector<std::pair<StringData*, Label*> >::const_iterator it2
           = (*it)->m_catchLabels.begin();
         it2 != (*it)->m_catchLabels.end(); ++it2) {
      Id id = m_unit.mergeLitstr(it2->first);
      Offset off = it2->second->getAbsoluteOffset();
      e.m_catches.push_back(std::pair<Id, Offset>(id, off));
    }
    delete *it;
  }
  m_exnHandlers.clear();
  for (std::deque<FaultRegion*>::iterator it = m_faultRegions.begin();
      it != m_faultRegions.end(); ++it) {
    EHEnt &e = f->addEHEnt();
    e.m_ehtype = EHEnt::EHType_Fault;
    e.m_base = (*it)->m_start;
    e.m_past = (*it)->m_end;
    e.m_fault = (*it)->m_func.getAbsoluteOffset();
    delete *it;
  }
  m_faultRegions.clear();
  // sort exnHandlers
  std::sort(f->m_ehtab.begin(), f->m_ehtab.end(), EHEntComp());
  for (unsigned int i = 0; i < f->m_ehtab.size(); i++) {
    f->m_ehtab[i].m_parentIndex = -1;
    for (int j = i - 1; j >= 0; j--) {
      if (f->m_ehtab[j].m_past > f->m_ehtab[i].m_past) {
        f->m_ehtab[i].m_parentIndex = j;
        break;
      }
    }
  }
}

void EmitterVisitor::copyOverFERegions(Func *f) {
  for (std::deque<ForEachRegion*>::iterator it = m_feRegions.begin();
       it != m_feRegions.end(); ++it) {
    FEEnt &e = f->addFEEnt();
    e.m_base = (*it)->m_start;
    e.m_past = (*it)->m_end;
    e.m_iterId = (*it)->m_iterId;
    delete *it;
  }
  m_feRegions.clear();
  // Sort it and fill in parent info
  std::sort(f->m_fetab.begin(), f->m_fetab.end(), FEEntComp());
  for (unsigned int i = 0; i < f->m_fetab.size(); i++) {
    f->m_fetab[i].m_parentIndex = -1;
    for (int j = i - 1; j >= 0; j--) {
      if (f->m_fetab[j].m_past > f->m_fetab[i].m_past) {
        f->m_fetab[i].m_parentIndex = j;
        break;
      }
    }
  }
}

void EmitterVisitor::copyOverFPIRegions(Func *f) {
  for (std::deque<FPIRegion*>::iterator it = m_fpiRegions.begin();
       it != m_fpiRegions.end(); ++it) {
    FPIEnt &e = f->addFPIEnt();
    e.m_base = (*it)->m_start;
    e.m_past = (*it)->m_end;
    e.m_fpOff = (*it)->m_fpOff;
    delete *it;
  }
  m_fpiRegions.clear();
  // Sort it and fill in parent info
  std::sort(f->m_fpitab.begin(), f->m_fpitab.end(), FPIEntComp());
  for (unsigned int i = 0; i < f->m_fpitab.size(); i++) {
    f->m_fpitab[i].m_parentIndex = -1;
    f->m_fpitab[i].m_fpiDepth = 1;
    for (int j = i - 1; j >= 0; j--) {
      if (f->m_fpitab[j].m_past > f->m_fpitab[i].m_past) {
        f->m_fpitab[i].m_parentIndex = j;
        f->m_fpitab[i].m_fpiDepth = f->m_fpitab[j].m_fpiDepth + 1;
        break;
      }
    }
    /* m_fpOff does not include the space taken up by locals, iterators
     * and the AR itself. Fix it here.
     */
    f->m_fpitab[i].m_fpOff += f->m_numLocals
      + f->m_numIterators * kNumIterCells
      + (f->m_fpitab[i].m_fpiDepth) * kNumActRecCells;
  }
}

void EmitterVisitor::saveMaxStackCells(Func* f) {
  f->setMaxStackCells(m_actualStackHighWater + kNumActRecCells
                      + m_fdescHighWater);
  m_actualStackHighWater = 0;
  m_fdescHighWater = 0;
}

// Are you sure you mean to be calling this directly? Would FuncFinisher
// be more appropriate?
void EmitterVisitor::finishFunc(Emitter& e, Func* f,
                                Offset funclets /* = InvalidAbsoluteOffset */) {
  if (funclets == InvalidAbsoluteOffset) {
    funclets = e.getUnit().bcPos();
  }
  emitFunclets(e);
  saveMaxStackCells(f);
  copyOverExnHandlers(f);
  copyOverFERegions(f);
  copyOverFPIRegions(f);
  m_gotoLabels.clear();
  Offset past = e.getUnit().bcPos();
  f->finish(funclets, past);
  e.getUnit().recordFunction(f);
}

StringData* EmitterVisitor::newClosureName() {
  std::ostringstream str;
  str << "closure$";
  if (m_curFunc->m_preClass != NULL) {
    str << m_curFunc->m_preClass->m_name->data();
  }
  str << "$";
  if (m_curFunc->isPseudoMain()) {
    // Pseudo-main. Encode unit pointer to uniquify.
    str << "__pseudoMain" << std::hex << m_curFunc->m_unit << std::dec;
  } else {
    str << m_curFunc->m_name->data();
  }
  str << "$" << m_closureCounter++;

  return StringData::GetStaticString(str.str());
}

void EmitterVisitor::initScalar(TypedValue& tvVal, ExpressionPtr val) {
  ASSERT(val->isScalar());
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
      const std::string *s;
      if (sval->getString(s)) {
        StringData* sd = StringData::GetStaticString(
          NEW(StringData)(s->c_str(), s->size(), CopyString));
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
        va = static_cast<HphpArray*>(Unit::mergeAnonArray(va));

        tvVal.m_data.parr = va;
        tvVal.m_type = KindOfArray;

        if (a->decRefCount() == 0) {
          a->release();
        }
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
      assert(false);
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
        return e->preOptimize(ar);

      default: break;
    }
  }
  return ConstructPtr();
}


static Unit* emitHHBCUnit(AnalysisResultPtr ar, FileScopePtr fsp) {
  if (fsp->getPseudoMain()) {
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
  Unit* unit = new Unit(msp->getLocation().get());
  EmitterVisitor ev(*unit);
  try {
    ev.visit(fsp);
  } catch (EmitterVisitor::IncludeTimeFatalException& ex) {
    // Replace the unit with an empty one, but preserve its file path.
    Unit* newUnit = new Unit(msp->getLocation().get());
    newUnit->m_filepath = unit->m_filepath;
    newUnit->m_dirpath = unit->m_dirpath;
    delete unit;
    unit = newUnit;

    EmitterVisitor fev(*unit);
    Emitter emitter(ex.m_node, *unit, fev);
    FuncFinisher ff(&fev, emitter, unit->getMain());
    fev.emitMakeUnitFatal(emitter, ex.getMessage());
  }

  if (RuntimeOption::EvalPeephole) {
    // Run the peephole optimizer.
    Peephole peephole(*unit);
  }
  if (RuntimeOption::EvalDumpBytecode) {
    // Dump human-readable bytecode.
    std::cout << unit->toString();
  }

  unit->close();
  return unit;
}

/**
 * This is the entry point for offline bytecode generation.
 */
void emitHHBCVisitor(AnalysisResultPtr ar, StatementPtr sp, void* data) {
  CodeGenerator::Output fmt = *(CodeGenerator::Output*)data;
  if (fmt != CodeGenerator::TextHHBC) {
    // Need a way to serialize HHBC
    not_implemented();
  }

  FileScopePtr fsp = sp->getFileScope();
  const char* filename = fsp->getName().c_str();
  std::string fullPath = AnalysisResult::prepareFile(
    ar->getOutputPath().c_str(), filename, true);
  fullPath += ".hhbc.txt";
  std::ofstream f(fullPath.c_str());
  if (f) {
    HPHP::VM::Unit* unit = emitHHBCUnit(ar, fsp);
    CodeGenerator cg(&f, fmt);
    cg.printRaw(unit->toString().c_str());
    f.close();
    delete unit;
  } else {
    Logger::Error("Unable to open %s for write", fullPath.c_str());
  }
}


/**
 * This is the entry point from the runtime; i.e. online bytecode generation.
 * The 'filename' parameter may be NULL if there is no file associated with
 * the source code.
 */

extern "C" {

Unit* hphp_compiler_parse(const char* code, int codeLen,
                          const char* filename) {
  ScopeGuard sg(SymbolTable::Purge);
  bool save = Option::ParseTimeOpts;
  Option::EnableHipHopSyntax = RuntimeOption::EnableHipHopSyntax;
  try {
    Option::ParseTimeOpts = false;

    AnalysisResultPtr ar(new AnalysisResult());
    Scanner scanner(code, codeLen, RuntimeOption::ScannerType);
    Parser parser(scanner, filename, ar, codeLen);
    if (!parser.parse()) {
      Option::ParseTimeOpts = save;
      std::string msg = "Parse error: ";
      msg += parser.errString();
      VMParserFrame parserFrame;
      parserFrame.filename = filename;
      parserFrame.lineNumber = parser.line1();
      ArrayPtr bt =
        ArrayPtr(new Array(
          g_context->debugBacktrace(false, true, false, &parserFrame)));
      throw FatalErrorException(msg, bt);
    }
    FileScopePtr fsp = parser.getFileScope();

    ar->setPhase(AnalysisResult::AnalyzeAll);
    fsp->analyzeProgram(ar);

    Option::ParseTimeOpts = save;
    return emitHHBCUnit(ar, fsp);
  } catch (...) {
    Option::ParseTimeOpts = save;
    throw;
  }
}

} // extern "C"

///////////////////////////////////////////////////////////////////////////////
}
}
