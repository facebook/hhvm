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

#include "hphp/runtime/vm/verifier/check.h"

#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/vm/native.h"

#include "hphp/runtime/vm/verifier/cfg.h"
#include "hphp/runtime/vm/verifier/util.h"
#include "hphp/runtime/vm/verifier/pretty.h"

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <stdexcept>

namespace HPHP {
namespace Verifier {

/**
 * State for one entry on the FPI stack which corresponds to one call-site
 * in progress.
 */
struct FpiState {
  Offset fpush;   // offset of fpush (can get num_params from this)
  int stkmin;     // stklen before FPush
  int next;       // next expected param number
  bool operator==(const FpiState& s) const {
    return fpush == s.fpush && stkmin == s.stkmin && next == s.next;
  }
  bool operator!=(const FpiState& s) const {
    return !(*this == s);
  }
};

/**
 * Facts about a Func's current frame at various program points
 */
struct State {
  FlavorDesc* stk; // Evaluation stack.
  FpiState* fpi;    // FPI stack.
  bool* iters;      // defined/not-defined state of each iter var.
  int stklen;       // length of evaluation stack.
  int fpilen;       // length of FPI stack.
  bool mbr_live;    // liveness of member base register
};

/**
 * Facts about a specific block.
 */
struct BlockInfo {
  State state_in;  // state at the start of the block
};

struct FuncChecker {
  FuncChecker(const Func* func, bool verbose);
  bool checkOffsets();
  bool checkFlow();

 private:
  struct unknown_length : std::runtime_error {
    unknown_length()
      : std::runtime_error("Unknown instruction length")
    {}
  };

  bool checkEdge(Block* b, const State& cur, Block* t);
  bool checkSuccEdges(Block* b, State* cur);
  bool checkOffset(const char* name, Offset o, const char* regionName,
                   Offset base, Offset past, bool check_instrs = true);
  bool checkRegion(const char* name, Offset b, Offset p,
                   const char* regionName, Offset base, Offset past,
                   bool check_instrs = true);
  bool checkSection(bool main, const char* name, Offset base, Offset past);
  bool checkImmediates(const char* name, PC instr);
  bool checkImmVec(PC& pc, size_t elemSize);
#define ARGTYPE(name, type) bool checkImm##name(PC& pc, PC instr);
#define ARGTYPEVEC(name, type) ARGTYPE(name, type)
  ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
  template<typename Subop> bool checkImmOAImpl(PC& pc, PC instr);
  bool checkInputs(State* cur, PC, Block* b);
  bool checkOutputs(State* cur, PC, Block* b);
  bool checkSig(PC pc, int len, const FlavorDesc* args, const FlavorDesc* sig);
  bool checkEHStack(const EHEnt&, Block* b);
  bool checkTerminal(State* cur, PC pc);
  bool checkFpi(State* cur, PC pc, Block* b);
  bool checkIter(State* cur, PC pc);
  bool checkLocal(PC pc, int val);
  bool checkString(PC pc, Id id);
  void reportStkUnderflow(Block*, const State& cur, PC);
  void reportStkOverflow(Block*, const State& cur, PC);
  void reportStkMismatch(Block* b, Block* target, const State& cur);
  void reportEscapeEdge(Block* b, Block* s);
  std::string stateToString(const State& cur);
  std::string sigToString(int len, const FlavorDesc* sig);
  std::string stkToString(int len, const FlavorDesc* args);
  std::string fpiToString(const FpiState&);
  std::string iterToString(const State& cur);
  void copyState(State* to, const State* from);
  void initState(State* s);
  const FlavorDesc* sig(PC pc);
  Offset offset(PC pc) const { return pc - unit()->entry(); }
  PC at(Offset off) const { return unit()->at(off); }
  int maxStack() const { return m_func->maxStackCells(); }
  int maxFpi() const { return m_func->fpitab().size(); }
  int numIters() const { return m_func->numIterators(); }
  int numLocals() const { return m_func->numLocals(); }
  int numParams() const { return m_func->numParams(); }
  const Unit* unit() const { return m_func->unit(); }

 private:
  template<class... Args>
  void error(const char* fmt, Args&&... args) {
    verify_error(unit(), m_func, fmt, std::forward<Args>(args)...);
  }
  template<class... Args>
  void ferror(Args&&... args) {
    verify_error(unit(), m_func, "%s",
                 folly::sformat(std::forward<Args>(args)...).c_str());
  }

 private:
  Arena m_arena;
  BlockInfo* m_info; // one per block
  const Func* const m_func;
  Graph* m_graph;
  Bits m_instrs;
  bool m_verbose;
  FlavorDesc* m_tmp_sig;
};

bool checkNativeFunc(const Func* func, bool verbose) {
  auto const funcname = func->name();
  auto const pc = func->preClass();
  auto const clsname = pc ? pc->name() : nullptr;
  auto const& info = Native::GetBuiltinFunction(funcname, clsname,
                                                func->isStatic());

  if (func->builtinFuncPtr() == Native::unimplementedWrapper) return true;

  if (func->isAsync()) {
    verify_error(func->unit(), func,
      "<<__Native>> function %s%s%s is declared async; <<__Native>> functions "
      "can return Awaitable<T>, but can not be declared async.\n",
      clsname ? clsname->data() : "",
      clsname ? "::" : "",
      funcname->data()
    );
    return false;
  }

  auto const& tc = func->returnTypeConstraint();
  auto const message = Native::checkTypeFunc(info.sig, tc, func);

  if (message) {
    auto const tstr = info.sig.toString(clsname ? clsname->data() : nullptr,
                                        funcname->data());

    verify_error(func->unit(), func,
      "<<__Native>> function %s%s%s does not match C++ function "
      "signature (%s): %s\n",
      clsname ? clsname->data() : "",
      clsname ? "::" : "",
      funcname->data(),
      tstr.c_str(),
      message
    );
    return false;
  }

  return true;
}

bool checkFunc(const Func* func, bool verbose) {
  if (verbose) {
    func->prettyPrint(std::cout);
    if (func->cls() || !func->preClass()) {
      printf("  FuncId %d\n", func->getFuncId());
    }
    printFPI(func);
  }
  FuncChecker v(func, verbose);
  return v.checkOffsets() &&
         v.checkFlow();
}

FuncChecker::FuncChecker(const Func* f, bool verbose)
: m_func(f)
, m_graph(0)
, m_instrs(m_arena, f->past() - f->base() + 1)
, m_verbose(verbose) {
}

// Needs to be a sorted map so we can divide funcs into contiguous sections.
using SectionMap = std::map<Offset,Offset>;

/**
 * Return the start offset of the nearest enclosing section.  Caller must
 * ensure that off is at least within the entire func's bytecode region.
 */
Offset findSection(SectionMap& sections, Offset off) {
  assert(!sections.empty());
  SectionMap::iterator i = sections.upper_bound(off);
  --i;
  return i->first;
}

/**
 * Make sure all offsets are in-bounds.  Offset 0 is unit->m_bc,
 * for all functions.  Jump instructions use an Offset relative to
 * the start of the jump instruction.
 */
bool FuncChecker::checkOffsets() {
  bool ok = true;
  assert(unit()->bclen() >= 0);
  PC bc = unit()->entry();
  Offset base = m_func->base();
  Offset past = m_func->past();
  checkRegion("func", base, past, "unit", 0, unit()->bclen(), false);
  // find instruction boundaries and make sure no branches escape
  SectionMap sections;
  for (auto& eh : m_func->ehtab()) {
    if (eh.m_type == EHEnt::Type::Fault) {
      ok &= checkOffset("fault funclet", eh.m_fault, "func bytecode", base,
                        past, false);
      sections[eh.m_fault] = 0;
    }
  }
  Offset funclets = !sections.empty() ? sections.begin()->first : past;
  sections[base] = funclets; // primary body
  // Get instruction boundaries and check branches within primary body
  // and each faultlet.
  for (auto i = sections.begin(), end = sections.end(); i != end;) {
    Offset section_base = i->first; ++i;
    Offset section_past = i == end ? past : i->first;
    sections[section_base] = section_past;
    ok &= checkSection(section_base == base,
                       section_base == base ? "primary body" : "funclet body",
                       section_base, section_past);
  }
  // DV entry points must be in the primary function body
  for (auto& param : m_func->params()) {
    if (param.hasDefaultValue()) {
      ok &= checkOffset("dv-entry", param.funcletOff, "func body", base,
                        funclets);
    }
  }
  // Every FPI region must be contained within one section, either the
  // primary body or one fault funclet
  for (auto& fpi : m_func->fpitab()) {
    Offset fpi_base = fpiBase(fpi, bc);
    Offset fpi_past = fpiPast(fpi, bc);
    if (checkRegion("fpi", fpi_base, fpi_past, "func", base, past)) {
      // FPI is within whole func, but we also need to check within the section
      Offset section_base = findSection(sections, fpi_base);
      Offset section_past = sections[section_base];
      ok &= checkRegion("fpi", fpi_base, fpi_past,
                        section_base == base ?  "func body" : "funclet",
                        section_base, section_past);
    } else {
      ok = false;
    }
  }
  // check EH regions and targets
  for (auto& eh : m_func->ehtab()) {
    if (eh.m_type == EHEnt::Type::Fault) {
      ok &= checkOffset("fault", eh.m_fault, "funclets", funclets, past);
    }
  }
  return ok;
}

/**
 * Scan instructions in the given section to find valid instruction
 * boundaries, and check that branches a) land on valid boundaries,
 * b) do not escape the section.
 */
bool FuncChecker::checkSection(bool is_main, const char* name, Offset base,
                               Offset past) {
  bool ok = true;
  typedef std::list<PC> BranchList;
  BranchList branches;
  PC bc = unit()->entry();
  // Find instruction boundaries and branch instructions.
  for (InstrRange i(at(base), at(past)); !i.empty();) {
    auto pc = i.popFront();
    auto const op = peek_op(pc);
    if (!checkImmediates(name, pc)) {
      ferror("checkImmediates failed for {} @ {}\n",
             opcodeToName(op), offset(pc));
      return false;
    }
    m_instrs.set(offset(pc) - m_func->base());
    if (isSwitch(op) ||
        instrJumpTarget(bc, offset(pc)) != InvalidAbsoluteOffset) {
      if (op == OpSwitch && getImm(pc, 2).u_IVA != 0) {
        int64_t switchBase = getImm(pc, 1).u_I64A;
        int32_t len = getImmVector(pc).size();
        if (len <= 2) {
          error("Bounded switch must have a vector of length > 2 [%d:%d]\n",
                base, past);
        }
        if (switchBase > std::numeric_limits<int64_t>::max() - len + 2) {
          error("Overflow in Switch bounds [%d:%d]\n", base, past);
        }
      } else if (op == Op::SSwitch) {
        foreachSwitchString(pc, [&](Id id) {
          ok &= checkString(pc, id);
        });
      }
      branches.push_back(pc);
    }
    if (i.empty()) {
      if (offset(pc + instrLen(pc)) != past) {
        error("Last instruction in %s at %d overflows [%d:%d]\n",
               name, offset(pc), base, past);
        ok = false;
      }
      if ((instrFlags(op) & TF) == 0) {
        error("Last instruction in %s is not terminal %d:%s\n",
               name, offset(pc), instrToString(pc, unit()).c_str());
        ok = false;
      } else {
        if (isRet(pc) && !is_main) {
          error("Ret* may not appear in %s\n", name);
          ok = false;
        } else if (op == Op::Unwind && is_main) {
          error("Unwind may not appear in %s\n", name);
          ok = false;
        }
      }
    }
  }
  // Check each branch target lands on a valid instruction boundary
  // within this region.
  for (auto branch : branches) {
    if (isSwitch(peek_op(branch))) {
      foreachSwitchTarget(branch, [&](Offset o) {
        // TODO(#2464197): dce breaks switch for verify
        if (offset(branch + o) != -1) {
          ok &= checkOffset("switch target", offset(branch + o),
                            name, base, past);
        }
      });
    } else {
      Offset target = instrJumpTarget(bc, offset(branch));
      ok &= checkOffset("branch target", target, name, base, past);
      if (peek_op(branch) == Op::JmpNS && target == offset(branch)) {
        error("JmpNS may not have zero offset in %s\n", name);
        ok = false;
      }
    }
  }
  return ok;
}

Id decodeId(PC* ppc) {
  Id id = *(Id*)*ppc;
  *ppc += sizeof(Id);
  return id;
}

Offset decodeOffset(PC* ppc) {
  Offset offset = *(Offset*)*ppc;
  *ppc += sizeof(Offset);
  return offset;
}

bool FuncChecker::checkLocal(PC pc, int k) {
  if (k < 0 || k >= numLocals()) {
    error("invalid local variable id %d at Offset %d\n",
           k, offset(pc));
    return false;
  }
  return true;
}

bool FuncChecker::checkString(PC pc, Id id) {
  return unit()->isLitstrId(id);
}

bool FuncChecker::checkImmVec(PC& pc, size_t elemSize) {
  auto const len = decode_raw<int32_t>(pc);
  if (len < 1) {
    error("invalid length of immediate vector %d at Offset %d\n",
          len, offset(pc));
    throw unknown_length{};
  }

  pc += len * elemSize;
  return true;
}

bool FuncChecker::checkImmBLA(PC& pc, PC const instr) {
  return checkImmVec(pc, sizeof(Offset));
}

bool FuncChecker::checkImmSLA(PC& pc, PC const instr) {
  return checkImmVec(pc, sizeof(Id) + sizeof(Offset));
}

bool FuncChecker::checkImmILA(PC& pc, PC const instr) {
  return checkImmVec(pc, sizeof(Id) + sizeof(Id));
}

bool FuncChecker::checkImmIVA(PC& pc, PC const instr) {
  auto const k = decode_iva(pc);
  if (peek_op(instr) == Op::ConcatN) {
    return k >= 2 && k <= kMaxConcatN;
  }

  return true;
}

bool FuncChecker::checkImmI64A(PC& pc, PC const instr) {
  pc += sizeof(int64_t);
  return true;
}

bool FuncChecker::checkImmLA(PC& pc, PC const instr) {
  auto ok = true;
  auto const k = decode_iva(pc);
  ok &= checkLocal(pc, k);
  if (peek_op(instr) == Op::VerifyParamType) {
    if (k >= numParams()) {
      error("invalid parameter id %d at %d\n", k, offset(instr));
      ok = false;
    }
  }

  return ok;
}

bool FuncChecker::checkImmIA(PC& pc, PC const instr) {
  auto const k = decode_iva(pc);
  if (k >= numIters()) {
    error("invalid iterator variable id %d at %d\n", k, offset(instr));
    return false;
  }
  return true;
}

bool FuncChecker::checkImmDA(PC& pc, PC const instr) {
  pc += sizeof(double);
  return true;
}

bool FuncChecker::checkImmSA(PC& pc, PC const instr) {
  auto const id = decodeId(&pc);
  return checkString(pc, id);
}

bool FuncChecker::checkImmAA(PC& pc, PC const instr) {
  auto const id = decodeId(&pc);
  if (id < 0 || id >= (Id)unit()->numArrays()) {
    error("invalid array id %d\n", id);
    return false;
  }
  return true;
}

bool FuncChecker::checkImmRATA(PC& pc, PC const instr) {
  // Nothing to check at the moment.
  pc += encodedRATSize(pc);
  return true;
}

bool FuncChecker::checkImmBA(PC& pc, PC const instr) {
  // we check branch offsets in checkSection(). ignore here.
  assert(instrJumpTarget(unit()->entry(), offset(instr)) !=
         InvalidAbsoluteOffset);
  pc += sizeof(Offset);
  return true;
}

bool FuncChecker::checkImmVSA(PC& pc, PC const instr) {
  auto const len = decode_raw<int32_t>(pc);
  if (len < 1 || len > StructArray::MaxMakeSize) {
    error("invalid length of immedate VSA vector %d at offset %d\n",
          len, offset(pc));
    throw unknown_length{};
  }

  auto ok = true;
  for (int i = 0; i < len; i++) {
    auto const id = decodeId(&pc);
    ok &= checkString(pc, id);
  }
  return ok;
}

template<typename Subop>
bool FuncChecker::checkImmOAImpl(PC& pc, PC const instr) {
  auto const subop = decode_oa<Subop>(pc);
  if (!subopValid(subop)) {
    ferror("Invalid subop {}\n", size_t(subop));
    return false;
  }
  return true;
}

bool FuncChecker::checkImmKA(PC& pc, PC const instr) {
  auto const mcode = decode_raw<MemberCode>(pc);
  if (mcode < 0 || mcode >= NumMemberCodes) {
    ferror("Invalid MemberCode {}\n", uint8_t{mcode});
    return false;
  }

  auto ok = true;
  switch (mcode) {
    case MW:
      break;
    case MEL: case MPL: {
      auto const loc = decode_iva(pc);
      ok &= checkLocal(pc, loc);
      break;
    }
    case MEC: case MPC:
      decode_iva(pc);
      break;
    case MEI:
      pc += sizeof(int64_t);
      break;
    case MET: case MPT: case MQT:
      auto const id = decode_raw<Id>(pc);
      ok &= checkString(pc, id);
      break;
  }

  return ok;
}

/**
 * Check instruction and its immediates. Returns false if we can't continue
 * because we don't know the length of this instruction or one of the
 * immediates was invalid.
 */
bool FuncChecker::checkImmediates(const char* name, PC const instr) {
  auto pc = instr;

  auto const op = decode_op(pc);
  if (!isValidOpcode(op)) {
    error("Invalid opcode %d in section %s at offset %d\n",
          size_t(op), name, offset(instr));
    return false;
  }
  bool ok = true;
  try {
    switch (op) {
#define NA
#define checkImmOA(ty) checkImmOAImpl<ty>
#define ONE(a) ok &= checkImm##a(pc, instr)
#define TWO(a, b) ONE(a); ok &= checkImm##b(pc, instr)
#define THREE(a, b, c) TWO(a, b); ok &= checkImm##c(pc, instr)
#define FOUR(a, b, c, d) THREE(a, b, c); ok &= checkImm##d(pc, instr)
#define O(name, imm, in, out, flags) case Op::name: imm; break;
      OPCODES
#undef NA
#undef checkOA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef O
    }
  } catch (const unknown_length&) {
    return false;
  }

  auto const expectPC = instr + instrLen(instr);
  if (pc != expectPC) {
    ferror("PC after decoding {} at {} is {} instead of expected {}\n",
           opcodeToName(op), instr, pc, expectPC);
    return false;
  }
  return ok;
}

static const char* stkflav(FlavorDesc f) {
  switch (f) {
  case NOV:  return "N";
  case CV:   return "C";
  case VV:   return "V";
  case AV:   return "A";
  case RV:   return "R";
  case FV:   return "F";
  case UV:   return "U";
  case CRV:  return "C|R";
  case CUV:  return "C|U";
  case CVV:  return "C|V";
  case CVUV: return "C|V|U";
  }
  not_reached();
}

static bool checkArg(FlavorDesc expect, FlavorDesc check) {
  if (expect == check) return true;

  switch (expect) {
    case CVV:  return check == CV || check == VV;
    case CRV:  return check == CV || check == RV;
    case CUV:  return check == CV || check == UV;
    case CVUV: return check == CV || check == VV || check == UV || check == CUV;
    default:   return false;
  }
}

bool FuncChecker::checkSig(PC pc, int len, const FlavorDesc* args,
                           const FlavorDesc* sig) {
  for (int i = 0; i < len; ++i) {
    if (!checkArg(sig[i], args[i])) {
      error("flavor mismatch at %d, got %s expected %s\n",
             offset(pc), stkToString(len, args).c_str(),
             sigToString(len, sig).c_str());
      return false;
    }
  }
  return true;
}

const FlavorDesc* FuncChecker::sig(PC pc) {
  static const FlavorDesc inputSigs[][4] = {
  #define NOV { },
  #define FMANY { },
  #define CVUMANY { },
  #define CMANY { },
  #define SMANY { },
  #define ONE(a) { a },
  #define TWO(a,b) { b, a },
  #define THREE(a,b,c) { c, b, a },
  #define FOUR(a,b,c,d) { d, c, b, a },
  #define MFINAL { },
  #define F_MFINAL { },
  #define C_MFINAL { },
  #define V_MFINAL { },
  #define IDX_A { },
  #define O(name, imm, pop, push, flags) pop
    OPCODES
  #undef O
  #undef MFINAL
  #undef F_MFINAL
  #undef C_MFINAL
  #undef V_MFINAL
  #undef FMANY
  #undef CVUMANY
  #undef CMANY
  #undef SMANY
  #undef FOUR
  #undef THREE
  #undef TWO
  #undef ONE
  #undef NOV
  #undef IDX_A
  };
  switch (peek_op(pc)) {
  case Op::QueryM:
  case Op::VGetM:
  case Op::FPassM:
  case Op::IncDecM:
  case Op::UnsetM:
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CRV;
    }
    return m_tmp_sig;
  case Op::SetM:
  case Op::SetOpM:
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = i == n - 1 ? CV : CRV;
    }
    return m_tmp_sig;
  case Op::BindM:
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = i == n - 1 ? VV : CRV;
    }
    return m_tmp_sig;
  case Op::FCall:       // ONE(IVA),            FMANY,   ONE(RV)
  case Op::FCallD:      // THREE(IVA,SA,SA),    FMANY,   ONE(RV)
  case Op::FCallAwait:  // THREE(IVA,SA,SA),    FMANY,   ONE(CV)
  case Op::FCallUnpack: // ONE(IVA),            FMANY,   ONE(RV)
  case Op::FCallArray:  // NA,                  ONE(FV), ONE(RV)
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = FV;
    }
    return m_tmp_sig;
  case Op::FCallBuiltin: //TWO(IVA, SA), CVUMANY,  ONE(RV)
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CVUV;
    }
    return m_tmp_sig;
  case Op::CreateCl:  // TWO(IVA,SA),  CVUMANY,   ONE(CV)
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CVUV;
    }
    return m_tmp_sig;
  case Op::NewPackedArray:  // ONE(IVA),     CMANY,   ONE(CV)
  case Op::NewStructArray:  // ONE(VSA),     SMANY,   ONE(CV)
  case Op::ConcatN:         // ONE(IVA),     CMANY,   ONE(CV)
    for (int i = 0, n = instrNumPops(pc); i < n; ++i) {
      m_tmp_sig[i] = CV;
    }
    return m_tmp_sig;
  case Op::BaseSC:   // TWO(IVA, IVA),    IDX_A,           IDX_A
  case Op::BaseSL: { // TWO(LA, IVA),    IDX_A,           IDX_A
    auto const pops = instrNumPops(pc);
    assert(pops == 2 || pops == 1);
    if (pops == 1) {
      m_tmp_sig[0] = AV;
    } else {
      m_tmp_sig[0] = AV;
      m_tmp_sig[1] = CVV;
    }
    return m_tmp_sig;
  }
  default:
    return &inputSigs[size_t(peek_op(pc))][0];
  }
}

bool FuncChecker::checkInputs(State* cur, PC pc, Block* b) {
  StackTransInfo info = instrStackTransInfo(pc);
  int min = cur->fpilen > 0 ? cur->fpi[cur->fpilen - 1].stkmin : 0;
  if (info.numPops > 0 && cur->stklen - info.numPops < min) {
    reportStkUnderflow(b, *cur, pc);
    cur->stklen = 0;
    return false;
  }
  cur->stklen -= info.numPops;
  auto ok = checkSig(pc, info.numPops, &cur->stk[cur->stklen], sig(pc));
  auto const op = peek_op(pc);
  auto const need_live = isMemberDimOp(op) || isMemberFinalOp(op);
  auto const live_ok = need_live || isTypeAssert(op);
  if ((need_live && !cur->mbr_live) || (cur->mbr_live && !live_ok)) {
    error("Member base register %s coming into %s\n",
          cur->mbr_live ? "live" : "dead", opcodeToName(op));
    ok = false;
  }
  return ok;
}

bool FuncChecker::checkTerminal(State* cur, PC pc) {
  if (isRet(pc) || peek_op(pc) == Op::Unwind) {
    if (cur->stklen != 0) {
      error("stack depth must equal 0 after Ret* and Unwind; got %d\n",
             cur->stklen);
      return false;
    }
  }
  return true;
}

bool FuncChecker::checkFpi(State* cur, PC pc, Block* b) {
  if (cur->fpilen <= 0) {
    error("%s", "cannot access empty FPI stack\n");
    return false;
  }
  bool ok = true;
  FpiState& fpi = cur->fpi[cur->fpilen - 1];
  auto const op = peek_op(pc);
  if (isFCallStar(op)) {
    --cur->fpilen;
    int call_params = op == Op::FCallArray ? 1 : getImmIva(pc);
    int push_params = getImmIva(at(fpi.fpush));
    if (call_params != push_params) {
      error("FCall* param_count (%d) doesn't match FPush* (%d)\n",
             call_params, push_params);
      ok = false;
    }
    if (fpi.next != push_params) {
      error("wrong # of params were passed; got %d expected %d\n",
             fpi.next, push_params);
      ok = false;
    }
    if (cur->stklen != fpi.stkmin) {
      error("%s", "FCall didn't consume the proper param count\n");
      ok = false;
    }
  } else {
    // FPass*
    int param_id = getImmIva(pc);
    int push_params = getImmIva(at(fpi.fpush));
    if (param_id >= push_params) {
      error("param_id %d out of range [0:%d)\n", param_id,
             push_params);
      return false;
    }
    if (param_id != fpi.next) {
      error("FPass* out of order; got id %d expected %d\n",
             param_id, fpi.next);
      ok = false;
    }
    if (isMemberBaseOp(op) || isMemberDimOp(op)) {
      // The argument isn't pushed until the final member operation. Skip the
      // last two checks.
      return ok;
    }
    // we have already popped FPass's input, but not pushed the output, so this
    // check doesn't count the F result of this FPass, but does count the
    // previous FPass*s.
    if (cur->stklen != fpi.stkmin + param_id) {
      error("Stack depth incorrect after FPass; got %d expected %d\n",
             cur->stklen, fpi.stkmin + param_id);
      ok = false;
    }
    fpi.next++;
  }
  return ok;
}

bool FuncChecker::checkIter(State* cur, PC const pc) {
  assert(isIter(pc));
  int id = getImmIva(pc);
  bool ok = true;
  auto op = peek_op(pc);
  if (op == Op::IterInit || op == Op::IterInitK ||
      op == Op::WIterInit || op == Op::WIterInitK ||
      op == Op::MIterInit || op == Op::MIterInitK ||
      op == Op::DecodeCufIter) {
    if (cur->iters[id]) {
      error(
        "IterInit* or MIterInit* <%d> trying to double-initialize\n", id);
      ok = false;
    }
  } else {
    if (!cur->iters[id]) {
      // TODO(#2608280): we produce incorrect bytecode for iterators still
      //error("Cannot access un-initialized iter %d\n", id);
      //ok = false;
    }
    if (op == Op::IterFree ||
        op == Op::MIterFree ||
        op == Op::CIterFree) {
      cur->iters[id] = false;
    }
  }
  return ok;
}

bool FuncChecker::checkOutputs(State* cur, PC pc, Block* b) {
  static const FlavorDesc outputSigs[][4] = {
  #define NOV { },
  #define FMANY { },
  #define CMANY { },
  #define SMANY { },
  #define ONE(a) { a },
  #define TWO(a,b) { a, b },
  #define THREE(a,b,c) { a, b, c },
  #define FOUR(a,b,c,d) { a, b, c, d },
  #define INS_1(a) { a },
  #define INS_2(a) { a },
  #define IDX_A { },
  #define O(name, imm, pop, push, flags) push
    OPCODES
  #undef O
  #undef FMANY
  #undef CMANY
  #undef SMANY
  #undef INS_1
  #undef INS_2
  #undef FOUR
  #undef THREE
  #undef TWO
  #undef ONE
  #undef NOV
  #undef IDX_A
  };
  bool ok = true;
  auto const op = peek_op(pc);
  StackTransInfo info = instrStackTransInfo(pc);
  if (info.kind == StackTransInfo::Kind::InsertMid) {
    int index = cur->stklen - info.pos - 1;
    if (index < 0) {
      reportStkUnderflow(b, *cur, pc);
      return false;
    }
    memmove(&cur->stk[index + 1], &cur->stk[index],
           (info.pos + 1) * sizeof(*cur->stk));
    cur->stk[index] = outputSigs[size_t(op)][0];
    cur->stklen++;
  } else {
    int pushes = info.numPushes;
    if (cur->stklen + pushes > maxStack()) reportStkOverflow(b, *cur, pc);
    FlavorDesc *outs = &cur->stk[cur->stklen];
    cur->stklen += pushes;
    if (op == Op::BaseSC || op == Op::BaseSL) {
      if (pushes == 1) outs[0] = outs[1];
    } else {
      for (int i = 0; i < pushes; ++i) {
        outs[i] = outputSigs[size_t(op)][i];
      }
    }
    if (isFPush(op)) {
      if (cur->fpilen >= maxFpi()) {
        error("%s", "more FPush* instructions than FPI regions\n");
        return false;
      }
      FpiState& fpi = cur->fpi[cur->fpilen];
      cur->fpilen++;
      fpi.fpush = offset(pc);
      fpi.next = 0;
      fpi.stkmin = cur->stklen;
    }
  }

  if (isMemberBaseOp(op))       cur->mbr_live = true;
  else if (isMemberFinalOp(op)) cur->mbr_live = false;

  return ok;
}

std::string FuncChecker::stkToString(int len, const FlavorDesc* args) {
  std::stringstream out;
  out << '[';
  for (int i = 0; i < len; ++i) {
    out << stkflav(args[i]);
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::sigToString(int len, const FlavorDesc* sig) {
  std::stringstream out;
  out << '[';
  for (int i = 0; i < len; ++i) {
    out << stkflav(sig[i]);
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::iterToString(const State& cur) {
  int n = numIters();
  if (!n) return "";
  std::stringstream out;
  out << '[';
  for (int i = 0; i < n; ++i) {
    out << (cur.iters[i] ? '1' : '0');
  }
  out << ']';
  return out.str();
}

std::string FuncChecker::stateToString(const State& cur) {
  return iterToString(cur) + stkToString(cur.stklen, cur.stk);
}

std::string FuncChecker::fpiToString(const FpiState& fpi) {
  std::stringstream out;
  out << '(' << fpi.fpush << ':' << fpi.stkmin << ',' << fpi.next << ')';
  return out.str();
}

void FuncChecker::initState(State* s) {
  s->stk = new (m_arena) FlavorDesc[maxStack()];
  s->fpi = new (m_arena) FpiState[maxFpi()];
  s->iters = new (m_arena) bool[numIters()];
  for (int i = 0, n = numIters(); i < n; ++i) s->iters[i] = false;
  s->stklen = 0;
  s->fpilen = 0;
  s->mbr_live = false;
}

void FuncChecker::copyState(State* to, const State* from) {
  assert(from->stk);
  if (!to->stk) initState(to);
  memcpy(to->stk, from->stk, from->stklen * sizeof(*to->stk));
  memcpy(to->fpi, from->fpi, from->fpilen * sizeof(*to->fpi));
  memcpy(to->iters, from->iters, numIters() * sizeof(*to->iters));
  to->stklen = from->stklen;
  to->fpilen = from->fpilen;
  to->mbr_live = from->mbr_live;
}

/**
 * Verify stack depth along all control flow paths
 */
bool FuncChecker::checkFlow() {
  bool ok = true;
  GraphBuilder builder(m_arena, m_func);
  m_graph = builder.build();
  m_info = new (m_arena) BlockInfo[m_graph->block_count];
  memset(m_info, 0, sizeof(BlockInfo) * m_graph->block_count);
  m_tmp_sig = new (m_arena) FlavorDesc[maxStack()];
  sortRpo(m_graph);
  State cur;
  initState(&cur);
  // initialize state at all entry points
  for (BlockPtrRange i = entryBlocks(m_graph); !i.empty();) {
    ok &= checkEdge(0, cur, i.popFront());
  }
  for (Block* b = m_graph->first_rpo; b; b = b->next_rpo) {
    copyState(&cur, &m_info[b->id].state_in);
    if (m_verbose) {
      std::cout << blockToString(b, m_graph, unit()) << std::endl;
    }
    for (InstrRange i = blockInstrs(b); !i.empty(); ) {
      PC pc = i.popFront();
      if (m_verbose) {
        std::cout << "  " << std::setw(5) << offset(pc) << ":" <<
                     stateToString(cur) << " " <<
                     instrToString(pc, unit()) << std::endl;
      }
      ok &= checkInputs(&cur, pc, b);
      auto const op = peek_op(pc);
      auto const flags = instrFlags(op);
      if (flags & TF) ok &= checkTerminal(&cur, pc);
      if (flags & FF) ok &= checkFpi(&cur, pc, b);
      if (isIter(pc)) ok &= checkIter(&cur, pc);
      ok &= checkOutputs(&cur, pc, b);
    }
    ok &= checkSuccEdges(b, &cur);
  }
  // Make sure eval stack is correct at start of each try region
  for (auto& handler : m_func->ehtab()) {
    if (handler.m_type == EHEnt::Type::Catch) {
      ok &= checkEHStack(handler, builder.at(handler.m_base));
    }
  }
  return ok;
}

bool FuncChecker::checkSuccEdges(Block* b, State* cur) {
  bool ok = true;
  // Reachable catch blocks and fault funclets have an empty stack.
  if (m_graph->exn_cap > 0) {
    int save_stklen = cur->stklen;
    int save_fpilen = cur->fpilen;
    cur->stklen = 0;
    cur->fpilen = 0;
    for (BlockPtrRange i = exnBlocks(m_graph, b); !i.empty(); ) {
      ok &= checkEdge(b, *cur, i.popFront());
    }
    cur->stklen = save_stklen;
    cur->fpilen = save_fpilen;
  }
  if (isIter(b->last) && numSuccBlocks(b) == 2) {
    // IterInit* and IterNext*, Both implicitly free their iterator variable
    // on the loop-exit path.  Compute the iterator state on the "taken" path;
    // the fall-through path has the opposite state.
    int id = getImmIva(b->last);
    auto const last_op = peek_op(b->last);
    bool taken_state =
      (last_op == OpIterNext || last_op == OpIterNextK ||
       last_op == OpMIterNext || last_op == OpMIterNextK ||
       last_op == OpWIterNext || last_op == OpWIterNextK);
    bool save = cur->iters[id];
    cur->iters[id] = taken_state;
    if (m_verbose) {
      std::cout << "        " << stateToString(*cur) <<
        " -> B" << b->succs[1]->id << std::endl;
    }
    ok &= checkEdge(b, *cur, b->succs[1]);
    cur->iters[id] = !taken_state;
    if (m_verbose) {
      std::cout << "        " << stateToString(*cur) <<
                   " -> B" << b->succs[0]->id << std::endl;
    }
    ok &= checkEdge(b, *cur, b->succs[0]);
    cur->iters[id] = save;
  } else {
    // Other branch instructions send the same state to all successors.
    if (m_verbose) {
      std::cout << "        " << stateToString(*cur) << std::endl;
    }
    for (BlockPtrRange i = succBlocks(b); !i.empty(); ) {
      ok &= checkEdge(b, *cur, i.popFront());
    }
  }
  if (cur->mbr_live) {
    // MBR must not be live across control flow edges.
    error("Member base register live at end of B%d\n", b->id);
    ok = false;
  }
  return ok;
}

bool FuncChecker::checkEHStack(const EHEnt& handler, Block* b) {
  const State& state = m_info[b->id].state_in;
  if (!state.stk) return true; // ignore unreachable block
  if (state.fpilen != 0) {
    error("EH region starts with non-empty FPI stack at B%d\n",
           b->id);
    return false;
  }
  return true;
}

/**
 * Check the edge b->t, given the current state at the end of b.
 * If this is the first edge ->t we've seen, copy the state to t.
 * Otherwise, require the state exactly match.
 */
bool FuncChecker::checkEdge(Block* b, const State& cur, Block *t) {
  bool ok = true;
  State& state = m_info[t->id].state_in;
  if (!state.stk) {
    copyState(&state, &cur);
    return true;
  }
  // Check stack.
  if (state.stklen != cur.stklen) {
    reportStkMismatch(b, t, cur);
    return false;
  }
  for (int i = 0, n = cur.stklen; i < n; i++) {
    if (state.stk[i] != cur.stk[i]) {
      error("mismatch on edge B%d->B%d, current %s target %s\n",
             b->id, t->id, stkToString(n, cur.stk).c_str(),
             stkToString(n, state.stk).c_str());
      return false;
    }
  }
  // Check FPI stack.
  if (state.fpilen != cur.fpilen) {
    error("FPI stack depth mismatch on edge B%d->B%d, "
           "current %d target %d\n", b->id, t->id, cur.fpilen, state.fpilen);
    return false;
  }
  for (int i = 0, n = cur.fpilen; i < n; i++) {
    if (state.fpi[i] != cur.fpi[i]) {
      error("FPI mismatch on edge B%d->B%d, current %s target %s\n",
             b->id, t->id, fpiToString(cur.fpi[i]).c_str(),
             fpiToString(state.fpi[i]).c_str());
      return false;
    }
  }
  // Check iterator variable state.
  if (false /* TODO(#1097182): Iterator verification disabled */) {
    for (int i = 0, n = numIters(); i < n; i++) {
      if (state.iters[i] != cur.iters[i]) {
        error("mismatched iterator state on edge B%d->B%d, "
               "current %s target %s\n", b->id, t->id,
               iterToString(cur).c_str(), iterToString(state).c_str());
        return false;
      }
    }
  }
  return ok;
}

void FuncChecker::reportStkUnderflow(Block*, const State& cur, PC pc) {
  int min = cur.fpilen > 0 ? cur.fpi[cur.fpilen - 1].stkmin : 0;
  error("Rule2: Stack underflow at PC %d, min depth %d\n",
         offset(pc), min);
}

void FuncChecker::reportStkOverflow(Block*, const State& cur, PC pc) {
  error("Rule2: Stack overflow at PC %d\n", offset(pc));
}

void FuncChecker::reportStkMismatch(Block* b, Block* t, const State& cur) {
  const State& st = m_info[t->id].state_in;
  error("Rule1: Stack mismatch on edge B%d->B%d; depth %d->%d\n",
          b->id, t->id, cur.stklen, st.stklen);
}

void FuncChecker::reportEscapeEdge(Block* b, Block* s) {
  error("Edge from B%d to offset %d escapes function\n",
         b->id, offset(s->start));
}

/**
 * Check that the offset is within the region and it lands on an exact
 * instruction start.
 */
bool FuncChecker::checkOffset(const char* name, Offset off,
                              const char* regionName, Offset base,
                              Offset past, bool check_instrs) {
  assert(past >= base);
  if (off < base || off >= past) {
    error("Offset %s %d is outside region %s %d:%d\n",
           name, off, regionName, base, past);
    return false;
  }
  if (check_instrs && !m_instrs.get(off - m_func->base())) {
    error("label %s %d is not on a valid instruction boundary\n",
           name, off);
    return false;
  }
  return true;
}

/**
 * Check that the given inner region is valid, within the outer region, and
 * the inner region boundaries are exact instructions.
 */
bool FuncChecker::checkRegion(const char* name, Offset b, Offset p,
                               const char* regionName, Offset base,
                               Offset past, bool check_instrs) {
  assert(past >= base);
  if (p < b) {
    error("region %s %d:%d has negative length\n",
           name, b, p);
    return false;
  }
  if (b < base || p > past) {
    error("region %s %d:%d is not inside region %s %d:%d\n",
           name, b, p, regionName, base, past);
    return false;
  } else if (check_instrs &&
             (!m_instrs.get(b - m_func->base()) ||
              !m_instrs.get(p - m_func->base()))) {
    error("region %s %d:%d boundaries are inbetween instructions\n",
           name, b, p);
    return false;
  }
  return true;
}

}} // HPHP::Verifier
